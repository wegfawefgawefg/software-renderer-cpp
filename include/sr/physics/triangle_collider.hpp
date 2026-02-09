#pragma once

#include "sr/assets/model.hpp"
#include "sr/math/mat4.hpp"
#include "sr/math/vec3.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace sr::physics {

struct Contact {
    bool hit = false;
    sr::math::Vec3 normal{0.0f, 1.0f, 0.0f}; // points away from surface (push direction)
    float penetration = 0.0f;
};

struct RayHit {
    bool hit = false;
    float t = 0.0f;
    sr::math::Vec3 p{0.0f, 0.0f, 0.0f};
    sr::math::Vec3 n{0.0f, 1.0f, 0.0f};
};

// Static triangle-mesh collider with a simple XZ spatial hash (good for "ground" worlds).
class TriangleMeshCollider {
  public:
    struct BuildOptions {
        float cell_size = 1.5f;
        bool two_sided = false; // if true, triangles collide from both sides
    };

    TriangleMeshCollider() = default;

    void build_from_model(const sr::assets::Model& model, const sr::math::Mat4& model_to_world);

    void build_from_model(const sr::assets::Model& model, const sr::math::Mat4& model_to_world,
                          const BuildOptions& opt);

    // Resolve a sphere against nearby triangles.
    // Moves `center` out of penetration and optionally removes velocity into surfaces.
    Contact resolve_sphere(sr::math::Vec3& center, float radius, sr::math::Vec3* vel_io = nullptr,
                           int iterations = 3) const;

    // Vertical ray down (y decreasing). Useful for spawn placement.
    RayHit raycast_down(float x, float z, float y_start, float max_dist) const;

  private:
    struct Tri {
        sr::math::Vec3 a, b, c;
        sr::math::Vec3 n; // unit normal (CCW)
        float minx = 0.0f, maxx = 0.0f;
        float minz = 0.0f, maxz = 0.0f;
    };

    struct CellKey {
        int x = 0;
        int z = 0;
        bool operator==(const CellKey& o) const { return x == o.x && z == o.z; }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& k) const noexcept {
            return (std::size_t(uint32_t(k.x)) << 32) ^ std::size_t(uint32_t(k.z));
        }
    };

    CellKey cell_for(float x, float z) const;

    void gather_candidates(float x, float z, float r, std::vector<uint32_t>& out,
                           uint32_t stamp) const;

    float cell_size_ = 1.5f;
    bool two_sided_ = false;

    std::vector<Tri> tris_;
    std::unordered_map<CellKey, std::vector<uint32_t>, CellKeyHash> grid_;
    mutable std::vector<uint32_t> seen_stamp_;
};

} // namespace sr::physics
