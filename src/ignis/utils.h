#ifndef IGNIS_UTILS_H
#define IGNIS_UTILS_H

#include "common.h"

#include <vulkan/vulkan.h>

#define IGNIS_VK_PFN(instance, name) ((PFN_##name)vkGetInstanceProcAddr((instance), (#name)))

VkSurfaceFormatKHR ignisChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface);
VkPresentModeKHR ignisChoosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface);

VkFormat ignisQueryDepthFormat(VkPhysicalDevice device);

uint8_t ignisCheckValidationLayerSupport(const char* const* layers, uint32_t count);

uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device, const char* const* extensions, uint32_t count);



VkExtent2D ignisClampExtent2D(VkExtent2D extent, VkExtent2D min, VkExtent2D max);

#endif // !IGNIS_UTILS_H
