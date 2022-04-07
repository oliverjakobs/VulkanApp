#include "Swapchain.h"

#include "Pipeline.h"
#include "Application.h"
#include "Core.h"

static int obeliskChooseSurfaceFormat(VkSurfaceFormatKHR* format) {
    VkPhysicalDevice device = obeliskGetPhysicalDevice();
    VkSurfaceKHR surface = obeliskGetSurface();

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);

    VkSurfaceFormatKHR* formats;
    if (!count || !(formats = malloc(sizeof(VkSurfaceFormatKHR) * count))) return OBELISK_FAIL;

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
    return OBELISK_OK;
}

static int obeliskChoosePresentMode(VkPresentModeKHR* mode) {
    VkPhysicalDevice device = obeliskGetPhysicalDevice();
    VkSurfaceKHR surface = obeliskGetSurface();

    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);

    VkPresentModeKHR* modes;
    if (!count || !(modes = malloc(sizeof(VkPresentModeKHR) * count))) return OBELISK_FAIL;

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
    return OBELISK_OK;
}

static uint32_t obeliskClamp32(uint32_t val, uint32_t min, uint32_t max) {
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
}

static VkExtent2D obeliskGetSurfaceExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h) {
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D extent = {
        .width = obeliskClamp32(w, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = obeliskClamp32(h, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return extent;
}

static uint32_t obeliskGetSurfaceImageCount(const VkSurfaceCapabilitiesKHR* capabilities) {
    uint32_t imageCount = capabilities->minImageCount + 1;
    if (capabilities->maxImageCount > 0 && imageCount > capabilities->maxImageCount) {
        imageCount = capabilities->maxImageCount;
    }
    return imageCount;
}

int obeliskSwapchainCreateImages(ObeliskSwapchain* swapchain) {
    OBELISK_ASSERT(swapchain->imageCount > 0, "image count must be greater than zero");

    VkDevice device = obeliskGetDevice();

    /* create images */
    swapchain->images = malloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->images) return OBELISK_FAIL;

    if (vkGetSwapchainImagesKHR(device, swapchain->handle, &swapchain->imageCount, swapchain->images) != VK_SUCCESS) {
        OBELISK_ERROR("failed to get images");
        return OBELISK_FAIL;
    }

    /* create image views */
    swapchain->imageViews = malloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->imageViews) return OBELISK_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i) {
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

        if (vkCreateImageView(device, &createInfo, NULL, &swapchain->imageViews[i]) != VK_SUCCESS) {
            OBELISK_ERROR("failed to create image view");
            return OBELISK_FAIL;
        }
    }

    /* create depth images and depth image views */
    swapchain->depthImages = malloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->depthImages) return OBELISK_FAIL;

    swapchain->depthImageViews = malloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->depthImageViews) return OBELISK_FAIL;

    swapchain->depthImageMemories = malloc(swapchain->imageCount * sizeof(VkDeviceMemory));
    if (!swapchain->depthImageMemories) return OBELISK_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i) {
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

        if (vkCreateImage(device, &imageInfo, NULL, &swapchain->depthImages[i]) != VK_SUCCESS) {
            OBELISK_ERROR("failed to create image!");
            return OBELISK_FAIL;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, swapchain->depthImages[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = obeliskFindMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };

        if (vkAllocateMemory(device, &allocInfo, NULL, &swapchain->depthImageMemories[i]) != VK_SUCCESS) {
            OBELISK_ERROR("failed to allocate image memory!");
            return OBELISK_FAIL;
        }

        if (vkBindImageMemory(device, swapchain->depthImages[i], swapchain->depthImageMemories[i], 0) != VK_SUCCESS) {
            OBELISK_ERROR("failed to bind image memory!");
            return OBELISK_FAIL;
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

        if (vkCreateImageView(device, &viewInfo, NULL, &swapchain->depthImageViews[i]) != VK_SUCCESS) {
            OBELISK_ERROR("failed to create depth image view!");
            return OBELISK_FAIL;
        }
    }

    return OBELISK_OK;
}

int obeliskSwapchainCreateRenderPass(ObeliskSwapchain* swapchain) {
    VkAttachmentDescription colorAttachment = {
        .format = swapchain->imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment = {
        .format = swapchain->depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = &colorAttachmentRef,
        .colorAttachmentCount = 1,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    VkSubpassDependency dependency = {
        .dstSubpass = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcAccessMask = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = attachments,
        .attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription),
        .pSubpasses = &subpass,
        .subpassCount = 1,
        .pDependencies = &dependency,
        .dependencyCount = 1
    };

    if (vkCreateRenderPass(obeliskGetDevice(), &renderPassInfo, NULL, &swapchain->renderPass) != VK_SUCCESS) {
        OBELISK_ERROR("failed to create render pass!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

int obeliskSwapchainCreateFramebuffers(ObeliskSwapchain* swapchain) {
    OBELISK_ASSERT(swapchain->imageCount > 0, "image count must be greater than zero");

    VkDevice device = obeliskGetDevice();

    swapchain->framebuffers = malloc(swapchain->imageCount * sizeof(VkFramebuffer));
    if (!swapchain->framebuffers) return OBELISK_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i) {
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

        if (vkCreateFramebuffer(device, &info, NULL, &swapchain->framebuffers[i]) != VK_SUCCESS) {
            OBELISK_ERROR("failed to create framebuffer");
            return OBELISK_FAIL;
        }
    }

    return OBELISK_OK;
}

int obeliskCreateSwapchain(ObeliskSwapchain* swapchain, VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height) {
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!obeliskChooseSurfaceFormat(&surfaceFormat)) {
        OBELISK_ERROR("failed to choose swap chain surface format!");
        return OBELISK_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode;
    if (!obeliskChoosePresentMode(&presentMode)) {
        OBELISK_ERROR("failed to choose swap chain presentation mode!");
        return OBELISK_FAIL;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    obeliskGetPhysicalDeviceSurfaceCapabilities(&capabilities);
    VkExtent2D extent = obeliskGetSurfaceExtent(&capabilities, width, height);
    uint32_t imageCount = obeliskGetSurfaceImageCount(&capabilities);
    if (!imageCount) return OBELISK_FAIL;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = obeliskGetSurface(),
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    uint32_t queueFamilyIndices[] = { obeliskGetQueueGraphicsFamilyIndex(), obeliskGetQueuePresentFamilyIndex() };
    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
        createInfo.queueFamilyIndexCount = 2;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.pQueueFamilyIndices = NULL; // Optional
        createInfo.queueFamilyIndexCount = 0; // Optional
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = oldSwapchain;

    if (vkCreateSwapchainKHR(obeliskGetDevice(), &createInfo, NULL, &swapchain->handle) != VK_SUCCESS) {
        OBELISK_ERROR("failed to create swap chain!");
        return OBELISK_FAIL;
    }

    swapchain->extent = extent;
    swapchain->imageCount = imageCount;
    swapchain->imageFormat = surfaceFormat.format;
    VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    uint32_t candidateCount = sizeof(candidates) / sizeof(VkFormat);
    swapchain->depthFormat = obeliskGetPhysicalDeviceFormat(candidates, candidateCount, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (!obeliskSwapchainCreateImages(swapchain))
        return OBELISK_FAIL;

    if (!obeliskSwapchainCreateRenderPass(swapchain))
        return OBELISK_FAIL;

    if (!obeliskSwapchainCreateFramebuffers(swapchain))
        return OBELISK_FAIL;
    
    return OBELISK_OK;
}

int obeliskRecreateSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(obeliskGetDevice());

    VkSwapchainKHR oldSwapchain = swapchain->handle;
    swapchain->handle = VK_NULL_HANDLE;

    obeliskDestroySwapchain(swapchain);

    int result = obeliskCreateSwapchain(swapchain, oldSwapchain, width, height);

    vkDestroySwapchainKHR(obeliskGetDevice(), oldSwapchain, NULL);

    if (!result) {
        OBELISK_ERROR("failed to recreate swap chain!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroySwapchain(ObeliskSwapchain* swapchain) {
    VkDevice device = obeliskGetDevice();

    /* destroy framebuffers */
    if (swapchain->framebuffers) {
        for (size_t i = 0; i < swapchain->imageCount; ++i) {
            vkDestroyFramebuffer(device, swapchain->framebuffers[i], NULL);
        }
        free(swapchain->framebuffers);
    }

    /* destroy render pass */
    vkDestroyRenderPass(device, swapchain->renderPass, NULL);

    /* destroy images */
    if (swapchain->images) free(swapchain->images);

    if (swapchain->imageViews) {
        for (size_t i = 0; i < swapchain->imageCount; ++i) {
            vkDestroyImageView(device, swapchain->imageViews[i], NULL);
        }
        free(swapchain->imageViews);
    }

    /* destroy depth images */
    if (swapchain->depthImages) {
        for (size_t i = 0; i < swapchain->imageCount; ++i) {
            vkDestroyImageView(device, swapchain->depthImageViews[i], NULL);
            vkDestroyImage(device, swapchain->depthImages[i], NULL);
            vkFreeMemory(device, swapchain->depthImageMemories[i], NULL);
        }
        free(swapchain->depthImages);
        free(swapchain->depthImageViews);
        free(swapchain->depthImageMemories);
    }

    /* destroy handle */
    vkDestroySwapchainKHR(device, swapchain->handle, NULL);
}

int obeliskCreateSyncObjects(ObeliskSwapchain* swapchain) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkDevice device = obeliskGetDevice();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &swapchain->imageAvailable[i]) != VK_SUCCESS)
            return OBELISK_FAIL;

        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &swapchain->renderFinished[i]) != VK_SUCCESS)
            return OBELISK_FAIL;

        if (vkCreateFence(device, &fenceInfo, NULL, &swapchain->fences[i]) != VK_SUCCESS)
            return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroySyncObjects(ObeliskSwapchain* swapchain) {
    VkDevice device = obeliskGetDevice();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, swapchain->imageAvailable[i], NULL);
        vkDestroySemaphore(device, swapchain->renderFinished[i], NULL);
        vkDestroyFence(device, swapchain->fences[i], NULL);
    }
}

int obeliskAcquireSwapchainImage(ObeliskSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex) {
    VkDevice device = obeliskGetDevice();

    vkWaitForFences(device, 1, &swapchain->fences[frame], VK_TRUE, UINT64_MAX);

    VkSemaphore semaphore = swapchain->imageAvailable[frame];
    VkResult result = vkAcquireNextImageKHR(device, swapchain->handle, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return OBELISK_FAIL;
    }

    vkResetFences(device, 1, &swapchain->fences[frame]);
    return OBELISK_OK;
}

int obeliskSubmitFrame(ObeliskSwapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame) {
    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { swapchain->imageAvailable[frame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.commandBufferCount = 1;

    VkSemaphore signalSemaphores[] = { swapchain->renderFinished[frame] };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    if (vkQueueSubmit(obeliskGetGraphicsQueue(), 1, &submitInfo, swapchain->fences[frame]) != VK_SUCCESS) {
        return OBELISK_FAIL;
    }
    return OBELISK_OK;
}

int obeliskPresentFrame(ObeliskSwapchain* swapchain, uint32_t imageIndex, uint32_t frame) {
    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = { swapchain->renderFinished[frame] };
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.waitSemaphoreCount = 1;

    VkSwapchainKHR swapChains[] = { swapchain->handle };
    presentInfo.pSwapchains = swapChains;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = &imageIndex;

    if (vkQueuePresentKHR(obeliskGetPresentQueue(), &presentInfo) != VK_SUCCESS) {
        return OBELISK_FAIL;
    }
    return OBELISK_OK;
}