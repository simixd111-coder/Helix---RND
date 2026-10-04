// gpu_vulkan_stub.cpp — Vulkan discovery API when the loader is disabled
#include "helix.h"
#include "gpu_vulkan_internal.h"

HxResult hx_vulkan_start(HxGpuPreference preference, uint32_t device_index) {
    (void)preference;
    (void)device_index;
    return HX_ERR_BACKEND_UNAVAILABLE;
}

void hx_vulkan_stop(void) {}
bool hx_vulkan_is_active(void) { return false; }

HX_API HxBuffer HX_CALL hx_make_buffer(size_t size, HxBufferFlags flags, const void* initial_data) {
    (void)size; (void)flags; (void)initial_data;
    return NULL;
}

HX_API void HX_CALL hx_write_buffer(HxBuffer buffer, size_t offset, size_t size, const void* data) {
    (void)buffer; (void)offset; (void)size; (void)data;
}

HX_API void HX_CALL hx_read_buffer(HxBuffer buffer, size_t offset, size_t size, void* data) {
    (void)buffer; (void)offset; (void)size; (void)data;
}

HX_API HxResult HX_CALL hx_fill_buffer(HxBuffer buffer, uint32_t value) {
    (void)value;
    return buffer ? HX_ERR_BACKEND_UNAVAILABLE : HX_ERR_INVALID_HANDLE;
}

HX_API HxResult HX_CALL hx_drop_buffer(HxBuffer buffer) {
    return buffer ? HX_ERR_BACKEND_UNAVAILABLE : HX_ERR_INVALID_HANDLE;
}

HX_API HxResult HX_CALL hx_get_gpu_devices(HxGpuDeviceInfo* devices, size_t capacity, size_t* out_count) {
    (void)devices;
    (void)capacity;
    if (out_count) *out_count = 0;
    return out_count ? HX_ERR_BACKEND_UNAVAILABLE : HX_ERR_INVALID_ARG;
}

HX_API HxResult HX_CALL hx_pick_gpu_device(
    const HxGpuDeviceInfo* devices,
    size_t count,
    HxGpuPreference preference,
    size_t* out_index
) {
    (void)devices;
    (void)count;
    (void)preference;
    (void)out_index;
    return HX_ERR_BACKEND_UNAVAILABLE;
}
