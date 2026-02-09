#include "sr/assets/animation.hpp"

#include <algorithm>
#include <cmath>

namespace sr::assets {

sr::math::Trs AnimationClip::sample_joint(uint32_t joint, float t,
                                          const sr::math::Trs& fallback) const {
    if (num_joints == 0 || joint >= num_joints)
        return fallback;
    uint32_t frames = num_frames();
    if (frames == 0 || samples.empty())
        return fallback;

    if (duration <= 0.0f || sample_rate <= 0.0f) {
        // Treat as static pose (first frame).
        return samples[size_t(joint)];
    }

    // Loop.
    float lt = std::fmod(t, duration);
    if (lt < 0.0f)
        lt += duration;

    float f = lt * sample_rate;
    uint32_t i0 = uint32_t(std::floor(f));
    uint32_t i1 = i0 + 1;
    float frac = f - float(i0);

    if (i0 >= frames)
        i0 = frames - 1;
    if (i1 >= frames)
        i1 = frames - 1;

    const sr::math::Trs& a = samples[size_t(i0) * num_joints + joint];
    const sr::math::Trs& b = samples[size_t(i1) * num_joints + joint];
    return sr::math::lerp(a, b, frac);
}

} // namespace sr::assets

