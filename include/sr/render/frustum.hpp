#pragma once

#include "sr/math/mat4.hpp"
#include "sr/math/vec3.hpp"

#include <array>
#include <cmath>

namespace sr::render {

struct Plane {
    // ax + by + cz + d >= 0 is inside.
    sr::math::Vec3 n{0.0f, 1.0f, 0.0f};
    float d = 0.0f;
};

struct Frustum {
    // Order: left, right, bottom, top, near, far.
    std::array<Plane, 6> planes{};

    static Frustum from_view_proj(const sr::math::Mat4& vp) {
        // With column vectors and row-major storage:
        // clip = VP * world_h
        // Inside is: -w<=x<=w, -w<=y<=w, -w<=z<=w.
        auto row = [&](int r) {
            return std::array<float, 4>{vp.m[r][0], vp.m[r][1], vp.m[r][2], vp.m[r][3]};
        };
        auto add = [&](const std::array<float, 4>& a, const std::array<float, 4>& b) {
            return std::array<float, 4>{a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
        };
        auto sub = [&](const std::array<float, 4>& a, const std::array<float, 4>& b) {
            return std::array<float, 4>{a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]};
        };
        auto make = [&](const std::array<float, 4>& eq) {
            Plane p;
            p.n = sr::math::Vec3{eq[0], eq[1], eq[2]};
            p.d = eq[3];
            float len = sr::math::length(p.n);
            if (len > 0.0f) {
                p.n = p.n / len;
                p.d /= len;
            }
            return p;
        };

        auto r0 = row(0);
        auto r1 = row(1);
        auto r2 = row(2);
        auto r3 = row(3);

        Frustum f;
        f.planes[0] = make(add(r3, r0)); // left:  x + w >= 0
        f.planes[1] = make(sub(r3, r0)); // right: -x + w >= 0
        f.planes[2] = make(add(r3, r1)); // bottom: y + w >= 0
        f.planes[3] = make(sub(r3, r1)); // top:   -y + w >= 0
        f.planes[4] = make(add(r3, r2)); // near:  z + w >= 0
        f.planes[5] = make(sub(r3, r2)); // far:   -z + w >= 0
        return f;
    }

    bool sphere_visible(const sr::math::Vec3& c, float r) const {
        for (const auto& p : planes) {
            float dist = sr::math::dot(p.n, c) + p.d;
            if (dist < -r)
                return false;
        }
        return true;
    }
};

} // namespace sr::render
