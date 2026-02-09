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
    float ground_normal_y = 0.55f;
    float max_dt = 0.05f;
    int max_substeps = 8;
    float min_substep = 0.05f;

    // Collision grid.
    float collider_cell_size = 1.25f;
};

} // namespace app
