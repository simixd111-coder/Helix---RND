// test_resources.cpp — Resource accounting and shutdown cleanup
#include "helix.h"
#include <cstdio>

bool hx_soft_init(int width, int height);
void hx_soft_get_pixels(void** out_pixels, size_t* out_stride, int* out_width, int* out_height);

int main() {
    HxCfg gpu_config = {};
    gpu_config.gpu = HX_GPU_VK;
    gpu_config.gpu_preference = HX_GPU_PREFERENCE_HIGH_PERFORMANCE;
    gpu_config.gpu_device_index = HX_GPU_DEVICE_DEFAULT;
    HxResult gpu_result = hx_boot(&gpu_config);
    if (gpu_result == HX_OK) {
        if (hx_get_backend() != HX_BACKEND_VULKAN) return 1;
        const uint32_t input_data[] = {0x12345678u, 0xCAFEBABEu};
        uint32_t output_data[2] = {};
        HxBuffer buffer = hx_make_buffer(sizeof(input_data), HX_BUF_STORAGE | HX_BUF_MAP_READ | HX_BUF_MAP_WRITE, input_data);
        if (!buffer) return 2;
        HxMemoryStats gpu_stats = {};
        hx_get_memory_stats(&gpu_stats);
        if (gpu_stats.live_resources != 1 || gpu_stats.gpu_bytes == 0) return 3;
        hx_read_buffer(buffer, 0, sizeof(output_data), output_data);
        if (output_data[0] != input_data[0] || output_data[1] != input_data[1]) return 4;
        if (hx_fill_buffer(buffer, 0xA5A5A5A5u) != HX_OK) return 5;
        hx_read_buffer(buffer, 0, sizeof(output_data), output_data);
        if (output_data[0] != 0xA5A5A5A5u || output_data[1] != 0xA5A5A5A5u) return 6;
        if (hx_drop_buffer(buffer) != HX_OK) return 7;
        if (hx_drop_buffer(buffer) != HX_ERR_ALREADY_DROPPED) return 8;
        hx_quit();
        hx_get_memory_stats(&gpu_stats);
        if (gpu_stats.live_resources != 0 || gpu_stats.cpu_bytes != 0 || gpu_stats.gpu_bytes != 0) return 9;
        std::puts("Vulkan GPU buffer upload/readback: PASS");
    } else if (gpu_result != HX_ERR_BACKEND_UNAVAILABLE) {
        return 10;
    }
    if (hx_get_backend() != HX_BACKEND_UNKNOWN) return 11;

    HxCfg config = {};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    config.app_name = "resource-test";
    if (hx_boot(&config) != HX_OK) return 12;

    HxCam camera = hx_make_cam3d();
    HxWin window = hx_make_win(16, 8, "resource-test", HX_WIN_HEADLESS);
    if (!camera || !window) return 13;
    if (!hx_soft_init(16, 8)) return 14;

    HxMemoryStats stats = {};
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 3 || stats.cpu_bytes == 0 || stats.gpu_bytes != 0) return 15;

    const size_t initial_cpu_bytes = stats.cpu_bytes;
    hx_set_win_size(window, 32, 16);
    hx_get_memory_stats(&stats);
    int window_width = 0;
    int window_height = 0;
    hx_get_win_size(window, &window_width, &window_height);
    if (window_width != 32 || window_height != 16) return 16;
    if (stats.cpu_bytes <= initial_cpu_bytes) return 17;
    const size_t resized_cpu_bytes = stats.cpu_bytes;
    hx_set_win_size(window, 0, 16);
    hx_get_memory_stats(&stats);
    if (stats.cpu_bytes != resized_cpu_bytes) return 18;

    if (hx_drop_cam(camera) != HX_OK) return 19;
    if (hx_drop_cam(camera) != HX_ERR_ALREADY_DROPPED) return 20;
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 2) return 21;

    hx_quit();
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 0 || stats.cpu_bytes != 0 || stats.gpu_bytes != 0) return 22;
    if (hx_drop_win(window) != HX_ERR_ALREADY_DROPPED) return 23;

    void* pixels = reinterpret_cast<void*>(1);
    size_t stride = 1;
    int width = 1;
    int height = 1;
    hx_soft_get_pixels(&pixels, &stride, &width, &height);
    if (pixels || stride != 0 || width != 0 || height != 0) return 24;

    hx_quit();

    std::puts("test_resources: PASS");
    return 0;
}
