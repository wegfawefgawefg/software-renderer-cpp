#include "sr/assets/gltf_model_loader.hpp"

#include "sr/gfx/texture.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace sr::assets {
namespace {

static std::filesystem::path resolve_uri(const std::filesystem::path& base_dir, const char* uri) {
    if (!uri)
        return {};
    std::filesystem::path p = base_dir / uri;
    if (std::filesystem::exists(p))
        return p;
    // Some assets expect textures under a nested directory; just return best-effort.
    return p;
}

static sr::math::Vec2 read_vec2(const cgltf_accessor* acc, cgltf_size i) {
    float v[2]{};
    cgltf_accessor_read_float(acc, i, v, 2);
    return sr::math::Vec2{v[0], v[1]};
}

static sr::math::Vec3 read_vec3(const cgltf_accessor* acc, cgltf_size i) {
    float v[3]{};
    cgltf_accessor_read_float(acc, i, v, 3);
    return sr::math::Vec3{v[0], v[1], v[2]};
}

static sr::math::Vec3 transform_point_col_major_4x4(const float m[16], const sr::math::Vec3& p) {
    // glTF matrices are column-major.
    const float x = p.x;
    const float y = p.y;
    const float z = p.z;
    const float rx = m[0] * x + m[4] * y + m[8] * z + m[12];
    const float ry = m[1] * x + m[5] * y + m[9] * z + m[13];
    const float rz = m[2] * x + m[6] * y + m[10] * z + m[14];
    return sr::math::Vec3{rx, ry, rz};
}

} // namespace

Model load_gltf_model(const std::filesystem::path& path, AssetStore& store,
                      const GltfModelLoadOptions& opt) {
    cgltf_options options{};
    cgltf_data* data = nullptr;

    cgltf_result r = cgltf_parse_file(&options, path.string().c_str(), &data);
    if (r != cgltf_result_success || !data)
        throw std::runtime_error("cgltf_parse_file failed: " + path.string());

    r = cgltf_load_buffers(&options, data, path.string().c_str());
    if (r != cgltf_result_success) {
        cgltf_free(data);
        throw std::runtime_error("cgltf_load_buffers failed: " + path.string());
    }

    r = cgltf_validate(data);
    if (r != cgltf_result_success) {
        cgltf_free(data);
        throw std::runtime_error("cgltf_validate failed: " + path.string());
    }

    const auto base_dir = path.parent_path();

    // Materials.
    std::vector<Material> materials;
    materials.reserve(data->materials_count ? data->materials_count : 1);
    if (data->materials_count == 0) {
        materials.push_back(Material{});
        materials.back().name = "default";
        materials.back().front_face_ccw = opt.front_face_ccw;
        materials.back().double_sided = opt.double_sided;
    } else {
        for (cgltf_size mi = 0; mi < data->materials_count; ++mi) {
            const cgltf_material& m = data->materials[mi];
            Material out;
            out.name = m.name ? m.name : ("mat_" + std::to_string(mi));
            out.front_face_ccw = opt.front_face_ccw;
            out.double_sided = opt.double_sided || (m.double_sided != 0);
            out.alpha_cutoff = (m.alpha_cutoff > 0.0f) ? float(m.alpha_cutoff) : 0.5f;
            switch (m.alpha_mode) {
            case cgltf_alpha_mode_mask:
                out.alpha_mode = AlphaMode::Mask;
                break;
            case cgltf_alpha_mode_blend:
                out.alpha_mode = AlphaMode::Blend;
                break;
            case cgltf_alpha_mode_opaque:
            default:
                out.alpha_mode = AlphaMode::Opaque;
                break;
            }

            // Base color texture.
            if (m.has_pbr_metallic_roughness) {
                const auto& pbr = m.pbr_metallic_roughness;
                out.base_color = sr::math::Vec3{pbr.base_color_factor[0], pbr.base_color_factor[1],
                                                pbr.base_color_factor[2]};
                if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
                    const cgltf_image* img = pbr.base_color_texture.texture->image;
                    if (img->uri) {
                        auto tex_path = resolve_uri(base_dir, img->uri);
                        if (std::filesystem::exists(tex_path))
                            out.base_color_tex = store.get_texture(tex_path.string());
                    }
                }
            }

            materials.push_back(std::move(out));
        }
    }

    // Build one unified mesh (simple + robust; duplicates vertices if primitives don't share
    // buffers).
    Model model;
    model.materials = std::move(materials);

    auto add_primitive = [&](const cgltf_primitive& prim, const float node_world[16]) {
        if (prim.type != cgltf_primitive_type_triangles)
            return;

        const cgltf_accessor* acc_pos = nullptr;
        const cgltf_accessor* acc_uv = nullptr;

        for (cgltf_size ai = 0; ai < prim.attributes_count; ++ai) {
            const cgltf_attribute& a = prim.attributes[ai];
            if (a.type == cgltf_attribute_type_position)
                acc_pos = a.data;
            if (a.type == cgltf_attribute_type_texcoord && a.index == 0)
                acc_uv = a.data;
        }
        if (!acc_pos)
            return;

        // Indices (or implicit).
        cgltf_size index_count = prim.indices ? prim.indices->count : acc_pos->count;
        if (index_count < 3)
            return;

        Primitive out_prim;
        out_prim.material_index = 0;
        if (prim.material) {
            out_prim.material_index = uint32_t(prim.material - data->materials);
            if (out_prim.material_index >= model.materials.size())
                out_prim.material_index = 0;
        }
        out_prim.index_offset = uint32_t(model.mesh.indices.size());

        // Expand vertices per index (keeps implementation small).
        for (cgltf_size ii = 0; ii < index_count; ++ii) {
            cgltf_size vi = prim.indices ? cgltf_accessor_read_index(prim.indices, ii) : ii;

            sr::math::Vec3 p = transform_point_col_major_4x4(node_world, read_vec3(acc_pos, vi));
            sr::math::Vec2 uv{0.0f, 0.0f};
            if (acc_uv)
                uv = read_vec2(acc_uv, vi);
            if (opt.flip_v)
                uv.y = 1.0f - uv.y;

            uint32_t out_i = uint32_t(model.mesh.positions.size());
            model.mesh.positions.push_back(p);
            model.mesh.uvs.push_back(uv);
            model.mesh.indices.push_back(out_i);
        }

        // Ensure triangle list.
        out_prim.index_count = uint32_t(model.mesh.indices.size()) - out_prim.index_offset;
        if (out_prim.index_count >= 3)
            model.primitives.push_back(out_prim);
    };

    auto visit_node = [&](auto&& self, const cgltf_node* node) -> void {
        if (!node)
            return;

        float world[16];
        cgltf_node_transform_world(node, world);

        if (node->mesh) {
            const cgltf_mesh& mesh = *node->mesh;
            for (cgltf_size pi = 0; pi < mesh.primitives_count; ++pi) {
                add_primitive(mesh.primitives[pi], world);
            }
        }

        for (cgltf_size ci = 0; ci < node->children_count; ++ci) {
            self(self, node->children[ci]);
        }
    };

    // Use default scene if present, else walk all meshes.
    if (data->scene) {
        const cgltf_scene& sc = *data->scene;
        for (cgltf_size ni = 0; ni < sc.nodes_count; ++ni) {
            visit_node(visit_node, sc.nodes[ni]);
        }
    } else {
        // No scene graph; treat meshes as identity-transformed.
        float ident[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
            const cgltf_mesh& mesh = data->meshes[mi];
            for (cgltf_size pi = 0; pi < mesh.primitives_count; ++pi) {
                add_primitive(mesh.primitives[pi], ident);
            }
        }
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
        float rad = 0.0f;
        for (const auto& p : model.mesh.positions) {
            rad = std::max(rad, sr::math::length(p - model.bounds_center));
        }
        model.bounds_radius = rad;
    }

    cgltf_free(data);
    return model;
}

} // namespace sr::assets
