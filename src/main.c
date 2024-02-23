#include "minimal/minimal.h"
#include "ignis/ignis.h"

#include "ignis/pipeline.h"
#include "ignis/buffer.h"
#include "ignis/texture.h"

#include "math/math.h"

static MinimalWindow* window;

static IgnisPipeline pipeline;
static IgnisBuffer vertexBuffer;
static IgnisBuffer indexBuffer;
static IgnisTexture texture;

#define VERTEX_SIZE ((3 + 3 + 2) * sizeof(float))

static float vertices[] = {
    -0.5f, -0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     0.5f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f,  0.5f,  0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f
};

static size_t vertexCount = sizeof(vertices) / VERTEX_SIZE;

const uint32_t indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

static size_t indexCount = sizeof(indices) / sizeof(uint32_t);

uint8_t onLoad(const char* title,  int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    /* minimal initialization */
    if (!minimalPlatformInit())
    {
        MINIMAL_ERROR("[App] Failed to initialize Minimal");
        return MINIMAL_FAIL;
    }

    /* creating the window */
    window = minimalCreateWindow(title, x, y, w, h);
    if (!window)
    {
        MINIMAL_ERROR("[App] Failed to create Minimal window");
        return MINIMAL_FAIL;
    }

    IgnisPlatform platform = {
        .createSurface = (ignisCreateSurfaceFn)minimalCreateWindowSurface,
        .queryExtensions = (ignisQueryExtensionFn)minimalQueryRequiredExtensions,
        .context = window
    };

    if (!ignisInit("VulkanApp", &platform))
    {
        MINIMAL_CRITICAL("Failed to create ignis context.");
        return MINIMAL_FAIL;
    }
    MINIMAL_TRACE("Ignis context created successfully.");

    ignisPrintInfo();

    ignisSetClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // create pipeline
    VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT,  0 * sizeof(float)}, /* position */
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT,  3 * sizeof(float)}, /* color */
        {2, 0, VK_FORMAT_R32G32_SFLOAT,     6 * sizeof(float)}  /* texCoord */
    };

    uint32_t attributeCount = sizeof(attributes) / sizeof(VkVertexInputAttributeDescription);

    IgnisPipelineConfig pipelineConfig = {
        .vertPath = "./res/shader/vert.spv",
        .fragPath = "./res/shader/frag.spv",
        .vertexAttributes = attributes,
        .attributeCount = attributeCount,
        .vertexStride = VERTEX_SIZE,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    if (!ignisCreatePipeline(&pipelineConfig, &pipeline))
    {
        MINIMAL_CRITICAL("failed to create pipeline");
        return MINIMAL_FAIL;
    }

    // create buffer
    if (!ignisCreateVertexBuffer(vertices, vertexCount * VERTEX_SIZE, &vertexBuffer))
        return MINIMAL_FAIL;

    if (!ignisCreateIndexBuffer(indices, indexCount, &indexBuffer))
        return MINIMAL_FAIL;

    // create texture
    if (!ignisCreateTexture("./res/texture.png", &texture, NULL))
        return MINIMAL_FAIL;

    MINIMAL_INFO("[Minimal] Version: %s", minimalGetVersionString());
    
    return MINIMAL_OK;
}

void onDestroy()
{
    vkDeviceWaitIdle(ignisGetVkDevice());

    ignisDestroyTexture(&texture);

    ignisDestroyBuffer(&vertexBuffer);
    ignisDestroyBuffer(&indexBuffer);

    ignisDestroyPipeline(&pipeline);

    ignisTerminate();
    MINIMAL_TRACE("Ignis context terminated.");

    minimalDestroyWindow(window);
    minimalPlatformTerminate();
}

uint8_t onEvent(void* context, const MinimalEvent* e)
{
    uint32_t width, height;
    if (minimalEventWindowSize(e, &width, &height))
    {
        ignisResize(width, height);
        ignisSetViewport(0.0f, 0.0f, width, height);
    }

    if (minimalEventKeyPressed(e) == MINIMAL_KEY_ESCAPE)
        minimalClose(window);

    return MINIMAL_OK;
}

void onTick(void* context, const MinimalFrameData* framedata)
{
    if (ignisBeginFrame())
    {
        VkCommandBuffer commandBuffer = ignisBeginCommandBuffer();

        ignisBindPipeline(commandBuffer, &pipeline);

        ignisBindTexture(&pipeline, &texture, 1);

        UniformBufferObject ubo = { 0 };

        mat4 model = mat4_rotation((vec3) { 0.0f, 0.0f, 1.0f }, minimalGetTime() * degToRad(90.0f));
        mat4 view = mat4_look_at((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f }, (vec3) { 0.0f, 0.0f, 1.0f });
        mat4 proj = mat4_perspective(degToRad(45.0f), ignisGetAspectRatio(), 0.1f, 10.0f);
        proj.v[1][1] *= -1;

        // mat4 model = mat4_identity();
        // mat4 view = mat4_identity();
        // mat4 proj = mat4_identity();

        ignisPushUniform(&pipeline, &model, sizeof(mat4), 0 * sizeof(mat4));
        ignisPushUniform(&pipeline, &view, sizeof(mat4), 1 * sizeof(mat4));
        ignisPushUniform(&pipeline, &proj, sizeof(mat4), 2 * sizeof(mat4));


        VkBuffer vertexBuffers[] = {vertexBuffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        ignisEndCommandBuffer(commandBuffer);

        ignisEndFrame();
    }
}

int main(void)
{
    if (onLoad("Vulkan Test", 100, 100, 1280, 720))
    {
        minimalSetCurrentContext(window);
        minimalSetEventHandler(NULL, (MinimalEventCB)onEvent);
        minimalRun(window, (MinimalTickCB)onTick, NULL);
    }

    onDestroy();

    return 0;
}