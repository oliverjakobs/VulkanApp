#ifndef OBELISK_CORE_H
#define OBELISK_CORE_H

#include "../../platform/platform.h"

#include <vulkan/vulkan.h>

typedef enum {
    OBELISK_QUEUE_GRAPHICS,
    OBELISK_QUEUE_PRESENT,
    OBELISK_QUEUE_COUNT
} ObeliskQueueFamilyIndex;

typedef enum {
    OBELISK_QUEUE_FLAG_NONE = 0,
    OBELISK_QUEUE_FLAG_GRAPHICS = 1 << 0,
    OBELISK_QUEUE_FLAG_PRESENT = 1 << 1,
    OBELISK_QUEUE_FLAG_ALL = OBELISK_QUEUE_FLAG_GRAPHICS | OBELISK_QUEUE_FLAG_PRESENT
} ObeliskQueueFamilyFlag;

/* context */
typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    uint32_t queueFamiliesSet;
    uint32_t queueFamilyIndices[OBELISK_QUEUE_COUNT];
    VkQueue queues[OBELISK_QUEUE_COUNT];

    VkCommandPool commandPool;

    VkAllocationCallbacks* allocator;
} ObeliskContext;

int obeliskCreateContext(GLFWwindow* window, const char* app, int debug);
void obeliskDestroyContext();

VkDevice         obeliskGetDevice();
VkPhysicalDevice obeliskGetPhysicalDevice();
VkSurfaceKHR     obeliskGetSurface();
VkQueue          obeliskGetGraphicsQueue();
VkQueue          obeliskGetPresentQueue();
uint32_t*        obeliskGetQueueFamilyIndices();
VkSharingMode    obeliskGetImageSharingMode();

/* device */
int obeliskPickPhysicalDevice(ObeliskContext* context);
int obeliskCreateLogicalDevice(ObeliskContext* context);

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities);
VkFormat obeliskGetPhysicalDeviceFormat(const VkFormat* candidates, uint32_t count, VkImageTiling tiling, VkFormatFeatureFlags features);
uint32_t obeliskFindPhysicalDeviceMemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags properties);

void obeliskPrintDeviceInfo();

/* command buffers */
VkResult obeliskAllocateCommandBuffers(VkCommandBuffer* buffers, VkCommandBufferLevel level, uint32_t count);
void obeliskFreeCommandBuffers(const VkCommandBuffer* buffers, uint32_t count);

#endif // !OBELISK_CORE_H
