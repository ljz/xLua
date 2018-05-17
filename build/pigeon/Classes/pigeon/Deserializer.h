#ifndef __pigeon_deserializer_h
#define __pigeon_deserializer_h

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <stdint.h>

#include <string>

/**
@desc 反序列化器。从字节流中读取数据。其中整数/浮点数按大端方式进行解码。
*/
class Deserializer
{
public:
    enum Errors {
        errNoError,
        errInvalidArguments,
        errNoData,
        errUnexpectedEOF,
    };

    Deserializer(void* data, size_t length, void (*freeFunc)(void*) = NULL);
    ~Deserializer();

    void setDataSource(void* data, size_t length, void (*freeFunc)(void*) = NULL);
    const uint8_t* buffer() const { return _buffer; }
    size_t length() const { return _length; }
    size_t tell() const { return _pointer; }
    size_t available() const { return _pointer<_length ? _length-_pointer : 0; }
    int seek(long offset, int fromwhere);
    void clean();
    void skip(size_t numberOfBytes) { advance(numberOfBytes); }
    int readBytes(void* bytes, size_t numberOfBytes);
    int readString(std::string& out, size_t numberOfBytes);

    int readUint8(uint8_t* value);
    int readUint16(uint16_t* value);
    int readUint32(uint32_t* value);
    int readUint64(uint64_t* value);
    int readInt8(int8_t* value);
    int readInt16(int16_t* value);
    int readInt32(int32_t* value);
    int readInt64(int64_t* value);
    int insureBuffer(size_t reservedBytes);

    // int readFloat(float* value); // 32bits
    // int readDouble(double* value); // 64bits
private:
    int checkBuffer(size_t wantedBytes);
    void advance(size_t step);

    uint8_t* _buffer;
    void (*_freeFunc)(void*);
    size_t _length;
    size_t _pointer;
};

#endif
