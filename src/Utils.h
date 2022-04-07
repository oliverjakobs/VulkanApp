#ifndef OBELISK_UTILS
#define OBELISK_UTILS

#include "common.h"

#define OBELISK_ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

char* obeliskReadFile(const char* path, size_t* sizeptr);

/* --------------------------| timer |----------------------------------- */
typedef struct {
    uint32_t frames;
    uint32_t fps;

    double seconds;
    double deltatime;
    double lastframe;
} MinimalTimer;

void MinimalTimerReset(MinimalTimer* timer);

void MinimalTimerStart(MinimalTimer* timer, double seconds);
void MinimalTimerEnd(MinimalTimer* timer, double seconds);

#endif // !OBELISK_UTILS
