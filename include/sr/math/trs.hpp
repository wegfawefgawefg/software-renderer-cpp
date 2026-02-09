#pragma once

#include "sr/math/mat4.hpp"
#include "sr/math/quat.hpp"
#include "sr/math/vec3.hpp"

namespace sr::math {

struct Trs {
    Vec3 t{0.0f, 0.0f, 0.0f};
    Quat r = Quat::identity();
    Vec3 s{1.0f, 1.0f, 1.0f};
};

inline Trs lerp(const Trs& a, const Trs& b, float t) {
    Trs out;
    out.t = a.t + (b.t - a.t) * t;
    out.s = a.s + (b.s - a.s) * t;
    out.r = slerp(a.r, b.r, t);
    return out;
}

inline Mat4 quat_to_mat4(const Quat& q_in) {
    Quat q = normalize(q_in);
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    Mat4 m = Mat4::identity();
    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[0][1] = 2.0f * (xy - wz);
    m.m[0][2] = 2.0f * (xz + wy);

    m.m[1][0] = 2.0f * (xy + wz);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[1][2] = 2.0f * (yz - wx);

    m.m[2][0] = 2.0f * (xz - wy);
    m.m[2][1] = 2.0f * (yz + wx);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return m;
}

inline Mat4 trs_to_mat4(const Trs& x) {
    Mat4 t = Mat4::translate(x.t);
    Mat4 r = quat_to_mat4(x.r);
    Mat4 s = Mat4::scale(x.s);
    return mul(t, mul(r, s));
}

} // namespace sr::math

