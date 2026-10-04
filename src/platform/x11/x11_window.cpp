// x11_window.cpp — X11 window implementation (stub for Phase 1)
#include "../platform_internal.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// -----------------------------------------------------------------------------
// X11 Window Data
// -----------------------------------------------------------------------------
typedef struct {
    Display* display;
    Window window;
    int screen;
    Visual* visual;
    Colormap colormap;
    Atom wm_delete_window;
    bool owns_display;
} HxX11Data;

// -----------------------------------------------------------------------------
// Platform Functions
// -----------------------------------------------------------------------------
bool hx_win_platform_create(HxWin win, HxWinFlags flags) {
    HxX11Data* data = (HxX11Data*)calloc(1, sizeof(HxX11Data));
    if (!data) return false;

    data->display = XOpenDisplay(NULL);
    if (!data->display) {
        free(data);
        return false;
    }
    data->owns_display = true;
    data->screen = DefaultScreen(data->display);
    data->visual = DefaultVisual(data->display, data->screen);

    XSetWindowAttributes attrs = {0};
    attrs.colormap = XCreateColormap(data->display, RootWindow(data->display, data->screen), data->visual, AllocNone);
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;
    data->colormap = attrs.colormap;

    unsigned long mask = CWColormap | CWEventMask;
    data->window = XCreateWindow(data->display, RootWindow(data->display, data->screen),
        0, 0, win->width, win->height, 0, DefaultDepth(data->display, data->screen),
        InputOutput, data->visual, mask, &attrs);

    if (!data->window) {
        XCloseDisplay(data->display);
        free(data);
        return false;
    }

    data->wm_delete_window = XInternAtom(data->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(data->display, data->window, &data->wm_delete_window, 1);

    XStoreName(data->display, data->window, win->title);

    if (!(flags & HX_WIN_HIDDEN)) {
        XMapWindow(data->display, data->window);
    }

    win->platform_data = data;
    return true;
}

void hx_win_platform_destroy(HxWin win) {
    HxX11Data* data = (HxX11Data*)win->platform_data;
    if (!data) return;

    if (data->window) XDestroyWindow(data->display, data->window);
    if (data->colormap) XFreeColormap(data->display, data->colormap);
    if (data->owns_display && data->display) XCloseDisplay(data->display);
    free(data);
    win->platform_data = NULL;
}

void hx_win_platform_poll(HxWin win) {
    HxX11Data* data = (HxX11Data*)win->platform_data;
    if (!data) return;

    XEvent event;
    while (XPending(data->display)) {
        XNextEvent(data->display, &event);
        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == data->wm_delete_window) {
                    hx_win_fire_close(win);
                }
                break;
            case ConfigureNotify:
                hx_win_fire_resize(win, event.xconfigure.width, event.xconfigure.height);
                break;
            case FocusIn:
                win->focused = true;
                break;
            case FocusOut:
                win->focused = false;
                break;
            case KeyPress:
            case KeyRelease: {
                KeySym keysym = XLookupKeysym(&event.xkey, 0);
                // Map keysym to HxKey (stub)
                hx_win_fire_key(win, (HxKey)keysym, event.type == KeyPress ? HX_KEY_DOWN : HX_KEY_UP);
                break;
            }
            case ButtonPress:
            case ButtonRelease: {
                HxMouseBtn btn = HX_MOUSE_LEFT;
                if (event.xbutton.button == 2) btn = HX_MOUSE_MIDDLE;
                else if (event.xbutton.button == 3) btn = HX_MOUSE_RIGHT;
                hx_win_fire_mouse_btn(win, btn, event.type == ButtonPress);
                break;
            }
            case MotionNotify:
                hx_win_fire_mouse_move(win, event.xmotion.x, event.xmotion.y);
                break;
        }
    }
}

void hx_win_platform_swap(HxWin win) {
    // For software renderer: no-op
    // For Vulkan/OpenGL: platform-specific swap
    (void)win;
}

void hx_win_platform_set_title(HxWin win, const char* title) {
    HxX11Data* data = (HxX11Data*)win->platform_data;
    if (data && title) XStoreName(data->display, data->window, title);
}

void hx_win_platform_set_size(HxWin win, int width, int height) {
    HxX11Data* data = (HxX11Data*)win->platform_data;
    if (data) XResizeWindow(data->display, data->window, width, height);
}

void hx_win_platform_set_vsync(HxWin win, bool enabled) {
    (void)win; (void)enabled;
}

void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen) {
    (void)win; (void)fullscreen;
}