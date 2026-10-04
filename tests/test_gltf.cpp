// test_gltf.cpp — Blender-compatible glTF static mesh import test
#include "helix.h"
#include <cstdio>
#include <cstring>

static bool write_fixture() {
    const char* gltf_path = "helix_test_model.gltf";
    const char* buffer_path = "helix_test_model.bin";
    const float positions[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    const uint16_t indices[] = {0, 1, 2};

    FILE* buffer_file = fopen(buffer_path, "wb");
    if (!buffer_file) return false;
    const bool wrote_buffer = fwrite(positions, sizeof(positions), 1, buffer_file) == 1 &&
                              fwrite(indices, sizeof(indices), 1, buffer_file) == 1;
    if (fclose(buffer_file) != 0 || !wrote_buffer) return false;

    FILE* gltf_file = fopen(gltf_path, "wb");
    if (!gltf_file) return false;
    const char gltf[] =
        "{\"asset\":{\"version\":\"2.0\"},"
        "\"buffers\":[{\"uri\":\"helix_test_model.bin\",\"byteLength\":42}],"
        "\"bufferViews\":["
        "{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36,\"target\":34962},"
        "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":6,\"target\":34963}],"
        "\"accessors\":["
        "{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\",\"min\":[0,0,0],\"max\":[1,1,0]},"
        "{\"bufferView\":1,\"componentType\":5123,\"count\":3,\"type\":\"SCALAR\"}],"
        "\"meshes\":[{\"name\":\"Triangle\",\"primitives\":[{\"attributes\":{\"POSITION\":0},\"indices\":1}]}],"
        "\"nodes\":[{\"mesh\":0}],\"scenes\":[{\"nodes\":[0]}],\"scene\":0}";
    const bool wrote_gltf = fwrite(gltf, sizeof(gltf) - 1, 1, gltf_file) == 1;
    return fclose(gltf_file) == 0 && wrote_gltf;
}

static void remove_fixture() {
    remove("helix_test_model.gltf");
    remove("helix_test_model.bin");
}

int main() {
    if (!write_fixture()) {
        remove_fixture();
        return 1;
    }

    HxModel model = hx_load_model("helix_test_model.gltf");
    remove_fixture();
    if (!model) return 2;
    if (hx_get_model_mesh_count(model) != 1 || hx_get_model_primitive_count(model, 0) != 1) return 3;

    HxModelPrimitiveView view{};
    if (hx_get_model_primitive(model, 0, 0, &view) != HX_OK) return 4;
    if (!view.mesh_name || strcmp(view.mesh_name, "Triangle") != 0) return 5;
    if (view.primitive != HX_PRIM_TRIANGLES || view.vertex_count != 3 || view.index_count != 3) return 6;
    if (!view.positions || view.normals || view.uv0 || !view.indices) return 7;
    if (view.positions[3] != 1.0f || view.positions[7] != 1.0f) return 8;
    if (view.indices[0] != 0 || view.indices[1] != 1 || view.indices[2] != 2) return 9;

    HxMemoryStats stats{};
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 1 || stats.cpu_bytes == 0) return 10;
    if (hx_drop_model(model) != HX_OK) return 11;
    if (hx_drop_model(model) != HX_ERR_ALREADY_DROPPED) return 12;
    hx_get_memory_stats(&stats);
    if (stats.live_resources != 0 || stats.cpu_bytes != 0) return 13;

    std::puts("test_gltf: PASS");
    return 0;
}
