#include "obelsik.h"

KAPI b8 obeliskLoad(ObeliskApp* app, const char* title, i32 x, i32 y, u32 w, u32 h)
{
    app->fps = 0;
    app->debug = 0;
    app->vsync = 1;

    if (!obeliskPlatformInit())
    {
        KERROR("[App] Failed to initialize platform");
        return FALSE;
    }

    /* creating the window */
    app->window = obeliskCreateWindow(title, x, y, w, h);
    if (!app->window)
    {
        KERROR("[App] Failed to create window");
        return FALSE;
    }

    return (app->on_load) ? app->on_load(app, w, h) : TRUE;
}

KAPI void obeliskDestroy(ObeliskApp* app)
{
    if (app->on_destroy) app->on_destroy(app);
    obeliskDestroyWindow(app->window);

    obeliskPlatformTerminate();
}

KAPI void obeliskRun(ObeliskApp* app)
{
    f64 seconds = 0.0;
    f64 deltatime = 0.0;
    f64 lastframe = 0.0;
    u32 frames = 0;

    while (!obeliskShouldClose(app->window))
    {
        f64 time = obeliskGetTime();
        deltatime = time - lastframe;
        lastframe = time;

        app->on_tick(app, (f32)deltatime);

        obeliskPollWindowEvents(app->window);

        frames++;
        if ((obeliskGetTime() - seconds) > 1.0)
        {
            seconds += 1.0;
            app->fps = frames;
            frames = 0;
        }
    }
}

KAPI void obeliskClose(ObeliskApp* app) { obeliskCloseWindow(app->window); }