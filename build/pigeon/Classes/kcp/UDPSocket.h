//
//  UDPSocket.h
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef SOCKET_H_
#define SOCKET_H_

#include "pigeon/netbasic.h"
#include "kcp/UDPProtocol.h"
#include "kcp/CommunicationWork.hpp"
#include "kcp/ikcp.h"

class UDPSocket {
public:
    UDPSocket(std::string ip ,int port);
    virtual ~UDPSocket();
    static UDPSocket* create(std::string ip ,int port,unsigned int userflag);
public:
    bool init(unsigned int userflag);
    bool initAddrInfo();
	void closeSocket();
    void setInterval(int interval);
    void setKCPConfig(int sndwnd =215, int rcvwnd = 512,int nodelay = 1, int interval = 10, int resend = 2, int nc = 1,int minrto=10,int mtu = 512);
	bool sendData(const char* data,int len);
	bool receiveData();
    void setUDPProtocol(UDPProtocol* protocol){_protocol = protocol;};
    UDPProtocol* getUDPProtocol(){ return _protocol;};
    void destorySelf();
    void select(long timeout);
    bool createKCP();
    bool reconnect();
    bool resetSocket();
private:
    int fdset(int* maxfd, fd_set* rset, fd_set* wset, fd_set* eset);
    void update(const fd_set* rset,
                        const fd_set* wset,
                        const fd_set* eset);

private:
    std::string _ip;
    int _port ;
    struct addrinfo* _addrinfo;
    int _interval;
    struct timeval _now;
    IUINT32 _nextUpdateTime;
    
	int fd;
    ikcpcb* _kcp;

    
    UDPProtocol* _protocol;
    CommunicationWork* _work;
    Serializer* _writer;
    Deserializer* _reader;
    IUINT32 _userFlag;
    
    bool _isNeedReSend;

};

#endif /* SOCKET_H_ */
