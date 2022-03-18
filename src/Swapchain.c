#include "Swapchain.h"

#include "Pipeline.h"

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

static VkExtent2D getSwapChainExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t w, uint32_t h) {
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D extent = {
        .width = clamp32(w, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = clamp32(h, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return extent;
}

int createSwapChain(VulkanContext* context, uint32_t width, uint32_t height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &capabilities);
    context->swapchain.extent = getSwapChainExtent(&capabilities, width, height);

    /* choose swap chain surface format */
    VkSurfaceFormatKHR surfaceFormat;
    if (!chooseSwapSurfaceFormat(context->physicalDevice, context->surface, &surfaceFormat)) {
        MINIMAL_ERROR("failed to choose swap chain surface format!");
        return MINIMAL_FAIL;
    }

    /* choose swap chain presentation mode */
    VkPresentModeKHR presentMode;
    if (!chooseSwapPresentMode(context->physicalDevice, context->surface, &presentMode)) {
        MINIMAL_ERROR("failed to choose swap chain presentation mode!");
        return MINIMAL_FAIL;
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
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

    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapchain.handle) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    context->swapchain.count = imageCount;
    context->swapchain.format = surfaceFormat.format;
    
    return MINIMAL_OK;
}

int recreateSwapChain(VulkanContext* context, GLFWwindow* window) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context->device);

    destroySwapChain(context);

    if (!createSwapChain(context, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to recreate swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createSwapChainImages(context)) {
        MINIMAL_ERROR("failed to recreate swap chain images!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(context)) {
        MINIMAL_ERROR("failed to recreate render pass!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(context)) {
        MINIMAL_ERROR("failed to recreate framebuffer!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroySwapChain(VulkanContext* context) {
    /* destroy framebuffers */
    if (context->swapchain.framebuffers) {
        for (size_t i = 0; i < context->swapchain.count; ++i) {
            vkDestroyFramebuffer(context->device, context->swapchain.framebuffers[i], NULL);
        }
        free(context->swapchain.framebuffers);
    }

    /* destroy render pass */
    vkDestroyRenderPass(context->device, context->swapchain.renderPass, NULL);

    /* destroy images */
    if (context->swapchain.images) free(context->swapchain.images);

    if (context->swapchain.views) {
        for (size_t i = 0; i < context->swapchain.count; ++i) {
            vkDestroyImageView(context->device, context->swapchain.views[i], NULL);
        }
        free(context->swapchain.views);
    }

    /* destroy handle */
    vkDestroySwapchainKHR(context->device, context->swapchain.handle, NULL);
}

int createRenderPass(VulkanContext* context) {
    VkAttachmentDescription colorAttachment = {
        .format = context->swapchain.format,
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

    if (vkCreateRenderPass(context->device, &renderPassInfo, NULL, &context->swapchain.renderPass) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

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
            .renderPass = context->swapchain.renderPass,
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
