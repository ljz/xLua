//
//  CommunicationWork.cpp
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#include "CommunicationWork.hpp"
#include "kcp/UDPSocket.h"
CommunicationWork::CommunicationWork(UDPSocket* socket)
{
    _keepRunning = false;
    _isPause = false;
    _loopInterval = 10;//(1/30.0)*1000;
    _workThread = NULL;
    _socket = socket;
}

CommunicationWork::~CommunicationWork()
{

    stop();
    _taskMutex.lock();
    _taskQueue.clear();
    _taskMutex.unlock();
    join();

}

void CommunicationWork::setLoopInterval(int milliseconds)
{
    _loopInterval = milliseconds;
}

bool CommunicationWork::isInWorkthread() const
{
    return _workThread && _workThread->get_id()==std::this_thread::get_id();
}

void CommunicationWork::start()
{
    if (_workThread) {
        return;
    }
    _keepRunning = true;
    _workThread = new std::thread([](CommunicationWork* obj) {
        obj->main();
    }, this);
}

void CommunicationWork::stop()
{
    _keepRunning = false;
}

void CommunicationWork::join()
{
    if (_workThread) {
        _workThread->join();
        delete _workThread;
        _workThread = NULL;
    }
}

void CommunicationWork::main()
{
    while (_keepRunning) {
        struct timeval begin;
        gettimeofday(&begin, NULL);
        if(!_isPause){
            update();
            doTasks();
        }
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
}

void CommunicationWork::update()
{
    _socket->select(0.0);
}
void CommunicationWork::runTask(const std::function<void(void)>& task)
{
    if (task==NULL) {
        return;
    }
    _taskMutex.lock();
    _taskQueue.push_back(task);
    _taskMutex.unlock();
}
void CommunicationWork::doTasks()
{
    do {
        _taskMutex.lock();
        if (_taskQueue.size()>0) {
            std::function<void(void)> task = _taskQueue.front();
            _taskQueue.pop_front();
            _taskMutex.unlock();
            task();
        } else {
            _taskMutex.unlock();
            break;
        }
    } while (true);
}
