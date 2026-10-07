#include "helix.h"
#include "resource_internal.h"
#include <chrono>
#include <cmath>
#include <new>

struct HxTweenImpl
{
    float from;
    float to;
    float duration;
    std::chrono::steady_clock::time_point started;
};

static void hx_tween_destroy_resource(void* resource) { delete static_cast<HxTween>(resource); }

HX_API HxTween HX_CALL hx_make_tween(float from, float to, float duration)
{
    if (!std::isfinite(from) || !std::isfinite(to) || !std::isfinite(duration) || duration < 0.0f)
        return nullptr;
    HxTween tween = new (std::nothrow) HxTweenImpl{from, to, duration, std::chrono::steady_clock::now()};
    if (!tween)
        return nullptr;
    if (!hx_resource_register(tween, sizeof(HxTweenImpl), 0, hx_tween_destroy_resource))
    {
        delete tween;
        return nullptr;
    }
    return tween;
}

HX_API float HX_CALL hx_get_tween_value(HxTween tween)
{
    if (!hx_resource_is_registered(tween))
        return 0.0f;
    if (tween->duration == 0.0f)
        return tween->to;
    const float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - tween->started).count();
    const float amount = elapsed <= 0.0f ? 0.0f : (elapsed >= tween->duration ? 1.0f : elapsed / tween->duration);
    return tween->from + (tween->to - tween->from) * amount;
}

HX_API bool HX_CALL hx_get_tween_done(HxTween tween)
{
    if (!hx_resource_is_registered(tween))
        return false;
    return tween->duration == 0.0f ||
           std::chrono::duration<float>(std::chrono::steady_clock::now() - tween->started).count() >= tween->duration;
}

HX_API HxResult HX_CALL hx_drop_tween(HxTween tween)
{
    if (!tween)
        return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(tween))
        return HX_ERR_ALREADY_DROPPED;
    delete tween;
    return HX_OK;
}
