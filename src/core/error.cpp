// error.cpp — Error handling utilities
#include "helix.h"
#include <stdarg.h>
#include <stdio.h>

// Thread-local error buffer (defined in helix_core.cpp)
extern __thread char tls_last_error[1024];

void hx_error_v(const char* fmt, va_list args) {
    vsnprintf(tls_last_error, sizeof(tls_last_error), fmt, args);
}

void hx_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    hx_error_v(fmt, args);
    va_end(args);
}

const char* hx_get_error(void) {
    return tls_last_error[0] ? tls_last_error : "";
}

void hx_clear_error(void) {
    tls_last_error[0] = '\0';
}