// win32_input.cpp — Win32 input implementation (stub for Phase 1)
#include "../platform_internal.h"
#include <windows.h>

// -----------------------------------------------------------------------------
// Key Mapping (Win32 VK -> HxKey)
// -----------------------------------------------------------------------------
static HxKey hx_win32_map_key(WPARAM vk) {
    switch (vk) {
        case VK_ESCAPE: return HX_KEY_ESCAPE;
        case VK_RETURN: return HX_KEY_ENTER;
        case VK_TAB: return HX_KEY_TAB;
        case VK_BACK: return HX_KEY_BACKSPACE;
        case VK_INSERT: return HX_KEY_INSERT;
        case VK_DELETE: return HX_KEY_DELETE;
        case VK_RIGHT: return HX_KEY_RIGHT;
        case VK_LEFT: return HX_KEY_LEFT;
        case VK_DOWN: return HX_KEY_DOWN;
        case VK_UP: return HX_KEY_UP;
        case VK_PRIOR: return HX_KEY_PAGE_UP;
        case VK_NEXT: return HX_KEY_PAGE_DOWN;
        case VK_HOME: return HX_KEY_HOME;
        case VK_END: return HX_KEY_END;
        case VK_CAPITAL: return HX_KEY_CAPS_LOCK;
        case VK_SCROLL: return HX_KEY_SCROLL_LOCK;
        case VK_NUMLOCK: return HX_KEY_NUM_LOCK;
        case VK_SNAPSHOT: return HX_KEY_PRINT_SCREEN;
        case VK_PAUSE: return HX_KEY_PAUSE;
        case VK_F1: return HX_KEY_F1;
        case VK_F2: return HX_KEY_F2;
        case VK_F3: return HX_KEY_F3;
        case VK_F4: return HX_KEY_F4;
        case VK_F5: return HX_KEY_F5;
        case VK_F6: return HX_KEY_F6;
        case VK_F7: return HX_KEY_F7;
        case VK_F8: return HX_KEY_F8;
        case VK_F9: return HX_KEY_F9;
        case VK_F10: return HX_KEY_F10;
        case VK_F11: return HX_KEY_F11;
        case VK_F12: return HX_KEY_F12;
        case VK_LSHIFT: return HX_KEY_LEFT_SHIFT;
        case VK_LCONTROL: return HX_KEY_LEFT_CONTROL;
        case VK_LMENU: return HX_KEY_LEFT_ALT;
        case VK_LWIN: return HX_KEY_LEFT_SUPER;
        case VK_RSHIFT: return HX_KEY_RIGHT_SHIFT;
        case VK_RCONTROL: return HX_KEY_RIGHT_CONTROL;
        case VK_RMENU: return HX_KEY_RIGHT_ALT;
        case VK_RWIN: return HX_KEY_RIGHT_SUPER;
        case VK_APPS: return HX_KEY_MENU;
        case VK_NUMPAD0: return HX_KEY_KP_0;
        case VK_NUMPAD1: return HX_KEY_KP_1;
        case VK_NUMPAD2: return HX_KEY_KP_2;
        case VK_NUMPAD3: return HX_KEY_KP_3;
        case VK_NUMPAD4: return HX_KEY_KP_4;
        case VK_NUMPAD5: return HX_KEY_KP_5;
        case VK_NUMPAD6: return HX_KEY_KP_6;
        case VK_NUMPAD7: return HX_KEY_KP_7;
        case VK_NUMPAD8: return HX_KEY_KP_8;
        case VK_NUMPAD9: return HX_KEY_KP_9;
        case VK_DECIMAL: return HX_KEY_KP_DECIMAL;
        case VK_DIVIDE: return HX_KEY_KP_DIVIDE;
        case VK_MULTIPLY: return HX_KEY_KP_MULTIPLY;
        case VK_SUBTRACT: return HX_KEY_KP_SUBTRACT;
        case VK_ADD: return HX_KEY_KP_ADD;
        default:
            if (vk >= 'A' && vk <= 'Z') return (HxKey)vk;
            if (vk >= '0' && vk <= '9') return (HxKey)vk;
            return HX_KEY_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
// Input Processing (called from WndProc)
// -----------------------------------------------------------------------------
void hx_win32_process_key(HxWin win, WPARAM vk, LPARAM lparam, bool down) {
    HxKey key = hx_win32_map_key(vk);
    if (key == HX_KEY_UNKNOWN) return;

    HxKeyState state = down ? HX_KEY_STATE_DOWN : HX_KEY_STATE_UP;
    // Check for repeat
    if (down && (lparam & (1 << 30))) state = HX_KEY_STATE_REPEAT;

    hx_input_key(key, state);
    hx_win_fire_key(win, key, state);
}

void hx_win32_process_mouse_btn(HxWin win, HxMouseBtn btn, bool down) {
    hx_input_mouse_btn(btn, down);
    hx_win_fire_mouse_btn(win, btn, down);
}

void hx_win32_process_mouse_move(HxWin win, int x, int y) {
    hx_input_mouse_move(static_cast<double>(x), static_cast<double>(y));
    hx_win_fire_mouse_move(win, static_cast<double>(x), static_cast<double>(y));
}

void hx_win32_process_mouse_wheel(HxWin win, short delta) {
    double d = static_cast<double>(delta) / 120.0;
    hx_input_mouse_wheel(0.0, d);
    hx_win_fire_mouse_wheel(win, d);
}

// -----------------------------------------------------------------------------
// Gamepad (XInput - stub)
// -----------------------------------------------------------------------------
void hx_win32_update_gamepads(void) {
    // XInputGetState for each controller
    // Stub for Phase 1
}