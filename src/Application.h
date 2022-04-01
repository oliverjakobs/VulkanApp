#ifndef MINIMAL_APPLICATION_H
#define MINIMAL_APPLICATION_H

#include "common.h"
#include "Event.h"
#include "Input.h"

#include "Renderer.h"

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

uint32_t MinimalGetFps(const MinimalApp* app);

/* --------------------------| minimal app |----------------------------- */
typedef int  (*MinimalLoadCB)    (MinimalApp* app, uint32_t w, uint32_t h);
typedef void (*MinimalDestroyCB) (MinimalApp* app);

typedef int  (*MinimalEventCB)   (MinimalApp* app, const MinimalEvent* e);
typedef void (*MinimalUpdateCB)  (MinimalApp* app, VkCommandBuffer cmdBuffer, float deltatime);

struct MinimalApp {
    GLFWwindow* window;
    ObeliskRenderer renderer;

    MinimalLoadCB    on_load;
    MinimalDestroyCB on_destroy;

    MinimalEventCB  on_event;
    MinimalUpdateCB on_update;

    int debug;
    int vsync;
    int inconified;

    MinimalTimer timer;
};

int MinimalLoad(MinimalApp* app, const char* title, uint32_t w, uint32_t h);
void MinimalDestroy(MinimalApp* app);

void MinimalRun(MinimalApp* app);
void MinimalClose(MinimalApp* app);

/* --------------------------| settings |-------------------------------- */
void MinimalSetWindowTitle(MinimalApp* app, const char* title);

void MinimalEnableDebug(MinimalApp* app, int b);
void MinimalEnableVsync(MinimalApp* app, int b);

void MinimalToggleDebug(MinimalApp* app);
void MinimalToggleVsync(MinimalApp* app);

#endif // !MINIMAL_APPLICATION_H
