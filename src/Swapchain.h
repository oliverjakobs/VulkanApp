#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Device.h"

int createSwapchain(const VulkanContext* context, Swapchain* swapchain, uint32_t width, uint32_t height);
int recreateSwapchain(const VulkanContext* context, Swapchain* swapchain, GLFWwindow* window);
void destroySwapchain(const VulkanContext* context, Swapchain* swapchain);

int createRenderPass(const VulkanContext* context, Swapchain* swapchain);
int createFramebuffers(const VulkanContext* context, Swapchain* swapchain);

int acquireSwapchainImage(const VulkanContext* context, Swapchain* swapchain, uint32_t frame, uint32_t* imageIndex);
int submitFrame(const VulkanContext* context, VkCommandBuffer cmdBuffer, uint32_t frame);
int presentFrame(const VulkanContext* context, Swapchain* swapchain, uint32_t imageIndex, uint32_t frame);

#endif // !SWAPCHAIN_H