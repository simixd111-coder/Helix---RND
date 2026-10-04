// test_sprite.cpp — Sprite sheet and animation lifetime test
#include "helix.h"
#include <cstdio>

static bool write_image(const char* path) {
    FILE* file = fopen(path, "wb");
    if (!file) return false;
    const unsigned char pixels[8 * 4 * 3] = {};
    bool ok = fputs("P6\n8 4\n255\n", file) >= 0 && fwrite(pixels, sizeof(pixels), 1, file) == 1;
    return fclose(file) == 0 && ok;
}

int main() {
    const char* path = "helix_sprite_sheet.ppm";
    if (!write_image(path)) return 1;
    HxPic pic = hx_load_pic(path);
    remove(path);
    if (!pic) return 2;

    HxSpriteSheet sheet = hx_make_sprite_sheet(pic, 4, 4);
    if (!sheet || hx_get_sprite_sheet_frame_count(sheet) != 2) return 3;

    HxRectI rect{};
    if (hx_get_sprite_sheet_frame(sheet, 1, &rect) != HX_OK || rect.x != 4 || rect.y != 0 || rect.width != 4 || rect.height != 4) return 4;

    const uint32_t frames[] = {0, 1, 0};
    const float durations[] = {0.1f, 0.2f, 0.1f};
    HxSpriteAnim anim = hx_make_sprite_anim(sheet, frames, durations, 3, true);
    if (!anim) return 5;

    if (hx_drop_sprite_sheet(sheet) != HX_OK || hx_drop_pic(pic) != HX_OK) return 6;
    if (hx_update_sprite_anim(anim, 0.15) != HX_OK) return 7;
    size_t sequence_index = 0;
    if (hx_get_sprite_anim_frame(anim, &sequence_index, &rect) != HX_OK || sequence_index != 1 || rect.x != 4) return 8;
    if (hx_update_sprite_anim(anim, 0.25) != HX_OK || hx_get_sprite_anim_done(anim)) return 9;
    if (hx_get_sprite_anim_frame(anim, &sequence_index, &rect) != HX_OK || sequence_index != 0 || rect.x != 0) return 10;
    if (hx_update_sprite_anim(anim, -1.0) != HX_ERR_INVALID_ARG) return 11;

    HxMemoryStats stats{};
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 1 || stats.cpu_bytes == 0) return 12;
    if (hx_drop_sprite_anim(anim) != HX_OK) return 13;
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 0 || stats.cpu_bytes != 0) return 14;

    std::puts("test_sprite: PASS");
    return 0;
}
