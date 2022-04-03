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

static int obeliskChoosePresentMode(VkPresentModeKHR* mode) {
    VkPhysicalDevice device = obeliskGetPhysicalDevice();
    VkSurfaceKHR surface = obeliskGetSurface();

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
    MINIMAL_ASSERT(swapchain->imageCount > 0, "image count must be greater than zero");

    VkDevice device = obeliskGetDevice();

    /* create images */
    swapchain->images = malloc(swapchain->imageCount * sizeof(VkImage));
    if (!swapchain->images) return MINIMAL_FAIL;

    if (vkGetSwapchainImagesKHR(device, swapchain->handle, &swapchain->imageCount, swapchain->images) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to get images");
        return MINIMAL_FAIL;
    }

    /* create image views */
    swapchain->imageViews = malloc(swapchain->imageCount * sizeof(VkImageView));
    if (!swapchain->imageViews) return MINIMAL_FAIL;

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
            MINIMAL_ERROR("failed to create image view");
            return MINIMAL_FAIL;
        }
    }

    return MINIMAL_OK;
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

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = &colorAttachmentRef,
        .colorAttachmentCount = 1
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = &colorAttachment,
        .attachmentCount = 1,
        .pSubpasses = &subpass,
        .subpassCount = 1
    };

    if (vkCreateRenderPass(obeliskGetDevice(), &renderPassInfo, NULL, &swapchain->renderPass) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create render pass!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int obeliskSwapchainCreateFramebuffers(ObeliskSwapchain* swapchain) {
    MINIMAL_ASSERT(swapchain->imageCount > 0, "image count must be greater than zero");

    VkDevice device = obeliskGetDevice();

    swapchain->framebuffers = malloc(swapchain->imageCount * sizeof(VkFramebuffer));
    if (!swapchain->framebuffers) return MINIMAL_FAIL;

    for (size_t i = 0; i < swapchain->imageCount; ++i) {
        VkImageView attachments[] = {
            swapchain->imageViews[i]
        };

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = swapchain->renderPass,
            .pAttachments = attachments,
            .attachmentCount = 1,
            .width = swapchain->extent.width,
            .height = swapchain->extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(device, &info, NULL, &swapchain->framebuffers[i]) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to create framebuffer");
            return MINIMAL_FAIL;
        }
    }

    return MINIMAL_OK;
}

int obeliskCreateSwapchain(ObeliskSwapchain* swapchain, VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height) {
    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!obeliskChooseSurfaceFormat(&surfaceFormat)) {
        MINIMAL_ERROR("failed to choose swap chain surface format!");
        return MINIMAL_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode;
    if (!obeliskChoosePresentMode(&presentMode)) {
        MINIMAL_ERROR("failed to choose swap chain presentation mode!");
        return MINIMAL_FAIL;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    obeliskGetPhysicalDeviceSurfaceCapabilities(&capabilities);
    VkExtent2D extent = obeliskGetSurfaceExtent(&capabilities, width, height);
    uint32_t imageCount = obeliskGetSurfaceImageCount(&capabilities);
    if (!imageCount) return MINIMAL_FAIL;

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
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    swapchain->imageCount = imageCount;
    swapchain->imageFormat = surfaceFormat.format;
    swapchain->extent = extent;

    if (!obeliskSwapchainCreateImages(swapchain))
        return MINIMAL_FAIL;

    if (!obeliskSwapchainCreateRenderPass(swapchain))
        return MINIMAL_FAIL;

    if (!obeliskSwapchainCreateFramebuffers(swapchain))
        return MINIMAL_FAIL;
    
    return MINIMAL_OK;
}

int obeliskRecreateSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(obeliskGetDevice());

    VkSwapchainKHR oldSwapchain = swapchain->handle;
    swapchain->handle = VK_NULL_HANDLE;

    obeliskDestroySwapchain(swapchain);

    int result = obeliskCreateSwapchain(swapchain, oldSwapchain, width, height);

    vkDestroySwapchainKHR(obeliskGetDevice(), oldSwapchain, NULL);

    if (!result) {
        MINIMAL_ERROR("failed to recreate swap chain!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
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
            return MINIMAL_FAIL;

        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &swapchain->renderFinished[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;

        if (vkCreateFence(device, &fenceInfo, NULL, &swapchain->fences[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
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
        return MINIMAL_FAIL;
    }

    vkResetFences(device, 1, &swapchain->fences[frame]);
    return MINIMAL_OK;
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
        return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
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
        return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}