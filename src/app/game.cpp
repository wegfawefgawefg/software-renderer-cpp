#include "app/game.hpp"

#include "app/util.hpp"

#include "sr/assets/fbx_skinned_model_loader.hpp"
#include "sr/assets/gltf_model_loader.hpp"
#include "sr/assets/obj_model_loader.hpp"

#include "sr/math/transform.hpp"

#include <algorithm>

namespace app {

Game init_game(sr::assets::AssetStore& store) {
    Settings settings;
    return init_game(store, settings);
}

Game init_game(sr::assets::AssetStore& store, const Settings& settings) {
    Game g;

    g.castle = std::make_shared<sr::assets::Model>(
        sr::assets::load_obj_model("./assets/models/peaches_castle.obj", store,
                                   sr::assets::ObjModelLoadOptions{
                                       .flip_v = true,
                                       .front_face_ccw = false,
                                       .double_sided = false,
                                   }));

    // Animated player (Kenney pack). This gives us a real skeleton + clips (FBX).
    g.player_skin = std::make_shared<sr::assets::SkinnedModel>(sr::assets::load_fbx_skinned_model(
        "./assets/models/kenney/characterMedium.fbx", store,
        sr::assets::FbxSkinnedModelLoadOptions{
            .flip_v = true,
            .front_face_ccw = true,
            .double_sided = false,
            .override_diffuse_texture = "./assets/textures/kenney/survivorMaleB.png",
        }));
    g.anim_idle = sr::assets::load_fbx_animation_clip("./assets/anims/kenney/idle.fbx",
                                                      g.player_skin->skeleton, "idle", 30.0f);
    g.anim_run = sr::assets::load_fbx_animation_clip("./assets/anims/kenney/run.fbx",
                                                     g.player_skin->skeleton, "run", 30.0f);
    g.anim_jump = sr::assets::load_fbx_animation_clip("./assets/anims/kenney/jump.fbx",
                                                      g.player_skin->skeleton, "jump", 30.0f);

    g.fov = settings.fov_deg * 3.14159265f / 180.0f;
    g.z_near = settings.z_near;
    g.z_far = settings.z_far;

    // Castle entity transform matches AFR scaling:
    // - Mario height becomes 1.0 world unit ("1 meter")
    // - Castle is ~100 Marios wide
    // - Castle recentered so base is on y=0 and centered on origin in XZ.
    sr::math::Vec3 castle_mn, castle_mx;
    app::compute_bounds(g.castle->mesh.positions, castle_mn, castle_mx);
    float castle_w = std::max(1e-6f, castle_mx.x - castle_mn.x);
    float castle_scale = settings.castle_width_marios * settings.mario_height_units / castle_w;
    sr::math::Vec3 castle_center{
        (castle_mn.x + castle_mx.x) * 0.5f,
        (castle_mn.y + castle_mx.y) * 0.5f,
        (castle_mn.z + castle_mx.z) * 0.5f,
    };
    sr::math::Vec3 castle_t{
        -castle_center.x * castle_scale,
        -castle_mn.y * castle_scale,
        -castle_center.z * castle_scale,
    };
    sr::math::Mat4 castle_recenter = sr::math::Mat4::translate(castle_t);
    sr::math::Mat4 castle_s =
        sr::math::Mat4::scale(sr::math::Vec3{castle_scale, castle_scale, castle_scale});
    sr::math::Mat4 castle_xform = sr::math::mul(castle_recenter, castle_s);

    sr::scene::Entity castle_ent;
    castle_ent.model = g.castle;
    castle_ent.transform = castle_xform;
    g.scene.entities.push_back(castle_ent);

    sr::scene::Entity player_ent;
    player_ent.model = g.player_skin->model;
    player_ent.transform = sr::math::Mat4::identity();
    g.scene.entities.push_back(player_ent);

    // Build collider from the scaled/recentered castle.
    g.world_col.build_from_model(*g.castle, castle_xform,
                                 sr::physics::TriangleMeshCollider::BuildOptions{
                                     .cell_size = settings.collider_cell_size,
                                     .two_sided = false,
                                 });

    // Player model correction:
    // - Kenney FBX characters are authored Z-up; rotate to our Y-up world.
    // - Scale to `settings.mario_height_units` tall (1.0 = "1 meter")
    // - Pivot at bottom-center.
    const auto& player_mesh = g.player_skin->model->mesh;
    sr::math::Mat4 rot_fix = sr::math::Mat4::rotate_x(-3.14159265f * 0.5f);

    sr::math::Vec3 p_mn, p_mx;
    if (!player_mesh.positions.empty()) {
        sr::math::Vec3 mn = sr::math::transform_point(rot_fix, player_mesh.positions[0]);
        sr::math::Vec3 mx = mn;
        for (const auto& p : player_mesh.positions) {
            sr::math::Vec3 pr = sr::math::transform_point(rot_fix, p);
            mn.x = std::min(mn.x, pr.x);
            mn.y = std::min(mn.y, pr.y);
            mn.z = std::min(mn.z, pr.z);
            mx.x = std::max(mx.x, pr.x);
            mx.y = std::max(mx.y, pr.y);
            mx.z = std::max(mx.z, pr.z);
        }
        p_mn = mn;
        p_mx = mx;
    } else {
        p_mn = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        p_mx = sr::math::Vec3{1.0f, 1.0f, 1.0f};
    }

    float p_h = std::max(1e-6f, p_mx.y - p_mn.y);
    float p_scale = settings.mario_height_units / p_h;
    sr::math::Vec3 pivot_rot{
        (p_mn.x + p_mx.x) * 0.5f,
        p_mn.y,
        (p_mn.z + p_mx.z) * 0.5f,
    };
    sr::math::Vec3 pivot_t{-pivot_rot.x * p_scale, -pivot_rot.y * p_scale, -pivot_rot.z * p_scale};

    // Apply rotation first, then scale, then translate the scaled pivot to origin: T * S * R.
    g.player_model_offset =
        sr::math::mul(sr::math::Mat4::translate(pivot_t),
                      sr::math::mul(sr::math::Mat4::scale(sr::math::Vec3{p_scale, p_scale, p_scale}),
                                    rot_fix));

    // Player.
    g.player.radius = settings.player_radius;
    g.player.yaw = 3.14159265f;
    g.player.pitch = -0.25f;

    // Spawn: raycast down at origin, starting above the castle.
    float castle_h_world = (castle_mx.y - castle_mn.y) * castle_scale;
    float y0 = castle_h_world + 50.0f;
    {
        auto hit = g.world_col.raycast_down(0.0f, 0.0f, y0, y0 + 1000.0f);
        float ground_y = hit.hit ? hit.p.y : (castle_h_world + 1.0f);
        g.player.pos = sr::math::Vec3{0.0f, ground_y + g.player.radius + 0.02f, 0.0f};
    }

    // Camera for first frame.
    g.scene.camera = g.player.to_camera(g.fov, g.z_near, g.z_far);
    return g;
}

} // namespace app
