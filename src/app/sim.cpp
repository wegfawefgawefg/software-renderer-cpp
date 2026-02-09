#include "app/sim.hpp"

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

    if (toggles.mouse_look)
        g.player.apply_mouse(mouse_dx, mouse_dy);

    // Horizontal movement (arcade style): set desired velocity on XZ plane.
    sr::math::Vec3 move = g.player.move_dir_from_keys(keys);
    float speed = g.player.move_speed;
    if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
        speed *= g.player.sprint_mul;
    g.player.vel.x = move.x * speed;
    g.player.vel.z = move.z * speed;

    // Jump.
    if ((keys[SDL_SCANCODE_SPACE]) && g.player.grounded) {
        g.player.vel.y = g.player.jump_speed;
        g.player.grounded = false;
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

    bool grounded_this_frame = false;
    for (int s = 0; s < steps; ++s) {
        g.player.pos = g.player.pos + g.player.vel * sdt;

        auto c = g.world_col.resolve_sphere(g.player.pos, g.player.radius, &g.player.vel, 3);
        if (c.hit && c.normal.y > settings.ground_normal_y)
            grounded_this_frame = true;
    }
    g.player.grounded = grounded_this_frame;

    // Camera follows the player.
    g.scene.camera = g.player.to_camera(g.fov, g.z_near, g.z_far);

    // Update player entity transform (translate + yaw + model correction).
    sr::math::Mat4 t = sr::math::Mat4::translate(g.player.pos);
    sr::math::Mat4 r = sr::math::Mat4::rotate_y(g.player.yaw);
    g.scene.entities[g.player_entity].transform =
        sr::math::mul(t, sr::math::mul(r, g.player_model_offset));

    // Animation selection:
    // - airborne => jump
    // - moving   => run
    // - else     => idle
    //
    // Keep anim time continuous within a clip. Reset on clip change.
    float move_mag = std::sqrt(g.player.vel.x * g.player.vel.x + g.player.vel.z * g.player.vel.z);
    int want = 0;
    if (!g.player.grounded) {
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
        g.anim_time += dt;
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
