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
typedef uint8_t (*MinimalEventCB)(void* context, const MinimalEvent* e);
void minimalSetEventHandler(void* context, MinimalEventCB callback);

void minimalDispatchEvent(uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam);
void minimalDispatchExternalEvent(uint32_t type, const void* data);

/* Utility */
MINIMAL_API uint8_t minimalEventIsType(const MinimalEvent* e, uint32_t type);
MINIMAL_API uint8_t minimalEventIsExternal(const MinimalEvent* e);

MINIMAL_API const void* minimalExternalEvent(const MinimalEvent* e);

MINIMAL_API uint8_t minimalEventWindowSize(const MinimalEvent* e, uint32_t* w, uint32_t* h);

MINIMAL_API uint8_t minimalEventMouseButton(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y);
MINIMAL_API uint8_t minimalEventMouseButtonPressed(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y);
MINIMAL_API uint8_t minimalEventMouseButtonReleased(const MinimalEvent* e, MinimalMouseButton button, float* x, float* y);
MINIMAL_API uint16_t minimalEventMouseButtonAction(const MinimalEvent* e);

MINIMAL_API uint8_t minimalEventMouseMoved(const MinimalEvent* e, float* x, float* y);
MINIMAL_API uint8_t minimalEventMouseScrolled(const MinimalEvent* e, float* xoffset, float* yoffset);

MINIMAL_API MinimalKeycode minimalEventKey(const MinimalEvent* e);
MINIMAL_API MinimalKeycode minimalEventKeyPressed(const MinimalEvent* e);
MINIMAL_API MinimalKeycode minimalEventKeyReleased(const MinimalEvent* e);

MINIMAL_API char minimalEventChar(const MinimalEvent* e);

#endif /* !MINIMAL_EVENT_H */
