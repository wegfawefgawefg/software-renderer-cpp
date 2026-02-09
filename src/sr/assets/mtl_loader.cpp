#include "sr/assets/mtl_loader.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

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

} // namespace

std::vector<Material> load_mtl(const std::filesystem::path& path, AssetStore& store) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("failed to open mtl: " + path.string());

    std::vector<Material> out;
    Material cur;
    bool has_cur = false;

    const auto base_dir = path.parent_path();
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "newmtl") {
            if (has_cur)
                out.push_back(cur);
            cur = Material{};
            iss >> cur.name;
            has_cur = true;
        } else if (cmd == "Kd") {
            float r, g, b;
            iss >> r >> g >> b;
            cur.base_color = sr::math::Vec3{r, g, b};
        } else if (cmd == "map_Kd") {
            std::string tex_name;
            iss >> tex_name;
            if (!tex_name.empty()) {
                std::filesystem::path tex_path = base_dir / tex_name;
                if (!std::filesystem::exists(tex_path)) {
                    // common layout in this repo
                    tex_path = std::filesystem::path("./assets/textures") / tex_name;
                }
                if (std::filesystem::exists(tex_path)) {
                    cur.base_color_tex = store.get_texture(tex_path.string());
                    if (cur.alpha_mode == AlphaMode::Opaque && cur.base_color_tex &&
                        cur.base_color_tex->likely_cutout()) {
                        cur.alpha_mode = AlphaMode::Mask;
                        cur.alpha_cutoff = 0.5f;
                    }
                }
            }
        } else if (cmd == "d") {
            float d = 1.0f;
            iss >> d;
            if (d < 0.999f)
                cur.alpha_mode = AlphaMode::Blend;
        } else if (cmd == "Tr") {
            float tr = 0.0f;
            iss >> tr;
            float d = 1.0f - tr;
            if (d < 0.999f)
                cur.alpha_mode = AlphaMode::Blend;
        } else if (cmd == "illum" || cmd == "Ns" || cmd == "Ka" || cmd == "Ks" || cmd == "Ke" ||
                   cmd == "Ni") {
            // ignore for now
        }
    }

    if (has_cur)
        out.push_back(cur);
    return out;
}

} // namespace sr::assets
