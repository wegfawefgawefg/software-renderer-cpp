#include "sr/scene/fly_camera.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>

namespace sr::scene {

void FlyCamera::apply_mouse(int dx, int dy) {
    yaw += float(dx) * mouse_sens;
    pitch += float(-dy) * mouse_sens;
    pitch = std::clamp(pitch, -1.25f, 1.25f);
}

void FlyCamera::apply_keys(const uint8_t* keys, float dt) {
    // Forward from yaw/pitch (right-handed; yaw=0 points +Z in our math).
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);

    sr::math::Vec3 forward{cp * sy, sp, cp * cy};
    forward = sr::math::normalize(forward);
    sr::math::Vec3 world_up{0.0f, 1.0f, 0.0f};
    sr::math::Vec3 right = sr::math::normalize(sr::math::cross(forward, world_up));

    sr::math::Vec3 move{0.0f, 0.0f, 0.0f};
    if (keys[SDL_SCANCODE_W])
        move = move + forward;
    if (keys[SDL_SCANCODE_S])
        move = move - forward;
    if (keys[SDL_SCANCODE_D])
        move = move + right;
    if (keys[SDL_SCANCODE_A])
        move = move - right;
    if (keys[SDL_SCANCODE_SPACE])
        move = move + world_up;
    if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
        move = move - world_up;

    float mag = sr::math::length(move);
    if (mag > 0.0f)
        move = move / mag;

    float speed = move_speed;
    if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
        speed *= fast_mul;

    pos = pos + move * (speed * dt);
}

sr::render::Camera FlyCamera::to_camera(float fov_y_rad, float z_near, float z_far) const {
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    sr::math::Vec3 forward{cp * sy, sp, cp * cy};
    forward = sr::math::normalize(forward);

    sr::render::Camera cam;
    cam.eye = pos;
    cam.target = pos + forward;
    cam.up = sr::math::Vec3{0.0f, 1.0f, 0.0f};
    cam.fov_y_rad = fov_y_rad;
    cam.z_near = z_near;
    cam.z_far = z_far;
    return cam;
}

} // namespace sr::scene
