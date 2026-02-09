#pragma once

#include "sr/math/vec3.hpp"
#include "sr/render/renderer.hpp"

namespace sr::scene {

struct FlyCamera {
  sr::math::Vec3 pos{0.0f, 6.0f, 14.0f};
  float yaw = 3.14159265f;   // radians
  float pitch = -0.25f;      // radians
  bool mouse_look = true;

  float move_speed = 6.0f;   // units/sec
  float fast_mul = 3.0f;     // when ctrl is held
  float mouse_sens = 0.0025f;

  void apply_mouse(int dx, int dy);
  void apply_keys(const uint8_t* keys, float dt);

  sr::render::Camera to_camera(float fov_y_rad, float z_near, float z_far) const;
};

}  // namespace sr::scene

