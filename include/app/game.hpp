#pragma once

#include "sr/assets/asset_store.hpp"
#include "sr/assets/animation.hpp"
#include "sr/assets/model.hpp"
#include "sr/assets/skinned_model.hpp"
#include "sr/math/mat4.hpp"
#include "sr/physics/triangle_collider.hpp"
#include "sr/scene/player_controller.hpp"
#include "sr/scene/scene.hpp"

#include <memory>

#include "app/settings.hpp"

namespace app {

struct Game {
    std::shared_ptr<sr::assets::Model> castle;
    std::shared_ptr<sr::assets::SkinnedModel> player_skin;

    sr::scene::Scene scene;
    sr::physics::TriangleMeshCollider world_col;

    sr::scene::PlayerController player;
    sr::math::Mat4 player_model_offset = sr::math::Mat4::identity();
    sr::assets::AnimationClip anim_idle;
    sr::assets::AnimationClip anim_run;
    sr::assets::AnimationClip anim_jump;
    float anim_time = 0.0f;
    int active_anim = 0; // 0=idle, 1=run, 2=jump

    // Camera/projection params.
    float fov = 65.0f * 3.14159265f / 180.0f;
    float z_near = 0.1f;
    float z_far = 5000.0f;

    // Camera mode (hold Tab for status camera).
    float status_cam_alpha = 0.0f; // 0=normal, 1=status

    // Entity slots.
    int castle_entity = 0;
    int player_entity = 1;
};

Game init_game(sr::assets::AssetStore& store);
Game init_game(sr::assets::AssetStore& store, const Settings& settings);

} // namespace app
