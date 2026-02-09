#include "app/render.hpp"

#include "app/util.hpp"

#include "sr/gfx/font5x7.hpp"
#include "sr/math/mat4.hpp"
#include "sr/math/transform.hpp"
#include "sr/render/frustum.hpp"

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstring>

namespace app {

void render_game(sr::render::Renderer& renderer, sr::gfx::Framebuffer& fb, Game& g,
                 const AppToggles& toggles, FpsCounter* fps) {
    renderer.clear(app::argb(0xFF, 10, 10, 16));

    // Frustum cull entities by bounds sphere.
    const float aspect = float(fb.width()) / float(fb.height());
    sr::math::Mat4 view =
        sr::math::Mat4::look_at(g.scene.camera.eye, g.scene.camera.target, g.scene.camera.up);
    sr::math::Mat4 proj = sr::math::Mat4::perspective(g.scene.camera.fov_y_rad, aspect,
                                                      g.scene.camera.z_near, g.scene.camera.z_far);
    sr::math::Mat4 vp = sr::math::mul(proj, view);
    sr::render::Frustum fr = sr::render::Frustum::from_view_proj(vp);

    for (const auto& ent : g.scene.entities) {
        if (!ent.model)
            continue;
        const auto& model = *ent.model;

        sr::math::Vec3 wc = sr::math::transform_point(ent.transform, model.bounds_center);
        float wr = model.bounds_radius * sr::math::max_scale_component(ent.transform);
        if (!fr.sphere_visible(wc, wr))
            continue;

        auto prepared = renderer.prepare_mesh(model.mesh, ent.transform, g.scene.camera);

        for (const auto& prim : model.primitives) {
            const auto& mat = model.materials.at(prim.material_index);
            if (!mat.base_color_tex)
                continue;

            bool ds = mat.double_sided;
            bool ff = mat.front_face_ccw;
            if (!toggles.cull_enabled)
                ds = true;
            if (toggles.flip_winding)
                ff = !ff;
            if (ent.model == g.castle && toggles.castle_double_sided)
                ds = true;

            renderer.draw_textured_mesh_prepared(prepared, *mat.base_color_tex, prim.index_offset,
                                                 prim.index_count, ds, ff, mat.alpha_mode,
                                                 mat.alpha_cutoff);
        }
    }

    if (fps && toggles.show_fps) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "FPS: %.1f", double(fps->value));
        sr::gfx::draw_text_5x7(fb, 8, 8, buf, 0xFFFFFFFFu, 2, 1);
    }
}

void present(SDL_Renderer* sdl_renderer, SDL_Texture* screen, const sr::gfx::Framebuffer& fb,
             int window_w, int window_h) {
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(screen, nullptr, &pixels, &pitch) == 0) {
        const int row_bytes = fb.width() * int(sizeof(uint32_t));
        for (int y = 0; y < fb.height(); ++y) {
            std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch, fb.pixels() + y * fb.width(),
                        row_bytes);
        }
        SDL_UnlockTexture(screen);
    }

    SDL_RenderClear(sdl_renderer);
    const SDL_Rect dst = app::centered_letterbox_rect(window_w, window_h, fb.width(), fb.height());
    SDL_RenderCopy(sdl_renderer, screen, nullptr, &dst);
    SDL_RenderPresent(sdl_renderer);
}

} // namespace app
