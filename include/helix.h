/*
 * helix.h — Helix RND Public C API (Stable ABI)
 * Version: 3.0.0
 * License: MIT
 * SPDX-License-Identifier: MIT
 *
 * Design principles:
 * - C11 compatible (works from C, C++, and all FFI)
 * - Opaque handles (forward-declared structs)
 * - Explicit error codes via HxResult + hx_last_error()
 * - No heap allocation in hot paths; caller owns buffers
 * - Thread-safe: boot/quit are process-global; handles are thread-confined
 * - Prefix: hx_ for functions, Hx for types, HX_ for constants
 * - Naming: hx_<verb>_<object> with verbs: make, load, drop, add, draw, move, spin, size, look, set, get, snap, say,
 * tween, on
 */

#ifndef HELIX_H
#define HELIX_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * Version & Platform Detection
     * ============================================================================ */

#define HX_VERSION_MAJOR 3
#define HX_VERSION_MINOR 0
#define HX_VERSION_PATCH 0
#define HX_VERSION_STRING "3.0.0"

/* Platform */
#if defined(_WIN32) || defined(__CYGWIN__)
#    define HX_PLATFORM_WINDOWS 1
#    define HX_PLATFORM_NAME "windows"
#elif defined(__APPLE__) && defined(__MACH__)
#    define HX_PLATFORM_MACOS 1
#    define HX_PLATFORM_NAME "macos"
#elif defined(__linux__)
#    define HX_PLATFORM_LINUX 1
#    define HX_PLATFORM_NAME "linux"
#elif defined(__EMSCRIPTEN__) || defined(__wasm__)
#    define HX_PLATFORM_WASM 1
#    define HX_PLATFORM_NAME "wasm"
#else
#    define HX_PLATFORM_UNKNOWN 1
#    define HX_PLATFORM_NAME "unknown"
#endif

/* Compiler attributes */
#if defined(_MSC_VER)
#    define HX_API __declspec(dllexport)
#    define HX_NORETURN __declspec(noreturn)
#    define HX_UNUSED
#    define HX_FALLTHROUGH __fallthrough
#elif defined(__GNUC__) || defined(__clang__)
#    define HX_API __attribute__((visibility("default")))
#    define HX_NORETURN __attribute__((noreturn))
#    define HX_UNUSED __attribute__((unused))
#    define HX_FALLTHROUGH __attribute__((fallthrough))
#else
#    define HX_API
#    define HX_NORETURN
#    define HX_UNUSED
#    define HX_FALLTHROUGH
#endif

/* Calling convention */
#if defined(_WIN32) && !defined(__clang__)
#    define HX_CALL __stdcall
#else
#    define HX_CALL
#endif

    /* ============================================================================
     * Fundamental Types
     * ============================================================================ */

#include <stddef.h>  /* size_t, ptrdiff_t */
#include <stdint.h>  /* int32_t, uint64_t, etc. */
#include <stdbool.h> /* bool */

    /* Opaque pointer handles */
    typedef struct HxWinImpl* HxWin;       /* Window */
    typedef struct HxWorldImpl* HxWorld;   /* Scene graph root */
    typedef struct HxMeshImpl* HxMesh;     /* Geometry (vertices + indices) */
    typedef struct HxSkinImpl* HxSkin;     /* Material / appearance */
    typedef struct HxCamImpl* HxCam;       /* Camera (2D or 3D) */
    typedef struct HxLampImpl* HxLamp;     /* Light source */
    typedef struct HxTexImpl* HxTex;       /* Texture / image */
    typedef struct HxPicImpl* HxPic;       /* Image / picture (loaded from file) */
    typedef struct HxShaderImpl* HxShader; /* Shader program */
    typedef struct HxBufferImpl* HxBuffer; /* GPU buffer (uniform, vertex, etc.) */
    typedef struct HxAtlasImpl* HxAtlas;   /* Texture atlas / sprite sheet */
    typedef struct HxFontImpl* HxFont;     /* TTF font */
    typedef struct HxLayerImpl* HxLayer;   /* Render layer (2D/3D sorting) */
    typedef struct HxTweenImpl* HxTween;   /* Animation tween */
    typedef struct HxFxImpl* HxFx;         /* Post-process effect */
    typedef struct HxRigImpl* HxRig;       /* Skeletal rig (animation) */
    typedef struct HxClipImpl* HxClip;     /* Animation clip */
    typedef struct HxAudioImpl* HxAudio;   /* Audio (future) */
    typedef struct HxModelImpl* HxModel;   /* Imported static model */

    typedef uint64_t HxEntity;
    typedef uint32_t HxComponentType;
#define HX_NULL_ENTITY ((HxEntity) 0)
#define HX_COMPONENT_INVALID 0u
    typedef struct HxSpriteSheetImpl* HxSpriteSheet;
    typedef struct HxSpriteAnimImpl* HxSpriteAnim;

/* Handle invalid sentinel */
#define HX_NULL_HANDLE ((HxWin) 0)

    /* ============================================================================
     * Result Codes
     * ============================================================================ */

    typedef int32_t HxResult;

/* Success */
#define HX_OK 0

/* Generic failures */
#define HX_ERR -1
#define HX_ERR_OOM -2             /* Out of memory */
#define HX_ERR_INVALID_ARG -3     /* Bad parameter */
#define HX_ERR_INVALID_HANDLE -4  /* Handle is null or freed */
#define HX_ERR_INVALID_STATE -5   /* Operation not allowed in current state */
#define HX_ERR_NOT_IMPLEMENTED -6 /* Feature not implemented yet */
#define HX_ERR_UNSUPPORTED -7     /* Not supported on this platform/backend */
#define HX_ERR_ALREADY_DROPPED -8 /* Double drop attempted (safe, returns this) */

/* Init / shutdown */
#define HX_ERR_ALREADY_BOOTED -10
#define HX_ERR_NOT_BOOTED -11
#define HX_ERR_BACKEND_UNAVAILABLE -12 /* No GPU backend could be initialized */

/* Window */
#define HX_ERR_WIN_CREATE -20
#define HX_ERR_WIN_CONTEXT -21
#define HX_ERR_WIN_SURFACE -22

/* Render */
#define HX_ERR_SHADER_COMPILE -30
#define HX_ERR_PIPELINE_CREATE -31
#define HX_ERR_TEXTURE_CREATE -32
#define HX_ERR_BUFFER_CREATE -33
#define HX_ERR_MESH_CREATE -34
#define HX_ERR_DRAW_INVALID -35

/* Assets */
#define HX_ERR_ASSET_NOT_FOUND -40
#define HX_ERR_ASSET_PARSE -41
#define HX_ERR_ASSET_UNSUPPORTED -42

/* Input */
#define HX_ERR_INPUT_UNAVAILABLE -50

    /* ============================================================================
     * Error Reporting
     * ============================================================================ */

    /* Returns a human-readable string for the last error on the calling thread.
     * Thread-local. Returns empty string if no error since last successful call.
     * The returned pointer is valid until the next API call on this thread. */
    HX_API const char* HX_CALL hx_last_error(void);

    /* ============================================================================
     * Boot Configuration
     * ============================================================================ */

    /* GPU backend requested by user */
    typedef uint8_t HxGpu;
#define HX_GPU_AUTO 0  /* Auto-select: Vulkan → OpenGL/Metal → Software */
#define HX_GPU_VK 1    /* Force Vulkan */
#define HX_GPU_GL 2    /* Force OpenGL */
#define HX_GPU_METAL 3 /* Force Metal (macOS/iOS) */
#define HX_GPU_SOFT 4  /* Force Software rasterizer */

    /* Backend actually in use (query via hx_get_backend) */
    typedef uint8_t HxBackend;
#define HX_BACKEND_UNKNOWN 0
#define HX_BACKEND_VULKAN 1
#define HX_BACKEND_OPENGL 2
#define HX_BACKEND_METAL 3
#define HX_BACKEND_SOFTWARE 4

    typedef uint8_t HxGpuDeviceType;
#define HX_GPU_DEVICE_OTHER 0
#define HX_GPU_DEVICE_INTEGRATED 1
#define HX_GPU_DEVICE_DISCRETE 2
#define HX_GPU_DEVICE_VIRTUAL 3
#define HX_GPU_DEVICE_CPU 4
#define HX_GPU_DEVICE_DEFAULT 0u

    typedef uint8_t HxGpuPreference;
#define HX_GPU_PREFERENCE_AUTO 0
#define HX_GPU_PREFERENCE_HIGH_PERFORMANCE 1
#define HX_GPU_PREFERENCE_LOW_POWER 2

    typedef struct
    {
        char name[256];
        uint32_t vendor_id;
        uint32_t device_id;
        HxGpuDeviceType type;
        uint32_t api_version;
        uint64_t local_memory_bytes;
        bool graphics_queue;
        bool compute_queue;
    } HxGpuDeviceInfo;

    /* Boot configuration structure */
    typedef struct
    {
        HxGpu gpu;                      /* GPU backend preference */
        bool headless;                  /* Run without window system */
        const char* app_name;           /* Application name for logging */
        void* reserved;                 /* Reserved for future use */
        HxGpuPreference gpu_preference; /* Auto, low-power or high-performance adapter */
        uint32_t gpu_device_index;      /* 0 = automatic; N = enumerated adapter N-1 */
    } HxCfg;

    /* Approximate accounting for resources owned and tracked by Helix. */
    typedef struct
    {
        size_t live_resources;
        size_t cpu_bytes;
        size_t gpu_bytes;
    } HxMemoryStats;

    /* Initialize the engine with configuration. Must be called before any other function.
     * Returns HX_OK on success. */
    HX_API HxResult HX_CALL hx_boot(const HxCfg* cfg);

    /* Query tracked resource count and memory. GPU bytes are zero until a GPU backend owns resources. */
    HX_API void HX_CALL hx_get_memory_stats(HxMemoryStats* out_stats);

    /* Shutdown the engine. Destroys all remaining handles. Idempotent. */
    HX_API void HX_CALL hx_quit(void);

    /* Query version at runtime. */
    HX_API void HX_CALL hx_version(int* major, int* minor, int* patch);

    /* Get active backend. */
    HX_API HxBackend HX_CALL hx_get_backend(void);
    HX_API const char* HX_CALL hx_get_backend_name(HxBackend b);

    /* GPU info strings (valid after boot) */
    HX_API const char* HX_CALL hx_get_gpu_vendor(void);
    HX_API const char* HX_CALL hx_get_gpu_renderer(void);
    HX_API const char* HX_CALL hx_get_gpu_version(void);
    HX_API HxResult HX_CALL hx_get_gpu_devices(HxGpuDeviceInfo* devices, size_t capacity, size_t* out_count);
    HX_API HxResult HX_CALL hx_pick_gpu_device(const HxGpuDeviceInfo* devices,
                                               size_t count,
                                               HxGpuPreference preference,
                                               size_t* out_index);

    /* ============================================================================
     * Window
     * ============================================================================ */

    /* Window creation flags (bitmask) */
    typedef uint32_t HxWinFlags;
#define HX_WIN_NONE 0u
#define HX_WIN_FULLSCREEN (1u << 0)
#define HX_WIN_BORDERLESS (1u << 1)
#define HX_WIN_RESIZABLE (1u << 2)
#define HX_WIN_VSYNC (1u << 3)
#define HX_WIN_HIDPI (1u << 4)    /* Enable high-DPI backing store */
#define HX_WIN_HIDDEN (1u << 5)   /* Create hidden, show later */
#define HX_WIN_HEADLESS (1u << 6) /* No visible window, offscreen render */
#define HX_WIN_FOREIGN (1u << 7)  /* Use foreign handle (see hx_make_win_foreign) */

    /* Create a window. Title must be UTF-8. Returns HX_NULL_HANDLE on failure. */
    HX_API HxWin HX_CALL hx_make_win(int width, int height, const char* title, HxWinFlags flags);

    /* Create window from foreign handle (HWND on Windows, NSView* on macOS, wl_surface* on Wayland, etc.).
     * The 'handle' interpretation depends on platform. */
    HX_API HxWin HX_CALL hx_make_win_foreign(void* handle, int width, int height);

    /* Destroy window. Invalidates handle. Safe to call twice (returns HX_ERR_ALREADY_DROPPED on second call). */
    HX_API HxResult HX_CALL hx_drop_win(HxWin win);

    /* Poll events (keyboard, mouse, gamepad, window resize, close request).
     * Must be called each frame. Returns false if window close requested. */
    HX_API bool HX_CALL hx_tick(HxWin win);

    /* Swap buffers / present frame. */
    HX_API void HX_CALL hx_show(HxWin win);

    /* Window state queries */
    HX_API bool HX_CALL hx_get_win_alive(HxWin win);                /* Not closed */
    HX_API bool HX_CALL hx_get_win_focused(HxWin win);              /* Has keyboard focus */
    HX_API bool HX_CALL hx_get_win_minimized(HxWin win);            /* Minimized / occluded */
    HX_API void HX_CALL hx_get_win_size(HxWin win, int* w, int* h); /* Framebuffer size in pixels */
    HX_API float HX_CALL hx_get_win_dpi_scale(HxWin win);           /* HiDPI scale factor (1.0, 1.5, 2.0...) */
    HX_API double HX_CALL hx_get_win_dt(HxWin win);                 /* Delta time since last tick (seconds) */
    HX_API double HX_CALL hx_get_win_time(HxWin win);               /* Total time since boot (seconds) */
    HX_API uint32_t HX_CALL hx_get_win_fps_limit(HxWin win);        /* 0 = uncapped */

    /* Window manipulation */
    HX_API void HX_CALL hx_set_win_title(HxWin win, const char* title);
    HX_API void HX_CALL hx_set_win_size(HxWin win, int width, int height);
    HX_API void HX_CALL hx_set_win_vsync(HxWin win, bool enabled);
    /* Limits the rate of hx_tick; 0 disables the limit. Native VSync takes precedence. */
    HX_API void HX_CALL hx_set_win_fps_limit(HxWin win, uint32_t max_fps);
    HX_API void HX_CALL hx_set_win_fullscreen(HxWin win, bool fullscreen);

    /* Headless windows can be captured to PNG; visible-window capture is unsupported. */
    HX_API HxResult HX_CALL hx_snap_win(HxWin win, const char* path);

    /* ============================================================================
     * Input — Keyboard
     * ============================================================================ */

    typedef uint32_t HxKey;
/* Helix key codes (aligned numerically with GLFW for convenience, but Helix does NOT depend on GLFW) */
#define HX_KEY_UNKNOWN 0
#define HX_KEY_SPACE 32
#define HX_KEY_APOSTROPHE 39
#define HX_KEY_COMMA 44
#define HX_KEY_MINUS 45
#define HX_KEY_PERIOD 46
#define HX_KEY_SLASH 47
#define HX_KEY_0 48
#define HX_KEY_1 49
#define HX_KEY_2 50
#define HX_KEY_3 51
#define HX_KEY_4 52
#define HX_KEY_5 53
#define HX_KEY_6 54
#define HX_KEY_7 55
#define HX_KEY_8 56
#define HX_KEY_9 57
#define HX_KEY_SEMICOLON 59
#define HX_KEY_EQUAL 61
#define HX_KEY_A 65
#define HX_KEY_B 66
#define HX_KEY_C 67
#define HX_KEY_D 68
#define HX_KEY_E 69
#define HX_KEY_F 70
#define HX_KEY_G 71
#define HX_KEY_H 72
#define HX_KEY_I 73
#define HX_KEY_J 74
#define HX_KEY_K 75
#define HX_KEY_L 76
#define HX_KEY_M 77
#define HX_KEY_N 78
#define HX_KEY_O 79
#define HX_KEY_P 80
#define HX_KEY_Q 81
#define HX_KEY_R 82
#define HX_KEY_S 83
#define HX_KEY_T 84
#define HX_KEY_U 85
#define HX_KEY_V 86
#define HX_KEY_W 87
#define HX_KEY_X 88
#define HX_KEY_Y 89
#define HX_KEY_Z 90
#define HX_KEY_LEFT_BRACKET 91
#define HX_KEY_BACKSLASH 92
#define HX_KEY_RIGHT_BRACKET 93
#define HX_KEY_GRAVE_ACCENT 96
#define HX_KEY_ESCAPE 256
#define HX_KEY_ENTER 257
#define HX_KEY_TAB 258
#define HX_KEY_BACKSPACE 259
#define HX_KEY_INSERT 260
#define HX_KEY_DELETE 261
#define HX_KEY_RIGHT 262
#define HX_KEY_LEFT 263
#define HX_KEY_DOWN 264
#define HX_KEY_UP 265
#define HX_KEY_PAGE_UP 266
#define HX_KEY_PAGE_DOWN 267
#define HX_KEY_HOME 268
#define HX_KEY_END 269
#define HX_KEY_CAPS_LOCK 280
#define HX_KEY_SCROLL_LOCK 281
#define HX_KEY_NUM_LOCK 282
#define HX_KEY_PRINT_SCREEN 283
#define HX_KEY_PAUSE 284
#define HX_KEY_F1 290
#define HX_KEY_F2 291
#define HX_KEY_F3 292
#define HX_KEY_F4 293
#define HX_KEY_F5 294
#define HX_KEY_F6 295
#define HX_KEY_F7 296
#define HX_KEY_F8 297
#define HX_KEY_F9 298
#define HX_KEY_F10 299
#define HX_KEY_F11 300
#define HX_KEY_F12 301
#define HX_KEY_F13 302
#define HX_KEY_F14 303
#define HX_KEY_F15 304
#define HX_KEY_F16 305
#define HX_KEY_F17 306
#define HX_KEY_F18 307
#define HX_KEY_F19 308
#define HX_KEY_F20 309
#define HX_KEY_F21 310
#define HX_KEY_F22 311
#define HX_KEY_F23 312
#define HX_KEY_F24 313
#define HX_KEY_KP_0 320
#define HX_KEY_KP_1 321
#define HX_KEY_KP_2 322
#define HX_KEY_KP_3 323
#define HX_KEY_KP_4 324
#define HX_KEY_KP_5 325
#define HX_KEY_KP_6 326
#define HX_KEY_KP_7 327
#define HX_KEY_KP_8 328
#define HX_KEY_KP_9 329
#define HX_KEY_KP_DECIMAL 330
#define HX_KEY_KP_DIVIDE 331
#define HX_KEY_KP_MULTIPLY 332
#define HX_KEY_KP_SUBTRACT 333
#define HX_KEY_KP_ADD 334
#define HX_KEY_KP_ENTER 335
#define HX_KEY_KP_EQUAL 336
#define HX_KEY_LEFT_SHIFT 340
#define HX_KEY_LEFT_CONTROL 341
#define HX_KEY_LEFT_ALT 342
#define HX_KEY_LEFT_SUPER 343
#define HX_KEY_RIGHT_SHIFT 344
#define HX_KEY_RIGHT_CONTROL 345
#define HX_KEY_RIGHT_ALT 346
#define HX_KEY_RIGHT_SUPER 347
#define HX_KEY_MENU 348

    /* Key state */
    typedef uint8_t HxKeyState;
#define HX_KEY_STATE_UP 0
#define HX_KEY_STATE_DOWN 1
#define HX_KEY_STATE_REPEAT 2

    HX_API HxKeyState HX_CALL hx_get_key_state(HxWin win, HxKey key);

    /* ============================================================================
     * Input — Mouse
     * ============================================================================ */

    typedef uint8_t HxMouseBtn;
#define HX_MOUSE_LEFT 0
#define HX_MOUSE_RIGHT 1
#define HX_MOUSE_MIDDLE 2
#define HX_MOUSE_4 3
#define HX_MOUSE_5 4
#define HX_MOUSE_6 5
#define HX_MOUSE_7 6
#define HX_MOUSE_8 7

    HX_API bool HX_CALL hx_get_mouse_btn(HxWin win, HxMouseBtn btn);
    HX_API void HX_CALL hx_get_mouse_pos(HxWin win, double* x, double* y);
    HX_API void HX_CALL hx_get_mouse_delta(HxWin win, double* dx, double* dy);
    HX_API double HX_CALL hx_get_mouse_wheel(HxWin win); /* Accumulated since last tick */

    /* ============================================================================
     * Input — Gamepad (up to 4)
     * ============================================================================ */

#define HX_MAX_GAMEPADS 4

    typedef uint32_t HxPadBtn;
#define HX_PAD_A (1u << 0)
#define HX_PAD_B (1u << 1)
#define HX_PAD_X (1u << 2)
#define HX_PAD_Y (1u << 3)
#define HX_PAD_BUMP_L (1u << 4)    /* Left bumper (LB) */
#define HX_PAD_BUMP_R (1u << 5)    /* Right bumper (RB) */
#define HX_PAD_TRIGGER_L (1u << 6) /* Left trigger (LT) */
#define HX_PAD_TRIGGER_R (1u << 7) /* Right trigger (RT) */
#define HX_PAD_BACK (1u << 8)
#define HX_PAD_START (1u << 9)
#define HX_PAD_STICK_L (1u << 10) /* Left stick press */
#define HX_PAD_STICK_R (1u << 11) /* Right stick press */
#define HX_PAD_UP (1u << 12)
#define HX_PAD_DOWN (1u << 13)
#define HX_PAD_LEFT (1u << 14)
#define HX_PAD_RIGHT (1u << 15)

    HX_API bool HX_CALL hx_get_pad_connected(HxWin win, int index); /* 0-3 */
    HX_API uint32_t HX_CALL hx_get_pad_buttons(HxWin win, int index);
    HX_API void HX_CALL
    hx_get_pad_stick(HxWin win, int index, int stick, float* x, float* y);      /* stick: 0=left, 1=right */
    HX_API float HX_CALL hx_get_pad_trigger(HxWin win, int index, int trigger); /* 0=LT, 1=RT */

    /* ============================================================================
     * Events — Unified Callback System
     * ============================================================================ */

    typedef uint32_t HxEventType;
#define HX_EV_KEY_DOWN (1u << 0)
#define HX_EV_KEY_UP (1u << 1)
#define HX_EV_KEY_REPEAT (1u << 2)
#define HX_EV_MOUSE_BTN_DOWN (1u << 3)
#define HX_EV_MOUSE_BTN_UP (1u << 4)
#define HX_EV_MOUSE_MOVE (1u << 5)
#define HX_EV_MOUSE_WHEEL (1u << 6)
#define HX_EV_RESIZE (1u << 7)
#define HX_EV_CLOSE (1u << 8)
#define HX_EV_FOCUS_GAINED (1u << 9)
#define HX_EV_FOCUS_LOST (1u << 10)
#define HX_EV_PAD_CONNECTED (1u << 11)
#define HX_EV_PAD_DISCONNECTED (1u << 12)
#define HX_EV_PAD_BTN_DOWN (1u << 13)
#define HX_EV_PAD_BTN_UP (1u << 14)
#define HX_EV_PAD_STICK (1u << 15)
#define HX_EV_PAD_TRIGGER (1u << 16)

    typedef struct
    {
        HxEventType type;
        union
        {
            struct
            {
                HxKey key;
                HxKeyState state;
            } key;
            struct
            {
                HxMouseBtn btn;
                bool down;
            } mouse_btn;
            struct
            {
                double x, y;
            } mouse_move;
            struct
            {
                double delta;
            } mouse_wheel;
            struct
            {
                int w, h;
            } resize;
            struct
            {
                int index;
            } pad;
            struct
            {
                int index;
                HxPadBtn btn;
                bool down;
            } pad_btn;
            struct
            {
                int index;
                int stick;
                float x, y;
            } pad_stick;
            struct
            {
                int index;
                int trigger;
                float value;
            } pad_trigger;
        };
    } HxEvent;

    /* Single callback for all events. Returns true if event was handled. */
    typedef bool(HX_CALL* HxEventCallback)(HxWin win, const HxEvent* ev, void* user);

    /* Register event callback. Replaces all individual set_*_cb functions. */
    HX_API void HX_CALL hx_on(HxWin win, HxEventType mask, HxEventCallback cb, void* user);

    /* ============================================================================
     * Math Types (POD, passed by value)
     * ============================================================================ */

    typedef struct
    {
        float x, y;
    } HxVec2;
    typedef struct
    {
        float x, y, z;
    } HxVec3;
    typedef struct
    {
        float x, y, z, w;
    } HxVec4;
    typedef struct
    {
        float m[4][4];
    } HxMat4; /* Column-major */
    typedef struct
    {
        float x, y, z, w;
    } HxQuat; /* xyz = vector, w = scalar */

    /* Color (linear RGBA, 0..1) */
    typedef struct
    {
        float r, g, b, a;
    } HxColor;

/* Common colors */
#ifdef __cplusplus
#    define HX_COLOR(r, g, b, a)                                                                                       \
        HxColor                                                                                                        \
        {                                                                                                              \
            (r), (g), (b), (a)                                                                                         \
        }
#else
#    define HX_COLOR(r, g, b, a)                                                                                       \
        (HxColor)                                                                                                      \
        {                                                                                                              \
            (r), (g), (b), (a)                                                                                         \
        }
#endif
#define HX_WHITE HX_COLOR(1, 1, 1, 1)
#define HX_BLACK HX_COLOR(0, 0, 0, 1)
#define HX_RED HX_COLOR(1, 0, 0, 1)
#define HX_GREEN HX_COLOR(0, 1, 0, 1)
#define HX_BLUE HX_COLOR(0, 0, 1, 1)
#define HX_YELLOW HX_COLOR(1, 1, 0, 1)
#define HX_CYAN HX_COLOR(0, 1, 1, 1)
#define HX_MAGENTA HX_COLOR(1, 0, 1, 1)
#define HX_ORANGE HX_COLOR(1, 0.5f, 0, 1)
#define HX_GRAY HX_COLOR(0.5f, 0.5f, 0.5f, 1)

    /* ============================================================================
     * World / Scene
     * ============================================================================ */

    HX_API HxWorld HX_CALL hx_make_world(void);
    HX_API HxResult HX_CALL hx_drop_world(HxWorld world);

    /* Add a mesh instance; mesh and skin handles must stay alive until cleared or the world is dropped. */
    HX_API void HX_CALL hx_add_mesh(HxWorld world, HxMesh mesh, HxSkin skin, const HxMat4* transform);

    /* Clear all mesh instances from world (does NOT destroy resources) */
    HX_API void HX_CALL hx_clear_world(HxWorld world);

    /* Generic ECS. Component data is copied on add; returned pointers are borrowed
     * and remain valid only until that component storage changes or the world drops. */
    typedef void(HX_CALL* HxComponentEachCallback)(HxWorld world, HxEntity entity, void* component, void* user);
    HX_API HxEntity HX_CALL hx_make_entity(HxWorld world);
    HX_API HxResult HX_CALL hx_drop_entity(HxWorld world, HxEntity entity);
    HX_API bool HX_CALL hx_get_entity_alive(HxWorld world, HxEntity entity);
    HX_API HxResult HX_CALL hx_register_component_type(
        HxWorld world, const char* name, size_t size, size_t alignment, HxComponentType* out_type);
    HX_API HxResult HX_CALL hx_add_component(HxWorld world,
                                             HxEntity entity,
                                             HxComponentType type,
                                             const void* initial_data);
    HX_API void* HX_CALL hx_get_component(HxWorld world, HxEntity entity, HxComponentType type);
    HX_API HxResult HX_CALL hx_remove_component(HxWorld world, HxEntity entity, HxComponentType type);
    HX_API HxResult HX_CALL hx_for_each_component(HxWorld world,
                                                  HxComponentType type,
                                                  HxComponentEachCallback callback,
                                                  void* user);

    /* ============================================================================
     * Mesh (Geometry)
     * ============================================================================ */

    /* Vertex format flags */
    typedef uint32_t HxVertexFlags;
#define HX_VERT_POS (1u << 0)     /* float3 position (required) */
#define HX_VERT_NORMAL (1u << 1)  /* float3 normal */
#define HX_VERT_TANGENT (1u << 2) /* float4 tangent */
#define HX_VERT_UV0 (1u << 3)     /* float2 texcoord 0 */
#define HX_VERT_UV1 (1u << 4)     /* float2 texcoord 1 */
#define HX_VERT_COLOR (1u << 5)   /* float4 color */
#define HX_VERT_JOINTS (1u << 6)  /* uint4 joint indices */
#define HX_VERT_WEIGHTS (1u << 7) /* float4 joint weights */

    /* Primitive topology */
    typedef uint8_t HxPrim;
#define HX_PRIM_TRIANGLES 0
#define HX_PRIM_LINES 1
#define HX_PRIM_POINTS 2
#define HX_PRIM_TRI_STRIP 3

    /* Create mesh from raw vertex/index data.
     * vertices: interleaved array matching 'format' flags.
     * vertex_stride: bytes per vertex (0 = auto from format).
     * indices: optional, 16 or 32 bit (see index32). */
    HX_API HxMesh HX_CALL hx_make_mesh(const void* vertices,
                                       size_t vertex_count,
                                       HxVertexFlags format,
                                       size_t vertex_stride,
                                       const void* indices,
                                       size_t index_count,
                                       bool index32,
                                       HxPrim prim);

    /* Primitive helpers (generate common shapes) */
    HX_API HxMesh HX_CALL hx_make_cube(HxSkin skin);
    HX_API HxMesh HX_CALL hx_make_sphere(HxSkin skin, int rings, int sectors);
    HX_API HxMesh HX_CALL hx_make_plane(HxSkin skin, float w, float h);
    HX_API HxMesh HX_CALL hx_make_quad(HxSkin skin); /* Unit quad at Z=0, for 2D sprites */

    HX_API HxResult HX_CALL hx_drop_mesh(HxMesh mesh);

    /* Mesh transform operations (apply to mesh's local transform) */
    HX_API void HX_CALL hx_move_mesh(HxMesh mesh, float x, float y, float z);
    HX_API void HX_CALL hx_spin_mesh(HxMesh mesh, float x, float y, float z); /* Rotate in radians */
    HX_API void HX_CALL hx_size_mesh(HxMesh mesh, float x, float y, float z); /* Scale */

    /* CPU-side glTF/GLB import. Returned primitive pointers stay valid until hx_drop_model. */
    typedef struct
    {
        const char* mesh_name;
        const float* positions; /* vertex_count * 3 floats */
        const float* normals;   /* Optional, vertex_count * 3 floats */
        const float* uv0;       /* Optional, vertex_count * 2 floats */
        size_t vertex_count;
        const uint32_t* indices; /* Optional, always 32-bit when present */
        size_t index_count;
        HxPrim primitive;
    } HxModelPrimitiveView;

    HX_API HxModel HX_CALL hx_load_model(const char* path); /* Blender glTF 2.0 / GLB static meshes */
    HX_API size_t HX_CALL hx_get_model_mesh_count(HxModel model);
    HX_API size_t HX_CALL hx_get_model_primitive_count(HxModel model, size_t mesh_index);
    HX_API HxResult HX_CALL hx_get_model_primitive(HxModel model,
                                                   size_t mesh_index,
                                                   size_t primitive_index,
                                                   HxModelPrimitiveView* out_view);
    HX_API HxResult HX_CALL hx_drop_model(HxModel model);

    /* ============================================================================
     * Skin (Material / Appearance)
     * ============================================================================ */

    typedef uint32_t HxSkinFlags;
#define HX_SKIN_NONE 0u
#define HX_SKIN_UNLIT (1u << 0) /* Base-color rendering */
#define HX_SKIN_WIREFRAME (1u << 1)
#define HX_SKIN_DOUBLE_SIDED (1u << 2)
#define HX_SKIN_TRANSPARENT (1u << 3) /* Alpha blend */
#define HX_SKIN_MASKED (1u << 4)      /* Alpha test (cutout) */
#define HX_SKIN_EMISSIVE (1u << 5)    /* Emissive color */

    /* Software skin creation supports NONE, UNLIT and DOUBLE_SIDED flags; other flags return NULL. */
    HX_API HxSkin HX_CALL hx_make_skin(HxColor color, HxSkinFlags flags);
    HX_API HxSkin HX_CALL hx_make_skin_tex(HxTex tex, HxColor tint, HxSkinFlags flags);
    /* PBR material creation is unsupported and returns NULL. */
    HX_API HxSkin HX_CALL hx_make_skin_pbr(HxTex albedo,
                                           HxTex normal,
                                           HxTex metal_rough,
                                           HxTex ao,
                                           HxTex emissive,
                                           float metallic,
                                           float roughness,
                                           HxColor emissive_color,
                                           HxSkinFlags flags);

    HX_API HxResult HX_CALL hx_drop_skin(HxSkin skin);

    /* ============================================================================
     * Camera
     * ============================================================================ */

    typedef uint8_t HxCamType;
#define HX_CAM_3D 0 /* Perspective */
#define HX_CAM_2D 1 /* Orthographic */

    HX_API HxCam HX_CALL hx_make_cam3d(void);
    HX_API HxCam HX_CALL hx_make_cam2d(void);

    /* 3D camera */
    HX_API void HX_CALL hx_look(HxCam cam, const HxVec3* at, const HxVec3* from, const HxVec3* up);
    HX_API void HX_CALL hx_set_cam_persp(HxCam cam, float fov_y_deg, float aspect, float near_z, float far_z);

    /* 2D camera */
    HX_API void HX_CALL
    hx_set_cam_ortho(HxCam cam, float left, float right, float bottom, float top, float near_z, float far_z);
    HX_API void HX_CALL hx_move_cam2d(HxCam cam, float x, float y);
    HX_API void HX_CALL hx_size_cam2d(HxCam cam, float zoom); /* zoom = scale */

    /* Common */
    HX_API void HX_CALL hx_get_cam_view(HxCam cam, HxMat4* out_view);
    HX_API void HX_CALL hx_get_cam_proj(HxCam cam, HxMat4* out_proj);
    HX_API void HX_CALL hx_get_cam_view_proj(HxCam cam, HxMat4* out_view_proj);

    HX_API HxResult HX_CALL hx_drop_cam(HxCam cam);

    /* ============================================================================
     * Light (Lamp)
     * ============================================================================ */

    typedef uint8_t HxLampType;
#define HX_LAMP_SUN 0   /* Directional — infinite distance, parallel rays */
#define HX_LAMP_POINT 1 /* Point — position + range + attenuation */
#define HX_LAMP_SPOT 2  /* Spot — position + direction + angle + range */

    /* Lighting/lamp APIs are unsupported; constructor returns NULL. */
    HX_API HxLamp HX_CALL hx_make_lamp(HxLampType type, HxColor color, float intensity);

    /* Sun: direction is -transform.forward */
    HX_API void HX_CALL hx_set_lamp_dir(HxLamp lamp, const HxVec3* dir);

    /* Point / Spot: position from transform */
    HX_API void HX_CALL hx_set_lamp_range(HxLamp lamp, float range);
    HX_API void HX_CALL hx_set_lamp_spot(HxLamp lamp, float inner_deg, float outer_deg);

    HX_API HxResult HX_CALL hx_drop_lamp(HxLamp lamp);

    /* ============================================================================
     * Texture
     * ============================================================================ */

    typedef uint32_t HxTexFlags;
#define HX_TEX_NONE 0u
#define HX_TEX_SRGB (1u << 0)    /* Data is sRGB, convert to linear on read */
#define HX_TEX_MIPMAPS (1u << 1) /* Generate mipmaps */
#define HX_TEX_REPEAT (1u << 2)  /* Wrap repeat (default clamp) */
#define HX_TEX_MIRROR (1u << 3)  /* Wrap mirror */
#define HX_TEX_LINEAR (1u << 4)  /* Linear filter (default nearest) */

    /* Create from raw pixel data. The software renderer supports 8-bit R/RG/RGB/RGBA/BGRA; alpha is currently opaque.
     */
    typedef uint8_t HxTexFmt;
#define HX_TEX_FMT_R8 0
#define HX_TEX_FMT_RG8 1
#define HX_TEX_FMT_RGB8 2
#define HX_TEX_FMT_RGBA8 3
#define HX_TEX_FMT_BGRA8 4
#define HX_TEX_FMT_R16F 5
#define HX_TEX_FMT_RG16F 6
#define HX_TEX_FMT_RGB16F 7
#define HX_TEX_FMT_RGBA16F 8
#define HX_TEX_FMT_R32F 9
#define HX_TEX_FMT_RG32F 10
#define HX_TEX_FMT_RGB32F 11
#define HX_TEX_FMT_RGBA32F 12
#define HX_TEX_FMT_D24S8 13 /* Depth-stencil */
#define HX_TEX_FMT_D32F 14

    HX_API HxTex HX_CALL hx_make_tex(int w, int h, HxTexFmt fmt, const void* pixels, HxTexFlags flags);
    HX_API HxTex HX_CALL hx_make_tex_cube(int size, HxTexFmt fmt, const void* faces[6], HxTexFlags flags);

    /* Load common image formats supported by the bundled stb_image decoder; stored as RGBA8. */
    HX_API HxTex HX_CALL hx_load_tex(const char* path, HxTexFlags flags);

    HX_API HxResult HX_CALL hx_drop_tex(HxTex tex);

    /* ============================================================================
     * Picture (CPU-side image, for loading/saving)
     * ============================================================================ */

    HX_API HxPic HX_CALL
    hx_load_pic(const char* path); /* Decode to top-left-origin RGBA8 CPU pixels; NULL on failure */
    HX_API HxResult HX_CALL hx_save_pic(HxPic pic, const char* path); /* Save RGBA8 pixels to PNG. */
    HX_API void HX_CALL hx_get_pic_size(HxPic pic, int* w, int* h);
    HX_API void HX_CALL hx_get_pic_pixels(HxPic pic, void** out_pixels, size_t* out_stride); /* RGBA8 */
    HX_API HxResult HX_CALL hx_drop_pic(HxPic pic);

    /* ============================================================================
     * Shader
     * ============================================================================ */

    /* Shader loading is unsupported and returns NULL. */
    HX_API HxShader HX_CALL hx_load_shader(const char* name); /* e.g. "pbr", "sprite", "skybox" */
    HX_API HxResult HX_CALL hx_drop_shader(HxShader shader);

    /* ============================================================================
     * Buffer (GPU)
     * ============================================================================ */

    typedef uint32_t HxBufferFlags;
#define HX_BUF_NONE 0u
#define HX_BUF_VERTEX (1u << 0)
#define HX_BUF_INDEX (1u << 1)
#define HX_BUF_UNIFORM (1u << 2)
#define HX_BUF_STORAGE (1u << 3)
#define HX_BUF_DYNAMIC (1u << 4)   /* CPU writes frequently */
#define HX_BUF_MAP_WRITE (1u << 5) /* Persistent map for write */
#define HX_BUF_MAP_READ (1u << 6)  /* Persistent map for read */

    HX_API HxBuffer HX_CALL hx_make_buffer(size_t size, HxBufferFlags flags, const void* initial_data);
    HX_API void HX_CALL hx_write_buffer(HxBuffer buf, size_t offset, size_t size, const void* data);
    HX_API void HX_CALL hx_read_buffer(HxBuffer buf, size_t offset, size_t size, void* data);
    HX_API HxResult HX_CALL hx_fill_buffer(HxBuffer buf, uint32_t value);
    HX_API HxResult HX_CALL hx_drop_buffer(HxBuffer buf);

    /* ============================================================================
     * Draw
     * ============================================================================ */

    /* Window/backbuffer drawing is unsupported; use hx_render_headless for software output. */
    HX_API void HX_CALL hx_draw_world(HxWin win, HxWorld world, HxCam cam);

    /* Window/backbuffer drawing is unsupported. */
    HX_API void HX_CALL hx_draw_mesh(HxWin win, HxMesh mesh, HxSkin skin, const HxMat4* transform, HxCam cam);

    /* Custom window render passes are unsupported. */
    HX_API void HX_CALL hx_begin_pass(HxWin win, HxCam cam);
    HX_API void HX_CALL hx_end_pass(HxWin win);

    /* ============================================================================
     * Math Helpers (convenience, not handles)
     * ============================================================================ */

    HX_API HxVec3 HX_CALL hx_vec3_add(HxVec3 a, HxVec3 b);
    HX_API HxVec3 HX_CALL hx_vec3_sub(HxVec3 a, HxVec3 b);
    HX_API HxVec3 HX_CALL hx_vec3_mul(HxVec3 v, float scale);
    HX_API float HX_CALL hx_vec3_dot(HxVec3 a, HxVec3 b);
    HX_API HxVec3 HX_CALL hx_vec3_cross(HxVec3 a, HxVec3 b);
    HX_API float HX_CALL hx_vec3_len(HxVec3 v);
    HX_API HxVec3 HX_CALL hx_vec3_norm(HxVec3 v);
    HX_API HxVec3 HX_CALL hx_vec3_lerp(HxVec3 a, HxVec3 b, float t);

    HX_API void HX_CALL hx_make_mat4_identity(HxMat4* m);
    HX_API void HX_CALL hx_mul_mat4(const HxMat4* a, const HxMat4* b, HxMat4* out);
    HX_API void HX_CALL hx_make_mat4_translate(const HxVec3* v, HxMat4* out);
    HX_API void HX_CALL hx_make_mat4_rotate(const HxQuat* q, HxMat4* out);
    HX_API void HX_CALL hx_make_mat4_scale(const HxVec3* v, HxMat4* out);
    HX_API void HX_CALL hx_make_mat4_trs(const HxVec3* t, const HxQuat* r, const HxVec3* s, HxMat4* out);
    HX_API void HX_CALL hx_inverse_mat4(const HxMat4* m, HxMat4* out);
    HX_API void HX_CALL hx_transpose_mat4(const HxMat4* m, HxMat4* out);

    HX_API void HX_CALL hx_make_quat_identity(HxQuat* q);
    HX_API void HX_CALL hx_mul_quat(const HxQuat* a, const HxQuat* b, HxQuat* out);
    HX_API void HX_CALL hx_make_quat_axis_angle(const HxVec3* axis, float rad, HxQuat* out);
    HX_API void HX_CALL hx_make_quat_euler(float x, float y, float z, HxQuat* out); /* XYZ order */
    HX_API void HX_CALL hx_slerp_quat(const HxQuat* a, const HxQuat* b, float t, HxQuat* out);
    HX_API void HX_CALL hx_rotate_vec_quat(const HxQuat* q, const HxVec3* v, HxVec3* out);

    /* ============================================================================
     * 2D / Sprite
     * ============================================================================ */

    typedef struct
    {
        int x, y, width, height;
    } HxRectI;

    HX_API HxSpriteSheet HX_CALL hx_make_sprite_sheet(HxPic pic, int frame_width, int frame_height);
    HX_API size_t HX_CALL hx_get_sprite_sheet_frame_count(HxSpriteSheet sheet);
    HX_API HxResult HX_CALL hx_get_sprite_sheet_frame(HxSpriteSheet sheet, size_t index, HxRectI* out_rect);
    HX_API HxResult HX_CALL hx_drop_sprite_sheet(HxSpriteSheet sheet);

    HX_API HxSpriteAnim HX_CALL hx_make_sprite_anim(HxSpriteSheet sheet,
                                                    const uint32_t* frame_indices,
                                                    const float* frame_durations,
                                                    size_t frame_count,
                                                    bool loop);
    HX_API HxResult HX_CALL hx_update_sprite_anim(HxSpriteAnim anim, double delta_seconds);
    HX_API HxResult HX_CALL hx_get_sprite_anim_frame(HxSpriteAnim anim, size_t* out_frame_index, HxRectI* out_rect);
    HX_API bool HX_CALL hx_get_sprite_anim_done(HxSpriteAnim anim);
    HX_API HxResult HX_CALL hx_restart_sprite_anim(HxSpriteAnim anim);
    HX_API HxResult HX_CALL hx_drop_sprite_anim(HxSpriteAnim anim);

    HX_API HxAtlas HX_CALL hx_make_atlas(int w, int h); /* Creates a transparent RGBA8 atlas. */
    /* Copies the texture's top-left w×h pixels to atlas coordinates x,y; name is reserved. */
    HX_API void HX_CALL hx_add_atlas(HxAtlas atlas, const char* name, HxTex tex, int x, int y, int w, int h);
    HX_API HxTex HX_CALL hx_build_atlas(HxAtlas atlas); /* Copies registered regions into an RGBA8 texture. */
    HX_API HxResult HX_CALL hx_drop_atlas(HxAtlas atlas);

    /* Font loading from TTF file. Returns NULL on failure. */
    HX_API HxFont HX_CALL hx_load_font(const char* path, float size);
    /* Font loading from raw TTF data. Returns NULL on failure. */
    HX_API HxFont HX_CALL hx_load_font_mem(const void* data, size_t size, float pt_size);
    /* Font loading with custom DPI. Returns NULL on failure. */
    HX_API HxFont HX_CALL hx_load_font_dpi(const char* path, float pt_size, float dpi);
    HX_API HxResult HX_CALL hx_drop_font(HxFont font);

    /* Text rendering - draws UTF-8 text at position (x,y) in screen coordinates (top-left origin). */
    HX_API void HX_CALL hx_draw_text(HxWin win, HxFont font, const char* text, float x, float y, HxColor color);
    /* Legacy alias for hx_draw_text. */
    HX_API void HX_CALL hx_say(HxWin win, HxFont font, const char* text, float x, float y, float size, HxColor color);

    /* Get text dimensions in pixels for layout purposes. */
    HX_API void HX_CALL hx_measure_text(HxFont font, const char* text, float* out_width, float* out_height);

    /* ============================================================================
     * Animation
     * ============================================================================ */

    HX_API HxTween HX_CALL hx_make_tween(float from, float to, float duration); /* Simple float tween */
    HX_API float HX_CALL hx_get_tween_value(HxTween tween);
    HX_API bool HX_CALL hx_get_tween_done(HxTween tween);
    HX_API HxResult HX_CALL hx_drop_tween(HxTween tween);

    /* Skeletal animation is unsupported; loaders return NULL and playback does nothing. */
    HX_API HxRig HX_CALL hx_load_rig(const char* path);   /* Load rig from glTF */
    HX_API HxClip HX_CALL hx_load_clip(const char* path); /* Load animation clip */
    HX_API void HX_CALL hx_play_clip(HxRig rig, HxClip clip, bool loop);
    HX_API HxResult HX_CALL hx_drop_rig(HxRig rig);
    HX_API HxResult HX_CALL hx_drop_clip(HxClip clip);

    /* ============================================================================
     * Post-Process Effects (unsupported)
     * ============================================================================ */

    typedef uint8_t HxFxType;
#define HX_FX_BLOOM 0
#define HX_FX_SSAO 1
#define HX_FX_FXAA 2
#define HX_FX_TONEMAP 3

    HX_API HxFx HX_CALL hx_add_fx(HxWin win, HxFxType type); /* Unsupported; returns NULL. */
    HX_API HxResult HX_CALL hx_drop_fx(HxFx fx);

    /* ============================================================================
     * Headless / Offscreen Render
     * ============================================================================ */

    /* Render solid or RGBA8-textured triangle meshes to an RGBA8 buffer with the software backend.
     * PBR lighting, lines and points are not supported by this path. */
    HX_API HxResult HX_CALL hx_render_headless(int width,
                                               int height,
                                               HxWorld world,
                                               HxCam cam,
                                               void* out_pixels,
                                               size_t out_stride /* stride in bytes, 0 = width*4 */
    );

    /* ============================================================================
     * Logging (optional callback)
     * ============================================================================ */

    typedef uint8_t HxLogLevel;
#define HX_LOG_TRACE 0
#define HX_LOG_DEBUG 1
#define HX_LOG_INFO 2
#define HX_LOG_WARN 3
#define HX_LOG_ERROR 4
#define HX_LOG_FATAL 5

    typedef void(HX_CALL* HxLogCallback)(HxLogLevel level, const char* msg, void* user);
    HX_API void HX_CALL hx_set_log_cb(HxLogCallback cb, void* user);

#ifdef __cplusplus
}
#endif

#endif /* HELIX_H */
