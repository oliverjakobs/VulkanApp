#ifndef OBELISK_H
#define OBELISK_H

#include <minimal/minimal.h>

typedef struct ObeliskApp ObeliskApp;

typedef u8   (*ObeliskLoadCB)    (ObeliskApp* app, u32 w, u32 h);
typedef void (*ObeliskDestroyCB) (ObeliskApp* app);

typedef u8   (*ObeliskEventCB)   (ObeliskApp* app, const MinimalEvent* e);
typedef void (*ObeliskTickCB)    (ObeliskApp* app, f32 deltatime);

struct ObeliskApp
{
    MinimalWindow* window;
    ObeliskLoadCB    on_load;
    ObeliskDestroyCB on_destroy;

    ObeliskEventCB on_event;
    ObeliskTickCB  on_tick;
};

MINIMAL_API u8 obeliskLoad(ObeliskApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h);
MINIMAL_API void obeliskDestroy(ObeliskApp* app);

MINIMAL_API void obeliskRun(ObeliskApp* app);

#endif /* !OBELISK_H */