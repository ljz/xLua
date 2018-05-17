//
//  CommunicationWork.hpp
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef CommunicationWork_hpp
#define CommunicationWork_hpp

#include <stdio.h>
#include "pigeon/netbasic.h"
class UDPSocket;
class CommunicationWork{
public:
    CommunicationWork(UDPSocket* socket);
    ~CommunicationWork();
public:
    void setLoopInterval(int milliseconds);
    bool isInWorkthread() const;
    void start();
    void stop();
    void pause(){_isPause = true;};
    void resume(){_isPause = false;};
    void join();
    void main();
    void update();
    typedef std::list< std::function<void(void)> > TaskQueue;
    void runTask(const std::function<void(void)>& task);
    void doTasks();

private:
    bool _keepRunning;
    bool _isPause;
    int _loopInterval;
    std::thread* _workThread;
    UDPSocket* _socket;
    
    TaskQueue _taskQueue;
    std::mutex _taskMutex;
};
#endif /* CommunicationWork_hpp */
