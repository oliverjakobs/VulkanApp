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

typedef enum
{
    IGNIS_QUEUE_GRAPHICS_BIT = 0x0001,
    IGNIS_QUEUE_TRANSFER_BIT = 0x0002,
    IGNIS_QUEUE_COMPUTE_BIT  = 0x0004,
    IGNIS_QUEUE_PRESENT_BIT  = 0x0008
} ignisQueueFamilyBits;

VkDeviceMemory ignisAllocateDeviceMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const VkAllocationCallbacks* allocator);

uint32_t ignisGetQueueFamilyIndex(IgnisQueueFamily family);

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


void ignisPrintInfo();

#endif /* IGNIS_CORE_H */