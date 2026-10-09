// main.cpp — Headless Triangle Demo (Phase 1)
// Renders a spinning colored triangle to a PNG file using the software backend.
#include "helix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Internal core PNG writer (valid PNG: zlib stored blocks + CRC32 chunks).
// Declared extern; the demo links the static core library.
extern bool hx_png_write_rgba8(const char* path, int width, int height, const void* pixels, size_t stride);

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    printf("Helix RND — Headless Triangle Demo (Phase 1)\n");
    printf("=============================================\n\n");

    // Boot engine (software backend)
    HxCfg cfg = {0};
    cfg.gpu = HX_GPU_SOFT;
    cfg.headless = true;
    cfg.app_name = "headless_triangle";
    HxResult res = hx_boot(&cfg);
    if (res != HX_OK)
    {
        fprintf(stderr, "hx_boot failed: %s\n", hx_last_error());
        return 1;
    }
    printf("Backend: %s\n", hx_get_backend_name(hx_get_backend()));

    // Create headless window (320x240)
    HxWin win = hx_make_win(320, 240, "Headless Triangle", HX_WIN_HEADLESS);
    if (!win)
    {
        fprintf(stderr, "hx_make_win failed: %s\n", hx_last_error());
        hx_quit();
        return 1;
    }

    // World / camera / skin
    HxWorld world = hx_make_world();
    HxCam cam = hx_make_cam3d();
    hx_set_cam_persp(cam, 60.0f, 320.0f / 240.0f, 0.1f, 100.0f);
    HxVec3 eye = {0, 0, 3};
    HxVec3 target = {0, 0, 0};
    HxVec3 up = {0, 1, 0};
    hx_look(cam, &target, &eye, &up);
    HxSkin skin = hx_make_skin(HX_WHITE, HX_SKIN_UNLIT | HX_SKIN_DOUBLE_SIDED);
    if (!world || !cam || !skin)
    {
        fprintf(stderr, "Failed to create world/camera/skin\n");
        if (skin)
            hx_drop_skin(skin);
        if (cam)
            hx_drop_cam(cam);
        if (world)
            hx_drop_world(world);
        hx_drop_win(win);
        hx_quit();
        return 1;
    }

    // Triangle vertices (NDC: -1..1), per-vertex RGB colors
    typedef struct
    {
        HxVec3 pos;
        HxVec4 color;
    } TriVertex;
    const TriVertex vertices[3] = {
        {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},   // Top (red)
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}, // Bottom-left (green)
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Bottom-right (blue)
    };
    HxMesh mesh =
        hx_make_mesh(vertices, 3, HX_VERT_POS | HX_VERT_COLOR, sizeof(TriVertex), NULL, 0, false, HX_PRIM_TRIANGLES);
    if (!mesh)
    {
        fprintf(stderr, "hx_make_mesh failed: %s\n", hx_last_error());
        hx_drop_skin(skin);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_drop_win(win);
        hx_quit();
        return 1;
    }

    // Output buffer (320x240 RGBA8)
    const int width = 320, height = 240;
    const size_t stride = (size_t) width * 4;
    uint8_t* pixels = (uint8_t*) malloc(stride * height);
    if (!pixels)
    {
        fprintf(stderr, "Out of memory\n");
        hx_drop_mesh(mesh);
        hx_drop_skin(skin);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_drop_win(win);
        hx_quit();
        return 1;
    }

    // Render a few frames (spinning); save frame 5 as output
    const int kSaveFrame = 5;
    for (int frame = 0; frame < 10; ++frame)
    {
        // Rotate triangle around Y
        float angle = frame * 0.2f;
        HxQuat rot;
        HxVec3 axis = {0, 1, 0};
        hx_make_quat_axis_angle(&axis, angle, &rot);
        HxVec3 t = {0, 0, 0};
        HxVec3 s = {1, 1, 1};
        HxMat4 transform;
        hx_make_mat4_trs(&t, &rot, &s, &transform);

        hx_clear_world(world);
        hx_add_mesh(world, mesh, skin, &transform);

        HxResult rr = hx_render_headless(width, height, world, cam, pixels, stride);
        if (rr != HX_OK)
        {
            fprintf(stderr, "hx_render_headless failed: %s\n", hx_last_error());
            free(pixels);
            hx_drop_mesh(mesh);
            hx_drop_skin(skin);
            hx_drop_cam(cam);
            hx_drop_world(world);
            hx_drop_win(win);
            hx_quit();
            return 1;
        }

        if (frame == kSaveFrame)
        {
            if (!hx_png_write_rgba8("triangle.png", width, height, pixels, stride))
            {
                fprintf(stderr, "Failed to write triangle.png\n");
                free(pixels);
                hx_drop_mesh(mesh);
                hx_drop_skin(skin);
                hx_drop_cam(cam);
                hx_drop_world(world);
                hx_drop_win(win);
                hx_quit();
                return 1;
            }
            printf("Wrote triangle.png (%dx%d)\n", width, height);
        }
    }

    // Cleanup
    free(pixels);
    hx_drop_mesh(mesh);
    hx_drop_skin(skin);
    hx_drop_cam(cam);
    hx_drop_world(world);
    hx_drop_win(win);
    hx_quit();

    printf("\nDone! Check triangle.png\n");
    return 0;
}