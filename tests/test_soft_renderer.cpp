#include "helix.h"
#include <cstdint>
#include <cstdio>

extern bool hx_soft_init(int width, int height);
extern void hx_soft_quit(void);
extern void hx_soft_clear(void);
extern void hx_soft_set_view_proj(const HxMat4* view_proj);
extern void hx_soft_draw_triangles(const HxVec3* positions, const HxVec3* colors, int count);
extern void hx_soft_get_pixels(void** pixels, size_t* stride, int* width, int* height);

int main()
{
    HxCfg config{};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    if (hx_boot(&config) != HX_OK)
        return 1;
    uint8_t output[16 * 16 * 4]{};
    if (hx_render_headless(16, 16, nullptr, nullptr, output, 16 * 4) != HX_ERR_INVALID_ARG)
    {
        hx_quit();
        return 2;
    }
    HxWorld world = hx_make_world();
    HxCam camera = hx_make_cam3d();
    if (!world || !camera || hx_render_headless(16, 16, world, camera, output, 16 * 4) != HX_OK)
    {
        if (camera)
            hx_drop_cam(camera);
        if (world)
            hx_drop_world(world);
        hx_quit();
        return 3;
    }
    if (output[0] != 25 || output[1] != 25 || output[2] != 38 || output[3] != 255)
    {
        hx_drop_cam(camera);
        hx_drop_world(world);
        hx_quit();
        return 3;
    }
    hx_drop_cam(camera);
    hx_drop_world(world);
    if (!hx_soft_init(16, 16))
    {
        hx_quit();
        return 2;
    }

    HxMat4 view_proj;
    hx_make_mat4_identity(&view_proj);
    hx_soft_set_view_proj(&view_proj);
    hx_soft_clear();

    const HxVec3 positions[] = {{-0.75f, -0.75f, 0.0f}, {0.75f, -0.75f, 0.0f}, {0.0f, 0.75f, 0.0f}};
    const HxVec3 colors[] = {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
    hx_soft_draw_triangles(positions, colors, 3);

    void* pixels = nullptr;
    size_t stride = 0;
    int width = 0;
    int height = 0;
    hx_soft_get_pixels(&pixels, &stride, &width, &height);
    const auto* bytes = static_cast<const uint8_t*>(pixels);
    const bool dimensions_ok = bytes && width == 16 && height == 16 && stride == 64;
    const bool triangle_ok = dimensions_ok && bytes[8 * stride + 8 * 4] == 255 && bytes[8 * stride + 8 * 4 + 1] == 0 &&
                             bytes[8 * stride + 8 * 4 + 2] == 0 && bytes[8 * stride + 8 * 4 + 3] == 255;
    const bool clear_ok = dimensions_ok && bytes[0] == 25 && bytes[1] == 25 && bytes[2] == 38 && bytes[3] == 255;

    hx_soft_quit();
    hx_quit();
    if (!triangle_ok || !clear_ok)
    {
        std::fprintf(stderr, "software raster pixel regression: triangle=%d clear=%d\n", triangle_ok, clear_ok);
        return 3;
    }
    return 0;
}
