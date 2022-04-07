#ifndef OBELISK_PLATFORM_H
#define OBELISK_PLATFORM_H

#include "../common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int obeliskInitPlatform(ObeliskApp* app, const char* title, uint32_t w, uint32_t h);
void obeliskTerminatePlatform(ObeliskApp* app);

#endif // !OBELISK_PLATFORM_H
