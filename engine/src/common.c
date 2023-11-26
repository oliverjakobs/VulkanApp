#include "common.h"

#include <stdio.h>
#include <string.h>

/* --------------------------| logging |--------------------------------- */
#define MINIMAL_LOG_BLACK       "\x1b[30m"
#define MINIMAL_LOG_RED         "\x1b[31m"
#define MINIMAL_LOG_GREEN       "\x1b[32m"
#define MINIMAL_LOG_YELLOW      "\x1b[33m"
#define MINIMAL_LOG_BLUE        "\x1b[34m"
#define MINIMAL_LOG_MAGENTA     "\x1b[35m"
#define MINIMAL_LOG_CYAN        "\x1b[36m"
#define MINIMAL_LOG_WHITE       "\x1b[37m"

#define MINIMAL_LOG_BG_BLACK    "\x1b[40m"
#define MINIMAL_LOG_BG_RED      "\x1b[41m"
#define MINIMAL_LOG_BG_GREEN    "\x1b[42m"
#define MINIMAL_LOG_BG_YELLOW   "\x1b[43m"
#define MINIMAL_LOG_BG_BLUE     "\x1b[44m"
#define MINIMAL_LOG_BG_MAGENTA  "\x1b[45m"
#define MINIMAL_LOG_BG_CYAN     "\x1b[46m"
#define MINIMAL_LOG_BG_WHITE    "\x1b[47m"

#define MINIMAL_LOG_RESET       "\x1b[0m" /* no color */

static const char* minimalLoggerGetLevelStr(MinimalLogLevel level)
{
    switch (level)
    {
    case MINIMAL_LOG_TRACE:     return MINIMAL_LOG_WHITE "[TRACE]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_INFO:      return MINIMAL_LOG_GREEN "[INFO]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_WARN:      return MINIMAL_LOG_YELLOW "[WARN]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_ERROR:     return MINIMAL_LOG_RED "[ERROR]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_CRITICAL:  return MINIMAL_LOG_WHITE MINIMAL_LOG_BG_RED "[CRITICAL]" MINIMAL_LOG_RESET " ";
    default: return "";
    }
}

void minimalLoggerPrint(MinimalLogLevel level, const char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    minimalLoggerPrintV(level, fmt, arg);
    va_end(arg);
}

void minimalLoggerPrintV(MinimalLogLevel level, const char* fmt, va_list args)
{
    fprintf(stderr, "%s", minimalLoggerGetLevelStr(level));
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}


void* minimalMemCopy(void* dst, const void* src, u64 size)
{
    return memcpy(dst, src, size);
}

void* minimalMemZero(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* minimalMemSet(void* block, i32 value, u64 size)
{
    return memset(block, value, size);
}
