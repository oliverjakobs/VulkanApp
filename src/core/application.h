#ifndef OBELISK_APPLICATION_H
#define OBELISK_APPLICATION_H

#include "../common.h"

#include "event.h"
#include "input.h"

#include "../utility/time.h"
#include "../graphics/Renderer.h"

/* --------------------------| minimal app |----------------------------- */
typedef int  (*ObeliskLoadCB)    (ObeliskApp* app, uint32_t w, uint32_t h);
typedef void (*ObeliskDestroyCB) (ObeliskApp* app);

typedef int  (*ObeliskEventCB)   (ObeliskApp* app, const ObeliskEvent* e);
typedef void (*ObeliskUpdateCB)  (ObeliskApp* app, VkCommandBuffer cmdBuffer, float deltatime);

struct ObeliskApp {
    GLFWwindow* window;
    ObeliskRenderer renderer;

    ObeliskLoadCB    on_load;
    ObeliskDestroyCB on_destroy;

    ObeliskEventCB  on_event;
    ObeliskUpdateCB on_update;

    int inconified;

    ObeliskTimer timer;
};

int obeliskLoad(ObeliskApp* app, const char* title, uint32_t w, uint32_t h);
void obeliskDestroy(ObeliskApp* app);

void obeliskRun(ObeliskApp* app);
void obeliskClose(ObeliskApp* app);

uint32_t obeliskGetFps(const ObeliskApp* app);

/* --------------------------| settings |-------------------------------- */
void obeliskSetWindowTitle(ObeliskApp* app, const char* title);

#endif // !OBELISK_APPLICATION_H
