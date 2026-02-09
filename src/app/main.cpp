#include "sr/assets/asset_store.hpp"
#include "sr/assets/gltf_model_loader.hpp"
#include "sr/assets/obj_model_loader.hpp"
#include "sr/gfx/depthbuffer.hpp"
#include "sr/gfx/font5x7.hpp"
#include "sr/gfx/framebuffer.hpp"
#include "sr/math/mat4.hpp"
#include "sr/math/transform.hpp"
#include "sr/physics/triangle_collider.hpp"
#include "sr/platform/sdl.hpp"
#include "sr/render/frustum.hpp"
#include "sr/render/renderer.hpp"
#include "sr/scene/player_controller.hpp"
#include "sr/scene/scene.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

static uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

static SDL_Rect centered_letterbox_rect(int dst_w, int dst_h, int src_w, int src_h) {
    // Preserve aspect ratio; center the scaled image inside the window.
    SDL_Rect r{0, 0, dst_w, dst_h};
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0)
        return r;

    const float src_aspect = float(src_w) / float(src_h);
    const float dst_aspect = float(dst_w) / float(dst_h);
    if (dst_aspect > src_aspect) {
        r.h = dst_h;
        r.w = int(std::lround(float(dst_h) * src_aspect));
    } else {
        r.w = dst_w;
        r.h = int(std::lround(float(dst_w) / src_aspect));
    }
    r.x = (dst_w - r.w) / 2;
    r.y = (dst_h - r.h) / 2;
    return r;
}

static void compute_bounds(const std::vector<sr::math::Vec3>& pos, sr::math::Vec3& out_mn,
                           sr::math::Vec3& out_mx) {
    if (pos.empty()) {
        out_mn = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        out_mx = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        return;
    }
    sr::math::Vec3 mn = pos[0];
    sr::math::Vec3 mx = pos[0];
    for (const auto& p : pos) {
        mn.x = std::min(mn.x, p.x);
        mn.y = std::min(mn.y, p.y);
        mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x);
        mx.y = std::max(mx.y, p.y);
        mx.z = std::max(mx.z, p.z);
    }
    out_mn = mn;
    out_mx = mx;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Always render sharp when scaling.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // nearest

    // Presentation (window) size. Render size can be lower for perf headroom.
    const int window_w = 1280;
    const int window_h = 720;
    const int render_w = 960;
    const int render_h = 540;

    sr::platform::WindowConfig cfg;
    cfg.width = window_w;
    cfg.height = window_h;
    cfg.title = "software-renderer-cpp";
    sr::platform::SdlApp app(cfg);

    sr::gfx::Framebuffer fb(render_w, render_h);
    sr::gfx::DepthBuffer zb(render_w, render_h);
    sr::render::Renderer renderer(fb, zb);

    SDL_Texture* screen = SDL_CreateTexture(app.renderer(), SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING, fb.width(), fb.height());
    if (!screen)
        return 1;

    sr::assets::AssetStore store(app.renderer());
    auto castle = std::make_shared<sr::assets::Model>(
        sr::assets::load_obj_model("./assets/models/peaches_castle.obj", store,
                                   sr::assets::ObjModelLoadOptions{
                                       .flip_v = true,
                                       .front_face_ccw = false,
                                       .double_sided = false,
                                   }));

    // Mario glTF copied from AFR.
    auto player_model = std::make_shared<sr::assets::Model>(sr::assets::load_gltf_model(
        "./assets/models/mario-64-mario/source/prototype_mario_super_mario_64/scene.gltf", store,
        sr::assets::GltfModelLoadOptions{
            .flip_v = false,
            .front_face_ccw = true,
            .double_sided = false,
        }));

    sr::scene::Scene scene;
    const float fov = 65.0f * 3.14159265f / 180.0f;
    const float z_near = 0.1f;
    const float z_far = 5000.0f;

    // World entities.
    sr::scene::Entity castle_ent;
    castle_ent.model = castle;
    // Match AFR scaling:
    // - Player height becomes 1.0 world unit ("1 meter")
    // - Castle is ~100 Marios wide
    // - Castle recentered so base is on y=0 and centered on origin in XZ.
    sr::math::Vec3 castle_mn, castle_mx;
    compute_bounds(castle->mesh.positions, castle_mn, castle_mx);
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
    castle_ent.transform = sr::math::mul(castle_recenter, castle_s);
    scene.entities.push_back(castle_ent);

    sr::scene::Entity player_ent;
    player_ent.model = player_model;
    // Player transform is updated each frame from controller state.
    player_ent.transform = sr::math::Mat4::identity();
    scene.entities.push_back(player_ent);

    // Build static collider from the castle mesh in world space.
    sr::physics::TriangleMeshCollider world_col;
    world_col.build_from_model(*castle, castle_ent.transform,
                               sr::physics::TriangleMeshCollider::BuildOptions{
                                   .cell_size = 1.25f,
                                   .two_sided = false,
                               });

    // Third-person player controller.
    sr::scene::PlayerController player;
    player.radius = 0.35f;
    player.yaw = 3.14159265f;
    player.pitch = -0.25f;

    // Model correction: scale player to 1.0 unit tall and pivot at bottom-center.
    sr::math::Vec3 player_mn, player_mx;
    compute_bounds(player_model->mesh.positions, player_mn, player_mx);
    float player_h = std::max(1e-6f, player_mx.y - player_mn.y);
    float player_scale = 1.0f / player_h;
    sr::math::Vec3 player_pivot{
        (player_mn.x + player_mx.x) * 0.5f,
        player_mn.y,
        (player_mn.z + player_mx.z) * 0.5f,
    };
    sr::math::Vec3 player_pivot_t{
        -player_pivot.x * player_scale,
        -player_pivot.y * player_scale,
        -player_pivot.z * player_scale,
    };
    sr::math::Mat4 player_model_offset = sr::math::mul(
        sr::math::Mat4::translate(player_pivot_t),
        sr::math::Mat4::scale(sr::math::Vec3{player_scale, player_scale, player_scale}));

    // Spawn: raycast down at origin, starting above the castle.
    float castle_h_world = (castle_mx.y - castle_mn.y) * castle_scale;
    float y0 = castle_h_world + 50.0f;
    {
        auto hit = world_col.raycast_down(0.0f, 0.0f, y0, y0 + 1000.0f);
        float ground_y = hit.hit ? hit.p.y : (castle_h_world + 1.0f);
        player.pos = sr::math::Vec3{0.0f, ground_y + player.radius + 0.02f, 0.0f};
    }

    // Place player on ground if possible.
    // (Already done via raycast above.)

    bool mouse_look = true;
    SDL_SetRelativeMouseMode(mouse_look ? SDL_TRUE : SDL_FALSE);
    SDL_ShowCursor(mouse_look ? SDL_DISABLE : SDL_ENABLE);

    // Debug toggles for culling/winding issues.
    bool cull_enabled = true;
    bool flip_winding = false;
    bool castle_double_sided = false;
    bool gravity_enabled = true;

    bool running = true;
    uint64_t last = SDL_GetPerformanceCounter();

    // FPS display (smoothed).
    float fps_accum_t = 0.0f;
    int fps_frames = 0;
    float fps_value = 0.0f;
    while (running) {
        uint64_t now = SDL_GetPerformanceCounter();
        float dt = float(double(now - last) / double(SDL_GetPerformanceFrequency()));
        last = now;
        dt = std::min(dt, 0.05f); // clamp huge frame hitches

        int mouse_dx = 0;
        int mouse_dy = 0;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
                mouse_look = !mouse_look;
                SDL_SetRelativeMouseMode(mouse_look ? SDL_TRUE : SDL_FALSE);
                SDL_ShowCursor(mouse_look ? SDL_DISABLE : SDL_ENABLE);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_c) {
                cull_enabled = !cull_enabled;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_v) {
                flip_winding = !flip_winding;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_t) {
                castle_double_sided = !castle_double_sided;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_g) {
                gravity_enabled = !gravity_enabled;
                if (!gravity_enabled) {
                    // Freeze vertical motion while gravity is disabled.
                    player.vel.y = 0.0f;
                    player.grounded = true;
                }
            }
            if (e.type == SDL_MOUSEMOTION && mouse_look) {
                mouse_dx += e.motion.xrel;
                mouse_dy += e.motion.yrel;
            }
        }

        const uint8_t* keys = SDL_GetKeyboardState(nullptr);

        // Update player orientation from mouse.
        if (mouse_look)
            player.apply_mouse(mouse_dx, mouse_dy);

        // Horizontal movement (arcade style): set desired velocity on XZ plane.
        sr::math::Vec3 move = player.move_dir_from_keys(keys);
        float speed = player.move_speed;
        if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
            speed *= player.sprint_mul;
        player.vel.x = move.x * speed;
        player.vel.z = move.z * speed;

        // Jump.
        if ((keys[SDL_SCANCODE_SPACE]) && player.grounded) {
            player.vel.y = player.jump_speed;
            player.grounded = false;
        }

        // Gravity.
        if (gravity_enabled) {
            player.vel.y -= player.gravity * dt;
        } else {
            player.vel.y = 0.0f;
            player.grounded = true;
        }

        // Substepping to prevent tunneling at low FPS / high speeds.
        const float max_step = std::max(0.05f, player.radius * 0.5f);
        float travel = sr::math::length(player.vel) * dt;
        int steps = int(std::ceil(travel / max_step));
        steps = std::clamp(steps, 1, 8);
        float sdt = dt / float(steps);

        bool grounded_this_frame = false;
        for (int s = 0; s < steps; ++s) {
            player.pos = player.pos + player.vel * sdt;

            auto c = world_col.resolve_sphere(player.pos, player.radius, &player.vel, 3);
            if (c.hit && c.normal.y > 0.55f)
                grounded_this_frame = true;
        }
        player.grounded = grounded_this_frame;

        // Camera follows the player.
        scene.camera = player.to_camera(fov, z_near, z_far);

        // Update player entity transform (translate + yaw + model correction).
        sr::math::Mat4 t = sr::math::Mat4::translate(player.pos);
        sr::math::Mat4 r = sr::math::Mat4::rotate_y(player.yaw);
        scene.entities[1].transform = sr::math::mul(t, sr::math::mul(r, player_model_offset));

        renderer.clear(argb(0xFF, 10, 10, 16));

        // Frustum cull entities by bounds sphere.
        const float aspect = float(fb.width()) / float(fb.height());
        sr::math::Mat4 view =
            sr::math::Mat4::look_at(scene.camera.eye, scene.camera.target, scene.camera.up);
        sr::math::Mat4 proj = sr::math::Mat4::perspective(scene.camera.fov_y_rad, aspect,
                                                          scene.camera.z_near, scene.camera.z_far);
        sr::math::Mat4 vp = sr::math::mul(proj, view);
        sr::render::Frustum fr = sr::render::Frustum::from_view_proj(vp);

        for (const auto& ent : scene.entities) {
            if (!ent.model)
                continue;
            const auto& model = *ent.model;

            sr::math::Vec3 wc = sr::math::transform_point(ent.transform, model.bounds_center);
            float wr = model.bounds_radius * sr::math::max_scale_component(ent.transform);
            if (!fr.sphere_visible(wc, wr))
                continue;

            auto prepared = renderer.prepare_mesh(model.mesh, ent.transform, scene.camera);

            // Draw each primitive with its material texture.
            for (const auto& prim : model.primitives) {
                const auto& mat = model.materials.at(prim.material_index);
                if (!mat.base_color_tex)
                    continue; // for now, skip untextured
                bool ds = mat.double_sided;
                bool ff = mat.front_face_ccw;
                if (!cull_enabled)
                    ds = true;
                if (flip_winding)
                    ff = !ff;
                if (ent.model == castle && castle_double_sided)
                    ds = true;

                renderer.draw_textured_mesh_prepared(prepared, *mat.base_color_tex,
                                                     prim.index_offset, prim.index_count, ds, ff);
            }
        }

        // FPS overlay (update a few times per second to avoid spammy jitter).
        fps_accum_t += dt;
        fps_frames += 1;
        if (fps_accum_t >= 0.25f) {
            fps_value = float(fps_frames) / fps_accum_t;
            fps_accum_t = 0.0f;
            fps_frames = 0;
        }
        char fps_text[64];
        std::snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", double(fps_value));
        sr::gfx::draw_text_5x7(fb, 8, 8, fps_text, 0xFFFFFFFFu, 2, 1);

        void* pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(screen, nullptr, &pixels, &pitch) == 0) {
            // Pitch is bytes per row.
            const int row_bytes = fb.width() * int(sizeof(uint32_t));
            for (int y = 0; y < fb.height(); ++y) {
                std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch, fb.pixels() + y * fb.width(),
                            row_bytes);
            }
            SDL_UnlockTexture(screen);
        }

        SDL_RenderClear(app.renderer());
        const SDL_Rect dst =
            centered_letterbox_rect(app.width(), app.height(), fb.width(), fb.height());
        SDL_RenderCopy(app.renderer(), screen, nullptr, &dst);
        SDL_RenderPresent(app.renderer());
    }

    SDL_DestroyTexture(screen);
    return 0;
}
