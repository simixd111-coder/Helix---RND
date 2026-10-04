// camera.cpp — Camera API implementation
#include "helix.h"
#include "resource_internal.h"
#include <math.h>
#include <stdlib.h>

struct HxCamImpl {
    HxMat4 view;
    HxMat4 projection;
    HxCamType type;
};

static void hx_cam_destroy_resource(void* resource) {
    free(resource);
}

static HxVec3 hx_cam_sub(HxVec3 a, HxVec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

static HxVec3 hx_cam_cross(HxVec3 a, HxVec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static float hx_cam_dot(HxVec3 a, HxVec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static HxVec3 hx_cam_normalize(HxVec3 v) {
    float length = sqrtf(hx_cam_dot(v, v));
    if (length <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return {v.x / length, v.y / length, v.z / length};
}

static HxCam hx_cam_create(HxCamType type) {
    HxCam cam = static_cast<HxCam>(calloc(1, sizeof(HxCamImpl)));
    if (!cam) return NULL;
    cam->type = type;
    hx_make_mat4_identity(&cam->view);
    hx_make_mat4_identity(&cam->projection);
    if (!hx_resource_register(cam, sizeof(HxCamImpl), 0, hx_cam_destroy_resource)) {
        free(cam);
        return NULL;
    }
    return cam;
}

HX_API HxCam HX_CALL hx_make_cam3d(void) { return hx_cam_create(HX_CAM_3D); }
HX_API HxCam HX_CALL hx_make_cam2d(void) { return hx_cam_create(HX_CAM_2D); }

HX_API void HX_CALL hx_look(HxCam cam, const HxVec3* at, const HxVec3* from, const HxVec3* up) {
    if (!cam || !at || !from || !up) return;
    HxVec3 forward = hx_cam_normalize(hx_cam_sub(*at, *from));
    HxVec3 right = hx_cam_normalize(hx_cam_cross(forward, *up));
    HxVec3 corrected_up = hx_cam_cross(right, forward);
    HxMat4* view = &cam->view;
    hx_make_mat4_identity(view);
    view->m[0][0] = right.x; view->m[1][0] = right.y; view->m[2][0] = right.z;
    view->m[0][1] = corrected_up.x; view->m[1][1] = corrected_up.y; view->m[2][1] = corrected_up.z;
    view->m[0][2] = -forward.x; view->m[1][2] = -forward.y; view->m[2][2] = -forward.z;
    view->m[3][0] = -hx_cam_dot(right, *from);
    view->m[3][1] = -hx_cam_dot(corrected_up, *from);
    view->m[3][2] = hx_cam_dot(forward, *from);
}

HX_API void HX_CALL hx_set_cam_persp(HxCam cam, float fov_y_deg, float aspect, float near_z, float far_z) {
    if (!cam || aspect == 0.0f || near_z == far_z) return;
    float f = 1.0f / tanf(fov_y_deg * 0.008726646259971648f);
    hx_make_mat4_identity(&cam->projection);
    cam->projection.m[0][0] = f / aspect;
    cam->projection.m[1][1] = f;
    cam->projection.m[2][2] = far_z / (near_z - far_z);
    cam->projection.m[2][3] = -1.0f;
    cam->projection.m[3][2] = near_z * far_z / (near_z - far_z);
    cam->projection.m[3][3] = 0.0f;
}

HX_API void HX_CALL hx_set_cam_ortho(HxCam cam, float left, float right, float bottom, float top, float near_z, float far_z) {
    if (!cam || right == left || top == bottom || near_z == far_z) return;
    hx_make_mat4_identity(&cam->projection);
    cam->projection.m[0][0] = 2.0f / (right - left);
    cam->projection.m[1][1] = 2.0f / (top - bottom);
    cam->projection.m[2][2] = 1.0f / (near_z - far_z);
    cam->projection.m[3][0] = (right + left) / (left - right);
    cam->projection.m[3][1] = (top + bottom) / (bottom - top);
    cam->projection.m[3][2] = near_z / (near_z - far_z);
}

HX_API void HX_CALL hx_move_cam2d(HxCam cam, float x, float y) {
    if (!cam) return;
    cam->view.m[3][0] = -x;
    cam->view.m[3][1] = -y;
}

HX_API void HX_CALL hx_size_cam2d(HxCam cam, float zoom) {
    if (!cam || zoom <= 0.0f) return;
    cam->projection.m[0][0] = zoom;
    cam->projection.m[1][1] = zoom;
}

HX_API void HX_CALL hx_get_cam_view(HxCam cam, HxMat4* out_view) {
    if (cam && out_view) *out_view = cam->view;
}

HX_API void HX_CALL hx_get_cam_proj(HxCam cam, HxMat4* out_proj) {
    if (cam && out_proj) *out_proj = cam->projection;
}

HX_API void HX_CALL hx_get_cam_view_proj(HxCam cam, HxMat4* out_view_proj) {
    if (cam && out_view_proj) hx_mul_mat4(&cam->projection, &cam->view, out_view_proj);
}

HX_API HxResult HX_CALL hx_drop_cam(HxCam cam) {
    if (!cam) return HX_ERR_INVALID_HANDLE;
    if (!hx_resource_unregister(cam)) return HX_ERR_ALREADY_DROPPED;
    free(cam);
    return HX_OK;
}
