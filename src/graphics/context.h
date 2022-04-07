#ifndef OBELISK_CORE_H
#define OBELISK_CORE_H

#include "../common.h"
#include "../platform/platform.h"

#include <vulkan/vulkan.h>

/* context */
int obeliskCreateContext(GLFWwindow* window, const char* app, int debug);
void obeliskDestroyContext();

VkDevice obeliskGetDevice();
VkPhysicalDevice obeliskGetPhysicalDevice();
VkSurfaceKHR obeliskGetSurface();
VkQueue obeliskGetGraphicsQueue();
VkQueue obeliskGetPresentQueue();
uint32_t obeliskGetQueueGraphicsFamilyIndex();
uint32_t obeliskGetQueuePresentFamilyIndex();

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities);

VkFormat obeliskGetPhysicalDeviceFormat(const VkFormat* candidates, uint32_t count, VkImageTiling tiling, VkFormatFeatureFlags features);

uint32_t obeliskFindMemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags properties);

void obeliskPrintInfo();

/* command buffers */
VkResult obeliskAllocateCommandBuffers(VkCommandBuffer* buffers, VkCommandBufferLevel level, uint32_t count);
void obeliskFreeCommandBuffers(const VkCommandBuffer* buffers, uint32_t count);

#endif // !OBELISK_CORE_H
