// resource.cpp — Central resource ownership and memory accounting
#include "resource_internal.h"
#include <mutex>
#include <stdlib.h>

struct HxResourceNode {
    void* resource;
    size_t cpu_bytes;
    size_t gpu_bytes;
    HxResourceDestroyFn destroy;
    HxResourceNode* next;
};

static std::mutex g_resource_mutex;
static HxResourceNode* g_resources = NULL;
static HxMemoryStats g_resource_stats = {0, 0, 0};

bool hx_resource_register(void* resource, size_t cpu_bytes, size_t gpu_bytes, HxResourceDestroyFn destroy) {
    if (!resource || !destroy) return false;

    HxResourceNode* node = static_cast<HxResourceNode*>(malloc(sizeof(HxResourceNode)));
    if (!node) return false;
    node->resource = resource;
    node->cpu_bytes = cpu_bytes;
    node->gpu_bytes = gpu_bytes;
    node->destroy = destroy;

    std::lock_guard<std::mutex> lock(g_resource_mutex);
    if (cpu_bytes > SIZE_MAX - g_resource_stats.cpu_bytes ||
        gpu_bytes > SIZE_MAX - g_resource_stats.gpu_bytes) {
        free(node);
        return false;
    }
    for (HxResourceNode* current = g_resources; current; current = current->next) {
        if (current->resource == resource) {
            free(node);
            return false;
        }
    }
    node->next = g_resources;
    g_resources = node;
    ++g_resource_stats.live_resources;
    g_resource_stats.cpu_bytes += cpu_bytes;
    g_resource_stats.gpu_bytes += gpu_bytes;
    return true;
}

bool hx_resource_unregister(void* resource) {
    if (!resource) return false;

    std::lock_guard<std::mutex> lock(g_resource_mutex);
    HxResourceNode** link = &g_resources;
    while (*link) {
        HxResourceNode* node = *link;
        if (node->resource == resource) {
            *link = node->next;
            --g_resource_stats.live_resources;
            g_resource_stats.cpu_bytes -= node->cpu_bytes;
            g_resource_stats.gpu_bytes -= node->gpu_bytes;
            free(node);
            return true;
        }
        link = &node->next;
    }
    return false;
}

bool hx_resource_is_registered(void* resource) {
    if (!resource) return false;
    std::lock_guard<std::mutex> lock(g_resource_mutex);
    for (HxResourceNode* node = g_resources; node; node = node->next) {
        if (node->resource == resource) return true;
    }
    return false;
}

bool hx_resource_resize(void* resource, size_t cpu_bytes, size_t gpu_bytes) {
    if (!resource) return false;

    std::lock_guard<std::mutex> lock(g_resource_mutex);
    for (HxResourceNode* node = g_resources; node; node = node->next) {
        if (node->resource != resource) continue;
        const size_t remaining_cpu = g_resource_stats.cpu_bytes - node->cpu_bytes;
        const size_t remaining_gpu = g_resource_stats.gpu_bytes - node->gpu_bytes;
        if (cpu_bytes > SIZE_MAX - remaining_cpu || gpu_bytes > SIZE_MAX - remaining_gpu) return false;
        g_resource_stats.cpu_bytes -= node->cpu_bytes;
        g_resource_stats.gpu_bytes -= node->gpu_bytes;
        node->cpu_bytes = cpu_bytes;
        node->gpu_bytes = gpu_bytes;
        g_resource_stats.cpu_bytes += cpu_bytes;
        g_resource_stats.gpu_bytes += gpu_bytes;
        return true;
    }
    return false;
}

void hx_resource_release_all(void) {
    HxResourceNode* resources;
    {
        std::lock_guard<std::mutex> lock(g_resource_mutex);
        resources = g_resources;
        g_resources = NULL;
        g_resource_stats = {0, 0, 0};
    }

    while (resources) {
        HxResourceNode* next = resources->next;
        resources->destroy(resources->resource);
        free(resources);
        resources = next;
    }
}

void hx_resource_get_stats(HxMemoryStats* out_stats) {
    if (!out_stats) return;
    std::lock_guard<std::mutex> lock(g_resource_mutex);
    *out_stats = g_resource_stats;
}
