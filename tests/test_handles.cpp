// test_handles.cpp — Handle system tests
#include "helix.h"
#include <cassert>
#include <cstdio>

#define HX_HANDLE_TAG_WIN  0x57494E00u
#define HX_HANDLE_TAG_MESH 0x4D455300u

uint64_t hx_handle_make(uint32_t tag);
uint32_t hx_handle_tag(uint64_t handle);
uint32_t hx_handle_index(uint64_t handle);
bool hx_handle_valid(uint64_t handle, uint32_t expected_tag);
void* hx_handle_alloc(uint32_t tag, void* ptr);
void* hx_handle_get(uint64_t handle, uint32_t expected_tag);
void hx_handle_free(uint64_t handle, uint32_t expected_tag);

int main() {
    printf("Testing handle system...\n");

    // Test handle creation
    uint64_t h1 = hx_handle_make(HX_HANDLE_TAG_WIN);
    uint64_t h2 = hx_handle_make(HX_HANDLE_TAG_WIN);
    uint64_t h3 = hx_handle_make(HX_HANDLE_TAG_MESH);

    assert(h1 != 0);
    assert(h2 != 0);
    assert(h3 != 0);
    assert(h1 != h2);
    assert(hx_handle_tag(h1) == HX_HANDLE_TAG_WIN);
    assert(hx_handle_tag(h3) == HX_HANDLE_TAG_MESH);
    assert(hx_handle_index(h1) == 1);
    assert(hx_handle_index(h2) == 2);
    assert(hx_handle_index(h3) == 3);

    // Test handle validation
    assert(hx_handle_valid(h1, HX_HANDLE_TAG_WIN));
    assert(!hx_handle_valid(h1, HX_HANDLE_TAG_MESH));
    assert(!hx_handle_valid(0, HX_HANDLE_TAG_WIN));

    // Test handle allocation
    int test_value = 42;
    void* handle = hx_handle_alloc(HX_HANDLE_TAG_WIN, &test_value);
    assert(handle != NULL);
    assert(hx_handle_valid((uint64_t)handle, HX_HANDLE_TAG_WIN));

    // Test handle retrieval
    void* retrieved = hx_handle_get((uint64_t)handle, HX_HANDLE_TAG_WIN);
    assert(retrieved == &test_value);
    assert(*(int*)retrieved == 42);

    // Test wrong tag
    retrieved = hx_handle_get((uint64_t)handle, HX_HANDLE_TAG_MESH);
    assert(retrieved == NULL);

    // Test handle free
    hx_handle_free((uint64_t)handle, HX_HANDLE_TAG_WIN);
    retrieved = hx_handle_get((uint64_t)handle, HX_HANDLE_TAG_WIN);
    assert(retrieved == NULL);

    printf("All handle tests PASSED\n");
    return 0;
}