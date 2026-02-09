#include "sr/assets/obj_loader.hpp"

#include "sr/math/vec2.hpp"
#include "sr/math/vec3.hpp"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sr::assets {
namespace {

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

static int parse_index(const std::string& s, int n) {
  // OBJ indices are 1-based; negative indices are relative to end.
  int i = std::stoi(s);
  if (i < 0) return n + i;
  return i - 1;
}

}  // namespace

Mesh load_obj_minimal(const std::string& path, bool flip_v) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("failed to open obj: " + path);

  std::vector<sr::math::Vec3> in_pos;
  std::vector<sr::math::Vec2> in_uv;

  Mesh out;
  std::unordered_map<Key, uint32_t, KeyHash> remap;

  std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "v") {
      float x, y, z;
      iss >> x >> y >> z;
      in_pos.emplace_back(x, y, z);
    } else if (cmd == "vt") {
      float u, v;
      iss >> u >> v;
      if (flip_v) v = 1.0f - v;
      in_uv.emplace_back(u, v);
    } else if (cmd == "f") {
      std::vector<Key> face;
      std::string tok;
      while (iss >> tok) {
        // v/vt/vn or v/vt or v//vn or v
        int vi = -1;
        int ti = -1;

        size_t p1 = tok.find('/');
        if (p1 == std::string::npos) {
          vi = parse_index(tok, int(in_pos.size()));
        } else {
          std::string a = tok.substr(0, p1);
          vi = parse_index(a, int(in_pos.size()));
          size_t p2 = tok.find('/', p1 + 1);
          std::string b = (p2 == std::string::npos) ? tok.substr(p1 + 1) : tok.substr(p1 + 1, p2 - (p1 + 1));
          if (!b.empty()) ti = parse_index(b, int(in_uv.size()));
        }
        face.push_back(Key{vi, ti});
      }

      if (face.size() < 3) continue;
      // Fan triangulation: (0,i,i+1)
      for (size_t i = 1; i + 1 < face.size(); ++i) {
        Key tri[3] = {face[0], face[i], face[i + 1]};
        for (int k = 0; k < 3; ++k) {
          auto it = remap.find(tri[k]);
          if (it == remap.end()) {
            uint32_t new_idx = uint32_t(out.positions.size());
            remap.emplace(tri[k], new_idx);
            out.positions.push_back(in_pos.at(tri[k].vi));
            if (!in_uv.empty()) {
              if (tri[k].ti >= 0 && tri[k].ti < int(in_uv.size())) {
                out.uvs.push_back(in_uv.at(tri[k].ti));
              } else {
                out.uvs.push_back(sr::math::Vec2{0.0f, 0.0f});
              }
            }
            out.indices.push_back(new_idx);
          } else {
            out.indices.push_back(it->second);
          }
        }
      }
    }
  }

  // Ensure uvs either empty or match vertex count.
  if (!out.uvs.empty() && out.uvs.size() != out.positions.size()) {
    out.uvs.resize(out.positions.size(), sr::math::Vec2{0.0f, 0.0f});
  }
  return out;
}

}  // namespace sr::assets

