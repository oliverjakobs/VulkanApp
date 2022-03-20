#include "Application.h"

#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Frame.h"
#include "Buffer.h"

const int debug = 1;

Buffer vertexBuffer;
Buffer indexBuffer;

const Vertex vertices[] = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
};

uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);

const uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };
uint32_t indexCount = sizeof(indices) / sizeof(indices[0]);

Pipeline pipeline;

int OnLoad(MinimalApp* app, uint32_t w, uint32_t h) {
    if (!createInstance(&app->context, app->window, "VulkanApp", "Ignis", debug)) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* pick physical device */
    if (!pickPhysicalDevice(&app->context)) {
        MINIMAL_ERROR("failed to find a suitable GPU!");
        return MINIMAL_FAIL;
    }

    /* create logical device */
    if (!createLogicalDevice(&app->context)) {
        MINIMAL_ERROR("failed to create logical device!");
        return MINIMAL_FAIL;
    }

    /* create swap chain */
    int width, height;
    glfwGetFramebufferSize(app->window, &width, &height);

    if (!createSwapchain(&app->context, &app->context.swapchain, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(&app->context, &app->context.swapchain)) {
        MINIMAL_ERROR("failed to create render pass!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(&app->context, &app->context.swapchain)) {
        MINIMAL_ERROR("failed to create framebuffer!");
        return MINIMAL_FAIL;
    }

    /* create pipeline */
    if (!createShaderStages(&app->context, &pipeline, "res/shader/vert.spv", "res/shader/frag.spv")) {
        MINIMAL_ERROR("failed to create shader stages!");
        return MINIMAL_FAIL;
    }

    if (!createPipeline(&app->context, &pipeline)) {
        MINIMAL_ERROR("failed to create graphics pipeline!");
        return MINIMAL_FAIL;
    }

    if (!createCommandPool(&app->context)) {
        MINIMAL_ERROR("failed to create command pool!");
        return MINIMAL_FAIL;
    }

    if (!createVertexBuffer(&app->context, &vertexBuffer, vertices, vertexCount)) {
        MINIMAL_ERROR("failed to create vertex buffer!");
        return MINIMAL_FAIL;
    }

    if (!createIndexBuffer(&app->context, &indexBuffer, indices, indexCount)) {
        MINIMAL_ERROR("failed to create index buffer!");
        return MINIMAL_FAIL;
    }

    if (!createCommandBuffer(&app->context)) {
        MINIMAL_ERROR("failed to allocate command buffers!");
        return MINIMAL_FAIL;
    }

    if (!createSyncObjects(&app->context)) {
        MINIMAL_ERROR("failed to create synchronization objects for a frame!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {
    destroySwapchain(&app->context, &app->context.swapchain);

    destroyShaderStages(&app->context, &pipeline);
    destroyPipeline(&app->context, &pipeline);

    destroyBuffer(&app->context, &vertexBuffer);
    destroyBuffer(&app->context, &indexBuffer);

    destroySyncObjects(&app->context);

    vkDestroyCommandPool(app->context.device, app->context.commandPool, NULL);

    /* destroy device */
    vkDestroyDevice(app->context.device, NULL);

    destroyInstance(&app->context);
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);
    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, float deltatime) {
    uint32_t frame = app->context.currentFrame;

    /* acquire swap chain image */
    vkWaitForFences(app->context.device, 1, &app->context.inFlightFences[frame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(app->context.device, app->context.swapchain.handle, UINT64_MAX, app->context.imageAvailableSemaphores[frame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(&app->context, &app->context.swapchain, app->window);
        recreatePipeline(&app->context, &pipeline);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        MINIMAL_ERROR("failed to acquire swap chain image!");
        return;
    }

    vkResetFences(app->context.device, 1, &app->context.inFlightFences[frame]);

    /* start frame */
    VkCommandBuffer cmdBuffer = app->context.commandBuffers[frame];
    commandBufferStart(cmdBuffer, &app->context.swapchain, imageIndex);

    /* actual rendering */
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

    VkBuffer vertexBuffers[] = { vertexBuffer.handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);

    /* end frame */
    commandBufferEnd(cmdBuffer);

    /* submit frame */
    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { app->context.imageAvailableSemaphores[frame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.commandBufferCount = 1;

    VkSemaphore signalSemaphores[] = { app->context.renderFinishedSemaphores[frame] };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    if (vkQueueSubmit(app->context.graphicsQueue, 1, &submitInfo, app->context.inFlightFences[frame]) != VK_SUCCESS) {
        MINIMAL_WARN("failed to submit draw command buffer!");
    }

    /* present frame */
    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { app->context.swapchain.handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(app->context.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->context.framebufferResized) {
        app->context.framebufferResized = 0;
        recreateSwapchain(&app->context, &app->context.swapchain, app->window);
        recreatePipeline(&app->context, &pipeline);
    } else if (result != VK_SUCCESS) {
        MINIMAL_ERROR("failed to present swap chain image!");
        return;
    }

    /* next frame */
    app->context.currentFrame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main() {
    MinimalApp app = {
        .on_load = OnLoad,
        .on_destroy = OnDestroy,
        .on_event = OnEvent,
        .on_update = OnUpdate
    };

    if (MinimalLoad(&app, "VulkanApp", 1024, 800))
        MinimalRun(&app);

    MinimalDestroy(&app);

    return 0;
}