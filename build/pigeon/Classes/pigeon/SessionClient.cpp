#include "SessionActor.h"
#include "Serializer.h"
#include "Deserializer.h"
#include "define.h"
#include "RPCTransport.h"

SessionClient::SessionClient()
{
    _addrinfo = NULL;

    _state = stClosed;
#if defined(WIN32)
	_sockfd = INVALID_SOCKET;
#else
    _sockfd = -1;
#endif

	memset((char*)&_readBuffer, 0, sizeof(_readBuffer));
	memset((char*)&_writeBuffer, 0, sizeof(_writeBuffer));

    _isRepairing = false;
    _connCounter = 0;
    _connTimeout = 0;

    _isHBTimedout = false;
    _hbinterval = 0;
    _hbtimeout = 0;

    _testState = stTestStopped;
    _testTimeout = 0;

    _dataCrypto = NULL;
    // Generate a 1024 bits keypair.
    _rsa.generateKeypair(1024, RSA_F4);

	memset(&_current, 0, sizeof(_current));
    _isNeedRecord = true;
    _heartBeatDelay = 0;
    _cryptoConfig = "";
    _cryptoType = 0;
}

SessionClient::~SessionClient()
{
    if (_addrinfo) {
        freeaddrinfo(_addrinfo);
        _addrinfo = NULL;
    }
    if (_sockfd>=0) {
#if defined(WIN32)
		closesocket(_sockfd);
		_sockfd = INVALID_SOCKET;
#else
        ::close(_sockfd);
        _sockfd = -1;
#endif
    }
    _cleanupQueues();
    clearMsgBuffer(&_readBuffer);
    clearMsgBuffer(&_writeBuffer);
}

int SessionClient::connect(const char* host, in_port_t port, long timeout)
{
    if (_state!=stClosed) {
        return EALREADY;
    }

    if (host==NULL || strlen(host)==0) {
        return EINVAL;
    }
    _hostname = host;
    _port = port;

    _connTimeout = timeout;

    _isRepairing = false;
    _connCounter = 0;
    _sid = "";

    _cleanupQueues();
    int error = _newConnection();
    if (error!=0 && error!=EINPROGRESS) {
        return error;
    }
    return 0;
}

int SessionClient::reconnect()
{
    if (_state==stClosing) {
        return ENOTCONN;
    }
    if (_isRepairing) {
        return EALREADY;
    }
    if (_sockfd>=0) {
#if defined(WIN32)
		closesocket(_sockfd);
		_sockfd = INVALID_SOCKET;
#else
		::close(_sockfd);
		_sockfd = -1;
#endif
        _state = stBroken;
    }
    int error = _newConnection();
    if (error!=0 && error!=EINPROGRESS) {
        // NLOG(NL_ERROR, "Failed to reconnect: %d\n", error);
        _state = stBroken2;
        return error;
    }
    _isRepairing = true;
    return 0;
}

void SessionClient::close()
{
    if (_state!=stClosed && _state!=stClosing) {
        struct data_t tosend;
		memset(&tosend, 0, sizeof(tosend));
        tosend.type = kTClose;
        tosend.valid = 1;

        if (_state==stEstablished) {
            _sendQueue.push_back(tosend);
        } else {
            _sendQueue.push_front(tosend);
        }

        _state = stClosing;

        if (_sockfd<0) {
            _state = stClosed;
        }
    }

    _testState = stTestStopped;
}

bool SessionClient::isClosed() const
{
    return _state == stClosed;
}

bool SessionClient::isBroken() const
{
    return _state == stBroken2;
}

void SessionClient::_cleanupQueues()
{
    if (_current.buffer) {
        ::free(_current.buffer);
    }
	memset(&_current, 0, sizeof(_current));

    DataQueue::iterator it = _sendQueue.begin();
    for (; it!=_sendQueue.end(); ++it) {
        void* buffer = it->buffer;
        if (buffer) { ::free(buffer); }
    }
    _sendQueue.clear();

    it = _recvQueue.begin();
    for (; it!=_recvQueue.end(); ++it) {
        void* buffer = it->buffer;
        if (buffer) { ::free(buffer); }
    }
    _recvQueue.clear();
}

int SessionClient::_newConnection()
{
    if (_addrinfo) {
        freeaddrinfo(_addrinfo);
        _addrinfo = NULL;
    }

    int error = 0;
    struct addrinfo hints;
    struct addrinfo *result = NULL;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED;
    // hints.ai_flags = AI_DEFAULT;
#if defined(ANDROID)
	hints.ai_flags = AI_PASSIVE;
#endif
    error = getaddrinfo(_hostname.c_str(), NULL, &hints, &result);
    if (error != 0) {
        return EINVAL;
    }

    _addrinfo = result;
    return _tryCandidateAddrinfos();
}

int SessionClient::_tryCandidateAddrinfos()
{
    int error = -1;
    while (_addrinfo!=NULL) {
        struct addrinfo* ai = _addrinfo;
        _addrinfo = ai->ai_next;
        ai->ai_next = NULL;
        if (ai->ai_family==AF_INET) {
            struct sockaddr_in* addr = (struct sockaddr_in*)ai->ai_addr;
            addr->sin_port = htons(_port);
        } else if (ai->ai_family==AF_INET6) {
            struct sockaddr_in6* addr = (struct sockaddr_in6*)ai->ai_addr;
            addr->sin6_port = htons(_port);
        } else {
            freeaddrinfo(ai);
            continue;
        }

        error = _tryConnecting(ai);
        freeaddrinfo(ai);
        if (error==0 || error==EINPROGRESS) {
            return error;
        }
    }

    // All candidates are tried and failed. Return the errno of last trying.
    return error;
}

int SessionClient::_tryConnecting(struct addrinfo* ai)
{
#if defined(WIN32)
	assert(_sockfd == INVALID_SOCKET && "_sockfd is in using");
#else
	assert(_sockfd<0 && "_sockfd is in using");
#endif

    // if (_sockfd>=0) {
    //     ::close(_sockfd);
    //     _sockfd = -1;
    // }

    _sockfd = ::socket(ai->ai_family, ai->ai_socktype, 0);
#if defined(WIN32)
	if (_sockfd == SOCKET_ERROR)
	{
		return errno;
	}
#else
    if (_sockfd<0) {
        // Failed to open socket
        return errno;
    }
#endif

    gettimeofday(&_connTime, NULL);

    int error = 0;
    do {
#if defined(WIN32)
		//u_long non_blk = 1; //非阻塞
		u_long non_blk = 0; //阻塞
		ioctlsocket(_sockfd, FIONBIO, (unsigned long*) &non_blk);
#else
		int flags = fcntl(_sockfd, F_GETFL, 0);
		fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);
#endif
		

        // int recvlowat = 4;
        // if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVLOWAT, &recvlowat, sizeof(recvlowat)) < 0) {
        //     error = errno;
        //     break;
        // }
#if defined(__APPLE__)
        int on = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
#if defined(WIN32)
		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = inet_addr(_hostname.c_str());  
		addrSrv.sin_family = AF_INET;  
		addrSrv.sin_port = htons(_port);
		int result = ::connect(_sockfd, (SOCKADDR*)&addrSrv, sizeof(addrSrv));
		if (result == 0)
		{
			// 设回为非阻塞socket  
			u_long non_blk = 1; 
			ioctlsocket(_sockfd, FIONBIO, (unsigned long*)&non_blk); //设置为阻塞模式  
		}
#else
		int result = ::connect(_sockfd, ai->ai_addr, ai->ai_addrlen);
#endif

        // fcntl(_sockfd, F_SETFL, flags);
        if (result==0) {
            // Connect successfully.
            // This may happen when client and server run on the same machine.
			_isRepairing = false;
            _state = stConnected;
			initMsgBufferForReading(&_readBuffer);
			clearMsgBuffer(&_writeBuffer);
        } else if (result<0) {
            error = errno;
            if (error==EINPROGRESS) {
                _state = stConnecting;
            }
        } else {
            // assert(false && "never reach here");
            error = -1; // unkown error
        }
    } while (0);

    if (error!=0 && error!=EINPROGRESS) {
#if defined(WIN32)
		closesocket(_sockfd);
		_sockfd = INVALID_SOCKET;
#else
        ::close(_sockfd);
		_sockfd = -1;
#endif
       
    }

    return error;
}

int SessionClient::testConnection(long timeout)
{
    if (_sockfd<0 || !ready()) {
        return ENOTCONN;
    }
    if (_testState!=stTestStopped) {
        return EALREADY;
    }
    gettimeofday(&_testTime, NULL);
    if (difftimeval(&_testTime, &_hbrecvtime) < 500) {
        // Connection was active in the past 500ms.
        return 0;
    }
    if (timeout<=0) {
        return ETIMEDOUT;
    }
    _testState = stTestStarted;
    _testTimeout = timeout;
    return EINPROGRESS;
}

size_t SessionClient::getSendLimits() const
{
    // n表示可以直接发送的最大数据长度
    // 算法中减掉的6为CRC32(4 bytes)以及length(2 bytes)所占用的长度
#if defined(WIN32)
	size_t n = 2 ^ (8 * sizeof(uint32_t)) - 6;
#else
    size_t n = 2^(8*sizeof(message_t::length)) - 6;
#endif
    // 考虑到加密算法可能需要填充数据，send()支持的长度须减掉最大填充长度
    // NOTE 此处假设所有支持的加密算法填充数据不超过512字节
    // 已支持的加密算法最大填充字节数如下：
    //   AES    16 (128 bits)
    return n - 512;
}

bool SessionClient::ready() const
{
    if (_state==stClosed || _state==stClosing) {
        return false;
    }
    return true;
}

bool SessionClient::send(const void* buffer, size_t length)
{
    if (!ready()) {
        return false;
    }

    if (buffer==NULL || length==0 || length>getSendLimits()) {
        return false;
    }

    struct data_t tosend;
	memset(&tosend, 0, sizeof(tosend));
    tosend.type = kTData;
    tosend.valid = 1;
    tosend.length = length;

    const uint8_t* src = (const uint8_t*)buffer;
    uint8_t* dest = (uint8_t*)::malloc(length);
    uint32_t crc32val = 0^0xffffffff;
    size_t i = 0;
    while (i < length) {
        uint8_t ch = src[i];
        crc32val = CRC32(crc32val, ch);
        dest[i] = ch;
        i++;
    }
    tosend.crc32 = crc32val^0xffffffff;
    tosend.buffer = dest;

    _sendQueue.push_back(tosend);

    return true;
}

size_t SessionClient::available() const
{
    return _recvQueue.size();
}

void* SessionClient::receive(size_t* length)
{
    if (_recvQueue.size()==0) {
        return NULL;
    }
    struct data_t recv = _recvQueue.front();
    if (length) {
        *length = recv.length;
    }
    _recvQueue.pop_front();
    return recv.buffer;
}

void SessionClient::freeReceived(void* buffer)
{
    if (buffer) {
        ::free(buffer);
    }
}

int SessionClient::fdset(int* maxfd, fd_set* rset, fd_set* wset, fd_set* eset)
{
    if (_sockfd>=0) {
        if (rset) { FD_SET(_sockfd, rset); }
        if (wset) { FD_SET(_sockfd, wset); }
        if (eset) { FD_SET(_sockfd, eset); }
        if (maxfd) { *maxfd = MAX(_sockfd, *maxfd); }
    }
    return 0;
}

void SessionClient::update(
    const fd_set* rset, const fd_set* wset, const fd_set* eset)
{
    if (_state==stConnecting) {
        assert(_sockfd>=0 && "invalid _sockfd");
        _checkConnecting(rset, wset, eset);
    }
    _heartBeatDelay = 0;
#if defined(WIN32)
	if (_sockfd != INVALID_SOCKET) {
#else
    if (_sockfd>=0) {
#endif
        if (FD_ISSET(_sockfd, rset)) {
            do {
                int error = fillMsgBuffer(&_readBuffer, _sockfd);
                if (error!=0) {
#if defined(WIN32)
					if (error != EWOULDBLOCK && error != 10035) {
#else
                    if (error!=EWOULDBLOCK) {
#endif
                        // process fatal errors
                        // possible errors
                        //   EOF  # socket closed by server
                        //   ECONNRESET
                        NLOG(NL_ERROR, "fill message error: %d\n", error);
#if defined(WIN32)
						closesocket(_sockfd);
						_sockfd = INVALID_SOCKET;
#else
						::close(_sockfd);
						_sockfd = -1;
#endif
						_state = stBroken2;
                        if (error==EOF) {
                            raiseException(ErrorTypes_ServerClosed, 0);
                        } else {
                            raiseException(ErrorTypes_ReadError, error);
                        }
                    }
                    break;
                }
                if (_readBuffer.msg) {
                    _processMessage(_readBuffer.msg);
                    if (_sockfd<0) { break; }
                }
                initMsgBufferForReading(&_readBuffer);
            } while (true);
        }
    }

    if (_sockfd>=0) {
        if (FD_ISSET(_sockfd, wset)) {
            do {
                if (_writeBuffer.msg==NULL) {
                    struct message_t* msg = _generateMessage();
                    if (msg==NULL) {
                        break;
                    }
#if DEBUG_PIGEON
					uint32_t datalen = 0;
                    decodeInteger(sizeof(datalen), &datalen, &msg->length);
                    NLOG(NL_INFO, "send (type=0x%x, len=%u)\n", msg->type, datalen);
                    if (datalen>0) {
                        hexdump(msg->data, datalen, msg->data);
                    }
#endif
                    initMsgBufferForWriting(&_writeBuffer, msg);
                }
                int error = flushMsgBuffer(&_writeBuffer, _sockfd);
                if (error!=0) {
                    if (error!=EWOULDBLOCK) {
                        // process fatal errors
                        // possible errors
                        //    EOF    # closed by server
                        //    EPIPE  # try writing a closed socket
                        NLOG(NL_ERROR, "flush message error: %d\n", error);
#if defined(WIN32)
						closesocket(_sockfd);
						_sockfd = INVALID_SOCKET;
#else
						::close(_sockfd);
						_sockfd = -1;
#endif 
						_state = stBroken2;
                        if (error==EOF) {
                            raiseException(ErrorTypes_ServerClosed, 0);
                        } else {
                            raiseException(ErrorTypes_WriteError, error);
                        }
                    }
                    break;
                }
                // send ok
                if (_current.valid) {
                    if (_current.buffer) {
                        ::free(_current.buffer);
                    }
                    if (_current.type==kTClose) {
                        // ::shutdown(_sockfd, SHUT_RDWR);
#if defined(WIN32)
						closesocket(_sockfd);
						_sockfd = INVALID_SOCKET;
#else
						::close(_sockfd);
						_sockfd = -1;
#endif
                        _state = stClosed;
                    }
					memset(&_current, 0, sizeof(_current));
                }
                clearMsgBuffer(&_writeBuffer);
            } while (true);
        }
    }

    if (_testState!=stTestStopped) {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(&now, &_testTime) >= _testTimeout) {
            _testState = stTestStopped;
            ArgTestResult result =  { ETIMEDOUT };
            triggerEventHandler(kEVTTestCompleted, &result);
        }
    }

	if (!_isHBTimedout && _hbtimeout>0 && (_state == stEstablished || _state == stBroken || _state == stEstablishing)) {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(&now, &_hbrecvtime) >= _hbtimeout) {
            // heart beat timeout
            _isHBTimedout = true;
			_state = stBroken2;
            raiseException(ErrorTypes_Inactive, ETIMEDOUT);
        }
    }

    if (_sockfd<0) {
        if (_state==stClosing) {
            _state = stClosed;
        } else if (_state==stBroken) {
            // reconnect
            int error = _newConnection();
            if (error!=0 && error!=EINPROGRESS) {
                _isRepairing = false;
                _state = stBroken2;
                NLOG(NL_ERROR, "Failed to reconnect: %d\n", error);
                raiseException(ErrorTypes_ConnectBroken, error);
            } else {
                _isRepairing = true;
            }
        }
    }
}

void SessionClient::_checkConnecting(
    const fd_set* rset, const fd_set* wset, const fd_set* eset)
{
    int error = 0;
    if (FD_ISSET(_sockfd, rset) || FD_ISSET(_sockfd, wset)) {
        int len = sizeof(error);
#if defined(WIN32)
		if (getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&len) < 0) {
#else
        if (getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
#endif
            // Solaris pending error
            error = errno;
        }
        if (error==0) {
            _connCounter++;
            _isHBTimedout = false;
            _isRepairing = false;
            _state = stConnected;
            // reset read/write buffers
            initMsgBufferForReading(&_readBuffer);
            clearMsgBuffer(&_writeBuffer);
            // NLOG(NL_INFO, "-->stConnected\n");
        }
    } else if (_connTimeout>0) {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(&now, &_connTime) >= _connTimeout) {
            NLOG(NL_ERROR, "connection timeout\n");
            error = ETIMEDOUT;
        }
    }

    if (error) {
        // connection failed
#if defined(WIN32)
		closesocket(_sockfd);
		_sockfd = INVALID_SOCKET;
#else
		::close(_sockfd);
		_sockfd = -1;
#endif

        if (_addrinfo!=NULL) {
            error = _tryCandidateAddrinfos();
            if (error==0 || error==EINPROGRESS) {
                return;
            }
            if (_sockfd>=0) {
#if defined(WIN32)
				closesocket(_sockfd);
				_sockfd = INVALID_SOCKET;
#else
				::close(_sockfd);
				_sockfd = -1;
#endif
            }
        }

        if (_connCounter==0) {
            _isRepairing = false;
            _state = stClosed;
            NLOG(NL_ERROR, "connection failed: %d\n", error);
            raiseException(ErrorTypes_ConnectFailed, error);
        } else {
            if (_isRepairing) {
                _isRepairing = false;
                _state = stBroken2;
                NLOG(NL_ERROR, "Failed to reconnect: %d\n", error);
                raiseException(ErrorTypes_ConnectBroken, error);
            } else {
                // will try re-connecting later
                _state = stBroken;
            }
        }
    }
}

void SessionClient::_processMessage(struct message_t* msg)
{
	uint32_t datalen = 0;
    decodeInteger(sizeof(datalen), &datalen, &msg->length);

#if DEBUG_PIGEON
    NLOG(NL_INFO, "received (type=0x%x, len=%u)\n", msg->type, datalen);
    if (datalen>0) {
        hexdump(msg->data, datalen, msg->data);
    }
#endif

    if (msg->type==kTError) {
        // assert(datalen>=4);
        int32_t svrerr = -1;
        Deserializer reader(msg->data, datalen);
        reader.readInt32(&svrerr);
        NLOG(NL_ERROR, "Server error [%d]: %s\n", svrerr, getErrorString(svrerr));
        if (svrerr==SvrErrors_InvalidSession 
			|| svrerr == SvrErrors_CannotFindSession
			|| svrerr==SvrErrors_AuthFailure
			|| svrerr == SvrErrors_UnknownAlgorithms
			|| svrerr == SvrErrors_UnknownCompressAlgorithms) {
            close();
			raiseException(ErrorTypes_SessionError, svrerr);
		}
		else if (svrerr == SvrErrors_CRCCheckFailed)
		{
			raiseException(ErrorTypes_CRCCheckFailed, svrerr);
		}
		else if (svrerr == SvrErrors_Request_ID_Error)
		{
			triggerEventHandler(kEVTRequestIdError, NULL);
		}

        return;
    }

    if (_state==stEstablishing) {
        notifySignalDelay();
        if (msg->type==kTAccept) {
            size_t len2 = _rsa.calcDecryptedLength(datalen);
            // assert(len2>0)
            size_t realLength = 0;
            uint8_t* decrypted = (uint8_t*)::malloc(len2);
            if (!_rsa.decryptWithPrivateKey(decrypted, &realLength, msg->data, datalen)) {
                // decrypt error
                NLOG(NL_ERROR, "Decrypt acception message error.\n");
                close();
                raiseException(ErrorTypes_EstablishError, -1);
                return;
            }
            do {
                bool firstAccepted = (_sid.length()==0);
                int16_t tmplen;
                Deserializer reader(decrypted, realLength, ::free);
                if (reader.readInt16(&tmplen) != 0 || tmplen==0) { break; }
                if (reader.readString(_sid, tmplen) != 0) { break; }
                uint8_t encryption = 0;
                if (reader.readUint8(&encryption) != 0) { break; }
				uint8_t compr = 0;
				if (reader.readUint8(&compr) != 0) { break; }
                if (_dataCrypto!=NULL) {
                    delete _dataCrypto; _dataCrypto = NULL;
                }
                if (encryption!=0) {
                    if (reader.readInt16(&tmplen) != 0) { break; }
                    std::string config;
                    if (reader.readString(config, tmplen) != 0) { break; }
                    _dataCrypto = SymmCryptoFactory::getInstance()->newCrypto(encryption);
                    _cryptoType = encryption;
                    if (!_dataCrypto) {
                        // encryption algorithm is NOT supported
                        break;
                    }
                    _cryptoConfig = config;
                    _dataCrypto->setConfig(config.c_str(), config.length());
                }
				ArgException args = { kEVTComprType, compr };
				triggerEventHandler(kEVTComprType, &args);
				
                _state = stEstablished;
                gettimeofday(&_hbsendtime, NULL);
                gettimeofday(&_hbrecvtime, NULL);
                gettimeofday(&_recordsendtime, NULL);
                // NLOG(NL_INFO, "-->stEstablished\n");
                if (firstAccepted) {
                    triggerEventHandler(kEVTConnected, NULL);
                } else {
                    triggerEventHandler(kEVTConnRepaired, NULL);
                }
                return;
            } while (false);
            // parse message error
            NLOG(NL_ERROR, "Invalid acception message structure.\n");
            close();
            raiseException(ErrorTypes_EstablishError, -2);
        }
    } else if (_state==stEstablished) {
        // if (msg->type==kTHeartbeat) {
        //     gettimeofday(&_hbrecvtime, NULL);
        // } else
        gettimeofday(&_hbrecvtime, NULL);
        if (_testState!=stTestStopped) {
            notifySignalDelay();
            _testState = stTestStopped;
            ArgTestResult result = { 0 }; // works well
            triggerEventHandler(kEVTTestCompleted, &result);
        }
        if (msg->type==kTData) {
            // receive data and append it to the receive queue
            data_t received;
			memset(&received, 0, sizeof(received));
            received.type = kTData;

            Deserializer reader(msg->data, datalen - sizeof(msg->type));
            do {
                if (reader.readUint32(&received.crc32)!=0) { break; }
				if (reader.readUint32(&received.length) != 0) { break; }
                if (_dataCrypto) {
                    size_t enclen = reader.available();
                    size_t declen = _dataCrypto->calcDecryptedLength(enclen);
                    received.buffer = (uint8_t*)::malloc(declen);
                    const uint8_t* src = reader.buffer() + reader.tell();
                    if (!_dataCrypto->decrypt(received.buffer, NULL, src, enclen)) {
                        NLOG(NL_ERROR, "Failed to decrypt message data.\n");
                        break;
                    }
                } else {
                    size_t length = reader.available();
                    received.buffer = (uint8_t*)::malloc(length);
                    if (reader.readBytes(received.buffer, received.length)!=0) {
                        break;
                    }
                }
                // check data with CRC32
                uint32_t crc32val = crc32(0, received.buffer, received.length);
                if (crc32val!=received.crc32) {
                    NLOG(NL_ERROR,
                        "Check failed: crc32(original)=0x%x, crc32(received)=0x%x",
                        received.crc32,
                        crc32val);
                    break;
                }
                received.valid = 1;
            } while (false);
            if (received.valid) {
                _recvQueue.push_back(received);
            } else {
                if (received.buffer) { ::free(received.buffer); }
            }
        }
        else{
            notifySignalDelay();
        }
    }
}

void SessionClient::recordHeartBeatSendTime()
{
    if(_isNeedRecord)
    {
        _isNeedRecord = false;
        gettimeofday(&_recordsendtime, NULL);
    }
}
void SessionClient::notifySignalDelay()
{
    if(!_isNeedRecord)
    {
        _isNeedRecord = true;
        struct timeval now;
        gettimeofday(&now, NULL);
        _heartBeatDelay =  difftimeval(&now, &_recordsendtime);
    }
}
struct SessionClient::message_t* SessionClient::_generateMessage()
{
    if (_state==stConnected) {
        Serializer writer(256);
        uint8_t msgType;
        if (_connCounter==1 || _sid.length()==0) {
            msgType = kTRegister;
            size_t keylen = 0;
            uint8_t* pubkey = _rsa.encodePublicKey(&keylen);
            // assert(keylen<0x7fff);
            writer.writeInt16((int16_t)keylen);
            writer.writeBytes(pubkey, keylen);
            _rsa.freeKeyBuffer(pubkey);
            size_t countpos = writer.tell();
            writer.writeInt8(0);
            int count = SymmCryptoFactory::getInstance()->listCryptos(
                [&writer](int cryptoType, const char*){
                    // assert(cryptoType<=0x7f);
                    writer.writeInt8((int8_t)cryptoType);
                });
            writer.seek(countpos, SEEK_SET);
            // assert(cryptoType<=0x7f);
            writer.writeInt8((int8_t)count);

			writer.seek(writer.length(), SEEK_SET);
			//压缩算法，目前只支持一种，后续扩展
			writer.writeInt8(1);
			writer.writeInt8(RPCTransport::kComprSnappy);
        } else {
            msgType = kTRepair;
            size_t slen = _sid.length();
            // assert(slen<=0x7fff);
            writer.writeInt16((int16_t)slen);
            writer.writeBytes(_sid.c_str(), slen);
            size_t encryptedLength = _rsa.calcEncryptedLength(slen);
            uint8_t* encrypted = (uint8_t*)::malloc(encryptedLength);
            _rsa.encryptWithPrivateKey(encrypted,
                                    &encryptedLength,
                                    (const uint8_t*)_sid.c_str(),
                                    slen);
            // assert(encryptedLength<=0x7fff);
            writer.writeInt16((int16_t)encryptedLength);
            writer.writeBytes(encrypted, encryptedLength);
			
        }
		gettimeofday(&_hbrecvtime, NULL);
        recordHeartBeatSendTime();
        _state = stEstablishing;
        assert(writer.length()<=0xffff);
        return packupMessage(msgType, writer.buffer(), writer.length());
    } else if (_state==stEstablished || _state==stClosing) {
        if (_testState==stTestStarted) {
            _testState = stTestWaitResp;
            gettimeofday(&_hbsendtime, NULL);
            recordHeartBeatSendTime();
            return packupMessage(kTHeartbeat, NULL, 0);
        } else if (_hbinterval > 0) {
            struct timeval now;
            gettimeofday(&now, NULL);
            if (difftimeval(&now, &_hbsendtime) > _hbinterval) {
                _hbsendtime = now;
                recordHeartBeatSendTime();
                return packupMessage(kTHeartbeat, NULL, 0);
            }
        }

        if (_current.buffer!=NULL) {
            // will resend current data
            assert(_current.valid && _current.length>0);
        } else {
            // send data in send queue
            if (_sendQueue.size() > 0) {
                _current = _sendQueue.front();
                _sendQueue.pop_front();
            }
        }
        if (_current.valid) {
            if (_current.type==kTClose) {
                // bzero(&_current, sizeof(_current));
                return packupMessage(kTClose, NULL, 0);
            }
            assert(_current.type==kTData);
            size_t datalen = 0;
            if (_dataCrypto) {
                datalen = _dataCrypto->calcEncryptedLength(_current.length);
            } else {
                datalen = _current.length;
            }
            // totallen = sizeof(crc32) + sizeof(length field) + datalen
            size_t totallen = 4 + 4 + datalen;
            Serializer writer(totallen);
            writer.writeUint32(_current.crc32);  // CRC32 of original data
            writer.writeInt32(_current.length); // length of original data
            if (_dataCrypto) {
                size_t pos = writer.tell();
                writer.reserve(datalen);
                uint8_t* dest = writer.buffer()+pos;
                if (!_dataCrypto->encrypt(dest, NULL, _current.buffer, _current.length)) {
                    NLOG(NL_ERROR, "Failed to encrypt message data.\n");
                }
            } else {
                writer.writeBytes(_current.buffer, _current.length);
            }

            return packupMessage(kTData, writer.buffer(), writer.length());
        }
    }
    return NULL;
}

void SessionClient::triggerEventHandler(int event, void* args)
{
	_isRepairing = false;
    if (_evtHandler) {
        _evtHandler(this, event, args);
    }
}

void SessionClient::raiseException(int errtype, int detail)
{
	_isRepairing = false;
    if (_evtHandler) {
        ArgException args = {errtype, detail};
        _evtHandler(this, kEVTException, &args);
    }
}
std::string SessionClient::getCryptoConfig(){
    return _cryptoConfig;
}
int SessionClient::getCryptoType()
{
    return _cryptoType;
}
