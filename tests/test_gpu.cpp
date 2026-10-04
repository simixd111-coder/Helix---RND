// test_gpu.cpp — GPU enumeration and adapter preference tests
#include "helix.h"
#include <cstring>
#include <cstdio>

int main() {
    HxGpuDeviceInfo devices[3] = {};
    std::strcpy(devices[0].name, "Discrete test adapter");
    devices[0].type = HX_GPU_DEVICE_DISCRETE;
    devices[0].graphics_queue = true;
    devices[0].compute_queue = true;
    std::strcpy(devices[1].name, "Integrated test adapter");
    devices[1].type = HX_GPU_DEVICE_INTEGRATED;
    devices[1].graphics_queue = true;
    devices[1].compute_queue = true;
    std::strcpy(devices[2].name, "CPU test adapter");
    devices[2].type = HX_GPU_DEVICE_CPU;
    devices[2].graphics_queue = true;

    size_t selected = SIZE_MAX;
    if (hx_pick_gpu_device(devices, 3, HX_GPU_PREFERENCE_HIGH_PERFORMANCE, &selected) != HX_OK || selected != 0) return 1;
    if (hx_pick_gpu_device(devices, 3, HX_GPU_PREFERENCE_LOW_POWER, &selected) != HX_OK || selected != 1) return 2;
    if (hx_pick_gpu_device(NULL, 0, HX_GPU_PREFERENCE_AUTO, &selected) != HX_ERR_INVALID_ARG) return 3;

    size_t device_count = 0;
    HxResult result = hx_get_gpu_devices(NULL, 0, &device_count);
    if (result == HX_OK) {
        if (device_count == 0) return 4;
        HxGpuDeviceInfo detected[8] = {};
        if (hx_get_gpu_devices(detected, 8, &device_count) != HX_OK || device_count == 0) return 5;
    } else if (result != HX_ERR_BACKEND_UNAVAILABLE) {
        return 6;
    }

    std::puts("test_gpu: PASS");
    return 0;
}
