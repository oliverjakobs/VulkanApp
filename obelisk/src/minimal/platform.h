#ifndef MINIMAL_PLATFORM_H
#define MINIMAL_PLATFORM_H

#include "common.h"

u8 minimalPlatformInit();
u8 minimalPlatformTerminate();

MinimalWindow* minimalCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h);
void minimalDestroyWindow(MinimalWindow* window);

MINIMAL_API void minimalSetWindowTitle(MinimalWindow* window, const char* title);

void minimalPollWindowEvents(MinimalWindow* window);

u8 minimalShouldClose(const MinimalWindow* window);
MINIMAL_API void minimalClose(MinimalWindow* window);

double minimalGetTime();

#ifdef MINIMAL_VULKAN

#include <vulkan/vulkan.h>

VkResult minimalCreateWindowSurface(VkInstance instance, const MinimalWindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

#endif

#endif // !MINIMAL_PLATFORM_H
