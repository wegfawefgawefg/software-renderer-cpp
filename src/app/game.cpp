#include "app/game.hpp"

#include "app/util.hpp"

#include "sr/assets/gltf_model_loader.hpp"
#include "sr/assets/obj_model_loader.hpp"

#include <algorithm>

namespace app {

Game init_game(sr::assets::AssetStore& store) {
    Game g;

    g.castle = std::make_shared<sr::assets::Model>(
        sr::assets::load_obj_model("./assets/models/peaches_castle.obj", store,
                                   sr::assets::ObjModelLoadOptions{
                                       .flip_v = true,
                                       .front_face_ccw = false,
                                       .double_sided = false,
                                   }));

    g.mario = std::make_shared<sr::assets::Model>(sr::assets::load_gltf_model(
        "./assets/models/mario-64-mario/source/prototype_mario_super_mario_64/scene.gltf", store,
        sr::assets::GltfModelLoadOptions{
            .flip_v = false,
            .front_face_ccw = true,
            .double_sided = false,
        }));

    // Castle entity transform matches AFR scaling:
    // - Mario height becomes 1.0 world unit ("1 meter")
    // - Castle is ~100 Marios wide
    // - Castle recentered so base is on y=0 and centered on origin in XZ.
    sr::math::Vec3 castle_mn, castle_mx;
    app::compute_bounds(g.castle->mesh.positions, castle_mn, castle_mx);
    float castle_w = std::max(1e-6f, castle_mx.x - castle_mn.x);
    float castle_scale = 100.0f / castle_w;
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

    sr::scene::Entity mario_ent;
    mario_ent.model = g.mario;
    mario_ent.transform = sr::math::Mat4::identity();
    g.scene.entities.push_back(mario_ent);

    // Build collider from the scaled/recentered castle.
    g.world_col.build_from_model(*g.castle, castle_xform,
                                 sr::physics::TriangleMeshCollider::BuildOptions{
                                     .cell_size = 1.25f,
                                     .two_sided = false,
                                 });

    // Mario model correction: scale to 1.0 unit tall and pivot at bottom-center.
    sr::math::Vec3 mario_mn, mario_mx;
    app::compute_bounds(g.mario->mesh.positions, mario_mn, mario_mx);
    float mario_h = std::max(1e-6f, mario_mx.y - mario_mn.y);
    float mario_scale = 1.0f / mario_h;
    sr::math::Vec3 mario_pivot{
        (mario_mn.x + mario_mx.x) * 0.5f,
        mario_mn.y,
        (mario_mn.z + mario_mx.z) * 0.5f,
    };
    sr::math::Vec3 mario_pivot_t{
        -mario_pivot.x * mario_scale,
        -mario_pivot.y * mario_scale,
        -mario_pivot.z * mario_scale,
    };
    g.mario_model_offset =
        sr::math::mul(sr::math::Mat4::translate(mario_pivot_t),
                      sr::math::Mat4::scale(sr::math::Vec3{mario_scale, mario_scale, mario_scale}));

    // Player.
    g.player.radius = 0.35f;
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
