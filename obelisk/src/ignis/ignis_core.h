#ifndef IGNIS_CORE_H
#define IGNIS_CORE_H

#include <stdint.h>
#include <string.h>

#include <vulkan/vulkan.h>


#ifdef _DEBUG
    #define IGNIS_DEBUG
#endif

#define IGNIS_FAIL    0
#define IGNIS_OK      1

void* ignisAlloc(size_t size);
void  ignisFree(void* block, size_t size);

typedef struct IgnisContext IgnisContext;

/* --------------------------| queue families |-------------------------- */
typedef enum
{
    IGNIS_QUEUE_GRAPHICS,
    IGNIS_QUEUE_TRANSFER,
    IGNIS_QUEUE_COMPUTE,
    IGNIS_QUEUE_PRESENT,
    IGNIS_MAX_QUEUE_INDEX
} IgnisQueueFamilyIndex;

#define IGNIS_QUEUE_FLAG_GRAPHICS   0x0001
#define IGNIS_QUEUE_FLAG_TRANSFER   0x0002
#define IGNIS_QUEUE_FLAG_COMPUTE    0x0004
#define IGNIS_QUEUE_FLAG_PRESENT    0x0008

/* --------------------------| swapchain |------------------------------- */
typedef struct
{
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkExtent2D extent;

    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;
} IgnisSwapchain;

int ignisCreateSwapchain(IgnisContext* context, IgnisSwapchain* swapchain, VkSwapchainKHR old, uint32_t w, uint32_t h);
void ignisDestroySwapchain(IgnisContext* context, IgnisSwapchain* swapchain);

/* --------------------------| context |--------------------------------- */
struct IgnisContext
{
    VkInstance instance;
    VkAllocationCallbacks* allocator;

    VkSurfaceKHR surface;

#ifdef IGNIS_DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    VkPhysicalDevice physical_device;
    VkDevice device;

    uint32_t queue_families_set;
    uint32_t queue_family_indices[IGNIS_MAX_QUEUE_INDEX];

    VkQueue queues[IGNIS_MAX_QUEUE_INDEX];

    IgnisSwapchain swapchain;
};

uint8_t ignisCreateContext(IgnisContext* context, const char* name, const char* const *extensions, uint32_t count, const void* debug);
void ignisDestroyContext(IgnisContext* context);

/* --------------------------| device |---------------------------------- */
uint8_t ignisPickPhysicalDevice(IgnisContext* context);
uint8_t ignisCreateLogicalDevice(IgnisContext* context);

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device);

/* --------------------------| debug |----------------------------------- */
#ifdef IGNIS_DEBUG

void ignisPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* info);

VkResult ignisCreateDebugMessenger(VkInstance instance, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger);
void ignisDestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator);

#endif


#endif /* IGNIS_CORE_H */