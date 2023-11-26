#ifndef MINIMAL_PLATFORM_H
#define MINIMAL_PLATFORM_H

#include "common.h"

u8 minimalPlatformInit();
u8 minimalPlatformTerminate();

MinimalWindow* minimalCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h);
void minimalDestroyWindow(MinimalWindow* window);

void minimalSetWindowTitle(MinimalWindow* context, const char* title);

void  minimalSetWindowEventHandler(MinimalWindow* window, void* handler);
void* minimalGetWindowEventHandler(MinimalWindow* window);

void minimalMakeContextCurrent(MinimalWindow* context);
MinimalWindow* minimalGetCurrentContext();

void minimalPollWindowEvents(MinimalWindow* context);

u8   minimalShouldClose(const MinimalWindow* context);
void minimalCloseWindow(MinimalWindow* context);

double minimalGetTime();

i8 minimalGetKeyState(const MinimalWindow* context, u32 keycode);
i8 minimalGetMouseButtonState(const MinimalWindow* context, u32 button);

void minimalGetCursorPos(const MinimalWindow* context, f32* x, f32* y);

#endif // !MINIMAL_PLATFORM_H
