#include "app/render.hpp"

#include "app/util.hpp"

#include "sr/math/mat4.hpp"
#include "sr/math/transform.hpp"
#include "sr/render/frustum.hpp"

#include <SDL2/SDL.h>

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

    (void)fps;
}

void present(SDL_Renderer* sdl_renderer, SDL_Texture* screen, const sr::gfx::Framebuffer& fb,
             int window_w, int window_h) {
    (void)sdl_renderer;
    (void)screen;
    (void)fb;
    (void)window_w;
    (void)window_h;
}

} // namespace app
