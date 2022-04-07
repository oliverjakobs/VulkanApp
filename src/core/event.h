#ifndef OBELISK_EVENT_H
#define OBELISK_EVENT_H

#include "../common.h"

#define OBELISK_EVENT_UNKOWN            0

/* window events*/
#define OBELISK_EVENT_WINDOW_SIZE       1
#define OBELISK_EVENT_WINDOW_ICONIFY    2
#define OBELISK_EVENT_FRAMEBUFFER_SIZE  3

/* key events */
#define OBELISK_EVENT_KEY               10
#define OBELISK_EVENT_CHAR              11

/* mouse events */
#define OBELISK_EVENT_MOUSE_BUTTON      12
#define OBELISK_EVENT_MOUSE_MOVED       13
#define OBELISK_EVENT_MOUSE_SCROLLED    14

void obeliskDispatchEvent(ObeliskApp* app, uint32_t type, uint32_t uParam, int32_t lParam, int32_t rParam);

/* Utility */
int obeliskCheckEventType(const ObeliskEvent* e, uint32_t type);

int obeliskEventWindowSize(const ObeliskEvent* e, uint32_t* w, uint32_t* h);
int obeliskEventWindowInconify(const ObeliskEvent* e);
int obeliskEventFramebufferSize(const ObeliskEvent* e, uint32_t* w, uint32_t* h);

int32_t obeliskEventMouseButton(const ObeliskEvent* e, float* x, float* y);
int32_t obeliskEventMouseButtonPressed(const ObeliskEvent* e, float* x, float* y);
int32_t obeliskEventMouseButtonReleased(const ObeliskEvent* e, float* x, float* y);
int obeliskEventMouseMoved(const ObeliskEvent* e, float* x, float* y);

int32_t obeliskEventKey(const ObeliskEvent* e);
int32_t obeliskEventKeyPressed(const ObeliskEvent* e);
int32_t obeliskEventKeyReleased(const ObeliskEvent* e);

char obeliskEventChar(const ObeliskEvent* e);

#endif // !OBELISK_EVENT_H
