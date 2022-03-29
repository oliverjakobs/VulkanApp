#include "Application.h"

#include <string.h>

#include "Swapchain.h"

/* --------------------------| timer |----------------------------------- */
void MinimalTimerReset(MinimalTimer* timer) {
    timer->seconds = 0.0;
    timer->frames = 0;
    timer->fps = 0;

    timer->deltatime = 0.0;
    timer->lastframe = 0.0;
}

void MinimalTimerStart(MinimalTimer* timer, double seconds) {
    timer->deltatime = seconds - timer->lastframe;
    timer->lastframe = seconds;
}

void MinimalTimerEnd(MinimalTimer* timer, double seconds) {
    timer->frames++;
    if ((seconds - timer->seconds) > 1.0) {
        timer->seconds += 1.0;
        timer->fps = timer->frames;
        timer->frames = 0;
    }
}

uint32_t MinimalGetFps(const MinimalApp* app) { return app->timer.fps; }

/* --------------------------| minimal app |----------------------------- */
void MinimalGLFWErrorCallback(int error, const char* desc) {
    MINIMAL_ERROR("[GLFW] (%d) %s", error, desc);
}

void MinimalGLFWWindowSizeCallback(GLFWwindow* window, int width, int height);
void MinimalGLFWWindowIconifyCallback(GLFWwindow* window, int iconified);
void MinimalGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height);

void MinimalGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MinimalGLFWCharCallback(GLFWwindow* window, unsigned int keycode);
void MinimalGLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MinimalGLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void MinimalGLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

static int MinimalInitGlfw(MinimalApp* app, const char* title, uint32_t w, uint32_t h) {
    if (!glfwInit()) return MINIMAL_FAIL;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwSetErrorCallback(MinimalGLFWErrorCallback);

    /* creating the window */
    app->window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!app->window) return MINIMAL_FAIL;

    glfwSetWindowUserPointer(app->window, app);

    /* set GLFW callbacks */
    glfwSetWindowSizeCallback(app->window,      MinimalGLFWWindowSizeCallback);
    glfwSetWindowIconifyCallback(app->window,   MinimalGLFWWindowIconifyCallback);
    glfwSetFramebufferSizeCallback(app->window, MinimalGLFWFramebufferSizeCallbak);
    glfwSetKeyCallback(app->window,             MinimalGLFWKeyCallback);
    glfwSetCharCallback(app->window,            MinimalGLFWCharCallback);
    glfwSetMouseButtonCallback(app->window,     MinimalGLFWMouseButtonCallback);
    glfwSetCursorPosCallback(app->window,       MinimalGLFWCursorPosCallback);
    glfwSetScrollCallback(app->window,          MinimalGLFWScrollCallback);

    return MINIMAL_OK;
}

int MinimalLoad(MinimalApp* app, const char* title, uint32_t w, uint32_t h) {
    if (MinimalInitGlfw(app, title, w, h) != MINIMAL_OK) {
        MINIMAL_ERROR("[GLFW] Failed to initialize GLFW.");
        glfwTerminate();
        return MINIMAL_FAIL;
    }

    /* apply settings */
    MinimalEnableDebug(app, app->debug);
    MinimalEnableVsync(app, app->vsync);

    app->inconified = 0;

    MinimalTimerReset(&app->timer);

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void MinimalDestroy(MinimalApp* app) {
    if (app->on_destroy) app->on_destroy(app);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

void MinimalRun(MinimalApp* app) {
    MINIMAL_ASSERT(app, "");
    MINIMAL_ASSERT(app->on_update, "Update callback missing!");

    uint32_t frame = 0;
    while (!glfwWindowShouldClose(app->window)) {
        MinimalTimerStart(&app->timer, glfwGetTime());
        MinimalUpdateInput(app->window);
        glfwPollEvents();

        if (app->inconified) continue;

        /* acquire swap chain image */
        uint32_t imageIndex = 0;
        if (!acquireSwapchainImage(&app->swapchain, frame, &imageIndex)) {
            MINIMAL_ERROR("failed to acquire swap chain image!");
            continue;
        }

        /* start frame */
        VkCommandBuffer cmdBuffer = app->swapchain.commandBuffers[frame];
        commandBufferStart(cmdBuffer, &app->swapchain, imageIndex);

        app->on_update(app, cmdBuffer, frame, (float)app->timer.deltatime);

        /* end frame */
        commandBufferEnd(cmdBuffer);

        /* submit frame */
        if (!submitFrame(&app->swapchain, cmdBuffer, frame)) {
            MINIMAL_ERROR("failed to submit draw command buffer!");
            continue;
        }

        /* present frame */
        if (!presentFrame(&app->swapchain, imageIndex, frame)) {
            MINIMAL_ERROR("failed to present swap chain image!");
            continue;
        }

        /* next frame */
        frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;

        MinimalTimerEnd(&app->timer, glfwGetTime());
    }

    vkDeviceWaitIdle(obeliskGetDevice());
}

void MinimalClose(MinimalApp* app) { glfwSetWindowShouldClose(app->window, GLFW_TRUE); }

/* --------------------------| settings |-------------------------------- */
void MinimalSetWindowTitle(MinimalApp* app, const char* title) { glfwSetWindowTitle(app->window, title); }

void MinimalEnableDebug(MinimalApp* app, int b) { app->debug = b; }
void MinimalEnableVsync(MinimalApp* app, int b) { app->vsync = b; }

void MinimalToggleDebug(MinimalApp* app) { MinimalEnableDebug(app, !app->debug); }
void MinimalToggleVsync(MinimalApp* app) { MinimalEnableVsync(app, !app->vsync); }

/* --------------------------| glfw events |----------------------------- */
void MinimalGLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_WINDOW_SIZE, 0, width, height);
}

void MinimalGLFWWindowIconifyCallback(GLFWwindow* window, int iconified) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) {
        app->inconified = iconified;
        MinimalDispatchEvent(app, MINIMAL_EVENT_WINDOW_SIZE, (uint32_t)iconified, 0, 0);
    }
}

void MinimalGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) {
        MinimalDispatchEvent(app, MINIMAL_EVENT_FRAMEBUFFER_SIZE, 0, width, height);
    }
}

void MinimalGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_KEY, key, action, mods);
}

void MinimalGLFWCharCallback(GLFWwindow* window, unsigned int keycode) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_CHAR, keycode, 0, 0);
}

void MinimalGLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        MinimalDispatchEvent(app, MINIMAL_EVENT_MOUSE_BUTTON, (button << 16) + action, (int32_t)xpos, (int32_t)ypos);
    }
}

void MinimalGLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_MOUSE_MOVED, 0, (int32_t)xpos, (int32_t)ypos);
}

void MinimalGLFWScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_MOUSE_SCROLLED, 0, (int32_t)xOffset, (int32_t)yOffset);
}
