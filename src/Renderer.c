#include "Renderer.h"

#include "Core.h"

int obeliskCreateRenderer(ObeliskRenderer* renderer, GLFWwindow* window) {
    /* create swap chain */
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    if (!obeliskCreateSwapchain(&renderer->swapchain, VK_NULL_HANDLE, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    /* create sync objects */
    if (!obeliskCreateSyncObjects(&renderer->swapchain)) {
        MINIMAL_ERROR("failed to create synchronization objects!");
        return MINIMAL_FAIL;
    }

    /* create uniform buffers */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!obeliskCreateUniformBuffer(&renderer->uniformBuffers[i], sizeof(UniformBufferObject))) {
            MINIMAL_ERROR("failed to create uniform buffer!");
            return MINIMAL_FAIL;
        }
    }

    /* allocate command buffers */
    if (obeliskAllocateCommandBuffers(renderer->commandBuffers, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate command buffers!");
        return MINIMAL_FAIL;
    }

    renderer->frame = 0;
    renderer->imageIndex = 0;

    return MINIMAL_OK;
}

void obeliskDestroyRenderer(ObeliskRenderer* renderer) {
    /* destroy uniform buffers */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        obeliskDestroyBuffer(&renderer->uniformBuffers[i]);
    }

    /* destroy snyc objects */
    obeliskDestroySyncObjects(&renderer->swapchain);

    /* destroy swapchain */
    obeliskDestroySwapchain(&renderer->swapchain);
}

int obeliskCreateDescriptorPool(ObeliskRenderer* renderer) {
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

    if (vkCreateDescriptorPool(obeliskGetDevice(), &poolInfo, NULL, &renderer->descriptorPool) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void obeliskDestroyDescriptorPool(ObeliskRenderer* renderer) {
    vkDestroyDescriptorPool(obeliskGetDevice(), renderer->descriptorPool, NULL);
}

int obeliskCreateDescriptorLayout(ObeliskRenderer* renderer) {
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

    if (vkCreateDescriptorSetLayout(obeliskGetDevice(), &layoutInfo, NULL, &renderer->descriptorSetLayout) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void obeliskDestroyDescriptorLayout(ObeliskRenderer* renderer) {
    vkDestroyDescriptorSetLayout(obeliskGetDevice(), renderer->descriptorSetLayout, NULL);
}

int obeliskCreateDescriptorSets(ObeliskRenderer* renderer) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = renderer->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = renderer->descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    if (vkAllocateDescriptorSets(obeliskGetDevice(), &allocInfo, renderer->descriptorSets) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = renderer->uniformBuffers[i].handle,
            .range = renderer->uniformBuffers[i].size,
            .offset = 0
        };

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = renderer->descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo
        };

        vkUpdateDescriptorSets(obeliskGetDevice(), 1, &descriptorWrite, 0, NULL);
    }

    return MINIMAL_OK;
}

VkCommandBuffer obeliskBeginFrame(ObeliskRenderer* renderer) {
    /* acquire swap chain image */
    if (!obeliskAcquireSwapchainImage(&renderer->swapchain, renderer->frame, &renderer->imageIndex)) {
        MINIMAL_ERROR("failed to acquire swap chain image!");
        return VK_NULL_HANDLE;
    }

    /* start frame */
    VkCommandBuffer cmdBuffer = renderer->commandBuffers[renderer->frame];

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
        MINIMAL_WARN("failed to begin recording command buffer!");
        return VK_NULL_HANDLE;
    }

    return cmdBuffer;
}

void obeliskEndFrame(ObeliskRenderer* renderer) {
    if (vkEndCommandBuffer(renderer->commandBuffers[renderer->frame]) != VK_SUCCESS) {
        MINIMAL_WARN("failed to record command buffer!");
    }

    /* submit frame */
    if (!obeliskSubmitFrame(&renderer->swapchain, renderer->commandBuffers[renderer->frame], renderer->frame)) {
        MINIMAL_ERROR("failed to submit draw command buffer!");
        return;
    }

    /* present frame */
    if (!obeliskPresentFrame(&renderer->swapchain, renderer->imageIndex, renderer->frame)) {
        MINIMAL_ERROR("failed to present swap chain image!");
        return;
    }

    /* next frame */
    renderer->frame = (renderer->frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void obeliskBeginRenderPass(ObeliskRenderer* renderer, VkCommandBuffer cmdBuffer) {
    VkClearValue clearValues[] = {
        {.color = { 0.0f, 0.0f, 0.0f, 1.0f } },
        {.depthStencil = {1.0f, 0}}
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderer->swapchain.renderPass,
        .framebuffer = renderer->swapchain.framebuffers[renderer->imageIndex],
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = renderer->swapchain.extent,
        .pClearValues = clearValues,
        .clearValueCount = sizeof(clearValues) / sizeof(VkClearValue)
    };

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)renderer->swapchain.extent.width,
        .height = (float)renderer->swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = renderer->swapchain.extent
    };

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

void obeliskEndRenderPass(ObeliskRenderer* renderer, VkCommandBuffer cmdBuffer) {
    vkCmdEndRenderPass(cmdBuffer);
}

void obeliskWriteUniform(ObeliskRenderer* renderer, UniformBufferObject* ubo) {
    ObeliskBuffer* uniformBuffer = &renderer->uniformBuffers[renderer->frame];
    obeliskWriteBuffer(uniformBuffer, ubo, uniformBuffer->size);
}

float obeliskGetRendererAspect(const ObeliskRenderer* renderer) {
    return renderer->swapchain.extent.width / (float)renderer->swapchain.extent.height;
}
