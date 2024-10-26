#ifndef IGNIS_SWAPCHAIN_H
#define IGNIS_SWAPCHAIN_H

#include "ignis_core.h"

#define IGNIS_MAX_FRAMES_IN_FLIGHT 2

typedef struct
{
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkFormat depthFormat;
    VkExtent2D extent;

    uint32_t imageCount;

    VkImage*        images;
    VkImageView*    imageViews;

    VkImage*        depthImages;
    VkImageView*    depthImageViews;
    VkDeviceMemory* depthImageMemories;
} IgnisSwapchain;

uint8_t ignisCreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkSwapchainKHR old, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);
void ignisDestroySwapchain(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);

uint8_t ignisRecreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);


#endif
