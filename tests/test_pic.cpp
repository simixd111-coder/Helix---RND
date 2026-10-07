// test_pic.cpp — CPU picture loading API test
#include "helix.h"
#include <cstdio>
#include <cstring>

static int fail_test(int code)
{
    std::fprintf(stderr, "test_pic: failed at check %d\n", code);
    return code;
}

static bool write_test_ppm(const char* path)
{
    FILE* file = fopen(path, "wb");
    if (!file)
        return false;
    const unsigned char pixels[] = {255, 0, 0, 0, 255, 0};
    bool ok = fputs("P6\n2 1\n255\n", file) >= 0 && fwrite(pixels, sizeof(pixels), 1, file) == 1;
    return fclose(file) == 0 && ok;
}

int main()
{
    if (hx_load_pic(NULL) != NULL)
        return fail_test(1);
    if (hx_load_pic("helix_missing_image.png") != NULL)
        return fail_test(2);

    const char* path = "helix_test_pic.ppm";
    if (!write_test_ppm(path))
        return fail_test(3);

    HxPic pic = hx_load_pic(path);
    remove(path);
    if (!pic)
        return fail_test(4);

    int width = 0;
    int height = 0;
    hx_get_pic_size(pic, &width, &height);
    if (width != 2 || height != 1)
        return fail_test(5);

    void* raw_pixels = NULL;
    size_t stride = 0;
    hx_get_pic_pixels(pic, &raw_pixels, &stride);
    if (!raw_pixels || stride != 8)
        return fail_test(6);

    const unsigned char expected[] = {255, 0, 0, 255, 0, 255, 0, 255};
    if (memcmp(raw_pixels, expected, sizeof(expected)) != 0)
        return fail_test(7);
    const char* png_path = "helix_test_pic.png";
    if (hx_save_pic(pic, png_path) != HX_OK)
        return fail_test(8);
    HxPic png = hx_load_pic(png_path);
    remove(png_path);
    if (!png)
        return fail_test(9);
    void* png_pixels = NULL;
    size_t png_stride = 0;
    hx_get_pic_pixels(png, &png_pixels, &png_stride);
    if (!png_pixels || png_stride != sizeof(expected) || memcmp(png_pixels, expected, sizeof(expected)) != 0)
        return fail_test(10);
    if (hx_drop_pic(png) != HX_OK)
        return fail_test(11);
    if (hx_drop_pic(pic) != HX_OK)
        return fail_test(12);

    std::puts("test_pic: PASS");
    return 0;
}
