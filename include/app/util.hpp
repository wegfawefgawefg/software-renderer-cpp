#pragma once

#include "sr/math/vec3.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace app {

inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

inline SDL_Rect centered_letterbox_rect(int dst_w, int dst_h, int src_w, int src_h) {
    SDL_Rect r{0, 0, dst_w, dst_h};
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0)
        return r;

    const float src_aspect = float(src_w) / float(src_h);
    const float dst_aspect = float(dst_w) / float(dst_h);
    if (dst_aspect > src_aspect) {
        r.h = dst_h;
        r.w = int(std::lround(float(dst_h) * src_aspect));
    } else {
        r.w = dst_w;
        r.h = int(std::lround(float(dst_w) / src_aspect));
    }
    r.x = (dst_w - r.w) / 2;
    r.y = (dst_h - r.h) / 2;
    return r;
}

inline void compute_bounds(const std::vector<sr::math::Vec3>& pos, sr::math::Vec3& out_mn,
                           sr::math::Vec3& out_mx) {
    if (pos.empty()) {
        out_mn = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        out_mx = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        return;
    }
    sr::math::Vec3 mn = pos[0];
    sr::math::Vec3 mx = pos[0];
    for (const auto& p : pos) {
        mn.x = std::min(mn.x, p.x);
        mn.y = std::min(mn.y, p.y);
        mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x);
        mx.y = std::max(mx.y, p.y);
        mx.z = std::max(mx.z, p.z);
    }
    out_mn = mn;
    out_mx = mx;
}

} // namespace app
