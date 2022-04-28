#include "event.h"

#include "application.h"
#include "input.h"

#define OBELISK_LOWORD(dw) ((uint16_t)(dw))
#define OBELISK_HIWORD(dw) ((uint16_t)(((uint32_t)(dw)) >> 16))

struct ObeliskEvent {
    uint32_t type;
    uint32_t uParam;
    int32_t lParam;
    int32_t rParam;
};

void obeliskDispatchEvent(ObeliskApp* app, uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam) {
    OBELISK_ASSERT(app, "Can not dispatch event. App pointer missing.");

    /* handle internally */
    switch (type)
    {
    case OBELISK_EVENT_WINDOW_ICONIFY:
        app->inconified = uParam;
        break;
    case OBELISK_EVENT_KEY:
        if (!obeliskProcessKey((ObeliskKey)uParam, (uint8_t)lParam)) return;
        break;
    case OBELISK_EVENT_MOUSE_BUTTON:
        if (!obeliskProcessMouseButton((ObeliskMouseButton)OBELISK_HIWORD(uParam), (uint8_t)uParam)) return;
        break;
    case OBELISK_EVENT_MOUSE_MOVED:
        if (!obeliskProcessMouseMovement(lParam, rParam)) return;
        break;
    }

    /* dispatch to user space */
    if (app->on_event) {
        ObeliskEvent e = {
            .type = type,
            .uParam = uParam,
            .lParam = lParam,
            .rParam = rParam
        };
        app->on_event(app, &e);
    }
}

int obeliskCheckEventType(const ObeliskEvent* e, uint32_t type) { return e->type == type; }

int obeliskEventWindowSize(const ObeliskEvent* e, uint32_t* w, uint32_t* h) {
    if (e->type != OBELISK_EVENT_WINDOW_SIZE) return 0;

    if (w) *w = (uint32_t)e->lParam;
    if (h) *h = (uint32_t)e->rParam;

    return 1;
}

int obeliskEventFramebufferSize(const ObeliskEvent* e, uint32_t* w, uint32_t* h) {
    if (e->type != OBELISK_EVENT_FRAMEBUFFER_SIZE) return 0;

    if (w) *w = (uint32_t)e->lParam;
    if (h) *h = (uint32_t)e->rParam;

    return 1;
}

int obeliskEventWindowInconify(const ObeliskEvent* e) {
    return (e->type == OBELISK_EVENT_WINDOW_ICONIFY) ? e->uParam : -1;
}

int32_t obeliskEventMouseButton(const ObeliskEvent* e, float* x, float* y) {
    if (e->type != OBELISK_EVENT_MOUSE_BUTTON) return GLFW_KEY_UNKNOWN;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return OBELISK_HIWORD(e->uParam);
}

int32_t obeliskEventMouseButtonPressed(const ObeliskEvent* e, float* x, float* y) {
    int32_t buttoncode = obeliskEventMouseButton(e, x, y);
    return (OBELISK_LOWORD(e->uParam) == GLFW_PRESS) ? buttoncode : GLFW_KEY_UNKNOWN;
}

int32_t obeliskEventMouseButtonReleased(const ObeliskEvent* e, float* x, float* y) {
    int32_t buttoncode = obeliskEventMouseButton(e, x, y);
    return (OBELISK_LOWORD(e->uParam) == GLFW_RELEASE) ? buttoncode : GLFW_KEY_UNKNOWN;
}

int obeliskEventMouseMoved(const ObeliskEvent* e, float* x, float* y) {
    if (e->type != OBELISK_EVENT_MOUSE_MOVED) return 0;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return 1;
}

int32_t obeliskEventKey(const ObeliskEvent* e) {
    return (e->type == OBELISK_EVENT_KEY) ? e->uParam : GLFW_KEY_UNKNOWN;
}

int32_t obeliskEventKeyPressed(const ObeliskEvent* e) {
    return (e->type == OBELISK_EVENT_KEY && e->lParam == GLFW_PRESS) ? e->uParam : GLFW_KEY_UNKNOWN;
}

int32_t obeliskEventKeyReleased(const ObeliskEvent* e) {
    return (e->type == OBELISK_EVENT_KEY && e->lParam == GLFW_RELEASE) ? e->uParam : GLFW_KEY_UNKNOWN;
}

char obeliskEventChar(const ObeliskEvent* e) {
    return (e->type == OBELISK_EVENT_CHAR) ? (char)e->uParam : '\0';
}