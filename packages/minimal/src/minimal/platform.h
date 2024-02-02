#ifndef MINIMAL_PLATFORM_H
#define MINIMAL_PLATFORM_H

#include "common.h"

uint8_t minimalPlatformInit();
uint8_t minimalPlatformTerminate();

MinimalWindow* minimalCreateWindow(const char* title, int32_t x, int32_t y, uint32_t w, uint32_t h);
void minimalDestroyWindow(MinimalWindow* window);

void minimalSetWindowTitle(MinimalWindow* window, const char* title);

void minimalPollWindowEvents(MinimalWindow* window);

uint8_t minimalShouldClose(const MinimalWindow* window);
void minimalClose(MinimalWindow* window);

double minimalGetTime();

#ifdef MINIMAL_VULKAN

#include <vulkan/vulkan.h>

VkResult minimalCreateWindowSurface(VkInstance instance, const MinimalWindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

const char* const *minimalQueryRequiredExtensions(uint32_t* count);

#endif

#endif // !MINIMAL_PLATFORM_H
