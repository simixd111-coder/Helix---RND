// png_internal.h — Internal PNG writer shared by core and demos.
// License: MIT
#pragma once

#include <cstddef>

// Write an RGBA8 top-left-origin pixel buffer as a valid PNG file
// (8-bit depth, color type 6, zlib stored-block IDAT, CRC32 chunks).
// Returns true on success. Thread-safe; no global state.
bool hx_png_write_rgba8(const char* path, int width, int height, const void* pixels, size_t stride);
