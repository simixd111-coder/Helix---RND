// ecs.cpp — Generic sparse-set ECS for custom engine/game logic
#include "helix.h"
#include "resource_internal.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>
#include <string>
#include <vector>

struct HxEntitySlot {
    uint32_t generation;
    bool alive;
};

struct HxComponentStorage {
    std::string name;
    size_t size;
    size_t alignment;
    size_t stride;
    std::vector<HxEntity> entities;
    std::vector<size_t> sparse;
    std::vector<std::max_align_t> bytes;
};

struct HxWorldImpl {
    std::vector<HxEntitySlot> slots;
    std::vector<uint32_t> free_slots;
    std::vector<HxComponentStorage> components;
};

static uint32_t hx_entity_slot_index(HxEntity entity) {
    const uint32_t encoded_index = static_cast<uint32_t>(entity & 0xFFFFFFFFu);
    return encoded_index == 0 ? UINT32_MAX : encoded_index - 1u;
}

static uint32_t hx_entity_generation(HxEntity entity) {
    return static_cast<uint32_t>(entity >> 32);
}

static HxEntity hx_entity_make(uint32_t index, uint32_t generation) {
    return (static_cast<uint64_t>(generation) << 32) | (static_cast<uint64_t>(index) + 1u);
}

static bool hx_entity_valid(HxWorld world, HxEntity entity) {
    if (!world || entity == HX_NULL_ENTITY) return false;
    const uint32_t index = hx_entity_slot_index(entity);
    if (index >= world->slots.size()) return false;
    const HxEntitySlot& slot = world->slots[index];
    return slot.alive && slot.generation == hx_entity_generation(entity);
}

static HxComponentStorage* hx_component_storage(HxWorld world, HxComponentType type) {
    if (!world || type == HX_COMPONENT_INVALID || type > world->components.size()) return NULL;
    return &world->components[type - 1u];
}

static size_t hx_component_bytes(const HxWorldImpl& world) {
    size_t bytes = sizeof(HxWorldImpl);
    bytes += world.slots.capacity() * sizeof(HxEntitySlot);
    bytes += world.free_slots.capacity() * sizeof(uint32_t);
    bytes += world.components.capacity() * sizeof(HxComponentStorage);
    for (const HxComponentStorage& component : world.components) {
        bytes += component.name.capacity();
        bytes += component.entities.capacity() * sizeof(HxEntity);
        bytes += component.sparse.capacity() * sizeof(size_t);
        bytes += component.bytes.capacity() * sizeof(std::max_align_t);
    }
    return bytes;
}

static void hx_world_refresh_memory(HxWorld world) {
    (void)hx_resource_resize(world, hx_component_bytes(*world), 0);
}

static void hx_world_destroy_resource(void* resource) {
    delete static_cast<HxWorld>(resource);
}

static size_t hx_component_data_units(size_t stride, size_t count) {
    if (count != 0 && stride > std::numeric_limits<size_t>::max() / count) return SIZE_MAX;
    const size_t bytes = stride * count;
    const size_t unit = sizeof(std::max_align_t);
    return bytes / unit + (bytes % unit != 0 ? 1u : 0u);
}

static void* hx_component_data(HxComponentStorage& storage, size_t packed_index) {
    auto* bytes = reinterpret_cast<unsigned char*>(storage.bytes.data());
    return bytes + packed_index * storage.stride;
}

static bool hx_component_remove(HxWorld world, HxEntity entity, HxComponentStorage& storage) {
    const uint32_t slot_index = hx_entity_slot_index(entity);
    if (slot_index >= storage.sparse.size() || storage.sparse[slot_index] == 0) return false;

    const size_t packed_index = storage.sparse[slot_index] - 1u;
    const size_t last_index = storage.entities.size() - 1u;
    if (packed_index != last_index) {
        const HxEntity moved_entity = storage.entities[last_index];
        std::memcpy(hx_component_data(storage, packed_index), hx_component_data(storage, last_index), storage.stride);
        storage.entities[packed_index] = moved_entity;
        const uint32_t moved_slot = hx_entity_slot_index(moved_entity);
        storage.sparse[moved_slot] = packed_index + 1u;
    }
    storage.entities.pop_back();
    storage.bytes.resize(hx_component_data_units(storage.stride, storage.entities.size()));
    storage.sparse[slot_index] = 0;
    (void)world;
    return true;
}

HX_API HxWorld HX_CALL hx_make_world(void) {
    HxWorld world = new (std::nothrow) HxWorldImpl{};
    if (!world) return NULL;
    if (!hx_resource_register(world, sizeof(HxWorldImpl), 0, hx_world_destroy_resource)) {
        delete world;
        return NULL;
    }
    return world;
}

HX_API HxResult HX_CALL hx_drop_world(HxWorld world) {
    if (!world) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(world)) return HX_ERR_ALREADY_DROPPED;
    delete world;
    return HX_OK;
}

HX_API HxEntity HX_CALL hx_make_entity(HxWorld world) {
    if (!world) return HX_NULL_ENTITY;
    try {
        uint32_t index;
        if (!world->free_slots.empty()) {
            index = world->free_slots.back();
            world->free_slots.pop_back();
        } else {
            if (world->slots.size() >= UINT32_MAX) return HX_NULL_ENTITY;
            index = static_cast<uint32_t>(world->slots.size());
            world->slots.push_back({1u, false});
        }
        HxEntitySlot& slot = world->slots[index];
        slot.alive = true;
        const HxEntity entity = hx_entity_make(index, slot.generation);
        hx_world_refresh_memory(world);
        return entity;
    } catch (...) {
        return HX_NULL_ENTITY;
    }
}

HX_API HxResult HX_CALL hx_drop_entity(HxWorld world, HxEntity entity) {
    if (!hx_entity_valid(world, entity)) return HX_ERR_INVALID_HANDLE;
    const uint32_t slot_index = hx_entity_slot_index(entity);
    try {
        world->free_slots.push_back(slot_index);
    } catch (...) {
        return HX_ERR_OOM;
    }

    for (HxComponentStorage& component : world->components) {
        hx_component_remove(world, entity, component);
    }

    HxEntitySlot& slot = world->slots[slot_index];
    slot.alive = false;
    ++slot.generation;
    if (slot.generation == 0) slot.generation = 1;
    hx_world_refresh_memory(world);
    return HX_OK;
}

HX_API bool HX_CALL hx_get_entity_alive(HxWorld world, HxEntity entity) {
    return hx_entity_valid(world, entity);
}

HX_API HxResult HX_CALL hx_register_component_type(
    HxWorld world,
    const char* name,
    size_t size,
    size_t alignment,
    HxComponentType* out_type
) {
    if (!world || !name || !name[0] || size == 0 || !out_type || alignment == 0 ||
        (alignment & (alignment - 1u)) != 0 || alignment > alignof(std::max_align_t)) {
        return HX_ERR_INVALID_ARG;
    }
    if (size > std::numeric_limits<size_t>::max() - (alignment - 1u)) return HX_ERR_INVALID_ARG;

    for (size_t index = 0; index < world->components.size(); ++index) {
        HxComponentStorage& current = world->components[index];
        if (current.name == name) {
            if (current.size != size || current.alignment != alignment) return HX_ERR_INVALID_ARG;
            *out_type = static_cast<HxComponentType>(index + 1u);
            return HX_OK;
        }
    }

    if (world->components.size() >= UINT32_MAX) return HX_ERR_OOM;
    try {
        HxComponentStorage component{};
        component.name = name;
        component.size = size;
        component.alignment = alignment;
        component.stride = ((size + alignment - 1u) / alignment) * alignment;
        world->components.push_back(std::move(component));
        *out_type = static_cast<HxComponentType>(world->components.size());
        hx_world_refresh_memory(world);
        return HX_OK;
    } catch (...) {
        return HX_ERR_OOM;
    }
}

HX_API HxResult HX_CALL hx_add_component(HxWorld world, HxEntity entity, HxComponentType type, const void* initial_data) {
    if (!hx_entity_valid(world, entity)) return HX_ERR_INVALID_HANDLE;
    HxComponentStorage* component = hx_component_storage(world, type);
    if (!component) return HX_ERR_INVALID_ARG;

    const uint32_t slot_index = hx_entity_slot_index(entity);
    try {
        component->sparse.resize(std::max(component->sparse.size(), static_cast<size_t>(slot_index) + 1u), 0);
        if (component->sparse[slot_index] != 0) {
            void* destination = hx_component_data(*component, component->sparse[slot_index] - 1u);
            if (initial_data) std::memcpy(destination, initial_data, component->size);
            else std::memset(destination, 0, component->size);
            return HX_OK;
        }

        const size_t new_count = component->entities.size() + 1u;
        const size_t data_units = hx_component_data_units(component->stride, new_count);
        if (data_units == SIZE_MAX) return HX_ERR_OOM;
        component->entities.reserve(new_count);
        component->bytes.resize(data_units);
        const size_t packed_index = component->entities.size();
        component->entities.push_back(entity);
        component->sparse[slot_index] = packed_index + 1u;
        void* destination = hx_component_data(*component, packed_index);
        std::memset(destination, 0, component->stride);
        if (initial_data) std::memcpy(destination, initial_data, component->size);
        hx_world_refresh_memory(world);
        return HX_OK;
    } catch (...) {
        return HX_ERR_OOM;
    }
}

HX_API void* HX_CALL hx_get_component(HxWorld world, HxEntity entity, HxComponentType type) {
    if (!hx_entity_valid(world, entity)) return NULL;
    HxComponentStorage* component = hx_component_storage(world, type);
    if (!component) return NULL;
    const uint32_t slot_index = hx_entity_slot_index(entity);
    if (slot_index >= component->sparse.size() || component->sparse[slot_index] == 0) return NULL;
    const size_t packed_index = component->sparse[slot_index] - 1u;
    if (component->entities[packed_index] != entity) return NULL;
    return hx_component_data(*component, packed_index);
}

HX_API HxResult HX_CALL hx_remove_component(HxWorld world, HxEntity entity, HxComponentType type) {
    if (!hx_entity_valid(world, entity)) return HX_ERR_INVALID_HANDLE;
    HxComponentStorage* component = hx_component_storage(world, type);
    if (!component) return HX_ERR_INVALID_ARG;
    if (!hx_component_remove(world, entity, *component)) return HX_ERR_INVALID_HANDLE;
    hx_world_refresh_memory(world);
    return HX_OK;
}

HX_API HxResult HX_CALL hx_for_each_component(
    HxWorld world,
    HxComponentType type,
    HxComponentEachCallback callback,
    void* user
) {
    HxComponentStorage* component = hx_component_storage(world, type);
    if (!component || !callback) return HX_ERR_INVALID_ARG;

    std::vector<HxEntity> snapshot;
    try {
        snapshot = component->entities;
    } catch (...) {
        return HX_ERR_OOM;
    }
    for (HxEntity entity : snapshot) {
        void* data = hx_get_component(world, entity, type);
        if (data) callback(world, entity, data, user);
    }
    return HX_OK;
}
