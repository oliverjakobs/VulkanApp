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
} Swapchain;

int createSwapchain(const VulkanContext* context, Swapchain* swapchain, uint32_t width, uint32_t height);
int recreateSwapchain(const VulkanContext* context, Swapchain* swapchain, GLFWwindow* window);
void destroySwapchain(const VulkanContext* context, Swapchain* swapchain);

int createRenderPass(const VulkanContext* context, Swapchain* swapchain);
int createFramebuffers(const VulkanContext* context, Swapchain* swapchain);

int acquireSwapchainImage(const VulkanContext* context, Swapchain* swapchain, uint32_t frame, uint32_t* imageIndex);
int submitFrame(const VulkanContext* context, Swapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame);
int presentFrame(const VulkanContext* context, Swapchain* swapchain, uint32_t imageIndex, uint32_t frame);

int createCommandBuffer(VulkanContext* context);
void commandBufferStart(VkCommandBuffer cmdBuffer, const Swapchain* swapchain, uint32_t imageIndex);
void commandBufferEnd(VkCommandBuffer cmdBuffer);

int createDescriptorPool(VulkanContext* context);
int createDescriptorSets(VulkanContext* context);
void destroyDescriptorSets(VulkanContext* context);

int createSyncObjects(VulkanContext* context, Swapchain* swapchain);
void destroySyncObjects(VulkanContext* context, Swapchain* swapchain);

#endif // !SWAPCHAIN_H