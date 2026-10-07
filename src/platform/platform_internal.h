// platform_internal.h — Internal platform declarations (NOT public API)
// SPDX-License-Identifier: MIT
#ifndef HELIX_PLATFORM_INTERNAL_H
#define HELIX_PLATFORM_INTERNAL_H

#include "helix.h"

// Platform init/quit (platform.cpp)
HxResult hx_platform_init(void);
void hx_platform_quit(void);
double hx_platform_time(void);
void hx_platform_sleep(double seconds);

// Dynamic library loading (platform.cpp)
void* hx_dlopen(const char* name);
void* hx_dlsym(void* handle, const char* symbol);
void hx_dlclose(void* handle);
const char* hx_dlerror(void);

// Logging (platform.cpp)
void hx_platform_log(HxLogLevel level, const char* fmt, ...);
void hx_platform_set_log_cb(HxLogCallback cb, void* user);

// Window (window.cpp) — internal API, public API wraps these
// Internal window structure (defined here so platform backends can access it)
struct HxWinImpl {
    uint64_t handle_id;          // Opaque handle ID
    int width, height;           // Window size
    int fb_width, fb_height;     // Actual framebuffer size (HiDPI)
    float dpi_scale;             // DPI scale factor
    bool alive;                  // Not closed
    bool focused;                // Has focus
    bool minimized;              // Minimized
    bool vsync;                  // VSync enabled
    bool fullscreen;             // Fullscreen
    bool headless;               // Headless mode
    uint32_t max_fps;            // 0 = uncapped
    double last_time;            // Last frame time
    double dt;                   // Delta time
    char title[256];             // Window title
    void* platform_data;         // Platform-specific data
    size_t cpu_bytes;            // Known CPU allocation size for diagnostics

    // Unified event callback
    HxEventCallback event_cb;
    HxEventType event_mask;
    void* event_user;
};

HxWin hx_win_create(int width, int height, const char* title, HxWinFlags flags);
void hx_win_destroy(HxWin win);
bool hx_win_alive(HxWin win);
bool hx_win_focused(HxWin win);
bool hx_win_minimized(HxWin win);
void hx_win_size(HxWin win, int* w, int* h);
float hx_win_dpi_scale(HxWin win);
double hx_win_dt(HxWin win);
double hx_win_time(HxWin win);
void hx_win_set_title(HxWin win, const char* title);
void hx_win_set_size(HxWin win, int width, int height);
void hx_win_set_vsync(HxWin win, bool enabled);
void hx_win_set_fps_limit(HxWin win, uint32_t max_fps);
uint32_t hx_win_fps_limit(HxWin win);
void hx_win_set_fullscreen(HxWin win, bool fullscreen);
bool hx_win_tick(HxWin win);
void hx_win_show(HxWin win);
void hx_win_on(HxWin win, uint32_t mask, HxEventCallback cb, void* user);

// Internal event firing (called from platform backends)
void hx_win_fire_key(HxWin win, HxKey key, HxKeyState state);
void hx_win_fire_mouse_btn(HxWin win, HxMouseBtn btn, bool down);
void hx_win_fire_mouse_move(HxWin win, double x, double y);
void hx_win_fire_mouse_wheel(HxWin win, double delta);
void hx_win_fire_resize(HxWin win, int w, int h);
void hx_win_fire_close(HxWin win);
void hx_win_fire_focus(HxWin win, bool gained);

// Platform-specific window backend (implemented per-platform)
bool hx_win_platform_create(HxWin win, HxWinFlags flags);
void hx_win_platform_destroy(HxWin win);
void hx_win_platform_poll(HxWin win);
void hx_win_platform_swap(HxWin win);
void hx_win_platform_set_title(HxWin win, const char* title);
void hx_win_platform_set_size(HxWin win, int width, int height);
void hx_win_platform_set_vsync(HxWin win, bool enabled);
void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen);

// Headless window backend (always compiled, separate from native platform symbols)
bool hx_headless_window_create(HxWin win);
void hx_headless_window_destroy(HxWin win);
void hx_headless_window_set_size(HxWin win, int width, int height);
HxResult hx_headless_get_pixels(HxWin win, void** out_pixels, size_t* out_stride, int* out_w, int* out_h);

// Input state (input.cpp)
void hx_input_reset(void);
void hx_input_key(HxKey key, HxKeyState state);
void hx_input_mouse_btn(HxMouseBtn btn, bool down);
void hx_input_mouse_move(double x, double y);
void hx_input_mouse_wheel(double dx, double dy);
void hx_input_pad_connect(int idx, bool connected);
void hx_input_pad_btn(int idx, HxPadBtn btn, bool down);
void hx_input_pad_stick(int idx, int stick, float x, float y);
void hx_input_pad_trigger(int idx, int trigger, float value);

#endif // HELIX_PLATFORM_INTERNAL_H
