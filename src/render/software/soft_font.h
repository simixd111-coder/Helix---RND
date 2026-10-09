// soft_font.h — Font rendering internal API
// SPDX-License-Identifier: MIT
#ifndef HELIX_SOFT_FONT_H
#define HELIX_SOFT_FONT_H

#include "helix.h"

HxFont hx_font_load_from_file(const char* path, float pt_size);
HxFont hx_font_load_from_memory(const void* data, size_t size, float pt_size);
HxFont hx_font_load_from_file_dpi(const char* path, float pt_size, float dpi);
void hx_font_drop(HxFont font);
void hx_font_measure(HxFont font, const char* text, float* out_width, float* out_height);
void hx_soft_draw_text(HxWin win, HxFont font, const char* text, float x, float y, HxColor color);

#endif // HELIX_SOFT_FONT_H