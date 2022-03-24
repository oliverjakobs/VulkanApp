#include "Swapchain.h"

#include "Pipeline.h"
#include "Application.h"

static int chooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* format) {
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

static int choosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR* mode) {
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

static VkExtent2D getSurfaceExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h) {
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D extent = {
        .width = clamp32(w, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = clamp32(h, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return extent;
}

static uint32_t getSurfaceImageCount(const VkSurfaceCapabilitiesKHR* capabilities) {
    uint32_t imageCount = capabilities->minImageCount + 1;
    if (capabilities->maxImageCount > 0 && imageCount > capabilities->maxImageCount) {
        imageCount = capabilities->maxImageCount;
    }
    return imageCount;
}

int createSwapchain(const VulkanContext* context, Swapchain* swapchain, uint32_t width, uint32_t height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &capabilities);
    swapchain->extent = getSurfaceExtent(&capabilities, width, height);

    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!chooseSurfaceFormat(context->physicalDevice, context->surface, &surfaceFormat)) {
        MINIMAL_ERROR("failed to choose swap chain surface format!");
        return MINIMAL_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode;
    if (!choosePresentMode(context->physicalDevice, context->surface, &presentMode)) {
        MINIMAL_ERROR("failed to choose swap chain presentation mode!");
        return MINIMAL_FAIL;
    }

    uint32_t imageCount = getSurfaceImageCount(&capabilities);
    if (!imageCount) return MINIMAL_FAIL;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = swapchain->extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    uint32_t queueFamilyIndices[] = { context->indices.graphicsFamily, context->indices.presentFamily };
    if (context->indices.graphicsFamily != context->indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &swapchain->handle) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    /* create images */
    swapchain->images = malloc(imageCount * sizeof(VkImage));
    if (!swapchain->images) return MINIMAL_FAIL;

    if (vkGetSwapchainImagesKHR(context->device, swapchain->handle, &imageCount, swapchain->images) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to get images");
        return MINIMAL_FAIL;
    }

    /* create image views */
    swapchain->views = malloc(imageCount * sizeof(VkImageView));
    if (!swapchain->views) return MINIMAL_FAIL;

    for (size_t i = 0; i < imageCount; ++i) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,
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

        if (vkCreateImageView(context->device, &createInfo, NULL, &swapchain->views[i]) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to create image view");
            return MINIMAL_FAIL;
        }
    }

    swapchain->count = imageCount;
    swapchain->format = surfaceFormat.format;
    
    return MINIMAL_OK;
}

int recreateSwapchain(const VulkanContext* context, Swapchain* swapchain, GLFWwindow* window) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    vkDeviceWaitIdle(context->device);

    destroySwapchain(context, swapchain);

    if (!createSwapchain(context, swapchain, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to recreate swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(context, swapchain)) {
        MINIMAL_ERROR("failed to recreate render pass!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(context, swapchain)) {
        MINIMAL_ERROR("failed to recreate framebuffer!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroySwapchain(const VulkanContext* context, Swapchain* swapchain) {
    /* destroy framebuffers */
    if (swapchain->framebuffers) {
        for (size_t i = 0; i < swapchain->count; ++i) {
            vkDestroyFramebuffer(context->device, swapchain->framebuffers[i], NULL);
        }
        free(swapchain->framebuffers);
    }

    /* destroy render pass */
    vkDestroyRenderPass(context->device, swapchain->renderPass, NULL);

    /* destroy images */
    if (swapchain->images) free(swapchain->images);

    if (swapchain->views) {
        for (size_t i = 0; i < swapchain->count; ++i) {
            vkDestroyImageView(context->device, swapchain->views[i], NULL);
        }
        free(swapchain->views);
    }

    /* destroy handle */
    vkDestroySwapchainKHR(context->device, swapchain->handle, NULL);
}

int createRenderPass(const VulkanContext* context, Swapchain* swapchain) {
    VkAttachmentDescription colorAttachment = {
        .format = swapchain->format,
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

    if (vkCreateRenderPass(context->device, &renderPassInfo, NULL, &swapchain->renderPass) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int createFramebuffers(const VulkanContext* context, Swapchain* swapchain) {
    if (!swapchain->count) return MINIMAL_FAIL;

    swapchain->framebuffers = malloc(swapchain->count * sizeof(VkFramebuffer));
    if (!swapchain->framebuffers) return MINIMAL_FAIL;

    for (size_t i = 0; i < swapchain->count; ++i) {
        VkImageView attachments[] = {
            swapchain->views[i]
        };

        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = context->swapchain.renderPass,
            .pAttachments = attachments,
            .attachmentCount = 1,
            .width = context->swapchain.extent.width,
            .height = context->swapchain.extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(context->device, &info, NULL, &swapchain->framebuffers[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int acquireSwapchainImage(const VulkanContext* context, Swapchain* swapchain, uint32_t frame, uint32_t* imageIndex) {
    vkWaitForFences(context->device, 1, &swapchain->frames[frame].fence, VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(context->device, swapchain->handle, UINT64_MAX, swapchain->frames[frame].imageAvailable, VK_NULL_HANDLE, imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return MINIMAL_FAIL;
    }

    vkResetFences(context->device, 1, &swapchain->frames[frame].fence);
    return MINIMAL_OK;
}

int submitFrame(const VulkanContext* context, Swapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame) {
    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { swapchain->frames[frame].imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.commandBufferCount = 1;

    VkSemaphore signalSemaphores[] = { swapchain->frames[frame].renderFinished };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, swapchain->frames[frame].fence) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}

int presentFrame(const VulkanContext* context, Swapchain* swapchain, uint32_t imageIndex, uint32_t frame) {
    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = { swapchain->frames[frame].renderFinished };
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.waitSemaphoreCount = 1;

    VkSwapchainKHR swapChains[] = { swapchain->handle };
    presentInfo.pSwapchains = swapChains;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = &imageIndex;

    if (vkQueuePresentKHR(context->presentQueue, &presentInfo) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}

int createCommandBuffer(VulkanContext* context) {
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    if (vkAllocateCommandBuffers(context->device, &info, context->commandBuffers) != VK_SUCCESS)
        return MINIMAL_FAIL;

    return MINIMAL_OK;
}

void commandBufferStart(VkCommandBuffer cmdBuffer, const Swapchain* swapchain, uint32_t imageIndex) {
    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
        MINIMAL_WARN("failed to begin recording command buffer!");
        return;
    }

    VkClearValue clearValue = {
        .color = {{0.0f, 0.0f, 0.0f, 1.0f}}
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = swapchain->renderPass,
        .framebuffer = swapchain->framebuffers[imageIndex],
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = swapchain->extent,
        .pClearValues = &clearValue,
        .clearValueCount = 1,
    };

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void commandBufferEnd(VkCommandBuffer cmdBuffer) {
    vkCmdEndRenderPass(cmdBuffer);
    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
        MINIMAL_WARN("failed to record command buffer!");
    }
}

int createDescriptorPool(VulkanContext* context) {
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pPoolSizes = &poolSize,
        .poolSizeCount = 1,
        .maxSets = MAX_FRAMES_IN_FLIGHT
    };

    if (vkCreateDescriptorPool(context->device, &poolInfo, NULL, &context->descriptorPool) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int createDescriptorSets(VulkanContext* context) {
    /* create descriptor set layout */
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL // Optional
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pBindings = &uboLayoutBinding,
        .bindingCount = 1
    };

    if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, NULL, &context->descriptorSetLayout) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create descriptor set layout!");
        return MINIMAL_FAIL;
    }

    /* create descriptor sets */
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = context->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = context->descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    if (vkAllocateDescriptorSets(context->device, &allocInfo, context->descriptorSets) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate descriptor sets!");
        return MINIMAL_FAIL;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = context->uniformBuffers[i].handle,
            .range = context->uniformBuffers[i].size,
            .offset = 0
        };

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = context->descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo
        };

        vkUpdateDescriptorSets(context->device, 1, &descriptorWrite, 0, NULL);
    }

    return MINIMAL_OK;
}

void destroyDescriptorSets(VulkanContext* context) {
    vkDestroyDescriptorSetLayout(context->device, context->descriptorSetLayout, NULL);
}

int createSyncObjects(VulkanContext* context, Swapchain* swapchain) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        FrameInfo* frame = &swapchain->frames[i];

        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &frame->imageAvailable) != VK_SUCCESS)
            return MINIMAL_FAIL;

        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &frame->renderFinished) != VK_SUCCESS)
            return MINIMAL_FAIL;

        if (vkCreateFence(context->device, &fenceInfo, NULL, &frame->fence) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroySyncObjects(VulkanContext* context, Swapchain* swapchain) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        FrameInfo* frame = &swapchain->frames[i];
        vkDestroySemaphore(context->device, frame->renderFinished, NULL);
        vkDestroySemaphore(context->device, frame->imageAvailable, NULL);
        vkDestroyFence(context->device, frame->fence, NULL);
    }
}