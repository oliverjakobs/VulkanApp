#include "swapchain.h"

#include "utils.h"


uint8_t ignisCreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkSwapchainKHR old, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat = ignisChooseSurfaceFormat(physical, surface);
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        IGNIS_ERROR("failed to choose swap chain surface format!");
        return IGNIS_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode = ignisChoosePresentMode(physical, surface);

    /* query for depth format */
    VkFormat depthFormat = ignisQueryDepthFormat(physical);
    if (depthFormat == VK_FORMAT_UNDEFINED)
    {
        IGNIS_ERROR("failed to find suitable depth format!");
        return IGNIS_FAIL;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);

    /* get surface extent */
    if (capabilities.currentExtent.width != 0xffffffff)
        extent = capabilities.currentExtent;
    else
        extent = ignisClampExtent2D(extent, capabilities.minImageExtent, capabilities.maxImageExtent);

    /* get image count */
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    /* create swapchain */
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = old
    };

    uint32_t queueFamilyIndices[] = {
        ignisGetQueueFamilyIndex(IGNIS_QUEUE_GRAPHICS),
        ignisGetQueueFamilyIndex(IGNIS_QUEUE_PRESENT)
    };

    if (queueFamilyIndices[0] != queueFamilyIndices[1])
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    if (vkCreateSwapchainKHR(device, &createInfo, allocator, &swapchain->handle) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create swap chain!");
        return IGNIS_FAIL;
    }

    swapchain->extent = extent;
    swapchain->imageCount = imageCount;
    swapchain->imageFormat = surfaceFormat.format;
    swapchain->depthFormat = depthFormat;

    /* create images */
    swapchain->images = ignisAlloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->images) return IGNIS_FAIL;

    if (vkGetSwapchainImagesKHR(device, swapchain->handle, &swapchain->imageCount, swapchain->images) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to get images");
        return IGNIS_FAIL;
    }

    swapchain->imageViews = ignisAlloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->imageViews) return IGNIS_FAIL;

    swapchain->depthImages = ignisAlloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->depthImages) return IGNIS_FAIL;

    swapchain->depthImageViews = ignisAlloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->depthImageViews) return IGNIS_FAIL;

    swapchain->depthImageMemories = ignisAlloc(swapchain->imageCount * sizeof(VkDeviceMemory));
    if (!swapchain->depthImageMemories) return IGNIS_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i)
    {
        /* create image view */
        VkImageViewCreateInfo imageViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->imageFormat,
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

        if (vkCreateImageView(device, &imageViewInfo, allocator, &swapchain->imageViews[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create image view");
            return IGNIS_FAIL;
        }

        /* create depth image */
        VkImageCreateInfo depthImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = swapchain->extent.width,
            .extent.height = swapchain->extent.height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = swapchain->depthFormat,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .flags = 0
        };

        if (vkCreateImage(device, &depthImageInfo, allocator, &swapchain->depthImages[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create image!");
            return IGNIS_FAIL;
        }

        VkMemoryRequirements memoryReq;
        vkGetImageMemoryRequirements(device, swapchain->depthImages[i], &memoryReq);

        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        swapchain->depthImageMemories[i] = ignisAllocateDeviceMemory(memoryReq, properties, allocator);
        if (!swapchain->depthImageMemories[i])
        {
            IGNIS_ERROR("failed to allocate image memory!");
            return IGNIS_FAIL;
        }

        if (vkBindImageMemory(device, swapchain->depthImages[i], swapchain->depthImageMemories[i], 0) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to bind image memory!");
            return IGNIS_FAIL;
        }

        /* create depth image view */
        VkImageViewCreateInfo depthImageViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->depthImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->depthFormat,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(device, &depthImageViewInfo, allocator, &swapchain->depthImageViews[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create depth image view!");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

void ignisDestroySwapchain(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    /* destroy depth images */
    if (swapchain->depthImages)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
        {
            vkDestroyImageView(device, swapchain->depthImageViews[i], allocator);
            vkDestroyImage(device, swapchain->depthImages[i], allocator);
            vkFreeMemory(device, swapchain->depthImageMemories[i], allocator);
        }
        ignisFree(swapchain->depthImages, swapchain->imageCount * sizeof(VkImage));
        ignisFree(swapchain->depthImageViews, swapchain->imageCount * sizeof(VkImageView));
        ignisFree(swapchain->depthImageMemories, swapchain->imageCount * sizeof(VkDeviceMemory));
    }

    /* destroy images */
    if (swapchain->imageViews)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
            vkDestroyImageView(device, swapchain->imageViews[i], allocator);

        ignisFree(swapchain->imageViews, swapchain->imageCount * sizeof(VkImageView));
    }
    if (swapchain->images) ignisFree(swapchain->images, swapchain->imageCount * sizeof(VkImage));

    /* destroy handle */
    vkDestroySwapchainKHR(device, swapchain->handle, allocator);
}

uint8_t ignisRecreateSwapchain(VkDevice device, VkPhysicalDevice physical, VkSurfaceKHR surface, VkExtent2D extent, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    vkDeviceWaitIdle(device);

    VkSwapchainKHR oldSwapchain = swapchain->handle;
    swapchain->handle = VK_NULL_HANDLE;

    ignisDestroySwapchain(device, allocator, swapchain);

    uint8_t result = ignisCreateSwapchain(device, physical, surface, oldSwapchain, extent, allocator, swapchain);

    vkDestroySwapchainKHR(device, oldSwapchain, allocator);

    return result;
}

