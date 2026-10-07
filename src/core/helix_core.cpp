// helix_core.cpp — Core engine implementation (Phase 1)
#include "helix.h"
#include "core/render_internal.h"
#include "gpu_vulkan_internal.h"
#include "resource_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits>

// -----------------------------------------------------------------------------
// Global State
// -----------------------------------------------------------------------------
static bool g_booted = false;
static HxBackend g_backend = HX_BACKEND_UNKNOWN;
static char g_last_error[1024] = {0};

// Thread-local error storage (simplified for Phase 1 — single thread)
// Must have external linkage for shared library builds
#if defined(_MSC_VER)
__declspec(thread) char tls_last_error[1024] = {0};
#else
__thread char tls_last_error[1024] = {0};
#endif

// -----------------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------------
static void hx_set_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(tls_last_error, sizeof(tls_last_error), fmt, args);
    va_end(args);
    // Also copy to global for single-threaded use
    strncpy(g_last_error, tls_last_error, sizeof(g_last_error) - 1);
}

HX_API const char* HX_CALL hx_last_error(void) {
    return tls_last_error[0] ? tls_last_error : "";
}

// -----------------------------------------------------------------------------
// Version
// -----------------------------------------------------------------------------
HX_API void HX_CALL hx_version(int* major, int* minor, int* patch) {
    if (major) *major = HX_VERSION_MAJOR;
    if (minor) *minor = HX_VERSION_MINOR;
    if (patch) *patch = HX_VERSION_PATCH;
}

// -----------------------------------------------------------------------------
// Backend Detection (Phase 1: Software only)
// -----------------------------------------------------------------------------
static HxBackend hx_detect_backend(void) {
    // Check environment variable
    const char* env = getenv("HX_GPU");
    if (env) {
        if (strcmp(env, "VK") == 0 || strcmp(env, "VULKAN") == 0) return HX_BACKEND_VULKAN;
        if (strcmp(env, "GL") == 0 || strcmp(env, "OPENGL") == 0) return HX_BACKEND_OPENGL;
        if (strcmp(env, "METAL") == 0) return HX_BACKEND_METAL;
        if (strcmp(env, "SOFT") == 0 || strcmp(env, "SOFTWARE") == 0) return HX_BACKEND_SOFTWARE;
    }
    // Phase 1: always software
    return HX_BACKEND_SOFTWARE;
}

// -----------------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------------
HX_API HxResult HX_CALL hx_boot(const HxCfg* cfg) {
    if (g_booted) {
        hx_set_error("Already booted");
        return HX_ERR_ALREADY_BOOTED;
    }

    HxGpu requested = HX_GPU_AUTO;
    if (cfg) {
        requested = cfg->gpu;
    } else {
        switch (hx_detect_backend()) {
            case HX_BACKEND_VULKAN: requested = HX_GPU_VK; break;
            case HX_BACKEND_OPENGL: requested = HX_GPU_GL; break;
            case HX_BACKEND_METAL: requested = HX_GPU_METAL; break;
            default: requested = HX_GPU_SOFT; break;
        }
    }

    if (requested == HX_GPU_VK || requested == HX_GPU_AUTO) {
        const HxGpuPreference preference = cfg ? cfg->gpu_preference : HX_GPU_PREFERENCE_AUTO;
        const uint32_t device_index = cfg ? cfg->gpu_device_index : HX_GPU_DEVICE_DEFAULT;
        HxResult gpu_result = hx_vulkan_start(preference, device_index);
        if (gpu_result == HX_OK) {
            g_backend = HX_BACKEND_VULKAN;
        } else if (requested == HX_GPU_VK) {
            hx_set_error("Vulkan device initialization failed or no compatible graphics device is available");
            g_backend = HX_BACKEND_UNKNOWN;
            return HX_ERR_BACKEND_UNAVAILABLE;
        } else {
#if defined(HX_HAS_SOFTWARE)
            g_backend = HX_BACKEND_SOFTWARE;
#else
            hx_set_error("No requested graphics backend is available in this build");
            g_backend = HX_BACKEND_UNKNOWN;
            return HX_ERR_BACKEND_UNAVAILABLE;
#endif
        }
    } else if (requested == HX_GPU_SOFT) {
#if defined(HX_HAS_SOFTWARE)
        g_backend = HX_BACKEND_SOFTWARE;
#else
        hx_set_error("The software renderer is disabled in this build");
        g_backend = HX_BACKEND_UNKNOWN;
        return HX_ERR_BACKEND_UNAVAILABLE;
#endif
    } else {
        hx_set_error("Requested GPU backend is not available in this build");
        g_backend = HX_BACKEND_UNKNOWN;
        return HX_ERR_BACKEND_UNAVAILABLE;
    }

    g_booted = true;
    tls_last_error[0] = '\0';
    return HX_OK;
}

HX_API void HX_CALL hx_quit(void) {
    hx_resource_release_all();
    hx_vulkan_stop();
    if (!g_booted) return;
    g_booted = false;
    g_backend = HX_BACKEND_UNKNOWN;
    tls_last_error[0] = '\0';
}

// -----------------------------------------------------------------------------
// Backend Info
// -----------------------------------------------------------------------------
HX_API HxBackend HX_CALL hx_get_backend(void) {
    return g_backend;
}

HX_API const char* HX_CALL hx_get_backend_name(HxBackend b) {
    switch (b) {
        case HX_BACKEND_VULKAN:   return "Vulkan";
        case HX_BACKEND_OPENGL:   return "OpenGL";
        case HX_BACKEND_METAL:    return "Metal";
        case HX_BACKEND_SOFTWARE: return "Software";
        default:                  return "Unknown";
    }
}

HX_API const char* HX_CALL hx_get_gpu_vendor(void)   { return "Helix RND (Software)"; }
HX_API const char* HX_CALL hx_get_gpu_renderer(void) { return "Software Rasterizer"; }
HX_API const char* HX_CALL hx_get_gpu_version(void)  { return "0.1.0"; }

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------
static HxLogCallback g_log_cb = NULL;
static void* g_log_user = NULL;

HX_API void HX_CALL hx_set_log_cb(HxLogCallback cb, void* user) {
    g_log_cb = cb;
    g_log_user = user;
}

HX_API HxResult HX_CALL hx_render_headless(
    int width, int height,
    HxWorld world, HxCam cam,
    void* out_pixels, size_t out_stride
) {
    if (!g_booted) {
        hx_set_error("Engine is not booted");
        return HX_ERR_NOT_BOOTED;
    }
    if (width <= 0 || height <= 0 || !world || !cam || !out_pixels ||
        static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / 4u) {
        hx_set_error("Invalid headless render arguments");
        return HX_ERR_INVALID_ARG;
    }
    if (!hx_resource_is_registered(world) || !hx_resource_is_registered(cam)) {
        hx_set_error("Invalid headless world or camera handle");
        return HX_ERR_INVALID_HANDLE;
    }
    const size_t row_bytes = static_cast<size_t>(width) * 4u;
    const size_t stride = out_stride == 0 ? row_bytes : out_stride;
    if (stride < row_bytes || (height > 1 && stride >
        (std::numeric_limits<size_t>::max() - row_bytes) / static_cast<size_t>(height - 1))) {
        hx_set_error("Invalid headless output stride");
        return HX_ERR_INVALID_ARG;
    }
    HxResult result = hx_soft_render_world(width, height, world, cam, out_pixels, stride);
    if (result != HX_OK) hx_set_error("Headless software rendering failed");
    return result;
}
HX_API void HX_CALL hx_get_memory_stats(HxMemoryStats* out_stats) {
    hx_resource_get_stats(out_stats);
}
