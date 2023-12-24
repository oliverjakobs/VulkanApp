#include "../ignis_core.h"

#include "minimal/common.h"

static uint8_t ignisChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* format)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);
    if (!count) return IGNIS_FAIL;

    VkSurfaceFormatKHR* formats = ignisAlloc(sizeof(VkSurfaceFormatKHR) * count);
    if (!formats) return IGNIS_FAIL;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    /* set fallback */
    *format = formats[0];
    for (uint32_t i = 0; i < count; ++i)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            *format = formats[i];
            break;
        }
    }

    ignisFree(formats, sizeof(VkSurfaceFormatKHR) * count);
    return IGNIS_OK;
}

static VkPresentModeKHR ignisChoosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);
    if (!count) return VK_PRESENT_MODE_FIFO_KHR;

    VkPresentModeKHR* modes = ignisAlloc(sizeof(VkPresentModeKHR) * count);
    if (!modes) return VK_PRESENT_MODE_FIFO_KHR;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes);

    /* set fallback */
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < count; ++i)
    {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            mode = modes[i];
            break;
        }
    }

    ignisFree(modes, sizeof(VkPresentModeKHR) * count);
    return mode;
}

static VkExtent2D ignisGetSurfaceExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h)
{
    if (capabilities->currentExtent.width != 0xffffffff)
        return capabilities->currentExtent;

    VkExtent2D extent = {
        .width = ignisClamp32(w, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = ignisClamp32(h, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return extent;
}

static uint8_t ignisCreateSwapchainImages(VkDevice device, uint32_t count, VkFormat format, IgnisSwapchain* swapchain)
{
    swapchain->imageCount = count;
    swapchain->imageFormat = format;

    /* create images */
    swapchain->images = ignisAlloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->images) return IGNIS_FAIL;

    if (vkGetSwapchainImagesKHR(device, swapchain->handle, &swapchain->imageCount, swapchain->images) != VK_SUCCESS)
    {
        MINIMAL_ERROR("failed to get images");
        return IGNIS_FAIL;
    }

    /* create image views */
    swapchain->imageViews = ignisAlloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->imageViews) return IGNIS_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i)
    {
        VkImageViewCreateInfo createInfo = {
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

        if (vkCreateImageView(device, &createInfo, ignisGetAllocator(), &swapchain->imageViews[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to create image view");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

static uint8_t ignisCreateSwapchainDepthImages(VkDevice device, VkPhysicalDevice physical, IgnisSwapchain* swapchain)
{
    swapchain->depthFormat = ignisQueryDeviceDepthFormat(physical);
    if (swapchain->depthFormat == VK_FORMAT_UNDEFINED) return IGNIS_FAIL;

    swapchain->depthImages = ignisAlloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->depthImages) return IGNIS_FAIL;

    swapchain->depthImageViews = ignisAlloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->depthImageViews) return IGNIS_FAIL;

    swapchain->depthImageMemories = ignisAlloc(swapchain->imageCount * sizeof(VkDeviceMemory));
    if (!swapchain->depthImageMemories) return IGNIS_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i)
    {
        VkImageCreateInfo imageInfo = {
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

        if (vkCreateImage(device, &imageInfo, ignisGetAllocator(), &swapchain->depthImages[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to create image!");
            return IGNIS_FAIL;
        }

        VkMemoryRequirements memoryReq;
        vkGetImageMemoryRequirements(device, swapchain->depthImages[i], &memoryReq);

        int32_t memoryTypeIndex = ignisFindMemoryTypeIndex(physical, memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (memoryTypeIndex < 0)
        {
            MINIMAL_ERROR("failed to find suitable memory type!");
            return IGNIS_FAIL;
        }

        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryReq.size,
            .memoryTypeIndex = memoryTypeIndex
        };

        if (vkAllocateMemory(device, &allocInfo, ignisGetAllocator(), &swapchain->depthImageMemories[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to allocate image memory!");
            return IGNIS_FAIL;
        }

        if (vkBindImageMemory(device, swapchain->depthImages[i], swapchain->depthImageMemories[i], 0) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to bind image memory!");
            return IGNIS_FAIL;
        }

        VkImageViewCreateInfo viewInfo = {
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

        if (vkCreateImageView(device, &viewInfo, ignisGetAllocator(), &swapchain->depthImageViews[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to create depth image view!");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

uint8_t ignisCreateSwapchain(const IgnisDevice* device, VkSurfaceKHR surface, VkSwapchainKHR old, uint32_t w, uint32_t h, IgnisSwapchain* swapchain)
{
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!ignisChooseSurfaceFormat(device->physical, surface, &surfaceFormat))
    {
        MINIMAL_ERROR("failed to choose swap chain surface format!");
        return IGNIS_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode = ignisChoosePresentMode(device->physical, surface);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, surface, &capabilities);

    VkExtent2D extent = ignisGetSurfaceExtent(&capabilities, w, h);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

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

    uint32_t queue_family_indices[] = {
        device->queueFamilyIndices[IGNIS_QUEUE_GRAPHICS],
        device->queueFamilyIndices[IGNIS_QUEUE_PRESENT]
    };

    if (queue_family_indices[0] != queue_family_indices[1])
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queue_family_indices;
    }

    if (vkCreateSwapchainKHR(device->handle, &createInfo, ignisGetAllocator(), &swapchain->handle) != VK_SUCCESS)
    {
        MINIMAL_ERROR("failed to create swap chain!");
        return IGNIS_FAIL;
    }
    
    swapchain->extent = extent;

    if (!ignisCreateSwapchainImages(device->handle, imageCount, surfaceFormat.format, swapchain))
        return IGNIS_FAIL;

    if (!ignisCreateSwapchainDepthImages(device->handle, device->physical, swapchain))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

void ignisDestroySwapchain(const IgnisDevice* device, IgnisSwapchain* swapchain)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    /* destroy depth images */
    if (swapchain->depthImages)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
        {
            vkDestroyImageView(device->handle, swapchain->depthImageViews[i], allocator);
            vkDestroyImage(device->handle, swapchain->depthImages[i], allocator);
            vkFreeMemory(device->handle, swapchain->depthImageMemories[i], allocator);
        }
        ignisFree(swapchain->depthImages, swapchain->imageCount * sizeof(VkImage));
        ignisFree(swapchain->depthImageViews, swapchain->imageCount * sizeof(VkImageView));
        ignisFree(swapchain->depthImageMemories, swapchain->imageCount * sizeof(VkDeviceMemory));
    }

    /* destroy images */
    if (swapchain->imageViews)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
            vkDestroyImageView(device->handle, swapchain->imageViews[i], allocator);

        ignisFree(swapchain->imageViews, swapchain->imageCount * sizeof(VkImageView));
    }
    if (swapchain->images) ignisFree(swapchain->images, swapchain->imageCount * sizeof(VkImage));

    /* destroy handle */
    vkDestroySwapchainKHR(device->handle, swapchain->handle, allocator);
}
