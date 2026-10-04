// cocoa_window.mm — Cocoa window implementation (stub for Phase 1)
#include "helix.h"
#import <Cocoa/Cocoa.h>

// -----------------------------------------------------------------------------
// Cocoa Window Data
// -----------------------------------------------------------------------------
typedef struct {
    NSWindow* window;
    NSView* content_view;
    bool owns_window;
} HxCocoaData;

// -----------------------------------------------------------------------------
// Platform Functions
// -----------------------------------------------------------------------------
bool hx_win_platform_create(HxWin win, HxWinFlags flags) {
    HxCocoaData* data = (HxCocoaData*)calloc(1, sizeof(HxCocoaData));
    if (!data) return false;

    NSRect frame = NSMakeRect(0, 0, win->width, win->height);
    NSUInteger style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    if (flags & HX_WIN_RESIZABLE) style |= NSResizableWindowMask;
    if (flags & HX_WIN_BORDERLESS) style = NSBorderlessWindowMask;

    data->window = [[NSWindow alloc] initWithContentRect:frame
        styleMask:style
        backing:NSBackingStoreBuffered
        defer:NO];
    data->owns_window = true;

    [data->window setTitle:[NSString stringWithUTF8String:win->title]];
    [data->window makeKeyAndOrderFront:nil];

    data->content_view = [data->window contentView];
    [data->content_view setWantsLayer:YES];

    win->platform_data = data;
    return true;
}

void hx_win_platform_destroy(HxWin win) {
    HxCocoaData* data = (HxCocoaData*)win->platform_data;
    if (!data) return;

    if (data->owns_window && data->window) {
        [data->window close];
        [data->window release];
    }
    free(data);
    win->platform_data = NULL;
}

void hx_win_platform_poll(HxWin win) {
    // Cocoa uses run loop; events delivered via callbacks
    (void)win;
}

void hx_win_platform_swap(HxWin win) {
    // For Metal: present drawable
    // For software: no-op
    (void)win;
}

void hx_win_platform_set_title(HxWin win, const char* title) {
    HxCocoaData* data = (HxCocoaData*)win->platform_data;
    if (data && title) {
        [data->window setTitle:[NSString stringWithUTF8String:title]];
    }
}

void hx_win_platform_set_size(HxWin win, int width, int height) {
    HxCocoaData* data = (HxCocoaData*)win->platform_data;
    if (data) {
        NSRect frame = [data->window frame];
        frame.size.width = width;
        frame.size.height = height;
        [data->window setFrame:frame display:YES];
    }
}

void hx_win_platform_set_vsync(HxWin win, bool enabled) {
    (void)win; (void)enabled;
}

void hx_win_platform_set_fullscreen(HxWin win, bool fullscreen) {
    (void)win; (void)fullscreen;
}