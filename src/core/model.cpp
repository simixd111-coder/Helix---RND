// model.cpp — CPU-side glTF/GLB mesh import for custom renderers
#include "helix.h"
#include "resource_internal.h"
#define CGLTF_IMPLEMENTATION
#define CGLTF_VALIDATE_ENABLE_ASSERTS 0
#include <cgltf.h>
#include <limits>
#include <new>
#include <string>
#include <vector>

struct HxImportedPrimitive {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> uv0;
    std::vector<uint32_t> indices;
    HxPrim topology;
};

struct HxImportedMesh {
    std::string name;
    std::vector<HxImportedPrimitive> primitives;
};

struct HxModelImpl {
    std::vector<HxImportedMesh> meshes;
};

static bool hx_unpack_attribute(const cgltf_accessor* accessor, size_t components, std::vector<float>& values) {
    if (!accessor || accessor->count > std::numeric_limits<size_t>::max() / components) return false;
    const size_t float_count = accessor->count * components;
    values.resize(float_count);
    return cgltf_accessor_unpack_floats(accessor, values.data(), float_count) == float_count;
}

static bool hx_unpack_indices(const cgltf_primitive& source, size_t vertex_count, HxImportedPrimitive& target) {
    const cgltf_size source_count = source.indices ? source.indices->count : static_cast<cgltf_size>(vertex_count);
    std::vector<uint32_t> raw_indices(static_cast<size_t>(source_count));
    for (cgltf_size index = 0; index < source_count; ++index) {
        const cgltf_size value = source.indices ? cgltf_accessor_read_index(source.indices, index) : index;
        if (value >= vertex_count || value > std::numeric_limits<uint32_t>::max()) return false;
        raw_indices[static_cast<size_t>(index)] = static_cast<uint32_t>(value);
    }

    switch (source.type) {
        case cgltf_primitive_type_triangles:
            target.topology = HX_PRIM_TRIANGLES;
            target.indices = source.indices ? std::move(raw_indices) : std::vector<uint32_t>{};
            return true;
        case cgltf_primitive_type_triangle_strip:
            target.topology = HX_PRIM_TRI_STRIP;
            target.indices = source.indices ? std::move(raw_indices) : std::vector<uint32_t>{};
            return true;
        case cgltf_primitive_type_points:
            target.topology = HX_PRIM_POINTS;
            target.indices = source.indices ? std::move(raw_indices) : std::vector<uint32_t>{};
            return true;
        case cgltf_primitive_type_lines:
            target.topology = HX_PRIM_LINES;
            target.indices = source.indices ? std::move(raw_indices) : std::vector<uint32_t>{};
            return true;
        case cgltf_primitive_type_line_strip:
        case cgltf_primitive_type_line_loop: {
            target.topology = HX_PRIM_LINES;
            const size_t segment_count = raw_indices.size() > 1 ? raw_indices.size() - 1 : 0;
            target.indices.reserve((segment_count + (source.type == cgltf_primitive_type_line_loop && segment_count ? 1 : 0)) * 2);
            for (size_t index = 0; index < segment_count; ++index) {
                target.indices.push_back(raw_indices[index]);
                target.indices.push_back(raw_indices[index + 1]);
            }
            if (source.type == cgltf_primitive_type_line_loop && segment_count) {
                target.indices.push_back(raw_indices.back());
                target.indices.push_back(raw_indices.front());
            }
            return true;
        }
        case cgltf_primitive_type_triangle_fan: {
            target.topology = HX_PRIM_TRIANGLES;
            if (raw_indices.size() < 3) return false;
            target.indices.reserve((raw_indices.size() - 2) * 3);
            for (size_t index = 1; index + 1 < raw_indices.size(); ++index) {
                target.indices.push_back(raw_indices[0]);
                target.indices.push_back(raw_indices[index]);
                target.indices.push_back(raw_indices[index + 1]);
            }
            return true;
        }
        default:
            return false;
    }
}

static size_t hx_model_cpu_bytes(const HxModelImpl& model) {
    size_t bytes = sizeof(HxModelImpl);
    for (const HxImportedMesh& mesh : model.meshes) {
        bytes += mesh.name.size();
        bytes += mesh.primitives.capacity() * sizeof(HxImportedPrimitive);
        for (const HxImportedPrimitive& primitive : mesh.primitives) {
            bytes += primitive.positions.capacity() * sizeof(float);
            bytes += primitive.normals.capacity() * sizeof(float);
            bytes += primitive.uv0.capacity() * sizeof(float);
            bytes += primitive.indices.capacity() * sizeof(uint32_t);
        }
    }
    return bytes;
}

static void hx_model_destroy_resource(void* resource) {
    delete static_cast<HxModel>(resource);
}

HX_API HxModel HX_CALL hx_load_model(const char* path) {
    if (!path || !path[0]) return NULL;

    cgltf_options options{};
    cgltf_data* data = NULL;
    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success) return NULL;
    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success ||
        cgltf_validate(data) != cgltf_result_success) {
        cgltf_free(data);
        return NULL;
    }

    HxModel model = new (std::nothrow) HxModelImpl{};
    if (!model) {
        cgltf_free(data);
        return NULL;
    }

    try {
        model->meshes.reserve(data->meshes_count);
        for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index) {
            const cgltf_mesh& source_mesh = data->meshes[mesh_index];
            HxImportedMesh mesh;
            if (source_mesh.name) mesh.name = source_mesh.name;
            mesh.primitives.reserve(source_mesh.primitives_count);

            for (cgltf_size primitive_index = 0; primitive_index < source_mesh.primitives_count; ++primitive_index) {
                const cgltf_primitive& source = source_mesh.primitives[primitive_index];
                const cgltf_accessor* position = cgltf_find_accessor(&source, cgltf_attribute_type_position, 0);
                if (!position || position->type != cgltf_type_vec3) {
                    cgltf_free(data);
                    delete model;
                    return NULL;
                }

                HxImportedPrimitive primitive{};
                const size_t vertex_count = static_cast<size_t>(position->count);
                if (!hx_unpack_attribute(position, 3, primitive.positions)) {
                    cgltf_free(data);
                    delete model;
                    return NULL;
                }

                const cgltf_accessor* normal = cgltf_find_accessor(&source, cgltf_attribute_type_normal, 0);
                if (normal && (normal->type != cgltf_type_vec3 || normal->count != position->count ||
                               !hx_unpack_attribute(normal, 3, primitive.normals))) {
                    cgltf_free(data);
                    delete model;
                    return NULL;
                }

                const cgltf_accessor* uv0 = cgltf_find_accessor(&source, cgltf_attribute_type_texcoord, 0);
                if (uv0 && (uv0->type != cgltf_type_vec2 || uv0->count != position->count ||
                            !hx_unpack_attribute(uv0, 2, primitive.uv0))) {
                    cgltf_free(data);
                    delete model;
                    return NULL;
                }

                if (!hx_unpack_indices(source, vertex_count, primitive)) {
                    cgltf_free(data);
                    delete model;
                    return NULL;
                }
                mesh.primitives.push_back(std::move(primitive));
            }
            model->meshes.push_back(std::move(mesh));
        }
    } catch (...) {
        cgltf_free(data);
        delete model;
        return NULL;
    }

    cgltf_free(data);
    if (!hx_resource_register(model, hx_model_cpu_bytes(*model), 0, hx_model_destroy_resource)) {
        delete model;
        return NULL;
    }
    return model;
}

HX_API size_t HX_CALL hx_get_model_mesh_count(HxModel model) {
    return model ? model->meshes.size() : 0;
}

HX_API size_t HX_CALL hx_get_model_primitive_count(HxModel model, size_t mesh_index) {
    if (!model || mesh_index >= model->meshes.size()) return 0;
    return model->meshes[mesh_index].primitives.size();
}

HX_API HxResult HX_CALL hx_get_model_primitive(
    HxModel model,
    size_t mesh_index,
    size_t primitive_index,
    HxModelPrimitiveView* out_view
) {
    if (!model || !out_view) return HX_ERR_INVALID_ARG;
    if (mesh_index >= model->meshes.size() || primitive_index >= model->meshes[mesh_index].primitives.size()) {
        return HX_ERR_INVALID_ARG;
    }

    const HxImportedMesh& mesh = model->meshes[mesh_index];
    const HxImportedPrimitive& primitive = mesh.primitives[primitive_index];
    out_view->mesh_name = mesh.name.c_str();
    out_view->positions = primitive.positions.empty() ? NULL : primitive.positions.data();
    out_view->normals = primitive.normals.empty() ? NULL : primitive.normals.data();
    out_view->uv0 = primitive.uv0.empty() ? NULL : primitive.uv0.data();
    out_view->vertex_count = primitive.positions.size() / 3;
    out_view->indices = primitive.indices.empty() ? NULL : primitive.indices.data();
    out_view->index_count = primitive.indices.size();
    out_view->primitive = primitive.topology;
    return HX_OK;
}

HX_API HxResult HX_CALL hx_drop_model(HxModel model) {
    if (!model) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(model)) return HX_ERR_ALREADY_DROPPED;
    delete model;
    return HX_OK;
}
