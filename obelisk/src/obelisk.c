#include "obelisk.h"

u8 obeliskLoad(MinimalApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h)
{
    MinimalLoadCB on_load = app->on_load;
    app->on_load = NULL;

    if (!minimalLoad(app, title, x, y, w, h))
    {
        MINIMAL_CRITICAL("Failed to load minimal.");
        return MINIMAL_FAIL;
    }

    if (!ignisCreateContext("obelisk"))
    {
        MINIMAL_CRITICAL("Failed to create ignis context.");
        return MINIMAL_FAIL;
    }
    MINIMAL_INFO("Ignis context created successfully.");

    return (on_load) ? on_load(app, w, h) : MINIMAL_OK;
}

void obeliskDestroy(MinimalApp* app)
{
    ignisDestroyContext();

    minimalDestroy(app);
}

void obeliskRun(MinimalApp* app)
{
    minimalRun(app);
}