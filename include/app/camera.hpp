#pragma once

#include "app/settings.hpp"
#include "sr/render/renderer.hpp"
#include "sr/scene/player_controller.hpp"

namespace app {

// Updates and returns the current camera (normal follow vs status camera when holding Tab).
sr::render::Camera update_camera(sr::scene::PlayerController& player, float& status_alpha,
                                 const Settings& settings, float fov_y_rad, float z_near,
                                 float z_far, bool status_held, float dt);

} // namespace app

