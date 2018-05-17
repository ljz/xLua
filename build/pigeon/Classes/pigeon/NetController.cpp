#include "NetController.h"

//#define LOCK_SELECTOR   _selectorMutex.lock();
//#define UNLOCK_SELECTOR _selectorMutex.lock();
#define LOCK_SELECTOR
#define UNLOCK_SELECTOR

//#define LOCK_TERMINALS   _terminalsMutex.lock();
//#define UNLOCK_TERMINALS _terminalsMutex.lock();
#define LOCK_TERMINALS
#define UNLOCK_TERMINALS

NetThread::NetThread()
{
    _keepRunning = false;
    _loopInterval = (1/30.0)*1000;
    _workThread = NULL;
}

NetThread::~NetThread()
{
    stop();
    join();
    
    do {
        TaskQueue::iterator it = _taskQueue.begin();
        for (; it!=_taskQueue.end(); ++it) {
            (*it)(this, 0);
        }
        _taskQueue.clear();
    } while (false);
    
    // checkManagedObjects(true);
}

void NetThread::setLoopInterval(int milliseconds)
{
    _loopInterval = milliseconds;
}

void NetThread::runTask(const std::function<TaskFunction>& task)
{
    if (task==NULL) {
        return;
    }
    _taskMutex.lock();
    _taskQueue.push_back(task);
    _taskMutex.unlock();
}

bool NetThread::isInWorkthread() const
{
    return _workThread && _workThread->get_id()==std::this_thread::get_id();
}

void NetThread::start()
{
    if (_workThread) {
        return;
    }
    _keepRunning = true;
    _workThread = new std::thread([](NetThread* obj) {
        obj->main();
    }, this);
}

void NetThread::stop()
{
    _keepRunning = false;
}

void NetThread::join()
{
    if (_workThread) {
        _workThread->join();
        delete _workThread;
        _workThread = NULL;
    }
}

void NetThread::main()
{
    onInit();
    while (_keepRunning) {
        struct timeval begin;
        gettimeofday(&begin, NULL);
        
        doTasks();
        update();
        checkManagedObjects(false);
        
        struct timeval now;
        gettimeofday(&now, NULL);
        int elapsed = (int)difftimeval(&now, &begin);
        if (elapsed<_loopInterval) {
#if defined(WIN32)
			Sleep((_loopInterval - elapsed));
#else
            usleep((_loopInterval-elapsed)*1000);
#endif
        }
    }
    onExit();
}

void NetThread::doTasks()
{
    do {
        _taskMutex.lock();
        if (_taskQueue.size()>0) {
            std::function<TaskFunction> task = _taskQueue.front();
            _taskQueue.pop_front();
            _taskMutex.unlock();
            task(this, 1);
        } else {
            _taskMutex.unlock();
            break;
        }
    } while (true);
}

void NetThread::onInit()
{
    
}

void NetThread::onExit()
{
//    _selector.select();
//    _selector.removeAllSessions();
    checkManagedObjects(true);
}

void NetThread::update()
{
    LOCK_TERMINALS
    for (auto it=_terminals.begin(); it!=_terminals.end(); ++it) {
        (*it)->flush();
    }
    UNLOCK_TERMINALS
    
    // select timeout is 10 milliseconds
    _selector.select(10);
    
    LOCK_TERMINALS
    for (auto it=_terminals.begin(); it!=_terminals.end(); ++it) {
        (*it)->receive();
    }
    UNLOCK_TERMINALS
}

void NetThread::manageObject(AutoObject* object)
{
    if (object==NULL) {
        return;
    }
    _autoObjectsMutex.lock();
    _managedObjects.insert(object);
    _autoObjectsMutex.unlock();
}

void NetThread::checkManagedObjects(bool forceCleanup)
{
    _autoObjectsMutex.lock();
    ManagedObjectPool::iterator it = _managedObjects.begin();
    while (it!=_managedObjects.end()) {
        AutoObject* object = *it;
        if (object->tryCleaning(this, forceCleanup)) {
            it = _managedObjects.erase(it);
            delete object;
        } else {
            ++it;
        }
    }
    _autoObjectsMutex.unlock();
}

void NetThread::addRPCTerminal(RPCTransport* rpc)
{
    if (rpc==NULL) {
        return;
    }
    LOCK_TERMINALS
    RPCTerminals::iterator it = std::find(_terminals.begin(), _terminals.end(), rpc);
    if (it==_terminals.end()) {
        _terminals.push_back(rpc);
    }
    UNLOCK_TERMINALS
}

void NetThread::removeRPCTerminal(RPCTransport* rpc)
{
    if (rpc==NULL) {
        return;
    }
    LOCK_TERMINALS
    RPCTerminals::iterator it = std::find(_terminals.begin(), _terminals.end(), rpc);
    if (it!=_terminals.end()) {
        _terminals.erase(it);
    }
    UNLOCK_TERMINALS
}

void NetThread::addSession(SessionActor* session)
{
    if (session) {
        LOCK_SELECTOR
        _selector.addSessionActor(session);
        UNLOCK_SELECTOR
    }
}

void NetThread::removeSession(SessionActor* session)
{
    if (session) {
        LOCK_SELECTOR
        _selector.removeSessionActor(session);
        UNLOCK_SELECTOR
    }
}

////
NetController::NetController()
{
    _clientThread = NULL;
}

NetController::~NetController()
{
    if (_clientThread) {
        _clientThread->stop();
        _clientThread->join();
        delete _clientThread;
    }
}

NetThread* NetController::allocateNetThread()
{
    if (_clientThread==NULL) {
        _clientThread = new NetThread();
        _clientThread->start();
    }
    return _clientThread;
}

RPCClient* NetController::createClient()
{
    RPCClient* client = NULL;
    NetThread* workThread = allocateNetThread();
    if (workThread) {
        client = new RPCClient(workThread);
    }
    if (client) {
        _managedActivities.push_back(client);
    }
    return client;
}

bool NetController::destroyActivity(RPCActivity *activity)
{
    if (activity==NULL) {
        return true;
    }
    RPCActivityList::iterator it = std::find(_managedActivities.begin(),
                                             _managedActivities.end(),
                                             activity);
    if (it!=_managedActivities.end()) {
        _managedActivities.erase(it);
        delete activity;
        return true;
    } else {
        return false;
    }
}
