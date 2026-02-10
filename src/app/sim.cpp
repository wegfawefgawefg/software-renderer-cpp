#include "app/sim.hpp"

#include "app/camera.hpp"

#include "sr/anim/skinning.hpp"
#include "sr/math/mat4.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>

namespace app {

void step_game(Game& g, const Settings& settings, const AppToggles& toggles, const uint8_t* keys,
               float dt, int mouse_dx, int mouse_dy) {
    dt = std::min(dt, settings.max_dt);

    const bool was_grounded = g.player.grounded;
    bool jumped_this_frame = false;

    if (toggles.mouse_look)
        g.player.apply_mouse(mouse_dx, mouse_dy);

    // Horizontal movement (arcade style): set desired velocity on XZ plane.
    sr::math::Vec3 move = g.player.move_dir_from_keys(keys);
    float speed = g.player.move_speed;
    if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
        speed *= g.player.sprint_mul;
    const sr::math::Vec3 desired_vel{move.x * speed, 0.0f, move.z * speed};
    g.player.vel.x = desired_vel.x;
    g.player.vel.z = desired_vel.z;

    const float move_mag = std::sqrt(desired_vel.x * desired_vel.x + desired_vel.z * desired_vel.z);

    // Jump.
    if ((keys[SDL_SCANCODE_SPACE]) && g.player.grounded) {
        g.player.vel.y = g.player.jump_speed;
        g.player.grounded = false;
        jumped_this_frame = true;
    }

    // Gravity.
    if (toggles.gravity_enabled) {
        g.player.vel.y -= g.player.gravity * dt;
    } else {
        g.player.vel.y = 0.0f;
        g.player.grounded = true;
    }

    // Substepping to prevent tunneling at low FPS / high speeds.
    const float max_step = std::max(settings.min_substep, g.player.radius * 0.5f);
    float travel = sr::math::length(g.player.vel) * dt;
    int steps = int(std::ceil(travel / max_step));
    steps = std::clamp(steps, 1, settings.max_substeps);
    float sdt = dt / float(steps);

    // Track best contact normal (most "ground-like") over this frame.
    // We use this to decide grounded state and slope friction.
    float best_ny = -1.0f;
    sr::math::Vec3 best_n{0.0f, 1.0f, 0.0f};
    // Track a steep contact normal to prevent "walking up walls".
    bool had_steep_contact = false;
    sr::math::Vec3 steep_n{0.0f, 1.0f, 0.0f};

    for (int s = 0; s < steps; ++s) {
        g.player.pos = g.player.pos + g.player.vel * sdt;

        auto c = g.world_col.resolve_sphere(g.player.pos, g.player.radius, &g.player.vel, 3);
        if (c.hit) {
            if (c.normal.y > best_ny) {
                best_ny = c.normal.y;
                best_n = c.normal;
            }
            if (c.normal.y < settings.ground_normal_y) {
                had_steep_contact = true;
                steep_n = c.normal;
            }
        }
    }

    // Grounded if we had a sufficiently up-facing contact this frame (and we didn't just jump).
    // Also require not moving upward (prevents "grounded" at jump start).
    g.player.grounded = (!jumped_this_frame) && (best_ny > settings.ground_normal_y) &&
                        (g.player.vel.y <= 0.25f);

    // Walk friction (no sliding) + move along slope plane when grounded.
    if (g.player.grounded) {
        g.player.vel.y = 0.0f;
        if (move_mag <= 0.1f) {
            g.player.vel.x = 0.0f;
            g.player.vel.z = 0.0f;
        } else {
            sr::math::Vec3 v{g.player.vel.x, 0.0f, g.player.vel.z};
            v = v - best_n * sr::math::dot(v, best_n);
            g.player.vel.x = v.x;
            g.player.vel.z = v.z;
        }
    }

    // If we are pushing into a steep surface, remove the uphill component so we can't "walk" up
    // near-vertical ramps due to the collision solver pushing us upward.
    if (!g.player.grounded && had_steep_contact && move_mag > 0.1f) {
        sr::math::Vec3 uphill_dir = sr::math::normalize(sr::math::Vec3{-steep_n.x, 0.0f, -steep_n.z});
        float upcomp = g.player.vel.x * uphill_dir.x + g.player.vel.z * uphill_dir.z;
        if (upcomp > 0.0f) {
            g.player.vel.x -= uphill_dir.x * upcomp;
            g.player.vel.z -= uphill_dir.z * upcomp;
        }
    }

    // Camera follows the player.
    const bool status_held = keys[SDL_SCANCODE_TAB] != 0;
    g.scene.camera = app::update_camera(g.player, g.status_cam_alpha, settings, g.fov, g.z_near,
                                        g.z_far, status_held, dt);

    // Update player entity transform (translate + yaw + model correction).
    // Player physics `pos` is the sphere center. Render mesh is authored with its feet at the
    // origin after `player_model_offset`, so render at (center - up*radius).
    sr::math::Vec3 render_pos = g.player.pos - sr::math::Vec3{0.0f, g.player.radius, 0.0f};
    sr::math::Mat4 t = sr::math::Mat4::translate(render_pos);

    // Make the character face the direction they're moving.
    if (move_mag > 0.1f) {
        g.model_yaw = std::atan2(g.player.vel.x, g.player.vel.z);
    }
    sr::math::Mat4 r = sr::math::Mat4::rotate_y(g.model_yaw);
    g.scene.entities[g.player_entity].transform =
        sr::math::mul(t, sr::math::mul(r, g.player_model_offset));

    // Animation selection:
    // - airborne => jump
    // - moving   => run
    // - else     => idle
    //
    // Keep anim time continuous within a clip. Reset on clip change.
    int want = 0;
    // Avoid animation flicker when we momentarily lose ground contact while going down slopes.
    // Only treat as airborne if vertical speed is meaningful.
    const float vy = g.player.vel.y;
    const bool really_airborne = (!g.player.grounded) && (vy > 0.75f || vy < -0.75f);
    if (really_airborne) {
        want = 2;
    } else if (move_mag > 0.1f) {
        want = 1;
    }

    // If we just left the ground (jump start), restart the jump animation so it reads well.
    const bool just_left_ground = was_grounded && !g.player.grounded;
    if (just_left_ground && want == 2) {
        g.active_anim = want;
        g.anim_time = 0.0f;
    } else if (want != g.active_anim) {
        g.active_anim = want;
        g.anim_time = 0.0f;
    } else {
        float anim_dt = dt;
        if (want == 1) {
            // Scale run cycle by actual speed so sprinting looks like faster legs.
            float base = std::max(1e-6f, g.player.move_speed);
            float speed_factor = std::clamp(move_mag / base, 0.25f, 3.0f);
            anim_dt *= speed_factor * settings.run_anim_speed_mul;
        }
        g.anim_time += anim_dt;
    }

    const sr::assets::AnimationClip* clip = &g.anim_idle;
    if (g.active_anim == 1)
        clip = &g.anim_run;
    if (g.active_anim == 2)
        clip = &g.anim_jump;

    if (g.player_skin)
        sr::anim::skin_model(*g.player_skin, *clip, g.anim_time);
}

} // namespace app
