//
//  UDPData.cpp
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#include "UDPDataUtils.h"
#include "pigeon/netbasic.h"
#include <snappy.h>

static char __endianBytes__[4] = {1,2,3,4};
static bool isLittleEndian()
{
    unsigned int iv = *(unsigned int*)__endianBytes__;
    return iv==0x04030201;
}

void UDPDataUtils::encodeInteger(size_t bytes, void* dest, const void* src)
{
    if (dest!=src) {
        memmove(dest, src, bytes);
    }
    if (isLittleEndian()) {
        uint8_t* p1 = (uint8_t*)dest;
        uint8_t* p2 = (uint8_t*)dest + (bytes-1);
        while (p1<p2) {
            uint8_t tmp = *p1; *p1 = *p2; *p2 = tmp;
            ++p1; --p2;
        }
    }
}
void UDPDataUtils::decodeInteger(size_t bytes, void* dest, const void* src)
{
    encodeInteger(bytes, dest, src);
}

int UDPDataUtils::compress(const void* src, size_t srclen, std::string& output)
{
    output.clear();
    if (src==NULL || srclen==0) { return 0; }
    return snappy::Compress((const char*)src, srclen, &output);
}

int UDPDataUtils::uncompress(const void* src, size_t srclen, std::string& output)
{
    output.clear();
    if (src==NULL || srclen==0) { return 0; }
    return snappy::Uncompress((const char*)src, srclen, &output);
}


UDPDataUtils::UDPDataUtils():
_requestId(0),
_dataCrypto(NULL)
{
    
}
UDPDataUtils::~UDPDataUtils()
{
    _requestsMutex.lock();
    while(!_requestsBuffer.empty()){
        Request* it = _requestsBuffer.front();
        _requestsBuffer.pop();
        delete it;
    }
    _requestsMutex.unlock();
    _responseMutex.lock();
    while(!_responseBuffer.empty()){
        Request* it = _requestsBuffer.front();
        _responseBuffer.pop();
        delete it;
    }
    _responseMutex.unlock();
}

int UDPDataUtils::pushRequestData(int method,std::string& rid, std::string& data)
{
    _requestsMutex.lock();
    Request* req = new Request();
    req->_data = data;
    req->_rid = rid;
    req->_method = method;
    _requestsBuffer.push(req);
    _requestsMutex.unlock();
    return 0;
}

void UDPDataUtils::setCryptoConfig(int  type, std::string config)
{
    if(_dataCrypto)
        delete _dataCrypto;
    _dataCrypto = NULL;
    _dataCrypto = SymmCryptoFactory::getInstance()->newCrypto(type);
    _dataCrypto->setConfig(config.c_str(), config.length());
}
int UDPDataUtils::pushResponseData(Deserializer* reader)
{
    _responseMutex.lock();
    int ret = 0;
    Response* res = new Response();
    UDPDataUtils::decodeInteger(sizeof(int32_t),&(res->_method), reader->buffer());
//    if(res->_method != 0)
    {
        uint8_t* buffer = NULL;
        size_t offset = sizeof(int32_t);
        if(_dataCrypto)
        {
            int32_t originlen = 0;
            UDPDataUtils::decodeInteger(sizeof(int32_t),&originlen, reader->buffer() + offset);
            
            size_t enclen = reader->tell() - offset*2;
            size_t declen = _dataCrypto->calcDecryptedLength(enclen);
            buffer = (uint8_t*)::malloc(declen);
            const uint8_t* src = reader->buffer() + offset*2;
            if (_dataCrypto->decrypt(buffer, NULL, src, enclen)) {
                if(!UDPDataUtils::uncompress(buffer, originlen, res->data))
                {
                    ret = -2 ;
                }
            }
            free(buffer);
            buffer = NULL;
        }
        else
        {
            if(!UDPDataUtils::uncompress(reader->buffer() +offset ,reader->tell() - offset, res->data))
            {
                ret = -2 ;
            }
        }
    }
//    else
//    {
//        res->data = "";
//    }
    _responseBuffer.push(res);
    _responseMutex.unlock();
    return ret ;
}

int UDPDataUtils::getWillSendData(Serializer* writer){
    _requestsMutex.lock();
    if(_requestsBuffer.size() > 0 )
    {
        Request* it = _requestsBuffer.front();
        _requestsBuffer.pop();
        {
            writer->writeInt8(it->_rid.length());
            writer->writeBytes(it->_rid.c_str(), it->_rid.length());
            writer->writeInt32(it->_method);
            std::string compressData;
            if(UDPDataUtils::compress(it->_data.c_str(), it->_data.length(), compressData))
            {
                size_t commpresslen = compressData.length();
                writer->writeInt32((int32_t)commpresslen);
                if(_dataCrypto)
                {
                    size_t size = 0;
                    size_t datalen = _dataCrypto->calcEncryptedLength(commpresslen);
                    size_t pos = writer->tell();
                    writer->reserve(datalen);
                    if(!_dataCrypto->encrypt(writer->buffer()+ pos, &size, (const uint8_t*)compressData.c_str(), commpresslen))
                    {
                        writer->clean();
                        _requestsMutex.unlock();
                        NLOG(NL_ERROR, "Failed to encrypt message data.\n");
                        return -1;
                    }
                }
                else
                {
                    writer->writeBytes(compressData.c_str(), compressData.length());
                }
                delete it;
            }
            else
            {
                writer->clean();
                _requestsMutex.unlock();
                return -1;
            }
        }
    }
    _requestsMutex.unlock();
    return 0;
}
Response* UDPDataUtils::getResponseData()
{
    Response* ret = NULL;
    _responseMutex.lock();
    if(!_responseBuffer.empty()){
        ret = _responseBuffer.front();
        _responseBuffer.pop();
    }
    _responseMutex.unlock();
    return ret ;
}
