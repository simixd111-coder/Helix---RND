// handle.cpp — Opaque handle management (Phase 1: simple generation)
#include "helix.h"
#include <atomic>
#include <cstdint>

// -----------------------------------------------------------------------------
// Handle System
// -----------------------------------------------------------------------------
// Handles are 64-bit integers with:
// - High 32 bits: type tag (for debug validation)
// - Low 32 bits: unique index
//
// Type tags:
#define HX_HANDLE_TAG_WIN      0x57494E00  // 'WIN'
#define HX_HANDLE_TAG_WORLD    0x57524C00  // 'WRL'
#define HX_HANDLE_TAG_MESH     0x4D455300  // 'MES'
#define HX_HANDLE_TAG_SKIN     0x534B4E00  // 'SKN'
#define HX_HANDLE_TAG_CAM      0x43414D00  // 'CAM'
#define HX_HANDLE_TAG_LAMP     0x4C4D5000  // 'LMP'
#define HX_HANDLE_TAG_TEX      0x54455800  // 'TEX'
#define HX_HANDLE_TAG_SHADER   0x53484400  // 'SHD'
#define HX_HANDLE_TAG_BUFFER   0x42554600  // 'BUF'
#define HX_HANDLE_TAG_ATLAS    0x41544C00  // 'ATL'
#define HX_HANDLE_TAG_FONT     0x464E5400  // 'FNT'
#define HX_HANDLE_TAG_LAYER    0x4C595200  // 'LYR'
#define HX_HANDLE_TAG_TWEEN    0x54574E00  // 'TWN'
#define HX_HANDLE_TAG_FX       0x46580000  // 'FX'
#define HX_HANDLE_TAG_AUDIO    0x41554400  // 'AUD'

static std::atomic<uint_fast32_t> g_handle_counter{0};

uint64_t hx_handle_make(uint32_t tag) {
    uint32_t idx = g_handle_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    return ((uint64_t)tag << 32) | idx;
}

uint32_t hx_handle_tag(uint64_t handle) {
    return (uint32_t)(handle >> 32);
}

uint32_t hx_handle_index(uint64_t handle) {
    return (uint32_t)(handle & 0xFFFFFFFFu);
}

bool hx_handle_valid(uint64_t handle, uint32_t expected_tag) {
    if (handle == 0) return false;
    return hx_handle_tag(handle) == expected_tag;
}

// -----------------------------------------------------------------------------
// Handle Maps (Phase 1: simple arrays, Phase 2+: proper hash maps)
// -----------------------------------------------------------------------------
#define HX_MAX_HANDLES 65536

static void* g_handles[HX_MAX_HANDLES] = {0};
static uint32_t g_handle_tags[HX_MAX_HANDLES] = {0};
static std::atomic<uint_fast32_t> g_handle_count{0};

void* hx_handle_alloc(uint32_t tag, void* ptr) {
    uint32_t idx = g_handle_count.fetch_add(1, std::memory_order_relaxed);
    if (idx >= HX_MAX_HANDLES) return NULL;

    uint64_t handle = hx_handle_make(tag);
    g_handles[idx] = ptr;
    g_handle_tags[idx] = tag;
    return (void*)handle;
}

void* hx_handle_get(uint64_t handle, uint32_t expected_tag) {
    if (!hx_handle_valid(handle, expected_tag)) return NULL;
    uint32_t idx = hx_handle_index(handle) - 1;
    if (idx >= HX_MAX_HANDLES) return NULL;
    if (g_handle_tags[idx] != expected_tag) return NULL;
    return g_handles[idx];
}

void hx_handle_free(uint64_t handle, uint32_t expected_tag) {
    if (!hx_handle_valid(handle, expected_tag)) return;
    uint32_t idx = hx_handle_index(handle) - 1;
    if (idx >= HX_MAX_HANDLES) return;
    if (g_handle_tags[idx] != expected_tag) return;
    g_handles[idx] = NULL;
    g_handle_tags[idx] = 0;
}