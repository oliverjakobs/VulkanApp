#ifndef IGNIS_TYPES_H
#define IGNIS_TYPES_H

#include "ignis.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                          \
    {                                           \
        MINIMAL_ASSERT(expr == VK_SUCCESS, ""); \
    }

#define VK_EXT_PFN(instance, name) (PFN_##name)vkGetInstanceProcAddr((instance), #name)

typedef struct
{
    VkPhysicalDevice physical;
    VkDevice handle;
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

#endif /* IGNIS_TYPES_H */