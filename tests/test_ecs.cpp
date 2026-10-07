#include "helix.h"
#include <cstdio>

struct Position
{
    float x;
    float y;
};

int main()
{
    HxCfg config{};
    config.gpu = HX_GPU_SOFT;
    config.headless = true;
    if (hx_boot(&config) != HX_OK)
        return 1;

    HxWorld world = hx_make_world();
    if (!world)
    {
        hx_quit();
        return 2;
    }

    int result = 0;
    HxEntity entity = hx_make_entity(world);
    HxComponentType position_type = 0;
    if (entity == HX_NULL_ENTITY ||
        hx_register_component_type(world, "Position", sizeof(Position), alignof(Position), &position_type) != HX_OK)
    {
        result = 3;
    }

    Position expected{3.5f, -1.25f};
    Position input = expected;
    if (result == 0 && hx_add_component(world, entity, position_type, &input) != HX_OK)
        result = 4;
    input.x = 0.0f;
    auto* stored = result == 0 ? static_cast<Position*>(hx_get_component(world, entity, position_type)) : nullptr;
    if (result == 0 && (!stored || stored->x != expected.x || stored->y != expected.y))
        result = 5;
    if (result == 0 && hx_remove_component(world, entity, position_type) != HX_OK)
        result = 6;
    if (result == 0 && hx_get_component(world, entity, position_type) != nullptr)
        result = 7;
    if (entity != HX_NULL_ENTITY && hx_drop_entity(world, entity) != HX_OK && result == 0)
        result = 8;
    if (hx_drop_world(world) != HX_OK && result == 0)
        result = 9;
    hx_quit();

    if (result != 0)
        std::fprintf(stderr, "ECS test failed: %d\n", result);
    return result;
}