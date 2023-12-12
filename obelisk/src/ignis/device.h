#ifndef IGNIS_DEVICE_H
#define IGNIS_DEVICE_H

#include "types.h"

uint8_t ignisCreateDevice(IgnisContext* context);
void ignisDestroyDevice(IgnisContext* context);

void ignisQueryDeviceSwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device);

#endif /* !IGNIS_DEVICE_H */