#pragma once

#include "sr/assets/model.hpp"
#include "sr/assets/skeleton.hpp"
#include "sr/math/vec3.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace sr::assets {

struct SkinInfluence4 {
    uint16_t joint[4]{0, 0, 0, 0};
    float weight[4]{1.0f, 0.0f, 0.0f, 0.0f};
};

struct SkinnedModel {
    // Deformed model that the renderer consumes.
    std::shared_ptr<Model> model;

    // Bind-pose positions in mesh-geometry space, parallel to model->mesh.positions.
    std::vector<sr::math::Vec3> bind_positions;

    // Skinning influences per vertex, parallel to model->mesh.positions.
    std::vector<SkinInfluence4> skin;

    Skeleton skeleton;

    // Converts skin matrices from FBX/world space back into mesh-geometry (model-local) space.
    // This is typically inverse(mesh_node.geometry_to_world) in the bind pose.
    sr::math::Mat4 world_to_model = sr::math::Mat4::identity();
};

} // namespace sr::assets

