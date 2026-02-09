#include "sr/physics/triangle_collider.hpp"

#include "sr/math/transform.hpp"

#include <algorithm>
#include <cmath>

namespace sr::physics {
namespace {

inline float clampf(float x, float a, float b) {
    return std::max(a, std::min(b, x));
}

static sr::math::Vec3 closest_point_on_triangle(const sr::math::Vec3& p, const sr::math::Vec3& a,
                                                const sr::math::Vec3& b, const sr::math::Vec3& c) {
    // Real-Time Collision Detection (Christer Ericson) 5.1.5
    const sr::math::Vec3 ab = b - a;
    const sr::math::Vec3 ac = c - a;
    const sr::math::Vec3 ap = p - a;

    float d1 = sr::math::dot(ab, ap);
    float d2 = sr::math::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        return a;

    const sr::math::Vec3 bp = p - b;
    float d3 = sr::math::dot(ab, bp);
    float d4 = sr::math::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
        return b;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return a + ab * v;
    }

    const sr::math::Vec3 cp = p - c;
    float d5 = sr::math::dot(ab, cp);
    float d6 = sr::math::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6)
        return c;

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + ac * w;
    }

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + (c - b) * w;
    }

    // Inside face.
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

} // namespace

TriangleMeshCollider::CellKey TriangleMeshCollider::cell_for(float x, float z) const {
    return CellKey{int(std::floor(x / cell_size_)), int(std::floor(z / cell_size_))};
}

void TriangleMeshCollider::build_from_model(const sr::assets::Model& model,
                                            const sr::math::Mat4& model_to_world) {
    BuildOptions opt;
    build_from_model(model, model_to_world, opt);
}

void TriangleMeshCollider::build_from_model(const sr::assets::Model& model,
                                            const sr::math::Mat4& model_to_world,
                                            const BuildOptions& opt) {
    cell_size_ = (opt.cell_size > 1e-6f) ? opt.cell_size : 1.5f;
    two_sided_ = opt.two_sided;

    tris_.clear();
    grid_.clear();

    const auto& mesh = model.mesh;
    if (mesh.positions.empty() || mesh.indices.empty())
        return;

    tris_.reserve(mesh.indices.size() / 3);

    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i + 0];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];
        if (i0 >= mesh.positions.size() || i1 >= mesh.positions.size() ||
            i2 >= mesh.positions.size())
            continue;

        sr::math::Vec3 a = sr::math::transform_point(model_to_world, mesh.positions[i0]);
        sr::math::Vec3 b = sr::math::transform_point(model_to_world, mesh.positions[i1]);
        sr::math::Vec3 c = sr::math::transform_point(model_to_world, mesh.positions[i2]);

        sr::math::Vec3 n = sr::math::cross(b - a, c - a);
        float nlen = sr::math::length(n);
        if (nlen <= 1e-10f)
            continue;
        n = n / nlen;

        Tri t;
        t.a = a;
        t.b = b;
        t.c = c;
        t.n = n;
        t.minx = std::min({a.x, b.x, c.x});
        t.maxx = std::max({a.x, b.x, c.x});
        t.minz = std::min({a.z, b.z, c.z});
        t.maxz = std::max({a.z, b.z, c.z});
        tris_.push_back(t);
    }

    seen_stamp_.assign(tris_.size(), 0);

    // Insert triangles into spatial grid.
    for (uint32_t ti = 0; ti < tris_.size(); ++ti) {
        const auto& t = tris_[ti];
        CellKey a = cell_for(t.minx, t.minz);
        CellKey b = cell_for(t.maxx, t.maxz);
        for (int cz = a.z; cz <= b.z; ++cz) {
            for (int cx = a.x; cx <= b.x; ++cx) {
                grid_[CellKey{cx, cz}].push_back(ti);
            }
        }
    }
}

void TriangleMeshCollider::gather_candidates(float x, float z, float r, std::vector<uint32_t>& out,
                                             uint32_t stamp) const {
    out.clear();
    CellKey a = cell_for(x - r, z - r);
    CellKey b = cell_for(x + r, z + r);
    for (int cz = a.z; cz <= b.z; ++cz) {
        for (int cx = a.x; cx <= b.x; ++cx) {
            auto it = grid_.find(CellKey{cx, cz});
            if (it == grid_.end())
                continue;
            for (uint32_t ti : it->second) {
                if (ti >= seen_stamp_.size())
                    continue;
                if (seen_stamp_[ti] == stamp)
                    continue;
                seen_stamp_[ti] = stamp;
                out.push_back(ti);
            }
        }
    }
}

Contact TriangleMeshCollider::resolve_sphere(sr::math::Vec3& center, float radius,
                                             sr::math::Vec3* vel_io, int iterations) const {
    Contact res;
    if (tris_.empty() || radius <= 0.0f)
        return res;

    std::vector<uint32_t> cands;
    cands.reserve(256);

    static uint32_t stamp = 1;
    stamp = (stamp == 0) ? 1 : stamp + 1;
    gather_candidates(center.x, center.z, radius, cands, stamp);

    for (int it = 0; it < std::max(1, iterations); ++it) {
        bool any = false;
        sr::math::Vec3 best_n{0.0f, 1.0f, 0.0f};
        float best_pen = 0.0f;

        for (uint32_t ti : cands) {
            const Tri& t = tris_[ti];

            // Quick XZ reject.
            if (center.x + radius < t.minx || center.x - radius > t.maxx)
                continue;
            if (center.z + radius < t.minz || center.z - radius > t.maxz)
                continue;

            // One-sided triangles: ignore if approaching from the "back" side.
            if (!two_sided_) {
                float side = sr::math::dot(t.n, center - t.a);
                if (side < -radius)
                    continue;
            }

            sr::math::Vec3 cp = closest_point_on_triangle(center, t.a, t.b, t.c);
            sr::math::Vec3 d = center - cp;
            float dist2 = sr::math::dot(d, d);
            if (dist2 >= radius * radius)
                continue;

            float dist = std::sqrt(std::max(0.0f, dist2));
            sr::math::Vec3 n = (dist > 1e-6f) ? (d / dist) : t.n;
            float pen = radius - dist;

            // Push out.
            center = center + n * pen;

            // Remove velocity component into the surface.
            if (vel_io) {
                float vn = sr::math::dot(*vel_io, n);
                if (vn < 0.0f)
                    *vel_io = *vel_io - n * vn;
            }

            any = true;
            if (pen > best_pen) {
                best_pen = pen;
                best_n = n;
            }
        }

        if (any) {
            res.hit = true;
            res.normal = best_n;
            res.penetration = std::max(res.penetration, best_pen);
        } else {
            break;
        }
    }

    return res;
}

RayHit TriangleMeshCollider::raycast_down(float x, float z, float y_start, float max_dist) const {
    RayHit best;
    if (tris_.empty() || max_dist <= 0.0f)
        return best;

    // Narrow search by cells around the ray column.
    std::vector<uint32_t> cands;
    cands.reserve(256);
    static uint32_t stamp = 1;
    stamp = (stamp == 0) ? 1 : stamp + 1;
    gather_candidates(x, z, 0.01f, cands, stamp);

    const sr::math::Vec3 ro{x, y_start, z};
    const sr::math::Vec3 rd{0.0f, -1.0f, 0.0f};

    for (uint32_t ti : cands) {
        const Tri& t = tris_[ti];

        // Plane intersection.
        float denom = sr::math::dot(t.n, rd);
        if (std::fabs(denom) < 1e-8f)
            continue;
        float tval = (sr::math::dot(t.n, t.a - ro)) / denom;
        if (tval < 0.0f || tval > max_dist)
            continue;

        sr::math::Vec3 p = ro + rd * tval;

        // Barycentric inside test via edge half-spaces using triangle normal.
        sr::math::Vec3 ab = t.b - t.a;
        sr::math::Vec3 bc = t.c - t.b;
        sr::math::Vec3 ca = t.a - t.c;
        sr::math::Vec3 ap = p - t.a;
        sr::math::Vec3 bp = p - t.b;
        sr::math::Vec3 cp = p - t.c;
        if (sr::math::dot(sr::math::cross(ab, ap), t.n) < 0.0f)
            continue;
        if (sr::math::dot(sr::math::cross(bc, bp), t.n) < 0.0f)
            continue;
        if (sr::math::dot(sr::math::cross(ca, cp), t.n) < 0.0f)
            continue;

        if (!best.hit || tval < best.t) {
            best.hit = true;
            best.t = tval;
            best.p = p;
            best.n = t.n;
        }
    }

    return best;
}

} // namespace sr::physics
