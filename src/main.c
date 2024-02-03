#include "minimal/minimal.h"
#include "ignis/ignis.h"

#include "ignis/pipeline.h"
#include "ignis/buffer.h"

#include "math/math.h"

static MinimalWindow* window;

static IgnisPipeline pipeline;
static IgnisBuffer vertexBuffer;
static IgnisBuffer indexBuffer;

static float vertices[] = {
    -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,   1.0f, 1.0f, 1.0f
};

static size_t vertexCount = 4;
static size_t vertexSize = 5 * sizeof(float);

const uint32_t indices[] = {
    0, 1, 2, 2, 3, 0
};

static size_t indexCount = 6;

uint8_t onEvent(void* handler, const MinimalEvent* e);

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
    
    minimalSetCurrentContext(window);
    minimalSetEventHandler(NULL, (MinimalEventCB)onEvent);

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

    VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT,     0 * sizeof(float)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT,  2 * sizeof(float)}
    };
    
    IgnisPipelineConfig pipelineConfig = {
        .vertPath = "./res/vert.spv",
        .fragPath = "./res/frag.spv",
        .vertexAttributes = attributes,
        .attributeCount = 2,
        .vertexStride = 5 * sizeof(float),
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE
    };

    if (!ignisCreatePipeline(&pipelineConfig, &pipeline))
    {
        MINIMAL_CRITICAL("failed to create pipeline");
        return MINIMAL_FAIL;
    }

    // create buffer
    if (!ignisCreateVertexBuffer(vertices, vertexCount * vertexSize, &vertexBuffer))
        return MINIMAL_FAIL;

    if (!ignisCreateIndexBuffer(indices, indexCount, &indexBuffer))
        return MINIMAL_FAIL;

    MINIMAL_INFO("[Minimal] Version: %s", minimalGetVersionString());
    
    return MINIMAL_OK;
}

void onDestroy()
{
    vkDeviceWaitIdle(ignisGetVkDevice());

    ignisDestroyBuffer(&vertexBuffer);
    ignisDestroyBuffer(&indexBuffer);

    ignisDestroyPipeline(&pipeline);

    ignisTerminate();
    MINIMAL_TRACE("Ignis context terminated.");

    minimalDestroyWindow(window);
    minimalPlatformTerminate();
}

uint8_t onEvent(void* handler, const MinimalEvent* e)
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
        VkCommandBuffer commandBuffer = ignisGetCommandBuffer();

        ignisBindPipeline(commandBuffer, &pipeline);

        UniformBufferObject ubo = { 0 };

        //mat4 model = mat4_rotate_z(mat4_identity(), minimalGetTime() * degToRad(90.0f));
        //mat4 view = mat4_look_at((vec3) { 2.0f, 2.0f, 2.0f }, (vec3) { 0.0f }, (vec3) { 0.0f, 0.0f, 1.0f });

        mat4 model = mat4_identity();
        mat4 view = mat4_identity();
        mat4 proj = mat4_identity();

        ignisPipelinePushUniform(&pipeline, &model, sizeof(mat4), 0 * sizeof(mat4));
        ignisPipelinePushUniform(&pipeline, &view, sizeof(mat4), 1 * sizeof(mat4));
        ignisPipelinePushUniform(&pipeline, &proj, sizeof(mat4), 2 * sizeof(mat4));

        VkBuffer vertexBuffers[] = {vertexBuffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        ignisEndFrame();
    }
}

int main(void)
{
    if (onLoad("Vulkan Test", 100, 100, 1280, 720))
        minimalRun(window, (MinimalTickCB)onTick, NULL);

    onDestroy();

    return 0;
}