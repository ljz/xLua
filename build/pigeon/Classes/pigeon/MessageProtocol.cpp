#include "SessionActor.h"

#define MAX_INT_BYTES 16
#define READMSG_SIZE_MIN 120

static char __endianBytes__[4] = {1,2,3,4};
static bool isLittleEndian()
{
    unsigned int iv = *(unsigned int*)__endianBytes__;
    return iv==0x04030201;
}

struct xmessage_t
{
    uint16_t capacity;
    uint16_t unused;
    struct MessageProtocol::message_t value;
};

struct MessageProtocol::message_t* tomsg(struct xmessage_t* xmsg) {
    if (xmsg==NULL) {
        return NULL;
    } else {
        return &xmsg->value;
    }
}

struct xmessage_t* toxmsg(struct MessageProtocol::message_t* msg) {
    if (msg==NULL) {
        return NULL;
    } else {
        size_t offset = (uint8_t*)&((struct xmessage_t*)0)->value - (uint8_t*)0;
        return (struct xmessage_t*)((uint8_t*)msg - offset);
    }
}

const char* MessageProtocol::getErrorString(int error)
{
    return "<error message>";
}

void MessageProtocol::encodeInteger(size_t bytes, void* dest, const void* src)
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

void MessageProtocol::decodeInteger(size_t bytes, void* dest, const void* src)
{
    encodeInteger(bytes, dest, src);
}

struct MessageProtocol::message_t* MessageProtocol::mallocMessage(uint32_t datalen)
{
    struct xmessage_t* xmsg = NULL;
    xmsg = (struct xmessage_t*)::malloc(sizeof(struct xmessage_t) + datalen);
    if (xmsg) {
        xmsg->capacity = datalen;
    }
    return tomsg(xmsg);
}

struct MessageProtocol::message_t* MessageProtocol::packupMessage(
    uint8_t type, const void* data, uint32_t datalen)
{
    struct message_t* msg = mallocMessage(datalen);
    if (!msg) {
        // out of memory
        return NULL;
    }
    memset(msg, 0, sizeof(struct message_t));
	uint32_t len = datalen + sizeof(((message_t*)0)->type);
	encodeInteger(sizeof(len), &msg->length, &len);
    msg->type = type;
    if (data!=NULL && datalen>0) {
        memcpy(msg->data, data, datalen);
    }
    return msg;
}

void MessageProtocol::freeMessage(struct message_t* msg)
{
    if (msg!=NULL) {
        struct xmessage_t* xmsg = toxmsg(msg);
        ::free(xmsg);
    }
}

void MessageProtocol::initMsgBufferForReading(struct msg_buffer_t* buffer, size_t datalen)
{
    if (buffer) {
        buffer->remainingBytes = sizeof(struct message_t);
        if (buffer->msg==NULL) {
            buffer->msg = mallocMessage(MAX(READMSG_SIZE_MIN, datalen));
        } else {
            struct xmessage_t* xmsg = toxmsg(buffer->msg);
            if (xmsg->capacity<datalen) {
                freeMessage(buffer->msg);
                buffer->msg = mallocMessage(MAX(READMSG_SIZE_MIN, datalen));
            }
        }
        buffer->ptr = (uint8_t*)buffer->msg;
        // memset(buffer->msg, 0, sizeof(struct message_t));
    }
}

void MessageProtocol::initMsgBufferForWriting(struct msg_buffer_t* buffer, struct message_t* msg)
{
    if (buffer) {
        if (msg!=NULL) {
			uint32_t datalen = 0;
            decodeInteger(sizeof(msg->length), &datalen, &msg->length);
			buffer->remainingBytes = sizeof(((struct message_t*)0)->length) + datalen;
        } else {
            buffer->remainingBytes = 0;
        }
        buffer->msg = msg;
        buffer->ptr = (uint8_t*)msg;
    }
}

void MessageProtocol::clearMsgBuffer(struct msg_buffer_t* buffer)
{
    if (buffer) {
        if (buffer->msg) {
            freeMessage(buffer->msg);
        }
        buffer->remainingBytes = 0;
        buffer->msg = NULL;
        buffer->ptr = NULL;
    }
}

int MessageProtocol::fillMsgBuffer(struct msg_buffer_t* buffer, int sockfd)
{
    int error = 0;
    while (buffer->remainingBytes > 0) {
        error = 0;
#if defined(WIN32)
		ssize_t n = ::recv(sockfd, (char*)buffer->ptr, (int)buffer->remainingBytes, 0);
#else
        ssize_t n = ::read(sockfd, buffer->ptr, buffer->remainingBytes);
#endif
        if (n < 0) {
#if defined(WIN32)
			error = WSAGetLastError();
#else
            error = errno;
#endif
            if (error==EINTR) {
                continue;
            } else {
                // EWOULDBLOCK or other errors
                break;
            }
        } else if (n==0) {
            error = EOF;
            break;
        } else {
            assert(n <= buffer->remainingBytes);
            buffer->remainingBytes -= n;
            buffer->ptr += n;
            if (buffer->remainingBytes==0) {
                uint8_t* p = (uint8_t*)buffer->msg->data;
                if (buffer->ptr==p) {
					uint32_t len = 0;
					decodeInteger(sizeof(len), &len, &buffer->msg->length);
					buffer->remainingBytes = len - sizeof(((struct message_t*)0)->type);
                    struct xmessage_t* xmsg = toxmsg(buffer->msg);
                    if (len > xmsg->capacity) {
                        struct message_t* msg = buffer->msg;
						//len 收到数据长度包含type和reserved
						uint32_t data_len = len - sizeof(((struct message_t*)0)->type);
// 						buffer->msg->type = msg->type;
// 						buffer->msg->reserved = msg->reserved;
						buffer->msg = mallocMessage(data_len);
						memcpy(buffer->msg, msg, sizeof(message_t));
						buffer->ptr = (uint8_t*)buffer->msg + sizeof(struct message_t);;
                        freeMessage(msg);
                    }
                }
            }
        }
    }
    return error;
}

int MessageProtocol::flushMsgBuffer(struct msg_buffer_t* buffer, int sockfd)
{
    int error = 0;
    while (buffer->remainingBytes > 0) {
        error = 0;
	
        // NOTE
        //     Writing a socket that received RST will cause SIGPIPE signal.
        // We should ignore or capture this signal (SIGPIPE) to avoid crash.
        // Anyway, ::write() will get EPIPE if the process is not terminated
        // by kernel. Following code can be used to ignore SIGPIPE:
        //     signal(SIGPIPE, SIG_IGN);
        //
        // Under OSX/iOS, we can set SO_NOSIGPIPE option to avoid SIGPIPE:
        //     int on=1;
        //     setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
        //
        // Under Linux/Android, we can use `::send(..., MSG_NOSIGNAL)` instead
        // of `::write(...)`.

#if defined(WIN32)
		ssize_t n = ::send(sockfd, (char*)buffer->ptr, (int)buffer->remainingBytes, 0);
#elif defined(ANDROID) || defined(__linux__)
        ssize_t n = ::send(sockfd, buffer->ptr, buffer->remainingBytes, MSG_NOSIGNAL);
#else
        ssize_t n = ::write(sockfd, buffer->ptr, buffer->remainingBytes);
#endif
        if (n<=0) {
            error = errno;
            if (error==EINTR) {
                continue;
            } else {
                // EWOULDBLOCK or other errors

                // NOTE
                //    Writing a socket that received RST will cause SIGPIPE signal.
                // We should ignore or capture this signal (SIGPIPE) to avoid crash.
                // Anyway, write() will get EPIPE if the process is not terminated
                // by kernel. Following code can be used to ignore SIGPIPE:
                //    signal(SIGPIPE, SIG_IGN);

                break;
            }
        } else {
            assert(n <= buffer->remainingBytes);
            buffer->remainingBytes -= n;
            buffer->ptr += n;
        }
    }
    return error;
}
