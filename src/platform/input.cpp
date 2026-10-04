// input.cpp — Input abstraction (common)
// SPDX-License-Identifier: MIT
#include "helix.h"
#include "platform_internal.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Keyboard State
// -----------------------------------------------------------------------------
#define HX_MAX_KEYS 512
static uint8_t g_key_states[HX_MAX_KEYS] = {0};

HxKeyState hx_key_state(HxWin win, HxKey key) {
    (void)win; // Phase 1: global state
    if (key >= HX_MAX_KEYS) return HX_KEY_STATE_UP;
    return (HxKeyState)g_key_states[key];
}

void hx_input_key(HxKey key, HxKeyState state) {
    if (key < HX_MAX_KEYS) g_key_states[key] = (uint8_t)state;
}

// -----------------------------------------------------------------------------
// Mouse State
// -----------------------------------------------------------------------------
static bool g_mouse_btns[8] = {0};
static double g_mouse_x = 0, g_mouse_y = 0;
static double g_mouse_dx = 0, g_mouse_dy = 0;
static double g_mouse_wheel = 0;

bool hx_mouse_btn(HxWin win, HxMouseBtn btn) {
    (void)win;
    if (btn >= 8) return false;
    return g_mouse_btns[btn];
}

void hx_mouse_pos(HxWin win, double* x, double* y) {
    (void)win;
    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
}

void hx_mouse_delta(HxWin win, double* dx, double* dy) {
    (void)win;
    if (dx) *dx = g_mouse_dx;
    if (dy) *dy = g_mouse_dy;
}

double hx_mouse_wheel(HxWin win) {
    (void)win;
    double w = g_mouse_wheel;
    g_mouse_wheel = 0; // Consume
    return w;
}

void hx_input_mouse_btn(HxMouseBtn btn, bool down) {
    if (btn < 8) g_mouse_btns[btn] = down;
}

void hx_input_mouse_move(double x, double y) {
    g_mouse_dx = x - g_mouse_x;
    g_mouse_dy = y - g_mouse_y;
    g_mouse_x = x;
    g_mouse_y = y;
}

void hx_input_mouse_wheel(double dx, double dy) {
    g_mouse_wheel += dy;
    (void)dx;
}

// -----------------------------------------------------------------------------
// Gamepad State
// -----------------------------------------------------------------------------
typedef struct {
    bool connected;
    uint32_t buttons;
    float lx, ly;  // Left stick
    float rx, ry;  // Right stick
    float lt, rt;  // Triggers
} HxPadState;

static HxPadState g_pads[HX_MAX_GAMEPADS] = {0};

bool hx_pad_connected(HxWin win, int index) {
    (void)win;
    if (index < 0 || index >= HX_MAX_GAMEPADS) return false;
    return g_pads[index].connected;
}

uint32_t hx_pad_buttons(HxWin win, int index) {
    (void)win;
    if (index < 0 || index >= HX_MAX_GAMEPADS) return 0;
    return g_pads[index].buttons;
}

void hx_pad_stick(HxWin win, int index, int stick, float* x, float* y) {
    (void)win;
    if (index < 0 || index >= HX_MAX_GAMEPADS) return;
    if (stick == 0) { if (x) *x = g_pads[index].lx; if (y) *y = g_pads[index].ly; }
    else { if (x) *x = g_pads[index].rx; if (y) *y = g_pads[index].ry; }
}

float hx_pad_trigger(HxWin win, int index, int trigger) {
    (void)win;
    if (index < 0 || index >= HX_MAX_GAMEPADS) return 0;
    return trigger == 0 ? g_pads[index].lt : g_pads[index].rt;
}

void hx_input_pad_connect(int index, bool connected) {
    if (index >= 0 && index < HX_MAX_GAMEPADS) g_pads[index].connected = connected;
}

void hx_input_pad_btn(int index, HxPadBtn btn, bool down) {
    if (index < 0 || index >= HX_MAX_GAMEPADS) return;
    if (down) g_pads[index].buttons |= btn;
    else g_pads[index].buttons &= ~btn;
}

void hx_input_pad_stick(int index, int stick, float x, float y) {
    if (index < 0 || index >= HX_MAX_GAMEPADS) return;
    if (stick == 0) { g_pads[index].lx = x; g_pads[index].ly = y; }
    else { g_pads[index].rx = x; g_pads[index].ry = y; }
}

void hx_input_pad_trigger(int index, int trigger, float value) {
    if (index < 0 || index >= HX_MAX_GAMEPADS) return;
    if (trigger == 0) g_pads[index].lt = value;
    else g_pads[index].rt = value;
}

void hx_input_reset(void) {
    memset(g_key_states, 0, sizeof(g_key_states));
    memset(g_mouse_btns, 0, sizeof(g_mouse_btns));
    memset(g_pads, 0, sizeof(g_pads));
    g_mouse_x = g_mouse_y = 0;
    g_mouse_dx = g_mouse_dy = 0;
    g_mouse_wheel = 0;
}

HX_API HxKeyState HX_CALL hx_get_key_state(HxWin win, HxKey key) { return hx_key_state(win, key); }
HX_API bool HX_CALL hx_get_mouse_btn(HxWin win, HxMouseBtn btn) { return hx_mouse_btn(win, btn); }
HX_API void HX_CALL hx_get_mouse_pos(HxWin win, double* x, double* y) { hx_mouse_pos(win, x, y); }
HX_API void HX_CALL hx_get_mouse_delta(HxWin win, double* dx, double* dy) { hx_mouse_delta(win, dx, dy); }
HX_API double HX_CALL hx_get_mouse_wheel(HxWin win) { return hx_mouse_wheel(win); }
HX_API bool HX_CALL hx_get_pad_connected(HxWin win, int index) { return hx_pad_connected(win, index); }
HX_API uint32_t HX_CALL hx_get_pad_buttons(HxWin win, int index) { return hx_pad_buttons(win, index); }
HX_API void HX_CALL hx_get_pad_stick(HxWin win, int index, int stick, float* x, float* y) { hx_pad_stick(win, index, stick, x, y); }
HX_API float HX_CALL hx_get_pad_trigger(HxWin win, int index, int trigger) { return hx_pad_trigger(win, index, trigger); }