#ifndef OBELISK_CORE_H
#define OBELISK_CORE_H

#include "../../platform/platform.h"

#include <vulkan/vulkan.h>

/* device */
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

typedef struct {
    VkDevice handle;
    VkPhysicalDevice physical;

    uint32_t queueFamiliesSet;
    uint32_t queueFamilyIndices[OBELISK_QUEUE_COUNT];
    VkQueue queues[OBELISK_QUEUE_COUNT];

    VkCommandPool commandPool;
} ObeliskDevice;

int obeliskPickPhysicalDevice(ObeliskDevice* device, VkInstance instance, VkSurfaceKHR surface);
int obeliskCreateLogicalDevice(ObeliskDevice* device);

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities);
VkFormat obeliskGetPhysicalDeviceFormat(const VkFormat* candidates, uint32_t count, VkImageTiling tiling, VkFormatFeatureFlags features);
uint32_t obeliskFindPhysicalDeviceMemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags properties);

void obeliskPrintDeviceInfo();

/* context */
typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;

    VkAllocationCallbacks* allocator;
} ObeliskContext;

int obeliskCreateContext(GLFWwindow* window, const char* app, int debug);
void obeliskDestroyContext();



VkDevice obeliskGetDevice();
VkPhysicalDevice obeliskGetPhysicalDevice();
VkSurfaceKHR obeliskGetSurface();
VkQueue obeliskGetGraphicsQueue();
VkQueue obeliskGetPresentQueue();
uint32_t obeliskGetQueueGraphicsFamilyIndex();
uint32_t obeliskGetQueuePresentFamilyIndex();

/* command buffers */
VkResult obeliskAllocateCommandBuffers(VkCommandBuffer* buffers, VkCommandBufferLevel level, uint32_t count);
void obeliskFreeCommandBuffers(const VkCommandBuffer* buffers, uint32_t count);

#endif // !OBELISK_CORE_H
