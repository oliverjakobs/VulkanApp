#ifndef IGNIS_CORE_H
#define IGNIS_CORE_H

#include <vulkan/vulkan.h>

#include "common.h"

const VkAllocationCallbacks* ignisGetAllocator();

typedef struct IgnisContext IgnisContext;

/* --------------------------| device |---------------------------------- */
typedef enum
{
    IGNIS_QUEUE_GRAPHICS,
    IGNIS_QUEUE_TRANSFER,
    IGNIS_QUEUE_COMPUTE,
    IGNIS_QUEUE_PRESENT,
    IGNIS_QUEUE_MAX_ENUM
} IgnisQueueFamilyIndex;

#define IGNIS_QUEUE_FLAG_GRAPHICS   0x0001
#define IGNIS_QUEUE_FLAG_TRANSFER   0x0002
#define IGNIS_QUEUE_FLAG_COMPUTE    0x0004
#define IGNIS_QUEUE_FLAG_PRESENT    0x0008

typedef struct
{
    VkPhysicalDevice physical;
    VkDevice handle;

    uint32_t queueFamiliesSet;
    uint32_t queueFamilyIndices[IGNIS_QUEUE_MAX_ENUM];

    VkQueue queues[IGNIS_QUEUE_MAX_ENUM];

    VkCommandPool commandPool;
} IgnisDevice;

uint8_t ignisCreateDevice(VkInstance instance, VkSurfaceKHR surface, IgnisDevice* device);
void ignisDestroyDevice(IgnisDevice* device);

VkFormat ignisQueryDeviceDepthFormat(VkPhysicalDevice device);
int32_t ignisFindMemoryTypeIndex(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties);

VkResult ignisAllocCmdBuffers();
void ignisFreeCmdBuffers();

void ignisPrintDeviceInfo(const IgnisDevice* device);

/* --------------------------| swapchain |------------------------------- */
typedef struct
{
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkFormat depthFormat;
    VkExtent2D extent;

    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;

    VkImage*        depthImages;
    VkImageView*    depthImageViews;
    VkDeviceMemory* depthImageMemories;
} IgnisSwapchain;

uint8_t ignisCreateSwapchain(const IgnisDevice* device, VkSurfaceKHR surface, VkSwapchainKHR old, uint32_t w, uint32_t h, IgnisSwapchain* swapchain);
void ignisDestroySwapchain(const IgnisDevice* device, IgnisSwapchain* swapchain);

/* --------------------------| render pass |----------------------------- */
typedef struct
{
    VkRenderPass handle;

    struct { float r,g,b,a; } clearColor;
} IgnisRenderPass;

uint8_t ignisCreateRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat, IgnisRenderPass* renderPass);
void ignisDestroyRenderPass(VkDevice device, IgnisRenderPass* renderPass);

/* --------------------------| pipeline |-------------------------------- */

uint8_t ignisCreatePipeline(const char* frag, const char* vert);

/* --------------------------| platform |-------------------------------- */
typedef VkResult (*ignisCreateSurfaceFn)(VkInstance, const void*, const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef const char* const*(*ignisQueryExtensionFn)(uint32_t*);
typedef struct
{
    ignisCreateSurfaceFn createSurface;
    ignisQueryExtensionFn queryExtensions;
    const void* context;
} IgnisPlatform;

/* --------------------------| context |--------------------------------- */
struct IgnisContext
{
    VkInstance instance;
    VkSurfaceKHR surface;

#ifdef IGNIS_DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    IgnisDevice device;
    IgnisSwapchain swapchain;
    
    IgnisRenderPass renderPass;
};

uint8_t ignisCreateContext(IgnisContext* context, const char* name, const IgnisPlatform* platform);
void ignisDestroyContext(IgnisContext* context);




#endif /* IGNIS_CORE_H */