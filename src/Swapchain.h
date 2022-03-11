#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"

typedef struct {
    VkImage* data;
    uint32_t count;
    VkFormat format;
    VkExtent2D extent;
} SwapChainImages;

int querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

int chooseSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* format);
int chooseSwapPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR* mode);

VkExtent2D getSwapChainExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h);

VkSwapchainKHR createSwapChain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface, uint32_t w, uint32_t h, SwapChainImages* images);

#endif // !SWAPCHAIN_H