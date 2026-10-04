// main.cpp — Headless Triangle Demo (Phase 1)
// Renders a spinning colored triangle to a PNG file using the software backend.
#include "helix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
// PNG Writer (minimal, no external deps)
// -----------------------------------------------------------------------------
static uint32_t hx_to_be32(uint32_t value) {
    return ((value & 0x000000FFu) << 24) |
           ((value & 0x0000FF00u) << 8) |
           ((value & 0x00FF0000u) >> 8) |
           ((value & 0xFF000000u) >> 24);
}

static void write_png(const char* path, int w, int h, const void* pixels, size_t stride) {
    FILE* f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "Failed to open %s\n", path); return; }

    // PNG signature
    fputc(0x89, f); fputc('P', f); fputc('N', f); fputc('G', f);
    fputc(0x0D, f); fputc(0x0A, f); fputc(0x1A, f); fputc(0x0A, f);

    // Helper: write chunk
    auto write_chunk = [&](const char* type, const void* data, size_t len) {
        uint32_t be_len = hx_to_be32(static_cast<uint32_t>(len));
        fwrite(&be_len, 4, 1, f);
        fwrite(type, 4, 1, f);
        if (data && len) fwrite(data, 1, len, f);
        // CRC (simplified: skip for demo, real impl would compute)
        uint32_t crc = 0;
        fwrite(&crc, 4, 1, f);
    };

    // IHDR
    struct { uint32_t w, h; uint8_t depth, color, comp, filter, interlace; } ihdr;
    ihdr.w = hx_to_be32(static_cast<uint32_t>(w));
    ihdr.h = hx_to_be32(static_cast<uint32_t>(h));
    ihdr.depth = 8;
    ihdr.color = 6; // RGBA
    ihdr.comp = 0;
    ihdr.filter = 0;
    ihdr.interlace = 0;
    write_chunk("IHDR", &ihdr, 13);

    // IDAT (uncompressed, each row with filter byte 0)
    size_t row_bytes = w * 4 + 1;
    size_t idat_size = row_bytes * h;
    uint8_t* idat = (uint8_t*)malloc(idat_size);
    const uint8_t* src = (const uint8_t*)pixels;
    for (int y = 0; y < h; ++y) {
        idat[y * row_bytes] = 0; // filter type 0
        memcpy(idat + y * row_bytes + 1, src + y * stride, w * 4);
    }
    write_chunk("IDAT", idat, idat_size);
    free(idat);

    // IEND
    write_chunk("IEND", NULL, 0);

    fclose(f);
    printf("Wrote %s (%dx%d)\n", path, w, h);
}

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("Helix RND — Headless Triangle Demo (Phase 1)\n");
    printf("=============================================\n\n");

    // Boot engine (software backend)
    HxCfg cfg = {0};
    cfg.gpu = HX_GPU_SOFT;
    cfg.headless = true;
    cfg.app_name = "headless_triangle";
    HxResult res = hx_boot(&cfg);
    if (res != HX_OK) {
        fprintf(stderr, "hx_boot failed: %s\n", hx_last_error());
        return 1;
    }
    printf("Backend: %s\n", hx_get_backend_name(hx_get_backend()));

    // Create headless window (320x240)
    HxWin win = hx_make_win(320, 240, "Headless Triangle", HX_WIN_HEADLESS);
    if (!win) {
        fprintf(stderr, "hx_make_win failed: %s\n", hx_last_error());
        hx_quit();
        return 1;
    }

    // Triangle vertices (NDC: -1..1)
    HxVec3 positions[3] = {
        { 0.0f,  0.5f, 0.0f },  // Top
        {-0.5f, -0.5f, 0.0f },  // Bottom-left
        { 0.5f, -0.5f, 0.0f },  // Bottom-right
    };
    HxVec3 colors[3] = {
        { 1.0f, 0.0f, 0.0f },  // Red
        { 0.0f, 1.0f, 0.0f },  // Green
        { 0.0f, 0.0f, 1.0f },  // Blue
    };

    // Camera
    HxCam cam = hx_make_cam3d();
    hx_set_cam_persp(cam, 60.0f, 320.0f / 240.0f, 0.1f, 100.0f);
    HxVec3 eye = {0, 0, 3};
    HxVec3 target = {0, 0, 0};
    HxVec3 up = {0, 1, 0};
    hx_look(cam, &target, &eye, &up);

    // View-projection matrix
    HxMat4 view, proj, vp;
    hx_get_cam_view(cam, &view);
    hx_get_cam_proj(cam, &proj);
    hx_mul_mat4(&view, &proj, &vp);

    // Set up software renderer
    extern void hx_soft_set_view_proj(const HxMat4*);
    extern void hx_soft_draw_triangles(const HxVec3*, const HxVec3*, int);
    extern void hx_soft_clear(void);
    extern void hx_soft_get_pixels(void**, size_t*, int*, int*);

    hx_soft_set_view_proj(&vp);

    // Render a few frames (spinning)
    for (int frame = 0; frame < 10; ++frame) {
        // Rotate triangle
        float angle = frame * 0.2f;
        HxQuat rot;
        HxVec3 axis = {0, 1, 0};
        hx_make_quat_axis_angle(&axis, angle, &rot);
        HxMat4 rot_mat;
        hx_make_mat4_rotate(&rot, &rot_mat);

        HxVec3 rotated[3];
        for (int i = 0; i < 3; ++i) {
            hx_rotate_vec_quat(&rot, &positions[i], &rotated[i]);
        }

        hx_soft_clear();
        hx_soft_draw_triangles(rotated, colors, 3);

        // Save frame 5 as output
        if (frame == 5) {
            void* pixels; size_t stride; int w, h;
            hx_soft_get_pixels(&pixels, &stride, &w, &h);
            write_png("triangle.png", w, h, pixels, stride);
        }
    }

    // Cleanup
    hx_drop_cam(cam);
    hx_drop_win(win);
    hx_quit();

    printf("\nDone! Check triangle.png\n");
    return 0;
}