#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

int queueFamilyIndicesComplete(QueueFamilyIndices indices);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

int pickPhysicalDevice(VulkanContext* context, QueueFamilyIndices* indices);

int createLogicalDevice(VulkanContext* context, QueueFamilyIndices indices);

#endif // !IGNIS_DEVICE_H