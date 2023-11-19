#pragma once

#include "common.h"

#include "platform/platform.h"

typedef struct ObeliskApp ObeliskApp;

typedef b8   (*ObeliskLoadCB)    (ObeliskApp* app, u32 w, u32 h);
typedef void (*ObeliskDestroyCB) (ObeliskApp* app);

//typedef int  (*ObeliskEventCB)   (ObeliskApp* app, const ObeliskEvent* e);
typedef void (*ObeliskTickCB)    (ObeliskApp* app, f32 deltatime);

struct ObeliskApp
{
    ObeliskWindow* window;

    ObeliskLoadCB    on_load;
    ObeliskDestroyCB on_destroy;

    //ObeliskEventCB on_event;
    ObeliskTickCB  on_tick;

    u32 fps;

    // settings
    u8 debug : 1;
    u8 vsync : 1;
};

KAPI b8   obeliskLoad(ObeliskApp* app, const char* title, i32 x, i32 y, u32 w, u32 h);
KAPI void obeliskDestroy(ObeliskApp* app);

KAPI void obeliskRun(ObeliskApp* app);
KAPI void obeliskClose(ObeliskApp* app);
