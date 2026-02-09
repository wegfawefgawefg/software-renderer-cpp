#include "sr/platform/sdl.hpp"
#include "sr/gfx/framebuffer.hpp"
#include "sr/gfx/depthbuffer.hpp"
#include "sr/assets/asset_store.hpp"
#include "sr/assets/obj_model_loader.hpp"
#include "sr/render/renderer.hpp"
#include "sr/render/frustum.hpp"
#include "sr/math/mat4.hpp"
#include "sr/math/transform.hpp"
#include "sr/physics/triangle_collider.hpp"
#include "sr/scene/scene.hpp"
#include "sr/scene/player_controller.hpp"

#include <SDL2/SDL.h>

#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>

static uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  sr::platform::WindowConfig cfg;
  cfg.width = 1280;
  cfg.height = 720;
  cfg.title = "software-renderer-cpp";
  sr::platform::SdlApp app(cfg);

  sr::gfx::Framebuffer fb(app.width(), app.height());
  sr::gfx::DepthBuffer zb(app.width(), app.height());
  sr::render::Renderer renderer(fb, zb);

  SDL_Texture* screen = SDL_CreateTexture(
      app.renderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, fb.width(),
      fb.height());
  if (!screen) return 1;

  sr::assets::AssetStore store(app.renderer());
  auto castle = std::make_shared<sr::assets::Model>(sr::assets::load_obj_model(
      "./assets/models/peaches_castle.obj", store,
      sr::assets::ObjModelLoadOptions{
          .flip_v = true,
          .front_face_ccw = false,
          .double_sided = false,
      }));

  auto player_model = std::make_shared<sr::assets::Model>(sr::assets::load_obj_model(
      "./assets/models/gba.obj", store,
      sr::assets::ObjModelLoadOptions{
          .flip_v = true,
          .front_face_ccw = true,
          .double_sided = false,
      }));

  sr::scene::Scene scene;
  const float fov = 70.0f * 3.14159265f / 180.0f;
  const float z_near = 0.1f;
  const float z_far = 200.0f;

  // Scale the castle to a sane world size based on its model-space bounds.
  // The OBJ is authored in a different unit scale; normalize it so it fits our camera nicely.
  const float desired_radius = 8.0f;  // world units
  float castle_scale = 1.0f;
  if (castle && castle->bounds_radius > 1e-6f) {
    castle_scale = desired_radius / castle->bounds_radius;
  }

  // World entities.
  sr::scene::Entity castle_ent;
  castle_ent.model = castle;
  castle_ent.transform = sr::math::Mat4::scale(sr::math::Vec3{castle_scale, castle_scale, castle_scale});
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
  player.pos = sr::math::Vec3{0.0f, 3.0f, desired_radius * 0.6f};
  player.yaw = 3.14159265f;
  player.pitch = -0.25f;

  // Place player on ground if possible.
  {
    auto hit = world_col.raycast_down(player.pos.x, player.pos.z, desired_radius * 5.0f, desired_radius * 20.0f);
    if (hit.hit) player.pos.y = hit.p.y + player.radius + 0.02f;
  }

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
  while (running) {
    uint64_t now = SDL_GetPerformanceCounter();
    float dt = float(double(now - last) / double(SDL_GetPerformanceFrequency()));
    last = now;
    dt = std::min(dt, 0.05f);  // clamp huge frame hitches

    int mouse_dx = 0;
    int mouse_dy = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
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
    if (mouse_look) player.apply_mouse(mouse_dx, mouse_dy);

    // Horizontal movement (arcade style): set desired velocity on XZ plane.
    sr::math::Vec3 move = player.move_dir_from_keys(keys);
    float speed = player.move_speed;
    if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]) speed *= player.sprint_mul;
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
      if (c.hit && c.normal.y > 0.55f) grounded_this_frame = true;
    }
    player.grounded = grounded_this_frame;

    // Camera follows the player.
    scene.camera = player.to_camera(fov, z_near, z_far);

    // Update player entity transform (simple: translate + uniform scale).
    const float player_scale = 0.9f;  // model units
    sr::math::Mat4 t = sr::math::Mat4::translate(player.pos);
    sr::math::Mat4 r = sr::math::Mat4::rotate_y(player.yaw);
    sr::math::Mat4 s = sr::math::Mat4::scale(sr::math::Vec3{player_scale, player_scale, player_scale});
    scene.entities[1].transform = sr::math::mul(t, sr::math::mul(r, s));

    renderer.clear(argb(0xFF, 10, 10, 16));

    // Frustum cull entities by bounds sphere.
    const float aspect = float(fb.width()) / float(fb.height());
    sr::math::Mat4 view = sr::math::Mat4::look_at(scene.camera.eye, scene.camera.target, scene.camera.up);
    sr::math::Mat4 proj = sr::math::Mat4::perspective(scene.camera.fov_y_rad, aspect, scene.camera.z_near, scene.camera.z_far);
    sr::math::Mat4 vp = sr::math::mul(proj, view);
    sr::render::Frustum fr = sr::render::Frustum::from_view_proj(vp);

    for (const auto& ent : scene.entities) {
      if (!ent.model) continue;
      const auto& model = *ent.model;

      sr::math::Vec3 wc = sr::math::transform_point(ent.transform, model.bounds_center);
      float wr = model.bounds_radius * sr::math::max_scale_component(ent.transform);
      if (!fr.sphere_visible(wc, wr)) continue;

      auto prepared = renderer.prepare_mesh(model.mesh, ent.transform, scene.camera);

      // Draw each primitive with its material texture.
      for (const auto& prim : model.primitives) {
        const auto& mat = model.materials.at(prim.material_index);
        if (!mat.base_color_tex) continue;  // for now, skip untextured
        bool ds = mat.double_sided;
        bool ff = mat.front_face_ccw;
        if (!cull_enabled) ds = true;
        if (flip_winding) ff = !ff;
        if (ent.model == castle && castle_double_sided) ds = true;

        renderer.draw_textured_mesh_prepared(
            prepared, *mat.base_color_tex,
            prim.index_offset, prim.index_count, ds, ff);
      }
    }

    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(screen, nullptr, &pixels, &pitch) == 0) {
      // Pitch is bytes per row.
      const int row_bytes = fb.width() * int(sizeof(uint32_t));
      for (int y = 0; y < fb.height(); ++y) {
        std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch,
                    fb.pixels() + y * fb.width(), row_bytes);
      }
      SDL_UnlockTexture(screen);
    }

    SDL_RenderClear(app.renderer());
    SDL_RenderCopy(app.renderer(), screen, nullptr, nullptr);
    SDL_RenderPresent(app.renderer());
  }

  SDL_DestroyTexture(screen);
  return 0;
}
