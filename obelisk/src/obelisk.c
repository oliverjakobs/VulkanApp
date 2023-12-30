#include "obelisk.h"

#include "ignis/ignis.h"

static u8 obeliskOnEvent(ObeliskApp* app, const MinimalEvent* e);

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
    minimalSetEventHandler(app, (MinimalEventCB)obeliskOnEvent);
    
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
    MINIMAL_TRACE("Ignis context created successfully.");

    ignisSetClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void obeliskDestroy(ObeliskApp* app)
{
    ignisTerminate();
    MINIMAL_TRACE("Ignis context terminated.");

    minimalDestroyWindow(app->window);
    minimalPlatformTerminate();
}

u8 obeliskOnEvent(ObeliskApp* app, const MinimalEvent* e)
{
    u32 width, height;
    if (minimalEventWindowSize(e, &width, &height))
    {
        ignisResize(width, height);
    }

    return app->on_event(app, e);
}

static void obeliskOnTick(ObeliskApp* app, const MinimalFrameData* framedata)
{
    if (ignisBeginFrame())
    {
        app->on_tick(app, framedata->deltatime);

        ignisEndFrame();
    }
}

void obeliskRun(ObeliskApp* app)
{
    minimalRun(app->window, (MinimalTickCB)obeliskOnTick, app);
}