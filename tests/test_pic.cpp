// test_pic.cpp — CPU picture loading API test
#include "helix.h"
#include <cstdio>
#include <cstring>

static bool write_test_ppm(const char* path) {
    FILE* file = fopen(path, "wb");
    if (!file) return false;
    const unsigned char pixels[] = {255, 0, 0, 0, 255, 0};
    bool ok = fputs("P6\n2 1\n255\n", file) >= 0 &&
              fwrite(pixels, sizeof(pixels), 1, file) == 1;
    return fclose(file) == 0 && ok;
}

int main() {
    if (hx_load_pic(NULL) != NULL) return 1;
    if (hx_load_pic("helix_missing_image.png") != NULL) return 2;

    const char* path = "helix_test_pic.ppm";
    if (!write_test_ppm(path)) return 3;

    HxPic pic = hx_load_pic(path);
    remove(path);
    if (!pic) return 4;

    int width = 0;
    int height = 0;
    hx_get_pic_size(pic, &width, &height);
    if (width != 2 || height != 1) return 5;

    void* raw_pixels = NULL;
    size_t stride = 0;
    hx_get_pic_pixels(pic, &raw_pixels, &stride);
    if (!raw_pixels || stride != 8) return 6;

    const unsigned char expected[] = {255, 0, 0, 255, 0, 255, 0, 255};
    if (memcmp(raw_pixels, expected, sizeof(expected)) != 0) return 7;
    if (hx_drop_pic(pic) != HX_OK) return 8;

    std::puts("test_pic: PASS");
    return 0;
}
