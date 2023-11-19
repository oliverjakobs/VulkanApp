#pragma once

#include "common.h"

typedef struct ObeliskWindow ObeliskWindow;

b8 obeliskPlatformInit();
b8 obeliskPlatformTerminate();

ObeliskWindow* obeliskCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h);
void obeliskDestroyWindow(ObeliskWindow* window);

void obeliskSetWindowTitle(ObeliskWindow* window, const char* title);

b8   obeliskShouldClose(const ObeliskWindow* context);
void obeliskCloseWindow(ObeliskWindow* context);

void obeliskPollWindowEvents(ObeliskWindow* context);

f64 obeliskGetTime();


void platform_console_write(const char* message, u8 colour);
void platform_console_write_error(const char* message, u8 colour);


// Sleep on the thread for the provided ms. This blocks the main thread.
// Should only be used for giving time back to the OS for unused update power.
// Therefore it is not exported.
void platform_sleep(u64 ms);