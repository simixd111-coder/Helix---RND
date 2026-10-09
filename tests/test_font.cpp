// test_font.cpp — Font loading and text rendering tests
#include "helix.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#    define TEST_FONT_PATH "C:\\Windows\\Fonts\\arial.ttf"
#elif defined(__APPLE__)
#    define TEST_FONT_PATH "/System/Library/Fonts/Helvetica.ttc"
#else
#    define TEST_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#endif

static bool file_exists(const char* path)
{
#if defined(_WIN32)
    FILE* f = NULL;
    fopen_s(&f, path, "rb");
#else
    FILE* f = fopen(path, "rb");
#endif
    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

int main()
{
    // Test 1: NULL/invalid parameters should return NULL
    if (hx_load_font(NULL, 16.0f) != NULL)
        return 1;
    if (hx_load_font("nonexistent.ttf", 16.0f) != NULL)
        return 2;
    if (hx_load_font(TEST_FONT_PATH, -1.0f) != NULL)
        return 3;

    // Test 2: drop with NULL should return invalid handle
    if (hx_drop_font(NULL) != HX_ERR_INVALID_HANDLE)
        return 4;

    // Test 3: measure with NULL font/text should not crash
    float w = -1, h = -1;
    hx_measure_text(NULL, "hello", &w, &h);
    if (w != 0 || h != 0)
        return 5;

    // If no system font is available, skip the rest (CI containers may lack fonts)
    if (!file_exists(TEST_FONT_PATH))
        return 0;

    // Test 4: Load a real font
    HxFont font = hx_load_font(TEST_FONT_PATH, 32.0f);
    if (!font)
        return 6;

    // Test 5: Measure text
    hx_measure_text(font, "Hello, World!", &w, &h);
    if (w <= 0 || h <= 0)
    {
        hx_drop_font(font);
        return 7;
    }

    // Test 6: Measure empty text
    hx_measure_text(font, "", &w, &h);
    if (w != 0)
    {
        hx_drop_font(font);
        return 8;
    }

    // Test 7: Draw text to a headless window (should not crash)
    HxWin win = hx_make_win(100, 100, "font-test", HX_WIN_HEADLESS);
    if (win)
    {
        hx_draw_text(win, font, "Hello", 10.0f, 10.0f, HX_WHITE);
        hx_say(win, font, "Hello", 10.0f, 10.0f, 32.0f, HX_WHITE);
        hx_drop_win(win);
    }

    // Test 8: Drop font
    if (hx_drop_font(font) != HX_OK)
        return 9;

        // Test 9: Load font from memory (read the file into a buffer)
#if defined(_WIN32)
    FILE* f = NULL;
    fopen_s(&f, TEST_FONT_PATH, "rb");
#else
    FILE* f = fopen(TEST_FONT_PATH, "rb");
#endif
    if (f)
    {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (size > 0)
        {
            unsigned char* data = (unsigned char*) malloc((size_t) size);
            if (data && fread(data, 1, (size_t) size, f) == (size_t) size)
            {
                HxFont mem_font = hx_load_font_mem(data, (size_t) size, 24.0f);
                if (mem_font)
                {
                    hx_drop_font(mem_font);
                }
            }
            free(data);
        }
        fclose(f);
    }

    return 0;
}
