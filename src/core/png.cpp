#include "helix.h"
#include "platform/platform_internal.h"
#include "resource_internal.h"
#include "png_internal.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <limits>
#include <vector>

#if defined(HX_TEST_PNG_DIAGNOSTICS)
static void hx_png_trace(const char* stage)
{
    std::fprintf(stderr, "PNG: %s\n", stage);
    std::fflush(stderr);
}
#    define HX_PNG_TRACE(stage) hx_png_trace(stage)
#else
#    define HX_PNG_TRACE(stage) ((void) 0)
#endif

static void hx_png_u32(std::vector<uint8_t>& out, uint32_t value)
{
    out.push_back(static_cast<uint8_t>(value >> 24));
    out.push_back(static_cast<uint8_t>(value >> 16));
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

static uint32_t hx_png_crc32(const uint8_t* bytes, size_t size)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

static void hx_png_chunk(std::vector<uint8_t>& png, const char type[4], const uint8_t* data, size_t size)
{
    hx_png_u32(png, static_cast<uint32_t>(size));
    const size_t crc_start = png.size();
    png.insert(png.end(), type, type + 4);
    if (size)
        png.insert(png.end(), data, data + size);
    hx_png_u32(png, hx_png_crc32(png.data() + crc_start, size + 4u));
}

bool hx_png_write_rgba8(const char* path, int width, int height, const void* pixels, size_t stride)
{
    HX_PNG_TRACE("write start");
    if (!path || !path[0] || width <= 0 || height <= 0 || !pixels
        || static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / 4u)
        return false;
    HX_PNG_TRACE("input validation passed");
    const size_t row_bytes = static_cast<size_t>(width) * 4u;
    if (stride < row_bytes || row_bytes == std::numeric_limits<size_t>::max()
        || (height > 1 && stride > (std::numeric_limits<size_t>::max() - row_bytes) / static_cast<size_t>(height - 1))
        || static_cast<size_t>(height) > std::numeric_limits<size_t>::max() / (row_bytes + 1u))
        return false;
    const size_t raw_size = static_cast<size_t>(height) * (row_bytes + 1u);
    const size_t block_count = raw_size / 65535u + (raw_size % 65535u != 0 ? 1u : 0u);
    const size_t max_chunk = std::numeric_limits<uint32_t>::max();
    if (raw_size > max_chunk - 6u || block_count > (max_chunk - raw_size - 6u) / 5u)
        return false;
    std::vector<uint8_t> raw;
    std::vector<uint8_t> compressed;
    std::vector<uint8_t> png;
    try
    {
        raw.resize(raw_size);
        HX_PNG_TRACE("raw buffer allocated");
        const auto* source = static_cast<const uint8_t*>(pixels);
        for (int y = 0; y < height; ++y)
        {
            const size_t destination_offset = static_cast<size_t>(y) * (row_bytes + 1u);
            raw[destination_offset] = 0;
            std::copy_n(source + static_cast<size_t>(y) * stride, row_bytes, raw.data() + destination_offset + 1u);
        }
        HX_PNG_TRACE("pixels copied");

        compressed.reserve(raw_size + block_count * 5u + 6u);
        compressed.push_back(0x78);
        compressed.push_back(0x01);
        size_t offset = 0;
        while (offset < raw.size())
        {
            const uint16_t block_size = static_cast<uint16_t>(std::min<size_t>(65535u, raw.size() - offset));
            const bool final_block = offset + block_size == raw.size();
            compressed.push_back(final_block ? 1u : 0u);
            compressed.push_back(static_cast<uint8_t>(block_size));
            compressed.push_back(static_cast<uint8_t>(block_size >> 8));
            const uint16_t inverse = static_cast<uint16_t>(~block_size);
            compressed.push_back(static_cast<uint8_t>(inverse));
            compressed.push_back(static_cast<uint8_t>(inverse >> 8));
            compressed.insert(compressed.end(), raw.begin() + offset, raw.begin() + offset + block_size);
            offset += block_size;
        }
        uint32_t a = 1, b = 0;
        for (uint8_t byte : raw)
        {
            a = (a + byte) % 65521u;
            b = (b + a) % 65521u;
        }
        hx_png_u32(compressed, (b << 16) | a);
        HX_PNG_TRACE("compressed stream built");

        static const uint8_t signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
        png.insert(png.end(), signature, signature + sizeof(signature));
        uint8_t ihdr[13] = {};
        ihdr[0] = static_cast<uint8_t>(static_cast<uint32_t>(width) >> 24);
        ihdr[1] = static_cast<uint8_t>(static_cast<uint32_t>(width) >> 16);
        ihdr[2] = static_cast<uint8_t>(static_cast<uint32_t>(width) >> 8);
        ihdr[3] = static_cast<uint8_t>(width);
        ihdr[4] = static_cast<uint8_t>(static_cast<uint32_t>(height) >> 24);
        ihdr[5] = static_cast<uint8_t>(static_cast<uint32_t>(height) >> 16);
        ihdr[6] = static_cast<uint8_t>(static_cast<uint32_t>(height) >> 8);
        ihdr[7] = static_cast<uint8_t>(height);
        ihdr[8] = 8;
        ihdr[9] = 6;
        hx_png_chunk(png, "IHDR", ihdr, sizeof(ihdr));
        HX_PNG_TRACE("IHDR chunk built");
        if (compressed.size() > std::numeric_limits<uint32_t>::max())
            return false;
        hx_png_chunk(png, "IDAT", compressed.data(), compressed.size());
        HX_PNG_TRACE("IDAT chunk built");
        hx_png_chunk(png, "IEND", nullptr, 0);
        HX_PNG_TRACE("IEND chunk built");
    }
    catch (...)
    {
        return false;
    }
    FILE* file = std::fopen(path, "wb");
    if (!file)
        return false;
    HX_PNG_TRACE("output file opened");
    const bool write_ok = std::fwrite(png.data(), 1, png.size(), file) == png.size();
    HX_PNG_TRACE("PNG bytes written");
    std::fclose(file);
    return write_ok;
}

HX_API HxResult HX_CALL hx_save_pic(HxPic picture, const char* path)
{
    HX_PNG_TRACE("hx_save_pic entered");
    if (!path || !path[0])
        return HX_ERR_INVALID_ARG;
    if (!hx_resource_is_registered(picture))
        return HX_ERR_INVALID_HANDLE;
    HX_PNG_TRACE("picture resource found");
    int width = 0;
    int height = 0;
    size_t stride = 0;
    void* pixels = nullptr;
    hx_get_pic_size(picture, &width, &height);
    hx_get_pic_pixels(picture, &pixels, &stride);
    HX_PNG_TRACE("picture metadata read");
    return hx_png_write_rgba8(path, width, height, pixels, stride) ? HX_OK : HX_ERR;
}

HX_API HxResult HX_CALL hx_snap_win(HxWin win, const char* path)
{
    HX_PNG_TRACE("hx_snap_win entered");
    if (!path || !path[0])
        return HX_ERR_INVALID_ARG;
    if (!hx_resource_is_registered(win))
        return HX_ERR_INVALID_HANDLE;
    HX_PNG_TRACE("window resource found");
    if (!win->headless)
        return HX_ERR_UNSUPPORTED;
    void* pixels = nullptr;
    size_t stride = 0;
    int width = 0;
    int height = 0;
    if (hx_headless_get_pixels(win, &pixels, &stride, &width, &height) != HX_OK)
        return HX_ERR_INVALID_STATE;
    HX_PNG_TRACE("headless pixels acquired");
    return hx_png_write_rgba8(path, width, height, pixels, stride) ? HX_OK : HX_ERR;
}
