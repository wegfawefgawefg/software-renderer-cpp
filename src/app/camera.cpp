#include "app/camera.hpp"

#include "sr/math/vec3.hpp"

#include <algorithm>
#include <cmath>

namespace app {
namespace {

static float clamp01(float x) { return std::clamp(x, 0.0f, 1.0f); }

static float smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }

static float exp_smooth(float current, float target, float speed, float dt) {
    // First-order low-pass: critical for "smooth return" behavior.
    float a = 1.0f - std::exp(-std::max(0.0f, speed) * std::max(0.0f, dt));
    return current + (target - current) * a;
}

} // namespace

sr::render::Camera update_camera(sr::scene::PlayerController& player, float& status_alpha,
                                 const Settings& settings, float fov_y_rad, float z_near,
                                 float z_far, bool status_held, float dt) {
    const float target = status_held ? 1.0f : 0.0f;
    status_alpha = exp_smooth(status_alpha, target, settings.status_cam_blend_speed, dt);
    float t = smoothstep(clamp01(status_alpha));

    // Normal camera (AFR-style).
    sr::render::Camera normal = player.to_camera(fov_y_rad, z_near, z_far);

    // Status camera: orbit around player to put them on left side of screen.
    // We approximate "screen composition" by looking slightly to the player's right.
    float cy = std::cos(player.yaw);
    float sy = std::sin(player.yaw);
    sr::math::Vec3 forward{sy, 0.0f, cy};
    sr::math::Vec3 up{0.0f, 1.0f, 0.0f};
    sr::math::Vec3 right = sr::math::normalize(sr::math::cross(forward, up));

    // Swing arc around the player by interpolating an orbit angle.
    // Angle 0 is "behind" the player, -pi/2 is to the left.
    const float behind_angle = player.yaw + 3.14159265f;
    const float status_angle = player.yaw + settings.status_cam_angle_offset_rad;
    float ang = behind_angle + (status_angle - behind_angle) * t;

    float r = 6.0f + (settings.status_cam_orbit_radius - 6.0f) * t;
    float h = 2.0f + (settings.status_cam_orbit_height - 2.0f) * t;

    sr::math::Vec3 orbit_dir{std::sin(ang), 0.0f, std::cos(ang)};
    sr::render::Camera status;
    status.eye = player.pos + orbit_dir * r + sr::math::Vec3{0.0f, h, 0.0f};
    status.target = player.pos + right * settings.status_cam_target_right +
                    sr::math::Vec3{0.0f, settings.status_cam_target_up, 0.0f};
    status.up = up;
    status.fov_y_rad = fov_y_rad;
    status.z_near = z_near;
    status.z_far = z_far;

    // Blend cameras (eye/target only). This preserves a nice 3D swing arc while maintaining
    // stable camera behavior at endpoints.
    sr::render::Camera out = normal;
    out.eye = normal.eye + (status.eye - normal.eye) * t;
    out.target = normal.target + (status.target - normal.target) * t;
    out.up = up;
    return out;
}

} // namespace app

