#include "swapchain.h"

static VkSurfaceFormatKHR ignisChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);
    if (!count) return (VkSurfaceFormatKHR){VK_FORMAT_UNDEFINED, 0};

    VkSurfaceFormatKHR* formats = ignisAlloc(sizeof(VkSurfaceFormatKHR) * count);
    if (!formats) return (VkSurfaceFormatKHR){VK_FORMAT_UNDEFINED, 0};

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    /* set fallback */
    VkSurfaceFormatKHR format = formats[0];
    for (uint32_t i = 0; i < count; ++i)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM
            && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = formats[i];
            break;
        }
    }

    ignisFree(formats, sizeof(VkSurfaceFormatKHR) * count);
    return format;
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

static VkFormat ignisQueryDepthFormat(VkPhysicalDevice device)
{
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    const uint32_t count = sizeof(candidates) / sizeof(candidates[0]);;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormat format = VK_FORMAT_UNDEFINED;
    for (uint32_t i = 0; i < count; ++i)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, candidates[i], &props);

        if ((props.linearTilingFeatures & features) == features)
        {
            format = candidates[i];
            break;
        }
        else if ((props.optimalTilingFeatures & features) == features)
        {
            format = candidates[i];
            break;
        }
    }

    return format;
}

static VkExtent2D ignisClampExtent2D(VkExtent2D extent, VkExtent2D min, VkExtent2D max)
{
    return (VkExtent2D){
        .width = ignisClamp32(extent.width, min.width, max.width),
        .height = ignisClamp32(extent.height, min.height, max.height)
    };
}

static uint8_t ignisCreateSwapchainRenderPass(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    VkAttachmentDescription attachments[] = {
        {
            // colorAttachment
            .format = swapchain->imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
        {
            // depthAttachment
            .format = swapchain->depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    VkAttachmentReference attachmentRefs[] = {
        { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = attachmentRefs,
        .colorAttachmentCount = 1,
        .pDepthStencilAttachment = &attachmentRefs[1]
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = attachments,
        .attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription),
        .pSubpasses = &subpass,
        .subpassCount = 1,
        .pDependencies = &dependency,
        .dependencyCount = 1
    };

    if (vkCreateRenderPass(device, &renderPassInfo, allocator, &swapchain->renderPass) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create render pass!");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

static uint8_t ignisCreateSwapchainImages(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    /* create images */
    swapchain->images = ignisAlloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->images) return IGNIS_FAIL;

    if (vkGetSwapchainImagesKHR(device, swapchain->handle, &swapchain->imageCount, swapchain->images) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to get images");
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
            IGNIS_ERROR("failed to create image view");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

static uint8_t ignisCreateSwapchainDepthImages(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    /* create depth images and views */
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

        if (vkCreateImage(device, &imageInfo, allocator, &swapchain->depthImages[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create image!");
            return IGNIS_FAIL;
        }

        VkMemoryRequirements memoryReq;
        vkGetImageMemoryRequirements(device, swapchain->depthImages[i], &memoryReq);

        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        swapchain->depthImageMemories[i] = ignisAllocateDeviceMemory( memoryReq, properties, allocator);
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

        if (vkCreateImageView(device, &viewInfo, allocator, &swapchain->depthImageViews[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create depth image view!");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

static uint8_t ignisCreateSwapchainFramebuffers(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    swapchain->framebuffers = ignisAlloc(swapchain->imageCount * sizeof(VkFramebuffer));
    if (!swapchain->framebuffers) return IGNIS_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i)
    {
        VkImageView attachments[] = {
            swapchain->imageViews[i],
            swapchain->depthImageViews[i]
        };

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = swapchain->renderPass,
            .pAttachments = attachments,
            .attachmentCount = sizeof(attachments) / sizeof(VkImageView),
            .width = swapchain->extent.width,
            .height = swapchain->extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(device, &info, allocator, &swapchain->framebuffers[i]) != VK_SUCCESS)
        {
            IGNIS_ERROR("failed to create framebuffer");
            return IGNIS_FAIL;
        }
    }

    return IGNIS_OK;
}

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

    if (!ignisCreateSwapchainRenderPass(device, allocator, swapchain))
        return IGNIS_FAIL;

    /* create frambuffers */
    if (!ignisCreateSwapchainImages(device, allocator, swapchain))
        return IGNIS_FAIL;

    if (!ignisCreateSwapchainDepthImages(device, allocator, swapchain))
        return IGNIS_FAIL;

    if (!ignisCreateSwapchainFramebuffers(device, allocator, swapchain))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

void ignisDestroySwapchain(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    vkDestroyRenderPass(device, swapchain->renderPass, allocator);

    if (swapchain->framebuffers)
    {
        for (size_t i = 0; i < swapchain->imageCount; ++i)
            vkDestroyFramebuffer(device, swapchain->framebuffers[i], allocator);

        ignisFree(swapchain->framebuffers, swapchain->imageCount * sizeof(VkFramebuffer));
    }

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

uint8_t ignisCreateSwapchainSyncObjects(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, allocator, &swapchain->imageAvailable[i]) != VK_SUCCESS)
            return IGNIS_FAIL;

        if (vkCreateSemaphore(device, &semaphoreInfo, allocator, &swapchain->renderFinished[i]) != VK_SUCCESS)
            return IGNIS_FAIL;

        if (vkCreateFence(device, &fenceInfo, allocator, &swapchain->inFlightFences[i]) != VK_SUCCESS)
            return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroySwapchainSyncObjects(VkDevice device, const VkAllocationCallbacks* allocator, IgnisSwapchain* swapchain)
{
    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, swapchain->imageAvailable[i], allocator);
        vkDestroySemaphore(device, swapchain->renderFinished[i], allocator);
        vkDestroyFence(device, swapchain->inFlightFences[i], allocator);
    }
}

uint8_t ignisAcquireNextImage(VkDevice device, IgnisSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex)
{
    vkWaitForFences(device, 1, &swapchain->inFlightFences[frame], VK_TRUE, -1);

    VkSemaphore semaphore = swapchain->imageAvailable[frame];
    VkResult result = vkAcquireNextImageKHR(device, swapchain->handle, -1, semaphore, VK_NULL_HANDLE, imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        return IGNIS_FAIL;

    vkResetFences(device, 1, &swapchain->inFlightFences[frame]);

    return IGNIS_OK;
}

uint8_t ingisSubmitFrame(VkQueue graphics, VkCommandBuffer buffer, uint32_t frame, IgnisSwapchain* swapchain)
{
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
        IGNIS_WARN("failed to record command buffer!");

    VkSemaphore waitSemaphores[] = { swapchain->imageAvailable[frame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSemaphore signalSemaphores[] = { swapchain->renderFinished[frame] };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .waitSemaphoreCount = 1,
        .pCommandBuffers = &buffer,
        .commandBufferCount = 1,
        .pSignalSemaphores = signalSemaphores,
        .signalSemaphoreCount = 1
    };

    if (vkQueueSubmit(graphics, 1, &submitInfo, swapchain->inFlightFences[frame]) != VK_SUCCESS)
        return IGNIS_FAIL;

    return IGNIS_OK;
}

uint8_t ignisPresentFrame(VkQueue present, uint32_t imageIndex, uint32_t frame, IgnisSwapchain* swapchain)
{
    VkSemaphore waitSemaphores[] = { swapchain->renderFinished[frame] };

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pWaitSemaphores = waitSemaphores,
        .waitSemaphoreCount = 1,
        .pSwapchains = &swapchain->handle,
        .swapchainCount = 1,
        .pImageIndices = &imageIndex
    };

    if (vkQueuePresentKHR(present, &presentInfo) != VK_SUCCESS)
        return IGNIS_FAIL;
        
    return IGNIS_OK;
}