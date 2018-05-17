#include "Deserializer.h"

static char __endianBytes__[4] = {1,2,3,4};
static inline bool isLittleEndian()
{
    unsigned int iv = *(unsigned int*)__endianBytes__;
    return iv==0x04030201;
}

Deserializer::Deserializer(void* data, size_t length, void (*freeFunc)(void*))
{
    _buffer = (uint8_t*)data;
    _freeFunc = freeFunc;
    _length = length;
    _pointer = 0;
}

Deserializer::~Deserializer()
{
    if (_buffer && _freeFunc) {
        _freeFunc(_buffer);
    }
    _buffer = NULL;
}

void Deserializer::setDataSource(void* data, size_t length, void (*freeFunc)(void*))
{
    if (_buffer && _freeFunc) {
        _freeFunc(_buffer);
    }
    _buffer = (uint8_t*)data;
    _freeFunc = freeFunc;
    _length = length;
    _pointer = 0;
}

int Deserializer::checkBuffer(size_t wantedBytes)
{
    if (_buffer==NULL) {
        return errNoData;
    }
    if (_pointer+wantedBytes > _length) {
        return errUnexpectedEOF;
    }
    return errNoError;
}

void Deserializer::advance(size_t step)
{
    _pointer += step;
}

int Deserializer::seek(long offset, int fromwhere)
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
void Deserializer::clean()
{
    _pointer = 0;
    memset(_buffer+_pointer, 0, _length);
}
int Deserializer::readBytes(void* bytes, size_t numberOfBytes)
{
    int error = errNoError;
    if (bytes==NULL || numberOfBytes==0) { return errInvalidArguments; }
    if ((error = checkBuffer(numberOfBytes))!=errNoError) { return error; }
    memcpy(bytes, _buffer+_pointer, numberOfBytes);
    advance(numberOfBytes);
    return error;
}

int Deserializer::readString(std::string& out, size_t numberOfBytes)
{
    if (numberOfBytes==0) { out = ""; return errNoError; }
    int error = errNoError;
    if ((error = checkBuffer(numberOfBytes))!=errNoError) { return error; }
    out.reserve(numberOfBytes);
    out.assign((const char*)_buffer+_pointer, numberOfBytes);
    advance(numberOfBytes);
    return error;
}

#define CHECK_ARGS(value) if (value==NULL) { return errInvalidArguments; }
#define CHECK_BUFFER(value_t) \
    if ((error = checkBuffer(sizeof(value_t)))!=errNoError) { return error; }

#define REPEAT2(expr) expr expr
#define REPEAT4(expr) REPEAT2(expr) REPEAT2(expr)
#define REPEAT8(expr) REPEAT4(expr) REPEAT4(expr)

#define READ_VALUE(n, value_t, value) {\
    union { value_t val; uint8_t bytes[sizeof(value_t)]; };\
    uint8_t* src = _buffer+_pointer;\
    if (isLittleEndian()) {\
        size_t i = sizeof(value_t);\
        REPEAT##n(bytes[--i] = *(src++);)\
    } else {\
        size_t i = 0;\
        REPEAT##n(bytes[i++] = *(src++);)\
    }\
    *value = val;\
}

#define IMPL_READ_VALUE(size, value_t, value) \
    int error = errNoError;\
    CHECK_ARGS(value)\
    CHECK_BUFFER(value_t)\
    READ_VALUE(size, value_t, value)\
    advance(sizeof(value_t));\
    return error;

int Deserializer::readUint8(uint8_t* value)
{
    int error = errNoError;
    CHECK_ARGS(value)
    CHECK_BUFFER(uint8_t)
    *value = _buffer[_pointer];
    advance(sizeof(uint8_t));
    return error;
}

int Deserializer::readUint16(uint16_t* value)
{
    IMPL_READ_VALUE(2, uint16_t, value)
}

int Deserializer::readUint32(uint32_t* value)
{
    IMPL_READ_VALUE(4, uint32_t, value)
}

int Deserializer::readUint64(uint64_t* value)
{
    IMPL_READ_VALUE(8, uint64_t, value)
}

int Deserializer::readInt8(int8_t* value)
{
    int error = errNoError;
    CHECK_ARGS(value)
    CHECK_BUFFER(uint8_t)
    *value = (int8_t)_buffer[_pointer];
    advance(sizeof(uint8_t));
    return error;
}

int Deserializer::readInt16(int16_t* value)
{
    IMPL_READ_VALUE(2, int16_t, value)
}

int Deserializer::readInt32(int32_t* value)
{
    IMPL_READ_VALUE(4, int32_t, value)
}

int Deserializer::readInt64(int64_t* value)
{
    IMPL_READ_VALUE(8, int64_t, value)
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

int Deserializer::insureBuffer(size_t reservedBytes)
{
    if (_buffer==NULL || _length < _pointer + reservedBytes) {

        uint8_t* oldbuffer = _buffer;
        size_t newCapacity = 0;
        if (_length==0) {
            newCapacity = reservedBytes;
        } else {
            newCapacity = nextPowerOf2((int32_t)(_pointer+reservedBytes));
        }
        void* newBuffer = ::malloc(newCapacity);
        if (newBuffer==NULL) {
            // out of memory
            return 2;
        }
        
        _buffer = (uint8_t*)newBuffer;
        _length = newCapacity;
        
        if (oldbuffer) {
            if (_buffer!=NULL && _length>0) {
                memcpy(_buffer, oldbuffer, _length);
            }
            _freeFunc(oldbuffer);
            oldbuffer = NULL;
        }
    }
    
    return errNoError;
}
