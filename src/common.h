#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define OBELISK_FAIL    0
#define OBELISK_OK      1

#ifdef _DEBUG

#define OBELISK_DEBUG

#define OBELISK_ENABLE_LOGGING
#define OBELISK_ENABLE_ASSERT

#endif

/* obelisk version numbers */
#define OBELISK_VERSION_MAJOR       0
#define OBELISK_VERSION_MINOR       4
#define OBELISK_VERSION_REVISION    0

void obeliskGetVersion(int* major, int* minor, int* rev);
const char* obeliskGetVersionString();

typedef struct ObeliskEvent ObeliskEvent;
typedef struct ObeliskApp ObeliskApp;

/* --------------------------| logging |--------------------------------- */
#ifdef OBELISK_ENABLE_LOGGING

#define OBELISK_TRACE(s, ...)   obeliskPrintf(stdout, OBELISK_LOG_LVL_TRACE, s, __VA_ARGS__)
#define OBELISK_INFO(s, ...)    obeliskPrintf(stdout, OBELISK_LOG_LVL_INFO, s, __VA_ARGS__)
#define OBELISK_WARN(s, ...)    obeliskPrintf(stdout, OBELISK_LOG_LVL_WARN, s, __VA_ARGS__)
#define OBELISK_ERROR(s, ...)   obeliskPrintf(stderr, OBELISK_LOG_LVL_ERROR, s, __VA_ARGS__)
#define OBELISK_FATAL(s, ...)   obeliskPrintf(stderr, OBELISK_LOG_LVL_FATAL, s, __VA_ARGS__)

#else

#define OBELISK_TRACE(s, ...)
#define OBELISK_INFO(s, ...)
#define OBELISK_WARN(s, ...)
#define OBELISK_ERROR(s, ...)
#define OBELISK_FATAL(s, ...)

#endif

typedef enum {
    OBELISK_LOG_LVL_TRACE,
    OBELISK_LOG_LVL_INFO,
    OBELISK_LOG_LVL_WARN,
    OBELISK_LOG_LVL_ERROR,
    OBELISK_LOG_LVL_FATAL
} ObeliskLogLevel;

void obeliskPrintf(FILE* const stream, ObeliskLogLevel level, const char* fmt, ...);

/* --------------------------| assert |---------------------------------- */
void obeliskLogAssertFailure(const char* expr, const char* msg, const char* file, uint32_t line);

#ifdef OBELISK_ENABLE_ASSERT

#if _MSC_VER
#include <intrin.h>
#define obeliskDebugBreak() __debugbreak()
#else
#define obeliskDebugBreak() __builtin_trap()
#endif

#define OBELISK_ASSERT(expr, msg)                                    \
    {                                                                \
        if (!(expr)) {                                               \
            obeliskLogAssertFailure(#expr, msg, __FILE__, __LINE__); \
            obeliskDebugBreak();                                     \
        }                                                            \
    }

#else

#define OBELISK_ASSERT(expr, msg)

#endif

#endif // !COMMON_H
