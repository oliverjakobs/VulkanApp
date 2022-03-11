#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

int QueueFamilyIndicesComplete(QueueFamilyIndices indices);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, QueueFamilyIndices* indices);

VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices);

#endif // !IGNIS_DEVICE_H