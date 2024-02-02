#ifndef IGNIS_COMMON_H
#define IGNIS_COMMON_H

#include <stdint.h>

#ifdef _DEBUG
    #define IGNIS_DEBUG
#endif

#define IGNIS_FAIL    0
#define IGNIS_OK      1

void* ignisAlloc(size_t size);
void  ignisFree(void* block, size_t size);

char* ignisReadFile(const char* path, size_t* sizeptr);

uint32_t ignisClamp32(uint32_t val, uint32_t min, uint32_t max);

/*
 * --------------------------------------------------------------
 *                          logging
 * --------------------------------------------------------------
 */
#ifndef IGNIS_DISABLE_LOGGING

#define IGNIS_TRACE(s, ...)     _ignisLog(IGNIS_LOG_TRACE, s, __VA_ARGS__)
#define IGNIS_INFO(s, ...)      _ignisLog(IGNIS_LOC_INFO, s, __VA_ARGS__)
#define IGNIS_WARN(s, ...)      _ignisLog(IGNIS_LOG_WARN, s, __VA_ARGS__)
#define IGNIS_ERROR(s, ...)     _ignisLog(IGNIS_LOG_ERROR, s, __VA_ARGS__)
#define IGNIS_CRITICAL(s, ...)  _ignisLog(IGNIS_LOG_CRITICAL, s, __VA_ARGS__)

#else

#define IGNIS_TRACE(s, ...)
#define IGNIS_INFO(s, ...)
#define IGNIS_WARN(s, ...)
#define IGNIS_ERROR(s, ...)
#define IGNIS_CRITICAL(s, ...)

#endif

typedef enum
{
    IGNIS_LOG_TRACE,
    IGNIS_LOC_INFO,
    IGNIS_LOG_WARN,
    IGNIS_LOG_ERROR,
    IGNIS_LOG_CRITICAL
} IgnisLogLevel;

void _ignisLog(IgnisLogLevel level, const char* fmt, ...);

#endif /* !IGNIS_COMMON_H */
