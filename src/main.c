#include "Application.h"

#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Buffer.h"

#include "cglm/cglm.h"

const int debug = 1;

Buffer vertexBuffer;
Buffer indexBuffer;

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

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

    printPhysicalDevice(app->context.physicalDevice);

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

    /* create uniform buffers */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!createUniformBuffer(&app->context, &app->context.uniformBuffers[i], sizeof(UniformBufferObject))) {
            MINIMAL_ERROR("failed to create uniform buffer!");
            return MINIMAL_FAIL;
        }
    }

    if (!createDescriptorPool(&app->context)) {
        MINIMAL_ERROR("failed to create descriptor pool!");
        return MINIMAL_FAIL;
    }

    if (!createDescriptorSets(&app->context)) {
        return MINIMAL_FAIL;
    }

    /* create pipeline */
    if (!createShaderStages(&app->context, &pipeline, "res/shader/vert.spv", "res/shader/frag.spv")) {
        MINIMAL_ERROR("failed to create shader stages!");
        return MINIMAL_FAIL;
    }

    if (!createPipelineLayout(&app->context, &pipeline)) {
        MINIMAL_ERROR("Failed to create pipeline layout!");
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

    if (!createSyncObjects(&app->context, &app->context.swapchain)) {
        MINIMAL_ERROR("failed to create synchronization objects for a frame!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {
    destroyShaderStages(&app->context, &pipeline);
    destroyPipelineLayout(&app->context, &pipeline);
    destroyPipeline(&app->context, &pipeline);

    destroyDescriptorSets(&app->context);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        destroyBuffer(&app->context, &app->context.uniformBuffers[i]);
    }

    destroyBuffer(&app->context, &vertexBuffer);
    destroyBuffer(&app->context, &indexBuffer);

    destroySwapchain(&app->context, &app->context.swapchain);

    destroySyncObjects(&app->context, &app->context.swapchain);

    vkDestroyDescriptorPool(app->context.device, app->context.descriptorPool, NULL);
    vkDestroyCommandPool(app->context.device, app->context.commandPool, NULL);

    /* destroy device */
    vkDestroyDevice(app->context.device, NULL);

    destroyInstance(&app->context);
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);

    uint32_t width, height;
    if (MinimalEventFramebufferSize(e, &width, &height)) {

        if (width == 0 || height == 0) return MINIMAL_FAIL;

        recreateSwapchain(&app->context, &app->context.swapchain, app->window);
        recreatePipeline(&app->context, &pipeline);
    }

    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, VkCommandBuffer cmdBuffer, uint32_t frame, float deltatime) {
    static float time;
    time += deltatime;

    UniformBufferObject ubo = { 0 };
    glm_rotate_make(ubo.model, time * glm_rad(90.0f), (vec3){ 0.0f, 0.0f, 1.0f });
    glm_lookat((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 1.0f }, ubo.view);
    float aspect = app->context.swapchain.extent.width / (float)app->context.swapchain.extent.height;
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1;

    writeBuffer(&app->context, &app->context.uniformBuffers[frame], &ubo, sizeof(ubo));

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &app->context.descriptorSets[frame], 0, NULL);

    VkBuffer vertexBuffers[] = { vertexBuffer.handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);
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