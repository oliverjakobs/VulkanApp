#include "obelisk.h"

#include "ignis/ignis.h"

#include "ignis/pipeline.h"
#include "ignis/buffer.h"

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

static u8 obeliskOnEvent(ObeliskApp* app, const MinimalEvent* e);

u8 obeliskLoad(ObeliskApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h)
{
    /* minimal initialization */
    if (!minimalPlatformInit())
    {
        MINIMAL_ERROR("[App] Failed to initialize Minimal");
        return MINIMAL_FAIL;
    }

    /* creating the window */
    app->window = minimalCreateWindow(title, x, y, w, h);
    if (!app->window)
    {
        MINIMAL_ERROR("[App] Failed to create Minimal window");
        return MINIMAL_FAIL;
    }
    
    minimalSetCurrentContext(app->window);
    minimalSetEventHandler(app, (MinimalEventCB)obeliskOnEvent);

    IgnisPlatform platform = {
        .createSurface = (ignisCreateSurfaceFn)minimalCreateWindowSurface,
        .queryExtensions = (ignisQueryExtensionFn)minimalQueryRequiredExtensions,
        .context = app->window
    };

    if (!ignisInit("ObeliskApp", &platform))
    {
        MINIMAL_CRITICAL("Failed to create ignis context.");
        return MINIMAL_FAIL;
    }
    MINIMAL_TRACE("Ignis context created successfully.");

    ignisPrintInfo();

    ignisSetClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    IgnisPipelineConfig pipelineConfig = {
        .vertPath = "./obelisk/res/vert.spv",
        .fragPath = "./obelisk/res/frag.spv",
        .renderPass = ignisGetVkRenderPass()
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

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void obeliskDestroy(ObeliskApp* app)
{
    ignisDestroyBuffer(&vertexBuffer);
    ignisDestroyBuffer(&indexBuffer);

    ignisDestroyPipeline(&pipeline);

    ignisTerminate();
    MINIMAL_TRACE("Ignis context terminated.");

    minimalDestroyWindow(app->window);
    minimalPlatformTerminate();
}

u8 obeliskOnEvent(ObeliskApp* app, const MinimalEvent* e)
{
    u32 width, height;
    if (minimalEventWindowSize(e, &width, &height))
    {
        ignisResize(width, height);
        ignisSetViewport(0.0f, 0.0f, width, height);
    }

    return app->on_event(app, e);
}

static void obeliskOnTick(ObeliskApp* app, const MinimalFrameData* framedata)
{
    if (ignisBeginFrame())
    {
        app->on_tick(app, framedata->deltatime);

        VkCommandBuffer commandBuffer = ignisGetCommandBuffer();

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

        VkBuffer vertexBuffers[] = {vertexBuffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        ignisEndFrame();
    }
}

void obeliskRun(ObeliskApp* app)
{
    minimalRun(app->window, (MinimalTickCB)obeliskOnTick, app);

    
    vkDeviceWaitIdle(ignisGetVkDevice());
}