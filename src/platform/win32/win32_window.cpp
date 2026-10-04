// win32_window.cpp — Win32 window implementation (stub for Phase 1)
#include "../platform_internal.h"
#include <windows.h>

// -----------------------------------------------------------------------------
// Win32 Window Data
// -----------------------------------------------------------------------------
typedef struct {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;  // For OpenGL context (future)
    bool owns_window;
} HxWin32Data;

// -----------------------------------------------------------------------------
// Window Class
// -----------------------------------------------------------------------------
static ATOM g_win32_class = 0;
static const wchar_t* g_win32_class_name = L"HelixWindowClass";

static void hx_win32_register_class(void) {
    if (g_win32_class) return;
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    wc.lpszClassName = g_win32_class_name;
    g_win32_class = RegisterClassExW(&wc);
}

// -----------------------------------------------------------------------------
// Platform Functions
// -----------------------------------------------------------------------------
bool hx_win_platform_create(HxWin win, HxWinFlags flags) {
    hx_win32_register_class();

    HxWin32Data* data = (HxWin32Data*)calloc(1, sizeof(HxWin32Data));
    if (!data) return false;

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;

    if (flags & HX_WIN_BORDERLESS) {
        style = WS_POPUP;
    }
    if (!(flags & HX_WIN_RESIZABLE)) {
        style &= ~WS_THICKFRAME;
        style &= ~WS_MAXIMIZEBOX;
    }
    if (flags & HX_WIN_FULLSCREEN) {
        style = WS_POPUP;
        exstyle |= WS_EX_TOPMOST;
    }

    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    int w = win->width, h = win->height;

    // Adjust for DPI (GetDpiForMonitor is not available in MinGW; use GetDeviceCaps)
    HDC screen_dc = GetDC(NULL);
    UINT dpi = (UINT)GetDeviceCaps(screen_dc, LOGPIXELSX);
    ReleaseDC(NULL, screen_dc);
    if (dpi == 0) dpi = 96;
    win->dpi_scale = (float)dpi / 96.0f;

    RECT rect = {0, 0, w, h};
    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;

    wchar_t wtitle[256];
    MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wtitle, 256);

    data->hwnd = CreateWindowExW(
        exstyle, g_win32_class_name, wtitle, style,
        x, y, w, h, NULL, NULL, GetModuleHandleW(NULL), NULL
    );

    if (!data->hwnd) {
        free(data);
        return false;
    }

    data->hdc = GetDC(data->hwnd);
    data->owns_window = true;
    win->cpu_bytes += sizeof(HxWin32Data);

    if (!(flags & HX_WIN_HIDDEN)) {
        ShowWindow(data->hwnd, SW_SHOW);
        SetForegroundWindow(data->hwnd);
        SetFocus(data->hwnd);
    }

    win->platform_data = data;
    return true;
}

void hx_win_platform_destroy(HxWin win) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (!data) return;

    if (data->hdc) ReleaseDC(data->hwnd, data->hdc);
    if (data->owns_window && data->hwnd) DestroyWindow(data->hwnd);
    free(data);
    win->platform_data = NULL;
}

void hx_win_platform_poll(HxWin win) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (!data) return;

    MSG msg;
    while (PeekMessageW(&msg, data->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void hx_win_platform_swap(HxWin win) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (data && data->hdc) {
        SwapBuffers(data->hdc);
    }
}

void hx_win_platform_set_title(HxWin win, const char* title) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (!data || !title) return;
    wchar_t wtitle[256];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);
    SetWindowTextW(data->hwnd, wtitle);
}

void hx_win_platform_set_size(HxWin win, int width, int height) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (!data) return;
    RECT rect = {0, 0, width, height};
    AdjustWindowRectEx(&rect, GetWindowLong(data->hwnd, GWL_STYLE), FALSE, GetWindowLong(data->hwnd, GWL_EXSTYLE));
    SetWindowPos(data->hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void hx_win_platform_set_vsync(HxWin win, bool enabled) {
    // wglSwapIntervalEXT(enabled ? 1 : 0); // OpenGL
    (void)win; (void)enabled;
}

void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen) {
    HxWin32Data* data = (HxWin32Data*)win->platform_data;
    if (!data) return;
    // Implementation would toggle fullscreen mode
    (void)fullscreen;
}