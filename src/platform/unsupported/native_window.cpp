// Native window adapter for platforms without a supported window backend.
#include "platform_internal.h"

bool hx_win_platform_create(HxWin win, HxWinFlags flags)
{
    (void)win;
    (void)flags;
    return false;
}

void hx_win_platform_destroy(HxWin win) { (void)win; }

void hx_win_platform_poll(HxWin win) { (void)win; }

void hx_win_platform_swap(HxWin win) { (void)win; }

void hx_win_platform_set_title(HxWin win, const char* title)
{
    (void)win;
    (void)title;
}

void hx_win_platform_set_size(HxWin win, int width, int height)
{
    (void)win;
    (void)width;
    (void)height;
}

void hx_win_platform_set_vsync(HxWin win, bool enabled)
{
    (void)win;
    (void)enabled;
}

void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen)
{
    (void)win;
    (void)fullscreen;
}
