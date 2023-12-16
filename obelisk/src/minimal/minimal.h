#ifndef MINIMAL_H
#define MINIMAL_H

#include "common.h"
#include "platform.h"
#include "event.h"
#include "input.h"

/* minimal version numbers */
#define MINIMAL_VERSION_MAJOR       1
#define MINIMAL_VERSION_MINOR       0
#define MINIMAL_VERSION_REVISION    0

MINIMAL_API void minimalGetVersion(int* major, int* minor, int* rev);
MINIMAL_API const char* minimalGetVersionString();


/* --------------------------| game loop |------------------------------- */
typedef struct
{
    f32 deltatime;
    u32 fps;
} MinimalFrameData;

typedef void (*MinimalTickCB)(void* context, const MinimalFrameData*);
MINIMAL_API void minimalRun(MinimalWindow* window, MinimalTickCB on_tick, void* context);

MINIMAL_API void minimalClose(MinimalWindow* window);

/* --------------------------| context |--------------------------------- */
void minimalSetCurrentContext(MinimalWindow* context);
MinimalWindow* minimalGetCurrentContext();

#endif // !MINIMAL_H
