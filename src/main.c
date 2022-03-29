#include "Application.h"

#include "Swapchain.h"
#include "Pipeline.h"
#include "Buffer.h"

#include "cglm/cglm.h"

const int debug = 1;

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct {
    float pos[2];
    float color[3];
} Vertex;

static VkVertexInputBindingDescription vertexBindingDescs[] = {{
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
}};

static VkVertexInputAttributeDescription vertexAttributeDescs[] = {{
    .binding = 0,
    .location = 0,
    .format = VK_FORMAT_R32G32_SFLOAT,
    .offset = offsetof(Vertex, pos)
}, {
    .binding = 0,
    .location = 1,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = offsetof(Vertex, color)
}};

static ObeliskPipelineVertexLayout pipelineLayout = {
    .vertexInputBindings = vertexBindingDescs,
    .vertexInputBindingCount = sizeof(vertexBindingDescs) / sizeof(VkVertexInputBindingDescription),
    .vertexInputAttributes = vertexAttributeDescs,
    .vertexInputAttributeCount = sizeof(vertexAttributeDescs) / sizeof(VkVertexInputAttributeDescription)
};

Pipeline pipeline;

const Vertex vertices[] = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
};

uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);

const uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };
uint32_t indexCount = sizeof(indices) / sizeof(indices[0]);

Buffer vertexBuffer;
Buffer indexBuffer;

int OnLoad(MinimalApp* app, uint32_t w, uint32_t h) {
    if (!obeliskCreateContext(app->window, "VulkanApp", "obelisk", debug)) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* create swap chain */
    int width, height;
    glfwGetFramebufferSize(app->window, &width, &height);

    if (!createSwapchain(&app->swapchain, VK_NULL_HANDLE, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(&app->swapchain)) {
        MINIMAL_ERROR("failed to create render pass!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(&app->swapchain)) {
        MINIMAL_ERROR("failed to create framebuffer!");
        return MINIMAL_FAIL;
    }

    /* create uniform buffers */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!createUniformBuffer(&app->swapchain.uniformBuffers[i], sizeof(UniformBufferObject))) {
            MINIMAL_ERROR("failed to create uniform buffer!");
            return MINIMAL_FAIL;
        }
    }

    if (!createDescriptorPool(&app->swapchain)) {
        MINIMAL_ERROR("failed to create descriptor pool!");
        return MINIMAL_FAIL;
    }

    if (!createDescriptorLayout(&app->swapchain)) {
        MINIMAL_ERROR("failed to create descriptor set layout!");
        return MINIMAL_FAIL;
    }

    if (!createDescriptorSets(&app->swapchain)) {
        MINIMAL_ERROR("failed to allocate descriptor sets!");
        return MINIMAL_FAIL;
    }

    /* create pipeline */
    if (!createShaderStages(&pipeline, "res/shader/vert.spv", "res/shader/frag.spv")) {
        MINIMAL_ERROR("failed to create shader stages!");
        return MINIMAL_FAIL;
    }

    if (!createPipelineLayout(&pipeline, &app->swapchain, &pipelineLayout)) {
        MINIMAL_ERROR("Failed to create pipeline layout!");
        return MINIMAL_FAIL;
    }

    if (!createPipeline(&pipeline, app->swapchain.renderPass)) {
        MINIMAL_ERROR("failed to create graphics pipeline!");
        return MINIMAL_FAIL;
    }

    if (!obeliskCreateCommandPool()) {
        MINIMAL_ERROR("failed to create command pool!");
        return MINIMAL_FAIL;
    }

    if (!createVertexBuffer(&vertexBuffer, vertices, vertexCount * sizeof(vertices))) {
        MINIMAL_ERROR("failed to create vertex buffer!");
        return MINIMAL_FAIL;
    }

    if (!createIndexBuffer(&indexBuffer, indices, indexCount)) {
        MINIMAL_ERROR("failed to create index buffer!");
        return MINIMAL_FAIL;
    }

    if (obeliskAllocateCommandBuffers(app->swapchain.commandBuffers, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate command buffers!");
        return MINIMAL_FAIL;
    }

    if (!createSyncObjects(&app->swapchain)) {
        MINIMAL_ERROR("failed to create synchronization objects for a frame!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {
    /* destroy pipeline */
    destroyShaderStages(&pipeline);
    destroyPipelineLayout(&pipeline);
    destroyPipeline(&pipeline);

    destroyBuffer(&vertexBuffer);
    destroyBuffer(&indexBuffer);

    /* destroy swapchain */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        destroyBuffer(&app->swapchain.uniformBuffers[i]);
    }

    destroyDescriptorLayout(&app->swapchain);
    destroyDescriptorPool(&app->swapchain);
    destroySyncObjects(&app->swapchain);
    destroySwapchain(&app->swapchain);

    obeliskDestroyCommandPool();
    obeliskDestroyContext();
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);

    uint32_t width, height;
    if (MinimalEventFramebufferSize(e, &width, &height)) {

        if (width == 0 || height == 0) return MINIMAL_FAIL;

        recreateSwapchain(&app->swapchain, width, height);
        recreatePipeline(&pipeline, app->swapchain.renderPass);
    }

    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, VkCommandBuffer cmdBuffer, uint32_t frame, float deltatime) {
    static float time;
    time += deltatime;

    VkExtent2D extent = app->swapchain.extent;

    UniformBufferObject ubo = { 0 };
    glm_rotate_make(ubo.model, time * glm_rad(90.0f), (vec3){ 0.0f, 0.0f, 1.0f });
    glm_lookat((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 1.0f }, ubo.view);
    float aspect = extent.width / (float)extent.height;
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1;

    writeBuffer(&app->swapchain.uniformBuffers[frame], &ubo, sizeof(ubo));

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = extent
    };

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &app->swapchain.descriptorSets[frame], 0, NULL);

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