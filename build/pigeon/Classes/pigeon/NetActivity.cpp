#include "NetActivity.h"
#include "NetController.h"
#include "define.h"

RPCClient::RPCClient(NetThread* workThread)
: RPCActivity(workThread)
, _rpcListener(this)
{
    _rpc = NULL;
    _client = NULL;
    _connBroken = false;
    
    // default heart beat settings:
    //   interval: 30s  timeout: 60s
    _hbinterval = 30*1000;
    _hbtimeout = 60*1000;
    _loopInterval = 1/30.0 * 1000;
    _listener = NULL;
}

RPCClient::~RPCClient()
{
    if (_rpc || _client) {
        close();
    }
}

void RPCClient::setHeartbeatInterval(long millisecond)
{
    _hbinterval = millisecond;
    if (_client) {
        _client->setHeartbeatInterval(_hbinterval);
    }
}

void RPCClient::setHeartbeatTimeout(long millisecond)
{
    _hbtimeout = millisecond;
    if (_client) {
        _client->setHeartbeatTimeout(_hbtimeout);
    }
}

void RPCClient::setloopInterval(int millisecond)
{
    _loopInterval = millisecond;
}

bool RPCClient::connect(const char *host, in_port_t port, long timeout)
{
    if (_workThread==NULL) {
        return false;
    }
    if (_rpc && _client) {
        // already connected
        _client->setConCfg(host,port,timeout);
        return _client->reconnect() == 0;
    }

    assert(_rpc==NULL && _client==NULL);
    _client = new SessionClient();
    _client->setHeartbeatInterval(_hbinterval);
    _client->setHeartbeatTimeout(_hbtimeout);
    int result = _client->connect(host, port, timeout);
    if (result!=0) {
        delete _client;
        _client = NULL;
        return false;
    }
    
	_client->setEventHandler([this](SessionClient* client, int event, void* args) {
		assert(_workThread == NULL || _workThread->isInWorkthread());
		this->onClientEvent(client, event, args);
	});
    
    _rpc = new RPCTransport();
    _rpc->attachSession(_client);
    _rpc->setListener(&_rpcListener);
    
//    _workThread->addRPCTerminal(_rpc);
//    _workThread->addSession(_client);
    
    SessionClient* client = _client;
    RPCTransport* rpc = _rpc;
    _workThread->runTask([=](NetThread* thread, int situation) {
        if (situation) {
            _connBroken = false;
            thread->addRPCTerminal(rpc);
            thread->addSession(client);
            thread->setLoopInterval(_loopInterval);
        }
    });
    
    return true;
}

bool RPCClient::repair(bool force)
{
    if (_workThread==NULL || _client==NULL) {
        return false;
    }
    
    if (_client->isClosed()) {
        return false;
    }
    
    _workThread->runTask([=](NetThread* thread, int situation) {
        if (situation) {
            _connBroken = false;
            if (!doRepair(force)) {
                _connBroken = true;
            }
        }
    });

    return true;
}

bool RPCClient::doRepair(bool force)
{
    if (_client==NULL || _client->isClosed()) {
        return false;
    }
    
    if (!force) {
        int result = _client->testConnection(3000);
        if (result==0 || result==EINPROGRESS || result==EALREADY) {
            if (result==0 && _rpc) {
                _rpc->restartUnfinishedRequests();
            }
            return true;
        }
    }
    
    int result = _client->reconnect();
    if (result==0 || result==EINPROGRESS || result==EALREADY) {
        return true;
    } else if(result==ENOTCONN) {
        return false;
    } else {
        if (!_connBroken) {
            _connBroken = true;
			if (_listener) { _listener->onError(this, Error_ConnectBroken, result); }
        }
    }
    
    return false;
}

void RPCClient::close()
{
    if (_workThread==NULL || _client==NULL) {
        return;
    }
    
    _rpc->setListener(NULL);
#if defined(WIN32)
	_client->setEventHandler([this](SessionClient* client, int event, void* args) {

	});
#else
    _client->setEventHandler(NULL);
#endif
    
    SessionClient* client = _client;
    _workThread->runTask([=](NetThread* thread, int situation) {
        if (situation) {
            client->close();
        }
    });
    
    class Cleaner: public NetThread::AutoObject {
        SessionActor* _session;
        RPCTransport* _rpc;
    public:
        Cleaner(SessionActor* session, RPCTransport* rpc) {
            _session = session; _rpc = rpc;
        }
        virtual bool tryCleaning(NetThread* thread, bool forced) override {
            if (forced || !_session->isAlive()) {
                thread->removeRPCTerminal(_rpc);
                thread->removeSession(_session);
                delete _rpc;
                delete _session;
                return true;
            }
            return false;
        }
    };
    
    _workThread->manageObject(new Cleaner(_client, _rpc));
    _client = NULL;
    _rpc = NULL;
}

void RPCClient::setListener(RPCClient::Listener *listener)
{
    if (_listener==listener) {
        return;
    }
    if (_listener!=NULL) {
        _listener->onUnbinded(this);
    }
    _listener = listener;
    if (_listener) {
        _listener->onBinded(this);
    }
}

RPCClient::Listener* RPCClient::getListener()
{
    return _listener;
}

void RPCClient::setReqRTOBounds(int lbound, int ubound)
{
    if (_rpc) {
        _rpc->setReqRTOBounds(lbound, ubound);
    }
}

void RPCClient::setReqTimeout(int milliseconds)
{
    if (_rpc) {
        _rpc->setReqTimeout(milliseconds);
    }
}

int RPCClient::getReqTimeout() const
{
    return _rpc!=NULL ? _rpc->getReqTimeout() : 0;
}

long RPCClient::getMaxReqDelay() const
{
    return _rpc!=NULL ? _rpc->getMaxReqDelay() : 0;
}

long RPCClient::getAvgReqDelay() const
{
    return _rpc!=NULL ? _rpc->getAvgReqDelay() : 0;
}

long RPCClient::remoteTimestamp(long* elb, long* eub) const
{
    if (_rpc) {
        return _rpc->remoteTimestamp(elb, eub);
    } else {
        if (*elb) { *elb = LONG_MIN; }
        if (*eub) { *eub = LONG_MAX; }
        return (long)time(NULL);
    }
}

int64_t RPCClient::remoteTimeMillis(long* elb, long* eub) const
{
    if (_rpc) {
        return _rpc->remoteTimeMillis(elb, eub);
    } else {
        if (*elb) { *elb = LONG_MIN; }
        if (*eub) { *eub = LONG_MAX; }
        struct timeval now; gettimeofday(&now, NULL);
        int64_t temp = ((int64_t)now.tv_sec)*1000 + now.tv_usec/1000;
		return temp;
	}
}
void RPCClient::timeCorrectorReset()
{
    if(_rpc)
        _rpc->timeCorrectorReset();
}
double RPCClient::remoteTimescale(double* lb, double* ub) const
{
    if (_rpc) {
        return _rpc->remoteTimescale(lb, ub);
    } else {
        if (*lb) { *lb = DBL_MIN; }
        if (*ub) { *ub = DBL_MAX; }
        return 1;
    }
}

int RPCClient::notify(int event, /*int compr,*/ const void* data, size_t len)
{
    if (_rpc) {
        return _rpc->notify(event, /*compr,*/ data, len);
    }
    return -0xff;
}

int RPCClient::request(int method,
                       //int compr,
                       const void* params,
                       size_t len,
                       RequestCallback callback,
                       uint16_t extraRTO)
{
    if (_rpc) {
        return _rpc->request(method, /*compr,*/ params, len, callback, extraRTO);
    }
    return -0xff;
}

int RPCClient::request(int request)
{
    if (_rpc) {
        return _rpc->request(request);
    }
    return -0xff;
}

int RPCClient::respond(int request, /*int compr,*/ int err, const void* data, size_t len)
{
    if (_rpc) {
        return _rpc->respond(request, /*compr,*/ err, data, len);
    }
    return -0xff;
}

void RPCClient::onRPCRequested(RPCTransport* rpc, int request, int method, const std::string& params)
{
    if (_listener) {
        _listener->onRequested(this, request, method, params);
    }
}

void RPCClient::onRPCNotified(RPCTransport* rpc, int event, const std::string& data,float timestamp)
{
    if (_listener) {
        _listener->onNotified(this, event, data,timestamp);
    }
}

void RPCClient::onRPCError(RPCTransport* rpc, int errtype, int detail)
{
#if DEBUG_PIGEON
    NLOG(NL_INFO, "[rpc error] %d: %d\n", errtype, detail);
#endif
	if (errtype == ErrorTypes_TimeOut) {
		if (_listener) { _listener->onError(this, Error_ReqTimedout, detail); }
        _client->testConnection(3000);
    }
}

void RPCClient::onClientEvent(SessionClient *client, int event, void *args)
{
    switch (event) {
        case SessionClient::kEVTConnRepaired:
            _connBroken = false;
            if (_rpc) {
                _rpc->restartUnfinishedRequests();
            }
            // NO break here
        case SessionClient::kEVTConnected:
            if (_listener) {
                _listener->onConnected(this, event==SessionClient::kEVTConnected);
            }
            break;
        case SessionClient::kEVTTestCompleted: {
            SessionClient::ArgTestResult* testargs = (SessionClient::ArgTestResult*)args;
            if (testargs->error==0) {
                if (_rpc) {
                    _rpc->restartUnfinishedRequests();
                }
            } else {
                int result = _client->reconnect();
                if (result==0 || result==EINPROGRESS || result==EALREADY) {
                    // reconnecting
                    // _rpc->restartUnfinishedRequests();
                    break;
                } else if(result==ENOTCONN) {
                    // connection closed
                    break;
                }
                if (!_connBroken) {
                    _connBroken = true;
					if (_listener) { _listener->onError(this, Error_ConnectBroken, testargs->error); }
                }
            }
            break;
        }
        case SessionClient::kEVTException: {
            int errtype = -1, detail = 0;
            SessionClient::ArgException* eargs = (SessionClient::ArgException*)args;
            if (eargs) {
                errtype = eargs->errtype; detail = eargs->detail;
            }
#if DEBUG_PIGEON
            NLOG(NL_INFO, "[client exception] %d: %d\n", errtype, detail);
#endif
            onClientException(client, errtype, detail);
            break;
        }
		case SessionClient::kEVTRequestIdError:
		{
			if (_rpc) {
				if (_rpc->isHaveMsgMiss())
				{
					if (_listener) { _listener->onError(this, Error_FatalError, ErrorTypes_RequestIdError); }
				}
				else
				{
					_rpc->restartUnfinishedRequests();
				}
			}
			break;
		}
		case SessionClient::kEVTComprType:
		{
			int compr;
			SessionClient::ArgException* eargs = (SessionClient::ArgException*)args;
			if (eargs) {
				compr = eargs->detail;
				_rpc->setComprType((RPCTransport::CompressionType)compr);
			}
			break;
		}
        default:
            break;
    }
}

void RPCClient::onClientException(SessionClient* client, int errtype, int detail)
{
    switch (errtype) {
		case ErrorTypes_ConnectBroken: {
            if (!_connBroken) {
                _connBroken = true;
				if (_listener) { _listener->onError(this, Error_ConnectBroken, detail); }
            }
            break;
        }
		case ErrorTypes_Inactive:
        {
			//if (_listener) { _listener->onError(this, Error_Inactive, detail); }
			if (_listener) { _listener->onError(this, Error_ConnectBroken, detail); }
            break;
        }
		case ErrorTypes_ConnectFailed:
        {
			if (_listener) { _listener->onError(this, Error_ConnectFailed, detail); }
            break;
        }
		case ErrorTypes_ServerClosed:
        {
            if (_listener) { _listener->onError(this, Error_ServerClosed, detail); }
//            if (_listener) { _listener->onError(this, Error_FatalError, detail); }
            break;
        }
        case ErrorTypes_ReadError:
        case ErrorTypes_WriteError:
        {
            if (detail==ETIMEDOUT || detail==EHOSTUNREACH || detail==ENETUNREACH) {
                if (_client) {
                    int result = _client->reconnect();
                    if (result==0 || result==EINPROGRESS) {
                        break;
                    }
                }
            }
            else if (detail==ECONNRESET) {
                if (_listener) {
//                    _listener->onError(this, Error_ConnectBroken, errtype);
                    _listener->onError(this, Error_ServerClosed, errtype);
                }
                break;
            }

            if (!_connBroken) {
                if (_client==NULL || _client->isBroken()) {
                    _connBroken = true;
					if (_listener) { _listener->onError(this, Error_ConnectBroken, detail); }
                }
            }
            break;
        }
        case ErrorTypes_EstablishError:
        {
            if (_listener) {
				_listener->onError(this, Error_ConnectFailed, errtype);
				//_listener->onError(this, Error_ConnectReset, errtype);
            }
            break;
        }
        case ErrorTypes_SessionError:
        {
            if (_listener) {
                _listener->onError(this, Error_SessionError, errtype);
                //_listener->onError(this, Error_ConnectReset, errtype);
            }
            break;
        }
        default: {
            if (_client && _client->isBroken()) {
                _connBroken = true;
            }
            if (_listener) {
				_listener->onError(this, Error_FatalError, errtype);
            }
            break;
        }
    }
}
std::string RPCClient::getCryptoConfig()
{
    if(_rpc)
    {
        return _rpc->getCryptoConfig();
    }
    return "";
}
int RPCClient::getCryptoType()
{
    if(_rpc)
    {
        return _rpc->getCryptoType();
    }
    return -1;
}
