// soft_renderer.cpp — Software rasterizer (Phase 1: minimal triangle)
#include "helix.h"
#include "core/resource_internal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
// Software Renderer State
// -----------------------------------------------------------------------------
typedef struct {
    uint8_t* color_buffer;   // RGBA8
    float* depth_buffer;     // Depth (0..1)
    int width, height;
    size_t stride;
    HxMat4 view_proj;
    HxVec3 clear_color;
    float clear_depth;
} HxSoftRenderer;

static HxSoftRenderer g_soft = {0};
static bool g_soft_resource_tracked = false;

// Forward declarations
void hx_soft_quit(void);

static void hx_soft_destroy_resource(void* resource) {
    (void)resource;
    hx_soft_quit();
}

// -----------------------------------------------------------------------------
// Init / Quit
// -----------------------------------------------------------------------------
bool hx_soft_init(int width, int height) {
    if (width <= 0 || height <= 0) return false;
    if (g_soft.color_buffer) hx_soft_quit();

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (pixel_count > SIZE_MAX / 4u || pixel_count > SIZE_MAX / sizeof(float)) return false;
    const size_t color_bytes = pixel_count * 4u;
    const size_t depth_bytes = pixel_count * sizeof(float);
    g_soft.width = width;
    g_soft.height = height;
    g_soft.stride = static_cast<size_t>(width) * 4u;
    g_soft.color_buffer = static_cast<uint8_t*>(calloc(1, color_bytes));
    g_soft.depth_buffer = static_cast<float*>(malloc(depth_bytes));
    if (!g_soft.color_buffer || !g_soft.depth_buffer) {
        hx_soft_quit();
        return false;
    }
    g_soft.clear_color = HxVec3{0.1f, 0.1f, 0.15f};
    g_soft.clear_depth = 1.0f;
    hx_make_mat4_identity(&g_soft.view_proj);
    if (!hx_resource_register(&g_soft, color_bytes + depth_bytes, 0, hx_soft_destroy_resource)) {
        hx_soft_quit();
        return false;
    }
    g_soft_resource_tracked = true;
    return true;
}

void hx_soft_quit(void) {
    if (g_soft_resource_tracked) {
        hx_resource_unregister(&g_soft);
        g_soft_resource_tracked = false;
    }
    free(g_soft.color_buffer);
    free(g_soft.depth_buffer);
    g_soft.color_buffer = NULL;
    g_soft.depth_buffer = NULL;
    g_soft.stride = 0;
    g_soft.width = g_soft.height = 0;
}

void hx_soft_resize(int width, int height) {
    hx_soft_quit();
    hx_soft_init(width, height);
}

// -----------------------------------------------------------------------------
// Clear
// -----------------------------------------------------------------------------
void hx_soft_clear(void) {
    if (!g_soft.color_buffer) return;
    uint32_t clear_rgba = ((uint32_t)(g_soft.clear_color.x * 255) << 0) |
                          ((uint32_t)(g_soft.clear_color.y * 255) << 8) |
                          ((uint32_t)(g_soft.clear_color.z * 255) << 16) |
                          (0xFF << 24);
    for (int y = 0; y < g_soft.height; ++y) {
        uint32_t* row = (uint32_t*)(g_soft.color_buffer + y * g_soft.stride);
        for (int x = 0; x < g_soft.width; ++x) row[x] = clear_rgba;
    }
    for (int i = 0; i < g_soft.width * g_soft.height; ++i) {
        g_soft.depth_buffer[i] = g_soft.clear_depth;
    }
}

// -----------------------------------------------------------------------------
// Vertex Shader (simple pass-through with MVP)
// -----------------------------------------------------------------------------
typedef struct {
    HxVec4 pos;      // Clip space
    HxVec3 color;    // Vertex color
} HxSoftVertex;

static HxSoftVertex hx_soft_vs(const HxVec3* pos, const HxVec3* color, const HxMat4* mvp) {
    HxVec4 v = {pos->x, pos->y, pos->z, 1.0f};
    HxVec4 clip;
    clip.x = mvp->m[0][0] * v.x + mvp->m[1][0] * v.y + mvp->m[2][0] * v.z + mvp->m[3][0] * v.w;
    clip.y = mvp->m[0][1] * v.x + mvp->m[1][1] * v.y + mvp->m[2][1] * v.z + mvp->m[3][1] * v.w;
    clip.z = mvp->m[0][2] * v.x + mvp->m[1][2] * v.y + mvp->m[2][2] * v.z + mvp->m[3][2] * v.w;
    clip.w = mvp->m[0][3] * v.x + mvp->m[1][3] * v.y + mvp->m[2][3] * v.z + mvp->m[3][3] * v.w;
    return HxSoftVertex{clip, *color};
}

// -----------------------------------------------------------------------------
// Triangle Rasterization (simple, no perspective correction for Phase 1)
// -----------------------------------------------------------------------------
static void hx_soft_draw_triangle(const HxSoftVertex* v0, const HxSoftVertex* v1, const HxSoftVertex* v2) {
    // Perspective divide
    float inv_w0 = 1.0f / v0->pos.w;
    float inv_w1 = 1.0f / v1->pos.w;
    float inv_w2 = 1.0f / v2->pos.w;

    // NDC to screen
    int x0 = (int)((v0->pos.x * inv_w0 * 0.5f + 0.5f) * g_soft.width);
    int y0 = (int)((-v0->pos.y * inv_w0 * 0.5f + 0.5f) * g_soft.height);
    float z0 = v0->pos.z * inv_w0;

    int x1 = (int)((v1->pos.x * inv_w1 * 0.5f + 0.5f) * g_soft.width);
    int y1 = (int)((-v1->pos.y * inv_w1 * 0.5f + 0.5f) * g_soft.height);
    float z1 = v1->pos.z * inv_w1;

    int x2 = (int)((v2->pos.x * inv_w2 * 0.5f + 0.5f) * g_soft.width);
    int y2 = (int)((-v2->pos.y * inv_w2 * 0.5f + 0.5f) * g_soft.height);
    float z2 = v2->pos.z * inv_w2;

    // Bounding box
    int min_x = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int max_x = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int min_y = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int max_y = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);

    // Clamp to screen
    if (min_x < 0) min_x = 0;
    if (max_x >= g_soft.width) max_x = g_soft.width - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= g_soft.height) max_y = g_soft.height - 1;

    // Edge functions
    float area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (fabsf(area) < 0.001f) return; // Degenerate

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            float w0 = ((x1 - x) * (y2 - y) - (y1 - y) * (x2 - x)) / area;
            float w1 = ((x2 - x) * (y0 - y) - (y2 - y) * (x0 - x)) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Interpolate depth
                float z = w0 * z0 + w1 * z1 + w2 * z2;
                int idx = y * g_soft.width + x;
                if (z < g_soft.depth_buffer[idx]) {
                    g_soft.depth_buffer[idx] = z;

                    // Interpolate color (simple, no perspective correction)
                    float r = w0 * v0->color.x + w1 * v1->color.x + w2 * v2->color.x;
                    float g = w0 * v0->color.y + w1 * v1->color.y + w2 * v2->color.y;
                    float b = w0 * v0->color.z + w1 * v1->color.z + w2 * v2->color.z;

                    uint8_t* pixel = g_soft.color_buffer + idx * 4;
                    pixel[0] = (uint8_t)(r * 255);
                    pixel[1] = (uint8_t)(g * 255);
                    pixel[2] = (uint8_t)(b * 255);
                    pixel[3] = 255;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Public Draw API
// -----------------------------------------------------------------------------
void hx_soft_set_view_proj(const HxMat4* view_proj) {
    g_soft.view_proj = *view_proj;
}

void hx_soft_draw_triangles(const HxVec3* positions, const HxVec3* colors, int count) {
    if (!g_soft.color_buffer) return;
    for (int i = 0; i < count; i += 3) {
        HxSoftVertex v0 = hx_soft_vs(&positions[i], &colors[i], &g_soft.view_proj);
        HxSoftVertex v1 = hx_soft_vs(&positions[i+1], &colors[i+1], &g_soft.view_proj);
        HxSoftVertex v2 = hx_soft_vs(&positions[i+2], &colors[i+2], &g_soft.view_proj);
        hx_soft_draw_triangle(&v0, &v1, &v2);
    }
}

// -----------------------------------------------------------------------------
// Get Pixels (for headless)
// -----------------------------------------------------------------------------
void hx_soft_get_pixels(void** out_pixels, size_t* out_stride, int* out_w, int* out_h) {
    if (out_pixels) *out_pixels = g_soft.color_buffer;
    if (out_stride) *out_stride = g_soft.stride;
    if (out_w) *out_w = g_soft.width;
    if (out_h) *out_h = g_soft.height;
}