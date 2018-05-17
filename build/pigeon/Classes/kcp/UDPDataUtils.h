//
//  UDPData.hpp
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef UDPData_hpp
#define UDPData_hpp

#include <stdio.h>
#include "pigeon/netbasic.h"
#include "pigeon/Deserializer.h"
#include "pigeon/Serializer.h"
#include "pigeon/NetCrypto.h"
#include <queue>
struct Request {
    int _method;
    std::string _rid;
    std::string _data;
};

struct Response
{
    int _method;
    std::string data;
};

class UDPDataUtils
{
public:
    static void encodeInteger(size_t bytes, void* dest, const void* src);
    static void decodeInteger(size_t bytes, void* dest, const void* src);
    int compress(const void* src, size_t srclen, std::string& output);
    int uncompress(const void* src, size_t srclen, std::string& output);
public:
    UDPDataUtils();
    ~UDPDataUtils();
public:
    int pushRequestData(int method,std::string& rid, std::string& data);
    int pushResponseData(Deserializer* reader);
    int getWillSendData(Serializer* writer);
    Response* getResponseData();
    void setCryptoConfig(int type,std::string config);
private:
    int _requestId;

    SymmCrypto* _dataCrypto;
    std::queue<Response*> _responseBuffer;
    std::queue<Request*> _requestsBuffer;
    std::mutex _requestsMutex;
    std::mutex _responseMutex;

};
#endif /* UDPData_hpp */
