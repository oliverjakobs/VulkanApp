#include "minimal.h"

#include <stdio.h>
#include <string.h>

void minimalGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL) *major = MINIMAL_VERSION_MAJOR;
    if (minor != NULL) *minor = MINIMAL_VERSION_MINOR;
    if (rev != NULL)   *rev = MINIMAL_VERSION_REVISION;
}

#define MINIMAL_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define MINIMAL_MAKE_VERSION_STR(m, n, r) MINIMAL_CONCAT_VERSION(m, n, r)

const char* minimalGetVersionString()
{
    return MINIMAL_MAKE_VERSION_STR(MINIMAL_VERSION_MAJOR, MINIMAL_VERSION_MINOR, MINIMAL_VERSION_REVISION);
}

/* --------------------------| game loop |------------------------------- */
void minimalRun(MinimalWindow* window, MinimalTickCB on_tick, void* context)
{
    double seconds = 0.0;
    double lastframe = 0.0;
    uint32_t frames = 0;

    MinimalFrameData framedata = {0};

    while (!minimalShouldClose(window))
    {
        double time = minimalGetTime();
        framedata.deltatime = time - lastframe;
        lastframe = time;

        minimalUpdateInput();

        on_tick(context, &framedata);

        minimalPollWindowEvents(window);

        frames++;
        if ((minimalGetTime() - seconds) > 1.0)
        {
            seconds += 1.0;
            framedata.fps = frames;
            frames = 0;

            MINIMAL_TRACE("FPS: %d", framedata.fps);
        }
    }
}

/* --------------------------| context |--------------------------------- */
static MinimalWindow* _current_context;

void minimalSetCurrentContext(MinimalWindow* context) { _current_context = context; }
MinimalWindow* minimalGetCurrentContext()             { return _current_context; }

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

