// soft_font.cpp — Font rendering using stb_truetype
// SPDX-License-Identifier: MIT
#include "helix.h"
#include "core/render_internal.h"
#include "core/resource_internal.h"
#include "soft_renderer_internal.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Forward declarations
HxFont hx_font_load_from_memory(const void* data, size_t size, float pt_size);

// -----------------------------------------------------------------------------
// Font Implementation
// -----------------------------------------------------------------------------
// HxFontImpl is forward declared in helix.h as opaque type
struct HxFontImpl
{
    stbtt_fontinfo info;
    float pt_size;
    float scale;
    int ascent, descent, line_gap;
    // Glyph cache (simple LRU could be added later)
    // For now, we rasterize on demand
};

static void hx_font_destroy_resource(void* resource)
{
    HxFontImpl* font = (HxFontImpl*) resource;
    if (font)
    {
        free(font);
    }
}

HxFont hx_font_load_from_file(const char* path, float pt_size)
{
    if (!path || pt_size <= 0)
        return NULL;

    FILE* f = fopen(path, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size <= 0)
    {
        fclose(f);
        return NULL;
    }

    unsigned char* data = (unsigned char*) malloc(file_size);
    if (!data)
    {
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, 1, file_size, f);
    fclose(f);

    if (read != (size_t) file_size)
    {
        free(data);
        return NULL;
    }

    HxFont font = hx_font_load_from_memory(data, file_size, pt_size);
    free(data); // hx_font_load_from_memory makes its own copy
    return font;
}

HxFont hx_font_load_from_memory(const void* data, size_t size, float pt_size)
{
    if (!data || size == 0 || pt_size <= 0)
        return NULL;

    // Make a copy of the font data since stb_truetype doesn't own it
    unsigned char* font_data = (unsigned char*) malloc(size);
    if (!font_data)
        return NULL;
    memcpy(font_data, data, size);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
    {
        free(font_data);
        return NULL;
    }

    HxFontImpl* font = (HxFontImpl*) calloc(1, sizeof(HxFontImpl));
    if (!font)
    {
        free(font_data);
        return NULL;
    }

    font->info = info;
    font->pt_size = pt_size;
    font->scale = stbtt_ScaleForPixelHeight(&info, pt_size);
    stbtt_GetFontVMetrics(&info, &font->ascent, &font->descent, &font->line_gap);
    font->ascent = (int) (font->ascent * font->scale);
    font->descent = (int) (font->descent * font->scale);
    font->line_gap = (int) (font->line_gap * font->scale);

    // Store the font data pointer so we can free it later
    // We'll store it in the resource tracking system
    // For simplicity, we'll just keep it in the font impl
    // Note: In a real implementation, we'd want to track this separately

    if (!hx_resource_register(font, sizeof(HxFontImpl) + size, 0, hx_font_destroy_resource))
    {
        free(font_data);
        free(font);
        return NULL;
    }

    // Store the font data pointer after the struct
    unsigned char** data_ptr = (unsigned char**) ((char*) font + sizeof(HxFontImpl));
    *data_ptr = font_data;

    return (HxFont) font;
}

HxFont hx_font_load_from_file_dpi(const char* path, float pt_size, float dpi)
{
    if (!path || pt_size <= 0 || dpi <= 0)
        return NULL;
    // Adjust point size for DPI: pt_size * (dpi / 72.0)
    float adjusted_size = pt_size * (dpi / 72.0f);
    return hx_font_load_from_file(path, adjusted_size);
}

void hx_font_drop(HxFont font)
{
    if (!font)
        return;
    if (!hx_resource_unregister(font))
        return;
    // hx_font_destroy_resource will be called by resource system
}

void hx_font_measure(HxFont font, const char* text, float* out_width, float* out_height)
{
    if (!font || !text)
    {
        if (out_width)
            *out_width = 0;
        if (out_height)
            *out_height = 0;
        return;
    }

    HxFontImpl* f = (HxFontImpl*) font;
    float x = 0;
    float max_width = 0;
    int prev_codepoint = 0;

    for (const char* p = text; *p;)
    {
        int codepoint = 0;
        // Simple UTF-8 decoding
        if ((*p & 0x80) == 0)
        {
            codepoint = *p++;
        }
        else if ((*p & 0xE0) == 0xC0)
        {
            codepoint = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        }
        else if ((*p & 0xF0) == 0xE0)
        {
            codepoint = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        }
        else if ((*p & 0xF8) == 0xF0)
        {
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        }
        else
        {
            p++; // Invalid UTF-8, skip
            continue;
        }

        if (codepoint == '\n')
        {
            if (x > max_width)
                max_width = x;
            x = 0;
            continue;
        }

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&f->info, codepoint, &advance, &lsb);
        x += advance * f->scale;

        if (prev_codepoint)
        {
            x += stbtt_GetCodepointKernAdvance(&f->info, prev_codepoint, codepoint) * f->scale;
        }
        prev_codepoint = codepoint;
    }

    if (x > max_width)
        max_width = x;

    if (out_width)
        *out_width = max_width;
    if (out_height)
        *out_height = (float) (f->ascent - f->descent + f->line_gap);
}

// -----------------------------------------------------------------------------
// Text Rendering
// -----------------------------------------------------------------------------
static void hx_soft_draw_glyph(HxFontImpl* font, int codepoint, float x, float y, HxColor color)
{
    if (!g_soft.color_buffer)
        return;

    int glyph_index = stbtt_FindGlyphIndex(&font->info, codepoint);
    if (glyph_index == 0 && codepoint != 0)
        return;

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel(&font->info, glyph_index, font->scale, font->scale, 0, 0, &x0, &y0, &x1, &y1);

    int glyph_w = x1 - x0;
    int glyph_h = y1 - y0;
    if (glyph_w <= 0 || glyph_h <= 0)
        return;

    unsigned char* bitmap = (unsigned char*) malloc(glyph_w * glyph_h);
    if (!bitmap)
        return;

    stbtt_MakeCodepointBitmapSubpixel(
        &font->info, bitmap, glyph_w, glyph_h, glyph_w, font->scale, font->scale, 0, 0, glyph_index);

    // Draw the glyph bitmap with alpha blending
    int draw_x = (int) (x + x0);
    int draw_y = (int) (y + font->ascent + y0); // y0 is negative (above baseline)

    for (int gy = 0; gy < glyph_h; ++gy)
    {
        int screen_y = draw_y + gy;
        if (screen_y < 0 || screen_y >= g_soft.height)
            continue;

        for (int gx = 0; gx < glyph_w; ++gx)
        {
            int screen_x = draw_x + gx;
            if (screen_x < 0 || screen_x >= g_soft.width)
                continue;

            unsigned char alpha = bitmap[gy * glyph_w + gx];
            if (alpha == 0)
                continue;

            size_t idx = (size_t) screen_y * (size_t) g_soft.width + (size_t) screen_x;
            uint8_t* pixel = g_soft.color_buffer + idx * 4u;

            float src_a = alpha / 255.0f;
            float dst_r = pixel[0] / 255.0f;
            float dst_g = pixel[1] / 255.0f;
            float dst_b = pixel[2] / 255.0f;
            float dst_a = pixel[3] / 255.0f;

            float src_r = color.r;
            float src_g = color.g;
            float src_b = color.b;

            float out_r = src_r * src_a + dst_r * (1.0f - src_a);
            float out_g = src_g * src_a + dst_g * (1.0f - src_a);
            float out_b = src_b * src_a + dst_b * (1.0f - src_a);
            float out_a = src_a + dst_a * (1.0f - src_a);

            pixel[0] = (uint8_t) (out_r * 255);
            pixel[1] = (uint8_t) (out_g * 255);
            pixel[2] = (uint8_t) (out_b * 255);
            pixel[3] = (uint8_t) (out_a * 255);
        }
    }

    free(bitmap);
}

void hx_soft_draw_text(HxWin win, HxFont font, const char* text, float x, float y, HxColor color)
{
    (void) win;
    if (!font || !text || !g_soft.color_buffer)
        return;

    HxFontImpl* f = (HxFontImpl*) font;
    float pen_x = x;
    float pen_y = y;
    int prev_codepoint = 0;

    for (const char* p = text; *p;)
    {
        int codepoint = 0;
        // Simple UTF-8 decoding
        if ((*p & 0x80) == 0)
        {
            codepoint = *p++;
        }
        else if ((*p & 0xE0) == 0xC0)
        {
            codepoint = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        }
        else if ((*p & 0xF0) == 0xE0)
        {
            codepoint = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        }
        else if ((*p & 0xF8) == 0xF0)
        {
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        }
        else
        {
            p++; // Invalid UTF-8, skip
            continue;
        }

        if (codepoint == '\n')
        {
            pen_x = x;
            pen_y += f->ascent - f->descent + f->line_gap;
            prev_codepoint = 0;
            continue;
        }

        if (prev_codepoint)
        {
            pen_x += stbtt_GetCodepointKernAdvance(&f->info, prev_codepoint, codepoint) * f->scale;
        }

        hx_soft_draw_glyph(f, codepoint, pen_x, pen_y, color);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&f->info, codepoint, &advance, &lsb);
        pen_x += advance * f->scale;
        prev_codepoint = codepoint;
    }
}