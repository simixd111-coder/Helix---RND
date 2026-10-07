#include "helix.h"

HxResult hx_soft_render_world(int width, int height, HxWorld world, HxCam cam, void* out_pixels, size_t out_stride)
{
    (void)width;
    (void)height;
    (void)world;
    (void)cam;
    (void)out_pixels;
    (void)out_stride;
    return HX_ERR_BACKEND_UNAVAILABLE;
}