#include "helix.h"
#include "core/render_internal.h"
#include "resource_internal.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <vector>

struct HxMeshImpl
{
    std::vector<HxVec3> positions;
    std::vector<HxVec2> texcoords;
    std::vector<HxVec4> colors;
    std::vector<uint32_t> indices;
    HxPrim topology = HX_PRIM_TRIANGLES;
    HxVec3 position{0.0f, 0.0f, 0.0f};
    HxVec3 rotation{0.0f, 0.0f, 0.0f};
    HxVec3 scale{1.0f, 1.0f, 1.0f};
};

struct HxSkinImpl
{
    HxColor color;
    HxSkinFlags flags;
    HxTex texture;
};

static void hx_mesh_destroy_resource(void* resource) { delete static_cast<HxMesh>(resource); }
static void hx_skin_destroy_resource(void* resource) { delete static_cast<HxSkin>(resource); }

static bool hx_mesh_memory_bytes(const HxMeshImpl& mesh, size_t* out_bytes)
{
    size_t total = sizeof(HxMeshImpl);
    const size_t capacities[] = {mesh.positions.capacity(), mesh.texcoords.capacity(), mesh.colors.capacity(),
                                 mesh.indices.capacity()};
    const size_t element_sizes[] = {sizeof(HxVec3), sizeof(HxVec2), sizeof(HxVec4), sizeof(uint32_t)};
    for (size_t i = 0; i < 4; ++i)
    {
        if (capacities[i] > (std::numeric_limits<size_t>::max() - total) / element_sizes[i])
            return false;
        total += capacities[i] * element_sizes[i];
    }
    *out_bytes = total;
    return true;
}

static size_t hx_append_field(size_t& offset, size_t bytes)
{
    const size_t result = offset;
    offset += bytes;
    return result;
}

HX_API HxMesh HX_CALL hx_make_mesh(const void* vertices, size_t vertex_count, HxVertexFlags format,
                                   size_t vertex_stride, const void* indices, size_t index_count, bool index32,
                                   HxPrim prim)
{
    const HxVertexFlags known = HX_VERT_POS | HX_VERT_NORMAL | HX_VERT_TANGENT | HX_VERT_UV0 | HX_VERT_UV1 |
                                HX_VERT_COLOR | HX_VERT_JOINTS | HX_VERT_WEIGHTS;
    if (!vertices || vertex_count == 0 || !(format & HX_VERT_POS) || (format & ~known) ||
        (index_count != 0 && !indices) || (prim != HX_PRIM_TRIANGLES && prim != HX_PRIM_TRI_STRIP))
        return NULL;

    size_t offset = 0;
    size_t position_offset = 0;
    size_t texcoord_offset = 0;
    size_t color_offset = 0;
    position_offset = hx_append_field(offset, (format & HX_VERT_POS) ? 3u * sizeof(float) : 0u);
    if (format & HX_VERT_NORMAL)
        hx_append_field(offset, 3u * sizeof(float));
    if (format & HX_VERT_TANGENT)
        hx_append_field(offset, 4u * sizeof(float));
    if (format & HX_VERT_UV0)
        texcoord_offset = hx_append_field(offset, 2u * sizeof(float));
    if (format & HX_VERT_UV1)
        hx_append_field(offset, 2u * sizeof(float));
    if (format & HX_VERT_COLOR)
        color_offset = hx_append_field(offset, 4u * sizeof(float));
    if (format & HX_VERT_JOINTS)
        hx_append_field(offset, 4u * sizeof(uint32_t));
    if (format & HX_VERT_WEIGHTS)
        hx_append_field(offset, 4u * sizeof(float));
    const size_t stride = vertex_stride == 0 ? offset : vertex_stride;
    if (stride < offset || vertex_count > std::numeric_limits<size_t>::max() / stride)
        return NULL;
    const size_t element_count = index_count == 0 ? vertex_count : index_count;
    if ((prim == HX_PRIM_TRIANGLES && element_count % 3u != 0) || (prim == HX_PRIM_TRI_STRIP && element_count < 3u))
        return NULL;

    HxMesh mesh = new (std::nothrow) HxMeshImpl{};
    if (!mesh)
        return NULL;
    try
    {
        mesh->positions.resize(vertex_count);
        if (format & HX_VERT_UV0)
            mesh->texcoords.resize(vertex_count);
        if (format & HX_VERT_COLOR)
            mesh->colors.resize(vertex_count);
        const auto* bytes = static_cast<const uint8_t*>(vertices);
        for (size_t i = 0; i < vertex_count; ++i)
        {
            const uint8_t* vertex = bytes + i * stride;
            std::memcpy(&mesh->positions[i], vertex + position_offset, sizeof(HxVec3));
            if (format & HX_VERT_UV0)
                std::memcpy(&mesh->texcoords[i], vertex + texcoord_offset, sizeof(HxVec2));
            if (format & HX_VERT_COLOR)
                std::memcpy(&mesh->colors[i], vertex + color_offset, sizeof(HxVec4));
        }
        if (index_count)
        {
            mesh->indices.resize(index_count);
            const auto* index_bytes = static_cast<const uint8_t*>(indices);
            for (size_t i = 0; i < index_count; ++i)
            {
                uint32_t value;
                if (index32)
                    std::memcpy(&value, index_bytes + i * sizeof(uint32_t), sizeof(uint32_t));
                else
                {
                    uint16_t index16;
                    std::memcpy(&index16, index_bytes + i * sizeof(uint16_t), sizeof(uint16_t));
                    value = index16;
                }
                if (value >= vertex_count)
                {
                    delete mesh;
                    return NULL;
                }
                mesh->indices[i] = value;
            }
        }
        mesh->topology = prim;
    }
    catch (...)
    {
        delete mesh;
        return NULL;
    }
    size_t memory_bytes = 0;
    if (!hx_mesh_memory_bytes(*mesh, &memory_bytes) ||
        !hx_resource_register(mesh, sizeof(HxMeshImpl), 0, hx_mesh_destroy_resource) ||
        !hx_resource_resize(mesh, memory_bytes, 0))
    {
        (void)hx_resource_unregister(mesh);
        delete mesh;
        return NULL;
    }
    return mesh;
}

HX_API HxResult HX_CALL hx_drop_mesh(HxMesh mesh)
{
    if (!mesh)
        return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(mesh))
        return HX_ERR_ALREADY_DROPPED;
    delete mesh;
    return HX_OK;
}

HX_API void HX_CALL hx_move_mesh(HxMesh mesh, float x, float y, float z)
{
    if (mesh)
        mesh->position = {x, y, z};
}
HX_API void HX_CALL hx_spin_mesh(HxMesh mesh, float x, float y, float z)
{
    if (mesh)
        mesh->rotation = {x, y, z};
}
HX_API void HX_CALL hx_size_mesh(HxMesh mesh, float x, float y, float z)
{
    if (mesh)
        mesh->scale = {x, y, z};
}

HX_API HxMesh HX_CALL hx_make_cube(HxSkin skin)
{
    (void)skin;
    const HxVec3 vertices[] = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
                               {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f}};
    const uint16_t indices[] = {0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 5, 4,
                                2, 3, 7, 2, 7, 6, 0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5};
    return hx_make_mesh(vertices, 8, HX_VERT_POS, 0, indices, 36, false, HX_PRIM_TRIANGLES);
}

static HxMesh hx_make_flat_quad(float width, float height)
{
    const HxVec3 vertices[] = {{-width / 2, -height / 2, 0},
                               {width / 2, -height / 2, 0},
                               {width / 2, height / 2, 0},
                               {-width / 2, height / 2, 0}};
    const uint16_t indices[] = {0, 1, 2, 0, 2, 3};
    return hx_make_mesh(vertices, 4, HX_VERT_POS, 0, indices, 6, false, HX_PRIM_TRIANGLES);
}
HX_API HxMesh HX_CALL hx_make_plane(HxSkin skin, float w, float h)
{
    (void)skin;
    if (!std::isfinite(w) || !std::isfinite(h) || w <= 0.0f || h <= 0.0f)
        return NULL;
    const HxVec3 vertices[] = {{-w / 2, 0, -h / 2}, {w / 2, 0, -h / 2}, {w / 2, 0, h / 2}, {-w / 2, 0, h / 2}};
    const uint16_t indices[] = {0, 2, 1, 0, 3, 2};
    return hx_make_mesh(vertices, 4, HX_VERT_POS, 0, indices, 6, false, HX_PRIM_TRIANGLES);
}
HX_API HxMesh HX_CALL hx_make_quad(HxSkin skin)
{
    (void)skin;
    return hx_make_flat_quad(1.0f, 1.0f);
}

HX_API HxMesh HX_CALL hx_make_sphere(HxSkin skin, int rings, int sectors)
{
    (void)skin;
    if (rings < 2 || sectors < 3)
        return NULL;
    const size_t ring_count = static_cast<size_t>(rings) + 1u;
    const size_t row = static_cast<size_t>(sectors) + 1u;
    if (ring_count > std::numeric_limits<size_t>::max() / row)
        return NULL;
    const size_t vertex_count = ring_count * row;
    if (vertex_count > UINT32_MAX ||
        static_cast<size_t>(rings) > std::numeric_limits<size_t>::max() / 6u / static_cast<size_t>(sectors))
        return NULL;
    try
    {
        std::vector<HxVec3> vertices(vertex_count);
        std::vector<uint32_t> indices;
        indices.reserve(static_cast<size_t>(rings) * static_cast<size_t>(sectors) * 6u);
        const float pi = 3.14159265358979323846f;
        for (int ring = 0; ring <= rings; ++ring)
        {
            const float phi = pi * static_cast<float>(ring) / static_cast<float>(rings);
            for (int sector = 0; sector <= sectors; ++sector)
            {
                const float theta = 2.0f * pi * static_cast<float>(sector) / static_cast<float>(sectors);
                vertices[static_cast<size_t>(ring) * row + static_cast<size_t>(sector)] = {
                    std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta)};
            }
        }
        for (int ring = 0; ring < rings; ++ring)
            for (int sector = 0; sector < sectors; ++sector)
            {
                const uint32_t a = static_cast<uint32_t>(static_cast<size_t>(ring) * row + static_cast<size_t>(sector));
                const uint32_t b = a + static_cast<uint32_t>(row);
                indices.insert(indices.end(), {a, b, a + 1u, a + 1u, b, b + 1u});
            }
        return hx_make_mesh(vertices.data(), vertices.size(), HX_VERT_POS, sizeof(HxVec3), indices.data(),
                            indices.size(), true, HX_PRIM_TRIANGLES);
    }
    catch (...)
    {
        return NULL;
    }
}

HX_API HxSkin HX_CALL hx_make_skin(HxColor color, HxSkinFlags flags)
{
    if (flags & ~(HX_SKIN_UNLIT | HX_SKIN_DOUBLE_SIDED))
        return nullptr;
    HxSkin skin = new (std::nothrow) HxSkinImpl{color, flags, nullptr};
    if (!skin)
        return NULL;
    if (!hx_resource_register(skin, sizeof(HxSkinImpl), 0, hx_skin_destroy_resource))
    {
        delete skin;
        return NULL;
    }
    return skin;
}
HX_API HxSkin HX_CALL hx_make_skin_tex(HxTex tex, HxColor tint, HxSkinFlags flags)
{
    if (!tex)
        return hx_make_skin(tint, flags);
    HxTextureRenderData texture_data{};
    if (!hx_texture_get_render_data(tex, &texture_data))
        return nullptr;
    HxSkin skin = hx_make_skin(tint, flags);
    if (skin)
        skin->texture = tex;
    return skin;
}
HX_API HxSkin HX_CALL hx_make_skin_pbr(HxTex albedo, HxTex normal, HxTex metal_rough, HxTex ao, HxTex emissive,
                                       float metallic, float roughness, HxColor emissive_color, HxSkinFlags flags)
{
    (void)albedo;
    (void)normal;
    (void)metal_rough;
    (void)ao;
    (void)emissive;
    (void)metallic;
    (void)roughness;
    (void)emissive_color;
    (void)flags;
    return NULL;
}
HX_API HxResult HX_CALL hx_drop_skin(HxSkin skin)
{
    if (!skin)
        return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(skin))
        return HX_ERR_ALREADY_DROPPED;
    delete skin;
    return HX_OK;
}

bool hx_mesh_get_render_data(HxMesh mesh, HxMeshRenderData* out_data)
{
    if (!out_data || !hx_resource_is_registered(mesh))
        return false;
    *out_data = {mesh->positions.data(),
                 mesh->texcoords.empty() ? nullptr : mesh->texcoords.data(),
                 mesh->colors.empty() ? nullptr : mesh->colors.data(),
                 mesh->indices.data(),
                 mesh->positions.size(),
                 mesh->indices.size(),
                 mesh->topology,
                 mesh->position,
                 mesh->rotation,
                 mesh->scale};
    return true;
}
bool hx_skin_get_render_data(HxSkin skin, HxColor* out_color, HxTex* out_texture)
{
    if (!out_color || !out_texture)
        return false;
    if (skin && !hx_resource_is_registered(skin))
        return false;
    if (skin && skin->texture && !hx_resource_is_registered(skin->texture))
        return false;
    *out_color = skin ? skin->color : HX_WHITE;
    *out_texture = skin ? skin->texture : nullptr;
    return true;
}
