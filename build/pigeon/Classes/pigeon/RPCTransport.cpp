#include "RPCTransport.h"
#include "define.h"
#include <snappy.h>
#include <list>
#include <algorithm>
using std::list;

#define MAX_REQ_HEADER_BYTES 48  // 为request数据头预留48个字节
#define MAX_RESP_HEADER_BYTES 48 // 为response数据头预留48个字节
#define DEFAULT_BUFFER_RANGE 8
#define REQ_CACHE_CAPACITY 32

#define REQDELAY_AVG_WEIGHT 0.7
#define TIMEDIFF_AVG_WEIGHT 0.7

// SRTT = SRTT + α(RTT–SRTT)
// DevRTT = (1-β)*DevRTT + β*(|RTT-SRTT|)
// Estimated RTO = µ*SRTT + ∂*DevRTT
#define SRTT_ALPHA  0.7  // α
#define DevRTT_BETA 0.7  // β
#define ERTO_MU     1.5   // µ
#define ERTO_DEE    4     // ∂
#define DEFAULT_RTO_LBOUND 2000
#define DEFAULT_RTO_UBOUND 10000

#define SAFE_DEL_REQINFO(p) \
    if (p) { if (p->resp) { delete p->resp; } delete p; }

int64_t timeval2timestampx(const struct timeval& tv)
{
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

struct timeval timestampx2timeval(int64_t tt)
{
    struct timeval tv = {
        (time_t)(tt/1000),
        (int)(tt%1000)*1000,
    };
    return tv;
}

template<typename TValue>
TValue smooth(TValue value0, TValue value, float factor) {
    if (value0<=0) { return value; }
    return value0 + (value-value0)*factor;
}

RPCTransport::RPCTransport()
: _timeCorrector(5)
{
    _session = NULL;
    _listener = NULL;

	_compr = kComprSnappy;

    _reqTimeout = 20*1000;
    
	_restartUnfinishedRequests = false;
    _responseChanged = false;

    _requestIdSeed = 0;

    _sendBaseId = 0;
    _sendBufferRange = DEFAULT_BUFFER_RANGE;

    _recvBaseId = 0;
	_recvPushId = 0;
    _recvBufferRange = DEFAULT_BUFFER_RANGE;

    _maxTmpRecvRequestId = 0;
    _maxTmpRecvResponseId = 0;
    _maxProcessedRequestId = 0;

    _reqRTOLBound = DEFAULT_RTO_LBOUND;
    _reqRTOUBound = DEFAULT_RTO_UBOUND;
    resetStatistics();
}

RPCTransport::~RPCTransport()
{
    setListener(NULL);
    cleanup();
}

int RPCTransport::compress(int compr, const void* src, size_t srclen, std::string& output)
{
    output.clear();
    if (src==NULL || srclen==0) { return 0; }
    switch (compr) {
    case kNonCompressed:
        output.assign((const char*)src, srclen);
        return 0;
    case kComprSnappy:
        snappy::Compress((const char*)src, srclen, &output);
        return 0;
    // default:
    //     return -1;
    }
    return -1;
}

int RPCTransport::uncompress(int compr, const void* src, size_t srclen, std::string& output)
{
    output.clear();
    if (src==NULL || srclen==0) { return 0; }
    switch (compr) {
    case kNonCompressed:
        output.assign((const char*)src, srclen);
        return 0;
    case kComprSnappy:
        if (!snappy::Uncompress((const char*)src, srclen, &output)) {
            return -2;
        }
        return 0;
    // default:
    //     return -1;
    }
    return -1;
}

void RPCTransport::setListener(RPCListener* listener)
{
    if (_listener==listener) {
        return;
    }
    if (_listener) {
        _listener->onUnbinded(this);
    }
    _listener = listener;
    if (_listener) {
        _listener->onBinded(this);
    }
}

RPCListener* RPCTransport::getListener()
{
    return _listener;
}

void RPCTransport::resetStatistics()
{
    _reqSRTT = 0;
    _reqDevRTT = 0;
    _reqERTO = 0;
    _maxReqDelay = 0;
    _avgReqDelay = 0;
}

struct timeval RPCTransport::remoteTimeval(long* elb, long* eub) const
{
    return timestampx2timeval(remoteTimeMillis(elb, eub));
}
void RPCTransport::timeCorrectorReset()
{
    _timeCorrector.reset();
}

long RPCTransport::remoteTimestamp(long* elb, long* eub) const
{
    long timestamp = (long)(remoteTimeMillis(elb, eub)/1000);
    if (elb) { *elb = (*elb + 999)/1000; }
    if (eub) { *eub = (*eub)/1000; }
    return timestamp;
}

int64_t RPCTransport::remoteTimeMillis(long* elb, long* eub) const
{
    return _timeCorrector.currentTimeMillis(elb, eub);
}

double RPCTransport::remoteTimescale(double* lb, double* ub) const
{
    return _timeCorrector.timeScale(lb, ub);
}

void RPCTransport::cleanup()
{
    for (auto it = _reqSendBuffer.begin(); it!=_reqSendBuffer.end(); ++it) {
        ReqSendInfo* req = *it;
        SAFE_DEL_REQINFO(req);
    }
    _reqSendBuffer.clear();

    for (auto it = _reqRecvBuffer.begin(); it!=_reqRecvBuffer.end(); ++it) {
        ReqRespInfo* req = *it;
        SAFE_DEL_REQINFO(req);
    }
    _reqRecvBuffer.clear();

    for (auto it = _cmdQueue.begin(); it!=_cmdQueue.end(); ++it) {
        cmd_t cmd = *it;
        if (cmd.args && cmd.destroy) {
            cmd.destroy(cmd.args);
        }
    }
    _cmdQueue.clear();

	_ongoingRequests.clear();
}

int RPCTransport::notify(int event, /*int compr,*/ const void* data, size_t len)
{
    if (_session==NULL) {
        return -1;
    }

    Request* notify = new Request();
    notify->id = 0;
    notify->respid = 0;
    notify->method = event;
    //notify->compr = compr;
    if (0!=compress(_compr, data, len, notify->data)) {
        compress(kNonCompressed, data, len, notify->data);
        //notify->compr = kNonCompressed;
    }

    size_t headerBytes = MAX_REQ_HEADER_BYTES;
    size_t dataBytes = notify->data.length();
    if (_session->getSendLimits() < headerBytes+dataBytes) {
        // too much data
		delete notify;
        return -2;
    }

    class Destroyer {
    public:
        static void run(void* args) {
            delete (Request*)args;
        }
    };

    _cmdMutex.lock();
    cmd_t cmd = {
        cmdNotify, notify, Destroyer::run,
    };
    _cmdQueue.push_back(cmd);
    _cmdMutex.unlock();
    
    return 0;
}

int RPCTransport::request(int method,
                          //int compr,
                          const void* params,
                          size_t len,
                          RequestCallback callback,
                          uint16_t extraRTO)
{
    if (_session==NULL) {
        return -1;
    }

    ReqSendInfo* req = new ReqSendInfo();
    req->state = stPrepared;
    req->sendcnt = 0;
    req->extraRTO = extraRTO;
    req->callback = callback;
    req->resp = NULL;
    req->reqval.id = 0;
    req->reqval.respid = 0;
    req->reqval.method = method;
    //req->reqval.compr = compr;
    gettimeofday(&req->inittime, NULL);
    req->sendtime0 = req->inittime;
    if (0!=compress(_compr, params, len, req->reqval.data)) {
        compress(kNonCompressed, params, len, req->reqval.data);
        //req->reqval.compr = kNonCompressed;
    }

    size_t headerBytes = MAX_REQ_HEADER_BYTES;
    size_t dataBytes = req->reqval.data.length();
    if (_session->getSendLimits() < headerBytes+dataBytes) {
        // too much data
        SAFE_DEL_REQINFO(req);
        return -2;
    }

    class Destroyer {
    public:
        static void run(void* args) {
            ReqSendInfo* req = (ReqSendInfo*)args;
            SAFE_DEL_REQINFO(req);
        }
    };
    
    _requestsMutex.lock();
    _ongoingRequests.insert(req);
    _requestsMutex.unlock();

    _cmdMutex.lock();
    int reqid = ++_requestIdSeed;
    req->reqval.id = reqid;
    cmd_t cmd = {
        cmdRequest, req, Destroyer::run,
    };
    _cmdQueue.push_back(cmd);
    _cmdMutex.unlock();

    return reqid;
}

int RPCTransport::request(int request)
{
    if (request<=_sendBaseId || request > _requestIdSeed) {
        // invalid request id
        return -1;
    }

    class Destroyer {
    public:
        static void run(void* args) {
            delete (RerequestArgs*)args;
        }
    };

    RerequestArgs* args = new RerequestArgs();
    args->id = request;
     _cmdMutex.lock();
    cmd_t cmd = {
        cmdRerequest, args, Destroyer::run,
    };
    _cmdQueue.push_back(cmd);
    _cmdMutex.unlock();

    return request;
}

void RPCTransport::restartUnfinishedRequests()
{
	_restartUnfinishedRequests = true;
}

int RPCTransport::respond(int request, 
	//int compr, 
	int err, 
	const void* data, 
	size_t len)
{
    if (request<=_recvBaseId || request > _recvBaseId+_recvBufferRange) {
        // invalid request id
        return -1;
    }

    Response* resp = new Response();
    resp->id = request;
    gettimeofday(&resp->sendtime, NULL);
    resp->elapsed = 0;
    resp->errcode = err;
    if (data!=NULL && len>0) {
        if (0!=compress(_compr, data, len, resp->data)) {
            // resp->data.clear();
            compress(kNonCompressed, data, len, resp->data);
            //resp->compr = kNonCompressed;
        }
// 		else {
//             resp->compr = compr;
//         }
    }

    class Destroyer {
    public:
        static void run(void* args) {
            delete (Response*)args;
        }
    };

    _cmdMutex.lock();
    cmd_t cmd = {
        cmdRespond, resp, Destroyer::run,
    };
    _cmdQueue.push_back(cmd);
    _cmdMutex.unlock();

    return 0;
}

int RPCTransport::pushrespond(int pushId)
{
	Response* resp = new Response();
	resp->id = pushId;
	gettimeofday(&resp->sendtime, NULL);
	resp->elapsed = 0;
	resp->errcode = 0;

	class Destroyer {
	public:
		static void run(void* args) {
			delete (Response*)args;
		}
	};

	_cmdMutex.lock();
	cmd_t cmd = {
		cmdPushSucc, resp, Destroyer::run,
	};
	_cmdQueue.push_back(cmd);
	_cmdMutex.unlock();

	return 0;
}

void RPCTransport::flush()
{
	if (_session == NULL || !_session->ready()) {
		return;
	}

	struct timeval now;
	gettimeofday(&now, NULL);

	bool forceResend = _restartUnfinishedRequests;
	_restartUnfinishedRequests = false;
    // resend buffered requests
    for (auto it = _reqSendBuffer.begin(); it!=_reqSendBuffer.end(); ++it) {
        ReqSendInfo* req = *it;
        if (req==NULL || req->state==stResponded) { continue; }
        if (forceResend && req->state==stTimeout && req->resp!=NULL) {
            req->state = stResponded;
            _responseChanged = true;
        } else if (req->state==stPrepared || forceResend) {
            req->sendcnt++;
            if (req->state==stTimeout) {
                req->sendtime0 = now;
            }
			req->state = stSending;
            req->sendtime = now;
            req->reqval.respid = _sendBaseId;
            _serializer.truncate(0);
            writeRequest(_serializer, req->reqval);
            _session->send(_serializer.buffer(), _serializer.length());
            _serializer.truncate(0);
        }
    }

    // resend prepared responses
    for (auto it = _reqRecvBuffer.begin(); it!=_reqRecvBuffer.end(); ++it) {
        ReqRespInfo* req = *it;
        if (req && req->state==stPrepared) {
            Response* resp = req->resp;
            assert(resp!=NULL);
            req->state = stSending;
            int elapsed = (int)difftimeval(&now, &req->recvtime);
            resp->elapsed = elapsed;
            resp->sendtime = now;
            _serializer.truncate(0);
            writeResponse(_serializer, *resp);
            _session->send(_serializer.buffer(), _serializer.length());
            _serializer.truncate(0);
        }
    }

    // execute commands in comand queue
    bool worked = true, isFirst = true;
    CommandQueue::iterator it;
    while (worked) {
        worked = false;
        _serializer.truncate(0);
        // gettimeofday(&now, NULL);

        _cmdMutex.lock();
        if (isFirst) {
            isFirst = false;
            it = _cmdQueue.begin();
        }
        while (!worked && it!=_cmdQueue.end()) {
            cmd_t cmd = *it;
            if (cmd.cmd==cmdRequest) {
                ReqSendInfo* req = (ReqSendInfo*)cmd.args;
                int reqId = req->reqval.id;
                int idx = reqId - _sendBaseId - 1;
                if (idx>=0) {
                    // if (idx==_sendBufferRange) {
                    //     // try expanding send buffer range
                    // }
                    if (idx>=_sendBufferRange) {
                        ++it;
                        continue;
                    }
                    if (_reqSendBuffer.size()<=idx) {
                        _reqSendBuffer.resize(idx+1, NULL);
                    }
                    assert(_reqSendBuffer[idx]==NULL);
                    _reqSendBuffer[idx] = req;
                    if (req->state==stPrepared) {
                        req->sendcnt = 1;
                        req->state = stSending;
                        // req->sendtime0 = now;
                        req->sendtime = now;
                        req->reqval.respid = _sendBaseId;
                        writeRequest(_serializer, req->reqval);
                    }
                    req = NULL;
                    worked = true;
                }
                it = _cmdQueue.erase(it);
                if (req && cmd.destroy) { cmd.destroy(req); }
            }
            else if (cmd.cmd==cmdRerequest) {
                RerequestArgs* args = (RerequestArgs*)cmd.args;
                int reqId = args->id;
                int idx = reqId - _sendBaseId - 1;
                ReqSendInfo* req = NULL;
                if (idx>=0 && idx<_reqSendBuffer.size()) {
                    // resend request in _reqSendBuffer
                    req = _reqSendBuffer[idx];
                    if (req && req->state!=stResponded) {
                        if (req->resp!=NULL && req->state==stTimeout) {
                            req->state = stResponded;
                            _responseChanged = true;
                        } else {
                            req->sendcnt++;
                            req->state = stSending;
                            req->sendtime0 = now;
                            req->sendtime = now;
                            req->reqval.respid = _sendBaseId;
                            writeRequest(_serializer, req->reqval);
                        }
                    }
                }
                if (req==NULL) {
                    _requestsMutex.lock();
                    auto findIt = std::find_if(_ongoingRequests.begin(),
                                                _ongoingRequests.end(),
                                                [reqId](ReqSendInfo* req)->bool {
                                                    return req->reqval.id==reqId;
                                                });
                    if (findIt!=_ongoingRequests.end()) {
                        req = *findIt;
                    }
                    _requestsMutex.unlock();
                    if (req && req->state!=stResponded) {
                        assert(req->state==stPrepared || req->state==stTimeout);
                        req->state = stPrepared;
                        req->sendtime0 = now;
                        // req->sendtime = now;
                    }
                }
                worked = true;
                it = _cmdQueue.erase(it);
                if (args && cmd.destroy) { cmd.destroy(args); }
            }
            else if (cmd.cmd==cmdRespond) {
                Response* resp = (Response*)cmd.args;
                int reqId = resp->id;
                int idx = reqId - _recvBaseId - 1;
                // send response
                if (idx>=0 && idx<_reqRecvBuffer.size()) {
                    ReqRespInfo* req = _reqRecvBuffer[idx];
                    if (req!=NULL && req->resp==NULL) {
                        // assert(req->state==stResponding);
                        req->resp = resp;
                        req->state = stSending;
                        int elapsed = (int)difftimeval(&now, &req->recvtime);
                        resp->elapsed = elapsed;
                        resp->sendtime = now;
                        writeResponse(_serializer, *resp);
                        // prevent `resp` being deleted
                        resp = NULL;
                    }
                }
                worked = true;
                it = _cmdQueue.erase(it);
                if (resp && cmd.destroy) { cmd.destroy(resp); }
            }
			else if (cmd.cmd == cmdNotify) {
				Request* notify = (Request*)cmd.args;
				writeNotification(_serializer, *notify);
				worked = true;
				it = _cmdQueue.erase(it);
				if (notify && cmd.destroy) { cmd.destroy(notify); }
			}
			else if (cmd.cmd == cmdPushSucc) {
				Request* pushResp = (Request*)cmd.args;
				writePushRespond(_serializer, *pushResp);
				worked = true;
				it = _cmdQueue.erase(it);
				if (pushResp && cmd.destroy) { cmd.destroy(pushResp); }
            } else {
                it = _cmdQueue.erase(it);
                if (cmd.args && cmd.destroy) { cmd.destroy(cmd.args); }
            }
        }
        _cmdMutex.unlock();

        if (_serializer.length()>0) {
            _session->send(_serializer.buffer(), _serializer.length());
            _serializer.truncate(0);
        }
    }
}

void RPCTransport::receive()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    
    calculateAvgDelay(_session->getHeartBeatDelay());
    
    if (_session!=NULL && _session->available()>0) {
        _maxTmpRecvRequestId = 0;
        _maxTmpRecvResponseId = 0;

        while (_session->available() > 0) {
            size_t bytes = 0;
            void* buffer = _session->receive(&bytes);
            if (buffer==NULL) {
                break;
            }
            Deserializer reader(buffer, bytes);
            int8_t msgtype = 0;
            if (Deserializer::errNoError==reader.readInt8(&msgtype)) {
                // gettimeofday(&now, NULL);
                processMessage(msgtype, reader, now);
            }
            _session->freeReceived(buffer);
        }

        if (_maxProcessedRequestId<_maxTmpRecvRequestId) {
            // some new requests are blocked or discarded
            // TODO send `kLackReq`
        }
        if (_sendBaseId<_maxTmpRecvResponseId) {
            // some new requests are blocked or discarded
            // TODO send `kLackResp`
        }
    }
    
    if (_responseChanged) {
        _responseChanged = false;
        int i = 0;
        for (; i < _reqSendBuffer.size(); ++i) {
            ReqSendInfo* req = _reqSendBuffer[i];
            if (req==NULL || req->state!=stResponded) {
                break;
            }
            assert(req->resp!=NULL);
            if (req->callback) {
                req->callback(req->reqval.id, req->resp->errcode, req->resp->data,pigeonTickCount());
            }
            
            _requestsMutex.lock();
            _ongoingRequests.erase(req);
            _requestsMutex.unlock();
            
            SAFE_DEL_REQINFO(req);
            // _reqSendBuffer[i] = NULL;
        }
        if (i>0) {
            _sendBaseId += i;
            _reqSendBuffer.erase(_reqSendBuffer.begin(),
                                 _reqSendBuffer.begin() + i);
        }
    }

    gettimeofday(&now, NULL);
    
    if (_reqTimeout>0) {
        // check requests timeout
        std::list<ReqSendInfo*> timeoutRequests;
        _requestsMutex.lock();
        for (auto it = _ongoingRequests.begin(); it!=_ongoingRequests.end(); ++it) {
            ReqSendInfo* req = *it;
            if (req && req->state!=stResponded && req->state!=stTimeout) {
                if (difftimeval(&now, &req->sendtime0)>=_reqTimeout) {
                    req->state = stTimeout;
                    timeoutRequests.push_back(req);
                }
            }
        }
        _requestsMutex.unlock();
        if (_listener!=NULL && timeoutRequests.size()>0) {
            for (auto it=timeoutRequests.begin(); it!=timeoutRequests.end(); ++it) {
                ReqSendInfo* req = *it;
				_listener->onError(this, ErrorTypes_TimeOut, req->reqval.id);
            }
        }
    }
    
    int rto = MAX(_reqRTOLBound, MIN(_reqRTOUBound, _reqERTO));
    if (rto>0) {
        int retries = 0;
        for (auto it = _reqSendBuffer.begin(); it!=_reqSendBuffer.end(); ++it) {
            ReqSendInfo* req = *it;
            if (req && req->state==stSending) {
                if (difftimeval(&now, &req->sendtime)>=rto+req->extraRTO) {
                    req->state = stPrepared; retries++;
#if DEBUG_PIGEON
                    NLOG(NL_INFO, "resend: %d (rto=%d+%d)\n", req->reqval.id, rto, req->extraRTO);
#endif
                }
            }
        }
        if (retries>0) {
			_reqERTO += retries * 1000;
        }
    }
}

void RPCTransport::processMessage(
    int8_t msgtype, Deserializer& reader, const struct timeval& now)
{
    switch (msgtype) {
        case kRequest: {
            ReqRespInfo* req = new ReqRespInfo();
            int error = 0;
            if (readRequest(reader, req->reqval, &error)) {
                req->recvtime = now;
                req->resp = NULL;
                req->state = stWaiting;
                acceptRequest(req);
            } else {
                if (error==eNetUnkownError) {
                    delete req;
                } else {
                    // req->reqval.data = "";
                    req->recvtime = now;
                    
                    Response* resp = new Response();
                    resp->id = req->reqval.id;
                    resp->sendtime = now;
                    resp->elapsed = 0;
                    resp->errcode = error;
                    //resp->compr = kNonCompressed;

                    req->resp = resp;
                    req->state = stPrepared;
                    acceptRequest(req);
                }
            }
            break;
        }
        case kResponse: {
            Response* resp = new Response();
            if (readResponse(reader, *resp, NULL)) {
                acceptResponse(resp, now);
            } else {
                delete resp;
            }
            break;
        }
        case kNotification: {
            Request notify;
            if (_listener!=NULL && readNotification(reader, notify, NULL)) {
                _listener->onNotified(this, notify.method, notify.data,pigeonTickCount());
            }
            break;
        }
		case  KPush: {
			ReqRespInfo* req = new ReqRespInfo();
			int error = 0;
			if (readPush(reader, req->reqval, &error)) {
				req->recvtime = now;
				req->resp = NULL;
				req->state = stWaiting;
				acceptPush(req);
			}
			else {
				if (error == eNetUnkownError) {
					delete req;
				}
			}
			break;
		}
        case kLackReq: {
            break;
        }
        case kLackResp: {
            break;
        }
        default: {
            break;
        }
    }
}

void RPCTransport::acceptRequest(ReqRespInfo* req)
{
    int respId = req->reqval.respid;
    if (respId > _recvBaseId) {
        int i = 0;
        for (; i < respId-_recvBaseId; ++i) {
            if (i>=_reqRecvBuffer.size()) {
                break;
            }
            ReqRespInfo* tmpreq = _reqRecvBuffer[i];
            if (tmpreq==NULL) {
                break;
            }
            SAFE_DEL_REQINFO(tmpreq);
            // _reqRecvBuffer[i] = NULL;
        }
        if (i>0) {
            _recvBaseId += i;
            _reqRecvBuffer.erase(_reqRecvBuffer.begin(),
                                _reqRecvBuffer.begin() + i);
        }
    }

    int reqId = req->reqval.id;
    if (reqId<=_recvBaseId) {
        SAFE_DEL_REQINFO(req);
    } else {
        if (reqId>_maxTmpRecvRequestId) {
            _maxTmpRecvRequestId = reqId;
        }
        int idx = reqId-_recvBaseId - 1;
        if (idx<_recvBufferRange) {
            if (_reqRecvBuffer.size()<=idx) {
                _reqRecvBuffer.resize(idx+1, NULL);
            }
            if (_reqRecvBuffer[idx]==NULL) {
                _reqRecvBuffer[idx] = req;
                processRequestsInCache();
            } else {
                // reduplicated request
                SAFE_DEL_REQINFO(req);
            }
        } else {
            SAFE_DEL_REQINFO(req);
        }
    }
}

void RPCTransport::acceptPush(ReqRespInfo* req)
{
	//给服务器返回PushResponse并处理消息
	int respId = req->reqval.respid;
	int reqId = req->reqval.id;
	pushrespond(reqId);
	//如果此消息已处理，直接删除
	if (reqId <= _recvPushId) {
		SAFE_DEL_REQINFO(req);
	}
	else {
		processPush(req);
		_recvPushId = reqId;
	}
}

int RPCTransport::processRequestsInCache()
{
    if (_listener==NULL) {
        return _maxProcessedRequestId;
    }

    // int i = _maxProcessedRequestId+1 - _recvBaseId - 1;
    int i = _maxProcessedRequestId-_recvBaseId;
    for (; i < _reqRecvBuffer.size(); ++i) {
        ReqRespInfo* req = _reqRecvBuffer[i];
        if (req==NULL) {
            break;
        }
        assert(req->state==stWaiting);
        req->state = stResponding;
        _maxProcessedRequestId++;
        _listener->onRequested(this, req->reqval.id, req->reqval.method, req->reqval.data);
    }

    return _maxProcessedRequestId;
}

int RPCTransport::processPush(ReqRespInfo* req)
{
	if (_listener != NULL) {
		_listener->onNotified(this, req->reqval.method, req->reqval.data,pigeonTickCount());
	}

	return req->reqval.id;
}

void RPCTransport::calculateAvgDelay(long reqDelay)
{
    if (reqDelay<= 0) { return; }
#if DEBUG_PIGEON
    fprintf(stdout, "reDelay = %ld\n",reqDelay);
#endif
    if (_maxReqDelay<reqDelay) { _maxReqDelay = reqDelay; }
    _avgReqDelay = smooth<long>(_avgReqDelay, reqDelay, REQDELAY_AVG_WEIGHT);
}

void RPCTransport::acceptResponse(Response* resp, const struct timeval& now)
{
    int respId = resp->id;
    if (respId>_maxTmpRecvResponseId) {
        _maxTmpRecvResponseId = respId;
    }

    int idx = respId - _sendBaseId - 1;
    if (idx<0 || idx>=_reqSendBuffer.size()) {
         delete resp;
         return;
     }
    ReqSendInfo* req = _reqSendBuffer[idx];
    if (req==NULL || req->resp!=NULL) {
        delete resp;
        return;
    }

    assert(req->reqval.id==respId);
    req->resp = resp;
    if (req->state!=stTimeout) {
        req->state = stResponded;
        _responseChanged = true;
    }

    long reqDelay = difftimeval(&now, &req->inittime) - resp->elapsed;
#if DEBUG_PIGEON
    NLOG(NL_WARNING, "___reqDelay = %ld\n",reqDelay);
#endif
    calculateAvgDelay(reqDelay);
    if (req->sendcnt==1) {
        _timeCorrector.addSample(req->sendtime, now, resp->sendtime, resp->elapsed);
        
        int rtt = (int)difftimeval(&now, &req->sendtime);
        if (rtt<0) { rtt = 0; }
        if (rtt<=60000) {
            _reqSRTT = smooth<int>(_reqSRTT, rtt, SRTT_ALPHA);
            int devRTT = rtt<_reqSRTT ? _reqSRTT-rtt : rtt-_reqSRTT;
            _reqDevRTT = smooth<int>(_reqDevRTT, devRTT, DevRTT_BETA);
            _reqERTO = ERTO_MU*_reqSRTT + ERTO_DEE*_reqDevRTT;
        }
    }
#if DEBUG_PIGEON
    NLOG(NL_INFO,
        "[RPCTransport] delay=%ld/%ld, srtt=%d, devrtt=%d, erto=%d\n",
        _avgReqDelay, _maxReqDelay, _reqSRTT, _reqDevRTT, _reqERTO);
    
    long elb, eub;
    int64_t nowmillis = now.tv_sec*1000 + now.tv_usec/1000;
    int64_t millis = _timeCorrector.timeMillis(nowmillis, &elb, &eub);
    double slb, sub;
    double scale = _timeCorrector.timeScale(&slb, &sub);
    NLOG(NL_INFO,
         "timediff=%lld (%ld, %ld), scale=%g(%g,%g)\n",
         millis-nowmillis, elb, eub, scale, slb, sub);
#endif
}

void RPCTransport::writeRequest(Serializer& writer, const Request& reqval)
{
#if defined(WIN32)
	NLOG(NL_INFO,
		"threadId = %d writeRequest method=%d\n",
		GetCurrentThreadId(), reqval.method);
#endif

    assert(reqval.id>0);
    writer.writeInt8(kRequest);
    //writer.writeInt8(reqval.compr);
    writer.writeInt32(reqval.id);
    writer.writeInt32(reqval.respid);
    writer.writeInt32(reqval.method);
    assert(reqval.data.length()<=0x7fffffff);
    writer.writeInt32(reqval.data.length());
    assert(writer.length()<=MAX_REQ_HEADER_BYTES);
    writer.writeBytes(reqval.data.c_str(), reqval.data.length());
}

bool RPCTransport::readRequest(Deserializer& reader, Request& reqval, int* error)
{
    int err = 0; if (error==NULL) { error = &err; };
    *error = eNetUnkownError;
    do {
        //if (Deserializer::errNoError!=reader.readInt8(&reqval.compr)) { break; }
        if (Deserializer::errNoError!=reader.readInt32(&reqval.id)) { break; }
        if (Deserializer::errNoError!=reader.readInt32(&reqval.respid)) { break; }
        if (Deserializer::errNoError!=reader.readInt32(&reqval.method)) { break; }
        int32_t datalen = 0;
		if (Deserializer::errNoError != reader.readInt32(&datalen)) { break; }
        if (reader.available()<datalen) { break; }
        if (0!=uncompress(_compr, reader.buffer()+reader.tell(), datalen, reqval.data)) {
            *error = eNetUncomprError;
            break;
        }
        *error = eNetNoError;
        return true;
    } while (false);
    return false;
}

bool RPCTransport::readPush(Deserializer& reader, Request& reqval, int* error)
{
	int err = 0; if (error == NULL) { error = &err; };
	*error = eNetUnkownError;
	do {
		//if (Deserializer::errNoError != reader.readInt8(&reqval.compr)) { break; }
		if (Deserializer::errNoError != reader.readInt32(&reqval.id)) { break; }
		//if (Deserializer::errNoError != reader.readInt32(&reqval.respid)) { break; }
		if (Deserializer::errNoError != reader.readInt32(&reqval.method)) { break; }
		int32_t datalen = 0;
		if (Deserializer::errNoError != reader.readInt32(&datalen)) { break; }
		if (reader.available() < datalen) { break; }
		if (0 != uncompress(_compr, reader.buffer() + reader.tell(), datalen, reqval.data)) {
			*error = eNetUncomprError;
			break;
		}
		*error = eNetNoError;
		return true;
	} while (false);
	return false;
}

void RPCTransport::writeNotification(Serializer& writer, const Request& reqval)
{
    assert(reqval.id==0);
    writer.writeInt8(kNotification);
    //writer.writeInt8(reqval.compr);
    writer.writeInt32(reqval.method);
    assert(reqval.data.length()<=0x7fffffff);
    writer.writeInt32(reqval.data.length());
    assert(writer.length()<=MAX_REQ_HEADER_BYTES);
    writer.writeBytes(reqval.data.c_str(), reqval.data.length());
}

bool RPCTransport::readNotification(Deserializer& reader, Request& reqval, int* error)
{
    int err = 0; if (error==NULL) { error = &err; };
    *error = eNetUnkownError;
    do {
        reqval.id = 0;
        reqval.respid = 0;
        //if (Deserializer::errNoError!=reader.readInt8(&reqval.compr)) { break; }
        if (Deserializer::errNoError!=reader.readInt32(&reqval.method)) { break; }
        int32_t datalen = 0;
		if (Deserializer::errNoError != reader.readInt32(&datalen)) { break; }
        if (reader.available()<datalen) { break; }
        if (0!=uncompress(_compr, reader.buffer()+reader.tell(), datalen, reqval.data)) {
            *error = eNetUncomprError;
            break;
        }
        *error = eNetNoError;
        return true;
    } while (false);
    return false;
}

void RPCTransport::writeResponse(Serializer& writer, const Response& respval)
{
    writer.writeInt8(kResponse);
    //writer.writeInt8(respval.compr);
    writer.writeInt32(respval.id);
    writer.writeInt64(timeval2timestampx(respval.sendtime));
    writer.writeInt32(respval.elapsed);
    writer.writeInt16(respval.errcode);
    assert(respval.data.length()<=0x7fffffff);
	writer.writeInt32(respval.data.length());
    assert(writer.length()<=MAX_RESP_HEADER_BYTES);
    writer.writeBytes(respval.data.c_str(), respval.data.length());
}

bool RPCTransport::readResponse(Deserializer& reader, Response& respval, int* error)
{
    int err = 0; if (error==NULL) { error = &err; };
    *error = eNetUnkownError;
    do {
        //if (Deserializer::errNoError!=reader.readInt8(&respval.compr)) { break; }
        if (Deserializer::errNoError!=reader.readInt32(&respval.id)) { break; }
        int64_t timestampx = 0;
        if (Deserializer::errNoError!=reader.readInt64(&timestampx)) { break; }
        respval.sendtime = timestampx2timeval(timestampx);
        if (Deserializer::errNoError!=reader.readInt32(&respval.elapsed)) { break; }
        if (Deserializer::errNoError!=reader.readInt16(&respval.errcode)) { break; }
        int32_t datalen = 0;
		if (Deserializer::errNoError != reader.readInt32(&datalen)) { break; }
        if (reader.available()<datalen) { break; }
        if (0!=uncompress(_compr, reader.buffer()+reader.tell(), datalen, respval.data)) {
            *error = eNetUncomprError;
            break;
        }
        *error = eNetNoError;
        return true;
    } while (false);
    return false;
}

void RPCTransport::writePushRespond(Serializer& writer, const Request& reqval)
{
	writer.writeInt8(KPushSucc);
	writer.writeInt32(reqval.id);
}

bool RPCTransport::isHaveMsgMiss()
{
	if (_requestIdSeed > _sendBaseId)
	{
		list<int32_t> temp;
		for (int i = _sendBaseId + 1; i <= _requestIdSeed; i++)
		{
			temp.push_back(i);
		}
		for (auto it = _reqSendBuffer.begin(); it != _reqSendBuffer.end(); ++it) {
			ReqSendInfo* req = *it;
			if (req == NULL) { continue; }
			if (find(temp.begin(), temp.end(), req->reqval.id) != temp.end())
			{
				temp.remove(req->reqval.id);
			}
		}
		if (temp.size() > 0)
		{
			return true;
		}
	}
	 
	return false;
}
