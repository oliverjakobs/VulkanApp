#include "event.h"

#include "minimal.h"

#define MINIMAL_LOWORD(dw) ((uint16_t)(dw))
#define MINIMAL_HIWORD(dw) ((uint16_t)(((uint32_t)(dw)) >> 16))

struct MinimalEvent
{
    uint32_t type;
    union
    {
        struct
        {
            uint32_t uParam;
            int32_t lParam;
            int32_t rParam;
        };
        const void* external;
    };
};

static struct
{
    void* context;
    MinimalEventCB callback;
} event_handler;

void minimalSetEventHandler(void* context, MinimalEventCB callback)
{
    event_handler.context = context;
    event_handler.callback = callback;
}

void minimalDispatchEvent(uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam)
{
    MinimalEvent e = { .type = type, .uParam = uParam, .lParam = lParam, .rParam = rParam };
    if (event_handler.callback) event_handler.callback(event_handler.context, &e);
}

void minimalDispatchExternalEvent(uint32_t type, const void* data)
{
    MinimalEvent e = { .type = type, .external = data };
    if (event_handler.callback) event_handler.callback(event_handler.context, &e);
}

uint8_t minimalEventIsType(const MinimalEvent* e, uint32_t type)  { return e->type == type; }
uint8_t minimalEventIsExternal(const MinimalEvent* e)        { return e->type > MINIMAL_EVENT_LAST; }

const void* minimalExternalEvent(const MinimalEvent* e)
{
    return minimalEventIsExternal(e) ? e->external : NULL;
}

uint8_t minimalEventWindowSize(const MinimalEvent* e, uint32_t* w, uint32_t* h)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_WINDOW_SIZE)) return 0;

    if (w) *w = (uint32_t)e->lParam;
    if (h) *h = (uint32_t)e->rParam;

    return 1;
}

uint8_t minimalEventMouseButton(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_BUTTON)) return 0;

    if (button != (MinimalMouseButton)MINIMAL_HIWORD(e->uParam)) return 0;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return 1;
}

uint8_t minimalEventMouseButtonPressed(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y)
{
    return minimalEventMouseButton(e, button, x, y) && MINIMAL_LOWORD(e->uParam) == MINIMAL_PRESS;
}

uint8_t minimalEventMouseButtonReleased(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y)
{
    return minimalEventMouseButton(e, button, x, y) && MINIMAL_LOWORD(e->uParam) == MINIMAL_RELEASE;
}

uint16_t minimalEventMouseButtonAction(const MinimalEvent* e)
{
    return MINIMAL_LOWORD(e->uParam);
}

uint8_t minimalEventMouseMoved(const MinimalEvent* e, float* x, float* y)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_MOVED)) return 0;

    if (x) *x = (float)e->lParam;
    if (y) *y = (float)e->rParam;

    return 1;
}

uint8_t minimalEventMouseScrolled(const MinimalEvent* e, float* xoffset, float* yoffset)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_SCROLLED)) return 0;

    if (xoffset) *xoffset = (float)e->lParam;
    if (yoffset) *yoffset = (float)e->rParam;

    return 1;
}

MinimalKeycode minimalEventKey(const MinimalEvent* e)
{
    return (e->type == MINIMAL_EVENT_KEY) ? e->uParam : MINIMAL_KEY_UNKNOWN;
}

MinimalKeycode minimalEventKeyPressed(const MinimalEvent* e)
{
    return (e->type == MINIMAL_EVENT_KEY && e->lParam == MINIMAL_PRESS) ? e->uParam : MINIMAL_KEY_UNKNOWN;
}

MinimalKeycode minimalEventKeyReleased(const MinimalEvent* e)
{
    return (e->type == MINIMAL_EVENT_KEY && e->lParam == MINIMAL_RELEASE) ? e->uParam : MINIMAL_KEY_UNKNOWN;
}

char minimalEventChar(const MinimalEvent* e)
{
    return (e->type == MINIMAL_EVENT_CHAR) ? (char)e->uParam : '\0';
}
