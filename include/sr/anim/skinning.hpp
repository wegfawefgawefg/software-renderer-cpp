#pragma once

#include "sr/assets/animation.hpp"
#include "sr/assets/skinned_model.hpp"
#include "sr/math/transform.hpp"
#include "sr/math/trs.hpp"

#include <vector>

namespace sr::anim {

// CPU skinning (Linear Blend Skinning).
// Updates `m.model->mesh.positions` in-place.
inline void skin_model(sr::assets::SkinnedModel& m, const sr::assets::AnimationClip& clip,
                       float time_sec) {
    if (!m.model)
        return;
    auto& mesh = m.model->mesh;
    const size_t n = mesh.positions.size();
    if (m.bind_positions.size() != n || m.skin.size() != n)
        return;
    const size_t joints = m.skeleton.joints.size();
    if (joints == 0)
        return;

    // Sample local pose for each joint (clip if available, else rest pose).
    std::vector<sr::math::Mat4> global(joints);
    for (size_t j = 0; j < joints; ++j) {
        const auto& sj = m.skeleton.joints[j];
        sr::math::Trs local = clip.sample_joint(uint32_t(j), time_sec, sj.rest_local);
        sr::math::Mat4 lm = sr::math::trs_to_mat4(local);
        if (sj.parent >= 0) {
            global[j] = sr::math::mul(global[size_t(sj.parent)], lm);
        } else {
            global[j] = lm;
        }
    }

    // Precompute skin matrices.
    std::vector<sr::math::Mat4> skin_mats(joints);
    for (size_t j = 0; j < joints; ++j) {
        skin_mats[j] =
            sr::math::mul(m.world_to_model, sr::math::mul(global[j], m.skeleton.joints[j].inv_bind));
    }

    // Skin positions.
    for (size_t i = 0; i < n; ++i) {
        const auto& inf = m.skin[i];
        const sr::math::Vec3 p = m.bind_positions[i];

        sr::math::Vec3 out{0.0f, 0.0f, 0.0f};
        for (int k = 0; k < 4; ++k) {
            float w = inf.weight[k];
            if (w <= 0.0f)
                continue;
            uint16_t j = inf.joint[k];
            if (j >= joints)
                continue;
            out = out + sr::math::transform_point(skin_mats[j], p) * w;
        }
        mesh.positions[i] = out;
    }
}

} // namespace sr::anim

