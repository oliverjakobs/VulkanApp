#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Device.h"

VkExtent2D getSwapChainExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h);

int createSwapChain(VulkanContext* context, const VkSurfaceCapabilitiesKHR* capabilities, QueueFamilyIndices indices);

int createSwapChainImages(VulkanContext* context);
void destroySwapChainImages(VulkanContext* context);

int createFramebuffers(VulkanContext* context);
void destroyFramebuffers(VulkanContext* context);

#endif // !SWAPCHAIN_H