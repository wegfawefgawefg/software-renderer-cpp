#include "sr/render/renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace sr::render {
namespace {

inline float edge_fn(float ax, float ay, float bx, float by, float px, float py) {
  return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

template <typename DistFn>
static std::vector<detail::ClipVert> clip_poly_against_plane(
    const std::vector<detail::ClipVert>& poly, DistFn dist_fn) {
  if (poly.empty()) return {};
  std::vector<detail::ClipVert> out;
  out.reserve(poly.size() + 2);

  auto prev = poly.back();
  float prev_d = dist_fn(prev.clip);
  bool prev_in = prev_d >= 0.0f;

  for (const auto& cur : poly) {
    float cur_d = dist_fn(cur.clip);
    bool cur_in = cur_d >= 0.0f;

    if (prev_in && cur_in) {
      out.push_back(cur);
    } else if (prev_in && !cur_in) {
      float denom = (prev_d - cur_d);
      if (denom != 0.0f) {
        float t = prev_d / denom;
        detail::ClipVert i;
        i.clip = prev.clip + (cur.clip - prev.clip) * t;
        i.uv = prev.uv + (cur.uv - prev.uv) * t;
        out.push_back(i);
      }
    } else if (!prev_in && cur_in) {
      float denom = (prev_d - cur_d);
      if (denom != 0.0f) {
        float t = prev_d / denom;
        detail::ClipVert i;
        i.clip = prev.clip + (cur.clip - prev.clip) * t;
        i.uv = prev.uv + (cur.uv - prev.uv) * t;
        out.push_back(i);
      }
      out.push_back(cur);
    }

    prev = cur;
    prev_d = cur_d;
    prev_in = cur_in;
  }

  return out;
}

}  // namespace

void Renderer::clear(uint32_t argb, float z) {
  fb_.clear(argb);
  zb_.clear(z);
}

std::vector<detail::ClipVert> Renderer::clip_triangle(std::vector<detail::ClipVert> poly) {
  // Clip volume (OpenGL): -w<=x<=w, -w<=y<=w, -w<=z<=w
  const auto left = [](const sr::math::Vec4& c) { return c.x + c.w; };
  const auto right = [](const sr::math::Vec4& c) { return -c.x + c.w; };
  const auto bottom = [](const sr::math::Vec4& c) { return c.y + c.w; };
  const auto top = [](const sr::math::Vec4& c) { return -c.y + c.w; };
  const auto nearp = [](const sr::math::Vec4& c) { return c.z + c.w; };
  const auto farp = [](const sr::math::Vec4& c) { return -c.z + c.w; };

  poly = clip_poly_against_plane(poly, left);
  if (poly.size() < 3) return {};
  poly = clip_poly_against_plane(poly, right);
  if (poly.size() < 3) return {};
  poly = clip_poly_against_plane(poly, bottom);
  if (poly.size() < 3) return {};
  poly = clip_poly_against_plane(poly, top);
  if (poly.size() < 3) return {};
  poly = clip_poly_against_plane(poly, nearp);
  if (poly.size() < 3) return {};
  poly = clip_poly_against_plane(poly, farp);
  if (poly.size() < 3) return {};
  return poly;
}

void Renderer::draw_textured_mesh(const sr::assets::Mesh& mesh, const sr::gfx::Texture& tex,
                                 const sr::math::Mat4& model, const Camera& cam,
                                 uint32_t index_offset, uint32_t index_count,
                                 bool double_sided, bool front_face_ccw) {
  auto prepared = prepare_mesh(mesh, model, cam);
  draw_textured_mesh_prepared(prepared, tex, index_offset, index_count, double_sided, front_face_ccw);
}

Renderer::PreparedMesh Renderer::prepare_mesh(const sr::assets::Mesh& mesh,
                                              const sr::math::Mat4& model,
                                              const Camera& cam) const {
  const float aspect = float(fb_.width()) / float(fb_.height());
  sr::math::Mat4 view = sr::math::Mat4::look_at(cam.eye, cam.target, cam.up);
  sr::math::Mat4 proj = sr::math::Mat4::perspective(cam.fov_y_rad, aspect, cam.z_near, cam.z_far);
  sr::math::Mat4 vp = sr::math::mul(proj, view);
  sr::math::Mat4 mvp = sr::math::mul(vp, model);

  PreparedMesh prepared;
  prepared.mesh = &mesh;
  prepared.has_uv = !mesh.uvs.empty() && mesh.uvs.size() == mesh.positions.size();

  prepared.clip_pos.reserve(mesh.positions.size());
  for (const auto& p : mesh.positions) {
    prepared.clip_pos.push_back(sr::math::mul(mvp, sr::math::Vec4{p.x, p.y, p.z, 1.0f}));
  }

  return prepared;
}

void Renderer::draw_textured_mesh_prepared(const PreparedMesh& prepared,
                                           const sr::gfx::Texture& tex,
                                           uint32_t index_offset, uint32_t index_count,
                                           bool double_sided, bool front_face_ccw) {
  if (!prepared.mesh) return;
  const sr::assets::Mesh& mesh = *prepared.mesh;
  const auto& clip_pos = prepared.clip_pos;
  const bool has_uv = prepared.has_uv;

  // Triangles.
  const uint32_t idx_base = index_offset;
  uint32_t count = index_count;
  if (count == 0) count = uint32_t(mesh.indices.size());
  if (idx_base >= mesh.indices.size()) return;
  uint32_t end = std::min<uint32_t>(uint32_t(mesh.indices.size()), idx_base + count);
  for (uint32_t i = idx_base; i + 2 < end; i += 3) {
    uint32_t i0 = mesh.indices[i + 0];
    uint32_t i1 = mesh.indices[i + 1];
    uint32_t i2 = mesh.indices[i + 2];
    if (i0 >= clip_pos.size() || i1 >= clip_pos.size() || i2 >= clip_pos.size()) continue;

    detail::ClipVert v0{clip_pos[i0], has_uv ? mesh.uvs[i0] : sr::math::Vec2{0, 0}};
    detail::ClipVert v1{clip_pos[i1], has_uv ? mesh.uvs[i1] : sr::math::Vec2{0, 0}};
    detail::ClipVert v2{clip_pos[i2], has_uv ? mesh.uvs[i2] : sr::math::Vec2{0, 0}};

    auto poly = clip_triangle({v0, v1, v2});
    if (poly.size() < 3) continue;

    // Fan triangulate.
    const detail::ClipVert vbase = poly[0];
    for (size_t k = 1; k + 1 < poly.size(); ++k) {
      const detail::ClipVert a = vbase;
      const detail::ClipVert b = poly[k];
      const detail::ClipVert c = poly[k + 1];

      // Perspective divide -> NDC.
      if (a.clip.w == 0.0f || b.clip.w == 0.0f || c.clip.w == 0.0f) continue;
      float invwa = 1.0f / a.clip.w;
      float invwb = 1.0f / b.clip.w;
      float invwc = 1.0f / c.clip.w;
      sr::math::Vec3 ndc_a{a.clip.x * invwa, a.clip.y * invwa, a.clip.z * invwa};
      sr::math::Vec3 ndc_b{b.clip.x * invwb, b.clip.y * invwb, b.clip.z * invwb};
      sr::math::Vec3 ndc_c{c.clip.x * invwc, c.clip.y * invwc, c.clip.z * invwc};

      auto to_screen = [&](const sr::math::Vec3& ndc, float invw, const sr::math::Vec2& uv) -> ScreenVert {
        float sx = (ndc.x * 0.5f + 0.5f) * float(fb_.width() - 1);
        float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * float(fb_.height() - 1);  // y down
        ScreenVert sv;
        sv.x = sx;
        sv.y = sy;
        sv.z = ndc.z;
        sv.inv_w = invw;
        sv.u_over_w = uv.x * invw;
        sv.v_over_w = uv.y * invw;
        return sv;
      };

      ScreenVert sa = to_screen(ndc_a, invwa, a.uv);
      ScreenVert sb = to_screen(ndc_b, invwb, b.uv);
      ScreenVert sc = to_screen(ndc_c, invwc, c.uv);

      // Backface cull in NDC (y up). CCW in NDC is the usual "front-face" convention.
      float area_ndc = (ndc_b.x - ndc_a.x) * (ndc_c.y - ndc_a.y) -
                       (ndc_b.y - ndc_a.y) * (ndc_c.x - ndc_a.x);
      if (!double_sided) {
        if (front_face_ccw) {
          if (area_ndc <= 0.0f) continue;
        } else {
          if (area_ndc >= 0.0f) continue;
        }
      }

      raster_triangle_textured(sa, sb, sc, tex);
    }
  }
}

void Renderer::raster_triangle_textured(const ScreenVert& a, const ScreenVert& b, const ScreenVert& c,
                                        const sr::gfx::Texture& tex) {
  // Bounding box.
  float minx_f = std::min({a.x, b.x, c.x});
  float maxx_f = std::max({a.x, b.x, c.x});
  float miny_f = std::min({a.y, b.y, c.y});
  float maxy_f = std::max({a.y, b.y, c.y});

  int minx = std::max(0, int(std::floor(minx_f)));
  int maxx = std::min(fb_.width() - 1, int(std::ceil(maxx_f)));
  int miny = std::max(0, int(std::floor(miny_f)));
  int maxy = std::min(fb_.height() - 1, int(std::ceil(maxy_f)));
  if (minx > maxx || miny > maxy) return;

  float area = edge_fn(a.x, a.y, b.x, b.y, c.x, c.y);
  if (area == 0.0f) return;
  float inv_area = 1.0f / area;

  for (int y = miny; y <= maxy; ++y) {
    float py = float(y) + 0.5f;
    for (int x = minx; x <= maxx; ++x) {
      float px = float(x) + 0.5f;
      float w0 = edge_fn(b.x, b.y, c.x, c.y, px, py);
      float w1 = edge_fn(c.x, c.y, a.x, a.y, px, py);
      float w2 = edge_fn(a.x, a.y, b.x, b.y, px, py);

      // Inside test supports both windings (area sign).
      if (area > 0.0f) {
        if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) continue;
      } else {
        if (w0 > 0.0f || w1 > 0.0f || w2 > 0.0f) continue;
      }

      float alpha = w0 * inv_area;
      float beta = w1 * inv_area;
      float gamma = w2 * inv_area;

      float z = alpha * a.z + beta * b.z + gamma * c.z;
      // NDC z: near=-1 is closer than far=+1.
      if (z >= zb_.get(x, y)) continue;

      float invw = alpha * a.inv_w + beta * b.inv_w + gamma * c.inv_w;
      if (invw == 0.0f) continue;
      float uow = alpha * a.u_over_w + beta * b.u_over_w + gamma * c.u_over_w;
      float vow = alpha * a.v_over_w + beta * b.v_over_w + gamma * c.v_over_w;
      float u = uow / invw;
      float v = vow / invw;

      uint32_t argb = tex.sample_repeat(u, v);

      fb_.pixels()[y * fb_.width() + x] = argb;
      zb_.set(x, y, z);
    }
  }
}

}  // namespace sr::render
