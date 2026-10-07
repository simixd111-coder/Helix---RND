#include "helix.h"
#include "core/render_internal.h"
#include "resource_internal.h"
#include <cstring>
#include <limits>
#include <new>
#include <utility>
#include <vector>

struct HxTexImpl
{
    int width = 0;
    int height = 0;
    HxTexFmt format = HX_TEX_FMT_RGBA8;
    HxTexFlags flags = HX_TEX_NONE;
    bool cube = false;
    std::vector<uint8_t> rgba_pixels;
};

static size_t hx_texture_source_bpp(HxTexFmt format)
{
    switch (format)
    {
    case HX_TEX_FMT_R8:
        return 1;
    case HX_TEX_FMT_RG8:
        return 2;
    case HX_TEX_FMT_RGB8:
        return 3;
    case HX_TEX_FMT_RGBA8:
    case HX_TEX_FMT_BGRA8:
        return 4;
    default:
        return 0;
    }
}

static void hx_texture_destroy_resource(void* resource) { delete static_cast<HxTex>(resource); }

static bool hx_texture_copy_face(std::vector<uint8_t>& output, size_t offset, size_t pixel_count, HxTexFmt format,
                                 const void* pixels)
{
    const size_t source_bpp = hx_texture_source_bpp(format);
    if (source_bpp == 0 || pixel_count > std::numeric_limits<size_t>::max() / source_bpp ||
        pixel_count > std::numeric_limits<size_t>::max() / 4u || offset > output.size() ||
        pixel_count * 4u > output.size() - offset)
        return false;

    const auto* source = static_cast<const uint8_t*>(pixels);
    for (size_t i = 0; i < pixel_count; ++i)
    {
        uint8_t* destination = output.data() + offset + i * 4u;
        destination[0] = source ? source[i * source_bpp] : 0;
        destination[1] = source_bpp > 1 && source ? source[i * source_bpp + 1] : 0;
        destination[2] = source_bpp > 2 && source ? source[i * source_bpp + 2] : 0;
        destination[3] = source_bpp > 3 && source ? source[i * source_bpp + 3] : 255;
        if (format == HX_TEX_FMT_BGRA8)
            std::swap(destination[0], destination[2]);
    }
    return true;
}

static HxTex hx_texture_create(int width, int height, HxTexFmt format, HxTexFlags flags, bool cube,
                               const void* const* faces)
{
    const size_t source_bpp = hx_texture_source_bpp(format);
    const HxTexFlags known_flags = HX_TEX_SRGB | HX_TEX_MIPMAPS | HX_TEX_REPEAT | HX_TEX_MIRROR | HX_TEX_LINEAR;
    if (width <= 0 || height <= 0 || source_bpp == 0 || (flags & ~known_flags) != 0 ||
        ((flags & HX_TEX_REPEAT) && (flags & HX_TEX_MIRROR)) ||
        static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / static_cast<size_t>(height))
        return nullptr;

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    const size_t face_count = cube ? 6u : 1u;
    if (pixel_count > std::numeric_limits<size_t>::max() / 4u / face_count)
        return nullptr;

    HxTex texture = new (std::nothrow) HxTexImpl{};
    if (!texture)
        return nullptr;
    try
    {
        texture->rgba_pixels.resize(pixel_count * 4u * face_count);
        for (size_t face = 0; face < face_count; ++face)
        {
            const void* data = faces ? faces[face] : nullptr;
            if (!hx_texture_copy_face(texture->rgba_pixels, face * pixel_count * 4u, pixel_count, format, data))
            {
                delete texture;
                return nullptr;
            }
        }
    }
    catch (...)
    {
        delete texture;
        return nullptr;
    }

    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->flags = flags;
    texture->cube = cube;
    if (texture->rgba_pixels.capacity() > std::numeric_limits<size_t>::max() - sizeof(HxTexImpl))
    {
        delete texture;
        return nullptr;
    }
    const size_t bytes = sizeof(HxTexImpl) + texture->rgba_pixels.capacity();
    if (!hx_resource_register(texture, bytes, 0, hx_texture_destroy_resource))
    {
        delete texture;
        return nullptr;
    }
    return texture;
}

HX_API HxTex HX_CALL hx_make_tex(int width, int height, HxTexFmt format, const void* pixels, HxTexFlags flags)
{
    const void* faces[] = {pixels};
    return hx_texture_create(width, height, format, flags, false, faces);
}

HX_API HxTex HX_CALL hx_make_tex_cube(int size, HxTexFmt format, const void* faces[6], HxTexFlags flags)
{
    if (!faces)
        return nullptr;
    for (size_t face = 0; face < 6; ++face)
        if (!faces[face])
            return nullptr;
    return hx_texture_create(size, size, format, flags, true, faces);
}

HX_API HxTex HX_CALL hx_load_tex(const char* path, HxTexFlags flags)
{
    HxPic picture = hx_load_pic(path);
    if (!picture)
        return nullptr;
    int width = 0;
    int height = 0;
    void* pixels = nullptr;
    size_t stride = 0;
    hx_get_pic_size(picture, &width, &height);
    hx_get_pic_pixels(picture, &pixels, &stride);
    HxTex texture = nullptr;
    if (pixels && stride == static_cast<size_t>(width) * 4u)
        texture = hx_make_tex(width, height, HX_TEX_FMT_RGBA8, pixels, flags);
    hx_drop_pic(picture);
    return texture;
}

HX_API HxResult HX_CALL hx_drop_tex(HxTex texture)
{
    if (!texture)
        return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(texture))
        return HX_ERR_ALREADY_DROPPED;
    delete texture;
    return HX_OK;
}

bool hx_texture_get_render_data(HxTex texture, HxTextureRenderData* out_data)
{
    if (!out_data || !hx_resource_is_registered(texture) || texture->cube)
        return false;
    *out_data = {texture->rgba_pixels.data(), texture->width, texture->height, texture->flags};
    return true;
}
