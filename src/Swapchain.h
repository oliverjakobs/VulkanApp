#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Device.h"

int createSwapchain(const VulkanContext* context, Swapchain* swapchain, uint32_t width, uint32_t height);
int recreateSwapchain(const VulkanContext* context, Swapchain* swapchain, GLFWwindow* window);
void destroySwapchain(const VulkanContext* context, Swapchain* swapchain);

int createRenderPass(const VulkanContext* context, Swapchain* swapchain);
int createFramebuffers(const VulkanContext* context, Swapchain* swapchain);

#endif // !SWAPCHAIN_H