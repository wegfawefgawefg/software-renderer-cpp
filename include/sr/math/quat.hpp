#pragma once

#include "sr/math/vec3.hpp"

#include <algorithm>
#include <cmath>

namespace sr::math {

struct Quat {
    // (x,y,z,w) where w is scalar component.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    static constexpr Quat identity() { return Quat{0.0f, 0.0f, 0.0f, 1.0f}; }

    static Quat from_axis_angle(const Vec3& axis_unit, float angle_rad) {
        float half = angle_rad * 0.5f;
        float s = std::sin(half);
        return Quat{axis_unit.x * s, axis_unit.y * s, axis_unit.z * s, std::cos(half)};
    }
};

inline float dot(const Quat& a, const Quat& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline Quat normalize(const Quat& q) {
    float d = dot(q, q);
    if (d <= 0.0f)
        return q;
    float inv = 1.0f / std::sqrt(d);
    return Quat{q.x * inv, q.y * inv, q.z * inv, q.w * inv};
}

inline Quat operator*(const Quat& a, const Quat& b) {
    // Hamilton product.
    return Quat{
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}

inline Quat slerp(Quat a, Quat b, float t) {
    // Standard unit-quaternion slerp with shortest-arc handling.
    a = normalize(a);
    b = normalize(b);

    float cos_omega = dot(a, b);
    if (cos_omega < 0.0f) {
        cos_omega = -cos_omega;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        b.w = -b.w;
    }

    // If very close, fall back to normalized lerp.
    if (cos_omega > 0.9995f) {
        Quat r{
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t,
        };
        return normalize(r);
    }

    cos_omega = std::clamp(cos_omega, -1.0f, 1.0f);
    float omega = std::acos(cos_omega);
    float sin_omega = std::sin(omega);
    float s0 = std::sin((1.0f - t) * omega) / sin_omega;
    float s1 = std::sin(t * omega) / sin_omega;

    return Quat{
        a.x * s0 + b.x * s1,
        a.y * s0 + b.y * s1,
        a.z * s0 + b.z * s1,
        a.w * s0 + b.w * s1,
    };
}

} // namespace sr::math

