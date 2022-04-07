#include "common.h"

#include <stdarg.h>

void ObeliskGetVersion(int* major, int* minor, int* rev) {
    if (major != NULL) *major = OBELISK_VERSION_MAJOR;
    if (minor != NULL) *minor = OBELISK_VERSION_MINOR;
    if (rev != NULL)   *rev =   OBELISK_VERSION_REVISION;
}

#define OBELISK_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define OBELISK_MAKE_VERSION_STR(m, n, r) OBELISK_CONCAT_VERSION(m, n, r)

const char* ObeliskGetVersionString() {
    return OBELISK_MAKE_VERSION_STR(OBELISK_VERSION_MAJOR, OBELISK_VERSION_MINOR, OBELISK_VERSION_REVISION);
}

/* --------------------------| logging |--------------------------------- */
/* foreground color */
#define OBELISK_LOG_BLACK       "\x1b[30m"
#define OBELISK_LOG_RED         "\x1b[31m"
#define OBELISK_LOG_GREEN       "\x1b[32m"
#define OBELISK_LOG_YELLOW      "\x1b[33m"
#define OBELISK_LOG_BLUE        "\x1b[34m"
#define OBELISK_LOG_MAGENTA     "\x1b[35m"
#define OBELISK_LOG_CYAN        "\x1b[36m"
#define OBELISK_LOG_WHITE       "\x1b[37m"
/* bakcground color */
#define OBELISK_LOG_BG_BLACK    "\x1b[40m"
#define OBELISK_LOG_BG_RED      "\x1b[41m"
#define OBELISK_LOG_BG_GREEN    "\x1b[42m"
#define OBELISK_LOG_BG_YELLOW   "\x1b[43m"
#define OBELISK_LOG_BG_BLUE     "\x1b[44m"
#define OBELISK_LOG_BG_MAGENTA  "\x1b[45m"
#define OBELISK_LOG_BG_CYAN     "\x1b[46m"
#define OBELISK_LOG_BG_WHITE    "\x1b[47m"
/* no color/reset */
#define OBELISK_LOG_RESET       "\x1b[0m"

static const char* obeliskLogLevelStrings[] = {
    [OBELISK_LOG_LVL_FATAL] = OBELISK_LOG_WHITE OBELISK_LOG_BG_RED "[FATAL]" OBELISK_LOG_RESET " ",
    [OBELISK_LOG_LVL_ERROR] = OBELISK_LOG_RED    "[ERROR]" OBELISK_LOG_RESET " ",
    [OBELISK_LOG_LVL_WARN] =  OBELISK_LOG_YELLOW "[WARN]"  OBELISK_LOG_RESET " ",
    [OBELISK_LOG_LVL_INFO] =  OBELISK_LOG_GREEN  "[INFO]"  OBELISK_LOG_RESET " ",
    [OBELISK_LOG_LVL_TRACE] = OBELISK_LOG_WHITE  "[TRACE]" OBELISK_LOG_RESET " "
};

void obeliskPrintf(FILE* const stream, ObeliskLogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stream, obeliskLogLevelStrings[level]);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
    va_end(args);
}

void obeliskLogAssertFailure(const char* expr, const char* msg, const char* file, uint32_t line) {
    obeliskPrintf(stderr, OBELISK_LOG_LVL_FATAL, "Assertion Failure: %s (%s) in file: %s, line: %d", msg, expr, file, line);
}
