#pragma once

#include "sr/math/mat4.hpp"
#include "sr/math/vec3.hpp"
#include "sr/math/vec4.hpp"

#include <algorithm>
#include <cmath>

namespace sr::math {

inline Vec3 transform_point(const Mat4& m, const Vec3& p) {
    Vec4 r = mul(m, Vec4{p.x, p.y, p.z, 1.0f});
    // Caller decides whether to perspective divide; most uses are model/world transforms.
    return Vec3{r.x, r.y, r.z};
}

inline Vec3 transform_vector(const Mat4& m, const Vec3& v) {
    Vec4 r = mul(m, Vec4{v.x, v.y, v.z, 0.0f});
    return Vec3{r.x, r.y, r.z};
}

// Conservative scale factor for bounding spheres under an affine transform.
inline float max_scale_component(const Mat4& m) {
    // Column vector convention; scale lives in the 3x3 upper-left.
    // Use row lengths (same result for pure scale+rotation).
    float sx = std::sqrt(m.m[0][0] * m.m[0][0] + m.m[0][1] * m.m[0][1] + m.m[0][2] * m.m[0][2]);
    float sy = std::sqrt(m.m[1][0] * m.m[1][0] + m.m[1][1] * m.m[1][1] + m.m[1][2] * m.m[1][2]);
    float sz = std::sqrt(m.m[2][0] * m.m[2][0] + m.m[2][1] * m.m[2][1] + m.m[2][2] * m.m[2][2]);
    return std::max({sx, sy, sz});
}

} // namespace sr::math
