#include "helix.h"
#include "render/software/soft_font.h"

static HxResult hx_unavailable_drop(const void* handle)
{
    return handle ? HX_ERR_NOT_IMPLEMENTED : HX_ERR_INVALID_HANDLE;
}

HX_API HxLamp HX_CALL hx_make_lamp(HxLampType type, HxColor color, float intensity)
{
    (void) type;
    (void) color;
    (void) intensity;
    return nullptr;
}
HX_API void HX_CALL hx_set_lamp_dir(HxLamp lamp, const HxVec3* direction)
{
    (void) lamp;
    (void) direction;
}
HX_API void HX_CALL hx_set_lamp_range(HxLamp lamp, float range)
{
    (void) lamp;
    (void) range;
}
HX_API void HX_CALL hx_set_lamp_spot(HxLamp lamp, float inner_deg, float outer_deg)
{
    (void) lamp;
    (void) inner_deg;
    (void) outer_deg;
}
HX_API HxResult HX_CALL hx_drop_lamp(HxLamp lamp)
{
    return hx_unavailable_drop(lamp);
}

HX_API HxShader HX_CALL hx_load_shader(const char* name)
{
    (void) name;
    return nullptr;
}
HX_API HxResult HX_CALL hx_drop_shader(HxShader shader)
{
    return hx_unavailable_drop(shader);
}

HX_API void HX_CALL hx_draw_world(HxWin win, HxWorld world, HxCam cam)
{
    (void) win;
    (void) world;
    (void) cam;
}
HX_API void HX_CALL hx_draw_mesh(HxWin win, HxMesh mesh, HxSkin skin, const HxMat4* transform, HxCam cam)
{
    (void) win;
    (void) mesh;
    (void) skin;
    (void) transform;
    (void) cam;
}
HX_API void HX_CALL hx_begin_pass(HxWin win, HxCam cam)
{
    (void) win;
    (void) cam;
}
HX_API void HX_CALL hx_end_pass(HxWin win)
{
    (void) win;
}

HX_API HxFont HX_CALL hx_load_font(const char* path, float size)
{
    return hx_font_load_from_file(path, size);
}
HX_API HxFont HX_CALL hx_load_font_mem(const void* data, size_t size, float pt_size)
{
    return hx_font_load_from_memory(data, size, pt_size);
}
HX_API HxFont HX_CALL hx_load_font_dpi(const char* path, float pt_size, float dpi)
{
    return hx_font_load_from_file_dpi(path, pt_size, dpi);
}
HX_API HxResult HX_CALL hx_drop_font(HxFont font)
{
    if (!font)
        return HX_ERR_INVALID_HANDLE;
    hx_font_drop(font);
    return HX_OK;
}
HX_API void HX_CALL hx_draw_text(HxWin win, HxFont font, const char* text, float x, float y, HxColor color)
{
    hx_soft_draw_text(win, font, text, x, y, color);
}
HX_API void HX_CALL hx_say(HxWin win, HxFont font, const char* text, float x, float y, float size, HxColor color)
{
    // Legacy function: ignore size parameter, use font's built-in size
    (void) size;
    hx_soft_draw_text(win, font, text, x, y, color);
}
HX_API void HX_CALL hx_measure_text(HxFont font, const char* text, float* out_width, float* out_height)
{
    hx_font_measure(font, text, out_width, out_height);
}

HX_API HxRig HX_CALL hx_load_rig(const char* path)
{
    (void) path;
    return nullptr;
}
HX_API HxClip HX_CALL hx_load_clip(const char* path)
{
    (void) path;
    return nullptr;
}
HX_API void HX_CALL hx_play_clip(HxRig rig, HxClip clip, bool loop)
{
    (void) rig;
    (void) clip;
    (void) loop;
}
HX_API HxResult HX_CALL hx_drop_rig(HxRig rig)
{
    return hx_unavailable_drop(rig);
}
HX_API HxResult HX_CALL hx_drop_clip(HxClip clip)
{
    return hx_unavailable_drop(clip);
}

HX_API HxFx HX_CALL hx_add_fx(HxWin win, HxFxType type)
{
    (void) win;
    (void) type;
    return nullptr;
}
HX_API HxResult HX_CALL hx_drop_fx(HxFx fx)
{
    return hx_unavailable_drop(fx);
}
