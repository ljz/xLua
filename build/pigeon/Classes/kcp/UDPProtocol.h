//
//  UDPProtocol.h
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef UDPProtocol_h
#define UDPProtocol_h

#include <stdio.h>
#include "pigeon/Serializer.h"
#include "pigeon/Deserializer.h"
#include "kcp/UDPDataUtils.h"
enum NetState{
    kUnknown = -1,
    kConnected = 0,
    kConnectErr = 1,
    kMemoryErr = 2,
    kKCPErr =3,
    kSendErr = 4,
    kRevErr = 5
};

class UDPProtocol{
public:
    virtual void getRequestData(Serializer* writer) = 0;
    virtual int receiveData(Deserializer* reader) = 0;
    virtual void notifyNetStateChange(NetState state) = 0;
    virtual void flush() = 0;
    virtual void setCryptoConfig(int cryptoType,std::string config) = 0;
};
#endif /* UDPProtocol_h */
