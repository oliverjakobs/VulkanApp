#ifndef OBELISK_SWAPCHAIN_H
#define OBELISK_SWAPCHAIN_H

#include "context.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkFormat depthFormat;
    VkExtent2D extent;

    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;

    VkImage* depthImages;
    VkImageView* depthImageViews;
    VkDeviceMemory* depthImageMemories;

    VkFramebuffer* framebuffers;
    VkRenderPass renderPass;

    /* frame data */
    VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
    VkFence fences[MAX_FRAMES_IN_FLIGHT];
} ObeliskSwapchain;

int obeliskCreateSwapchain(ObeliskSwapchain* swapchain, VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height);
int obeliskRecreateSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height);
void obeliskDestroySwapchain(ObeliskSwapchain* swapchain);

int obeliskCreateSyncObjects(ObeliskSwapchain* swapchain);
void obeliskDestroySyncObjects(ObeliskSwapchain* swapchain);

int obeliskAcquireSwapchainImage(ObeliskSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);

int obeliskSubmitFrame(ObeliskSwapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame);
int obeliskPresentFrame(ObeliskSwapchain* swapchain, uint32_t imageIndex, uint32_t frame);

#endif // !OBELISK_SWAPCHAIN_H