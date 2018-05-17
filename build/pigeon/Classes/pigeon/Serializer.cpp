#include "Serializer.h"

#define MAX_CAPACITY 0x10000000
#define MIN_CAPACITY 16

static char __endianBytes__[4] = {1,2,3,4};
static inline bool isLittleEndian()
{
    unsigned int iv = *(unsigned int*)__endianBytes__;
    return iv==0x04030201;
}

static uint32_t nextPowerOf2(uint32_t x){
    x = x-1;
    x = x|(x>>1);
    x = x|(x>>2);
    x = x|(x>>4);
    x = x|(x>>8);
    x = x|(x>>16);
    return x+1;
}

Serializer::Serializer(size_t capacity)
{
    _buffer = NULL;
    _capacity = 0;
    _length = 0;
    _pointer = 0;

    insureBuffer(capacity>MAX_CAPACITY ? MAX_CAPACITY : capacity);
}

Serializer::~Serializer()
{
    if (_buffer) {
        ::free(_buffer);
        _buffer = NULL;
    }
}

int Serializer::insureBuffer(size_t reservedBytes)
{
    if (_buffer==NULL || _capacity < _pointer + reservedBytes) {
        if (_pointer+reservedBytes > MAX_CAPACITY) {
            // capacity too large
            return errCapacityLimited;
        }

        uint8_t* oldbuffer = _buffer;
        size_t newCapacity = 0;
        if (_capacity==0) {
            newCapacity = reservedBytes;
        } else {
            newCapacity = nextPowerOf2((int32_t)(_pointer+reservedBytes));
        }
        if (newCapacity < MIN_CAPACITY) {
            newCapacity = MIN_CAPACITY;
        }

        void* newBuffer = ::malloc(newCapacity);
        if (newBuffer==NULL) {
            // out of memory
            return errOutOfMemory;
        }

        _buffer = (uint8_t*)newBuffer;
        _capacity = newCapacity;

        if (oldbuffer) {
            if (_buffer!=NULL && _length>0) {
                memcpy(_buffer, oldbuffer, _length);
            }
            ::free(oldbuffer);
        }
    }

    return errNoError;
}

void Serializer::advance(size_t step)
{
    _pointer += step;
    if (_pointer>_length) {
        _length = _pointer;
    }
}

int Serializer::seek(long offset, int fromwhere)
{
    size_t pos = 0;
    if (fromwhere==SEEK_CUR) {
        pos = _pointer;
    } else if (fromwhere==SEEK_END) {
        pos = _length;
    }
    pos += offset;
    if (pos<=_length) {
        _pointer = pos;
        return 0;
    } else {
        return -1;
    }
}

size_t Serializer::truncate(size_t length)
{
    if (_length>length) {
        _length = length;
        if (_pointer>length) {
            _pointer = length;
        }
    }
    return _length;
}

int Serializer::reserve(size_t numberOfBytes, bool fill, uint8_t filled)
{
    int error = errNoError;
    if ((error = insureBuffer(numberOfBytes))!=errNoError) {
        return error;
    }
    if (fill) {
        memset(_buffer+_pointer, filled, numberOfBytes);
    }
    advance(numberOfBytes);
    return error;
}
int Serializer::clean()
{
    int error = errNoError;
    _pointer = 0;
    _length = _capacity;
    memset(_buffer+_pointer, 0, _capacity);
    return error;
}

int Serializer::writeBytes(const void* bytes, size_t numberOfBytes)
{
    int error = errNoError;
    if (bytes==NULL || numberOfBytes==0) { return errInvalidArguments; }
    if ((error = insureBuffer(numberOfBytes))!=errNoError) { return error; }
    memcpy(_buffer+_pointer, bytes, numberOfBytes);
    advance(numberOfBytes);
    return error;
}

#define INSURE_BUFFER(value) \
    if ((error = insureBuffer(sizeof(value)))!=errNoError) { return error; }

#define REPEAT2(expr) expr expr
#define REPEAT4(expr) REPEAT2(expr) REPEAT2(expr)
#define REPEAT8(expr) REPEAT4(expr) REPEAT4(expr)

#define WRITE_VALUE(n, value_t, value) {\
    union { value_t val; uint8_t bytes[sizeof(value_t)]; };\
    val = value;\
    uint8_t* dest = _buffer+_pointer;\
    if (isLittleEndian()) {\
        size_t i = sizeof(value);\
        REPEAT##n(*(dest++) = bytes[--i];)\
    } else {\
        size_t i = 0;\
        REPEAT##n(*(dest++) = bytes[i++];)\
    }\
}

#define IMPL_WRITE_VALUE(size, value_t, value) \
    int error = errNoError;\
    INSURE_BUFFER(value)\
    WRITE_VALUE(size, value_t, value)\
    advance(sizeof(value));\
    return error;

int Serializer::writeUint8(uint8_t value)
{
    int error = errNoError;
    INSURE_BUFFER(value)
    _buffer[_pointer] = value;
    advance(sizeof(value));
    return error;
}

int Serializer::writeUint16(uint16_t value)
{
    IMPL_WRITE_VALUE(2, uint16_t, value)
}

int Serializer::writeUint32(uint32_t value)
{
    IMPL_WRITE_VALUE(4, uint32_t, value)
}

int Serializer::writeUint64(uint64_t value)
{
    IMPL_WRITE_VALUE(8, uint64_t, value)
}

int Serializer::writeInt8(int8_t value)
{
    int error = errNoError;
    INSURE_BUFFER(value)
    _buffer[_pointer] = (uint8_t)value;
    advance(sizeof(value));
    return error;
}

int Serializer::writeInt16(int16_t value)
{
    IMPL_WRITE_VALUE(2, int16_t, value)
}

int Serializer::writeInt32(int32_t value)
{
    IMPL_WRITE_VALUE(4, int32_t, value)
}

int Serializer::writeInt64(int64_t value)
{
    IMPL_WRITE_VALUE(8, int64_t, value)
}
