#include "Application.h"

#include <string.h>

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

    while (!glfwWindowShouldClose(app->window)) {
        MinimalTimerStart(&app->timer, glfwGetTime());
        MinimalUpdateInput(app->window);

        app->on_update(app, (float)app->timer.deltatime);

        glfwPollEvents();

        MinimalTimerEnd(&app->timer, glfwGetTime());
    }

    vkDeviceWaitIdle(app->context.device);
}

void MinimalClose(MinimalApp* app) { glfwSetWindowShouldClose(app->window, GLFW_TRUE); }

/* --------------------------| settings |-------------------------------- */
void MinimalSetWindowTitle(MinimalApp* app, const char* title) { glfwSetWindowTitle(app->window, title); }

void MinimalEnableDebug(MinimalApp* app, int b) { app->debug = b; }
void MinimalEnableVsync(MinimalApp* app, int b) { glfwSwapInterval(b); app->vsync = b; }

void MinimalToggleDebug(MinimalApp* app) { MinimalEnableDebug(app, !app->debug); }
void MinimalToggleVsync(MinimalApp* app) { MinimalEnableVsync(app, !app->vsync); }

/* --------------------------| input |----------------------------------- */
typedef struct {
    int state;
    int prev;
} InputState;

static InputState key_states[GLFW_KEY_LAST + 1];
static InputState mouse_states[GLFW_MOUSE_BUTTON_LAST + 1];

void MinimalUpdateInput(GLFWwindow* context) {
    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i) {
        key_states[i].prev = key_states[i].state;
        key_states[i].state = (glfwGetKey(context, i) == GLFW_PRESS);
    }

    for (int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        mouse_states[i].prev = key_states[i].state;
        mouse_states[i].state = (glfwGetMouseButton(context, i) == GLFW_PRESS);
    }
}

int MinimalKeyPressed(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;

    int state = glfwGetKey(glfwGetCurrentContext(), keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

int MinimalKeyReleased(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;
    return glfwGetKey(glfwGetCurrentContext(), keycode) == GLFW_RELEASE;
}

int MinimalKeyHit(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].state && !key_states[keycode].prev;
}

int MinimalKeyDown(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].state;
}

int MinimalKeyUp(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].prev && !key_states[keycode].state;
}

int MinimalMousePressed(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
}

int MinimalMouseReleased(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_RELEASE;
}

int MinimalMouseHit(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].state && !mouse_states[button].prev;
}

int MinimalMouseDown(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].state;
}

int MinimalMouseUp(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].prev && !mouse_states[button].state;
}

void MinimalCursorPos(float* x, float* y) {
    double xpos, ypos;
    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

    if (x) *x = (float)xpos;
    if (y) *y = (float)ypos;
}

float MinimalCursorX() {
    float x;
    MinimalCursorPos(&x, NULL);
    return x;
}

float MinimalCursorY() {
    float y;
    MinimalCursorPos(NULL, &y);
    return y;
}


/* --------------------------| glfw events |----------------------------- */
void MinimalGLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) MinimalDispatchEvent(app, MINIMAL_EVENT_WINDOW_SIZE, 0, width, height);
}

void MinimalGLFWFramebufferSizeCallbak(GLFWwindow* window, int width, int height) {
    MinimalApp* app = (MinimalApp*)glfwGetWindowUserPointer(window);
    if (app) app->context.framebufferResized = 1;
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
