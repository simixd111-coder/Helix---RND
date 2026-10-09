// soft_renderer_internal.h — Shared software renderer state
// SPDX-License-Identifier: MIT
#pragma once
#include "helix.h"

typedef struct
{
    uint8_t* color_buffer; // RGBA8
    float* depth_buffer;   // Depth (0..1)
    int width, height;
    size_t stride;
    HxMat4 view_proj;
    HxVec3 clear_color;
    float clear_depth;
} HxSoftRenderer;

extern HxSoftRenderer g_soft;