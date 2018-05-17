#ifndef __pigeon_netcontroller_h
#define __pigeon_netcontroller_h

#include "netbasic.h"
#include "SessionSelector.h"
#include "SessionActor.h"
#include "RPCTransport.h"
#include "NetActivity.h"

class NetThread
{
public:
    NetThread();
    virtual ~NetThread();

    void setLoopInterval(int milliseconds);

    void start();
    void stop();
    void join();

    virtual void main();
    virtual void onInit();
    virtual void onExit();
    virtual void doTasks();
    virtual void update();

    void addRPCTerminal(RPCTransport* rpc);
    void removeRPCTerminal(RPCTransport* rpc);
    void addSession(SessionActor* session);
    void removeSession(SessionActor* session);

    typedef void TaskFunction(NetThread*, int);
    void runTask(const std::function<TaskFunction>& task);
    bool isInWorkthread() const;
    
    class AutoObject
    {
    public:
        virtual ~AutoObject() {}
        virtual bool tryCleaning(NetThread* thread, bool forced) = 0;
    };
    void manageObject(AutoObject* object);

protected:
    void checkManagedObjects(bool forceCleanup);

protected:
    bool _keepRunning;
    int _loopInterval;

    std::thread* _workThread;

    typedef std::list< std::function<TaskFunction> > TaskQueue;
    TaskQueue _taskQueue;
    std::mutex _taskMutex;

    typedef std::set<AutoObject*> ManagedObjectPool;
    ManagedObjectPool _managedObjects;
    std::mutex _autoObjectsMutex;

    // _selector is accessed ONLY in work thread
    SessionSelector _selector;
    // std::mutex _selectorMutex;

    // _terminals is accessed ONLY in work thread
    typedef std::list<RPCTransport*> RPCTerminals;
    RPCTerminals _terminals;
    // std::mutex   _terminalsMutex;
};

class NetController
{
public:
    // static NetController* getInstance();
    NetController();
    ~NetController();
    
    RPCClient* createClient();
    bool destroyActivity(RPCActivity* activity);
protected:
    NetThread* allocateNetThread();
    // typedef std::list<NetThread*> NetThreadPool;
    // NetThreadPool _threadPool;
    NetThread* _clientThread;

    typedef std::list<RPCActivity*> RPCActivityList;
    RPCActivityList _managedActivities;
};

#endif /* __pigeon_netcontroller_h */
