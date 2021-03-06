#ifndef __pigeon_rpctransport_h
#define __pigeon_rpctransport_h

#include "SessionSelector.h"
#include "SessionActor.h"
#include "Serializer.h"
#include "Deserializer.h"
#include "TimeCorrector.h"
#include "netbasic.h"


class RPCTransport;

class RPCListener
{
public:
    virtual ~RPCListener() {}
    virtual void onBinded(RPCTransport* rpc) {}
    virtual void onUnbinded(RPCTransport* rpc) {}

    virtual void onError(RPCTransport* rpc, int errtype, int detail) = 0;
    virtual void onRequested(RPCTransport* rpc, int request, int method, const std::string& params) = 0;
    virtual void onNotified(RPCTransport* rpc, int event, const std::string& data,float timestamp) = 0;
};

enum RPCErrors {
    eNetNoError         = 0,
    eNetInvalidOp       = 1,
    eNetUncomprError    = 2,
    eNetDecodeError     = 3,
    eNetUnkownError     = -1,
};

class RPCTransport
{
public:
    enum CompressionType
    {
        kNonCompressed = 0,
        kComprSnappy = 1,
    };

//     enum ErrorTypes {
//         kReqTimedout = 1,
//     };

    RPCTransport();
    ~RPCTransport();

    void attachSession(SessionActor* session) {
        _session = session;
    }
    SessionActor* getSession() {
        return _session;
    }

    void setListener(RPCListener* listener);
    RPCListener* getListener();

    // RTO = MAX(LBOUND, MIN(UBOUND, Estimated_RTO))
    void setReqRTOBounds(int lbound, int ubound) {
        _reqRTOLBound = lbound; _reqRTOUBound = ubound;
    }
    void setReqTimeout(int milliseconds) {
        _reqTimeout = milliseconds;
    }
    int getReqTimeout() const {
        return _reqTimeout;
    }
    void resetStatistics();
    // 获取最大请求延迟时间(毫秒)。
    // 请求延迟时间 := 收到响应与发出请求(调用request())的间隔时间 - 对方处理请求耗时
    long getMaxReqDelay() const {
        return _maxReqDelay;
    }
    // 获取平均请求延迟时间(毫秒)
    long getAvgReqDelay() const {
        return _avgReqDelay;
    }
    // 获取对方机器时间，理论误差为[elb, eub](毫秒)
    struct timeval remoteTimeval(long* elb=NULL, long* eub=NULL) const;
    // 获取对方机器时间戳(秒)，理论误差为[elb, eub](秒)
    long remoteTimestamp(long* elb=NULL, long* eub=NULL) const;
    // 获取对方机器时间戳(毫秒)，理论误差为[elb, eub](毫秒)
    int64_t remoteTimeMillis(long* elb=NULL, long* eub=NULL) const;
    // 获取`对方机器时间速度/本地机器时间速度`的值，比值的理论范围是[lb, ub]
    double remoteTimescale(double* lb=NULL, double* ub=NULL) const;

    /**
     @desc 清除所有缓存内容
    */
    void cleanup();
    /**
     @desc 发送通知
     @return 成功则返回0，失败则返回错误码(<0)
    */
    int notify(int event, /*int compr,*/ const void* data, size_t len);

    typedef std::function<void (int req, int err, const std::string& resp,float timestamp)> RequestCallback;
    /**
     @desc 发起新请求
     @return 成功则返回请求ID(>0)，失败则返回错误码(<0)
    */
    int request(int method,
                //int compr,
                const void* params,
                size_t len,
                RequestCallback callback,
                uint16_t extraRTO = 0);
    /**
     @desc 重发请求
     @return 成功则返回请求ID(>0)，失败则返回错误码(<0)
    */
    int request(int request);
    /**
     @desc 重发所有发送缓冲区中的未完成的请求
     */
    void restartUnfinishedRequests();
    /**
     @desc 响应特定请求
     @return 成功则返回0，失败则返回错误码(<0)
    */
    int respond(int request, 
		//int compr, 
		int err, 
		const void* data, 
		size_t len);
    
	/**
	@desc 响应KPush请求
	@return 只返回pushid即可
	*/
	int pushrespond(int pushId);

    /**
        计算平均延时
     */
    void calculateAvgDelay(long delay);
    void flush();
    void receive();

	/************************************************************************/
	/* 设置压缩方式                                                                     */
	/************************************************************************/
	void setComprType(CompressionType type) { _compr = type; }

	/**
	* 判断消息是否丢失
	*
	**/
	bool isHaveMsgMiss();
    
    /*
     * 使用服务器时间，把本地的采样，时间缩放等系数初始化
     */
    void timeCorrectorReset();
protected:
    struct Request {
        int id;  // if id==0, it means it's a notification, not a request.
        int respid;
        int method;
        //int8_t compr;
        std::string data;
    };

    struct Response
    {
        int id;
        struct timeval sendtime;
        int32_t elapsed;
        int16_t errcode;
        //int8_t compr;
        std::string data;
    };

    struct ReqSendInfo {
        int8_t state;
        int8_t sendcnt;
        uint16_t extraRTO;
        struct timeval inittime, sendtime0, sendtime;
        RequestCallback callback;
        struct Request reqval;
        struct Response* resp;
    };

    struct ReqRespInfo {
        int8_t state;
        struct timeval recvtime;
        struct Request reqval;
        struct Response* resp;
    };

    /**
        state transfer graph of ReqSendInfo:
        stPrepared -> stSending [ -> stReceived ] -> stResponded -> (complete)

        state transfer graph of ReqRespInfo:
        stWaiting -> stResponding [ -> stPrepared ] -> stSending [ -> stReceived ] -> (complete)
    */
    enum ElemStates {
        stWaiting   = 0,
        stPrepared  = 1,
        stSending   = 2,
        stReceived  = 3,
        stResponding = 4,
        stResponded = 5,

        stTimeout   = -1,
    };

    enum MsgTypes {
        kRequest  = 1,	//请求返回对应，收到返回才不重发
        kResponse = 2,
        kNotification = 3,
		KPush = 4,	//与KPushResponse对应，服务器收到返回后不再重发（客户端不发送KPush，收到后即返回服务器KPushResponse）
		KPushSucc = 5,
        kLackReq  = 6,
        kLackResp = 7,
    };

    enum CmdTypes {
        cmdRequest = 1,
        cmdRespond = 2,
        cmdNotify = 3,
		cmdPushSucc = 4,
        cmdRerequest = 11,
    };
    typedef ReqSendInfo RequestArgs;
    typedef struct { int id; } RerequestArgs;
    typedef Request     NotifyArgs;
    typedef Response    RespondArgs;

    struct cmd_t {
        int cmd;
        void* args;
        void (*destroy)(void*);
    };
    typedef std::list<cmd_t> CommandQueue;

protected:
    int compress(int compr, const void* src, size_t srclen, std::string& output);
    int uncompress(int compr, const void* src, size_t srclen, std::string& output);

    void processMessage(int8_t msgtype, Deserializer& reader, const struct timeval& now);
    void acceptRequest(ReqRespInfo* req);
	void acceptPush(ReqRespInfo* req);
    int processRequestsInCache();
	int processPush(ReqRespInfo* req);
    void acceptResponse(Response* resp, const struct timeval& now);

    void writeRequest(Serializer& writer, const Request& reqval);
    bool readRequest(Deserializer& reader, Request& reqval, int* error);
	bool readPush(Deserializer& reader, Request& reqval, int* error);
    void writeNotification(Serializer& writer, const Request& reqval);
    bool readNotification(Deserializer& reader, Request& reqval, int* error);
    void writeResponse(Serializer& writer, const Response& respval);
    bool readResponse(Deserializer& reader, Response& respval, int* error);
	void writePushRespond(Serializer& writer, const Request& reqval);

protected:
    SessionActor* _session;
    RPCListener* _listener;

    CommandQueue _cmdQueue;
    std::mutex _cmdMutex;

	CompressionType _compr;
	
	std::atomic_bool _restartUnfinishedRequests;
    bool _responseChanged;

    int _requestIdSeed;

    int _sendBaseId, _sendBufferRange;
    std::vector<ReqSendInfo*> _reqSendBuffer;
    std::set<ReqSendInfo*> _ongoingRequests;
    std::mutex _requestsMutex;

    int _recvBaseId, _recvBufferRange;
	int _recvPushId;
    std::vector<ReqRespInfo*> _reqRecvBuffer;

    // 在一次receive()中收到的最大请求号码
    int _maxTmpRecvRequestId;
    int _maxTmpRecvResponseId;
    int _maxProcessedRequestId;

    Serializer _serializer;

    int _reqTimeout;
    int _reqSRTT, _reqDevRTT;
    int _reqERTO;
    int _reqRTOLBound, _reqRTOUBound;
    long _maxReqDelay;
    long _avgReqDelay;

    TimeCorrector _timeCorrector;
public:
    std::string getCryptoConfig(){ return _session->getCryptoConfig();};
    int getCryptoType(){ return _session->getCryptoType();};
};

#endif
