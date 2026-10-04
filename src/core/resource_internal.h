// resource_internal.h — Internal resource lifetime and memory tracking
#ifndef HELIX_RESOURCE_INTERNAL_H
#define HELIX_RESOURCE_INTERNAL_H

#include "helix.h"

typedef void (*HxResourceDestroyFn)(void* resource);

bool hx_resource_register(void* resource, size_t cpu_bytes, size_t gpu_bytes, HxResourceDestroyFn destroy);
bool hx_resource_unregister(void* resource);
bool hx_resource_resize(void* resource, size_t cpu_bytes, size_t gpu_bytes);
void hx_resource_release_all(void);
void hx_resource_get_stats(HxMemoryStats* out_stats);

#endif // HELIX_RESOURCE_INTERNAL_H
