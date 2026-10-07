// gpu_vulkan.cpp — Runtime Vulkan device discovery without a build-time SDK
#include "helix.h"
#include "gpu_vulkan_internal.h"
#include "resource_internal.h"
#include <volk.h>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <vector>

static std::mutex g_vulkan_probe_mutex;

struct HxVulkanContext {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family;
    VkPhysicalDeviceMemoryProperties memory_properties;
    bool active;
};

struct HxBufferImpl {
    VkBuffer buffer;
    VkDeviceMemory memory;
    size_t size;
    size_t gpu_allocation_bytes;
};

static HxVulkanContext g_vulkan = {};

static HxGpuDeviceType hx_gpu_device_type(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return HX_GPU_DEVICE_INTEGRATED;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return HX_GPU_DEVICE_DISCRETE;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return HX_GPU_DEVICE_VIRTUAL;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return HX_GPU_DEVICE_CPU;
        default: return HX_GPU_DEVICE_OTHER;
    }
}

static HxResult hx_gpu_probe_devices(std::vector<HxGpuDeviceInfo>& devices) {
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    const bool temporary_instance = !g_vulkan.active;
    if (temporary_instance && volkInitialize() != VK_SUCCESS) return HX_ERR_BACKEND_UNAVAILABLE;

    VkInstance instance = g_vulkan.instance;
    if (temporary_instance) {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Helix RND device query";
        app_info.applicationVersion = VK_MAKE_API_VERSION(0, HX_VERSION_MAJOR, HX_VERSION_MINOR, HX_VERSION_PATCH);
        app_info.pEngineName = "Helix RND";
        app_info.engineVersion = app_info.applicationVersion;
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;

        if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS) {
            volkFinalize();
            return HX_ERR_BACKEND_UNAVAILABLE;
        }
        volkLoadInstance(instance);
    }

    auto cleanup_temporary_instance = [&]() {
        if (temporary_instance) {
            vkDestroyInstance(instance, NULL);
            volkFinalize();
        }
    };

    uint32_t physical_count = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &physical_count, NULL);
    if (result != VK_SUCCESS || physical_count == 0) {
        cleanup_temporary_instance();
        return result == VK_SUCCESS ? HX_OK : HX_ERR_BACKEND_UNAVAILABLE;
    }

    std::vector<VkPhysicalDevice> physical_devices(physical_count);
    result = vkEnumeratePhysicalDevices(instance, &physical_count, physical_devices.data());
    if (result != VK_SUCCESS) {
        cleanup_temporary_instance();
        return HX_ERR_BACKEND_UNAVAILABLE;
    }

    devices.clear();
    devices.reserve(physical_count);
    for (VkPhysicalDevice physical_device : physical_devices) {
        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceMemoryProperties memory_properties{};
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

        HxGpuDeviceInfo device{};
        std::strncpy(device.name, properties.deviceName, sizeof(device.name) - 1);
        device.vendor_id = properties.vendorID;
        device.device_id = properties.deviceID;
        device.type = hx_gpu_device_type(properties.deviceType);
        device.api_version = properties.apiVersion;

        for (uint32_t heap_index = 0; heap_index < memory_properties.memoryHeapCount; ++heap_index) {
            const VkMemoryHeap& heap = memory_properties.memoryHeaps[heap_index];
            if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0 &&
                heap.size <= UINT64_MAX - device.local_memory_bytes) {
                device.local_memory_bytes += heap.size;
            }
        }

        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
        std::vector<VkQueueFamilyProperties> queues(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queues.data());
        for (const VkQueueFamilyProperties& queue : queues) {
            if (queue.queueCount == 0) continue;
            device.graphics_queue = device.graphics_queue || (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
            device.compute_queue = device.compute_queue || (queue.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
        }
        devices.push_back(device);
    }

    cleanup_temporary_instance();
    return HX_OK;
}

HxResult hx_vulkan_start(HxGpuPreference preference, uint32_t requested_index) {
    if (preference > HX_GPU_PREFERENCE_LOW_POWER) return HX_ERR_INVALID_ARG;
    {
        std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
        if (g_vulkan.active) return HX_ERR_ALREADY_BOOTED;
    }

    std::vector<HxGpuDeviceInfo> devices;
    HxResult probe_result = hx_gpu_probe_devices(devices);
    if (probe_result != HX_OK || devices.empty()) return HX_ERR_BACKEND_UNAVAILABLE;

    size_t selected_index = 0;
    if (requested_index == HX_GPU_DEVICE_DEFAULT) {
        if (hx_pick_gpu_device(devices.data(), devices.size(), preference, &selected_index) != HX_OK) {
            return HX_ERR_BACKEND_UNAVAILABLE;
        }
    } else {
        selected_index = static_cast<size_t>(requested_index - 1u);
    }
    if (selected_index >= devices.size() || !devices[selected_index].graphics_queue) {
        return HX_ERR_UNSUPPORTED;
    }

    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (g_vulkan.active) return HX_ERR_ALREADY_BOOTED;
    if (volkInitialize() != VK_SUCCESS) return HX_ERR_BACKEND_UNAVAILABLE;

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Helix RND";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, HX_VERSION_MAJOR, HX_VERSION_MINOR, HX_VERSION_PATCH);
    app_info.pEngineName = "Helix RND";
    app_info.engineVersion = app_info.applicationVersion;
    app_info.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS) {
        volkFinalize();
        return HX_ERR_BACKEND_UNAVAILABLE;
    }
    volkLoadInstance(instance);

    uint32_t physical_count = 0;
    if (vkEnumeratePhysicalDevices(instance, &physical_count, NULL) != VK_SUCCESS || selected_index >= physical_count) {
        vkDestroyInstance(instance, NULL);
        volkFinalize();
        return HX_ERR_BACKEND_UNAVAILABLE;
    }
    std::vector<VkPhysicalDevice> physical_devices(physical_count);
    if (vkEnumeratePhysicalDevices(instance, &physical_count, physical_devices.data()) != VK_SUCCESS) {
        vkDestroyInstance(instance, NULL);
        volkFinalize();
        return HX_ERR_BACKEND_UNAVAILABLE;
    }
    VkPhysicalDevice physical_device = physical_devices[selected_index];

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queues.data());
    uint32_t queue_family = UINT32_MAX;
    for (uint32_t index = 0; index < queue_count; ++index) {
        if (queues[index].queueCount > 0 && (queues[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            queue_family = index;
            break;
        }
    }
    if (queue_family == UINT32_MAX) {
        vkDestroyInstance(instance, NULL);
        volkFinalize();
        return HX_ERR_UNSUPPORTED;
    }

    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;
    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;

    VkDevice device = VK_NULL_HANDLE;
    if (vkCreateDevice(physical_device, &device_info, NULL, &device) != VK_SUCCESS) {
        vkDestroyInstance(instance, NULL);
        volkFinalize();
        return HX_ERR_BACKEND_UNAVAILABLE;
    }
    volkLoadDevice(device);

    g_vulkan.instance = instance;
    g_vulkan.physical_device = physical_device;
    g_vulkan.device = device;
    g_vulkan.queue_family = queue_family;
    g_vulkan.active = true;
    vkGetDeviceQueue(device, queue_family, 0, &g_vulkan.queue);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &g_vulkan.memory_properties);
    return HX_OK;
}

void hx_vulkan_stop(void) {
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (!g_vulkan.active) return;
    vkDeviceWaitIdle(g_vulkan.device);
    vkDestroyDevice(g_vulkan.device, NULL);
    vkDestroyInstance(g_vulkan.instance, NULL);
    g_vulkan = {};
    volkFinalize();
}

bool hx_vulkan_is_active(void) {
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    return g_vulkan.active;
}

static uint32_t hx_vulkan_find_memory_type(uint32_t type_bits, VkMemoryPropertyFlags required) {
    for (uint32_t index = 0; index < g_vulkan.memory_properties.memoryTypeCount; ++index) {
        const bool allowed = (type_bits & (1u << index)) != 0;
        const VkMemoryPropertyFlags properties = g_vulkan.memory_properties.memoryTypes[index].propertyFlags;
        if (allowed && (properties & required) == required) return index;
    }
    return UINT32_MAX;
}

static void hx_vulkan_destroy_buffer(void* resource) {
    HxBuffer buffer = static_cast<HxBuffer>(resource);
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (g_vulkan.active) {
        if (buffer->buffer != VK_NULL_HANDLE) vkDestroyBuffer(g_vulkan.device, buffer->buffer, NULL);
        if (buffer->memory != VK_NULL_HANDLE) vkFreeMemory(g_vulkan.device, buffer->memory, NULL);
    }
    free(buffer);
}

HX_API HxBuffer HX_CALL hx_make_buffer(size_t size, HxBufferFlags flags, const void* initial_data) {
    if (size == 0) return NULL;

    HxBuffer buffer = static_cast<HxBuffer>(calloc(1, sizeof(HxBufferImpl)));
    if (!buffer) return NULL;
    buffer->size = size;

    {
        std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
        if (!g_vulkan.active) {
            free(buffer);
            return NULL;
        }

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if ((flags & HX_BUF_VERTEX) != 0) usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if ((flags & HX_BUF_INDEX) != 0) usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if ((flags & HX_BUF_UNIFORM) != 0) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if ((flags & HX_BUF_STORAGE) != 0) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (usage == 0) usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = static_cast<VkDeviceSize>(size);
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(g_vulkan.device, &buffer_info, NULL, &buffer->buffer) != VK_SUCCESS) {
            free(buffer);
            return NULL;
        }

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(g_vulkan.device, buffer->buffer, &requirements);
        if (requirements.size > SIZE_MAX) {
            vkDestroyBuffer(g_vulkan.device, buffer->buffer, NULL);
            free(buffer);
            return NULL;
        }
        buffer->gpu_allocation_bytes = static_cast<size_t>(requirements.size);
        const uint32_t memory_type = hx_vulkan_find_memory_type(
            requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        if (memory_type == UINT32_MAX) {
            vkDestroyBuffer(g_vulkan.device, buffer->buffer, NULL);
            free(buffer);
            return NULL;
        }

        VkMemoryAllocateInfo allocation_info{};
        allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocation_info.allocationSize = requirements.size;
        allocation_info.memoryTypeIndex = memory_type;
        if (vkAllocateMemory(g_vulkan.device, &allocation_info, NULL, &buffer->memory) != VK_SUCCESS ||
            vkBindBufferMemory(g_vulkan.device, buffer->buffer, buffer->memory, 0) != VK_SUCCESS) {
            if (buffer->memory != VK_NULL_HANDLE) vkFreeMemory(g_vulkan.device, buffer->memory, NULL);
            vkDestroyBuffer(g_vulkan.device, buffer->buffer, NULL);
            free(buffer);
            return NULL;
        }
    }

    if (!hx_resource_register(buffer, sizeof(HxBufferImpl), buffer->gpu_allocation_bytes, hx_vulkan_destroy_buffer)) {
        hx_vulkan_destroy_buffer(buffer);
        return NULL;
    }

    if (initial_data) hx_write_buffer(buffer, 0, size, initial_data);
    return buffer;
}

HX_API void HX_CALL hx_write_buffer(HxBuffer buffer, size_t offset, size_t size, const void* data) {
    if (!buffer || !data || size == 0 || offset > buffer->size || size > buffer->size - offset) return;
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (!g_vulkan.active) return;
    void* mapped = NULL;
    if (vkMapMemory(g_vulkan.device, buffer->memory, static_cast<VkDeviceSize>(offset),
                    static_cast<VkDeviceSize>(size), 0, &mapped) != VK_SUCCESS) return;
    std::memcpy(mapped, data, size);
    vkUnmapMemory(g_vulkan.device, buffer->memory);
}

HX_API void HX_CALL hx_read_buffer(HxBuffer buffer, size_t offset, size_t size, void* data) {
    if (!buffer || !data || size == 0 || offset > buffer->size || size > buffer->size - offset) return;
    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (!g_vulkan.active) return;
    void* mapped = NULL;
    if (vkMapMemory(g_vulkan.device, buffer->memory, static_cast<VkDeviceSize>(offset),
                    static_cast<VkDeviceSize>(size), 0, &mapped) != VK_SUCCESS) return;
    std::memcpy(data, mapped, size);
    vkUnmapMemory(g_vulkan.device, buffer->memory);
}

HX_API HxResult HX_CALL hx_fill_buffer(HxBuffer buffer, uint32_t value) {
    if (!buffer) return HX_ERR_INVALID_HANDLE;
    if (buffer->size == 0 || buffer->size % sizeof(uint32_t) != 0) return HX_ERR_INVALID_ARG;

    std::lock_guard<std::mutex> lock(g_vulkan_probe_mutex);
    if (!g_vulkan.active) return HX_ERR_INVALID_STATE;

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = g_vulkan.queue_family;
    VkCommandPool pool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(g_vulkan.device, &pool_info, NULL, &pool) != VK_SUCCESS) return HX_ERR_OOM;

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;
    VkCommandBuffer command = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(g_vulkan.device, &allocate_info, &command) != VK_SUCCESS) {
        vkDestroyCommandPool(g_vulkan.device, pool, NULL);
        return HX_ERR_OOM;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult result = vkBeginCommandBuffer(command, &begin_info);
    if (result == VK_SUCCESS) {
        vkCmdFillBuffer(command, buffer->buffer, 0, static_cast<VkDeviceSize>(buffer->size), value);
        VkBufferMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = buffer->buffer;
        barrier.offset = 0;
        barrier.size = static_cast<VkDeviceSize>(buffer->size);
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                             0, 0, NULL, 1, &barrier, 0, NULL);
        result = vkEndCommandBuffer(command);
    }

    if (result == VK_SUCCESS) {
        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command;
        result = vkQueueSubmit(g_vulkan.queue, 1, &submit, VK_NULL_HANDLE);
        if (result == VK_SUCCESS) result = vkQueueWaitIdle(g_vulkan.queue);
    }

    vkFreeCommandBuffers(g_vulkan.device, pool, 1, &command);
    vkDestroyCommandPool(g_vulkan.device, pool, NULL);
    return result == VK_SUCCESS ? HX_OK : HX_ERR_INVALID_STATE;
}

HX_API HxResult HX_CALL hx_drop_buffer(HxBuffer buffer) {
    if (!buffer) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(buffer)) return HX_ERR_ALREADY_DROPPED;
    hx_vulkan_destroy_buffer(buffer);
    return HX_OK;
}

HX_API HxResult HX_CALL hx_get_gpu_devices(HxGpuDeviceInfo* devices, size_t capacity, size_t* out_count) {
    if (!out_count || (capacity > 0 && !devices)) return HX_ERR_INVALID_ARG;

    std::vector<HxGpuDeviceInfo> detected;
    HxResult result = hx_gpu_probe_devices(detected);
    *out_count = detected.size();
    if (result != HX_OK) return result;

    const size_t copy_count = std::min(capacity, detected.size());
    if (copy_count > 0) std::memcpy(devices, detected.data(), copy_count * sizeof(HxGpuDeviceInfo));
    return HX_OK;
}

// hx_pick_gpu_device lives in gpu_select.cpp: it is a pure selection policy
// over a caller-supplied device list and must not depend on the Vulkan backend.
