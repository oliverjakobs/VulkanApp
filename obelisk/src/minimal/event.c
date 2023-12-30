#include "event.h"

#include "minimal.h"

#define MINIMAL_LOWORD(dw) ((u16)(dw))
#define MINIMAL_HIWORD(dw) ((u16)(((u32)(dw)) >> 16))

struct MinimalEvent
{
    u32 type;
    union
    {
        struct
        {
            u32 uParam;
            i32 lParam;
            i32 rParam;
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

void minimalDispatchEvent(u32 type, u32 uParam, i32 lParam, i32 rParam)
{
    MinimalEvent e = { .type = type, .uParam = uParam, .lParam = lParam, .rParam = rParam };
    if (event_handler.callback) event_handler.callback(event_handler.context, &e);
}

void minimalDispatchExternalEvent(u32 type, const void* data)
{
    MinimalEvent e = { .type = type, .external = data };
    if (event_handler.callback) event_handler.callback(event_handler.context, &e);
}

u8 minimalEventIsType(const MinimalEvent* e, u32 type)  { return e->type == type; }
u8 minimalEventIsExternal(const MinimalEvent* e)        { return e->type > MINIMAL_EVENT_LAST; }

const void* minimalExternalEvent(const MinimalEvent* e)
{
    return minimalEventIsExternal(e) ? e->external : NULL;
}

u8 minimalEventWindowSize(const MinimalEvent* e, u32* w, u32* h)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_WINDOW_SIZE)) return 0;

    if (w) *w = (u32)e->lParam;
    if (h) *h = (u32)e->rParam;

    return 1;
}

u8 minimalEventMouseButton(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_BUTTON)) return 0;

    if (button != (MinimalMouseButton)MINIMAL_HIWORD(e->uParam)) return 0;

    if (x) *x = (f32)e->lParam;
    if (y) *y = (f32)e->rParam;

    return 1;
}

u8 minimalEventMouseButtonPressed(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y)
{
    return minimalEventMouseButton(e, button, x, y) && MINIMAL_LOWORD(e->uParam) == MINIMAL_PRESS;
}

u8 minimalEventMouseButtonReleased(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y)
{
    return minimalEventMouseButton(e, button, x, y) && MINIMAL_LOWORD(e->uParam) == MINIMAL_RELEASE;
}

u16 minimalEventMouseButtonAction(const MinimalEvent* e)
{
    return MINIMAL_LOWORD(e->uParam);
}

u8 minimalEventMouseMoved(const MinimalEvent* e, f32* x, f32* y)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_MOVED)) return 0;

    if (x) *x = (f32)e->lParam;
    if (y) *y = (f32)e->rParam;

    return 1;
}

u8 minimalEventMouseScrolled(const MinimalEvent* e, f32* xoffset, f32* yoffset)
{
    if (!minimalEventIsType(e, MINIMAL_EVENT_MOUSE_SCROLLED)) return 0;

    if (xoffset) *xoffset = (f32)e->lParam;
    if (yoffset) *yoffset = (f32)e->rParam;

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
