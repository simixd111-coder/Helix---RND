// wasm_window.cpp — WebAssembly/Canvas window implementation (stub for Phase 1)
#include "helix.h"
#include <emscripten.h>
#include <emscripten/html5.h>

// -----------------------------------------------------------------------------
// WASM Window Data
// -----------------------------------------------------------------------------
typedef struct {
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context;
    int canvas_width, canvas_height;
    double last_time;
} HxWasmData;

// -----------------------------------------------------------------------------
// Platform Functions
// -----------------------------------------------------------------------------
bool hx_win_platform_create(HxWin win, HxWinFlags flags) {
    HxWasmData* data = (HxWasmData*)calloc(1, sizeof(HxWasmData));
    if (!data) return false;

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = EM_TRUE;
    attrs.depth = EM_TRUE;
    attrs.stencil = EM_FALSE;
    attrs.antialias = EM_TRUE;
    attrs.premultipliedAlpha = EM_TRUE;
    attrs.preserveDrawingBuffer = EM_FALSE;
    attrs.preferLowPowerToHighPerformance = EM_FALSE;
    attrs.failIfMajorPerformanceCaveat = EM_FALSE;
    attrs.enableExtensionsByDefault = EM_TRUE;
    attrs.explicitSwapControl = EM_FALSE;

    data->gl_context = emscripten_webgl_create_context("#canvas", &attrs);
    if (!data->gl_context) {
        free(data);
        return false;
    }

    emscripten_webgl_make_context_current(data->gl_context);

    // Set up resize callback
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, win, true, [](int eventType, const EmscriptenUiEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        hx_win_fire_resize(win, e->windowInnerWidth, e->windowInnerHeight);
        return EM_TRUE;
    });

    // Set up input callbacks
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, win, true, [](int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        // Map DOM key to HxKey (stub)
        hx_win_fire_key(win, (HxKey)0, HX_KEY_DOWN);
        return EM_TRUE;
    });

    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, win, true, [](int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        hx_win_fire_key(win, (HxKey)0, HX_KEY_UP);
        return EM_TRUE;
    });

    emscripten_set_mousedown_callback("#canvas", win, true, [](int eventType, const EmscriptenMouseEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        HxMouseBtn btn = (e->button == 0) ? HX_MOUSE_LEFT : (e->button == 1) ? HX_MOUSE_MIDDLE : HX_MOUSE_RIGHT;
        hx_win_fire_mouse_btn(win, btn, true);
        return EM_TRUE;
    });

    emscripten_set_mouseup_callback("#canvas", win, true, [](int eventType, const EmscriptenMouseEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        HxMouseBtn btn = (e->button == 0) ? HX_MOUSE_LEFT : (e->button == 1) ? HX_MOUSE_MIDDLE : HX_MOUSE_RIGHT;
        hx_win_fire_mouse_btn(win, btn, false);
        return EM_TRUE;
    });

    emscripten_set_mousemove_callback("#canvas", win, true, [](int eventType, const EmscriptenMouseEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        hx_win_fire_mouse_move(win, e->clientX, e->clientY);
        return EM_TRUE;
    });

    emscripten_set_wheel_callback("#canvas", win, true, [](int eventType, const EmscriptenWheelEvent* e, void* userData) {
        HxWin win = (HxWin)userData;
        hx_win_fire_mouse_wheel(win, e->deltaY);
        return EM_TRUE;
    });

    data->last_time = emscripten_get_now() / 1000.0;
    win->platform_data = data;
    return true;
}

void hx_win_platform_destroy(HxWin win) {
    HxWasmData* data = (HxWasmData*)win->platform_data;
    if (!data) return;
    if (data->gl_context) emscripten_webgl_destroy_context(data->gl_context);
    free(data);
    win->platform_data = NULL;
}

void hx_win_platform_poll(HxWin win) {
    HxWasmData* data = (HxWasmData*)win->platform_data;
    if (!data) return;
    double now = emscripten_get_now() / 1000.0;
    win->dt = now - data->last_time;
    data->last_time = now;
    // Events are callback-driven
}

void hx_win_platform_swap(HxWin win) {
    // emscripten_webgl_commit_frame(); // If using explicit swap control
    (void)win;
}

void hx_win_platform_set_title(HxWin win, const char* title) {
    (void)win; (void)title;
    // document.title = title;
}

void hx_win_platform_set_size(HxWin win, int width, int height) {
    (void)win; (void)width; (void)height;
}

void hx_win_platform_set_vsync(HxWin win, bool enabled) {
    (void)win; (void)enabled;
}

void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen) {
    (void)win; (void)fullscreen;
}