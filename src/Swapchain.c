#include "Swapchain.h"

static int chooseSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* format) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);

    VkSurfaceFormatKHR* formats;
    if (!count || !(formats = malloc(sizeof(VkSurfaceFormatKHR) * count))) return MINIMAL_FAIL;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    /* set fallback */
    *format = formats[0];

    for (uint32_t i = 0; i < count; ++i) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
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

int createSwapChain(VulkanContext* context, const VkSurfaceCapabilitiesKHR* capabilities, QueueFamilyIndices indices) {
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

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = context->swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

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

    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapchain.handle) != VK_SUCCESS)
        return MINIMAL_FAIL;

    context->swapchain.count = imageCount;
    context->swapchain.format = surfaceFormat.format;
    
    return MINIMAL_OK;
}

int createSwapChainImages(VulkanContext* context) {
    if (!context->swapchain.count) return MINIMAL_FAIL;

    context->swapchain.images = malloc(context->swapchain.count * sizeof(VkImage));
    if (!context->swapchain.images) return MINIMAL_FAIL;

    vkGetSwapchainImagesKHR(context->device, context->swapchain.handle, &context->swapchain.count, context->swapchain.images);

    context->swapchain.views = malloc(context->swapchain.count * sizeof(VkImageView));
    if (!context->swapchain.views) return MINIMAL_FAIL;

    for (size_t i = 0; i < context->swapchain.count; ++i) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = context->swapchain.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = context->swapchain.format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(context->device, &createInfo, NULL, &context->swapchain.views[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}

void destroySwapChainImages(VulkanContext* context) {
    if (context->swapchain.images) free(context->swapchain.images);

    if (context->swapchain.views) {
        for (size_t i = 0; i < context->swapchain.count; ++i) {
            vkDestroyImageView(context->device, context->swapchain.views[i], NULL);
        }
        free(context->swapchain.views);
    }
}

int createFramebuffers(VulkanContext* context) {
    if (!context->swapchain.count) return MINIMAL_FAIL;

    context->swapchain.framebuffers = malloc(context->swapchain.count * sizeof(VkFramebuffer));
    if (!context->swapchain.framebuffers) return MINIMAL_FAIL;

    for (size_t i = 0; i < context->swapchain.count; ++i) {
        VkImageView attachments[] = {
            context->swapchain.views[i]
        };

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = context->renderPass,
            .pAttachments = attachments,
            .attachmentCount = 1,
            .width = context->swapchain.extent.width,
            .height = context->swapchain.extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(context->device, &info, NULL, &context->swapchain.framebuffers[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyFramebuffers(VulkanContext* context) {
    if (context->swapchain.framebuffers) {
        for (size_t i = 0; i < context->swapchain.count; ++i) {
            vkDestroyFramebuffer(context->device, context->swapchain.framebuffers[i], NULL);
        }
        free(context->swapchain.framebuffers);
    }
}