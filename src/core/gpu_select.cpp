// gpu_select.cpp — Backend-agnostic GPU device selection policy
// The pick algorithm is pure logic over a caller-supplied device list and
// therefore must work whether or not any hardware backend is compiled in.
#include "helix.h"

HX_API HxResult HX_CALL hx_pick_gpu_device(const HxGpuDeviceInfo* devices, size_t count, HxGpuPreference preference,
                                           size_t* out_index)
{
    if (!devices || count == 0 || !out_index || preference > HX_GPU_PREFERENCE_LOW_POWER)
    {
        return HX_ERR_INVALID_ARG;
    }

    size_t best_index = 0;
    int best_score = -1;
    for (size_t index = 0; index < count; ++index)
    {
        const HxGpuDeviceInfo& device = devices[index];
        int score = 0;
        if (preference == HX_GPU_PREFERENCE_LOW_POWER)
        {
            switch (device.type)
            {
            case HX_GPU_DEVICE_INTEGRATED:
                score = 500;
                break;
            case HX_GPU_DEVICE_VIRTUAL:
                score = 400;
                break;
            case HX_GPU_DEVICE_CPU:
                score = 300;
                break;
            case HX_GPU_DEVICE_DISCRETE:
                score = 200;
                break;
            default:
                score = 100;
                break;
            }
        }
        else
        {
            switch (device.type)
            {
            case HX_GPU_DEVICE_DISCRETE:
                score = 500;
                break;
            case HX_GPU_DEVICE_INTEGRATED:
                score = 400;
                break;
            case HX_GPU_DEVICE_VIRTUAL:
                score = 300;
                break;
            case HX_GPU_DEVICE_OTHER:
                score = 200;
                break;
            case HX_GPU_DEVICE_CPU:
                score = 100;
                break;
            default:
                score = 0;
                break;
            }
        }
        if (device.graphics_queue)
            score += 20;
        if (device.compute_queue)
            score += 10;
        if (score > best_score)
        {
            best_score = score;
            best_index = index;
        }
    }

    *out_index = best_index;
    return HX_OK;
}
