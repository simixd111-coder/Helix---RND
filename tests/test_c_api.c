/* test_c_api.c — Compile and link the public interface as C99 */
#include "helix.h"

int main(void) {
    HxCfg config = {0};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    config.app_name = "c-api-test";
    if (hx_boot(&config) != HX_OK) return 1;

    HxWin window = hx_make_win(8, 8, "C API test", HX_WIN_HEADLESS);
    if (window == HX_NULL_HANDLE) return 2;

    HxMemoryStats stats = {0};
    hx_get_memory_stats(&stats);
    if (stats.live_resources == 0 || stats.cpu_bytes == 0) return 3;

    if (hx_drop_win(window) != HX_OK) return 4;
    hx_quit();
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 0 || stats.cpu_bytes != 0 || stats.gpu_bytes != 0) return 5;
    return 0;
}
