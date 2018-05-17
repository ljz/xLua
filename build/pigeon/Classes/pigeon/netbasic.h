#ifndef __pigeon_netbasic_h
#define __pigeon_netbasic_h
// #ifdef WIN32
// #include <cocos2d.h>
// #endif
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <assert.h>

#include <errno.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <io.h>
#include <WS2tcpip.h>
#include <Winsock2.h>
#include "time.h"
#include <windows.h>
#define in_port_t unsigned short
//#pragma  comment(lib,"ws2_32.lib")
#define bzero(a, b) memset(a, 0, b);
#define ssize_t long
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#endif
#else
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <strings.h>
#endif

#if defined(ANDROID)
#include <fcntl.h>
#include <sys/select.h>
typedef unsigned short in_port_t; //找不到android所在的头文件，可能是个隐患吧
#elif defined(WIN32)
#else
#include <sys/fcntl.h>
#endif

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <functional>

#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <sstream>


#define BEGIN_NS_PIGEON  namespace pigeon {
#define END_NS_PIGEON    }
#define USING_NS_PIGEON  using namespace pigeon;

#if !defined(WIN32)
int          inet_pton(int, const char *, void *);
const char  *inet_ntop(int, const void *, char *, size_t);
#else
extern  int gettimeofday(struct timeval *tp, void *tzp);
#endif

// /* Define bzero() as a macro if it's not in standard C library. */
// #ifndef HAVE_BZERO
// #define bzero(ptr,n)        memset(ptr, 0, n)
// #endif

// /* Older resolvers do not have gethostbyname2() */
// #ifndef HAVE_GETHOSTBYNAME2
// #define gethostbyname2(host,family)     gethostbyname((host))
// #endif

#define NL_DEBUG     0
#define NL_INFO      1
#define NL_WARNING   2
#define NL_ERROR     3

#define NLOG(level, fmt, ...)  printf(fmt, ##__VA_ARGS__)

#ifndef MAX
#   define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#   define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define TEST_TIMESCALE 0

#if TEST_TIMESCALE && !defined(IMPL_GETTIME2)
extern int gettimeofday2(struct timeval * __restrict, void * __restrict);
#   define gettimeofday gettimeofday2
#endif

/* Calculates the difference in milliseconds between begin and end. */
inline double difftimeval(const struct timeval* end, const struct timeval* begin) {
    return 1000*(end->tv_sec - begin->tv_sec) + 0.001*(end->tv_usec - begin->tv_usec);
}

extern void hexdump(const void* data, size_t len, const void* startAddr);

// crc32
extern const unsigned int __crc32tab__[];
#define CRC32(crc, ch)   (crc = (crc >> 8) ^ __crc32tab__[(crc ^ (ch)) & 0xff])
extern uint32_t crc32(uint32_t crc0, const void* data, size_t len);

extern float pigeonTickCount();
#endif
