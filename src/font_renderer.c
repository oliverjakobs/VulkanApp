#include "font_renderer.h"

#include "ignis/pipeline.h"
#include "ignis/buffer.h"
#include "ignis/texture.h"

#include <stdarg.h>
#include <stdio.h>


static struct IgnisFontRendererStorage
{
    IgnisBuffer vertexBuffer;
    IgnisBuffer indexBuffer;
    IgnisPipeline pipeline;

    IgnisFont* font;

    float* vertices;
    size_t index;
    size_t quad_count;

    char line_buffer[IGNIS_FONTRENDERER_MAX_LINE_LENGTH];
} render_data;

void ignisGenerateQuadIndices(uint32_t* indices, size_t max)
{
    uint32_t offset = 0;
    for (size_t i = 0; i < max - 6; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;

        offset += 4;
    }
}

uint8_t ignisFontRendererInit()
{
    size_t size = IGNIS_FONTRENDERER_BUFFER_SIZE * sizeof(float);
    if (!ignisCreateBuffer(NULL, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &render_data.vertexBuffer))
    {
        IGNIS_ERROR("failed to create vertex buffer");
        return IGNIS_FAIL;
    }

    vkMapMemory(ignisGetVkDevice(), render_data.vertexBuffer.memory, 0, size, 0, &render_data.vertices);

    uint32_t indices[IGNIS_FONTRENDERER_INDEX_COUNT];
    ignisGenerateQuadIndices(indices, IGNIS_FONTRENDERER_INDEX_COUNT);

    if (!ignisCreateIndexBuffer(indices, IGNIS_FONTRENDERER_INDEX_COUNT, &render_data.indexBuffer))
    {
        IGNIS_ERROR("failed to create index buffer");
        return IGNIS_FAIL;
    }

    render_data.index = 0;
    render_data.quad_count = 0;

    render_data.font = NULL;

    VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT,  0 * sizeof(float)}, /* position */
        {1, 0, VK_FORMAT_R32G32_SFLOAT,  2 * sizeof(float)}  /* texCoord */
    };

    uint32_t attributeCount = sizeof(attributes) / sizeof(VkVertexInputAttributeDescription);
    uint32_t vertexStride = 4 * sizeof(float);
    uint32_t uniformBufferSize = (4 * 4 * sizeof(float));

    IgnisPipelineConfig pipelineConfig = {
        .vertexAttributes = attributes,
        .attributeCount = attributeCount,
        .vertexStride = vertexStride,
        .uniformBufferSize = uniformBufferSize,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkShaderModule vertShader = ignisCreateShaderModule("./res/shader/font.vert.spv");
    VkShaderModule fragShader = ignisCreateShaderModule("./res/shader/font.frag.spv");

    if (!ignisCreatePipeline(&pipelineConfig, vertShader, fragShader, &render_data.pipeline))
    {
        IGNIS_ERROR("failed to create pipeline");
        return IGNIS_FAIL;
    }

    ignisDestroyShaderModule(vertShader);
    ignisDestroyShaderModule(fragShader);

    return IGNIS_OK;
}

void ignisFontRendererDestroy()
{
    ignisDestroyBuffer(&render_data.vertexBuffer);
    ignisDestroyBuffer(&render_data.indexBuffer);

    ignisDestroyPipeline(&render_data.pipeline);
}

void ignisFontRendererBindFont(IgnisFont* font)
{
    render_data.font = font;
}

void ignisFontRendererSetProjection(const float* proj)
{
    ignisPushUniform(&render_data.pipeline, proj, 4 * 4 * sizeof(float), 0);
}


void ignisFontRendererStart(VkCommandBuffer commandBuffer)
{
    ignisBindPipeline(commandBuffer, &render_data.pipeline);
    ignisBindTexture(&render_data.pipeline, render_data.font->texture, 1);
}

void ignisFontRendererFlush(VkCommandBuffer commandBuffer)
{
    if (render_data.index == 0) return;

    VkBuffer vertexBuffers[] = { render_data.vertexBuffer.handle };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, render_data.indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, IGNIS_INDICES_PER_QUAD * render_data.quad_count, 1, 0, 0, 0);

    render_data.index = 0;
    render_data.quad_count = 0;
}

static uint8_t ignisFontRendererLoadGlyph(size_t offset, const IgnisGlyph* glyph, float x, float y, float scale)
{
    if (!glyph) return IGNIS_FAIL;

    float x0 = x + (glyph->x0 * scale);
    float y0 = y + (glyph->y0 * scale);
    float x1 = x + (glyph->x1 * scale);
    float y1 = y + (glyph->y1 * scale);
    render_data.vertices[offset + 0] = x0;
    render_data.vertices[offset + 1] = y0;
    render_data.vertices[offset + 2] = glyph->u0;
    render_data.vertices[offset + 3] = glyph->v0;
    render_data.vertices[offset + 4] = x0;
    render_data.vertices[offset + 5] = y1;
    render_data.vertices[offset + 6] = glyph->u0;
    render_data.vertices[offset + 7] = glyph->v1;
    render_data.vertices[offset + 8] = x1;
    render_data.vertices[offset + 9] = y1;
    render_data.vertices[offset + 10] = glyph->u1;
    render_data.vertices[offset + 11] = glyph->v1;
    render_data.vertices[offset + 12] = x1;
    render_data.vertices[offset + 13] = y0;
    render_data.vertices[offset + 14] = glyph->u1;
    render_data.vertices[offset + 15] = glyph->v0;

    return IGNIS_OK;
}

void ignisFontRendererRenderText(VkCommandBuffer commandBuffer, float x, float y, float height, const char* text)
{
    if (!render_data.font)
    {
        IGNIS_WARN("[FontRenderer] No font bound");
        return;
    }

    float scale = height / render_data.font->size;
    for (size_t i = 0; i < strlen(text); i++)
    {
        if (render_data.index + IGNIS_FONTRENDERER_QUAD_SIZE >= IGNIS_FONTRENDERER_BUFFER_SIZE)
            break;

        const IgnisGlyph* glyph = ignisFontFindGlyph(render_data.font, text[i]);
        if (!ignisFontRendererLoadGlyph(render_data.index, glyph, x, y, scale))
            IGNIS_WARN("[FontRenderer] Failed to load quad for %c", text[i]);

        if (glyph == render_data.font->fallback)
            IGNIS_WARN("[FontRenderer] Used fallback for %c", text[i]);

        x += glyph->xadvance * scale;
        render_data.index += IGNIS_FONTRENDERER_QUAD_SIZE;
        render_data.quad_count++;
    }
}

static void ignisFontRendererRenderTextVA(VkCommandBuffer commandBuffer, float x, float y, float height, const char* fmt, va_list args)
{
    size_t buffer_size = vsnprintf(NULL, 0, fmt, args);
    vsnprintf(render_data.line_buffer, buffer_size + 1, fmt, args);

    ignisFontRendererRenderText(commandBuffer, x, y, height, render_data.line_buffer);
}

void ignisFontRendererRenderTextFmt(VkCommandBuffer commandBuffer, float x, float y, float height, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ignisFontRendererRenderTextVA(commandBuffer, x, y, height, fmt, args);
    va_end(args);
}