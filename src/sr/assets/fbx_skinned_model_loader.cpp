#include "sr/assets/fbx_skinned_model_loader.hpp"

#include "sr/assets/material.hpp"
#include "sr/math/quat.hpp"
#include "sr/math/trs.hpp"
#include "sr/math/vec2.hpp"

#include <ufbx.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sr::assets {
namespace {

static std::string normalize_joint_name(std::string s) {
    // FBX exporters often namespace joint names: "Armature|Hips", "mixamorig:Hips", etc.
    // For our simple loaders we match by the "leaf" name.
    auto cut_after_last = [&](char c) {
        size_t p = s.find_last_of(c);
        if (p != std::string::npos && p + 1 < s.size())
            s = s.substr(p + 1);
    };
    cut_after_last('|');
    cut_after_last(':');
    // Trim whitespace.
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n'))
        s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n'))
        i++;
    if (i > 0)
        s = s.substr(i);
    return s;
}

static sr::math::Mat4 mat4_from_ufbx(const ufbx_matrix& m) {
    // ufbx_matrix is a 4x3 affine matrix represented as column vectors.
    sr::math::Mat4 out = sr::math::Mat4::identity();
    out.m[0][0] = float(m.m00);
    out.m[1][0] = float(m.m10);
    out.m[2][0] = float(m.m20);

    out.m[0][1] = float(m.m01);
    out.m[1][1] = float(m.m11);
    out.m[2][1] = float(m.m21);

    out.m[0][2] = float(m.m02);
    out.m[1][2] = float(m.m12);
    out.m[2][2] = float(m.m22);

    out.m[0][3] = float(m.m03);
    out.m[1][3] = float(m.m13);
    out.m[2][3] = float(m.m23);
    return out;
}

static sr::math::Trs trs_from_ufbx(const ufbx_transform& t) {
    sr::math::Trs out;
    out.t = sr::math::Vec3{float(t.translation.x), float(t.translation.y), float(t.translation.z)};
    out.s = sr::math::Vec3{float(t.scale.x), float(t.scale.y), float(t.scale.z)};
    out.r = sr::math::Quat{float(t.rotation.x), float(t.rotation.y), float(t.rotation.z),
                           float(t.rotation.w)};
    out.r = sr::math::normalize(out.r);
    return out;
}

static std::filesystem::path resolve_texture(const std::filesystem::path& base_dir,
                                             const std::string& rel) {
    if (rel.empty())
        return {};
    std::filesystem::path p = rel;
    if (p.is_absolute())
        return p;
    // Try as-is first (project-relative paths like "./assets/..." are common).
    if (std::filesystem::exists(p))
        return p;
    // Then try relative to the FBX file directory.
    std::filesystem::path bp = base_dir / p;
    if (std::filesystem::exists(bp))
        return bp;
    // Best-effort.
    return bp;
}

static std::string to_string(ufbx_string s) {
    return std::string(s.data, s.length);
}

static SkinInfluence4 influences_from_vertex(const ufbx_skin_deformer& skin, uint32_t vertex_id) {
    SkinInfluence4 out;
    if (vertex_id >= skin.vertices.count)
        return out;
    const ufbx_skin_vertex sv = skin.vertices.data[vertex_id];
    const uint32_t begin = sv.weight_begin;
    const uint32_t count = sv.num_weights;
    if (begin >= skin.weights.count || count == 0)
        return out;

    float sum = 0.0f;
    int taken = 0;
    for (uint32_t i = 0; i < count && taken < 4; ++i) {
        uint32_t wi = begin + i;
        if (wi >= skin.weights.count)
            break;
        const ufbx_skin_weight w = skin.weights.data[wi];
        out.joint[taken] = uint16_t(w.cluster_index);
        out.weight[taken] = float(w.weight);
        sum += out.weight[taken];
        taken++;
    }

    if (sum > 0.0f) {
        for (int i = 0; i < 4; ++i)
            out.weight[i] /= sum;
    } else {
        out.weight[0] = 1.0f;
        out.weight[1] = out.weight[2] = out.weight[3] = 0.0f;
    }
    return out;
}

static void compute_bounds_sphere(const std::vector<sr::math::Vec3>& pts, sr::math::Vec3& center,
                                  float& radius) {
    if (pts.empty()) {
        center = sr::math::Vec3{0.0f, 0.0f, 0.0f};
        radius = 1.0f;
        return;
    }
    sr::math::Vec3 mn = pts[0];
    sr::math::Vec3 mx = pts[0];
    for (const auto& p : pts) {
        mn.x = std::min(mn.x, p.x);
        mn.y = std::min(mn.y, p.y);
        mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x);
        mx.y = std::max(mx.y, p.y);
        mx.z = std::max(mx.z, p.z);
    }
    center = (mn + mx) * 0.5f;
    radius = 0.0f;
    for (const auto& p : pts) {
        radius = std::max(radius, sr::math::length(p - center));
    }
}

} // namespace

SkinnedModel load_fbx_skinned_model(const std::filesystem::path& path, AssetStore& store,
                                    const FbxSkinnedModelLoadOptions& opt) {
    ufbx_load_opts load_opts{};
    load_opts.target_axes.right = UFBX_COORDINATE_AXIS_POSITIVE_X;
    load_opts.target_axes.up = UFBX_COORDINATE_AXIS_POSITIVE_Y;
    load_opts.target_axes.front = UFBX_COORDINATE_AXIS_NEGATIVE_Z; // "front" is opposite of forward
    load_opts.target_unit_meters = 1.0f;
    load_opts.allow_nodes_out_of_root = false;

    ufbx_error err{};
    ufbx_scene* scene = ufbx_load_file(path.string().c_str(), &load_opts, &err);
    if (!scene) {
        throw std::runtime_error("ufbx_load_file failed: " + path.string());
    }

    auto base_dir = path.parent_path();

    // Pick first skinned mesh.
    const ufbx_mesh* mesh = nullptr;
    const ufbx_node* mesh_node = nullptr;
    const ufbx_skin_deformer* skin = nullptr;
    for (size_t mi = 0; mi < scene->meshes.count; ++mi) {
        const ufbx_mesh* m = scene->meshes.data[mi];
        if (!m || m->skin_deformers.count == 0)
            continue;
        mesh = m;
        skin = m->skin_deformers.data[0];
        if (m->instances.count > 0)
            mesh_node = m->instances.data[0];
        break;
    }
    if (!mesh || !skin) {
        ufbx_free_scene(scene);
        throw std::runtime_error("FBX has no skinned mesh: " + path.string());
    }
    if (!mesh_node)
        mesh_node = scene->root_node;

    SkinnedModel out;
    out.model = std::make_shared<Model>();

    // world_to_model: inverse of mesh node geometry_to_world in bind pose.
    ufbx_matrix inv_geom_world = ufbx_matrix_invert(&mesh_node->geometry_to_world);
    out.world_to_model = mat4_from_ufbx(inv_geom_world);

    // Skeleton: include all bones + parent chain up to root.
    std::unordered_set<const ufbx_node*> required;
    required.insert(scene->root_node);
    for (size_t ci = 0; ci < skin->clusters.count; ++ci) {
        const ufbx_skin_cluster* c = skin->clusters.data[ci];
        if (!c || !c->bone_node)
            continue;
        const ufbx_node* n = c->bone_node;
        while (n) {
            required.insert(n);
            if (n->is_root)
                break;
            n = n->parent;
        }
    }

    // Deterministic ordering: DFS from root, parents before children.
    std::vector<const ufbx_node*> ordered;
    ordered.reserve(required.size());
    std::unordered_map<const ufbx_node*, uint16_t> node_to_joint;
    node_to_joint.reserve(required.size());

    auto dfs = [&](auto&& self, const ufbx_node* n) -> void {
        if (!n)
            return;
        if (required.find(n) != required.end()) {
            uint16_t idx = uint16_t(ordered.size());
            ordered.push_back(n);
            node_to_joint.emplace(n, idx);
        }
        for (size_t i = 0; i < n->children.count; ++i) {
            self(self, n->children.data[i]);
        }
    };
    dfs(dfs, scene->root_node);

    out.skeleton.joints.resize(ordered.size());
    for (size_t i = 0; i < ordered.size(); ++i) {
        const ufbx_node* n = ordered[i];
        auto& j = out.skeleton.joints[i];
        j.name = normalize_joint_name(to_string(n->name));
        j.rest_local = trs_from_ufbx(n->local_transform);
        j.inv_bind = sr::math::Mat4::identity();

        int parent = -1;
        if (n->parent) {
            auto it = node_to_joint.find(n->parent);
            if (it != node_to_joint.end())
                parent = int(it->second);
        }
        j.parent = parent;
    }

    // Map skin cluster -> joint, store inverse bind matrices.
    std::vector<uint16_t> cluster_to_joint(skin->clusters.count, 0);
    for (size_t ci = 0; ci < skin->clusters.count; ++ci) {
        const ufbx_skin_cluster* c = skin->clusters.data[ci];
        if (!c || !c->bone_node)
            continue;
        auto it = node_to_joint.find(c->bone_node);
        if (it == node_to_joint.end())
            continue;
        uint16_t joint_i = it->second;
        cluster_to_joint[ci] = joint_i;
        out.skeleton.joints[joint_i].inv_bind = mat4_from_ufbx(c->geometry_to_bone);
    }

    // Materials.
    // Prefer per-instance materials from the mesh node, otherwise fall back to mesh materials.
    const ufbx_material_list* mats = &mesh_node->materials;
    if (mats->count == 0)
        mats = &mesh->materials;

    // Resolve diffuse/base texture once (we can expand this later).
    std::filesystem::path tex_path;
    if (!opt.override_diffuse_texture.empty()) {
        tex_path = resolve_texture(base_dir, opt.override_diffuse_texture);
    } else if (scene->texture_files.count > 0) {
        const ufbx_texture_file tf = scene->texture_files.data[0];
        std::string rel(tf.filename.data, tf.filename.length);
        tex_path = resolve_texture(base_dir, rel);
    }
    std::shared_ptr<sr::gfx::Texture> resolved_tex;
    if (!tex_path.empty() && std::filesystem::exists(tex_path))
        resolved_tex = store.get_texture(tex_path.string());

    if (mats->count == 0) {
        Material m;
        m.name = "default";
        m.front_face_ccw = opt.front_face_ccw;
        m.double_sided = opt.double_sided;
        m.base_color_tex = resolved_tex;
        out.model->materials.push_back(std::move(m));
    } else {
        for (size_t i = 0; i < mats->count; ++i) {
            Material m;
            const ufbx_material* mat = mats->data[i];
            m.name = mat ? to_string(mat->name) : ("mat_" + std::to_string(i));
            m.front_face_ccw = opt.front_face_ccw;
            m.double_sided = opt.double_sided;
            m.base_color_tex = resolved_tex;
            out.model->materials.push_back(std::move(m));
        }
    }

    // Build triangles grouped by FBX material parts.
    out.model->mesh.positions.clear();
    out.model->mesh.uvs.clear();
    out.model->mesh.indices.clear();
    out.bind_positions.clear();
    out.skin.clear();
    out.model->primitives.clear();

    auto append_index_vert = [&](uint32_t idx, uint32_t material_index) {
        (void)material_index;
        // Position.
        sr::math::Vec3 p{0.0f, 0.0f, 0.0f};
        if (mesh->vertex_position.exists && idx < mesh->vertex_position.indices.count) {
            uint32_t vi = mesh->vertex_position.indices.data[idx];
            if (vi < mesh->vertex_position.values.count) {
                const ufbx_vec3 v = mesh->vertex_position.values.data[vi];
                p = sr::math::Vec3{float(v.x), float(v.y), float(v.z)};
            }
        }

        // UV0.
        sr::math::Vec2 uv{0.0f, 0.0f};
        if (mesh->vertex_uv.exists && idx < mesh->vertex_uv.indices.count) {
            uint32_t ui = mesh->vertex_uv.indices.data[idx];
            if (ui < mesh->vertex_uv.values.count) {
                const ufbx_vec2 v = mesh->vertex_uv.values.data[ui];
                uv = sr::math::Vec2{float(v.x), float(v.y)};
            }
        }
        if (opt.flip_v)
            uv.y = 1.0f - uv.y;

        // Skin influences are indexed by logical mesh vertex (not by split attribute index).
        // Use `mesh->vertex_indices[idx]` to map from index -> logical vertex.
        uint32_t vertex_id = 0;
        if (idx < mesh->vertex_indices.count) {
            vertex_id = mesh->vertex_indices.data[idx];
        } else if (mesh->vertex_position.exists && idx < mesh->vertex_position.indices.count) {
            // Fallback: best-effort (works if positions are unique-per-vertex).
            vertex_id = mesh->vertex_position.indices.data[idx];
        }
        SkinInfluence4 inf = influences_from_vertex(*skin, vertex_id);
        for (int k = 0; k < 4; ++k) {
            uint16_t cl = inf.joint[k];
            if (cl < cluster_to_joint.size())
                inf.joint[k] = cluster_to_joint[cl];
            else
                inf.joint[k] = 0;
        }

        uint32_t out_i = uint32_t(out.model->mesh.positions.size());
        out.model->mesh.positions.push_back(p);
        out.model->mesh.uvs.push_back(uv);
        out.model->mesh.indices.push_back(out_i);
        out.bind_positions.push_back(p);
        out.skin.push_back(inf);
    };

    std::vector<uint32_t> tri;
    tri.resize(mesh->max_face_triangles * 3);

    // If no material parts, treat as one part using material 0.
    if (mesh->material_parts.count == 0) {
        Primitive prim;
        prim.index_offset = 0;
        prim.material_index = 0;

        for (size_t fi = 0; fi < mesh->faces.count; ++fi) {
            const ufbx_face face = mesh->faces.data[fi];
            if (face.num_indices < 3)
                continue;
            uint32_t ntri = ufbx_triangulate_face(tri.data(), tri.size(), mesh, face);
            for (uint32_t t = 0; t < ntri; ++t) {
                uint32_t i0 = tri[t * 3 + 0];
                uint32_t i1 = tri[t * 3 + 1];
                uint32_t i2 = tri[t * 3 + 2];
                append_index_vert(i0, 0);
                append_index_vert(i1, 0);
                append_index_vert(i2, 0);
            }
        }

        prim.index_count = uint32_t(out.model->mesh.indices.size());
        out.model->primitives.push_back(prim);
    } else {
        for (size_t pi = 0; pi < mesh->material_parts.count; ++pi) {
            const ufbx_mesh_part part = mesh->material_parts.data[pi];
            Primitive prim;
            uint32_t mi = part.index;
            if (!out.model->materials.empty())
                mi = std::min<uint32_t>(mi, uint32_t(out.model->materials.size() - 1));
            else
                mi = 0;
            prim.material_index = mi;
            prim.index_offset = uint32_t(out.model->mesh.indices.size());

            for (size_t i = 0; i < part.face_indices.count; ++i) {
                uint32_t fi = part.face_indices.data[i];
                if (fi >= mesh->faces.count)
                    continue;
                const ufbx_face face = mesh->faces.data[fi];
                if (face.num_indices < 3)
                    continue;
                uint32_t ntri = ufbx_triangulate_face(tri.data(), tri.size(), mesh, face);
                for (uint32_t t = 0; t < ntri; ++t) {
                    uint32_t i0 = tri[t * 3 + 0];
                    uint32_t i1 = tri[t * 3 + 1];
                    uint32_t i2 = tri[t * 3 + 2];
                    append_index_vert(i0, prim.material_index);
                    append_index_vert(i1, prim.material_index);
                    append_index_vert(i2, prim.material_index);
                }
            }

            prim.index_count = uint32_t(out.model->mesh.indices.size()) - prim.index_offset;
            if (prim.index_count > 0)
                out.model->primitives.push_back(prim);
        }
    }

    // Bounds.
    compute_bounds_sphere(out.bind_positions, out.model->bounds_center, out.model->bounds_radius);

    // Safety: ensure deformed positions vector exists and matches bind count.
    out.model->mesh.positions = out.bind_positions;

    // Quick sanity check: if almost all vertices are influenced only by joint 0, skinning will
    // look "nearly static". This usually means the index->vertex mapping was wrong.
    {
        size_t multi = 0;
        size_t non0 = 0;
        for (const auto& inf : out.skin) {
            int nonzero_w = 0;
            for (int k = 0; k < 4; ++k) {
                if (inf.weight[k] > 0.0001f)
                    nonzero_w++;
                if (inf.weight[k] > 0.0001f && inf.joint[k] != 0)
                    non0++;
            }
            if (nonzero_w > 1)
                multi++;
        }
        if (!out.skin.empty() && non0 < out.skin.size() / 50) {
            std::fprintf(stderr,
                         "[fbx] warning: skin influences look suspicious (non0=%zu of %zu, "
                         "multi=%zu)\n",
                         non0, out.skin.size(), multi);
        }
    }

    ufbx_free_scene(scene);
    return out;
}

AnimationClip load_fbx_animation_clip(const std::filesystem::path& path, const Skeleton& skel,
                                      const std::string& clip_name, float sample_rate) {
    ufbx_load_opts load_opts{};
    load_opts.target_axes.right = UFBX_COORDINATE_AXIS_POSITIVE_X;
    load_opts.target_axes.up = UFBX_COORDINATE_AXIS_POSITIVE_Y;
    load_opts.target_axes.front = UFBX_COORDINATE_AXIS_NEGATIVE_Z;
    load_opts.target_unit_meters = 1.0f;

    ufbx_error err{};
    ufbx_scene* scene = ufbx_load_file(path.string().c_str(), &load_opts, &err);
    if (!scene) {
        throw std::runtime_error("ufbx_load_file failed: " + path.string());
    }

    const ufbx_anim* anim = scene->anim;
    double t0 = 0.0;
    double t1 = 0.0;

    // Pick the longest anim stack if available (Kenney's FBXs tend to store clips there).
    double best_dur = 0.0;
    if (scene->anim_stacks.count > 0) {
        for (size_t i = 0; i < scene->anim_stacks.count; ++i) {
            const ufbx_anim_stack* st = scene->anim_stacks.data[i];
            if (!st)
                continue;
            double a = st->time_begin;
            double b = st->time_end;
            double d = b - a;
            if (d > best_dur) {
                best_dur = d;
                t0 = a;
                t1 = b;
                if (st->anim)
                    anim = st->anim;
            }
        }
    }

    if (anim) {
        // Fall back to default anim range if stack range absent.
        if (t1 <= t0) {
            t0 = anim->time_begin;
            t1 = anim->time_end;
        }
    }

    // Last-resort: derive time range from animation curves (some files leave stack/default ranges 0).
    if (t1 <= t0 && scene->anim_curves.count > 0) {
        double mn = 0.0;
        double mx = 0.0;
        bool any = false;
        for (size_t i = 0; i < scene->anim_curves.count; ++i) {
            const ufbx_anim_curve* c = scene->anim_curves.data[i];
            if (!c || c->keyframes.count == 0)
                continue;
            if (!any) {
                mn = c->min_time;
                mx = c->max_time;
                any = true;
            } else {
                mn = std::min(mn, c->min_time);
                mx = std::max(mx, c->max_time);
            }
        }
        if (any) {
            t0 = mn;
            t1 = mx;
        }
    }

    if (t1 <= t0) {
        // If still unknown, pick something reasonable so we can at least sample at t=0..1.
        t0 = 0.0;
        t1 = 1.0;
    }

    if (!anim) {
        ufbx_free_scene(scene);
        throw std::runtime_error("FBX has no anim data: " + path.string());
    }

    AnimationClip clip;
    clip.name = clip_name;
    clip.sample_rate = (sample_rate > 0.0f) ? sample_rate : 30.0f;
    clip.num_joints = uint32_t(skel.joints.size());

    double dur = std::max(0.0, t1 - t0);
    clip.duration = float(dur);

    // Map nodes by name for this clip.
    std::unordered_map<std::string, const ufbx_node*> node_by_name;
    node_by_name.reserve(scene->nodes.count);
    for (size_t i = 0; i < scene->nodes.count; ++i) {
        const ufbx_node* n = scene->nodes.data[i];
        if (!n)
            continue;
        node_by_name.emplace(normalize_joint_name(to_string(n->name)), n);
    }

    // Debug signal if name matching is failing (T-pose).
    size_t matched = 0;
    for (const auto& j : skel.joints) {
        if (node_by_name.find(j.name) != node_by_name.end())
            matched++;
    }
    if (matched < std::min<size_t>(skel.joints.size(), 8)) {
        std::fprintf(stderr,
                     "[fbx] clip '%s' weak joint-name match: %zu/%zu (t0=%.3f t1=%.3f) file=%s\n",
                     clip_name.c_str(), matched, skel.joints.size(), t0, t1,
                     path.string().c_str());
    }

    // Bake samples uniformly.
    const float dt = 1.0f / clip.sample_rate;
    uint32_t frames = 1;
    if (clip.duration > 0.0f)
        frames = uint32_t(std::ceil(clip.duration / dt)) + 1;

    clip.samples.resize(size_t(frames) * clip.num_joints);
    for (uint32_t f = 0; f < frames; ++f) {
        double t = t0 + double(f) * double(dt);
        if (t > t1)
            t = t1;
        for (uint32_t j = 0; j < clip.num_joints; ++j) {
            const auto& sj = skel.joints[j];
            sr::math::Trs trs = sj.rest_local;
            auto it = node_by_name.find(sj.name);
            if (it != node_by_name.end()) {
                ufbx_transform et = ufbx_evaluate_transform(anim, it->second, t);
                trs = trs_from_ufbx(et);
            }
            clip.samples[size_t(f) * clip.num_joints + j] = trs;
        }
    }

    ufbx_free_scene(scene);
    return clip;
}

} // namespace sr::assets
