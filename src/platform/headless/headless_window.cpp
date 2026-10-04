// headless_window.cpp — Headless/offscreen window implementation (Phase 1)
#include "../platform_internal.h"
#include "../../core/resource_internal.h"
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Headless Window Data
// -----------------------------------------------------------------------------
typedef struct {
    uint8_t* pixels;      // RGBA8 buffer
    size_t stride;        // Bytes per row
    int width, height;
} HxHeadlessData;

// -----------------------------------------------------------------------------
// Platform Functions
// -----------------------------------------------------------------------------
bool hx_headless_window_create(HxWin win) {
    HxHeadlessData* data = (HxHeadlessData*)calloc(1, sizeof(HxHeadlessData));
    if (!data) return false;

    data->width = win->width;
    data->height = win->height;
    data->stride = (size_t)win->width * 4;
    data->pixels = (uint8_t*)calloc(1, data->stride * win->height);
    if (!data->pixels) {
        free(data);
        return false;
    }

    win->cpu_bytes += sizeof(HxHeadlessData) + data->stride * static_cast<size_t>(data->height);
    win->platform_data = data;
    win->headless = true;
    return true;
}

void hx_headless_window_destroy(HxWin win) {
    HxHeadlessData* data = (HxHeadlessData*)win->platform_data;
    if (data) {
        free(data->pixels);
        free(data);
        win->platform_data = NULL;
    }
}

void hx_headless_window_set_size(HxWin win, int width, int height) {
    if (width <= 0 || height <= 0) return;
    HxHeadlessData* data = (HxHeadlessData*)win->platform_data;
    if (!data) return;

    if (width == data->width && height == data->height) return;

    size_t new_stride = (size_t)width * 4;
    uint8_t* new_pixels = (uint8_t*)calloc(1, new_stride * height);
    if (!new_pixels) return;

    // Copy old content (minimal)
    int copy_h = height < data->height ? height : data->height;
    int copy_w = width < data->width ? width : data->width;
    for (int y = 0; y < copy_h; ++y) {
        memcpy(new_pixels + y * new_stride, data->pixels + y * data->stride, copy_w * 4);
    }

    free(data->pixels);
    data->pixels = new_pixels;
    data->stride = new_stride;
    data->width = width;
    data->height = height;
    win->width = width;
    win->height = height;
    win->cpu_bytes = sizeof(HxWinImpl) + sizeof(HxHeadlessData) + new_stride * static_cast<size_t>(height);
    hx_resource_resize(win, win->cpu_bytes, 0);
}

// -----------------------------------------------------------------------------
// Headless Render Access
// -----------------------------------------------------------------------------
HxResult hx_headless_get_pixels(HxWin win, void** out_pixels, size_t* out_stride, int* out_w, int* out_h) {
    HxHeadlessData* data = (HxHeadlessData*)win->platform_data;
    if (!data) return HX_ERR_INVALID_HANDLE;
    if (out_pixels) *out_pixels = data->pixels;
    if (out_stride) *out_stride = data->stride;
    if (out_w) *out_w = data->width;
    if (out_h) *out_h = data->height;
    return HX_OK;
}