#include "Event.h"

#include "Application.h"

#define MINIMAL_LOWORD(dw) ((uint16_t)(dw))
#define MINIMAL_HIWORD(dw) ((uint16_t)(((uint32_t)(dw)) >> 16))

struct MinimalEvent {
    uint32_t type;
    uint32_t uParam;
    int32_t lParam;
    int32_t rParam;
};

void MinimalDispatchEvent(MinimalApp* app, uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam) {
    MinimalEvent e = { .type = type, .uParam = uParam, .lParam = lParam, .rParam = rParam };
    if (app->on_event) app->on_event(app, &e);
}

int MinimalCheckEventType(const MinimalEvent* e, uint32_t type) { return e->type == type; }

int MinimalEventWindowSize(const MinimalEvent* e, float* w, float* h) {
    if (e->type != MINIMAL_EVENT_WINDOW_SIZE) return 0;

    if (w) *w = (float)e->lParam;
    if (h) *h = (float)e->rParam;

    return 1;
}

int32_t MinimalEventMouseButton(const MinimalEvent* e, float* x, float* y) {
    if (e->type != MINIMAL_EVENT_MOUSE_BUTTON) return GLFW_KEY_UNKNOWN;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return MINIMAL_HIWORD(e->uParam);
}

int32_t MinimalEventMouseButtonPressed(const MinimalEvent* e, float* x, float* y) {
    int32_t buttoncode = MinimalEventMouseButton(e, x, y);
    return (MINIMAL_LOWORD(e->uParam) == GLFW_PRESS) ? buttoncode : GLFW_KEY_UNKNOWN;
}

int32_t MinimalEventMouseButtonReleased(const MinimalEvent* e, float* x, float* y) {
    int32_t buttoncode = MinimalEventMouseButton(e, x, y);
    return (MINIMAL_LOWORD(e->uParam) == GLFW_RELEASE) ? buttoncode : GLFW_KEY_UNKNOWN;
}

int MinimalEventMouseMoved(const MinimalEvent* e, float* x, float* y) {
    if (e->type != MINIMAL_EVENT_MOUSE_MOVED) return 0;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return 1;
}

int32_t MinimalEventKey(const MinimalEvent* e) {
    return (e->type == MINIMAL_EVENT_KEY) ? e->uParam : GLFW_KEY_UNKNOWN;
}

int32_t MinimalEventKeyPressed(const MinimalEvent* e) {
    return (e->type == MINIMAL_EVENT_KEY && e->lParam == GLFW_PRESS) ? e->uParam : GLFW_KEY_UNKNOWN;
}

int32_t MinimalEventKeyReleased(const MinimalEvent* e) {
    return (e->type == MINIMAL_EVENT_KEY && e->lParam == GLFW_RELEASE) ? e->uParam : GLFW_KEY_UNKNOWN;
}

char MinimalEventChar(const MinimalEvent* e) {
    return (e->type == MINIMAL_EVENT_CHAR) ? (char)e->uParam : '\0';
}