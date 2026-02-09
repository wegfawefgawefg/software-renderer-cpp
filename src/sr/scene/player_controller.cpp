#include "sr/scene/player_controller.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>

namespace sr::scene {

void PlayerController::apply_mouse(int dx, int dy) {
    // +dx should turn right (positive yaw turns forward vector toward +X).
    yaw += float(dx) * mouse_sens;
    pitch += float(-dy) * mouse_sens;
    pitch = std::clamp(pitch, -1.0f, 0.8f);
}

sr::math::Vec3 PlayerController::move_dir_from_keys(const uint8_t* keys) const {
    // Ground-plane basis from yaw.
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    sr::math::Vec3 forward{sy, 0.0f, cy};
    sr::math::Vec3 world_up{0.0f, 1.0f, 0.0f};
    sr::math::Vec3 right = sr::math::normalize(sr::math::cross(world_up, forward));

    sr::math::Vec3 move{0.0f, 0.0f, 0.0f};
    if (keys[SDL_SCANCODE_W])
        move = move + forward;
    if (keys[SDL_SCANCODE_S])
        move = move - forward;
    if (keys[SDL_SCANCODE_D])
        move = move + right;
    if (keys[SDL_SCANCODE_A])
        move = move - right;

    float mag = sr::math::length(move);
    if (mag > 0.0f)
        move = move / mag;
    return move;
}

sr::render::Camera PlayerController::to_camera(float fov_y_rad, float z_near, float z_far) const {
    // Mirror AFR's third-person camera exactly:
    //   flat_forward = normalize(Vec3(sin(yaw), 0, cos(yaw)))
    //   cam_forward  = normalize(Vec3(cos(pitch)*sin(yaw), sin(pitch), cos(pitch)*cos(yaw)))
    //   cam_pos      = mario_pos - flat_forward*6 + (0,2,0)
    //   cam_target   = mario_pos + (0,1,0) + cam_forward*2
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);

    sr::math::Vec3 flat_forward = sr::math::normalize(sr::math::Vec3{sy, 0.0f, cy});
    sr::math::Vec3 cam_forward = sr::math::normalize(sr::math::Vec3{cp * sy, sp, cp * cy});

    sr::render::Camera cam;
    const float follow_dist = 6.0f;
    const float follow_height = 2.0f;
    cam.eye = pos - flat_forward * follow_dist + sr::math::Vec3{0.0f, follow_height, 0.0f};
    cam.target = pos + sr::math::Vec3{0.0f, 1.0f, 0.0f} + cam_forward * 2.0f;
    cam.up = sr::math::Vec3{0.0f, 1.0f, 0.0f};
    cam.fov_y_rad = fov_y_rad;
    cam.z_near = z_near;
    cam.z_far = z_far;
    return cam;
}

} // namespace sr::scene
