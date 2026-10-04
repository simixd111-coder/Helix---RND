// sprite.cpp — Grid sprite sheets and deterministic frame animation
#include "helix.h"
#include "resource_internal.h"
#include <algorithm>
#include <cmath>
#include <new>
#include <vector>

struct HxSpriteSheetImpl {
    int image_width;
    int image_height;
    int frame_width;
    int frame_height;
    size_t columns;
    size_t rows;
    size_t frame_count;
};

struct HxSpriteAnimImpl {
    std::vector<HxRectI> frames;
    std::vector<double> durations;
    double duration_total;
    double elapsed;
    size_t current;
    bool loop;
    bool done;
};

static void hx_sprite_sheet_destroy(void* resource) {
    delete static_cast<HxSpriteSheet>(resource);
}

static void hx_sprite_anim_destroy(void* resource) {
    delete static_cast<HxSpriteAnim>(resource);
}

HX_API HxSpriteSheet HX_CALL hx_make_sprite_sheet(HxPic pic, int frame_width, int frame_height) {
    if (!pic || frame_width <= 0 || frame_height <= 0) return NULL;
    int image_width = 0;
    int image_height = 0;
    hx_get_pic_size(pic, &image_width, &image_height);
    if (image_width <= 0 || image_height <= 0 || image_width % frame_width != 0 || image_height % frame_height != 0) {
        return NULL;
    }

    HxSpriteSheet sheet = new (std::nothrow) HxSpriteSheetImpl{};
    if (!sheet) return NULL;
    sheet->image_width = image_width;
    sheet->image_height = image_height;
    sheet->frame_width = frame_width;
    sheet->frame_height = frame_height;
    sheet->columns = static_cast<size_t>(image_width / frame_width);
    sheet->rows = static_cast<size_t>(image_height / frame_height);
    sheet->frame_count = sheet->columns * sheet->rows;
    if (!hx_resource_register(sheet, sizeof(HxSpriteSheetImpl), 0, hx_sprite_sheet_destroy)) {
        delete sheet;
        return NULL;
    }
    return sheet;
}

HX_API size_t HX_CALL hx_get_sprite_sheet_frame_count(HxSpriteSheet sheet) {
    return sheet ? sheet->frame_count : 0;
}

HX_API HxResult HX_CALL hx_get_sprite_sheet_frame(HxSpriteSheet sheet, size_t index, HxRectI* out_rect) {
    if (!sheet || !out_rect || index >= sheet->frame_count) return HX_ERR_INVALID_ARG;
    const size_t column = index % sheet->columns;
    const size_t row = index / sheet->columns;
    out_rect->x = static_cast<int>(column) * sheet->frame_width;
    out_rect->y = static_cast<int>(row) * sheet->frame_height;
    out_rect->width = sheet->frame_width;
    out_rect->height = sheet->frame_height;
    return HX_OK;
}

HX_API HxResult HX_CALL hx_drop_sprite_sheet(HxSpriteSheet sheet) {
    if (!sheet) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(sheet)) return HX_ERR_ALREADY_DROPPED;
    delete sheet;
    return HX_OK;
}

HX_API HxSpriteAnim HX_CALL hx_make_sprite_anim(
    HxSpriteSheet sheet,
    const uint32_t* frame_indices,
    const float* frame_durations,
    size_t frame_count,
    bool loop
) {
    if (!sheet || !frame_indices || !frame_durations || frame_count == 0) return NULL;

    HxSpriteAnim anim = new (std::nothrow) HxSpriteAnimImpl{};
    if (!anim) return NULL;
    anim->loop = loop;

    try {
        anim->frames.reserve(frame_count);
        anim->durations.reserve(frame_count);
        for (size_t index = 0; index < frame_count; ++index) {
            if (frame_indices[index] >= sheet->frame_count || !std::isfinite(frame_durations[index]) || frame_durations[index] <= 0.0f) {
                delete anim;
                return NULL;
            }
            HxRectI rect{};
            const size_t frame_index = frame_indices[index];
            const size_t column = frame_index % sheet->columns;
            const size_t row = frame_index / sheet->columns;
            rect.x = static_cast<int>(column) * sheet->frame_width;
            rect.y = static_cast<int>(row) * sheet->frame_height;
            rect.width = sheet->frame_width;
            rect.height = sheet->frame_height;
            anim->frames.push_back(rect);
            anim->durations.push_back(static_cast<double>(frame_durations[index]));
            anim->duration_total += static_cast<double>(frame_durations[index]);
            if (!std::isfinite(anim->duration_total)) {
                delete anim;
                return NULL;
            }
        }
    } catch (...) {
        delete anim;
        return NULL;
    }

    const size_t cpu_bytes = sizeof(HxSpriteAnimImpl) +
        anim->frames.capacity() * sizeof(HxRectI) +
        anim->durations.capacity() * sizeof(double);
    if (!hx_resource_register(anim, cpu_bytes, 0, hx_sprite_anim_destroy)) {
        delete anim;
        return NULL;
    }
    return anim;
}

HX_API HxResult HX_CALL hx_update_sprite_anim(HxSpriteAnim anim, double delta_seconds) {
    if (!anim || !std::isfinite(delta_seconds) || delta_seconds < 0.0) return HX_ERR_INVALID_ARG;
    if (anim->done || anim->frames.empty()) return HX_OK;

    if (anim->loop) {
        const double cycle_position = anim->elapsed + std::fmod(delta_seconds, anim->duration_total);
        const double boundary_tolerance = 1e-6 * std::max(1.0, anim->duration_total);
        anim->elapsed = cycle_position >= anim->duration_total - boundary_tolerance
            ? 0.0
            : std::fmod(cycle_position, anim->duration_total);
    } else {
        anim->elapsed = std::min(anim->duration_total, anim->elapsed + delta_seconds);
        if (anim->elapsed >= anim->duration_total) anim->done = true;
    }

    double remaining = anim->elapsed;
    anim->current = anim->frames.size() - 1;
    for (size_t index = 0; index < anim->durations.size(); ++index) {
        if (remaining < anim->durations[index]) {
            anim->current = index;
            break;
        }
        remaining -= anim->durations[index];
    }
    return HX_OK;
}

HX_API HxResult HX_CALL hx_get_sprite_anim_frame(HxSpriteAnim anim, size_t* out_frame_index, HxRectI* out_rect) {
    if (!anim || anim->frames.empty() || !out_frame_index || !out_rect) return HX_ERR_INVALID_ARG;
    *out_frame_index = anim->current;
    *out_rect = anim->frames[anim->current];
    return HX_OK;
}

HX_API bool HX_CALL hx_get_sprite_anim_done(HxSpriteAnim anim) {
    return anim && anim->done;
}

HX_API HxResult HX_CALL hx_restart_sprite_anim(HxSpriteAnim anim) {
    if (!anim) return HX_ERR_INVALID_HANDLE;
    anim->elapsed = 0.0;
    anim->current = 0;
    anim->done = false;
    return HX_OK;
}

HX_API HxResult HX_CALL hx_drop_sprite_anim(HxSpriteAnim anim) {
    if (!anim) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(anim)) return HX_ERR_ALREADY_DROPPED;
    delete anim;
    return HX_OK;
}
