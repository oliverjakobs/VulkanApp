#include "Frame.h"


int createCommandPool(VulkanContext* context) {
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context->indices.graphicsFamily
    };

    if (vkCreateCommandPool(context->device, &info, NULL, &context->commandPool) != VK_SUCCESS)
        return MINIMAL_FAIL;

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

int createSyncObjects(VulkanContext* context) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    context->currentFrame = 0;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->imageAvailableSemaphores[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;

        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->renderFinishedSemaphores[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;

        if (vkCreateFence(context->device, &fenceInfo, NULL, &context->inFlightFences[i]) != VK_SUCCESS)
            return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroySyncObjects(VulkanContext* context) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->device, context->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(context->device, context->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(context->device, context->inFlightFences[i], NULL);
    }
}

void recordCommandBuffer(const VulkanContext* context, VkCommandBuffer cmdBuffer, const Buffer* vertexBuffer, uint32_t imageIndex) {
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
        .renderPass = context->renderPass,
        .framebuffer = context->swapchain.framebuffers[imageIndex],
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = context->swapchain.extent,
        .pClearValues = &clearValue,
        .clearValueCount = 1,
    };

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphicsPipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer->handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
        MINIMAL_WARN("failed to record command buffer!");
    }
}