#include "helix.h"
#include <cstdint>
#include <cstdio>

static uint64_t image_hash(const uint8_t* bytes, size_t count)
{
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < count; ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

int main()
{
    HxCfg config{};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    if (hx_boot(&config) != HX_OK)
        return 1;

    HxWorld world = hx_make_world();
    HxCam camera = hx_make_cam3d();
    const HxVec3 vertices[] = {{-0.75f, -0.75f, 0.0f}, {0.75f, -0.75f, 0.0f}, {0.0f, 0.75f, 0.0f}};
    HxMesh mesh = hx_make_mesh(vertices, 3, HX_VERT_POS, 0, nullptr, 0, false, HX_PRIM_TRIANGLES);
    HxSkin skin = hx_make_skin(HxColor{1, 0, 0, 1}, HX_SKIN_UNLIT);
    HxMesh textured_mesh = nullptr;
    HxSkin textured_skin = nullptr;
    HxTex texture = nullptr;
    int result = 0;
    if (!world || !camera || !mesh || !skin)
        result = 2;
    if (result == 0)
    {
        hx_set_cam_ortho(camera, -1, 1, -1, 1, -1, 1);
        HxMat4 transform;
        hx_make_mat4_identity(&transform);
        hx_add_mesh(world, mesh, skin, &transform);
    }

    uint8_t pixels[16 * 16 * 4]{};
    if (result == 0 && hx_render_headless(16, 16, world, camera, pixels, 16 * 4) != HX_OK)
        result = 3;
    if (result == 0)
    {
        const size_t center = (8u * 16u + 8u) * 4u;
        if (pixels[center] != 255 || pixels[center + 1] != 0 || pixels[center + 2] != 0 || pixels[center + 3] != 255)
            result = 4;
        if (pixels[0] != 25 || pixels[1] != 25 || pixels[2] != 38 || pixels[3] != 255)
            result = 5;
        const uint64_t hash = image_hash(pixels, sizeof(pixels));
        if (hash != 0x0AD6BF1A47C9E085ull)
            result = 6;
    }

    if (result == 0)
    {
        hx_clear_world(world);
        const float textured_vertices[][5] = {
            {-0.75f, -0.75f, 0.0f, 0.5f, 0.5f},
            {0.75f, -0.75f, 0.0f, 0.5f, 0.5f},
            {0.0f, 0.75f, 0.0f, 0.5f, 0.5f},
        };
        const uint8_t texel[] = {32, 192, 64, 255};
        texture = hx_make_tex(1, 1, HX_TEX_FMT_RGBA8, texel, HX_TEX_NONE);
        textured_mesh = hx_make_mesh(textured_vertices, 3, HX_VERT_POS | HX_VERT_UV0, sizeof(textured_vertices[0]),
                                     nullptr, 0, false, HX_PRIM_TRIANGLES);
        textured_skin = hx_make_skin_tex(texture, HX_WHITE, HX_SKIN_UNLIT);
        if (!texture || !textured_mesh || !textured_skin)
            result = 7;
        else
        {
            HxMat4 transform;
            hx_make_mat4_identity(&transform);
            hx_add_mesh(world, textured_mesh, textured_skin, &transform);
            if (hx_render_headless(16, 16, world, camera, pixels, 16 * 4) != HX_OK)
                result = 8;
            else
            {
                const size_t center = (8u * 16u + 8u) * 4u;
                if (pixels[center] != 32 || pixels[center + 1] != 192 || pixels[center + 2] != 64 ||
                    pixels[center + 3] != 255)
                    result = 9;
            }
        }
    }

    if (result == 0)
    {
        hx_drop_tex(texture);
        texture = nullptr;
        if (hx_render_headless(16, 16, world, camera, pixels, 16 * 4) != HX_ERR_INVALID_HANDLE)
            result = 10;
    }

    if (result == 0)
    {
        hx_drop_mesh(textured_mesh);
        textured_mesh = nullptr;
        if (hx_render_headless(16, 16, world, camera, pixels, 16 * 4) != HX_ERR_INVALID_HANDLE)
            result = 11;
    }

    if (world)
        hx_drop_world(world);
    if (camera)
        hx_drop_cam(camera);
    if (skin)
        hx_drop_skin(skin);
    if (textured_skin)
        hx_drop_skin(textured_skin);
    if (texture)
        hx_drop_tex(texture);
    if (mesh)
        hx_drop_mesh(mesh);
    if (textured_mesh)
        hx_drop_mesh(textured_mesh);
    hx_quit();
    if (result)
        std::fprintf(stderr, "scene render test failed: %d\n", result);
    return result;
}
