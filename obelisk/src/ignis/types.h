#ifndef IGNIS_TYPES_H
#define IGNIS_TYPES_H

#include "ignis.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                          \
    {                                           \
        MINIMAL_ASSERT(expr == VK_SUCCESS, ""); \
    }

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

typedef struct
{
    VkPhysicalDevice physical;
    VkDevice handle;

    uint32_t queue_families_set;
    uint32_t queue_family_indices[IGNIS_MAX_QUEUE_INDEX];
} IgnisDevice;

typedef struct
{
    VkInstance instance;
    VkAllocationCallbacks* allocator;

    VkSurfaceKHR surface;

#ifdef IGNIS_DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    IgnisDevice device;
} IgnisContext;


#ifdef IGNIS_DEBUG

VkResult ignisCreateDebugUtilsMessenger(VkInstance instance, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger);
void ignisDestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator);

#endif

#endif /* IGNIS_TYPES_H */