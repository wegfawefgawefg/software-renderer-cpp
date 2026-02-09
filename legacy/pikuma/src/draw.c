#include <math.h>

#include "globals.h"
#include "draw.h"
#include "draw_lib.h"
#include "primitives.h"
#include "input.h"
#include "colors.h"
#include "texture.h"
#include "assets.h"
#include "utils.h"
#include "vec2.h"
#include "vec3.h"
#include "sfa.h"
#include "mesh.h"
#include "mat4.h"
#include "projection.h"
#include "f_texture.h"
#include "light.h"

void draw_mesh(
    Texture *pb,
    FTexture *z_buffer,

    State *state,
    Texture *texture,
    SFA *vertices,
    SU32A *indices,
    SFA *texcoords,
    SU32A *texcoord_indices,

    Vec3 pos,
    Vec3 rot,
    Vec3 scale)
{

    Mat4 model = mat4_create_model(pos, rot, scale);
    SFA *model_transformed_vertices = sfa_transform_vertices(vertices, &model);
    // calculate normals
    // vertices are still in x y z x y z x y z
    int num_faces = indices->length / 3;
    SFA *normals = sfa_new(num_faces * 3);
    for (int face = 0; face < num_faces; face++)
    {
        int idx1 = indices->data[face * 3];
        int idx2 = indices->data[face * 3 + 1];
        int idx3 = indices->data[face * 3 + 2];

        Vec3 p1 = vec3_create(
            model_transformed_vertices->data[idx1 * 4],
            model_transformed_vertices->data[idx1 * 4 + 1],
            model_transformed_vertices->data[idx1 * 4 + 2]);
        Vec3 p2 = vec3_create(
            model_transformed_vertices->data[idx2 * 4],
            model_transformed_vertices->data[idx2 * 4 + 1],
            model_transformed_vertices->data[idx2 * 4 + 2]);
        Vec3 p3 = vec3_create(
            model_transformed_vertices->data[idx3 * 4],
            model_transformed_vertices->data[idx3 * 4 + 1],
            model_transformed_vertices->data[idx3 * 4 + 2]);

        Vec3 normal = vec3_cross(vec3_sub(p2, p1), vec3_sub(p3, p1));
        normal = vec3_normalize(normal);

        normals->data[face] = normal.x;
        normals->data[face + 1] = normal.y;
        normals->data[face + 2] = normal.z;
    }

    Mat4 vp = mat4_create_vp(
        state->camera_pos, state->camera_target, state->camera_up,
        // degrees_to_radians(90.0), (float)pb->width / (float)pb->height, 0.1f, 100.0f);
        degrees_to_radians(90.0), (float)pb->width / (float)pb->height, 2.0f, 200.0f);

    Mat4 mvp = mat4_multiply(vp, model);

    SFA *transformed_vertices // x y z w
        = sfa_transform_vertices(vertices, &mvp);

    perspective_divide(transformed_vertices); // Implement perspective_divide

    // Map to screen coordinates with camera space distance
    SFA *screen_coords // x y depth
        = sfa_new(transformed_vertices->length / 4 * 3);
    map_to_screen_keep_z(transformed_vertices, screen_coords, pb->width, pb->height);

    // calculate cam dir
    Vec3 cam_dir = vec3_sub(state->camera_target, state->camera_pos);
    cam_dir = vec3_normalize(cam_dir);

    draw_tris_textured(
        pb,
        texture,
        z_buffer,
        screen_coords,
        indices,
        texcoords,
        texcoord_indices,
        normals,
        cam_dir);

    // Render lines
    // draw_tris_lines_with_depth(pb, screen_coords, indices, 0xFFFFFF09);

    // Cleanup
    sfa_free(model_transformed_vertices);
    sfa_free(transformed_vertices);
    sfa_free(screen_coords);
    sfa_free(normals);
}

void draw(Texture *pb, FTexture *z_buffer, State *state, Assets *assets)
{
    // IVec2 screen_center = ivec2_create(pb->width / 2, pb->height / 2);
    draw_grid(pb, ivec2_create(0, 0), ivec2_create(pb->width - 1, pb->height - 1), 20, COLOR_GRAY_DARK);

    // every 10 frames increment earth_mft
    if (state->frame_count % 4 == 0)
    {
        mft_next_frame(assets->earth_mft);
    }

    float scalef = 200.0;
    // Model *model = model_manager_get_model(assets->model_manager, "gba.obj");
    // Texture *texture = texture_manager_get(assets->texture_manager, "gba.png");
    // Shape *shape = model_get_shape(model, "gba");
    // // Mesh *mesh = assets->earth_mesh;
    // // Texture *texture = assets->manhat_texture;
    // // Texture *texture = assets->earth_mft->frames[assets->earth_mft->current_frame];

    // float x_pos = 0.0;
    // float y_pos = 0.0;
    // float z_pos = 0.0;
    // float x_angle = 0.0;
    // float y_angle = state->frame_count * 0.01 - 0.3;
    // // float y_angle = radians_to_degrees(90.0);
    // float z_angle = 0.0;
    // draw_mesh(
    //     pb,
    //     z_buffer,

    //     state,
    //     texture,
    //     model->mesh->vertices,
    //     shape->vertex_indices,
    //     model->mesh->texcoords,
    //     shape->texcoord_indices,

    //     vec3_create(x_pos, y_pos, z_pos),       // position
    //     vec3_create(x_angle, y_angle, z_angle), // rotation
    //     vec3_create(scalef, scalef, scalef)     // scale
    // );

    // draw peaches_castle.obj
    // we have to loop through all the shapes in the model
    // and draw them
    Model *model;
    Shape *shape;
    Texture *texture;
    model = model_manager_get_model(assets->model_manager, "peaches_castle.obj");
    MaterialLibrary *material_library = material_manager_get_library(assets->material_manager, model->material_library_name);
    for (int i = 0; i < model->shape_count; i++)
    {
        shape = &model->shapes[i];
        Material *material = material_library_get_material(material_library, shape->material_name);
        // print the diffuse_map name
        texture = texture_manager_get(assets->texture_manager, material->diffuse_map);
        draw_mesh(
            pb,
            z_buffer,

            state,
            texture,
            model->mesh->vertices,
            shape->vertex_indices,
            model->mesh->texcoords,
            shape->texcoord_indices,

            vec3_create(0.0, 0.0, 0.0),         // position
            vec3_create(0.0, 0.0, 0.0),         // rotation
            vec3_create(scalef, scalef, scalef) // scale
        );
    }

    // Vec2 mouse_pos = ivec2_to_vec2(get_mouse_pos());
    // draw_cursor(pb, mouse_pos.x, mouse_pos.y, 10, 0xFFFFFFFF);
}
