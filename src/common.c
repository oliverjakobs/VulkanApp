#include "common.h"

#include <stdio.h>

void MinimalGetVersion(int* major, int* minor, int* rev) {
    if (major != NULL) *major = MINIMAL_VERSION_MAJOR;
    if (minor != NULL) *minor = MINIMAL_VERSION_MINOR;
    if (rev != NULL)   *rev = MINIMAL_VERSION_REVISION;
}

#define MINIMAL_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define MINIMAL_MAKE_VERSION_STR(m, n, r) MINIMAL_CONCAT_VERSION(m, n, r)

const char* MinimalGetVersionString() {
    return MINIMAL_MAKE_VERSION_STR(MINIMAL_VERSION_MAJOR, MINIMAL_VERSION_MINOR, MINIMAL_VERSION_REVISION);
}

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

static const char* MinimalLoggerGetLevelStr(MinimalLogLevel level) {
    switch (level) {
    case MINIMAL_LOG_TRACE:     return MINIMAL_LOG_WHITE "[TRACE]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_INFO:      return MINIMAL_LOG_GREEN "[INFO]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_WARN:      return MINIMAL_LOG_YELLOW "[WARN]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_ERROR:     return MINIMAL_LOG_RED "[ERROR]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_CRITICAL:  return MINIMAL_LOG_WHITE MINIMAL_LOG_BG_RED "[CRITICAL]" MINIMAL_LOG_RESET " ";
    default: return "";
    }
}

void MinimalLoggerPrint(FILE* const stream, MinimalLogLevel level, const char* fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    MinimalLoggerPrintV(stream, level, fmt, arg);
    va_end(arg);
}

void MinimalLoggerPrintV(FILE* const stream, MinimalLogLevel level, const char* fmt, va_list args) {
    fprintf(stream, MinimalLoggerGetLevelStr(level));
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

/* --------------------------| utils |----------------------------------- */
char* readFile(const char* path, size_t* sizeptr) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        MINIMAL_ERROR("Failed to open file: %s", path);
        return NULL;
    }

    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        MINIMAL_ERROR("Failed to allocate memory for file: %s", path);
        fclose(file);
        return NULL;
    }

    if (fread(buffer, size, 1, file) != 1) {
        MINIMAL_ERROR("Failed to read file: %s", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[size] = '\0'; /* zero terminate buffer */
    if (sizeptr) *sizeptr = size;

    fclose(file);
    return buffer;
}