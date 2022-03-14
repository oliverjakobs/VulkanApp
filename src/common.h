#ifndef COMMON_H
#define COMMON_H

#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    VkImageView* swapChainViews;
    uint32_t swapChainImageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
} VulkanContext;


uint32_t clamp32(uint32_t val, uint32_t min, uint32_t max);
char* readSPIRV(const char* path, size_t* sizeptr);


#endif // !COMMON_H
