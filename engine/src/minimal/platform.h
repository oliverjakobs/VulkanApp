#ifndef MINIMAL_PLATFORM_H
#define MINIMAL_PLATFORM_H

#include "common.h"

u8 minimalPlatformInit();
u8 minimalPlatformTerminate();

MinimalWindow* minimalCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h);
void minimalDestroyWindow(MinimalWindow* window);

void minimalSetWindowTitle(MinimalWindow* window, const char* title);

void minimalPollWindowEvents(MinimalWindow* window);

u8   minimalShouldClose(const MinimalWindow* window);
void minimalCloseWindow(MinimalWindow* window);

double minimalGetTime();

i8 minimalGetKeyState(const MinimalWindow* window, u32 keycode);
i8 minimalGetMouseButtonState(const MinimalWindow* window, u32 button);

void minimalGetCursorPos(const MinimalWindow* window, f32* x, f32* y);

#endif // !MINIMAL_PLATFORM_H
