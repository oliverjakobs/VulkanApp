#include "platform.h"

#include "../core/application.h"

static void obeliskGLFWErrorCallback(int error, const char* desc) {
    OBELISK_ERROR("[GLFW] (%d) %s", error, desc);
}

void obeliskGLFWWindowSizeCallback(GLFWwindow* window, int width, int height);
void obeliskGLFWWindowIconifyCallback(GLFWwindow* window, int iconified);
void obeliskGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height);

void obeliskGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void obeliskGLFWCharCallback(GLFWwindow* window, unsigned int keycode);
void obeliskGLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void obeliskGLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void obeliskGLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

int obeliskInitPlatform(ObeliskApp* app, const char* title, uint32_t w, uint32_t h) {
    if (!glfwInit()) return OBELISK_FAIL;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwSetErrorCallback(obeliskGLFWErrorCallback);

    /* creating the window */
    app->window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!app->window) return OBELISK_FAIL;

    glfwSetWindowUserPointer(app->window, app);

    /* set GLFW callbacks */
    glfwSetWindowSizeCallback(app->window,      obeliskGLFWWindowSizeCallback);
    glfwSetWindowIconifyCallback(app->window,   obeliskGLFWWindowIconifyCallback);
    glfwSetFramebufferSizeCallback(app->window, obeliskGLFWFramebufferSizeCallbak);
    glfwSetKeyCallback(app->window,             obeliskGLFWKeyCallback);
    glfwSetCharCallback(app->window,            obeliskGLFWCharCallback);
    glfwSetMouseButtonCallback(app->window,     obeliskGLFWMouseButtonCallback);
    glfwSetCursorPosCallback(app->window,       obeliskGLFWCursorPosCallback);
    glfwSetScrollCallback(app->window,          obeliskGLFWScrollCallback);

    return OBELISK_OK;
}

void obeliskTerminatePlatform(ObeliskApp* app) {
    if (app->window) glfwDestroyWindow(app->window);
    glfwTerminate();
}

/* --------------------------| glfw events |----------------------------- */
void obeliskGLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) obeliskDispatchEvent(app, OBELISK_EVENT_WINDOW_SIZE, 0, width, height);
}

void obeliskGLFWWindowIconifyCallback(GLFWwindow* window, int iconified) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) {
        app->inconified = iconified;
        obeliskDispatchEvent(app, OBELISK_EVENT_WINDOW_SIZE, (uint32_t)iconified, 0, 0);
    }
}

void obeliskGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) {
        obeliskDispatchEvent(app, OBELISK_EVENT_FRAMEBUFFER_SIZE, 0, width, height);
    }
}

void obeliskGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) obeliskDispatchEvent(app, OBELISK_EVENT_KEY, key, action, mods);
}

void obeliskGLFWCharCallback(GLFWwindow* window, unsigned int keycode) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) obeliskDispatchEvent(app, OBELISK_EVENT_CHAR, keycode, 0, 0);
}

void obeliskGLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        obeliskDispatchEvent(app, OBELISK_EVENT_MOUSE_BUTTON, (button << 16) + action, (int32_t)xpos, (int32_t)ypos);
    }
}

void obeliskGLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) obeliskDispatchEvent(app, OBELISK_EVENT_MOUSE_MOVED, 0, (int32_t)xpos, (int32_t)ypos);
}

void obeliskGLFWScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    ObeliskApp* app = (ObeliskApp*)glfwGetWindowUserPointer(window);
    if (app) obeliskDispatchEvent(app, OBELISK_EVENT_MOUSE_SCROLLED, 0, (int32_t)xOffset, (int32_t)yOffset);
}