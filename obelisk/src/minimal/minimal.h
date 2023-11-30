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

/* --------------------------| minimal app |----------------------------- */
typedef u8   (*MinimalLoadCB)    (MinimalApp* app, u32 w, u32 h);
typedef void (*MinimalDestroyCB) (MinimalApp* app);

typedef u8   (*MinimalEventCB)   (MinimalApp* app, const MinimalEvent* e);
typedef void (*MinimalTickCB)    (MinimalApp* app, f32 deltatime);

struct MinimalApp
{
    MinimalWindow* window;

    MinimalLoadCB    on_load;
    MinimalDestroyCB on_destroy;

    MinimalEventCB on_event;
    MinimalTickCB  on_tick;
};

MINIMAL_API u8   minimalLoad(MinimalApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h);
MINIMAL_API void minimalDestroy(MinimalApp* app);

/* --------------------------| game loop |------------------------------- */
MINIMAL_API void minimalRun(MinimalApp* app);
MINIMAL_API void minimalClose(MinimalApp* app);

/* --------------------------| context |--------------------------------- */
void minimalSetCurrentContext(MinimalApp* context);
MinimalApp* minimalGetCurrentContext();

#endif // !MINIMAL_H