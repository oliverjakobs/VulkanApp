#include "obelisk.h"

#include "ignis/ignis.h"

u8 obeliskLoad(ObeliskApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h)
{
    /* minimal initialization */
    if (!minimalPlatformInit())
    {
        MINIMAL_ERROR("[App] Failed to initialize Minimal");
        return MINIMAL_FAIL;
    }

    /* creating the window */
    app->window = minimalCreateWindow(title, x, y, w, h);
    if (!app->window)
    {
        MINIMAL_ERROR("[App] Failed to create Minimal window");
        return MINIMAL_FAIL;
    }
    
    minimalSetCurrentContext(app->window);
    minimalSetEventHandler(app, (MinimalEventCB)app->on_event);
    
    IgnisPlatform platform = {
        .createSurface = (ignisCreateSurfaceFn)minimalCreateWindowSurface,
        .queryExtensions = (ignisQueryExtensionFn)minimalQueryRequiredExtensions,
        .context = app->window
    };

    if (!ignisInit("ObeliskApp", &platform))
    {
        MINIMAL_CRITICAL("Failed to create ignis context.");
        return MINIMAL_FAIL;
    }
    MINIMAL_INFO("Ignis context created successfully.");

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void obeliskDestroy(ObeliskApp* app)
{
    ignisTerminate();

    minimalDestroyWindow(app->window);
    minimalPlatformTerminate();
}

static void obeliskOnTick(ObeliskApp* app, const MinimalFrameData* framedata)
{
    app->on_tick(app, framedata->deltatime);
}

void obeliskRun(ObeliskApp* app)
{
    minimalRun(app->window, (MinimalTickCB)obeliskOnTick, app);
}