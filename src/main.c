#include "Application.h"

#include "Swapchain.h"
#include "Pipeline.h"
#include "Buffer.h"

#include "cglm/cglm.h"

const int debug = 1;

typedef struct {
    vec3 pos;
    vec3 color;
} Vertex;

static VkVertexInputAttributeDescription pipelineVertexAttributes[] = {
    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) },
    { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) }
};

ObeliskPipeline pipeline;

static ObeliskVertexLayout pipelineVertexLayout = {
    .stride = sizeof(Vertex),
    .attributes = pipelineVertexAttributes,
    .attributeCount = sizeof(pipelineVertexAttributes) / sizeof(pipelineVertexAttributes[0])
};

const Vertex vertices[] = {

    // left face (white)
    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

    // right face (yellow)
    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
    {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
    {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
    {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
    {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

    // top face (orange, remember y axis points down)
    {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
    {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
    {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
    {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

    // bottom face (red)
    {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
    {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
    {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
    {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
    {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
    {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

    // nose face (blue)
    {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
    {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
    {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
    {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
    {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
    {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

    // tail face (green)
    {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
    {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
    {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

};

uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);


const uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };
uint32_t indexCount = sizeof(indices) / sizeof(indices[0]);

Buffer vertexBuffer;
Buffer indexBuffer;

int OnLoad(MinimalApp* app, uint32_t w, uint32_t h) {
    if (!obeliskCreateContext(app->window, "VulkanApp", debug)) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    if (!obeliskCreateRenderer(&app->renderer, app->window)) {
        MINIMAL_ERROR("Failed to create renderer!");
    }

    if (!obeliskCreateDescriptorPool(&app->renderer)) {
        MINIMAL_ERROR("failed to create descriptor pool!");
        return MINIMAL_FAIL;
    }

    if (!obeliskCreateDescriptorLayout(&app->renderer)) {
        MINIMAL_ERROR("failed to create descriptor set layout!");
        return MINIMAL_FAIL;
    }

    if (!obeliskCreateDescriptorSets(&app->renderer)) {
        MINIMAL_ERROR("failed to allocate descriptor sets!");
        return MINIMAL_FAIL;
    }

    /* create pipeline */
    if (!createShaderStages(&pipeline, "res/shader/vert.spv", "res/shader/frag.spv")) {
        MINIMAL_ERROR("failed to create shader stages!");
        return MINIMAL_FAIL;
    }

    if (!createPipelineLayout(&pipeline, app->renderer.descriptorSetLayout)) {
        MINIMAL_ERROR("Failed to create pipeline layout!");
        return MINIMAL_FAIL;
    }

    if (!createPipeline(&pipeline, app->renderer.swapchain.renderPass, &pipelineVertexLayout)) {
        MINIMAL_ERROR("failed to create graphics pipeline!");
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

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {
    /* destroy pipeline */
    destroyShaderStages(&pipeline);
    destroyPipelineLayout(&pipeline);
    destroyPipeline(&pipeline);

    /* destroy buffers */
    destroyBuffer(&vertexBuffer);
    destroyBuffer(&indexBuffer);

    /* destroy renderer */
    obeliskDestroyDescriptorLayout(&app->renderer);
    obeliskDestroyDescriptorPool(&app->renderer);

    obeliskDestroyRenderer(&app->renderer);

    obeliskDestroyContext();
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);

    uint32_t width, height;
    if (MinimalEventFramebufferSize(e, &width, &height)) {

        if (width == 0 || height == 0) return MINIMAL_FAIL;

        obeliskRecreateSwapchain(&app->renderer.swapchain, width, height);
        recreatePipeline(&pipeline, app->renderer.swapchain.renderPass);
    }

    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, VkCommandBuffer cmdBuffer, float deltatime) {
    static float time;
    time += deltatime;

    VkExtent2D extent = app->renderer.swapchain.extent;

    UniformBufferObject ubo = { 0 };
    glm_lookat((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 1.0f }, ubo.view);
    float aspect = extent.width / (float)extent.height;
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1;

    obeliskWriteUniform(&app->renderer, &ubo);

    mat4 model = { 0 };
    glm_rotate_make(model, time * glm_rad(90.0f), (vec3) { 0.0f, 0.0f, 1.0f });

    vkCmdPushConstants(cmdBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);

    obeliskBeginRenderPass(&app->renderer, cmdBuffer);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &app->renderer.descriptorSets[app->renderer.frame], 0, NULL);

    VkBuffer vertexBuffers[] = { vertexBuffer.handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    // vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT16);
    // vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);

    vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);

    obeliskEndRenderPass(&app->renderer, cmdBuffer);
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