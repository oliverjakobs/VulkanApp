#ifndef IGNIS_CORE_H
#define IGNIS_CORE_H

#include <vulkan/vulkan.h>

#include "common.h"

#define IGNIS_VK_PFN(instance, name) ((PFN_##name)vkGetInstanceProcAddr((instance), (#name)))

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
uint8_t ignisCreateContext(const char* name, const IgnisPlatform* platform);
void ignisDestroyContext();

typedef enum
{
    IGNIS_QUEUE_GRAPHICS,
    IGNIS_QUEUE_TRANSFER,
    IGNIS_QUEUE_COMPUTE,
    IGNIS_QUEUE_PRESENT,
    IGNIS_QUEUE_FAMILY_MAX_ENUM
} IgnisQueueFamily;

#define IGNIS_QUEUE_FLAG_GRAPHICS   0x0001
#define IGNIS_QUEUE_FLAG_TRANSFER   0x0002
#define IGNIS_QUEUE_FLAG_COMPUTE    0x0004
#define IGNIS_QUEUE_FLAG_PRESENT    0x0008

VkDeviceMemory ignisAllocateDeviceMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const VkAllocationCallbacks* allocator);

uint32_t ignisGetQueueFamilyIndex(IgnisQueueFamily family);

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device);

uint8_t ignisResize(uint32_t width, uint32_t height);

void ignisSetClearColor(float r, float g, float b, float a);
void ignisSetDepthStencil(float depth, uint32_t stencil);
void ignisSetViewport(float x, float y, float width, float height);
void ignisSetDepthRange(float nearVal, float farVal);
void ignisSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);

uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();


VkInstance       ignisGetVkInstance();
VkDevice         ignisGetVkDevice();
VkPhysicalDevice ignisGetVkPhysicalDevice();
VkRenderPass     ignisGetVkRenderPass();

VkCommandBuffer  ignisGetCommandBuffer();

const VkAllocationCallbacks* ignisGetAllocator();


#endif /* IGNIS_CORE_H */