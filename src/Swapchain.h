#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Device.h"

int createSwapChain(VulkanContext* context, uint32_t width, uint32_t height);
int recreateSwapChain(VulkanContext* context, GLFWwindow* window);
void destroySwapChain(VulkanContext* context);

int createSwapChainImages(VulkanContext* context);

int createFramebuffers(VulkanContext* context);

#endif // !SWAPCHAIN_H