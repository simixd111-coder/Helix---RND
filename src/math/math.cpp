// math.cpp — Math library implementation (header-only in practice, but compiled for Phase 1)
#include "helix.h"
#include <math.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Vec2
// -----------------------------------------------------------------------------
static inline HxVec2 hx_vec2(float x, float y) {
    HxVec2 v = {x, y};
    return v;
}

static inline HxVec2 hx_vec2_add(HxVec2 a, HxVec2 b) {
    return hx_vec2(a.x + b.x, a.y + b.y);
}

static inline HxVec2 hx_vec2_sub(HxVec2 a, HxVec2 b) {
    return hx_vec2(a.x - b.x, a.y - b.y);
}

static inline HxVec2 hx_vec2_mul(HxVec2 a, float s) {
    return hx_vec2(a.x * s, a.y * s);
}

static inline float hx_vec2_dot(HxVec2 a, HxVec2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float hx_vec2_len(HxVec2 v) {
    return sqrtf(hx_vec2_dot(v, v));
}

static inline HxVec2 hx_vec2_norm(HxVec2 v) {
    float l = hx_vec2_len(v);
    return l > 0 ? hx_vec2_mul(v, 1.0f / l) : hx_vec2(0, 0);
}

// -----------------------------------------------------------------------------
// Vec3
// -----------------------------------------------------------------------------
static inline HxVec3 hx_vec3(float x, float y, float z) {
    HxVec3 v = {x, y, z};
    return v;
}

HX_API HxVec3 HX_CALL hx_vec3_add(HxVec3 a, HxVec3 b) {
    return hx_vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

HX_API HxVec3 HX_CALL hx_vec3_sub(HxVec3 a, HxVec3 b) {
    return hx_vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

HX_API HxVec3 HX_CALL hx_vec3_mul(HxVec3 a, float s) {
    return hx_vec3(a.x * s, a.y * s, a.z * s);
}

HX_API float HX_CALL hx_vec3_dot(HxVec3 a, HxVec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

HX_API HxVec3 HX_CALL hx_vec3_cross(HxVec3 a, HxVec3 b) {
    return hx_vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

HX_API float HX_CALL hx_vec3_len(HxVec3 v) {
    return sqrtf(hx_vec3_dot(v, v));
}

HX_API HxVec3 HX_CALL hx_vec3_norm(HxVec3 v) {
    float l = hx_vec3_len(v);
    return l > 0 ? hx_vec3_mul(v, 1.0f / l) : hx_vec3(0, 0, 0);
}

HX_API HxVec3 HX_CALL hx_vec3_lerp(HxVec3 a, HxVec3 b, float t) {
    return hx_vec3_add(hx_vec3_mul(a, 1.0f - t), hx_vec3_mul(b, t));
}

// -----------------------------------------------------------------------------
// Vec4
// -----------------------------------------------------------------------------
static inline HxVec4 hx_vec4(float x, float y, float z, float w) {
    HxVec4 v = {x, y, z, w};
    return v;
}

static inline HxVec4 hx_vec4_add(HxVec4 a, HxVec4 b) {
    return hx_vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

static inline HxVec4 hx_vec4_mul(HxVec4 a, float s) {
    return hx_vec4(a.x * s, a.y * s, a.z * s, a.w * s);
}

static inline float hx_vec4_dot(HxVec4 a, HxVec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// -----------------------------------------------------------------------------
// Quat
// -----------------------------------------------------------------------------
static inline HxQuat hx_quat(float x, float y, float z, float w) {
    HxQuat q = {x, y, z, w};
    return q;
}

HX_API void HX_CALL hx_quat_identity(HxQuat* q) {
    q->x = 0; q->y = 0; q->z = 0; q->w = 1;
}

HX_API void HX_CALL hx_quat_mul(const HxQuat* a, const HxQuat* b, HxQuat* out) {
    out->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
    out->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
    out->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
    out->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
}

HX_API void HX_CALL hx_quat_from_axis_angle(const HxVec3* axis, float rad, HxQuat* out) {
    float half = rad * 0.5f;
    float s = sinf(half);
    HxVec3 n = hx_vec3_norm(*axis);
    out->x = n.x * s;
    out->y = n.y * s;
    out->z = n.z * s;
    out->w = cosf(half);
}

HX_API void HX_CALL hx_quat_from_euler(float x, float y, float z, HxQuat* out) {
    // XYZ order
    float cx = cosf(x * 0.5f), sx = sinf(x * 0.5f);
    float cy = cosf(y * 0.5f), sy = sinf(y * 0.5f);
    float cz = cosf(z * 0.5f), sz = sinf(z * 0.5f);

    out->x = sx * cy * cz - cx * sy * sz;
    out->y = cx * sy * cz + sx * cy * sz;
    out->z = cx * cy * sz - sx * sy * cz;
    out->w = cx * cy * cz + sx * sy * sz;
}

HX_API void HX_CALL hx_quat_slerp(const HxQuat* a, const HxQuat* b, float t, HxQuat* out) {
    float dot = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    HxQuat b2 = *b;
    if (dot < 0) {
        dot = -dot;
        b2.x = -b2.x; b2.y = -b2.y; b2.z = -b2.z; b2.w = -b2.w;
    }
    if (dot > 0.9995f) {
        // Linear interpolation for close quaternions
        out->x = a->x + t * (b2.x - a->x);
        out->y = a->y + t * (b2.y - a->y);
        out->z = a->z + t * (b2.z - a->z);
        out->w = a->w + t * (b2.w - a->w);
        float len = sqrtf(out->x*out->x + out->y*out->y + out->z*out->z + out->w*out->w);
        out->x /= len; out->y /= len; out->z /= len; out->w /= len;
        return;
    }
    float theta = acosf(dot);
    float sin_theta = sinf(theta);
    float wa = sinf((1.0f - t) * theta) / sin_theta;
    float wb = sinf(t * theta) / sin_theta;
    out->x = wa * a->x + wb * b2.x;
    out->y = wa * a->y + wb * b2.y;
    out->z = wa * a->z + wb * b2.z;
    out->w = wa * a->w + wb * b2.w;
}

HX_API void HX_CALL hx_quat_rotate_vec(const HxQuat* q, const HxVec3* v, HxVec3* out) {
    // v' = q * v * q^-1 (where v is pure quaternion)
    HxQuat p = {v->x, v->y, v->z, 0};
    HxQuat q_conj = {-q->x, -q->y, -q->z, q->w};
    HxQuat tmp, res;
    hx_quat_mul(q, &p, &tmp);
    hx_quat_mul(&tmp, &q_conj, &res);
    out->x = res.x;
    out->y = res.y;
    out->z = res.z;
}

// -----------------------------------------------------------------------------
// Mat4 (column-major)
// -----------------------------------------------------------------------------
HX_API void HX_CALL hx_mat4_identity(HxMat4* m) {
    memset(m, 0, sizeof(HxMat4));
    m->m[0][0] = 1;
    m->m[1][1] = 1;
    m->m[2][2] = 1;
    m->m[3][3] = 1;
}

HX_API void HX_CALL hx_mat4_mul(const HxMat4* a, const HxMat4* b, HxMat4* out) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out->m[c][r] = a->m[c][0] * b->m[0][r] +
                           a->m[c][1] * b->m[1][r] +
                           a->m[c][2] * b->m[2][r] +
                           a->m[c][3] * b->m[3][r];
        }
    }
}

HX_API void HX_CALL hx_mat4_translate(const HxVec3* v, HxMat4* out) {
    hx_mat4_identity(out);
    out->m[3][0] = v->x;
    out->m[3][1] = v->y;
    out->m[3][2] = v->z;
}

HX_API void HX_CALL hx_mat4_rotate(const HxQuat* q, HxMat4* out) {
    float xx = q->x * q->x;
    float yy = q->y * q->y;
    float zz = q->z * q->z;
    float xy = q->x * q->y;
    float xz = q->x * q->z;
    float yz = q->y * q->z;
    float wx = q->w * q->x;
    float wy = q->w * q->y;
    float wz = q->w * q->z;

    out->m[0][0] = 1 - 2 * (yy + zz);
    out->m[0][1] = 2 * (xy + wz);
    out->m[0][2] = 2 * (xz - wy);
    out->m[0][3] = 0;

    out->m[1][0] = 2 * (xy - wz);
    out->m[1][1] = 1 - 2 * (xx + zz);
    out->m[1][2] = 2 * (yz + wx);
    out->m[1][3] = 0;

    out->m[2][0] = 2 * (xz + wy);
    out->m[2][1] = 2 * (yz - wx);
    out->m[2][2] = 1 - 2 * (xx + yy);
    out->m[2][3] = 0;

    out->m[3][0] = 0;
    out->m[3][1] = 0;
    out->m[3][2] = 0;
    out->m[3][3] = 1;
}

HX_API void HX_CALL hx_mat4_scale(const HxVec3* v, HxMat4* out) {
    hx_mat4_identity(out);
    out->m[0][0] = v->x;
    out->m[1][1] = v->y;
    out->m[2][2] = v->z;
}

HX_API void HX_CALL hx_mat4_trs(const HxVec3* t, const HxQuat* r, const HxVec3* s, HxMat4* out) {
    HxMat4 T, R, S;
    hx_mat4_translate(t, &T);
    hx_mat4_rotate(r, &R);
    hx_mat4_scale(s, &S);
    HxMat4 TR;
    hx_mat4_mul(&T, &R, &TR);
    hx_mat4_mul(&TR, &S, out);
}

HX_API void HX_CALL hx_mat4_inverse(const HxMat4* m, HxMat4* out) {
    // Gauss-Jordan elimination for 4x4
    float aug[4][8];
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            aug[r][c] = m->m[c][r];
            aug[r][c + 4] = (r == c) ? 1.0f : 0.0f;
        }
    }

    for (int col = 0; col < 4; ++col) {
        // Find pivot
        int pivot = col;
        float max_val = fabsf(aug[pivot][col]);
        for (int row = col + 1; row < 4; ++row) {
            float val = fabsf(aug[row][col]);
            if (val > max_val) {
                max_val = val;
                pivot = row;
            }
        }
        if (max_val < 1e-10f) {
            hx_mat4_identity(out);
            return;
        }
        if (pivot != col) {
            for (int c = 0; c < 8; ++c) {
                float tmp = aug[col][c];
                aug[col][c] = aug[pivot][c];
                aug[pivot][c] = tmp;
            }
        }
        // Normalize pivot row
        float div = aug[col][col];
        for (int c = 0; c < 8; ++c) aug[col][c] /= div;
        // Eliminate other rows
        for (int row = 0; row < 4; ++row) {
            if (row == col) continue;
            float factor = aug[row][col];
            for (int c = 0; c < 8; ++c) {
                aug[row][c] -= factor * aug[col][c];
            }
        }
    }

    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out->m[c][r] = aug[r][c + 4];
        }
    }
}

HX_API void HX_CALL hx_mat4_transpose(const HxMat4* m, HxMat4* out) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out->m[c][r] = m->m[r][c];
        }
    }
}

// -----------------------------------------------------------------------------
// Perspective / Ortho matrices (helpers)
// -----------------------------------------------------------------------------
static void hx_mat4_persp(float fov_y, float aspect, float near_z, float far_z, HxMat4* out) {
    float f = 1.0f / tanf(fov_y * 0.5f);
    hx_mat4_identity(out);
    out->m[0][0] = f / aspect;
    out->m[1][1] = f;
    out->m[2][2] = far_z / (near_z - far_z);
    out->m[2][3] = -1;
    out->m[3][2] = near_z * far_z / (near_z - far_z);
    out->m[3][3] = 0;
}

static void hx_mat4_ortho(float l, float r, float b, float t, float n, float f, HxMat4* out) {
    hx_mat4_identity(out);
    out->m[0][0] = 2 / (r - l);
    out->m[1][1] = 2 / (t - b);
    out->m[2][2] = 1 / (n - f);
    out->m[3][0] = (r + l) / (l - r);
    out->m[3][1] = (t + b) / (b - t);
    out->m[3][2] = n / (n - f);
}

// -----------------------------------------------------------------------------
// Color helpers
// -----------------------------------------------------------------------------
static inline HxColor hx_color_linear(float r, float g, float b, float a) {
    HxColor c = {r, g, b, a};
    return c;
}

static inline HxColor hx_color_srgb(float r, float g, float b, float a) {
    // sRGB to linear
    auto to_linear = [](float c) {
        return c <= 0.04045f ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f);
    };
    HxColor c = {to_linear(r), to_linear(g), to_linear(b), a};
    return c;
}

HX_API void HX_CALL hx_make_mat4_identity(HxMat4* m) { hx_mat4_identity(m); }
HX_API void HX_CALL hx_mul_mat4(const HxMat4* a, const HxMat4* b, HxMat4* out) { hx_mat4_mul(a, b, out); }
HX_API void HX_CALL hx_make_mat4_translate(const HxVec3* v, HxMat4* out) { hx_mat4_translate(v, out); }
HX_API void HX_CALL hx_make_mat4_rotate(const HxQuat* q, HxMat4* out) { hx_mat4_rotate(q, out); }
HX_API void HX_CALL hx_make_mat4_scale(const HxVec3* v, HxMat4* out) { hx_mat4_scale(v, out); }
HX_API void HX_CALL hx_make_mat4_trs(const HxVec3* t, const HxQuat* r, const HxVec3* s, HxMat4* out) { hx_mat4_trs(t, r, s, out); }
HX_API void HX_CALL hx_inverse_mat4(const HxMat4* m, HxMat4* out) { hx_mat4_inverse(m, out); }
HX_API void HX_CALL hx_transpose_mat4(const HxMat4* m, HxMat4* out) { hx_mat4_transpose(m, out); }

HX_API void HX_CALL hx_make_quat_identity(HxQuat* q) { hx_quat_identity(q); }
HX_API void HX_CALL hx_mul_quat(const HxQuat* a, const HxQuat* b, HxQuat* out) { hx_quat_mul(a, b, out); }
HX_API void HX_CALL hx_make_quat_axis_angle(const HxVec3* axis, float rad, HxQuat* out) { hx_quat_from_axis_angle(axis, rad, out); }
HX_API void HX_CALL hx_make_quat_euler(float x, float y, float z, HxQuat* out) { hx_quat_from_euler(x, y, z, out); }
HX_API void HX_CALL hx_slerp_quat(const HxQuat* a, const HxQuat* b, float t, HxQuat* out) { hx_quat_slerp(a, b, t, out); }
HX_API void HX_CALL hx_rotate_vec_quat(const HxQuat* q, const HxVec3* v, HxVec3* out) { hx_quat_rotate_vec(q, v, out); }