// test_platform_stub.cpp — Platform stub tests (Phase 1)
#include "helix.h"
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    printf("Testing platform stubs...\n");

    // Test boot with software backend
    HxResult res = hx_boot(NULL);
    assert(res == HX_OK);
    assert(hx_get_backend() == HX_BACKEND_SOFTWARE);
    assert(strcmp(hx_get_backend_name(HX_BACKEND_SOFTWARE), "Software") == 0);

    // Test headless window creation
    HxWin win = hx_make_win(320, 240, "Test", HX_WIN_HEADLESS);
    assert(win != NULL);
    assert(hx_get_win_alive(win));
    assert(hx_get_win_focused(win));
    assert(!hx_get_win_minimized(win));

    int w, h;
    hx_get_win_size(win, &w, &h);
    assert(w == 320);
    assert(h == 240);

    assert(hx_get_win_dpi_scale(win) == 1.0f);
    assert(hx_get_win_dt(win) >= 0.0);

    // Test tick
    bool alive = hx_tick(win);
    assert(alive);
    assert(hx_get_win_dt(win) >= 0.0);

    // Test show (no-op for headless)
    hx_show(win);

    hx_on(win, UINT32_MAX, [](HxWin, const HxEvent*, void*) { return false; }, NULL);

    // Test input (defaults)
    assert(hx_get_key_state(win, HX_KEY_SPACE) == HX_KEY_STATE_UP);
    assert(!hx_get_mouse_btn(win, HX_MOUSE_LEFT));

    double mx, my;
    hx_get_mouse_pos(win, &mx, &my);
    assert(mx == 0.0 && my == 0.0);

    double mdx, mdy;
    hx_get_mouse_delta(win, &mdx, &mdy);
    assert(mdx == 0.0 && mdy == 0.0);

    assert(hx_get_mouse_wheel(win) == 0.0);

    // Test gamepad (not connected)
    assert(!hx_get_pad_connected(win, 0));
    assert(hx_get_pad_buttons(win, 0) == 0);

    float sx, sy;
    hx_get_pad_stick(win, 0, 0, &sx, &sy);
    assert(sx == 0.0f && sy == 0.0f);

    assert(hx_get_pad_trigger(win, 0, 0) == 0.0f);
    assert(hx_get_pad_trigger(win, 0, 1) == 0.0f);

    // Cleanup
    hx_drop_win(win);
    hx_quit();

    printf("All platform stub tests PASSED\n");
    return 0;
}