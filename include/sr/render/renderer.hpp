#pragma once

#include "sr/assets/mesh.hpp"
#include "sr/gfx/depthbuffer.hpp"
#include "sr/gfx/framebuffer.hpp"
#include "sr/gfx/texture.hpp"
#include "sr/math/mat4.hpp"
#include "sr/math/vec2.hpp"
#include "sr/math/vec3.hpp"
#include "sr/math/vec4.hpp"

#include <cstdint>
#include <vector>

namespace sr::render {

namespace detail {
struct ClipVert {
  sr::math::Vec4 clip{};
  sr::math::Vec2 uv{};
};
}  // namespace detail

struct Camera {
  sr::math::Vec3 eye{0.0f, 0.0f, 3.0f};
  sr::math::Vec3 target{0.0f, 0.0f, 0.0f};
  sr::math::Vec3 up{0.0f, 1.0f, 0.0f};

  float fov_y_rad = 1.04719755f;  // 60 deg
  float z_near = 0.1f;
  float z_far = 200.0f;
};

class Renderer {
 public:
  Renderer(sr::gfx::Framebuffer& fb, sr::gfx::DepthBuffer& zb) : fb_(fb), zb_(zb) {}

  void clear(uint32_t argb, float z = std::numeric_limits<float>::infinity());

  struct PreparedMesh {
    const sr::assets::Mesh* mesh = nullptr;
    std::vector<sr::math::Vec4> clip_pos;
    bool has_uv = false;
  };

  PreparedMesh prepare_mesh(const sr::assets::Mesh& mesh, const sr::math::Mat4& model,
                            const Camera& cam) const;

  void draw_textured_mesh_prepared(
      const PreparedMesh& prepared,
      const sr::gfx::Texture& tex,
      uint32_t index_offset = 0,
      uint32_t index_count = 0,
      bool double_sided = false,
      bool front_face_ccw = true);

  void draw_textured_mesh(
      const sr::assets::Mesh& mesh,
      const sr::gfx::Texture& tex,
      const sr::math::Mat4& model,
      const Camera& cam,
      uint32_t index_offset = 0,
      uint32_t index_count = 0,
      bool double_sided = false,
      bool front_face_ccw = true);

 private:
  struct ScreenVert {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;  // NDC z in [-1,1] (smaller is closer)
    float u_over_w = 0.0f;
    float v_over_w = 0.0f;
    float inv_w = 0.0f;
  };

  void raster_triangle_textured(const ScreenVert& a, const ScreenVert& b, const ScreenVert& c,
                                const sr::gfx::Texture& tex);

  static std::vector<detail::ClipVert> clip_triangle(std::vector<detail::ClipVert> poly);

  sr::gfx::Framebuffer& fb_;
  sr::gfx::DepthBuffer& zb_;
};

}  // namespace sr::render
