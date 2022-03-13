#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"

VkExtent2D getSwapChainExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h);

int createSwapChain(VulkanContext* context, const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D extent);

int createSwapChainImages(VulkanContext* context);
void destroySwapChainImages(VulkanContext* context);

#endif // !SWAPCHAIN_H