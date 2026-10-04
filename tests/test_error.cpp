// test_error.cpp — Error handling tests
#include "helix.h"
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    printf("Testing error handling...\n");

    // Test initial state
    assert(strcmp(hx_last_error(), "") == 0);

    // Test boot/quit
    HxResult res = hx_boot(NULL);
    assert(res == HX_OK);
    assert(strcmp(hx_last_error(), "") == 0);

    // Test double boot
    res = hx_boot(NULL);
    assert(res == HX_ERR_ALREADY_BOOTED);
    assert(strstr(hx_last_error(), "Already booted") != NULL);

    hx_quit();

    // Test operations without boot
    res = hx_boot(NULL);
    assert(res == HX_OK);

    HxWin win = hx_make_win(100, 100, "Test", HX_WIN_HEADLESS);
    assert(win != NULL);

    hx_drop_win(win);
    hx_quit();

    printf("All error tests PASSED\n");
    return 0;
}