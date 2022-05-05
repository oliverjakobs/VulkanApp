#ifndef OBELISK_PLATFORM_H
#define OBELISK_PLATFORM_H

#include "../common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GLFWwindow* obeliskInitPlatform(const char* title, uint32_t w, uint32_t h, void* userdata);
void obeliskTerminatePlatform(GLFWwindow* window);

VkResult obeliskCreateWindowSurface(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface);

#endif // !OBELISK_PLATFORM_H
