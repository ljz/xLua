//
//  UDPServer.cpp
//  pigeon
//
//  Created by Vega on 2017/10/19.
//  Copyright © 2017年 gamebase. All rights reserved.
//

// #ifdef WIN32
// #include "cocos2d.h"
// #endif

#include "kcp/UDPServer.h"
#include "kcp/UDPDataUtils.h"
UDPServer* UDPServer::create(std::string ip, int port, unsigned int userflag)
{
    UDPServer* server = new UDPServer(ip,port,userflag);
    if(server->getSocket())
    {
        return server;
    }
    delete server;
    server = NULL;
    return NULL;
}
void UDPServer::destroyUDPServer(UDPServer *server)
{
    if(server)
    {
        delete server;
    }
    server = NULL;
}
UDPServer::UDPServer(std::string ip,int port,unsigned int userflag)
:_dataUtil(NULL)
,_callBack(nullptr)
,_state(NetState::kUnknown)
{
    _socket = UDPSocket::create(ip,port,userflag);
    if(_socket)
    {
		_dataUtil = new UDPDataUtils();
        _socket->setUDPProtocol(this);
    }
    else
    {
        _socket = NULL;
    }

}
UDPServer::~UDPServer()
{
    if(_socket){
        _socket->destorySelf();
        delete _socket;
    }
    _socket = NULL;
    
    if(_dataUtil)
    {
        delete _dataUtil;
    }
    _dataUtil = NULL;
}
void UDPServer::setIntervalMs(int interval)
{
    if(_socket)
    {
        _socket->setInterval(interval);
    }
}

void UDPServer::sendUDPData(int method,std::string rid, std::string data)
{
    if(_dataUtil)
    {
        _dataUtil->pushRequestData(method,rid,data);
    }
}
void UDPServer::getRequestData(Serializer* writer)
{
    if(_dataUtil)
    {
        _dataUtil->getWillSendData(writer);
    }
}

void UDPServer::setCryptoConfig(int cryptoType,std::string config)
{
    if(_dataUtil)
        _dataUtil->setCryptoConfig(cryptoType,config);
}
int UDPServer::receiveData(Deserializer* reader)
{
    if(_dataUtil)
    {
       return  _dataUtil->pushResponseData(reader);
    }
    return 0;
}

void UDPServer::notifyNetStateChange(NetState state)
{
    if(_state == state)
        return;
    _state = state;
    _callBack((int)state, -1, "",pigeonTickCount());
}
bool UDPServer::reconnect()
{
    if(_socket)
    {
        return _socket->reconnect();
    }
    return false;
}
bool UDPServer::resetSocket()
{
    if(_socket)
    {
        return _socket->resetSocket();
    }
    return false;
}

void UDPServer::flush()
{
    Response* res = NULL;
    while((res = _dataUtil->getResponseData()))
    {   
//#ifdef WIN32
        if (!_callBack._Empty())
//#else
//		if (_callBack)
//#endif
		{
            _state = NetState::kConnected;
            _callBack(0, res->_method, res->data,pigeonTickCount());
		}
        delete res;
    }
}
