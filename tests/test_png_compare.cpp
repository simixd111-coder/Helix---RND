// test_png_compare.cpp — Visual regression: compare a rendered PNG against a
// reference image with per-channel tolerance and an allowance for a small
// fraction of outlier pixels (cross-platform float/libm differences).
//
// Usage: test_png_compare <rendered.png> <reference.png>
// Exit 0 = within tolerance, 1 = mismatch/failure.
#define STB_IMAGE_IMPLEMENTATION
#include "../src/thirdparty/stb/stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tolerance: max per-channel delta (0..255) for a pixel to count as "equal".
static const int kChannelTolerance = 8;
// Allow up to 0.5% of pixels to exceed the channel tolerance (edge rounding
// differences between MSVC and GCC/Clang libm).
static const double kMaxOutlierRatio = 0.005;

static int fail(const char* reason)
{
    fprintf(stderr, "test_png_compare: FAIL — %s\n", reason);
    return 1;
}

int main(int argc, char** argv)
{
    if (argc != 3)
        return fail("usage: test_png_compare <rendered.png> <reference.png>");

    const char* rendered_path = argv[1];
    const char* reference_path = argv[2];

    int rw = 0, rh = 0, rc = 0;
    unsigned char* rendered = stbi_load(rendered_path, &rw, &rh, &rc, 4);
    if (!rendered)
    {
        fprintf(stderr,
                "test_png_compare: cannot decode '%s' (%s)\n",
                rendered_path,
                stbi_failure_reason() ? stbi_failure_reason() : "unknown");
        return 1;
    }

    int fw = 0, fh = 0, fc = 0;
    unsigned char* reference = stbi_load(reference_path, &fw, &fh, &fc, 4);
    if (!reference)
    {
        fprintf(stderr,
                "test_png_compare: cannot decode '%s' (%s)\n",
                reference_path,
                stbi_failure_reason() ? stbi_failure_reason() : "unknown");
        stbi_image_free(rendered);
        return 1;
    }

    if (rw != fw || rh != fh)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "size mismatch: rendered %dx%d vs reference %dx%d", rw, rh, fw, fh);
        stbi_image_free(rendered);
        stbi_image_free(reference);
        return fail(buf);
    }

    const size_t pixel_count = (size_t) rw * (size_t) rh;
    size_t outliers = 0;
    size_t max_channel_sum = 0; // worst per-pixel channel delta (for reporting)

    for (size_t i = 0; i < pixel_count; ++i)
    {
        const unsigned char* p = rendered + i * 4;
        const unsigned char* q = reference + i * 4;
        int pixel_bad = 0;
        int channel_sum = 0;
        for (int c = 0; c < 4; ++c)
        {
            const int delta = p[c] > q[c] ? (int) p[c] - (int) q[c] : (int) q[c] - (int) p[c];
            channel_sum += delta;
            if (delta > kChannelTolerance)
                pixel_bad = 1;
        }
        if (channel_sum > (int) max_channel_sum)
            max_channel_sum = channel_sum;
        if (pixel_bad)
            ++outliers;
    }

    stbi_image_free(rendered);
    stbi_image_free(reference);

    const double outlier_ratio = (double) outliers / (double) pixel_count;
    printf("test_png_compare: %dx%d, %zu outlier pixels (%.4f%%), worst channel sum %zu (tolerance %d/255)\n",
           rw,
           rh,
           outliers,
           outlier_ratio * 100.0,
           max_channel_sum,
           kChannelTolerance);

    if (outlier_ratio > kMaxOutlierRatio)
    {
        char buf[160];
        snprintf(buf,
                 sizeof(buf),
                 "%.4f%% of pixels exceed tolerance (limit %.4f%%, worst channel sum %zu)",
                 outlier_ratio * 100.0,
                 kMaxOutlierRatio * 100.0,
                 max_channel_sum);
        return fail(buf);
    }

    printf("test_png_compare: PASS\n");
    return 0;
}
