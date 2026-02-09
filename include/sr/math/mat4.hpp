#pragma once

#include "sr/math/vec3.hpp"
#include "sr/math/vec4.hpp"

#include <cmath>

namespace sr::math {

// Row-major 4x4 matrix; vectors treated as column vectors:
//   v' = M * v
struct Mat4 {
    float m[4][4]{};

    static Mat4 identity() {
        Mat4 r{};
        r.m[0][0] = 1.0f;
        r.m[1][1] = 1.0f;
        r.m[2][2] = 1.0f;
        r.m[3][3] = 1.0f;
        return r;
    }

    static Mat4 translate(const Vec3& t) {
        Mat4 r = identity();
        r.m[0][3] = t.x;
        r.m[1][3] = t.y;
        r.m[2][3] = t.z;
        return r;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 r = identity();
        r.m[0][0] = s.x;
        r.m[1][1] = s.y;
        r.m[2][2] = s.z;
        return r;
    }

    static Mat4 rotate_x(float a) {
        Mat4 r = identity();
        float c = std::cos(a);
        float s = std::sin(a);
        r.m[1][1] = c;
        r.m[1][2] = -s;
        r.m[2][1] = s;
        r.m[2][2] = c;
        return r;
    }

    static Mat4 rotate_y(float a) {
        Mat4 r = identity();
        float c = std::cos(a);
        float s = std::sin(a);
        r.m[0][0] = c;
        r.m[0][2] = s;
        r.m[2][0] = -s;
        r.m[2][2] = c;
        return r;
    }

    static Mat4 rotate_z(float a) {
        Mat4 r = identity();
        float c = std::cos(a);
        float s = std::sin(a);
        r.m[0][0] = c;
        r.m[0][1] = -s;
        r.m[1][0] = s;
        r.m[1][1] = c;
        return r;
    }

    // OpenGL-style right-handed perspective projection (NDC z in [-1,1]).
    static Mat4 perspective(float fov_y_rad, float aspect, float z_near, float z_far) {
        Mat4 r{};
        float f = 1.0f / std::tan(fov_y_rad * 0.5f);
        r.m[0][0] = f / aspect;
        r.m[1][1] = f;
        r.m[2][2] = (z_far + z_near) / (z_near - z_far);
        r.m[2][3] = (2.0f * z_far * z_near) / (z_near - z_far);
        r.m[3][2] = -1.0f;
        return r;
    }

    // Right-handed look-at (camera looks toward target).
    static Mat4 look_at(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = normalize(target - eye);
        // Standard right-handed OpenGL-style view matrix (camera forward is -Z in view space).
        // This matches the common formulation:
        //   s = normalize(f x up)
        //   u = s x f
        // with the third row using -f.
        //
        // NOTE: If your controls feel inverted, fix the controller math (strafe basis / yaw sign),
        // not the view matrix (changing handedness here will mirror the whole world and flip winding).
        Vec3 s = normalize(cross(f, up)); // camera-right in the view transform
        Vec3 u = cross(s, f);             // camera-up

        Mat4 r = identity();
        r.m[0][0] = s.x;
        r.m[0][1] = s.y;
        r.m[0][2] = s.z;
        r.m[0][3] = -dot(s, eye);

        r.m[1][0] = u.x;
        r.m[1][1] = u.y;
        r.m[1][2] = u.z;
        r.m[1][3] = -dot(u, eye);

        r.m[2][0] = -f.x;
        r.m[2][1] = -f.y;
        r.m[2][2] = -f.z;
        r.m[2][3] = dot(f, eye);

        return r;
    }
};

inline Mat4 mul(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a.m[i][k] * b.m[k][j];
            r.m[i][j] = s;
        }
    }
    return r;
}

inline Vec4 mul(const Mat4& a, const Vec4& v) {
    return {
        a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.z + a.m[0][3] * v.w,
        a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.z + a.m[1][3] * v.w,
        a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.z + a.m[2][3] * v.w,
        a.m[3][0] * v.x + a.m[3][1] * v.y + a.m[3][2] * v.z + a.m[3][3] * v.w,
    };
}

} // namespace sr::math
