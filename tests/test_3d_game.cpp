// test_3d_game.cpp — 3D Space Flight Game Test
// Demonstrates: mesh creation, camera control, world rendering, animation, PNG capture
// Inspired by classic space sims (Elite, Wing Commander, Freelancer)

#include "helix.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>

// Internal PNG writer (from core)
extern bool hx_png_write_rgba8(const char* path, int width, int height, const void* pixels, size_t stride);

// Simple FNV-1a hash for frame verification
static uint64_t image_hash(const uint8_t* bytes, size_t count)
{
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < count; ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

// Create a simple spaceship mesh (triangular wedge with cockpit)
static HxMesh make_spaceship()
{
    // Vertices: position + color
    typedef struct { HxVec3 pos; HxVec4 color; } Vertex;
    
    const Vertex vertices[] = {
        // Nose (red)
        {{ 0.0f,  0.0f,  1.5f }, { 1.0f, 0.2f, 0.2f, 1.0f }},
        // Cockpit (cyan)
        {{ 0.0f,  0.3f,  0.5f }, { 0.2f, 1.0f, 1.0f, 1.0f }},
        // Left wing tip (blue)
        {{-1.2f,  0.0f, -0.5f }, { 0.2f, 0.4f, 1.0f, 1.0f }},
        // Right wing tip (blue)
        {{ 1.2f,  0.0f, -0.5f }, { 0.2f, 0.4f, 1.0f, 1.0f }},
        // Rear center (gray)
        {{ 0.0f, -0.2f, -1.0f }, { 0.5f, 0.5f, 0.5f, 1.0f }},
        // Left engine (orange)
        {{-0.4f, -0.2f, -1.2f }, { 1.0f, 0.5f, 0.0f, 1.0f }},
        // Right engine (orange)
        {{ 0.4f, -0.2f, -1.2f }, { 1.0f, 0.5f, 0.0f, 1.0f }},
        // Rear bottom (dark gray) - 8th vertex for index 7
        {{ 0.0f, -0.4f, -1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }},
    };
    
    // Indices for triangles (12 triangles = 36 indices)
    const uint16_t indices[] = {
        // Nose cone (4 triangles)
        0, 2, 1,   0, 1, 3,   0, 3, 4,   0, 4, 2,
        // Cockpit top (2 triangles)
        1, 2, 4,   1, 4, 3,
        // Wings (4 triangles)
        2, 5, 4,   4, 5, 6,   3, 4, 6,   3, 6, 7,
        // Rear (2 triangles)
        4, 5, 6,   4, 6, 7,
    };
    
    return hx_make_mesh(vertices, 8, HX_VERT_POS | HX_VERT_COLOR, sizeof(Vertex),
                        indices, 36, false, HX_PRIM_TRIANGLES);
}

// Create asteroid mesh (irregular rock)
static HxMesh make_asteroid(float scale, const HxVec4& color)
{
    typedef struct { HxVec3 pos; HxVec4 color; } Vertex;
    
    // Simple icosphere-like shape (12 vertices, 20 triangles)
    const float t = (1.0f + sqrtf(5.0f)) / 2.0f; // golden ratio
    const Vertex vertices[] = {
        {{-1,  t,  0}, color}, {{ 1,  t,  0}, color}, {{-1, -t,  0}, color}, {{ 1, -t,  0}, color},
        {{ 0, -1,  t}, color}, {{ 0,  1,  t}, color}, {{ 0, -1, -t}, color}, {{ 0,  1, -t}, color},
        {{ t,  0, -1}, color}, {{ t,  0,  1}, color}, {{-t,  0, -1}, color}, {{-t,  0,  1}, color},
    };
    
    const uint16_t indices[] = {
        0,11,5,  0,5,1,  0,1,7,  0,7,10,  0,10,11,
        1,5,9,  5,11,4,  11,10,2, 10,7,6,  7,1,8,
        3,9,4,  3,4,2,  3,2,6,  3,6,8,  3,8,9,
        4,9,5,  2,4,11,  6,2,10,  8,6,7,  9,8,1,
    };
    
    // Scale vertices
    Vertex scaled[12];
    for (int i = 0; i < 12; ++i) {
        scaled[i].pos = { vertices[i].pos.x * scale, vertices[i].pos.y * scale, vertices[i].pos.z * scale };
        scaled[i].color = vertices[i].color;
    }
    
    return hx_make_mesh(scaled, 12, HX_VERT_POS | HX_VERT_COLOR, sizeof(Vertex),
                        indices, 60, false, HX_PRIM_TRIANGLES);
}

// Create starfield (points)
static HxMesh make_starfield(int count, float radius)
{
    typedef struct { HxVec3 pos; HxVec4 color; } Vertex;
    Vertex* vertices = (Vertex*)malloc(count * sizeof(Vertex));
    
    for (int i = 0; i < count; ++i) {
        // Random spherical distribution
        float u = (float)rand() / RAND_MAX;
        float v = (float)rand() / RAND_MAX;
        float theta = 2.0f * 3.14159f * u;
        float phi = acosf(2.0f * v - 1.0f);
        float r = radius * cbrtf(static_cast<float>(rand()) / RAND_MAX); // uniform in volume
        
        vertices[i].pos = {
            r * sinf(phi) * cosf(theta),
            r * sinf(phi) * sinf(theta),
            r * cosf(phi)
        };
        // White to slightly yellow stars
        float brightness = 0.5f + 0.5f * static_cast<float>(rand()) / RAND_MAX;
        vertices[i].color = { brightness, brightness * 0.95f, brightness * 0.9f, 1.0f };
    }
    
    // Create small quads (2 triangles each) for each star instead of points
    // Each star becomes 4 vertices (a small billboard quad)
    Vertex* star_vertices = (Vertex*)malloc(count * 4 * sizeof(Vertex));
    uint16_t* star_indices = (uint16_t*)malloc(count * 6 * sizeof(uint16_t));
    
    const float star_size = 0.1f; // Size of each star quad
    for (int i = 0; i < count; ++i) {
        HxVec3 center = vertices[i].pos;
        HxVec4 color = vertices[i].color;
        
        // Create a small quad facing the camera (billboard)
        // We'll orient it in XY plane, camera will see it as a point
        star_vertices[i * 4 + 0] = {{center.x - star_size, center.y - star_size, center.z}, color};
        star_vertices[i * 4 + 1] = {{center.x + star_size, center.y - star_size, center.z}, color};
        star_vertices[i * 4 + 2] = {{center.x + star_size, center.y + star_size, center.z}, color};
        star_vertices[i * 4 + 3] = {{center.x - star_size, center.y + star_size, center.z}, color};
        
        // Two triangles per quad
        star_indices[i * 6 + 0] = i * 4 + 0;
        star_indices[i * 6 + 1] = i * 4 + 1;
        star_indices[i * 6 + 2] = i * 4 + 2;
        star_indices[i * 6 + 3] = i * 4 + 0;
        star_indices[i * 6 + 4] = i * 4 + 2;
        star_indices[i * 6 + 5] = i * 4 + 3;
    }
    
    free(vertices);
    
    HxMesh mesh = hx_make_mesh(star_vertices, count * 4, HX_VERT_POS | HX_VERT_COLOR, sizeof(Vertex),
                               star_indices, count * 6, false, HX_PRIM_TRIANGLES);
    free(star_vertices);
    free(star_indices);
    return mesh;
}

int main()
{
    printf("Helix RND — 3D Space Flight Game Test\n");
    printf("======================================\n\n");
    
    // Boot engine (software backend, headless)
    HxCfg cfg = {0};
    cfg.gpu = HX_GPU_SOFT;
    cfg.headless = true;
    cfg.app_name = "test_3d_game";
    HxResult res = hx_boot(&cfg);
    if (res != HX_OK) {
        fprintf(stderr, "hx_boot failed: %s\n", hx_last_error());
        return 1;
    }
    printf("Backend: %s\n", hx_get_backend_name(hx_get_backend()));
    
    // Create world and camera
    HxWorld world = hx_make_world();
    HxCam cam = hx_make_cam3d();
    if (!world || !cam) {
        fprintf(stderr, "Failed to create world/camera\n");
        if (cam) hx_drop_cam(cam);
        if (world) hx_drop_world(world);
        hx_quit();
        return 1;
    }
    
    // Perspective camera
    const int width = 640, height = 480;
    hx_set_cam_persp(cam, 75.0f, (float)width / height, 0.1f, 500.0f);
    
    // Camera starts behind ship
    HxVec3 eye = {0, 2, -8};
    HxVec3 target = {0, 0, 0};
    HxVec3 up = {0, 1, 0};
    hx_look(cam, &target, &eye, &up);
    
    // Create game objects
    HxMesh ship = make_spaceship();
    HxMesh asteroid1 = make_asteroid(1.5f, {0.4f, 0.35f, 0.3f, 1.0f});
    HxMesh asteroid2 = make_asteroid(0.8f, {0.5f, 0.4f, 0.35f, 1.0f});
    HxMesh asteroid3 = make_asteroid(2.0f, {0.35f, 0.3f, 0.25f, 1.0f});
    HxMesh stars = make_starfield(500, 100.0f);
    
    if (!ship || !asteroid1 || !asteroid2 || !asteroid3 || !stars) {
        fprintf(stderr, "Failed to create meshes\n");
        if (ship) hx_drop_mesh(ship);
        if (asteroid1) hx_drop_mesh(asteroid1);
        if (asteroid2) hx_drop_mesh(asteroid2);
        if (asteroid3) hx_drop_mesh(asteroid3);
        if (stars) hx_drop_mesh(stars);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_quit();
        return 1;
    }
    
    // Unlit skin for all objects
    HxSkin skin = hx_make_skin(HX_WHITE, HX_SKIN_UNLIT | HX_SKIN_DOUBLE_SIDED);
    if (!skin) {
        fprintf(stderr, "Failed to create skin\n");
        hx_drop_mesh(ship);
        hx_drop_mesh(asteroid1);
        hx_drop_mesh(asteroid2);
        hx_drop_mesh(asteroid3);
        hx_drop_mesh(stars);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_quit();
        return 1;
    }
    
    // Output buffer
    const size_t stride = static_cast<size_t>(width) * 4;
    uint8_t* pixels = static_cast<uint8_t*>(malloc(stride * static_cast<size_t>(height)));
    if (!pixels) {
        fprintf(stderr, "Out of memory\n");
        hx_drop_skin(skin);
        hx_drop_mesh(ship);
        hx_drop_mesh(asteroid1);
        hx_drop_mesh(asteroid2);
        hx_drop_mesh(asteroid3);
        hx_drop_mesh(stars);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_quit();
        return 1;
    }
    
    // Game state
    float ship_yaw = 0.0f;
    float ship_pitch = 0.0f;
    float ship_roll = 0.0f;
    HxVec3 ship_pos = {0, 0, 0};
    HxVec3 ship_vel = {0, 0, 0};
    
    // Asteroid positions and rotations
    struct Asteroid { HxVec3 pos; HxVec3 rot; HxVec3 rot_speed; HxMesh mesh; };
    Asteroid asteroids[3] = {
        {{ 15,  3, 20}, {0,0,0}, {0.01f, 0.02f, 0.015f}, asteroid1},
        {{-10, -2, 30}, {0,0,0}, {0.02f, 0.01f, 0.025f}, asteroid2},
        {{  5,  8, 40}, {0,0,0}, {0.015f, 0.025f, 0.01f}, asteroid3},
    };
    
    // Render animation sequence (30 frames = ~1 second at 30fps)
    const int total_frames = 30;
    const int save_frame = 15; // Save middle frame
    
    printf("Rendering %d frames...\n", total_frames);
    
    for (int frame = 0; frame < total_frames; ++frame) {
        float dt = 1.0f / 30.0f;
        float t = static_cast<float>(frame) * dt;
        
        // Ship movement: gentle banking turn
        ship_yaw += 0.5f * dt;      // Turn left
        ship_pitch = 0.3f * sinf(t * 2.0f); // Gentle up/down
        ship_roll = -0.4f * sinf(t * 2.0f); // Bank into turn
        
        // Forward velocity
        ship_vel.z = -5.0f; // Move forward (negative Z)
        ship_pos.z += ship_vel.z * dt;
        
        // Camera follows ship (chase cam)
        HxVec3 cam_offset = {0, 3, -10};
        // Rotate offset by ship yaw
        float cy = cosf(ship_yaw), sy = sinf(ship_yaw);
        eye = {
            ship_pos.x + cam_offset.x * cy - cam_offset.z * sy,
            ship_pos.y + cam_offset.y,
            ship_pos.z + cam_offset.x * sy + cam_offset.z * cy
        };
        target = {ship_pos.x, ship_pos.y, ship_pos.z};
        hx_look(cam, &target, &eye, &up);
        
        // Clear and rebuild world each frame
        hx_clear_world(world);
        
        // Add starfield (static, far away)
        HxMat4 star_transform;
        hx_make_mat4_identity(&star_transform);
        hx_add_mesh(world, stars, skin, &star_transform);
        
        // Add ship
        HxQuat ship_rot_yaw, ship_rot_pitch, ship_rot_roll, ship_rot;
        HxVec3 yaw_axis = {0, 1, 0};
        HxVec3 pitch_axis = {1, 0, 0};
        HxVec3 roll_axis = {0, 0, 1};
        HxVec3 scale_one = {1,1,1};
        hx_make_quat_axis_angle(&yaw_axis, ship_yaw, &ship_rot_yaw);
        hx_make_quat_axis_angle(&pitch_axis, ship_pitch, &ship_rot_pitch);
        hx_make_quat_axis_angle(&roll_axis, ship_roll, &ship_rot_roll);
        // Combine: roll * pitch * yaw
        hx_mul_quat(&ship_rot_pitch, &ship_rot_yaw, &ship_rot);
        hx_mul_quat(&ship_rot_roll, &ship_rot, &ship_rot);
        
        HxMat4 ship_transform;
        hx_make_mat4_trs(&ship_pos, &ship_rot, &scale_one, &ship_transform);
        hx_add_mesh(world, ship, skin, &ship_transform);
        
        // Add asteroids (rotating, orbiting slightly)
        for (int i = 0; i < 3; ++i) {
            // Update asteroid rotation
            asteroids[i].rot.x += asteroids[i].rot_speed.x;
            asteroids[i].rot.y += asteroids[i].rot_speed.y;
            asteroids[i].rot.z += asteroids[i].rot_speed.z;
            
            // Slight orbital motion
            float orbit_angle = t * 0.3f + static_cast<float>(i) * 2.1f;
            HxVec3 asteroid_pos = asteroids[i].pos;
            asteroid_pos.x += 2.0f * sinf(orbit_angle);
            asteroid_pos.z += 1.0f * cosf(orbit_angle * 0.7f);
            asteroid_pos.y += 0.5f * sinf(orbit_angle * 1.3f);
            
            // Move asteroids toward camera (ship moves forward, so relative motion)
            asteroid_pos.z += ship_vel.z * dt * 0.5f;
            
            HxQuat ast_rot_x, ast_rot_y, ast_rot_z, ast_rot;
            HxVec3 axis_x = {1,0,0};
            HxVec3 axis_y = {0,1,0};
            HxVec3 axis_z = {0,0,1};
            hx_make_quat_axis_angle(&axis_x, asteroids[i].rot.x, &ast_rot_x);
            hx_make_quat_axis_angle(&axis_y, asteroids[i].rot.y, &ast_rot_y);
            hx_make_quat_axis_angle(&axis_z, asteroids[i].rot.z, &ast_rot_z);
            hx_mul_quat(&ast_rot_y, &ast_rot_x, &ast_rot);
            hx_mul_quat(&ast_rot_z, &ast_rot, &ast_rot);
            
            HxMat4 ast_transform;
            hx_make_mat4_trs(&asteroid_pos, &ast_rot, &scale_one, &ast_transform);
            hx_add_mesh(world, asteroids[i].mesh, skin, &ast_transform);
        }
        
        // Render frame
        HxResult rr = hx_render_headless(width, height, world, cam, pixels, stride);
        if (rr != HX_OK) {
            fprintf(stderr, "Frame %d: hx_render_headless failed: %s\n", frame, hx_last_error());
            free(pixels);
            hx_drop_skin(skin);
            hx_drop_mesh(ship);
            hx_drop_mesh(asteroid1);
            hx_drop_mesh(asteroid2);
            hx_drop_mesh(asteroid3);
            hx_drop_mesh(stars);
            hx_drop_cam(cam);
            hx_drop_world(world);
            hx_quit();
            return 1;
        }
        
        // Save middle frame as reference
        if (frame == save_frame) {
            if (!hx_png_write_rgba8("test_3d_game.png", width, height, pixels, stride)) {
                fprintf(stderr, "Failed to write test_3d_game.png\n");
                free(pixels);
                hx_drop_skin(skin);
                hx_drop_mesh(ship);
                hx_drop_mesh(asteroid1);
                hx_drop_mesh(asteroid2);
                hx_drop_mesh(asteroid3);
                hx_drop_mesh(stars);
                hx_drop_cam(cam);
                hx_drop_world(world);
                hx_quit();
                return 1;
            }
            printf("  Saved frame %d as test_3d_game.png (%dx%d)\n", frame, width, height);
        }
        
        if (frame % 5 == 0) {
            printf("  Frame %d/%d\r", frame + 1, total_frames);
            fflush(stdout);
        }
    }
    printf("\n");
    
    // Verify saved frame has expected content (not all black/white)
    uint64_t hash = image_hash(pixels, stride * height);
    printf("Final frame hash: 0x%016llX\n", static_cast<unsigned long long>(hash));
    
    // Basic sanity checks
    int non_black = 0, non_white = 0;
    for (size_t i = 0; i < static_cast<size_t>(width) * static_cast<size_t>(height) * 4; i += 4) {
        if (pixels[i] > 10 || pixels[i+1] > 10 || pixels[i+2] > 10) non_black++;
        if (pixels[i] < 245 || pixels[i+1] < 245 || pixels[i+2] < 245) non_white++;
    }
    printf("Non-black pixels: %d / %d\n", non_black, width * height);
    printf("Non-white pixels: %d / %d\n", non_white, width * height);
    
    if (non_black < 100 || non_white < 100) {
        fprintf(stderr, "FAIL: Image appears empty or solid color\n");
        free(pixels);
        hx_drop_skin(skin);
        hx_drop_mesh(ship);
        hx_drop_mesh(asteroid1);
        hx_drop_mesh(asteroid2);
        hx_drop_mesh(asteroid3);
        hx_drop_mesh(stars);
        hx_drop_cam(cam);
        hx_drop_world(world);
        hx_quit();
        return 1;
    }
    
    // Cleanup
    free(pixels);
    hx_drop_skin(skin);
    hx_drop_mesh(ship);
    hx_drop_mesh(asteroid1);
    hx_drop_mesh(asteroid2);
    hx_drop_mesh(asteroid3);
    hx_drop_mesh(stars);
    hx_drop_cam(cam);
    hx_drop_world(world);
    hx_quit();
    
    printf("\n✓ 3D Space Flight Game Test PASSED\n");
    return 0;
}