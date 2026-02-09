#pragma once

#include "sr/math/trs.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sr::assets {

struct AnimationClip {
    std::string name;
    float duration = 0.0f;    // seconds
    float sample_rate = 30.0f; // Hz
    uint32_t num_joints = 0;

    // Packed samples: frame-major, then joint index.
    // samples[frame * num_joints + joint]
    std::vector<sr::math::Trs> samples;

    uint32_t num_frames() const {
        if (num_joints == 0)
            return 0;
        return uint32_t(samples.size() / size_t(num_joints));
    }

    sr::math::Trs sample_joint(uint32_t joint, float t, const sr::math::Trs& fallback) const;
};

} // namespace sr::assets

