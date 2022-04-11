#include "core/Application.h"
#include "core/memory.h"

#include "graphics/Swapchain.h"
#include "graphics/Pipeline.h"
#include "graphics/Buffer.h"

#include "Utils.h"

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
    { { -0.5f, -0.5f, -0.5f }, { 0.9f, 0.9f, 0.9f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.9f, 0.9f, 0.9f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.9f, 0.9f, 0.9f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.9f, 0.9f, 0.9f } },
    // right face (yellow)
    { {  0.5f, -0.5f, -0.5f }, { 0.8f, 0.8f, 0.1f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.8f, 0.8f, 0.1f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.8f, 0.8f, 0.1f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.8f, 0.8f, 0.1f } },
    // top face (orange, remember y axis points down)
    { { -0.5f, -0.5f, -0.5f }, { 0.9f, 0.6f, 0.1f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.9f, 0.6f, 0.1f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.9f, 0.6f, 0.1f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.9f, 0.6f, 0.1f } },
    // bottom face (red)
    { { -0.5f,  0.5f, -0.5f }, { 0.8f, 0.1f, 0.1f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.8f, 0.1f, 0.1f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.8f, 0.1f, 0.1f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.8f, 0.1f, 0.1f } },
    // nose face (blue)
    { { -0.5f, -0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f } },
    // tail face (green)
    { { -0.5f, -0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f } },
};

uint32_t vertexCount = OBELISK_ARRAY_LEN(vertices);

const uint32_t indices[] = {
     0,  1,  2,  0,  3,  1,
     4,  5,  6,  4,  7,  5,
     8,  9,  10, 8,  11, 9,
    12, 13, 14, 12, 15, 13,
    16, 17, 18, 16, 19, 17,
    20, 21, 22, 20, 23, 21
};

uint32_t indexCount = OBELISK_ARRAY_LEN(indices);

ObeliskBuffer vertexBuffer;
ObeliskBuffer indexBuffer;

vec3 rotation = { 0.0f };

int OnLoad(ObeliskApp* app, uint32_t width, uint32_t height) {
    if (!obeliskCreateContext(app->window, "VulkanApp", debug)) {
        OBELISK_ERROR("Failed to create vulkan instance!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreateRenderer(&app->renderer, width, height)) {
        OBELISK_ERROR("Failed to create renderer!");
    }

    if (!obeliskCreateDescriptorPool(&app->renderer)) {
        OBELISK_ERROR("failed to create descriptor pool!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreateDescriptorLayout(&app->renderer)) {
        OBELISK_ERROR("failed to create descriptor set layout!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreateDescriptorSets(&app->renderer)) {
        OBELISK_ERROR("failed to allocate descriptor sets!");
        return OBELISK_FAIL;
    }

    /* create pipeline */
    if (!obeliskCreateShaderStages(&pipeline, "res/shader/vert.spv", "res/shader/frag.spv")) {
        OBELISK_ERROR("failed to create shader stages!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreatePipelineLayout(&pipeline, app->renderer.descriptorSetLayout, sizeof(mat4))) {
        OBELISK_ERROR("Failed to create pipeline layout!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreatePipeline(&pipeline, app->renderer.swapchain.renderPass, &pipelineVertexLayout)) {
        OBELISK_ERROR("failed to create graphics pipeline!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreateVertexBuffer(&vertexBuffer, vertices, vertexCount, sizeof(Vertex))) {
        OBELISK_ERROR("failed to create vertex buffer!");
        return OBELISK_FAIL;
    }

    if (!obeliskCreateIndexBuffer(&indexBuffer, indices, indexCount)) {
        OBELISK_ERROR("failed to create index buffer!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void OnDestroy(ObeliskApp* app) {
    /* destroy pipeline */
    obeliskDestroyShaderStages(&pipeline);
    obeliskDestroyPipelineLayout(&pipeline);
    obeliskDestroyPipeline(&pipeline);

    /* destroy buffers */
    obeliskDestroyBuffer(&vertexBuffer);
    obeliskDestroyBuffer(&indexBuffer);

    /* destroy renderer */
    obeliskDestroyDescriptorLayout(&app->renderer);
    obeliskDestroyDescriptorPool(&app->renderer);

    obeliskDestroyRenderer(&app->renderer);

    obeliskDestroyContext();
}

int OnEvent(ObeliskApp* app, const ObeliskEvent* e) {
    if (obeliskEventKeyPressed(e) == GLFW_KEY_ESCAPE) obeliskClose(app);

    uint32_t width, height;
    if (obeliskEventFramebufferSize(e, &width, &height)) {
        if (width == 0 || height == 0) return OBELISK_FAIL;

        obeliskRecreateSwapchain(&app->renderer.swapchain, width, height);
        obeliskRecreatePipeline(&pipeline, app->renderer.swapchain.renderPass);
    }

    return OBELISK_OK;
}

void OnUpdate(ObeliskApp* app, VkCommandBuffer cmdBuffer, float deltatime) {
    UniformBufferObject ubo = { 0 };
    glm_lookat((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 1.0f }, ubo.view);
    glm_perspective(glm_rad(45.0f), obeliskGetRendererAspect(&app->renderer), 0.1f, 10.0f, ubo.proj);

    obeliskWriteUniform(&app->renderer, &ubo);

    rotation[0] += 0.5f * deltatime;
    rotation[1] += 1.0f * deltatime;

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_euler_yxz(rotation, model);

    vkCmdPushConstants(cmdBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);

    obeliskBeginRenderPass(&app->renderer, cmdBuffer);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &app->renderer.descriptorSets[app->renderer.frame], 0, NULL);

    VkBuffer vertexBuffers[] = { vertexBuffer.handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);

    // vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);

    obeliskEndRenderPass(&app->renderer, cmdBuffer);
}

#include "utility/array.h"

int main() {

    /* array test */
    typedef struct {
        int val;
    } element;

    element* arr = NULL;
    stbds_arrreserve(arr, 5);
    for (int i = 0; i < 10; ++i) {
        element e = { .val = i };
        obeliskArrayPush(arr, e);
    }

    element e = { .val = 42 };
    stbds_arrins(arr, 5, e);

    stbds_arrdel(arr, 3);

    for (int i = 0; i < obeliskArrayLen(arr); ++i) {
        OBELISK_INFO("%i: %i", i, arr[i].val);
    }

    obeliskArrayPop(arr);
    OBELISK_INFO("Last: %i", obeliskArrayLast(arr)->val);

    return 0;

    ObeliskApp app = {
        .on_load = OnLoad,
        .on_destroy = OnDestroy,
        .on_event = OnEvent,
        .on_update = OnUpdate
    };

    if (obeliskLoad(&app, "VulkanApp", 1024, 800))
        obeliskRun(&app);

    obeliskDestroy(&app);

    size_t bytes = obeliskMemoryGetBytes();
    if (bytes != 0) OBELISK_WARN("%llu bytes not freed", bytes);

    return 0;
}

/*
  Dynamic Arrays

    Functions (actually macros)


      arrreserve:
        size_t arrsetcap(T* a, int n);
          Sets the length of allocated storage to at least n. It will not
          change the length of the array.
      arrfree:
        void arrfree(T*);
          Frees the array.
      arrcap:
        size_t arrcap(T* a);
          Returns the number of total elements the array can contain without
          needing to be reallocated.
      arrlen:
        ptrdiff_t arrlen(T*);
          Returns the number of elements in the array.
      arrpop:
        T arrpop(T* a)
          Removes the final element of the array and returns it.
      arrpush:
        T arrpush(T* a, T b);
          Appends the item b to the end of array a. Returns b.


      arrins:
        T arrins(T* a, int p, T b);
          Inserts the item b into the middle of array a, into a[p],
          moving the rest of the array over. Returns b.
      arrinsn:
        void arrinsn(T* a, int p, int n);
          Inserts n uninitialized items into array a starting at a[p],
          moving the rest of the array over.
      arraddnptr:
        T* arraddnptr(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns a pointer to the first uninitialized item added.
      arraddnindex:
        size_t arraddnindex(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns the index of the first uninitialized item added.
      arrdel:
        void arrdel(T* a, int p);
          Deletes the element at a[p], moving the rest of the array over.
      arrdeln:
        void arrdeln(T* a, int p, int n);
          Deletes n elements starting at a[p], moving the rest of the array over.

*/