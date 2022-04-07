#include "application.h"

#include "../platform/platform.h"

#include <string.h>

/* --------------------------| minimal app |----------------------------- */
int obeliskLoad(ObeliskApp* app, const char* title, uint32_t w, uint32_t h) {
    if (obeliskInitPlatform(app, title, w, h) != OBELISK_OK) {
        OBELISK_ERROR("[GLFW] Failed to initialize GLFW.");
        return OBELISK_FAIL;
    }

    /* apply settings */
    obeliskEnableDebug(app, app->debug);
    obeliskEnableVsync(app, app->vsync);

    app->inconified = 0;

    obeliskResetTimer(&app->timer);

    return (app->on_load) ? app->on_load(app, w, h) : OBELISK_OK;
}

void obeliskDestroy(ObeliskApp* app) {
    if (app->on_destroy) app->on_destroy(app);
    obeliskTerminatePlatform(app);
}

void obeliskRun(ObeliskApp* app) {
    OBELISK_ASSERT(app, "");
    OBELISK_ASSERT(app->on_update, "Update callback missing!");

    while (!glfwWindowShouldClose(app->window)) {
        obeliskStartTimer(&app->timer, glfwGetTime());
        obeliskUpdateInput(app->window);
        glfwPollEvents();

        if (app->inconified) continue;

        /* begin frame */
        VkCommandBuffer cmdBuffer = obeliskBeginFrame(&app->renderer);
        if (cmdBuffer == VK_NULL_HANDLE) break;

        app->on_update(app, cmdBuffer, (float)app->timer.deltatime);

        /* end frame */
        obeliskEndFrame(&app->renderer);
        obeliskEndTimer(&app->timer, glfwGetTime());
    }

    vkDeviceWaitIdle(obeliskGetDevice());
}

void obeliskClose(ObeliskApp* app) { glfwSetWindowShouldClose(app->window, GLFW_TRUE); }

uint32_t obeliskGetFps(const ObeliskApp* app) { return app->timer.fps; }

/* --------------------------| settings |-------------------------------- */
void obeliskSetWindowTitle(ObeliskApp* app, const char* title) { glfwSetWindowTitle(app->window, title); }

void obeliskEnableDebug(ObeliskApp* app, int b) { app->debug = b; }
void obeliskEnableVsync(ObeliskApp* app, int b) { app->vsync = b; }

void obeliskToggleDebug(ObeliskApp* app) { obeliskEnableDebug(app, !app->debug); }
void obeliskToggleVsync(ObeliskApp* app) { obeliskEnableVsync(app, !app->vsync); }
