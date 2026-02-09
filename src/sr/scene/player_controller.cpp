#include "sr/scene/player_controller.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>

namespace sr::scene {

void PlayerController::apply_mouse(int dx, int dy) {
  yaw += float(dx) * mouse_sens;
  pitch += float(-dy) * mouse_sens;
  pitch = std::clamp(pitch, -1.0f, 0.8f);
}

sr::math::Vec3 PlayerController::move_dir_from_keys(const uint8_t* keys) const {
  // Ground-plane basis from yaw.
  float cy = std::cos(yaw);
  float sy = std::sin(yaw);
  sr::math::Vec3 forward{sy, 0.0f, cy};
  sr::math::Vec3 right{cy, 0.0f, -sy};

  sr::math::Vec3 move{0.0f, 0.0f, 0.0f};
  if (keys[SDL_SCANCODE_W]) move = move + forward;
  if (keys[SDL_SCANCODE_S]) move = move - forward;
  if (keys[SDL_SCANCODE_D]) move = move + right;
  if (keys[SDL_SCANCODE_A]) move = move - right;

  float mag = sr::math::length(move);
  if (mag > 0.0f) move = move / mag;
  return move;
}

sr::render::Camera PlayerController::to_camera(float fov_y_rad, float z_near, float z_far) const {
  float cy = std::cos(yaw);
  float sy = std::sin(yaw);
  float cp = std::cos(pitch);
  float sp = std::sin(pitch);

  // Camera forward in world space.
  sr::math::Vec3 forward{cp * sy, sp, cp * cy};
  forward = sr::math::normalize(forward);

  sr::render::Camera cam;
  cam.eye = pos - forward * cam_distance + sr::math::Vec3{0.0f, cam_height, 0.0f};
  cam.target = pos + sr::math::Vec3{0.0f, target_height, 0.0f};
  cam.up = sr::math::Vec3{0.0f, 1.0f, 0.0f};
  cam.fov_y_rad = fov_y_rad;
  cam.z_near = z_near;
  cam.z_far = z_far;
  return cam;
}

}  // namespace sr::scene

