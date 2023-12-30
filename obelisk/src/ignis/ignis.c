#include "ignis.h"


#include "minimal/common.h"


static IgnisContext context;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    if (!ignisCreateContext(&context, name, platform))
    {
        MINIMAL_CRITICAL("failed to create context");
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
    ignisDestroyContext(&context);
}

uint8_t ignisResize(uint32_t width, uint32_t height)
{
    return ignisRecreateSwapchain(&context.device, context.surface, width, height, &context.swapchain);
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
    if (!ignisAcquireSwapchainImage(context.device.handle, &context.swapchain, context.currentFrame, &context.imageIndex))
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

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set dynamic state
    vkCmdSetViewport(commandBuffer, 0, 1, &context.viewport);

    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    VkCommandBuffer commandBuffer = context.commandBuffers[context.currentFrame];
    
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        MINIMAL_WARN("failed to record command buffer!");

    VkQueue graphicsQueue = context.device.queues[IGNIS_QUEUE_GRAPHICS];
    if (!ignisSubmitFrame(graphicsQueue, &context.swapchain, commandBuffer, context.currentFrame))
        MINIMAL_WARN("failed to submit frame");

    VkQueue presentQueue = context.device.queues[IGNIS_QUEUE_PRESENT];
    if (!ignisPresentFrame(presentQueue, &context.swapchain, context.imageIndex, context.currentFrame))
        MINIMAL_WARN("failed to present frame");

    /* next frame */
    context.currentFrame = (context.currentFrame + 1) % IGNIS_MAX_FRAMES_IN_FLIGHT;

    return IGNIS_OK;
}
