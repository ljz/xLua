#ifndef __pigeon_sessionactor_h
#define __pigeon_sessionactor_h

#include "netbasic.h"
#include "NetCrypto.h"

class SessionActor
{
public:
    virtual ~SessionActor() {}

    virtual size_t getSendLimits() const = 0;
    virtual bool ready() const = 0;
    virtual bool send(const void* buffer, size_t length) = 0;
    virtual size_t available() const = 0;
    virtual void* receive(size_t* length) = 0;
    virtual void freeReceived(void* buffer) = 0;

    virtual int fdset(int* maxfd, fd_set* rset, fd_set* wset, fd_set* eset) = 0;
    virtual void update(const fd_set* rset,
                        const fd_set* wset,
                        const fd_set* eset) = 0;
    virtual bool isAlive() const = 0;
    virtual long getHeartBeatDelay() =0 ;
    
    virtual std::string getCryptoConfig() = 0;
    virtual int getCryptoType() = 0;
};

class MessageProtocol
{
public:
    enum MessageTypes {
        kTData = 0,
        kTHeartbeat = 1,
        kTRegister = 0x10,
        kTRepair   = 0x11,
        kTAccept   = 0x12,
        kTClose    = 0x13,
        kTError    = 0xff,
    };

#pragma pack (1) 
    struct message_t
    {
        //uint16_t length;  // Length of `data`. Encoded in big endian.
		uint32_t length;  //长度改成int
        uint8_t type;
        //uint8_t reserved;
        uint8_t data[0];
    };
#pragma pack () /*取消指定对齐，恢复缺省对齐*/

    struct msg_buffer_t {
        uint32_t remainingBytes;
        uint8_t* ptr;
        struct message_t* msg;
    };

    const char* getErrorString(int error);

    void encodeInteger(size_t bytes, void* dest, const void* src);
    void decodeInteger(size_t bytes, void* dest, const void* src);

	struct message_t* mallocMessage(uint32_t datalen);
    struct message_t* packupMessage(uint8_t type, const void* data, uint32_t datalen);
    void freeMessage(struct message_t* msg);

    void initMsgBufferForReading(struct msg_buffer_t* buffer, size_t datalen = 0);
    void initMsgBufferForWriting(struct msg_buffer_t* buffer, struct message_t* msg);
    void clearMsgBuffer(struct msg_buffer_t* buffer);
    int fillMsgBuffer(struct msg_buffer_t* buffer, int sockfd);
    int flushMsgBuffer(struct msg_buffer_t* buffer, int sockfd);
};

class SessionClient: public SessionActor, protected MessageProtocol
{
public:
    enum Event {
        kEVTException = 0,
        kEVTConnected = 1,
        kEVTConnRepaired = 2,
        kEVTTestCompleted = 3,
		kEVTRequestIdError = 4,
		kEVTComprType = 5,
    };

    struct ArgException {
        int errtype;
        int detail;
    };
    struct ArgTestResult {
        // 0: the test success
        // non-zero: the errno
        int error;
    };

    typedef void fnEventHandler(SessionClient* client, int event, void* args);
public:
    SessionClient();
    ~SessionClient();

    /*
     @desc  Connect to the server.
     @param host     host name of the server
     @param port     port of the server
     @param timeout  timeout in milliseconds
    */
    int connect(const char* host, in_port_t port, long timeout);
    void setConCfg(const char* host, in_port_t port, long timeout){
        _hostname = host;
        _port = port;
        _connTimeout = timeout;
    };
    int reconnect();
    void close();
    bool isClosed() const;
    bool isBroken() const;
    /**
     @desc  Test if the connection is active at present.
        This will send a heartbeat message and return EINPROGRESS. If any data are
        received before timed out, the test successes, otherwise it fails.
        `kEVTTestCompleted` will be triggered with `args` pointing to the `ArgTestResult`
        structure when the test completed.
     @param timeout  testing timeout in milliseconds.
     @returns 0 if testing success, EINPROGRESS if it's in process, other errno if fails.
    */
    int testConnection(long timeout);

    // Set the heart beat interval. If it is negative or 0, heart beat will not be sent.
    void setHeartbeatInterval(long millisecond) { _hbinterval = millisecond; }
    long getHeartbeatInterval() const { return _hbinterval; }
    // Set the timeout for heart beat. It should NOT be less than heart beat interval,
    // or timeout may happen improperly. If it is negative or 0, the client will not
    // check heart beat timeout.
    void setHeartbeatTimeout(long millisecond) { _hbtimeout = millisecond; }
    long getHeartbeatTimeout() const { return _hbtimeout; }
    // Setup the event handler. The event handler is alawys triggered during updating,
    // which means it is running in the net thread.
    void setEventHandler(const std::function<fnEventHandler>& handler) {
        _evtHandler = handler;
    }
    const std::function<fnEventHandler>& getEventHandler() const {
        return _evtHandler;
    }
    virtual std::string getCryptoConfig() override;
    virtual int getCryptoType() override;
    
    void recordHeartBeatSendTime();
    void notifySignalDelay();
    long getHeartBeatDelay(){ return _heartBeatDelay; }
    virtual size_t getSendLimits() const override;
    virtual bool ready() const override;
    virtual bool send(const void* buffer, size_t length) override;
    virtual size_t available() const override;
    virtual void* receive(size_t* length) override;
    virtual void freeReceived(void* buffer) override;
    virtual int fdset(int* maxfd, fd_set* rset, fd_set* wset, fd_set* eset) override;
    virtual void update(const fd_set* rset,
                        const fd_set* wset,
                        const fd_set* eset) override;
    virtual bool isAlive() const override { return !isClosed(); }
protected:
    int _newConnection();
    int _tryCandidateAddrinfos();
    int _tryConnecting(struct addrinfo* ai);
    void _cleanupQueues();
    void _checkConnecting(const fd_set*, const fd_set*, const fd_set*);
    void _processMessage(struct message_t* msg);
    struct message_t* _generateMessage();
    void triggerEventHandler(int event, void* args);
    void raiseException(int errtype, int detail);

protected:
    struct data_t {
        uint8_t  valid;
        uint8_t  type;
        uint32_t length;
        uint32_t crc32;
        uint8_t* buffer;
    };

    typedef std::list<data_t> DataQueue;

    std::string _hostname;
    in_port_t _port;
    struct addrinfo* _addrinfo;

    int _state;
#if defined(WIN32)
	SOCKET _sockfd;
#else
    int _sockfd;
#endif
    struct msg_buffer_t _readBuffer;
    struct msg_buffer_t _writeBuffer;

    bool _isRepairing;
    bool _isHBTimedout;
    
    int _connCounter;
    long _connTimeout;
    struct timeval _connTime;

    int _testState;
    struct timeval _testTime;
    long _testTimeout;

    long _hbinterval, _hbtimeout;
    struct timeval _hbsendtime, _hbrecvtime ,_recordsendtime;
    long _heartBeatDelay;
    bool _isNeedRecord;
    std::function<fnEventHandler> _evtHandler;

    std::string _sid;
    RSACrypto _rsa;
    SymmCrypto* _dataCrypto;

    struct data_t _current;
    DataQueue _sendQueue;
    DataQueue _recvQueue;
    
    std::string _cryptoConfig;
    int _cryptoType;
};

class SessionServer: public SessionActor, protected MessageProtocol
{
public:
    void close();
};

class SessionListener: public SessionActor, protected MessageProtocol
{
public:
};

#endif
