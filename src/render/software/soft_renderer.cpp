// soft_renderer.cpp — Software rasterizer (Phase 1: minimal triangle)
#include "helix.h"
#include "core/render_internal.h"
#include "core/resource_internal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <climits>
#include <vector>

// -----------------------------------------------------------------------------
// Software Renderer State
// -----------------------------------------------------------------------------
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

static HxSoftRenderer g_soft = {0};
static bool g_soft_resource_tracked = false;

// Forward declarations
void hx_soft_quit(void);

static void hx_soft_destroy_resource(void* resource)
{
    (void)resource;
    hx_soft_quit();
}

// -----------------------------------------------------------------------------
// Init / Quit
// -----------------------------------------------------------------------------
bool hx_soft_init(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;
    if (g_soft.color_buffer)
        hx_soft_quit();

    if (static_cast<size_t>(width) > SIZE_MAX / static_cast<size_t>(height) ||
        static_cast<size_t>(width) > SIZE_MAX / 4u)
        return false;
    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (pixel_count > SIZE_MAX / (4u + sizeof(float)))
        return false;
    const size_t color_bytes = pixel_count * 4u;
    const size_t depth_bytes = pixel_count * sizeof(float);
    g_soft.width = width;
    g_soft.height = height;
    g_soft.stride = static_cast<size_t>(width) * 4u;
    g_soft.color_buffer = static_cast<uint8_t*>(calloc(1, color_bytes));
    g_soft.depth_buffer = static_cast<float*>(malloc(depth_bytes));
    if (!g_soft.color_buffer || !g_soft.depth_buffer)
    {
        hx_soft_quit();
        return false;
    }
    g_soft.clear_color = HxVec3{0.1f, 0.1f, 0.15f};
    g_soft.clear_depth = 1.0f;
    hx_make_mat4_identity(&g_soft.view_proj);
    if (!hx_resource_register(&g_soft, color_bytes + depth_bytes, 0, hx_soft_destroy_resource))
    {
        hx_soft_quit();
        return false;
    }
    g_soft_resource_tracked = true;
    return true;
}

void hx_soft_quit(void)
{
    if (g_soft_resource_tracked)
    {
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

void hx_soft_resize(int width, int height)
{
    hx_soft_quit();
    hx_soft_init(width, height);
}

// -----------------------------------------------------------------------------
// Clear
// -----------------------------------------------------------------------------
void hx_soft_clear(void)
{
    if (!g_soft.color_buffer)
        return;
    uint32_t clear_rgba = ((uint32_t)(g_soft.clear_color.x * 255) << 0) |
                          ((uint32_t)(g_soft.clear_color.y * 255) << 8) |
                          ((uint32_t)(g_soft.clear_color.z * 255) << 16) | (0xFF << 24);
    for (int y = 0; y < g_soft.height; ++y)
    {
        uint32_t* row = (uint32_t*)(g_soft.color_buffer + y * g_soft.stride);
        for (int x = 0; x < g_soft.width; ++x)
            row[x] = clear_rgba;
    }
    const size_t pixel_count = static_cast<size_t>(g_soft.width) * static_cast<size_t>(g_soft.height);
    for (size_t i = 0; i < pixel_count; ++i)
    {
        g_soft.depth_buffer[i] = g_soft.clear_depth;
    }
}

// -----------------------------------------------------------------------------
// Vertex Shader (simple pass-through with MVP)
// -----------------------------------------------------------------------------
typedef struct
{
    HxVec4 pos;   // Clip space
    HxVec3 color; // Vertex color
    HxVec2 texcoord;
} HxSoftVertex;

static HxTextureRenderData g_soft_texture = {};
static bool g_soft_texture_enabled = false;
static HxVec3 hx_soft_texture_sample(float u, float v);

static HxSoftVertex hx_soft_vs(const HxVec3* pos, const HxVec3* color, const HxVec2* texcoord, const HxMat4* mvp)
{
    HxVec4 v = {pos->x, pos->y, pos->z, 1.0f};
    HxVec4 clip;
    clip.x = mvp->m[0][0] * v.x + mvp->m[1][0] * v.y + mvp->m[2][0] * v.z + mvp->m[3][0] * v.w;
    clip.y = mvp->m[0][1] * v.x + mvp->m[1][1] * v.y + mvp->m[2][1] * v.z + mvp->m[3][1] * v.w;
    clip.z = mvp->m[0][2] * v.x + mvp->m[1][2] * v.y + mvp->m[2][2] * v.z + mvp->m[3][2] * v.w;
    clip.w = mvp->m[0][3] * v.x + mvp->m[1][3] * v.y + mvp->m[2][3] * v.z + mvp->m[3][3] * v.w;
    return HxSoftVertex{clip, *color, texcoord ? *texcoord : HxVec2{0.0f, 0.0f}};
}

// -----------------------------------------------------------------------------
// Triangle Rasterization (simple, no perspective correction for Phase 1)
// -----------------------------------------------------------------------------
static void hx_soft_draw_triangle(const HxSoftVertex* v0, const HxSoftVertex* v1, const HxSoftVertex* v2)
{
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
    if (min_x < 0)
        min_x = 0;
    if (max_x >= g_soft.width)
        max_x = g_soft.width - 1;
    if (min_y < 0)
        min_y = 0;
    if (max_y >= g_soft.height)
        max_y = g_soft.height - 1;

    // Edge functions
    float area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (fabsf(area) < 0.001f)
        return; // Degenerate

    for (int y = min_y; y <= max_y; ++y)
    {
        for (int x = min_x; x <= max_x; ++x)
        {
            float w0 = ((x1 - x) * (y2 - y) - (y1 - y) * (x2 - x)) / area;
            float w1 = ((x2 - x) * (y0 - y) - (y2 - y) * (x0 - x)) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
            {
                // Interpolate depth
                float z = w0 * z0 + w1 * z1 + w2 * z2;
                const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(g_soft.width) + static_cast<size_t>(x);
                if (z < g_soft.depth_buffer[idx])
                {
                    g_soft.depth_buffer[idx] = z;

                    // Interpolate color (simple, no perspective correction)
                    float r = w0 * v0->color.x + w1 * v1->color.x + w2 * v2->color.x;
                    float g = w0 * v0->color.y + w1 * v1->color.y + w2 * v2->color.y;
                    float b = w0 * v0->color.z + w1 * v1->color.z + w2 * v2->color.z;
                    if (g_soft_texture_enabled)
                    {
                        const float u = w0 * v0->texcoord.x + w1 * v1->texcoord.x + w2 * v2->texcoord.x;
                        const float v = w0 * v0->texcoord.y + w1 * v1->texcoord.y + w2 * v2->texcoord.y;
                        const HxVec3 texel = hx_soft_texture_sample(u, v);
                        r *= texel.x;
                        g *= texel.y;
                        b *= texel.z;
                    }

                    uint8_t* pixel = g_soft.color_buffer + idx * 4u;
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
void hx_soft_set_view_proj(const HxMat4* view_proj) { g_soft.view_proj = *view_proj; }

void hx_soft_draw_triangles(const HxVec3* positions, const HxVec3* colors, int count)
{
    if (!g_soft.color_buffer)
        return;
    for (int i = 0; i < count; i += 3)
    {
        HxSoftVertex v0 = hx_soft_vs(&positions[i], &colors[i], nullptr, &g_soft.view_proj);
        HxSoftVertex v1 = hx_soft_vs(&positions[i + 1], &colors[i + 1], nullptr, &g_soft.view_proj);
        HxSoftVertex v2 = hx_soft_vs(&positions[i + 2], &colors[i + 2], nullptr, &g_soft.view_proj);
        hx_soft_draw_triangle(&v0, &v1, &v2);
    }
}

static void hx_soft_draw_textured_triangles(const HxVec3* positions, const HxVec3* colors, const HxVec2* texcoords,
                                            HxTex texture, int count)
{
    g_soft_texture_enabled = texture && hx_texture_get_render_data(texture, &g_soft_texture);
    for (int i = 0; i + 2 < count; i += 3)
    {
        HxSoftVertex v0 = hx_soft_vs(&positions[i], &colors[i], texcoords ? &texcoords[i] : nullptr, &g_soft.view_proj);
        HxSoftVertex v1 =
            hx_soft_vs(&positions[i + 1], &colors[i + 1], texcoords ? &texcoords[i + 1] : nullptr, &g_soft.view_proj);
        HxSoftVertex v2 =
            hx_soft_vs(&positions[i + 2], &colors[i + 2], texcoords ? &texcoords[i + 2] : nullptr, &g_soft.view_proj);
        hx_soft_draw_triangle(&v0, &v1, &v2);
    }
    g_soft_texture_enabled = false;
}

// -----------------------------------------------------------------------------
// Get Pixels (for headless)
// -----------------------------------------------------------------------------
void hx_soft_get_pixels(void** out_pixels, size_t* out_stride, int* out_w, int* out_h)
{
    if (out_pixels)
        *out_pixels = g_soft.color_buffer;
    if (out_stride)
        *out_stride = g_soft.stride;
    if (out_w)
        *out_w = g_soft.width;
    if (out_h)
        *out_h = g_soft.height;
}
static float hx_soft_clamp01(float value) { return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }

static int hx_soft_texture_index(int coordinate, int extent, HxTexFlags flags)
{
    if (flags & HX_TEX_MIRROR)
    {
        const int64_t period = static_cast<int64_t>(extent) * 2;
        int64_t wrapped = coordinate % period;
        if (wrapped < 0)
            wrapped += period;
        return static_cast<int>(wrapped < extent ? wrapped : period - wrapped - 1);
    }
    if (flags & HX_TEX_REPEAT)
    {
        const int wrapped = coordinate % extent;
        return wrapped < 0 ? wrapped + extent : wrapped;
    }
    return coordinate < 0 ? 0 : (coordinate >= extent ? extent - 1 : coordinate);
}

static HxVec3 hx_soft_texture_sample(float u, float v)
{
    if (!g_soft_texture_enabled || !g_soft_texture.rgba_pixels)
        return {1.0f, 1.0f, 1.0f};
    if (g_soft_texture.flags & HX_TEX_REPEAT)
    {
        u -= floorf(u);
        v -= floorf(v);
    }
    else if (g_soft_texture.flags & HX_TEX_MIRROR)
    {
        u = 1.0f - fabsf(fmodf(u, 2.0f) - 1.0f);
        v = 1.0f - fabsf(fmodf(v, 2.0f) - 1.0f);
    }
    else
    {
        u = hx_soft_clamp01(u);
        v = hx_soft_clamp01(v);
    }

    const float fx =
        u * static_cast<float>(g_soft_texture.width) - ((g_soft_texture.flags & HX_TEX_LINEAR) ? 0.5f : 0.0f);
    const float fy =
        v * static_cast<float>(g_soft_texture.height) - ((g_soft_texture.flags & HX_TEX_LINEAR) ? 0.5f : 0.0f);
    const int x0 = static_cast<int>(floorf(fx));
    const int y0 = static_cast<int>(floorf(fy));
    const int x1 = x0 + ((g_soft_texture.flags & HX_TEX_LINEAR) ? 1 : 0);
    const int y1 = y0 + ((g_soft_texture.flags & HX_TEX_LINEAR) ? 1 : 0);
    const float tx = (g_soft_texture.flags & HX_TEX_LINEAR) ? fx - floorf(fx) : 0.0f;
    const float ty = (g_soft_texture.flags & HX_TEX_LINEAR) ? fy - floorf(fy) : 0.0f;
    const int xs[] = {hx_soft_texture_index(x0, g_soft_texture.width, g_soft_texture.flags),
                      hx_soft_texture_index(x1, g_soft_texture.width, g_soft_texture.flags)};
    const int ys[] = {hx_soft_texture_index(y0, g_soft_texture.height, g_soft_texture.flags),
                      hx_soft_texture_index(y1, g_soft_texture.height, g_soft_texture.flags)};
    float channels[3] = {};
    for (int iy = 0; iy < 2; ++iy)
    {
        for (int ix = 0; ix < 2; ++ix)
        {
            const size_t offset = (static_cast<size_t>(ys[iy]) * static_cast<size_t>(g_soft_texture.width) +
                                   static_cast<size_t>(xs[ix])) *
                                  4u;
            const float weight = (ix ? tx : 1.0f - tx) * (iy ? ty : 1.0f - ty);
            for (int channel = 0; channel < 3; ++channel)
                channels[channel] +=
                    g_soft_texture.rgba_pixels[offset + static_cast<size_t>(channel)] * (weight / 255.0f);
        }
    }
    return {channels[0], channels[1], channels[2]};
}

static HxVec3 hx_soft_transform_position(const HxMeshRenderData& mesh, const HxMat4& world, HxVec3 p)
{
    p.x *= mesh.scale.x;
    p.y *= mesh.scale.y;
    p.z *= mesh.scale.z;
    const float cx = cosf(mesh.rotation.x), sx = sinf(mesh.rotation.x);
    const float cy = cosf(mesh.rotation.y), sy = sinf(mesh.rotation.y);
    const float cz = cosf(mesh.rotation.z), sz = sinf(mesh.rotation.z);
    const float y1 = p.y * cx - p.z * sx;
    const float z1 = p.y * sx + p.z * cx;
    const float x2 = p.x * cy + z1 * sy;
    const float z2 = -p.x * sy + z1 * cy;
    const float x3 = x2 * cz - y1 * sz;
    const float y3 = x2 * sz + y1 * cz;
    p.x = x3 + mesh.position.x;
    p.y = y3 + mesh.position.y;
    p.z = z2 + mesh.position.z;
    return {world.m[0][0] * p.x + world.m[1][0] * p.y + world.m[2][0] * p.z + world.m[3][0],
            world.m[0][1] * p.x + world.m[1][1] * p.y + world.m[2][1] * p.z + world.m[3][1],
            world.m[0][2] * p.x + world.m[1][2] * p.y + world.m[2][2] * p.z + world.m[3][2]};
}

HxResult hx_soft_render_world(int width, int height, HxWorld world, HxCam cam, void* out_pixels, size_t out_stride)
{
    if (!world || !cam || !out_pixels || !hx_soft_init(width, height))
        return HX_ERR_OOM;
    HxMat4 view_proj;
    hx_get_cam_view_proj(cam, &view_proj);
    hx_soft_set_view_proj(&view_proj);
    hx_soft_clear();
    try
    {
        const size_t item_count = hx_world_mesh_count(world);
        std::vector<HxVec3> positions;
        std::vector<HxVec3> colors;
        std::vector<HxVec2> texcoords;
        for (size_t item_index = 0; item_index < item_count; ++item_index)
        {
            HxWorldMeshItem item{};
            HxMeshRenderData mesh{};
            HxColor skin_color{};
            HxTex texture = nullptr;
            if (!hx_world_mesh_get(world, item_index, &item) || !hx_mesh_get_render_data(item.mesh, &mesh) ||
                !hx_skin_get_render_data(item.skin, &skin_color, &texture))
            {
                hx_soft_quit();
                return HX_ERR_INVALID_HANDLE;
            }
            const size_t element_count = mesh.index_count ? mesh.index_count : mesh.vertex_count;
            if (element_count > static_cast<size_t>(INT_MAX) || mesh.vertex_count > static_cast<size_t>(INT_MAX))
            {
                hx_soft_quit();
                return HX_ERR_UNSUPPORTED;
            }
            positions.clear();
            colors.clear();
            texcoords.clear();
            positions.reserve(element_count + 2u);
            colors.reserve(element_count + 2u);
            texcoords.reserve(element_count + 2u);
            auto append_vertex = [&](uint32_t index)
            {
                if (index >= mesh.vertex_count)
                    return false;
                positions.push_back(hx_soft_transform_position(mesh, item.transform, mesh.positions[index]));
                const HxVec4 vertex_color = mesh.colors ? mesh.colors[index] : HxVec4{1, 1, 1, 1};
                colors.push_back({hx_soft_clamp01(skin_color.r * vertex_color.x),
                                  hx_soft_clamp01(skin_color.g * vertex_color.y),
                                  hx_soft_clamp01(skin_color.b * vertex_color.z)});
                texcoords.push_back(mesh.texcoords ? mesh.texcoords[index] : HxVec2{0.0f, 0.0f});
                return true;
            };
            auto draw_triangle = [&](size_t a, size_t b, size_t c)
            {
                const uint32_t ia = mesh.index_count ? mesh.indices[a] : static_cast<uint32_t>(a);
                const uint32_t ib = mesh.index_count ? mesh.indices[b] : static_cast<uint32_t>(b);
                const uint32_t ic = mesh.index_count ? mesh.indices[c] : static_cast<uint32_t>(c);
                return append_vertex(ia) && append_vertex(ib) && append_vertex(ic);
            };
            if (mesh.topology == HX_PRIM_TRIANGLES)
            {
                for (size_t i = 0; i < element_count; i += 3u)
                {
                    if (!draw_triangle(i, i + 1u, i + 2u))
                    {
                        hx_soft_quit();
                        return HX_ERR_INVALID_ARG;
                    }
                }
            }
            else if (mesh.topology == HX_PRIM_TRI_STRIP)
            {
                for (size_t i = 2; i < element_count; ++i)
                {
                    const bool ok = (i & 1u) ? draw_triangle(i - 1u, i - 2u, i) : draw_triangle(i - 2u, i - 1u, i);
                    if (!ok)
                    {
                        hx_soft_quit();
                        return HX_ERR_INVALID_ARG;
                    }
                }
            }
            else
            {
                hx_soft_quit();
                return HX_ERR_UNSUPPORTED;
            }
            if (!positions.empty())
                hx_soft_draw_textured_triangles(positions.data(), colors.data(), texcoords.data(), texture,
                                                static_cast<int>(positions.size()));
        }
        void* source = nullptr;
        size_t source_stride = 0;
        hx_soft_get_pixels(&source, &source_stride, nullptr, nullptr);
        if (!source)
        {
            hx_soft_quit();
            return HX_ERR;
        }
        auto* destination = static_cast<uint8_t*>(out_pixels);
        const auto* source_bytes = static_cast<const uint8_t*>(source);
        const size_t row_bytes = static_cast<size_t>(width) * 4u;
        for (int y = 0; y < height; ++y)
        {
            memcpy(destination + static_cast<size_t>(y) * out_stride,
                   source_bytes + static_cast<size_t>(y) * source_stride, row_bytes);
        }
    }
    catch (...)
    {
        hx_soft_quit();
        return HX_ERR_OOM;
    }
    hx_soft_quit();
    return HX_OK;
}
