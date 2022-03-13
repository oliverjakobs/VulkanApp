#include "Swapchain.h"

#include "Device.h"
#include "Minimal/Utils.h"


static int chooseSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* format) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);

    VkSurfaceFormatKHR* formats;
    if (!count || !(formats = malloc(sizeof(VkSurfaceFormatKHR) * count))) return MINIMAL_FAIL;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    /* set fallback */
    *format = formats[0];

    for (uint32_t i = 0; i < count; ++i) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            *format = formats[i];
            break;
        }
    }

    free(formats);
    return MINIMAL_OK;
}

static int chooseSwapPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR* mode) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);

    VkPresentModeKHR* modes;
    if (!count || !(modes = malloc(sizeof(VkPresentModeKHR) * count))) return MINIMAL_FAIL;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes);

    /* set fallback */
    *mode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < count; ++i) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            *mode = modes[i];
            break;
        }
    }

    free(modes);
    return MINIMAL_OK;
}

VkExtent2D getSwapChainExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h) {
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D extent = {
        .width = clamp32(w, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = clamp32(h, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return extent;
}


int createSwapChain(VulkanContext* context, const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D extent) {
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!chooseSwapSurfaceFormat(context->physicalDevice, context->surface, &surfaceFormat))
        return MINIMAL_FAIL;

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode;
    if (!chooseSwapPresentMode(context->physicalDevice, context->surface, &presentMode))
        return MINIMAL_FAIL;

    uint32_t imageCount = capabilities->minImageCount + 1;
    if (capabilities->maxImageCount > 0 && imageCount > capabilities->maxImageCount) {
        imageCount = capabilities->maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context->surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(context->physicalDevice, context->surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = capabilities->currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapChain) != VK_SUCCESS)
        return MINIMAL_FAIL;

    context->swapChainImageCount = imageCount;
    context->swapChainImageFormat = surfaceFormat.format;
    context->swapChainExtent = extent;
    
    return MINIMAL_OK;
}

int createSwapChainImages(VulkanContext* context) {
    if (!context->swapChainImageCount) return MINIMAL_FAIL;

    context->swapChainImages = malloc(context->swapChainImageCount * sizeof(VkImage));
    context->swapChainViews = malloc(context->swapChainImageCount * sizeof(VkImageView));
    if (!context->swapChainImages || !context->swapChainViews) return MINIMAL_FAIL;

    vkGetSwapchainImagesKHR(context->device, context->swapChain, &context->swapChainImageCount, context->swapChainImages);

    for (size_t i = 0; i < context->swapChainImageCount; ++i) {
        VkImageViewCreateInfo createInfo = { 0 };
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = context->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = context->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(context->device, &createInfo, NULL, &context->swapChainViews[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}

void destroySwapChainImages(VulkanContext* context) {
    if (context->swapChainImages) free(context->swapChainImages);

    if (context->swapChainViews) {
        for (size_t i = 0; i < context->swapChainImageCount; ++i) {
            vkDestroyImageView(context->device, context->swapChainViews[i], NULL);
        }
        free(context->swapChainViews);
    }
}