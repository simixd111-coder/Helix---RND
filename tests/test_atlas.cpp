#include "helix.h"

int main()
{
    HxCfg config{};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    if (hx_boot(&config) != HX_OK)
        return 1;

    const uint8_t texel[] = {42, 180, 90, 255};
    HxTex source = hx_make_tex(1, 1, HX_TEX_FMT_RGBA8, texel, HX_TEX_NONE);
    HxAtlas atlas = hx_make_atlas(4, 4);
    if (!source || !atlas)
        return 2;
    hx_add_atlas(atlas, "sample", source, 2, 1, 1, 1);
    HxTex packed = hx_build_atlas(atlas);
    HxWorld world = hx_make_world();
    HxCam camera = hx_make_cam3d();
    const float vertices[][5] = {
        {-0.75f, -0.75f, 0.0f, 0.625f, 0.375f},
        {0.75f, -0.75f, 0.0f, 0.625f, 0.375f},
        {0.0f, 0.75f, 0.0f, 0.625f, 0.375f},
    };
    HxMesh mesh =
        hx_make_mesh(vertices, 3, HX_VERT_POS | HX_VERT_UV0, sizeof(vertices[0]), nullptr, 0, false, HX_PRIM_TRIANGLES);
    HxSkin skin = hx_make_skin_tex(packed, HX_WHITE, HX_SKIN_UNLIT);
    if (!packed || !world || !camera || !mesh || !skin)
        return 3;

    hx_set_cam_ortho(camera, -1, 1, -1, 1, -1, 1);
    HxMat4 transform;
    hx_make_mat4_identity(&transform);
    hx_add_mesh(world, mesh, skin, &transform);
    uint8_t pixels[16 * 16 * 4]{};
    const HxResult render_result = hx_render_headless(16, 16, world, camera, pixels, 16 * 4);
    const size_t center = (8u * 16u + 8u) * 4u;
    const bool copied = pixels[center] == texel[0] && pixels[center + 1] == texel[1] && pixels[center + 2] == texel[2];

    hx_drop_skin(skin);
    hx_drop_mesh(mesh);
    hx_drop_cam(camera);
    hx_drop_world(world);
    hx_drop_tex(packed);
    hx_drop_atlas(atlas);
    hx_drop_tex(source);
    hx_quit();
    return render_result == HX_OK && copied ? 0 : 4;
}
