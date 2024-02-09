#ifndef IGNIS_FONT_H
#define IGNIS_FONT_H

#include "texture.h"

typedef uint32_t IgnisRune;

#define IGNIS_ASSERT(expr)

typedef struct
{
    IgnisRune codepoint;
    float xadvance;
    float x0, y0, x1, y1, w, h;
    float u0, v0, u1, v1;
} IgnisGlyph;

typedef struct
{
    const IgnisTexture* texture;
    float size;

    IgnisGlyph* glyphs;
    const IgnisGlyph* fallback; /* fallback glyph to use if a given rune is not found */

    const IgnisRune* range; /* list of unicode ranges (2 values per range, zero terminated) */
} IgnisFont;

const IgnisGlyph* ignisFontFindGlyph(const IgnisFont* font, IgnisRune unicode);

typedef enum
{
    IGNIS_FONT_COORD_UV, /* texture coordinates inside font glyphs are clamped between 0-1 */
    IGNIS_FONT_COORD_PIXEL /* texture coordinates inside font glyphs are in absolute pixel */
} IgnisFontCoordType;

typedef struct
{
    const void* ttf_blob;   /* pointer to loaded TTF file memory block. */
    size_t ttf_size;        /* size of the loaded TTF file memory block */

    unsigned char pixel_snap; /* align every character to pixel boundary (if true set oversample (1,1)) */
    unsigned char oversample_v, oversample_h; /* rasterize at high quality for sub-pixel position */

    IgnisFontCoordType coord_type; /* texture coordinate format with either pixel or UV coordinates */

    float size; /* pixel height of the font */

    size_t glyph_offset;        /* glyph array offset inside the font glyph baking output array  */
    const IgnisRune* range;     /* list of unicode ranges (2 values per range, zero terminated) */
    IgnisRune fallback_glyph;   /* fallback glyph to use if a given rune is not found */
} IgnisFontConfig;

typedef struct
{
    IgnisFont* fonts;
    size_t font_count;

    IgnisGlyph* glyphs;
    size_t glyph_count;

    IgnisTexture texture;
} IgnisFontAtlas;

uint8_t ignisFontAtlasLoadFromFile(IgnisFontConfig* config, const char* path, float height);
uint8_t ignisFontAtlasLoadFromMemory(IgnisFontConfig* config, const void* data, size_t size, float height);

void ignisFontConfigClear(IgnisFontConfig* config, size_t count);

typedef enum
{
    IGNIS_FONT_FORMAT_ALPHA8,
    IGNIS_FONT_FORMAT_RGBA32
} IgnisFontFormat;

uint8_t ignisFontAtlasBake(IgnisFontAtlas* atlas, IgnisFontConfig* configs, size_t count, IgnisFontFormat fmt);
void ignisFontAtlasClear(IgnisFontAtlas* atlas);

/* some language glyph codepoint ranges */
const IgnisRune* ignisGlyphRangeDefault();
const IgnisRune* ignisGlyphRangeChinese();
const IgnisRune* ignisGlyphRangeKorean();
const IgnisRune* ignisGlyphRangeCyrillic();

#endif // !IGNIS_FONT_H
