#pragma once

#include "sr/assets/asset_store.hpp"
#include "sr/assets/model.hpp"
#include "sr/math/mat4.hpp"
#include "sr/physics/triangle_collider.hpp"
#include "sr/scene/player_controller.hpp"
#include "sr/scene/scene.hpp"

#include <memory>

namespace app {

struct Game {
    std::shared_ptr<sr::assets::Model> castle;
    std::shared_ptr<sr::assets::Model> mario;

    sr::scene::Scene scene;
    sr::physics::TriangleMeshCollider world_col;

    sr::scene::PlayerController player;
    sr::math::Mat4 mario_model_offset = sr::math::Mat4::identity();

    // Camera/projection params.
    float fov = 65.0f * 3.14159265f / 180.0f;
    float z_near = 0.1f;
    float z_far = 5000.0f;

    // Entity slots.
    int castle_entity = 0;
    int mario_entity = 1;
};

Game init_game(sr::assets::AssetStore& store);

} // namespace app
