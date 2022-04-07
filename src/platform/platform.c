#include "platform.h"

#include "../core/event.h"

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

GLFWwindow* obeliskInitPlatform(const char* title, uint32_t w, uint32_t h, void* userdata) {
    if (!glfwInit()) return NULL;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwSetErrorCallback(obeliskGLFWErrorCallback);

    /* creating the window */
    GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!window) return NULL;

    glfwSetWindowUserPointer(window, userdata);

    /* set GLFW callbacks */
    glfwSetWindowSizeCallback(window,       obeliskGLFWWindowSizeCallback);
    glfwSetWindowIconifyCallback(window,    obeliskGLFWWindowIconifyCallback);
    glfwSetFramebufferSizeCallback(window,  obeliskGLFWFramebufferSizeCallbak);
    glfwSetKeyCallback(window,              obeliskGLFWKeyCallback);
    glfwSetCharCallback(window,             obeliskGLFWCharCallback);
    glfwSetMouseButtonCallback(window,      obeliskGLFWMouseButtonCallback);
    glfwSetCursorPosCallback(window,        obeliskGLFWCursorPosCallback);
    glfwSetScrollCallback(window,           obeliskGLFWScrollCallback);

    return window;
}

void obeliskTerminatePlatform(GLFWwindow* window) {
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

/* --------------------------| glfw events |----------------------------- */
void obeliskGLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_WINDOW_SIZE, 0, width, height);
}

void obeliskGLFWWindowIconifyCallback(GLFWwindow* window, int iconified) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_WINDOW_ICONIFY, (uint32_t)iconified, 0, 0);
}

void obeliskGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_FRAMEBUFFER_SIZE, 0, width, height);
}

void obeliskGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_KEY, key, action, mods);
}

void obeliskGLFWCharCallback(GLFWwindow* window, unsigned int keycode) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_CHAR, keycode, 0, 0);
}

void obeliskGLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    void* userdata = glfwGetWindowUserPointer(window);
    obeliskDispatchEvent(userdata, OBELISK_EVENT_MOUSE_BUTTON, (button << 16) + action, (int32_t)xpos, (int32_t)ypos);
}

void obeliskGLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    obeliskDispatchEvent(glfwGetWindowUserPointer(window), OBELISK_EVENT_MOUSE_MOVED, 0, (int32_t)xpos, (int32_t)ypos);
}

void obeliskGLFWScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    void* userdata = glfwGetWindowUserPointer(window);
    obeliskDispatchEvent(userdata, OBELISK_EVENT_MOUSE_SCROLLED, 0, (int32_t)xOffset, (int32_t)yOffset);
}