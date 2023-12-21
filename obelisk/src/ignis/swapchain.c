#include "ignis_core.h"

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

static uint32_t ignisClamp32(uint32_t val, uint32_t min, uint32_t max)
{
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
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

static int ignisSwapchainCreateImages(VkDevice device, IgnisSwapchain* swapchain, const VkAllocationCallbacks* allocator)
{
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

        if (vkCreateImageView(device, &createInfo, allocator, &swapchain->imageViews[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to create image view");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

int ignisCreateSwapchain(IgnisContext* context, IgnisSwapchain* swapchain, VkSwapchainKHR old, uint32_t width, uint32_t height)
{
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!ignisChooseSurfaceFormat(context->physical_device, context->surface, &surfaceFormat))
    {
        MINIMAL_ERROR("failed to choose swap chain surface format!");
        return IGNIS_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode = ignisChoosePresentMode(context->physical_device, context->surface);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, context->surface, &capabilities);

    VkExtent2D extent = ignisGetSurfaceExtent(&capabilities, width, height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
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
        context->queue_family_indices[IGNIS_QUEUE_GRAPHICS],
        context->queue_family_indices[IGNIS_QUEUE_PRESENT]
    };

    if (queue_family_indices[0] != queue_family_indices[1])
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queue_family_indices;
    }

    if (vkCreateSwapchainKHR(context->device, &createInfo, context->allocator, &swapchain->handle) != VK_SUCCESS)
    {
        MINIMAL_ERROR("failed to create swap chain!");
        return IGNIS_FAIL;
    }
    
    swapchain->extent = extent;
    swapchain->imageCount = imageCount;
    swapchain->imageFormat = surfaceFormat.format;

    if (!ignisSwapchainCreateImages(context->device, swapchain, context->allocator))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

void ignisDestroySwapchain(IgnisContext* context, IgnisSwapchain* swapchain)
{
    /* destroy images */
    if (swapchain->images) ignisFree(swapchain->images, swapchain->imageCount * sizeof(VkImage));

    if (swapchain->imageViews)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
            vkDestroyImageView(context->device, swapchain->imageViews[i], context->allocator);

        ignisFree(swapchain->imageViews, swapchain->imageCount * sizeof(VkImageView));
    }

    /* destroy handle */
    vkDestroySwapchainKHR(context->device, swapchain->handle, context->allocator);
}
