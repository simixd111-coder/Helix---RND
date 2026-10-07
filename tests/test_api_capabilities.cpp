#include "helix.h"

int main()
{
    int major = 0, minor = 0, patch = 0;
    hx_version(&major, &minor, &patch);
    if (major != 2 || minor != 0 || patch != 0)
        return 10;

    if (hx_make_lamp(HX_LAMP_POINT, HX_WHITE, 1.0f) != nullptr)
        return 1;
    if (hx_load_shader("sprite") != nullptr || hx_load_font("font.ttf", 16.0f) != nullptr)
        return 2;
    if (hx_load_rig("rig.gltf") != nullptr || hx_load_clip("walk.gltf") != nullptr)
        return 3;
    if (hx_add_fx(nullptr, HX_FX_FXAA) != nullptr)
        return 4;
    if (hx_drop_fx(nullptr) != HX_ERR_INVALID_HANDLE || hx_drop_font(nullptr) != HX_ERR_INVALID_HANDLE)
        return 5;
    if (hx_make_skin(HX_WHITE, HX_SKIN_TRANSPARENT) != nullptr)
        return 6;
    HxAtlas atlas = hx_make_atlas(16, 16);
    if (!atlas)
        return 7;
    HxTex atlas_texture = hx_build_atlas(atlas);
    if (!atlas_texture)
        return 8;
    if (hx_drop_tex(atlas_texture) != HX_OK || hx_drop_atlas(atlas) != HX_OK)
        return 9;
    return 0;
}
