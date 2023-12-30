#ifndef MINIMAL_EVENT_H
#define MINIMAL_EVENT_H

#include "common.h"
#include "input.h"

#define MINIMAL_EVENT_UNKOWN            0

/* window events*/
#define MINIMAL_EVENT_WINDOW_SIZE       1
#define MINIMAL_EVENT_WINDOW_MINIMIZE   2
#define MINIMAL_EVENT_WINDOW_MAXIMIZE   3

/* key events */
#define MINIMAL_EVENT_KEY               10
#define MINIMAL_EVENT_CHAR              11

/* mouse events */
#define MINIMAL_EVENT_MOUSE_BUTTON      12
#define MINIMAL_EVENT_MOUSE_MOVED       13
#define MINIMAL_EVENT_MOUSE_SCROLLED    14

#define MINIMAL_EVENT_LAST              MINIMAL_EVENT_MOUSE_SCROLLED

/* Dispatch */
typedef u8 (*MinimalEventCB)(void* context, const MinimalEvent* e);
void minimalSetEventHandler(void* context, MinimalEventCB callback);

void minimalDispatchEvent(u32 type, u32 uParam, i32 lParam, i32 rParam);
void minimalDispatchExternalEvent(u32 type, const void* data);

/* Utility */
MINIMAL_API u8 minimalEventIsType(const MinimalEvent* e, u32 type);
MINIMAL_API u8 minimalEventIsExternal(const MinimalEvent* e);

MINIMAL_API const void* minimalExternalEvent(const MinimalEvent* e);

MINIMAL_API u8 minimalEventWindowSize(const MinimalEvent* e, u32* w, u32* h);

MINIMAL_API u8 minimalEventMouseButton(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y);
MINIMAL_API u8 minimalEventMouseButtonPressed(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y);
MINIMAL_API u8 minimalEventMouseButtonReleased(const MinimalEvent* e, MinimalMouseButton button, f32* x, f32* y);
MINIMAL_API u16 minimalEventMouseButtonAction(const MinimalEvent* e);

MINIMAL_API u8 minimalEventMouseMoved(const MinimalEvent* e, f32* x, f32* y);
MINIMAL_API u8 minimalEventMouseScrolled(const MinimalEvent* e, f32* xoffset, f32* yoffset);

MINIMAL_API MinimalKeycode minimalEventKey(const MinimalEvent* e);
MINIMAL_API MinimalKeycode minimalEventKeyPressed(const MinimalEvent* e);
MINIMAL_API MinimalKeycode minimalEventKeyReleased(const MinimalEvent* e);

MINIMAL_API char minimalEventChar(const MinimalEvent* e);

#endif /* !MINIMAL_EVENT_H */
