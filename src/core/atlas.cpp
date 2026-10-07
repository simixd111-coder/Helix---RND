#include "helix.h"
#include "core/render_internal.h"
#include "resource_internal.h"
#include <cstring>
#include <limits>
#include <new>
#include <vector>

struct HxAtlasImpl
{
    int width;
    int height;
    bool valid;
    std::vector<uint8_t> rgba_pixels;
};

static void hx_atlas_destroy_resource(void* resource) { delete static_cast<HxAtlas>(resource); }

HX_API HxAtlas HX_CALL hx_make_atlas(int width, int height)
{
    if (width <= 0 || height <= 0 ||
        static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / static_cast<size_t>(height) / 4u)
        return nullptr;
    HxAtlas atlas = new (std::nothrow) HxAtlasImpl{width, height, true, {}};
    if (!atlas)
        return nullptr;
    try
    {
        atlas->rgba_pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u, 0);
    }
    catch (...)
    {
        delete atlas;
        return nullptr;
    }
    if (atlas->rgba_pixels.capacity() > std::numeric_limits<size_t>::max() - sizeof(HxAtlasImpl))
    {
        delete atlas;
        return nullptr;
    }
    const size_t bytes = sizeof(HxAtlasImpl) + atlas->rgba_pixels.capacity();
    if (!hx_resource_register(atlas, bytes, 0, hx_atlas_destroy_resource))
    {
        delete atlas;
        return nullptr;
    }
    return atlas;
}

HX_API void HX_CALL hx_add_atlas(HxAtlas atlas, const char* name, HxTex texture, int x, int y, int width, int height)
{
    (void)name;
    if (!hx_resource_is_registered(atlas))
        return;
    HxTextureRenderData source{};
    if (!hx_texture_get_render_data(texture, &source) || x < 0 || y < 0 || width <= 0 || height <= 0 ||
        width > atlas->width || height > atlas->height || x > atlas->width - width || y > atlas->height - height ||
        width > source.width || height > source.height)
    {
        atlas->valid = false;
        return;
    }
    for (int row = 0; row < height; ++row)
    {
        const size_t source_offset = static_cast<size_t>(row) * static_cast<size_t>(source.width) * 4u;
        const size_t destination_offset =
            (static_cast<size_t>(y + row) * static_cast<size_t>(atlas->width) + static_cast<size_t>(x)) * 4u;
        std::memcpy(atlas->rgba_pixels.data() + destination_offset, source.rgba_pixels + source_offset,
                    static_cast<size_t>(width) * 4u);
    }
}

HX_API HxTex HX_CALL hx_build_atlas(HxAtlas atlas)
{
    if (!hx_resource_is_registered(atlas) || !atlas->valid)
        return nullptr;
    return hx_make_tex(atlas->width, atlas->height, HX_TEX_FMT_RGBA8, atlas->rgba_pixels.data(), HX_TEX_NONE);
}

HX_API HxResult HX_CALL hx_drop_atlas(HxAtlas atlas)
{
    if (!atlas)
        return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(atlas))
        return HX_ERR_ALREADY_DROPPED;
    delete atlas;
    return HX_OK;
}
