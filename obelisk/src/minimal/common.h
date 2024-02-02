#ifndef MINIMAL_COMMON_H
#define MINIMAL_COMMON_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) // Windows
    #define MINIMAL_PLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows!"
    #endif
// Linux
#elif defined(__linux__) || defined(__gnu_linux__)
    #define MINIMAL_PLATFORM_LINUX 1
// Catch anything not caught by the above.
#elif defined(__unix__)
    #define MINIMAL_PLATFORM_UNIX 1
// Posix
#elif defined(_POSIX_VERSION)
    #define MINIMAL_PLATFORM_POSIX 1
#else
    #error "Unknown platform!"
#endif

#ifdef MINIMAL_EXPORT // Exports
    #ifdef _MSC_VER
        #define MINIMAL_API __declspec(dllexport)
    #else
        #define MINIMAL_API __attribute__((visibility("default")))
    #endif
#else // Imports
    #ifdef _MSC_VER
        #define MINIMAL_API __declspec(dllimport)
    #else
        #define MINIMAL_API
    #endif
#endif

#define MINIMAL_FAIL    0
#define MINIMAL_OK      1

#ifndef _DEBUG
#define MINIMAL_DISABLE_LOGGING
#define MINIMAL_DISABLE_ASSERT
#endif

typedef struct MinimalWindow MinimalWindow;
typedef struct MinimalEvent MinimalEvent;

/* --------------------------| logging |--------------------------------- */
#ifndef MINIMAL_DISABLE_LOGGING

#define MINIMAL_TRACE(s, ...)       minimalLoggerPrint(MINIMAL_LOG_TRACE, s, __VA_ARGS__)
#define MINIMAL_INFO(s, ...)        minimalLoggerPrint(MINIMAL_LOG_INFO, s, __VA_ARGS__)
#define MINIMAL_WARN(s, ...)        minimalLoggerPrint(MINIMAL_LOG_WARN, s, __VA_ARGS__)
#define MINIMAL_ERROR(s, ...)       minimalLoggerPrint(MINIMAL_LOG_ERROR, s, __VA_ARGS__)
#define MINIMAL_CRITICAL(s, ...)    minimalLoggerPrint(MINIMAL_LOG_CRITICAL, s, __VA_ARGS__)

#else

#define MINIMAL_TRACE(s, ...)
#define MINIMAL_INFO(s, ...)
#define MINIMAL_WARN(s, ...)
#define MINIMAL_ERROR(s, ...)
#define MINIMAL_CRITICAL(s, ...)

#endif

typedef enum
{
    MINIMAL_LOG_TRACE,
    MINIMAL_LOG_INFO,
    MINIMAL_LOG_WARN,
    MINIMAL_LOG_ERROR,
    MINIMAL_LOG_CRITICAL
} MinimalLogLevel;

MINIMAL_API void minimalLoggerPrint(MinimalLogLevel level, const char* fmt, ...);
MINIMAL_API void minimalLoggerPrintV(MinimalLogLevel level, const char* fmt, va_list args);

/* --------------------------| assert |---------------------------------- */
#ifndef MINIMAL_DISABLE_ASSERT
#include <assert.h>
#define MINIMAL_ASSERT(expr, msg) assert(((void)(msg), (expr)))
#else
#define MINIMAL_ASSERT(expr, msg)
#endif

/* --------------------------| memory |---------------------------------- */
#define MINIMAL_ALLOC(size)             malloc(size)
#define MINIMAL_FREE(block, size)       free(block)
#define MINIMAL_MEMCPY(dst, src, size)  memcpy(dst, src, size);

#endif // !MINIMAL_COMMON_H
