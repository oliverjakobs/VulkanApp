#include "application.h"

#include "../platform/platform.h"

#include <string.h>

/* --------------------------| minimal app |----------------------------- */
int obeliskLoad(ObeliskApp* app, const char* title, uint32_t w, uint32_t h) {
    app->window = obeliskInitPlatform(title, w, h, app);
    if (!app->window) {
        OBELISK_ERROR("[GLFW] Failed to initialize GLFW.");
        return OBELISK_FAIL;
    }

    app->inconified = 0;

    obeliskResetTimer(&app->timer);

    return (app->on_load) ? app->on_load(app, w, h) : OBELISK_OK;
}

void obeliskDestroy(ObeliskApp* app) {
    if (app->on_destroy) app->on_destroy(app);
    obeliskTerminatePlatform(app->window);
}

void obeliskRun(ObeliskApp* app) {
    OBELISK_ASSERT(app, "");
    OBELISK_ASSERT(app->on_update, "Update callback missing!");

    while (!glfwWindowShouldClose(app->window)) {
        obeliskStartTimer(&app->timer, glfwGetTime());
        glfwPollEvents();
        obeliskUpdateInput();

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
