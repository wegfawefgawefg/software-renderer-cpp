#pragma once

#include "sr/math/vec3.hpp"
#include "sr/render/renderer.hpp"

#include <cstdint>

namespace sr::scene {

struct PlayerController {
    // Player physical state.
    sr::math::Vec3 pos{0.0f, 2.0f, 0.0f};
    sr::math::Vec3 vel{0.0f, 0.0f, 0.0f};
    float radius = 0.4f;
    bool grounded = false;

    // Orientation (yaw controls movement direction; pitch is for camera only).
    float yaw = 0.0f;    // radians; yaw=0 faces +Z
    float pitch = -0.2f; // radians

    // Tuning.
    float move_speed = 4.0f;    // units/sec
    float sprint_mul = 2.0f;    // when shift held
    float mouse_sens = 0.0025f; // radians/pixel
    float gravity = 18.0f;      // units/sec^2
    float jump_speed = 6.5f;    // units/sec

    void apply_mouse(int dx, int dy);

    // Compute desired horizontal move direction in world space from key state.
    sr::math::Vec3 move_dir_from_keys(const uint8_t* keys) const;

    sr::render::Camera to_camera(float fov_y_rad, float z_near, float z_far) const;
};

} // namespace sr::scene
