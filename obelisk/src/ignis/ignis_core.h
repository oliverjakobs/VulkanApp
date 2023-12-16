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


typedef VkResult (*ignisCreateSurfaceFn)(VkInstance, const void*, const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef struct
{
    ignisCreateSurfaceFn create_surface;
    const void* context;
} IgnisPlatform;

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
} IgnisContext;

uint8_t ignisCreateContext(IgnisContext* context, const char* name, const IgnisPlatform* platform);
void ignisDestroyContext(IgnisContext* context);

uint8_t ignisPickPhysicalDevice(IgnisContext* context);
uint8_t ignisCreateLogicalDevice(IgnisContext* context);

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device);

#ifdef IGNIS_DEBUG

void ignisFillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* info);

VkResult ignisCreateDebugMessenger(IgnisContext* context);
void ignisDestroyDebugMessenger(IgnisContext* context);

#endif


void* ignisAlloc(size_t size);
void  ignisFree(void* block, size_t size);


#endif /* IGNIS_CORE_H */