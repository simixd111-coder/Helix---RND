#pragma once
#include "helix.h"

struct HxWorldMeshItem
{
    HxMesh mesh;
    HxSkin skin;
    HxMat4 transform;
};

struct HxMeshRenderData
{
    const HxVec3* positions;
    const HxVec2* texcoords;
    const HxVec4* colors;
    const uint32_t* indices;
    size_t vertex_count;
    size_t index_count;
    HxPrim topology;
    HxVec3 position;
    HxVec3 rotation;
    HxVec3 scale;
};

struct HxTextureRenderData
{
    const uint8_t* rgba_pixels;
    int width;
    int height;
    HxTexFlags flags;
};

size_t hx_world_mesh_count(HxWorld world);
bool hx_world_mesh_get(HxWorld world, size_t index, HxWorldMeshItem* out_item);
bool hx_mesh_get_render_data(HxMesh mesh, HxMeshRenderData* out_data);
bool hx_skin_get_render_data(HxSkin skin, HxColor* out_color, HxTex* out_texture);
bool hx_texture_get_render_data(HxTex texture, HxTextureRenderData* out_data);
HxResult hx_soft_render_world(int width, int height, HxWorld world, HxCam cam, void* out_pixels, size_t out_stride);
