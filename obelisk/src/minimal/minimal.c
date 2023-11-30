#include "minimal.h"

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

/* --------------------------| minimal app |----------------------------- */
u8 minimalLoad(MinimalApp* app, const char* title, i32 x, i32 y, u32 w, u32 h)
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
    
    minimalSetCurrentContext(app);

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void minimalDestroy(MinimalApp* app)
{
    if (app->on_destroy) app->on_destroy(app);

    minimalDestroyWindow(app->window);
    minimalPlatformTerminate();
}

void minimalRun(MinimalApp* app)
{
    f64 seconds = 0.0;
    f64 deltatime = 0.0;
    f64 lastframe = 0.0;
    // u32 frames = 0;
    // u32 fps = 0;

    while (!minimalShouldClose(app->window))
    {
        f64 time = minimalGetTime();
        deltatime = time - lastframe;
        lastframe = time;

        minimalUpdateInput();

        app->on_tick(app, (float)deltatime);

        minimalPollWindowEvents(app->window);

        //frames++;
        if ((minimalGetTime() - seconds) > 1.0)
        {
            seconds += 1.0;
            // fps = frames;
            // frames = 0;
        }
    }
}

void minimalClose(MinimalApp* app) { minimalCloseWindow(app->window); }


/* --------------------------| context |--------------------------------- */
static MinimalApp* _current_context;

void minimalSetCurrentContext(MinimalApp* context) { _current_context = context; }
MinimalApp* minimalGetCurrentContext()             { return _current_context; }