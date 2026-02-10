#pragma once

namespace app {

// Centralized tuning constants to avoid magic numbers scattered across modules.
// Keep these as plain values; later we can load them from a config file or CLI flags.
struct Settings {
    // Camera.
    float fov_deg = 65.0f;
    float z_near = 0.1f;
    float z_far = 5000.0f;

    // World scaling.
    float mario_height_units = 1.0f; // "1 meter"
    float castle_width_marios = 100.0f;

    // Physics.
    float player_radius = 0.35f;
    // Walkable slope threshold: treat contacts with normal.y > this as "ground".
    // This is cos(max_slope_angle_deg). For 45 degrees, cos(45deg) ~= 0.707.
    float ground_normal_y = 0.707f;
    float ground_hysteresis = 0.08f; // allow slightly worse normals before dropping ground
    float ground_snap_dist = 0.10f; // how far below feet we still "snap" to ground
    float max_step_up = 0.25f;      // allow stepping up small ledges/stairs
    float jump_coyote_time = 0.10f; // seconds
    float max_dt = 0.05f;
    int max_substeps = 8;
    float min_substep = 0.05f;

    // Collision grid.
    float collider_cell_size = 1.25f;

    // Status camera (hold Tab).
    float status_cam_blend_speed = 4.0f; // higher = snappier
    float status_cam_orbit_radius = 2.75f;
    float status_cam_orbit_height = 1.55f;
    float status_cam_angle_offset_rad = 0.0f; // 0 = in front of the player (look back at face)
    float status_cam_target_right = 0.75f;    // look a bit to player's right -> player appears left
    float status_cam_target_up = 1.15f;       // bias up so the player sits lower in frame

    // Animation tuning.
    float run_anim_speed_mul = 1.35f; // scaled by player move speed (sprint animates faster)
};

} // namespace app
