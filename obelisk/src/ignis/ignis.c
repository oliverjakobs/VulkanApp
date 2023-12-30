#include "ignis.h"


#include "minimal/common.h"


static IgnisContext context;
uint32_t cached_width, cached_height;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    if (!ignisCreateContext(&context, name, platform))
    {
        MINIMAL_CRITICAL("failed to create context");
        return IGNIS_FAIL;
    }
    
    if (!ignisCreateDevice(context.instance, context.surface, &context.device))
    {
        MINIMAL_CRITICAL("failed to create device");
        return IGNIS_FAIL;
    }
    
    ignisPrintDeviceInfo(&context.device);

    if (!ignisCreateSwapchain(&context.device, context.surface, VK_NULL_HANDLE, 1280, 720, &context.swapchain))
    {
        MINIMAL_CRITICAL("failed to create swapchain");
        return IGNIS_FAIL;
    }

    if (ignisAllocCommandBuffers(&context.device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, IGNIS_MAX_FRAMES_IN_FLIGHT, context.swapchain.commandBuffers) != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to allocate command buffers");
        return IGNIS_FAIL;
    }

    if (!ignisCreateSwapchainSyncObjects(context.device.handle, &context.swapchain))
    {
        MINIMAL_CRITICAL("failed to create swapchain sync objects");
        return IGNIS_FAIL;
    }

    // set default state
    ignisSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ignisSetDepthStencil(1.0f, 0);
    ignisSetViewport(0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height);
    ignisSetDepthRange(0.0f, 1.0f);

    return IGNIS_OK;
}

void ignisTerminate()
{
    vkDeviceWaitIdle(context.device.handle);

    ignisDestroySwapchainSyncObjects(context.device.handle, &context.swapchain);
    ignisFreeCommandBuffers(&context.device, IGNIS_MAX_FRAMES_IN_FLIGHT, context.swapchain.commandBuffers);

    ignisDestroySwapchain(context.device.handle, &context.swapchain);
    ignisDestroyDevice(&context.device);

    ignisDestroyContext(&context);
}

uint8_t ignisResize(uint32_t width, uint32_t height)
{
    cached_width = width;
    cached_height = height;
    context.swapchainGeneration++;

    return IGNIS_OK;
}

void ignisSetClearColor(float r, float g, float b, float a)
{
    context.clearColor.float32[0] = r;
    context.clearColor.float32[1] = g;
    context.clearColor.float32[2] = b;
    context.clearColor.float32[3] = a;
}

void ignisSetDepthStencil(float depth, uint32_t stencil)
{
    context.depthStencil.depth = depth;
    context.depthStencil.stencil = stencil;
}

void ignisSetViewport(float x, float y, float width, float height)
{
    context.viewport.x = x;
    context.viewport.y = y;
    context.viewport.width = width;
    context.viewport.height = height;
}

void ignisSetDepthRange(float nearVal, float farVal)
{
    context.viewport.minDepth = nearVal;
    context.viewport.maxDepth = farVal;
}

uint8_t ignisBeginFrame()
{
    // Check if swapchain is out of date.
    if (context.swapchainGeneration != context.swapchainLastGeneration)
    {
        // window is minimized
        if (cached_width == 0 || cached_height == 0)
            return IGNIS_FAIL;

        if (!ignisRecreateSwapchain(&context.device, context.surface, cached_width, cached_height, &context.swapchain))
        {
            MINIMAL_ERROR("Failed to recreate swapchain");
            return IGNIS_FAIL;
        }
        context.swapchainLastGeneration = context.swapchainGeneration;

        MINIMAL_TRACE("Recreated Swapchchain");
    }

    if (!ignisAcquireNextImage(context.device.handle, &context.swapchain, context.currentFrame, &context.imageIndex))
        return IGNIS_FAIL;

    context.commandBuffer = ignisBeginCommandBuffer(&context.swapchain, context.currentFrame);

    // begin render pass
    VkClearValue clearValues[] = {
        {.color = context.clearColor},
        {.depthStencil = context.depthStencil}
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context.swapchain.renderPass,
        .framebuffer = context.swapchain.framebuffers[context.imageIndex],
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = context.swapchain.extent,
        .pClearValues = clearValues,
        .clearValueCount = 2
    };

    vkCmdBeginRenderPass(context.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set dynamic state
    vkCmdSetViewport(context.commandBuffer, 0, 1, &context.viewport);

    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    vkCmdEndRenderPass(context.commandBuffer);

    VkQueue graphicsQueue = context.device.queues[IGNIS_QUEUE_GRAPHICS];
    if (!ingisSubmitFrame(graphicsQueue, context.commandBuffer, context.currentFrame, &context.swapchain))
        MINIMAL_WARN("failed to submit frame");

    VkQueue presentQueue = context.device.queues[IGNIS_QUEUE_PRESENT];
    if (!ignisPresentFrame(presentQueue, context.imageIndex, context.currentFrame, &context.swapchain))
        MINIMAL_WARN("failed to present frame");
    
    /* next frame */
    context.currentFrame = (context.currentFrame + 1) % IGNIS_MAX_FRAMES_IN_FLIGHT;

    return IGNIS_OK;
}
