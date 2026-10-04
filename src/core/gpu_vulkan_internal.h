// gpu_vulkan_internal.h — Private Vulkan backend contract
#ifndef HELIX_GPU_VULKAN_INTERNAL_H
#define HELIX_GPU_VULKAN_INTERNAL_H

#include "helix.h"

HxResult hx_vulkan_start(HxGpuPreference preference, uint32_t device_index);
void hx_vulkan_stop(void);
bool hx_vulkan_is_active(void);

#endif // HELIX_GPU_VULKAN_INTERNAL_H
