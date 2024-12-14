#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

#include "ignis/ignis.h"
#include "ignis/font.h"

#define IGNIS_VERTICES_PER_QUAD  4
#define IGNIS_INDICES_PER_QUAD   6


/* font renderer */
#define IGNIS_FONTRENDERER_MAX_QUADS        512
#define IGNIS_FONTRENDERER_VERTEX_SIZE      (2 + 2) /* 2f: vec; 2f: tex */

#define IGNIS_FONTRENDERER_QUAD_SIZE    (IGNIS_VERTICES_PER_QUAD * IGNIS_FONTRENDERER_VERTEX_SIZE)
#define IGNIS_FONTRENDERER_INDEX_COUNT  (IGNIS_FONTRENDERER_MAX_QUADS * IGNIS_INDICES_PER_QUAD)
#define IGNIS_FONTRENDERER_BUFFER_SIZE  (IGNIS_FONTRENDERER_MAX_QUADS * IGNIS_FONTRENDERER_QUAD_SIZE)

#define IGNIS_FONTRENDERER_MAX_LINE_LENGTH    128


uint8_t ignisFontRendererInit();
void ignisFontRendererDestroy();

void ignisFontRendererBindFont(IgnisFont* font);

void ignisFontRendererSetProjection(const float* proj);

void ignisFontRendererStart(VkCommandBuffer commandBuffer);
void ignisFontRendererFlush(VkCommandBuffer commandBuffer);

void ignisRenderText(VkCommandBuffer commandBuffer, float x, float y, float height, const char* text);
void ignisRenderTextFmt(VkCommandBuffer commandBuffer, float x, float y, float height, const char* fmt, ...);


#endif // !FONT_RENDERER_H
