#ifndef __pigeon_serializer_h
#define __pigeon_serializer_h

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <stdint.h>

/**
@desc 序列化器。把指定的数据写入到字节流中。其中整数/浮点数按大端方式进行编码。
*/
class Serializer
{
public:
    enum Errors {
        errNoError,
        errInvalidArguments,
        errOutOfMemory,
        errCapacityLimited,
    };

    Serializer(size_t capacity = 0);
    ~Serializer();

    uint8_t* buffer() { return _buffer; }
    const uint8_t* buffer() const { return _buffer; }
    size_t length() const { return _length; }
    size_t tell() const { return _pointer; }
    int seek(long offset, int fromwhere);
    size_t truncate(size_t length);
    
    int clean();
    // insure enough buffer and advance the pointer
    int reserve(size_t numberOfBytes, bool fill = false, uint8_t filled = 0);
    int writeBytes(const void* bytes, size_t numberOfBytes);

    int writeUint8(uint8_t value);
    int writeUint16(uint16_t value);
    int writeUint32(uint32_t value);
    int writeUint64(uint64_t value);
    int writeInt8(int8_t value);
    int writeInt16(int16_t value);
    int writeInt32(int32_t value);
    int writeInt64(int64_t value);

    // int writeFloat(float value); // 32bits
    // int writeDouble(double value); // 64bits
private:
    int insureBuffer(size_t reservedBytes);
    void advance(size_t step);

    uint8_t* _buffer;
    size_t _capacity;
    size_t _length;
    size_t _pointer;
};

#endif
