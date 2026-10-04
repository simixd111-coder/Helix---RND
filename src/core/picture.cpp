// picture.cpp — CPU image loading for renderer-independent asset pipelines
#include "helix.h"
#include "resource_internal.h"
#include <stdint.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct HxPicImpl {
    int width;
    int height;
    size_t stride;
    uint8_t* pixels;
};

static void hx_pic_destroy_resource(void* resource) {
    HxPic pic = static_cast<HxPic>(resource);
    stbi_image_free(pic->pixels);
    free(pic);
}

HX_API HxPic HX_CALL hx_load_pic(const char* path) {
    if (!path || !path[0]) return NULL;

    int width = 0;
    int height = 0;
    int source_channels = 0;
    stbi_uc* decoded = stbi_load(path, &width, &height, &source_channels, STBI_rgb_alpha);
    if (!decoded || width <= 0 || height <= 0) {
        stbi_image_free(decoded);
        return NULL;
    }

    HxPic pic = static_cast<HxPic>(malloc(sizeof(HxPicImpl)));
    if (!pic) {
        stbi_image_free(decoded);
        return NULL;
    }

    pic->width = width;
    pic->height = height;
    pic->stride = static_cast<size_t>(width) * 4u;
    pic->pixels = decoded;
    const size_t cpu_bytes = sizeof(HxPicImpl) + pic->stride * static_cast<size_t>(height);
    if (!hx_resource_register(pic, cpu_bytes, 0, hx_pic_destroy_resource)) {
        stbi_image_free(decoded);
        free(pic);
        return NULL;
    }
    return pic;
}

HX_API void HX_CALL hx_get_pic_size(HxPic pic, int* width, int* height) {
    if (width) *width = pic ? pic->width : 0;
    if (height) *height = pic ? pic->height : 0;
}

HX_API void HX_CALL hx_get_pic_pixels(HxPic pic, void** out_pixels, size_t* out_stride) {
    if (out_pixels) *out_pixels = pic ? pic->pixels : NULL;
    if (out_stride) *out_stride = pic ? pic->stride : 0;
}

HX_API HxResult HX_CALL hx_drop_pic(HxPic pic) {
    if (!pic) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(pic)) return HX_ERR_ALREADY_DROPPED;
    stbi_image_free(pic->pixels);
    free(pic);
    return HX_OK;
}
