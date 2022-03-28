#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"

typedef struct {
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence fence;
} FrameInfo;

typedef struct {
    VkSwapchainKHR handle;

    VkImage* images;
    VkImageView* views;
    VkFramebuffer* framebuffers;
    uint32_t count;

    VkFormat format;
    VkExtent2D extent;
    VkRenderPass renderPass;

    FrameInfo frames[MAX_FRAMES_IN_FLIGHT];
} ObeliskSwapchain;

int createSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height);
int recreateSwapchain(ObeliskSwapchain* swapchain, GLFWwindow* window);
void destroySwapchain(ObeliskSwapchain* swapchain);

int createRenderPass(ObeliskSwapchain* swapchain);
int createFramebuffers(ObeliskSwapchain* swapchain);

int acquireSwapchainImage(ObeliskSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);
int submitFrame(ObeliskSwapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame);
int presentFrame(ObeliskSwapchain* swapchain, uint32_t imageIndex, uint32_t frame);

void commandBufferStart(VkCommandBuffer cmdBuffer, const ObeliskSwapchain* swapchain, uint32_t imageIndex);
void commandBufferEnd(VkCommandBuffer cmdBuffer);

int createDescriptorPool(VulkanContext* context);
int createDescriptorSets(VulkanContext* context);
void destroyDescriptorSets(VulkanContext* context);

int createSyncObjects(VulkanContext* context, ObeliskSwapchain* swapchain);
void destroySyncObjects(VulkanContext* context, ObeliskSwapchain* swapchain);

#endif // !SWAPCHAIN_H