#include "ignis.h"

#include "minimal/common.h"


static IgnisContext context;
uint32_t cachedWidth, cachedHeight;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    if (!ignisCreateContext(&context, name, platform))
    {
        MINIMAL_CRITICAL("failed to create context");
        return IGNIS_FAIL;
    }
    
    ignisPrintPhysicalDeviceInfo(context.device.physical);

    // set default state
    ignisSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ignisSetDepthStencil(1.0f, 0);
    ignisSetViewport(0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height);
    ignisSetDepthRange(0.0f, 1.0f);
    ignisSetScissor(0, 0, context.swapchain.extent.width, context.swapchain.extent.height);

    return IGNIS_OK;
}

void ignisTerminate()
{
    ignisDestroyContext(&context);
}

uint8_t ignisResize(uint32_t width, uint32_t height)
{
    cachedWidth = width;
    cachedHeight = height;
    context.swapchainGeneration++;

    return IGNIS_OK;
}

void ignisSetClearColor(float r, float g, float b, float a)
{
    context.clearColor.color.float32[0] = r;
    context.clearColor.color.float32[1] = g;
    context.clearColor.color.float32[2] = b;
    context.clearColor.color.float32[3] = a;
}

void ignisSetDepthStencil(float depth, uint32_t stencil)
{
    context.depthStencil.depthStencil.depth = depth;
    context.depthStencil.depthStencil.stencil = stencil;
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

void ignisSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    context.scissor.offset.x = x;
    context.scissor.offset.y = y;
    context.scissor.extent.width = w;
    context.scissor.extent.height = h;
}

uint8_t ignisBeginFrame()
{
    // Check if swapchain is out of date.
    if (context.swapchainGeneration != context.swapchainLastGeneration)
    {
        // window is minimized
        if (cachedWidth == 0 || cachedHeight == 0)
            return IGNIS_FAIL;

        VkExtent2D extent = { cachedWidth, cachedHeight };
        if (!ignisRecreateSwapchain(&context.device, context.surface, extent, &context.swapchain))
        {
            MINIMAL_ERROR("Failed to recreate swapchain");
            return IGNIS_FAIL;
        }
        context.swapchainLastGeneration = context.swapchainGeneration;

        MINIMAL_TRACE("Recreated Swapchchain");
    }

    if (!ignisAcquireNextImage(context.device.handle, &context.swapchain, context.currentFrame, &context.imageIndex))
        return IGNIS_FAIL;

    // Begin recording commands.
    VkCommandBuffer commandBuffer = context.commandBuffers[context.currentFrame];
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        MINIMAL_WARN("failed to begin recording command buffer!");
        return IGNIS_FAIL;
    }

    // set dynamic state
    vkCmdSetViewport(commandBuffer, 0, 1, &context.viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &context.scissor);

    // begin render pass
    VkClearValue clearValues[] = {
        context.clearColor,
        context.depthStencil
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

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    VkCommandBuffer commandBuffer = context.commandBuffers[context.currentFrame];
    vkCmdEndRenderPass(commandBuffer);

    VkQueue graphicsQueue = context.device.queues[IGNIS_QUEUE_GRAPHICS];
    if (!ingisSubmitFrame(graphicsQueue, commandBuffer, context.currentFrame, &context.swapchain))
        MINIMAL_WARN("failed to submit frame");

    VkQueue presentQueue = context.device.queues[IGNIS_QUEUE_PRESENT];
    if (!ignisPresentFrame(presentQueue, context.imageIndex, context.currentFrame, &context.swapchain))
        MINIMAL_WARN("failed to present frame");
    
    /* next frame */
    context.currentFrame = (context.currentFrame + 1) % IGNIS_MAX_FRAMES_IN_FLIGHT;

    return IGNIS_OK;
}


IgnisContext* ignisGetContext() { return &context; }

VkCommandBuffer ignisGetCommandBuffer() { return context.commandBuffers[context.currentFrame]; }