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
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM  && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

        uint32_t memoryTypeIndex = ignisFindMemoryTypeIndex(physical, memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (memoryTypeIndex == UINT32_MAX)
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

static uint8_t ignisCreateSwapchainRenderPass(VkDevice device, IgnisSwapchain* swapchain)
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

    if (vkCreateRenderPass(device, &renderPassInfo, ignisGetAllocator(), &swapchain->renderPass) != VK_SUCCESS)
    {
        MINIMAL_ERROR("failed to create render pass!");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

static uint8_t ignisCreateSwapchainFramebuffers(VkDevice device, IgnisSwapchain* swapchain)
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

        if (vkCreateFramebuffer(device, &info, NULL, &swapchain->framebuffers[i]) != VK_SUCCESS)
        {
            MINIMAL_ERROR("failed to create framebuffer");
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
    
    if (!ignisCreateSwapchainRenderPass(device->handle, swapchain))
        return IGNIS_FAIL;

    if (!ignisCreateSwapchainFramebuffers(device->handle, swapchain))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

void ignisDestroySwapchain(VkDevice device, IgnisSwapchain* swapchain)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

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

uint8_t ignisRecreateSwapchain(const IgnisDevice* device, VkSurfaceKHR surface, uint32_t width, uint32_t height, IgnisSwapchain* swapchain)
{
    vkDeviceWaitIdle(device->handle);

    VkSwapchainKHR oldSwapchain = swapchain->handle;
    swapchain->handle = VK_NULL_HANDLE;

    ignisDestroySwapchain(device->handle, swapchain);

    uint8_t result = ignisCreateSwapchain(device, surface, oldSwapchain, width, height, swapchain);
    
    vkDestroySwapchainKHR(device->handle, oldSwapchain, ignisGetAllocator());

    return result;
}

uint8_t ignisCreateSwapchainSyncObjects(VkDevice device, IgnisSwapchain* swapchain)
{
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    const VkAllocationCallbacks* allocator = ignisGetAllocator();
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

void ignisDestroySwapchainSyncObjects(VkDevice device, IgnisSwapchain* swapchain)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();
    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, swapchain->imageAvailable[i], allocator);
        vkDestroySemaphore(device, swapchain->renderFinished[i], allocator);
        vkDestroyFence(device, swapchain->inFlightFences[i], allocator);
    }
}

uint8_t ignisAcquireNextImage(VkDevice device, IgnisSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex)
{
    vkWaitForFences(device, 1, &swapchain->inFlightFences[frame], VK_TRUE, UINT64_MAX);

    VkSemaphore semaphore = swapchain->imageAvailable[frame];
    VkResult result = vkAcquireNextImageKHR(device, swapchain->handle, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        return IGNIS_FAIL;

    vkResetFences(device, 1, &swapchain->inFlightFences[frame]);

    return IGNIS_OK;
}

VkCommandBuffer ignisBeginCommandBuffer(IgnisSwapchain* swapchain, uint32_t frame)
{
    // Begin recording commands.
    VkCommandBuffer commandBuffer = swapchain->commandBuffers[frame];
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        MINIMAL_WARN("failed to begin recording command buffer!");
        return VK_NULL_HANDLE;
    }

    return commandBuffer;
}

uint8_t ingisSubmitFrame(VkQueue graphics, VkCommandBuffer buffer, uint32_t frame, IgnisSwapchain* swapchain)
{
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
        MINIMAL_WARN("failed to record command buffer!");

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