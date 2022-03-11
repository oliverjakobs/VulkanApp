#ifndef MINIMAL_EVENT_H
#define MINIMAL_EVENT_H

#include "Utils.h"

#define MINIMAL_EVENT_UNKOWN            0

/* window events*/
#define MINIMAL_EVENT_WINDOW_SIZE       1

/* key events */
#define MINIMAL_EVENT_KEY               10
#define MINIMAL_EVENT_CHAR              11

/* mouse events */
#define MINIMAL_EVENT_MOUSE_BUTTON      12
#define MINIMAL_EVENT_MOUSE_MOVED       13
#define MINIMAL_EVENT_MOUSE_SCROLLED    14

void MinimalDispatchEvent(MinimalApp* app, uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam);

/* Utility */
int MinimalCheckEventType(const MinimalEvent* e, uint32_t type);

int MinimalEventWindowSize(const MinimalEvent* e, float* w, float* h);

int32_t MinimalEventMouseButton(const MinimalEvent* e, float* x, float* y);
int32_t MinimalEventMouseButtonPressed(const MinimalEvent* e, float* x, float* y);
int32_t MinimalEventMouseButtonReleased(const MinimalEvent* e, float* x, float* y);
int MinimalEventMouseMoved(const MinimalEvent* e, float* x, float* y);

int32_t MinimalEventKey(const MinimalEvent* e);
int32_t MinimalEventKeyPressed(const MinimalEvent* e);
int32_t MinimalEventKeyReleased(const MinimalEvent* e);

char MinimalEventChar(const MinimalEvent* e);

#endif // !MINIMAL_EVENT_H
