#include "helix.h"

static HxResult hx_unavailable_drop(const void* handle)
{
    return handle ? HX_ERR_NOT_IMPLEMENTED : HX_ERR_INVALID_HANDLE;
}

HX_API HxLamp HX_CALL hx_make_lamp(HxLampType type, HxColor color, float intensity)
{
    (void)type;
    (void)color;
    (void)intensity;
    return nullptr;
}
HX_API void HX_CALL hx_set_lamp_dir(HxLamp lamp, const HxVec3* direction)
{
    (void)lamp;
    (void)direction;
}
HX_API void HX_CALL hx_set_lamp_range(HxLamp lamp, float range)
{
    (void)lamp;
    (void)range;
}
HX_API void HX_CALL hx_set_lamp_spot(HxLamp lamp, float inner_deg, float outer_deg)
{
    (void)lamp;
    (void)inner_deg;
    (void)outer_deg;
}
HX_API HxResult HX_CALL hx_drop_lamp(HxLamp lamp) { return hx_unavailable_drop(lamp); }

HX_API HxShader HX_CALL hx_load_shader(const char* name)
{
    (void)name;
    return nullptr;
}
HX_API HxResult HX_CALL hx_drop_shader(HxShader shader) { return hx_unavailable_drop(shader); }

HX_API void HX_CALL hx_draw_world(HxWin win, HxWorld world, HxCam cam)
{
    (void)win;
    (void)world;
    (void)cam;
}
HX_API void HX_CALL hx_draw_mesh(HxWin win, HxMesh mesh, HxSkin skin, const HxMat4* transform, HxCam cam)
{
    (void)win;
    (void)mesh;
    (void)skin;
    (void)transform;
    (void)cam;
}
HX_API void HX_CALL hx_begin_pass(HxWin win, HxCam cam)
{
    (void)win;
    (void)cam;
}
HX_API void HX_CALL hx_end_pass(HxWin win) { (void)win; }

HX_API HxFont HX_CALL hx_load_font(const char* path, float size)
{
    (void)path;
    (void)size;
    return nullptr;
}
HX_API HxResult HX_CALL hx_drop_font(HxFont font) { return hx_unavailable_drop(font); }
HX_API void HX_CALL hx_say(HxWin win, HxFont font, const char* text, float x, float y, float size, HxColor color)
{
    (void)win;
    (void)font;
    (void)text;
    (void)x;
    (void)y;
    (void)size;
    (void)color;
}

HX_API HxRig HX_CALL hx_load_rig(const char* path)
{
    (void)path;
    return nullptr;
}
HX_API HxClip HX_CALL hx_load_clip(const char* path)
{
    (void)path;
    return nullptr;
}
HX_API void HX_CALL hx_play_clip(HxRig rig, HxClip clip, bool loop)
{
    (void)rig;
    (void)clip;
    (void)loop;
}
HX_API HxResult HX_CALL hx_drop_rig(HxRig rig) { return hx_unavailable_drop(rig); }
HX_API HxResult HX_CALL hx_drop_clip(HxClip clip) { return hx_unavailable_drop(clip); }

HX_API HxFx HX_CALL hx_add_fx(HxWin win, HxFxType type)
{
    (void)win;
    (void)type;
    return nullptr;
}
HX_API HxResult HX_CALL hx_drop_fx(HxFx fx) { return hx_unavailable_drop(fx); }
