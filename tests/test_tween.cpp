#include "helix.h"
#include <chrono>
#include <cmath>
#include <thread>

int main()
{
    HxTween tween = hx_make_tween(2.0f, 8.0f, 0.04f);
    if (!tween || std::fabs(hx_get_tween_value(tween) - 2.0f) > 0.01f || hx_get_tween_done(tween))
        return 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    if (std::fabs(hx_get_tween_value(tween) - 8.0f) > 0.01f || !hx_get_tween_done(tween))
        return 2;
    if (hx_drop_tween(tween) != HX_OK || hx_drop_tween(tween) != HX_ERR_ALREADY_DROPPED)
        return 3;
    HxTween immediate = hx_make_tween(-1.0f, 3.0f, 0.0f);
    if (!immediate || hx_get_tween_value(immediate) != 3.0f || !hx_get_tween_done(immediate))
        return 4;
    hx_drop_tween(immediate);
    return hx_make_tween(0.0f, 1.0f, -1.0f) ? 5 : 0;
}
