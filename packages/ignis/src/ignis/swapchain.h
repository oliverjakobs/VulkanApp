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

    VkRenderPass renderPass;

    uint32_t imageCount;

    VkImage*        images;
    VkImageView*    imageViews;

    VkImage*        depthImages;
    VkImageView*    depthImageViews;
    VkDeviceMemory* depthImageMemories;

    VkFramebuffer* framebuffers;

    /* Sync objects */
    VkSemaphore imageAvailable[IGNIS_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[IGNIS_MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[IGNIS_MAX_FRAMES_IN_FLIGHT];
} IgnisSwapchain;

uint8_t ignisCreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkSwapchainKHR old, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);
void ignisDestroySwapchain(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);

uint8_t ignisRecreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);

uint8_t ignisCreateSwapchainSyncObjects(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);
void ignisDestroySwapchainSyncObjects(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain);

uint8_t ignisAcquireNextImage(VkDevice device, IgnisSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);

uint8_t ingisSubmitFrame(VkQueue graphics, VkCommandBuffer buffer, uint32_t frame, IgnisSwapchain* swapchain);
uint8_t ignisPresentFrame(VkQueue present, uint32_t imageIndex, uint32_t frame, IgnisSwapchain* swapchain);

#endif
