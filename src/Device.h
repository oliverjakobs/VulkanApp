#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"
#include "Core.h"

int queueFamilyIndicesComplete(QueueFamilyIndices indices);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

int pickPhysicalDevice(VulkanContext* context);

int createLogicalDevice(VulkanContext* context);

void printPhysicalDevice(VkPhysicalDevice device);

#endif // !IGNIS_DEVICE_H