#include "font.h"

#include "ignis.h"

/*
 * ==============================================================
 *
 *                          stb
 *
 * ===============================================================
 */
#define STB_RECT_PACK_IMPLEMENTATION
#include "external/stb_rect_pack.h"

#define STBTT_MAX_OVERSAMPLE   8
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

/*
 * ==============================================================
 *
 *                          Ranges
 *
 * ===============================================================
 */
static size_t ignisRangeCount(const IgnisRune* range)
{
    IGNIS_ASSERT(range);
    if (!range) return 0;

    const IgnisRune* rune = range;
    while (*(rune++) != 0);
    return (rune == range) ? 0 : (size_t)((rune - range) / 2);
}

static size_t ignisRangeGlyphCount(const IgnisRune* range, size_t count)
{
    size_t total_glyphs = 0;
    for (size_t i = 0; i < count; ++i)
    {
        IgnisRune f = range[(i * 2) + 0];
        IgnisRune t = range[(i * 2) + 1];
        IGNIS_ASSERT(t >= f);
        total_glyphs += (int)((t - f) + 1);
    }
    return total_glyphs;
}

const IgnisRune* ignisGlyphRangeDefault()
{
    static const IgnisRune ranges[] = { 0x0020, 0x00FF, 0 };
    return ranges;
}

const IgnisRune* ignisGlyphRangeChinese()
{
    static const IgnisRune ranges[] = {
        0x0020, 0x00FF,
        0x3000, 0x30FF,
        0x31F0, 0x31FF,
        0xFF00, 0xFFEF,
        0x4E00, 0x9FAF,
        0
    };
    return ranges;
}

const IgnisRune* ignisGlyphRangeCyrillic()
{
    static const IgnisRune ranges[] = {
        0x0020, 0x00FF,
        0x0400, 0x052F,
        0x2DE0, 0x2DFF,
        0xA640, 0xA69F,
        0
    };
    return ranges;
}

const IgnisRune* ignisGlyphRangeKorean()
{
    static const IgnisRune ranges[] = {
        0x0020, 0x00FF,
        0x3131, 0x3163,
        0xAC00, 0xD79D,
        0
    };
    return ranges;
}

/*
 * ==============================================================
 *
 *                          Font Baking
 *
 * ===============================================================
 */
typedef struct
{
    stbtt_fontinfo info;
    stbrp_rect* rects;
    stbtt_pack_range* ranges;
    size_t range_count;
} IgnisBakeData;

typedef struct
{
    stbtt_pack_context spc;
    IgnisBakeData *build;
    stbtt_packedchar *packed;
    stbrp_rect *rects;
    stbtt_pack_range *ranges;
} IgnisFontBaker;

static uint8_t ignisFontBakerAlloc(IgnisFontBaker* baker, size_t fonts, size_t glyphs, size_t ranges)
{
    size_t size = sizeof(IgnisBakeData) * fonts;
    baker->build =  malloc(size);
    if (!baker->build) return IGNIS_FAIL;
    memset(baker->build, 0, size);

    size = sizeof(stbtt_packedchar) * glyphs;
    baker->packed = malloc(size);
    if (!baker->packed) return IGNIS_FAIL;
    memset(baker->packed, 0, size);

    size = sizeof(stbrp_rect) * glyphs;
    baker->rects = malloc(size);
    if (!baker->rects) return IGNIS_FAIL;
    memset(baker->rects, 0, size);

    size = sizeof(stbtt_pack_range) * ranges;
    baker->ranges = malloc(size);
    if (!baker->ranges) return IGNIS_FAIL;
    memset(baker->ranges, 0, size);

    return IGNIS_OK;
}

static void ignisFontBakerFree(IgnisFontBaker* baker)
{
    if (!baker->build)  free(baker->build);
    if (!baker->packed) free(baker->packed);
    if (!baker->rects)  free(baker->rects);
    if (!baker->ranges) free(baker->ranges);
}

static uint32_t ignisPackGlyphs(IgnisFontBaker* baker, const IgnisFontConfig* configs, size_t count)
{
    uint32_t height = 0;
    size_t range_offset = 0;
    size_t char_offset = 0;
    size_t rect_offset = 0;

    /* first font pass: pack all glyphs */
    for (int i = 0; i < count; ++i)
    {
        const IgnisFontConfig* cfg = &configs[i];
        IgnisBakeData* tmp = &baker->build[i];

        /* count glyphs + ranges in current font */
        int glyph_count = 0;
        int range_count = 0;
        for (const IgnisRune* in_range = cfg->range; in_range[0] && in_range[1]; in_range += 2)
        {
            glyph_count += (int)(in_range[1] - in_range[0]) + 1;
            range_count++;
        }

        /* setup ranges  */
        tmp->ranges = baker->ranges + range_offset;
        tmp->range_count = range_count;
        range_offset += range_count;
        for (size_t r = 0; r < range_count; ++r)
        {
            const IgnisRune* in_range = &cfg->range[r * 2];
            tmp->ranges[r].font_size = cfg->size;
            tmp->ranges[r].first_unicode_codepoint_in_range = (int)in_range[0];
            tmp->ranges[r].num_chars = (int)(in_range[1] - in_range[0]) + 1;
            tmp->ranges[r].chardata_for_range = baker->packed + char_offset;
            char_offset += tmp->ranges[r].num_chars;
        }

        /* pack */
        tmp->rects = baker->rects + rect_offset;
        rect_offset += glyph_count;

        stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
        int n = stbtt_PackFontRangesGatherRects(&baker->spc, &tmp->info, tmp->ranges, (int)tmp->range_count, tmp->rects);
        stbrp_pack_rects((stbrp_context*)baker->spc.pack_info, tmp->rects, n);

        /* texture height */
        for (size_t t = 0; t < n; ++t)
        {
            uint32_t rh = tmp->rects[t].y + tmp->rects[t].h;
            if (tmp->rects[t].was_packed && rh > height)
                height = rh;
        }
    }

    return height;
}

static uint32_t ignisRoundUpPow2(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static void* ignisPackFont(IgnisFontBaker *baker, uint32_t *w, uint32_t *h, const IgnisFontConfig *configs, size_t font_count, size_t glyph_count, size_t range_count)
{
    IGNIS_ASSERT(w);
    IGNIS_ASSERT(h);
    IGNIS_ASSERT(configs);

    if (!w || !h || !configs) return NULL;

    /* setup font baker */
    for (size_t i = 0; i < font_count; ++i)
    {
        const IgnisFontConfig* cfg = &configs[i];
        if (!stbtt_InitFont(&baker->build[i].info, cfg->ttf_blob, stbtt_GetFontOffsetForIndex(cfg->ttf_blob, 0)))
            return NULL;
    }

    uint32_t height = 0;
    uint32_t width = (glyph_count > 1000) ? 1024 : 512;
    static const uint32_t max_height = 1024 * 32;
    stbtt_PackBegin(&baker->spc, 0, width, max_height, 0, 1, NULL);
    
    height = ignisPackGlyphs(baker, configs, font_count);
    height = ignisRoundUpPow2(height);

    size_t size = (size_t)width * (size_t)height;
    void* pixels = malloc(size);
    IGNIS_ASSERT(pixels);
    if (!pixels) return NULL;

    memset(pixels, 0, size);

    /* second font pass: render glyphs */
    baker->spc.pixels = pixels;
    baker->spc.height = height;

    for (int i = 0; i < font_count; ++i)
    {
        const IgnisFontConfig* cfg = &configs[i];
        IgnisBakeData* tmp = &baker->build[i];
        stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
        stbtt_PackFontRangesRenderIntoRects(&baker->spc, &tmp->info, tmp->ranges, (int)tmp->range_count, tmp->rects);
    }
    stbtt_PackEnd(&baker->spc);

    *w = width;
    *h = height;

    return pixels;
}

static IgnisGlyph* ignisBakeGlyphs(size_t glyph_count, uint32_t width, uint32_t height, IgnisFontConfig* configs, IgnisBakeData* build, size_t count)
{
    IgnisGlyph* glyphs = malloc(sizeof(IgnisGlyph) * glyph_count);
    if (!glyphs) return NULL;

    size_t glyph_offset = 0;
    for (size_t i = 0; i < count; ++i)
    {
        IgnisFontConfig* config = &configs[i];
        config->glyph_offset = glyph_offset;

        IgnisBakeData* tmp = &build[i];
        float font_scale = stbtt_ScaleForPixelHeight(&tmp->info, config->size);

        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&tmp->info, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        float ascent = (float)unscaled_ascent * font_scale;

        /* fill own baked font glyph array */
        IgnisRune glyph_count = 0;
        for (size_t r = 0; r < tmp->range_count; ++r)
        {
            stbtt_pack_range* range = &tmp->ranges[r];
            for (int char_idx = 0; char_idx < range->num_chars; char_idx++)
            {
                /* query glyph bounds from stb_truetype */
                const stbtt_packedchar* pc = &range->chardata_for_range[char_idx];

                stbtt_aligned_quad q;
                float dummy_x = 0, dummy_y = 0;
                stbtt_GetPackedQuad(range->chardata_for_range, width, height, char_idx, &dummy_x, &dummy_y, &q, 0);

                /* fill own glyph type with data */
                IgnisGlyph* glyph = &glyphs[glyph_offset + glyph_count];
                glyph->codepoint = (IgnisRune)(range->first_unicode_codepoint_in_range + char_idx);
                glyph->x0 = q.x0;
                glyph->y0 = q.y0 + (ascent + 0.5f);
                glyph->x1 = q.x1;
                glyph->y1 = q.y1 + (ascent + 0.5f);
                glyph->w = glyph->x1 - glyph->x0 + 0.5f;
                glyph->h = glyph->y1 - glyph->y0;

                if (config->coord_type == IGNIS_FONT_COORD_PIXEL)
                {
                    glyph->u0 = q.s0 * (float)width;
                    glyph->v0 = q.t0 * (float)height;
                    glyph->u1 = q.s1 * (float)width;
                    glyph->v1 = q.t1 * (float)height;
                }
                else
                {
                    glyph->u0 = q.s0;
                    glyph->v0 = q.t0;
                    glyph->u1 = q.s1;
                    glyph->v1 = q.t1;
                }
                glyph->xadvance = pc->xadvance;

                if (config->pixel_snap)
                    glyph->xadvance = (float)(int)(glyph->xadvance + 0.5f);

                glyph_count++;
            }
        }
        glyph_offset += glyph_count;
    }

    return glyphs;
}

static void ignisFontConvertRGBA(uint32_t* dst, uint32_t img_width, uint32_t img_height, const uint8_t* src)
{
    IGNIS_ASSERT(dst);
    IGNIS_ASSERT(src);
    IGNIS_ASSERT(img_width);
    IGNIS_ASSERT(img_height);
    if (!dst || !src || !img_height || !img_width) return;

    for (uint32_t n = (uint32_t)(img_width * img_height); n > 0; n--)
        *dst++ = ((uint32_t)(*src++) << 24) | 0x00FFFFFF;
}

/*
 * ==============================================================
 *
 *                          Interface
 *
 * ===============================================================
 */
uint8_t ignisFontAtlasBake(IgnisFontAtlas* atlas, IgnisFontConfig* configs, size_t count, IgnisFontFormat fmt)
{
    IGNIS_ASSERT(atlas);
    IGNIS_ASSERT(configs);
    IGNIS_ASSERT(count);
    if (!atlas || !configs || !count) return IGNIS_FAIL;

    size_t glyph_count = 0;
    size_t range_count = 0;
    for (size_t i = 0; i < count; ++i)
    {
        IgnisFontConfig* cfg = &configs[i];
        if (!cfg->range) cfg->range = ignisGlyphRangeDefault();

        size_t rc = ignisRangeCount(cfg->range);
        range_count += rc;
        glyph_count += ignisRangeGlyphCount(cfg->range, rc);
    }

    IgnisFontBaker baker = { 0 };
    ignisFontBakerAlloc(&baker, count, glyph_count, range_count);

    /* pack all glyphs into a tight fit space */
    uint32_t width, height;
    void* pixels = ignisPackFont(&baker, &width, &height, configs, count, glyph_count, range_count);

    IGNIS_ASSERT(pixels);
    if (!pixels)
        goto failed;

    if (fmt == IGNIS_FONT_FORMAT_RGBA32)
    {
        /* convert alpha8 image into rgba32 image */
        void* rgba = malloc((size_t)width * (size_t)height * 4);
        IGNIS_ASSERT(rgba);
        if (!rgba) goto failed;

        ignisFontConvertRGBA(rgba, width, height, pixels);

        free(pixels);
        pixels = rgba;
    }

    /* bake glyphs */
    atlas->glyphs = ignisBakeGlyphs(glyph_count, width, height, configs, baker.build, count);
    atlas->glyph_count = glyph_count;

    IGNIS_ASSERT(atlas->glyphs);
    if (!atlas->glyphs)
        goto failed;

    /* create texture */
    /*
    IgnisTextureConfig tex_config = IGNIS_DEFAULT_CONFIG;
    if (fmt == IGNIS_FONT_FORMAT_ALPHA8)
    {
        tex_config.internal_format = GL_R8;
        tex_config.format = GL_RED;
    }
    ignisGenerateTexture2D(&atlas->texture, width, height, pixels, &tex_config);
    */

    /* initialize each font */
    atlas->fonts = malloc(sizeof(IgnisFont) * count);
    atlas->font_count = count;

    IGNIS_ASSERT(atlas->fonts);
    if (!atlas->fonts)
        goto failed;

    for (int i = 0; i < count; ++i)
    {
        IgnisFont* font = &atlas->fonts[i];
        IgnisFontConfig* config = &configs[i];

        font->texture = &atlas->texture;
        font->size = config->size;
        font->range = config->range;
        font->glyphs = &atlas->glyphs[config->glyph_offset];
        font->fallback = ignisFontFindGlyph(font, config->fallback_glyph);
    }

    /* free temporary memory */
    ignisFontBakerFree(&baker);
    free(pixels);
    return IGNIS_OK;

failed:
    /* error so cleanup all memory */
    ignisFontBakerFree(&baker);
    if (pixels) free(pixels);
    return IGNIS_FAIL;
}

void ignisFontAtlasClear(IgnisFontAtlas* atlas)
{
    if (atlas->glyphs) free(atlas->glyphs);
    if (atlas->fonts) free(atlas->fonts);

    ignisDestroyTexture(&atlas->texture);
}

const IgnisGlyph* ignisFontFindGlyph(const IgnisFont* font, IgnisRune unicode)
{
    IGNIS_ASSERT(font);
    IGNIS_ASSERT(font->glyphs);
    if (!font || !font->glyphs) return 0;

    size_t total_glyphs = 0;
    size_t count = ignisRangeCount(font->range);
    for (size_t i = 0; i < count; ++i)
    {
        IgnisRune f = font->range[(i * 2) + 0];
        IgnisRune t = font->range[(i * 2) + 1];

        if (unicode >= f && unicode <= t)
            return &font->glyphs[total_glyphs + (unicode - f)];
        
        total_glyphs += (size_t)(t - f) + 1;
    }
    return font->fallback;
}

static void ignisFontConfigLoadDefault(IgnisFontConfig* config, float pixel_height)
{
    config->ttf_blob = NULL;
    config->ttf_size = 0;
    config->size = pixel_height;
    config->oversample_h = 3;
    config->oversample_v = 1;
    config->pixel_snap = 0;
    config->coord_type = IGNIS_FONT_COORD_UV;
    config->range = ignisGlyphRangeDefault();
    config->fallback_glyph = '?';
}

uint8_t ignisFontAtlasLoadFromFile(IgnisFontConfig* config, const char* path, float height)
{
    size_t size;
    char* memory = ignisReadFile(path, &size);
    if (!memory) return IGNIS_FAIL;

    ignisFontConfigLoadDefault(config, height);
    config->ttf_blob = memory;
    config->ttf_size = size;

    return IGNIS_OK;
}

uint8_t ignisFontAtlasLoadFromMemory(IgnisFontConfig* config, const void* data, size_t size, float height)
{
    IGNIS_ASSERT(data);
    if (!data) return IGNIS_FAIL;

    ignisFontConfigLoadDefault(config, height);
    config->ttf_blob = data;
    config->ttf_size = size;

    return IGNIS_OK;
}

void ignisFontConfigClear(IgnisFontConfig* config, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        IgnisFontConfig* cfg = &config[i];
        if (cfg->ttf_blob) free((void*)cfg->ttf_blob);
    }
}



