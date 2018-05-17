//
//  UDPServer.hpp
//  pigeon
//
//  Created by Vega on 2017/10/19.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef UDPServer_hpp
#define UDPServer_hpp

#include <stdio.h>
#include "UDPSocket.h"
#include "kcp/UDPProtocol.h"
#include "kcp/UDPDataUtils.h"


typedef std::function<void (int err, int method, const std::string resp,float timestamp)> UDPCallBack;

class UDPServer:public UDPProtocol  {
public:
    static UDPServer* create(std::string ip, int port,unsigned int userflag);
    static void destroyUDPServer(UDPServer* server);
    UDPServer(std::string ip, int port,unsigned int userflag);
    ~UDPServer();
public:
    void setIntervalMs(int interval);
    void setUDPCallBack(UDPCallBack callBack){ _callBack = callBack; };
    void sendUDPData(int method,std::string rid ,std::string data);
    virtual void setCryptoConfig(int cryptoType,std::string config) override;
    bool reconnect();
    bool resetSocket();
    UDPSocket* getSocket(){ return _socket;};

public:
    virtual void getRequestData(Serializer* writer) override;
    virtual int receiveData(Deserializer* reader)  override;
    virtual void notifyNetStateChange(NetState state) override;
    virtual void flush() override;
public:
    UDPDataUtils* getDataUtils(){ return _dataUtil;};
private:
    UDPSocket* _socket;
    UDPCallBack _callBack;
    UDPDataUtils* _dataUtil;
    
    NetState _state;
};
#endif /* UDPServer_hpp */
