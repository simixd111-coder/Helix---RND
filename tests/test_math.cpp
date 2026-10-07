// test_math.cpp — Math library tests
#include "helix.h"
#include <cassert>
#include <cmath>
#include <cstdio>

#define EPS 1e-5f
#define ASSERT_EQ(a, b) do { if (fabsf((a) - (b)) > EPS) { printf("FAIL: %s == %s (%.6f vs %.6f)\n", #a, #b, (double)(a), (double)(b)); return 1; } } while(0)
#define ASSERT_VEC3_EQ(a, ...) do { HxVec3 expected = __VA_ARGS__; ASSERT_EQ((a).x, expected.x); ASSERT_EQ((a).y, expected.y); ASSERT_EQ((a).z, expected.z); } while(0)
#define ASSERT_QUAT_EQ(a, ...) do { HxQuat expected = __VA_ARGS__; ASSERT_EQ((a).x, expected.x); ASSERT_EQ((a).y, expected.y); ASSERT_EQ((a).z, expected.z); ASSERT_EQ((a).w, expected.w); } while(0)

int test_vec3() {
    HxVec3 a = {1, 2, 3};
    HxVec3 b = {4, 5, 6};
    HxVec3 c = hx_vec3_add(a, b);
    ASSERT_VEC3_EQ(c, HxVec3{5, 7, 9});

    c = hx_vec3_sub(b, a);
    ASSERT_VEC3_EQ(c, HxVec3{3, 3, 3});

    c = hx_vec3_mul(a, 2.0f);
    ASSERT_VEC3_EQ(c, HxVec3{2, 4, 6});

    float d = hx_vec3_dot(a, b);
    ASSERT_EQ(d, 32.0f);

    HxVec3 cross = hx_vec3_cross(a, b);
    ASSERT_VEC3_EQ(cross, HxVec3{-3, 6, -3});

    float len = hx_vec3_len(HxVec3{3, 4, 0});
    ASSERT_EQ(len, 5.0f);

    HxVec3 norm = hx_vec3_norm(HxVec3{3, 4, 0});
    ASSERT_VEC3_EQ(norm, HxVec3{0.6f, 0.8f, 0.0f});

    HxVec3 lerp = hx_vec3_lerp(HxVec3{0,0,0}, HxVec3{10,10,10}, 0.5f);
    ASSERT_VEC3_EQ(lerp, HxVec3{5, 5, 5});

    printf("test_vec3: PASS\n");
    return 0;
}

int test_quat() {
    HxQuat q;
    hx_make_quat_identity(&q);
    ASSERT_QUAT_EQ(q, HxQuat{0, 0, 0, 1});

    HxQuat a = {0, 0, 0.7071067f, 0.7071067f}; // 90 deg around Z
    HxQuat b = {0, 0.7071067f, 0, 0.7071067f}; // 90 deg around Y
    HxQuat c;
    hx_mul_quat(&a, &b, &c);

    HxVec3 axis = {0, 0, 1};
    hx_make_quat_axis_angle(&axis, 1.570796f, &q); // 90 deg
    ASSERT_QUAT_EQ(q, HxQuat{0, 0, 0.7071067f, 0.7071067f});

    hx_make_quat_euler(1.570796f, 0, 0, &q); // 90 deg X
    ASSERT_QUAT_EQ(q, HxQuat{0.7071067f, 0, 0, 0.7071067f});

    HxQuat q1 = {0, 0, 0, 1};
    HxQuat q2 = {0, 0, 0.7071067f, 0.7071067f};
    hx_slerp_quat(&q1, &q2, 0.5f, &c);
    // Should be 45 deg around Z
    ASSERT_QUAT_EQ(c, HxQuat{0, 0, 0.382683f, 0.92388f});

    HxVec3 v = {1, 0, 0};
    HxVec3 out;
    hx_rotate_vec_quat(&q2, &v, &out); // Rotate X by 90 deg Z -> Y
    ASSERT_VEC3_EQ(out, HxVec3{0, 1, 0});

    printf("test_quat: PASS\n");
    return 0;
}

int test_mat4() {
    HxMat4 m;
    hx_make_mat4_identity(&m);
    for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) {
        ASSERT_EQ(m.m[c][r], (c == r) ? 1.0f : 0.0f);
    }

    HxMat4 a, b, c;
    hx_make_mat4_identity(&a);
    hx_make_mat4_identity(&b);
    a.m[3][0] = 1; a.m[3][1] = 2; a.m[3][2] = 3;
    b.m[3][0] = 4; b.m[3][1] = 5; b.m[3][2] = 6;
    hx_mul_mat4(&a, &b, &c);
    ASSERT_EQ(c.m[3][0], 5);
    ASSERT_EQ(c.m[3][1], 7);
    ASSERT_EQ(c.m[3][2], 9);

    HxVec3 t = {1, 2, 3};
    hx_make_mat4_translate(&t, &m);
    ASSERT_EQ(m.m[3][0], 1);
    ASSERT_EQ(m.m[3][1], 2);
    ASSERT_EQ(m.m[3][2], 3);

    HxQuat q = {0, 0, 0.7071067f, 0.7071067f};
    hx_make_mat4_rotate(&q, &m);
    // 90 deg Z rotation
    ASSERT_EQ(m.m[0][0], 0); ASSERT_EQ(m.m[0][1], 1);
    ASSERT_EQ(m.m[1][0], -1); ASSERT_EQ(m.m[1][1], 0);

    HxVec3 s = {2, 3, 4};
    hx_make_mat4_scale(&s, &m);
    ASSERT_EQ(m.m[0][0], 2); ASSERT_EQ(m.m[1][1], 3); ASSERT_EQ(m.m[2][2], 4);

    HxVec3 t2 = {1, 2, 3};
    HxQuat r = {0, 0, 0, 1};
    HxVec3 s2 = {1, 1, 1};
    hx_make_mat4_trs(&t2, &r, &s2, &m);
    ASSERT_EQ(m.m[3][0], 1); ASSERT_EQ(m.m[3][1], 2); ASSERT_EQ(m.m[3][2], 3);

    HxMat4 inv;
    hx_make_mat4_translate(&t, &m);
    hx_inverse_mat4(&m, &inv);
    ASSERT_EQ(inv.m[3][0], -1); ASSERT_EQ(inv.m[3][1], -2); ASSERT_EQ(inv.m[3][2], -3);

    hx_transpose_mat4(&m, &inv);
    ASSERT_EQ(inv.m[0][3], 1); ASSERT_EQ(inv.m[1][3], 2); ASSERT_EQ(inv.m[2][3], 3);

    printf("test_mat4: PASS\n");
    return 0;
}

int main() {
    int failed = 0;
    failed += test_vec3();
    failed += test_quat();
    failed += test_mat4();
    if (failed == 0) {
        printf("\nAll math tests PASSED\n");
    } else {
        printf("\n%d test(s) FAILED\n", failed);
    }
    return failed;
}