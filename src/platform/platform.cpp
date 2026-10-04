// platform.cpp — Platform abstraction layer (common)
#include "helix.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <time.h>
    #include <unistd.h>
#endif

// -----------------------------------------------------------------------------
// Platform State
// -----------------------------------------------------------------------------
typedef struct {
    bool initialized;
    HxLogCallback log_cb;
    void* log_user;
} HxPlatformState;

static HxPlatformState g_platform = {0};

// -----------------------------------------------------------------------------
// Platform Init / Quit
// -----------------------------------------------------------------------------
HxResult hx_platform_init(void) {
    if (g_platform.initialized) return HX_OK;
    g_platform.initialized = true;
    return HX_OK;
}

void hx_platform_quit(void) {
    if (!g_platform.initialized) return;
    g_platform.initialized = false;
}

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------
void hx_platform_log(HxLogLevel level, const char* fmt, ...) {
    if (!g_platform.log_cb) return;
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_platform.log_cb(level, buf, g_platform.log_user);
}

void hx_platform_set_log_cb(HxLogCallback cb, void* user) {
    g_platform.log_cb = cb;
    g_platform.log_user = user;
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------
double hx_platform_time(void) {
    // Platform-specific implementation in platform_*.cpp
    return 0.0;
}

void hx_platform_sleep(double seconds) {
    // Platform-specific
}

// -----------------------------------------------------------------------------
// Dynamic Library Loading (for Vulkan/OpenGL/Wayland)
// -----------------------------------------------------------------------------
void* hx_dlopen(const char* name) {
#if defined(_WIN32)
    return LoadLibraryA(name);
#else
    return dlopen(name, RTLD_LAZY | RTLD_LOCAL);
#endif
}

void* hx_dlsym(void* handle, const char* symbol) {
#if defined(_WIN32)
    return (void*)GetProcAddress((HMODULE)handle, symbol);
#else
    return dlsym(handle, symbol);
#endif
}

void hx_dlclose(void* handle) {
#if defined(_WIN32)
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

const char* hx_dlerror(void) {
#if defined(_WIN32)
    static char buf[256];
    DWORD err = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buf, sizeof(buf), NULL);
    return buf;
#else
    return dlerror();
#endif
}