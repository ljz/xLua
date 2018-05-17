#ifndef __pigeon_netactivity_h
#define __pigeon_netactivity_h

#include "netbasic.h"
#include "SessionSelector.h"
#include "SessionActor.h"
#include "RPCTransport.h"

class NetController;
class NetThread;

class RPCActivity
{
public:
    RPCActivity(NetThread* workThread) {
        _workThread = workThread;
    }
    virtual ~RPCActivity() {}
protected:
    NetThread* _workThread;
};

class RPCClient: public RPCActivity
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void onBinded(RPCClient* client) {}
        virtual void onUnbinded(RPCClient* client) {}

        virtual void onError(RPCClient* client, int errcode, int detail) = 0;
        virtual void onConnected(RPCClient* client, bool firstConnect) {}
        virtual void onRequested(RPCClient* client, int request, int method, const std::string& params) = 0;
        virtual void onNotified(RPCClient* client, int event, const std::string& data,float timestamp) = 0;
    };

//     enum Errors {
//         kServerClosed   = 4000,
//         kConnectFailed  = 4001,
//         kConnectBroken  = 4002,
//         kConnectReset   = 4003,
//         kInactive       = 4004,
//         kReqTimedout    = 4100,
//         kFatalError     = 4999,
//     };

public:
    RPCClient(NetThread* workThread);
    virtual ~RPCClient();

    void setHeartbeatInterval(long millisecond);
    long getHeartbeatInterval() const { return _hbinterval; }
    void setHeartbeatTimeout(long millisecond);
    long getHeartbeatTimeout() const { return _hbtimeout; }
    void setloopInterval(int loopInterval);
    bool connect(const char* host, in_port_t port, long timeout);
    bool repair(bool force);
    void close();

    void setListener(Listener* listener);
    Listener* getListener();

    void setReqRTOBounds(int lbound, int ubound);
    void setReqTimeout(int milliseconds);
    int getReqTimeout() const;
    long getMaxReqDelay() const;
    long getAvgReqDelay() const;
    long remoteTimestamp(long* elb=NULL, long* eub=NULL) const;
    int64_t remoteTimeMillis(long* elb=NULL, long* eub=NULL) const;
    double remoteTimescale(double* lb=NULL, double* ub=NULL) const;
    
    typedef RPCTransport::RequestCallback RequestCallback;
    
    int notify(int event, /*int compr,*/ const void* data, size_t len);
    int request(int method,
                //int compr,
                const void* params,
                size_t len,
                RequestCallback callback,
                uint16_t extraRTO = 0);
    int request(int request);
    int respond(int request, /*int compr,*/ int err, const void* data, size_t len);
    
    void timeCorrectorReset();
    std::string getCryptoConfig();
    int getCryptoType();
protected:
    bool doRepair(bool force);
    void onRPCRequested(RPCTransport* rpc, int request, int method, const std::string& params);
    void onRPCNotified(RPCTransport* rpc, int event, const std::string& data,float timestamp);
    void onRPCError(RPCTransport* rpc, int errtype, int detail);
    void onClientEvent(SessionClient* client, int event, void* args);
    void onClientException(SessionClient* client, int errtype, int detail);

    class MyRPCListener: public RPCListener
    {
        RPCClient* _client;
    public:
        MyRPCListener(RPCClient* client) {
            _client = client;
        }
        virtual void onBinded(RPCTransport* rpc) override { }
        virtual void onUnbinded(RPCTransport* rpc) override { /*delete this;*/ }
        virtual void onError(RPCTransport* rpc, int errtype, int detail) override {
            _client->onRPCError(rpc, errtype, detail);
        }
        virtual void onRequested(RPCTransport* rpc, int request, int method, const std::string& params) override {
            _client->onRPCRequested(rpc, request, method, params);
        }
        virtual void onNotified(RPCTransport* rpc, int event, const std::string& data,float timestamp) override {
            _client->onRPCNotified(rpc, event, data,timestamp);
        }
    };
protected:
    long _hbinterval, _hbtimeout;
    int _loopInterval;
    bool _connBroken;
    RPCTransport* _rpc;
    SessionClient* _client;
    Listener* _listener;
    MyRPCListener _rpcListener;
};

#endif /* __pigeon_netactivity_h */
