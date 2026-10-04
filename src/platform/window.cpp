// window.cpp — Window abstraction (common, internal)
// SPDX-License-Identifier: MIT
#include "helix.h"
#include "platform_internal.h"
#include "../core/resource_internal.h"
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Window Creation / Destruction
// (struct HxWinImpl is defined in platform_internal.h)
// -----------------------------------------------------------------------------
static void hx_win_destroy_resource(void* resource) {
    hx_win_destroy(static_cast<HxWin>(resource));
}

HxWin hx_win_create(int width, int height, const char* title, HxWinFlags flags) {
    if (width <= 0 || height <= 0) return NULL;
    HxWin win = static_cast<HxWin>(calloc(1, sizeof(HxWinImpl)));
    if (!win) return NULL;
    win->cpu_bytes = sizeof(HxWinImpl);

    win->width = width;
    win->height = height;
    win->fb_width = width;
    win->fb_height = height;
    win->dpi_scale = 1.0f;
    win->alive = true;
    win->focused = true;
    win->vsync = (flags & HX_WIN_VSYNC) != 0;
    win->fullscreen = (flags & HX_WIN_FULLSCREEN) != 0;
    win->headless = (flags & HX_WIN_HEADLESS) != 0;
    win->last_time = hx_platform_time();

    if (title) {
        strncpy(win->title, title, sizeof(win->title) - 1);
    }

    // Platform-specific creation
    const bool created = win->headless
        ? hx_headless_window_create(win)
        : hx_win_platform_create(win, flags);
    if (!created) {
        free(win);
        return NULL;
    }

    if (!hx_resource_register(win, win->cpu_bytes, 0, hx_win_destroy_resource)) {
        hx_win_destroy(win);
        return NULL;
    }

    return win;
}

void hx_win_destroy(HxWin win) {
    if (!win) return;
    if (win->headless) hx_headless_window_destroy(win);
    else hx_win_platform_destroy(win);
    free(win);
}

// -----------------------------------------------------------------------------
// Window State Queries
// -----------------------------------------------------------------------------
bool hx_win_alive(HxWin win) { return win && win->alive; }
bool hx_win_focused(HxWin win) { return win && win->focused; }
bool hx_win_minimized(HxWin win) { return win && win->minimized; }

void hx_win_size(HxWin win, int* w, int* h) {
    if (win) {
        if (w) *w = win->width;
        if (h) *h = win->height;
    }
}

float hx_win_dpi_scale(HxWin win) { return win ? win->dpi_scale : 1.0f; }

double hx_win_dt(HxWin win) { return win ? win->dt : 0.0; }
double hx_win_time(HxWin win) { return win ? hx_platform_time() : 0.0; }

// -----------------------------------------------------------------------------
// Window Manipulation
// -----------------------------------------------------------------------------
void hx_win_set_title(HxWin win, const char* title) {
    if (!win || !title) return;
    strncpy(win->title, title, sizeof(win->title) - 1);
    if (!win->headless) hx_win_platform_set_title(win, title);
}

void hx_win_set_size(HxWin win, int width, int height) {
    if (!win || width <= 0 || height <= 0) return;
    win->width = width;
    win->height = height;
    if (win->headless) hx_headless_window_set_size(win, width, height);
    else hx_win_platform_set_size(win, width, height);
}

void hx_win_set_vsync(HxWin win, bool enabled) {
    if (!win) return;
    win->vsync = enabled;
    if (!win->headless) hx_win_platform_set_vsync(win, enabled);
}

void hx_win_set_fullscreen(HxWin win, bool fullscreen) {
    if (!win) return;
    win->fullscreen = fullscreen;
    if (!win->headless) hx_win_platform_set_fullscreen(win, fullscreen);
}

// -----------------------------------------------------------------------------
// Main Loop
// -----------------------------------------------------------------------------
bool hx_win_tick(HxWin win) {
    if (!win || !win->alive) return false;

    double now = hx_platform_time();
    win->dt = now - win->last_time;
    win->last_time = now;

    // Platform-specific event polling
    if (!win->headless) hx_win_platform_poll(win);

    return win->alive;
}

void hx_win_show(HxWin win) {
    if (!win) return;
    if (!win->headless) hx_win_platform_swap(win);
}

// -----------------------------------------------------------------------------
// Unified Event System
// -----------------------------------------------------------------------------
void hx_win_on(HxWin win, HxEventType mask, HxEventCallback cb, void* user) {
    if (!win) return;
    win->event_cb = cb;
    win->event_mask = mask;
    win->event_user = user;
}

// -----------------------------------------------------------------------------
// Internal: Fire events (called from platform code)
// -----------------------------------------------------------------------------
static void hx_win_fire(HxWin win, HxEvent* ev) {
    if (!win || !ev) return;
    if (!win->event_cb) return;
    if (!(win->event_mask & ev->type)) return;
    win->event_cb(win, ev, win->event_user);
}

void hx_win_fire_key(HxWin win, HxKey key, HxKeyState state) {
    if (!win) return;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = (state == HX_KEY_STATE_DOWN) ? HX_EV_KEY_DOWN :
              (state == HX_KEY_STATE_REPEAT) ? HX_EV_KEY_REPEAT : HX_EV_KEY_UP;
    ev.key.key = key;
    ev.key.state = state;
    hx_win_fire(win, &ev);
}

void hx_win_fire_mouse_btn(HxWin win, HxMouseBtn btn, bool down) {
    if (!win) return;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = down ? HX_EV_MOUSE_BTN_DOWN : HX_EV_MOUSE_BTN_UP;
    ev.mouse_btn.btn = btn;
    ev.mouse_btn.down = down;
    hx_win_fire(win, &ev);
}

void hx_win_fire_mouse_move(HxWin win, double x, double y) {
    if (!win) return;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = HX_EV_MOUSE_MOVE;
    ev.mouse_move.x = x;
    ev.mouse_move.y = y;
    hx_win_fire(win, &ev);
}

void hx_win_fire_mouse_wheel(HxWin win, double delta) {
    if (!win) return;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = HX_EV_MOUSE_WHEEL;
    ev.mouse_wheel.delta = delta;
    hx_win_fire(win, &ev);
}

void hx_win_fire_resize(HxWin win, int w, int h) {
    if (!win) return;
    win->width = w;
    win->height = h;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = HX_EV_RESIZE;
    ev.resize.w = w;
    ev.resize.h = h;
    hx_win_fire(win, &ev);
}

void hx_win_fire_close(HxWin win) {
    if (!win) return;
    win->alive = false;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = HX_EV_CLOSE;
    hx_win_fire(win, &ev);
}

void hx_win_fire_focus(HxWin win, bool gained) {
    if (!win) return;
    win->focused = gained;
    HxEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = gained ? HX_EV_FOCUS_GAINED : HX_EV_FOCUS_LOST;
    hx_win_fire(win, &ev);
}

HX_API HxWin HX_CALL hx_make_win(int width, int height, const char* title, HxWinFlags flags) {
    return hx_win_create(width, height, title, flags);
}

HX_API HxWin HX_CALL hx_make_win_foreign(void* handle, int width, int height) {
    (void)handle;
    (void)width;
    (void)height;
    return NULL;
}

HX_API HxResult HX_CALL hx_drop_win(HxWin win) {
    if (!win) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(win)) return HX_ERR_ALREADY_DROPPED;
    hx_win_destroy(win);
    return HX_OK;
}

HX_API bool HX_CALL hx_tick(HxWin win) { return hx_win_tick(win); }
HX_API void HX_CALL hx_show(HxWin win) { hx_win_show(win); }
HX_API bool HX_CALL hx_get_win_alive(HxWin win) { return hx_win_alive(win); }
HX_API bool HX_CALL hx_get_win_focused(HxWin win) { return hx_win_focused(win); }
HX_API bool HX_CALL hx_get_win_minimized(HxWin win) { return hx_win_minimized(win); }
HX_API void HX_CALL hx_get_win_size(HxWin win, int* w, int* h) { hx_win_size(win, w, h); }
HX_API float HX_CALL hx_get_win_dpi_scale(HxWin win) { return hx_win_dpi_scale(win); }
HX_API double HX_CALL hx_get_win_dt(HxWin win) { return hx_win_dt(win); }
HX_API double HX_CALL hx_get_win_time(HxWin win) { return hx_win_time(win); }
HX_API void HX_CALL hx_set_win_title(HxWin win, const char* title) { hx_win_set_title(win, title); }
HX_API void HX_CALL hx_set_win_size(HxWin win, int width, int height) { hx_win_set_size(win, width, height); }
HX_API void HX_CALL hx_set_win_vsync(HxWin win, bool enabled) { hx_win_set_vsync(win, enabled); }
HX_API void HX_CALL hx_set_win_fullscreen(HxWin win, bool enabled) { hx_win_set_fullscreen(win, enabled); }
HX_API void HX_CALL hx_on(HxWin win, HxEventType mask, HxEventCallback cb, void* user) { hx_win_on(win, mask, cb, user); }