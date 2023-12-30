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
uint32_t ignisFindMemoryTypeIndex(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties);

VkResult ignisAllocCommandBuffers(const IgnisDevice* device, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer* buffers);
void ignisFreeCommandBuffers(const IgnisDevice* device, uint32_t count, const VkCommandBuffer* buffers);

void ignisPrintDeviceInfo(const IgnisDevice* device);

/* --------------------------| swapchain |------------------------------- */
#define IGNIS_MAX_FRAMES_IN_FLIGHT 2

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

    VkRenderPass renderPass;

    VkFramebuffer* framebuffers;

    /* Sync objects */
    VkSemaphore imageAvailable[IGNIS_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[IGNIS_MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[IGNIS_MAX_FRAMES_IN_FLIGHT];
} IgnisSwapchain;

uint8_t ignisCreateSwapchain(const IgnisDevice* device, VkSurfaceKHR surface, VkSwapchainKHR old, uint32_t w, uint32_t h, IgnisSwapchain* swapchain);
void ignisDestroySwapchain(VkDevice device, IgnisSwapchain* swapchain);

uint8_t ignisRecreateSwapchain(const IgnisDevice* device, VkSurfaceKHR surface, uint32_t width, uint32_t height, IgnisSwapchain* swapchain);

uint8_t ignisCreateSwapchainSyncObjects(VkDevice device, IgnisSwapchain* swapchain);
void ignisDestroySwapchainSyncObjects(VkDevice device, IgnisSwapchain* swapchain);

uint8_t ignisAcquireSwapchainImage(VkDevice device, IgnisSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);

uint8_t ignisSubmitFrame(VkQueue graphics, IgnisSwapchain* swapchain, VkCommandBuffer commandBuffer, uint32_t frame);
uint8_t ignisPresentFrame(VkQueue present, IgnisSwapchain* swapchain, uint32_t imageIndex, uint32_t frame);

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

    // render data
    uint32_t currentFrame;
    uint32_t imageIndex;

    VkCommandBuffer commandBuffers[IGNIS_MAX_FRAMES_IN_FLIGHT];

    // state
    VkViewport viewport;
    VkClearColorValue clearColor;
    VkClearDepthStencilValue depthStencil;
};

uint8_t ignisCreateContext(IgnisContext* context, const char* name, const IgnisPlatform* platform);
void ignisDestroyContext(IgnisContext* context);




#endif /* IGNIS_CORE_H */