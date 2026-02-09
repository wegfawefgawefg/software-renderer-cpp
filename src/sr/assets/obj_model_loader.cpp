#include "sr/assets/obj_model_loader.hpp"

#include "sr/assets/mtl_loader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sr::assets {
namespace {

static std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a])))
        ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1])))
        --b;
    return s.substr(a, b - a);
}

static int parse_index(const std::string& s, int n) {
    int i = std::stoi(s);
    if (i < 0)
        return n + i;
    return i - 1;
}

struct Key {
    int vi = -1;
    int ti = -1;
    bool operator==(const Key& o) const { return vi == o.vi && ti == o.ti; }
};

struct KeyHash {
    std::size_t operator()(const Key& k) const noexcept {
        return (std::size_t(uint32_t(k.vi)) << 1) ^ std::size_t(uint32_t(k.ti));
    }
};

} // namespace

Model load_obj_model(const std::filesystem::path& path, AssetStore& store,
                     const ObjModelLoadOptions& opt) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("failed to open obj: " + path.string());

    const auto base_dir = path.parent_path();

    std::vector<sr::math::Vec3> in_pos;
    std::vector<sr::math::Vec2> in_uv;

    std::vector<Material> materials;
    std::unordered_map<std::string, uint32_t> mat_index;
    uint32_t cur_mat = 0;

    // Always have a default material at index 0.
    materials.push_back(Material{});
    materials.back().name = "default";
    materials.back().front_face_ccw = opt.front_face_ccw;
    materials.back().double_sided = opt.double_sided;
    mat_index["default"] = 0;

    // Indices per material (into unified vertex buffer).
    std::unordered_map<uint32_t, std::vector<uint32_t>> mat_indices;

    Mesh mesh;
    std::unordered_map<Key, uint32_t, KeyHash> remap;

    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "mtllib") {
            std::string mtl_name;
            iss >> mtl_name;
            if (!mtl_name.empty()) {
                std::filesystem::path mtl_path = base_dir / mtl_name;
                if (!std::filesystem::exists(mtl_path)) {
                    mtl_path = std::filesystem::path("./assets/materials") / mtl_name;
                }
                if (std::filesystem::exists(mtl_path)) {
                    auto mats = load_mtl(mtl_path, store);
                    for (auto& m : mats) {
                        if (m.name.empty())
                            continue;
                        if (mat_index.find(m.name) != mat_index.end())
                            continue;
                        uint32_t idx = uint32_t(materials.size());
                        mat_index[m.name] = idx;
                        m.front_face_ccw = opt.front_face_ccw;
                        m.double_sided = opt.double_sided;
                        materials.push_back(std::move(m));
                    }
                }
            }
        } else if (cmd == "usemtl") {
            std::string name;
            iss >> name;
            if (name.empty())
                name = "default";
            auto it = mat_index.find(name);
            if (it == mat_index.end()) {
                uint32_t idx = uint32_t(materials.size());
                mat_index[name] = idx;
                materials.push_back(Material{});
                materials.back().name = name;
                materials.back().front_face_ccw = opt.front_face_ccw;
                materials.back().double_sided = opt.double_sided;
                cur_mat = idx;
            } else {
                cur_mat = it->second;
            }
        } else if (cmd == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            in_pos.emplace_back(x, y, z);
        } else if (cmd == "vt") {
            float u, v;
            iss >> u >> v;
            // OBJ convention is usually bottom-left; textures in this repo are top-left.
            if (opt.flip_v)
                v = 1.0f - v;
            in_uv.emplace_back(u, v);
        } else if (cmd == "f") {
            std::vector<Key> face;
            std::string tok;
            while (iss >> tok) {
                int vi = -1;
                int ti = -1;
                size_t p1 = tok.find('/');
                if (p1 == std::string::npos) {
                    vi = parse_index(tok, int(in_pos.size()));
                } else {
                    vi = parse_index(tok.substr(0, p1), int(in_pos.size()));
                    size_t p2 = tok.find('/', p1 + 1);
                    std::string b = (p2 == std::string::npos) ? tok.substr(p1 + 1)
                                                              : tok.substr(p1 + 1, p2 - (p1 + 1));
                    if (!b.empty())
                        ti = parse_index(b, int(in_uv.size()));
                }
                face.push_back(Key{vi, ti});
            }
            if (face.size() < 3)
                continue;

            auto& idxs = mat_indices[cur_mat];

            // Fan triangulation.
            for (size_t i = 1; i + 1 < face.size(); ++i) {
                Key tri[3] = {face[0], face[i], face[i + 1]};
                for (int k = 0; k < 3; ++k) {
                    auto it = remap.find(tri[k]);
                    uint32_t out_i = 0;
                    if (it == remap.end()) {
                        out_i = uint32_t(mesh.positions.size());
                        remap.emplace(tri[k], out_i);
                        mesh.positions.push_back(in_pos.at(tri[k].vi));
                        if (!in_uv.empty()) {
                            if (tri[k].ti >= 0 && tri[k].ti < int(in_uv.size())) {
                                mesh.uvs.push_back(in_uv.at(tri[k].ti));
                            } else {
                                mesh.uvs.push_back(sr::math::Vec2{0.0f, 0.0f});
                            }
                        }
                    } else {
                        out_i = it->second;
                    }
                    idxs.push_back(out_i);
                }
            }
        }
    }

    // Build a single index buffer grouped by materials, with primitive ranges.
    Model model;
    model.mesh = std::move(mesh);
    model.materials = std::move(materials);

    // If uvs exist, size should match.
    if (!model.mesh.uvs.empty() && model.mesh.uvs.size() != model.mesh.positions.size()) {
        model.mesh.uvs.resize(model.mesh.positions.size(), sr::math::Vec2{0.0f, 0.0f});
    }

    // Concatenate indices, stable by material index.
    model.mesh.indices.clear();
    model.primitives.clear();
    model.mesh.indices.reserve([&]() {
        size_t n = 0;
        for (const auto& kv : mat_indices)
            n += kv.second.size();
        return n;
    }());

    for (uint32_t mi = 0; mi < model.materials.size(); ++mi) {
        auto it = mat_indices.find(mi);
        if (it == mat_indices.end() || it->second.empty())
            continue;
        Primitive prim;
        prim.material_index = mi;
        prim.index_offset = uint32_t(model.mesh.indices.size());
        prim.index_count = uint32_t(it->second.size());
        model.mesh.indices.insert(model.mesh.indices.end(), it->second.begin(), it->second.end());
        model.primitives.push_back(prim);
    }

    // Bounds (sphere) in model space.
    if (!model.mesh.positions.empty()) {
        sr::math::Vec3 mn = model.mesh.positions[0];
        sr::math::Vec3 mx = model.mesh.positions[0];
        for (const auto& p : model.mesh.positions) {
            mn.x = std::min(mn.x, p.x);
            mn.y = std::min(mn.y, p.y);
            mn.z = std::min(mn.z, p.z);
            mx.x = std::max(mx.x, p.x);
            mx.y = std::max(mx.y, p.y);
            mx.z = std::max(mx.z, p.z);
        }
        model.bounds_center = (mn + mx) * 0.5f;
        float r = 0.0f;
        for (const auto& p : model.mesh.positions) {
            float d = sr::math::length(p - model.bounds_center);
            r = std::max(r, d);
        }
        model.bounds_radius = r;
    }

    return model;
}

} // namespace sr::assets
