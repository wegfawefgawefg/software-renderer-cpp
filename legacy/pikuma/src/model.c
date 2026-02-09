#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "model.h"
#include "shape.h"
#include "mesh.h"
#include "sfa.h"
#include "su32a.h"
#include "utils.h" // Assume this contains trim_whitespace and other helper functions

#define MAX_LINE_LENGTH 1024

// Helper function to create a new Shape
static int initialize_shape(Shape *shape)
{
    shape->material_name = NULL;
    shape->vertex_indices = su32a_new(0);
    shape->normal_indices = su32a_new(0);
    shape->texcoord_indices = su32a_new(0);
    if (!shape->vertex_indices || !shape->normal_indices || !shape->texcoord_indices)
    {
        fprintf(stderr, "Failed to initialize Shape's indices.\n");
        return -1;
    }
    return 0;
}

Model *model_load_from_file(const char *filename)
{
    if (!filename)
    {
        fprintf(stderr, "model_load_from_file: filename is NULL.\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("fopen");
        fprintf(stderr, "Failed to open OBJ file: %s\n", filename);
        return NULL;
    }

    // First Pass: Count vertices, texcoords, normals, shapes, and faces per shape
    int vertex_count = 0;
    int texcoord_count = 0;
    int normal_count = 0;
    int shape_count = 0;
    int face_count = 0;

    // To store the number of faces per shape
    int *faces_per_shape = NULL;

    char line[MAX_LINE_LENGTH];
    int current_shape_faces = 0;
    while (fgets(line, sizeof(line), file))
    {
        char *trimmed = trim_whitespace(line);

        if (strlen(trimmed) == 0 || trimmed[0] == '#')
        {
            continue;
        }

        if (strncmp(trimmed, "v ", 2) == 0)
        {
            vertex_count++;
        }
        else if (strncmp(trimmed, "vt ", 3) == 0)
        {
            texcoord_count++;
        }
        else if (strncmp(trimmed, "vn ", 3) == 0)
        {
            normal_count++;
        }
        else if (strncmp(trimmed, "o ", 2) == 0 || strncmp(trimmed, "g ", 2) == 0)
        {
            // If not the first shape, store the face count of the previous shape
            if (shape_count > 0)
            {
                faces_per_shape = realloc(faces_per_shape, sizeof(int) * shape_count);
                if (!faces_per_shape)
                {
                    fprintf(stderr, "Failed to allocate memory for faces_per_shape.\n");
                    fclose(file);
                    return NULL;
                }
                faces_per_shape[shape_count - 1] = current_shape_faces;
                current_shape_faces = 0;
            }
            shape_count++;
        }
        else if (strncmp(trimmed, "f ", 2) == 0)
        {
            face_count++;
            current_shape_faces++;
        }
    }

    // After the loop, store the face count for the last shape
    if (shape_count > 0)
    {
        faces_per_shape = realloc(faces_per_shape, sizeof(int) * shape_count);
        if (!faces_per_shape)
        {
            fprintf(stderr, "Failed to allocate memory for faces_per_shape.\n");
            fclose(file);
            return NULL;
        }
        faces_per_shape[shape_count - 1] = current_shape_faces;
    }

    // Reset file pointer to beginning for second pass
    rewind(file);

    // Allocate Model structure
    Model *model = (Model *)malloc(sizeof(Model));
    if (!model)
    {
        fprintf(stderr, "Failed to allocate memory for Model.\n");
        fclose(file);
        free(faces_per_shape);
        return NULL;
    }
    memset(model, 0, sizeof(Model));
    model->mesh = mesh_new();
    if (!model->mesh)
    {
        fprintf(stderr, "Failed to initialize Mesh.\n");
        fclose(file);
        free(model);
        free(faces_per_shape);
        return NULL;
    }

    // Allocate memory for Mesh vertices, texcoords, normals
    if (vertex_count > 0)
    {
        model->mesh->vertices = sfa_new(vertex_count * 3);
        if (!model->mesh->vertices)
        {
            fprintf(stderr, "Failed to allocate Mesh vertices.\n");
            model_free(model);
            fclose(file);
            free(faces_per_shape);
            return NULL;
        }
    }

    if (texcoord_count > 0)
    {
        model->mesh->texcoords = sfa_new(texcoord_count * 2);
        if (!model->mesh->texcoords)
        {
            fprintf(stderr, "Failed to allocate Mesh texcoords.\n");
            model_free(model);
            fclose(file);
            free(faces_per_shape);
            return NULL;
        }
    }

    if (normal_count > 0)
    {
        model->mesh->normals = sfa_new(normal_count * 3);
        if (!model->mesh->normals)
        {
            fprintf(stderr, "Failed to allocate Mesh normals.\n");
            model_free(model);
            fclose(file);
            free(faces_per_shape);
            return NULL;
        }
    }

    // Allocate memory for Shapes
    if (shape_count > 0)
    {
        model->shapes = (Shape *)malloc(sizeof(Shape) * shape_count);
        if (!model->shapes)
        {
            fprintf(stderr, "Failed to allocate Shapes array.\n");
            model_free(model);
            fclose(file);
            free(faces_per_shape);
            return NULL;
        }
        memset(model->shapes, 0, sizeof(Shape) * shape_count);
        model->shape_count = shape_count;

        // Initialize each Shape and allocate memory for indices based on faces_per_shape
        for (int i = 0; i < shape_count; i++)
        {
            if (initialize_shape(&model->shapes[i]) != 0)
            {
                fprintf(stderr, "Failed to initialize Shape %d.\n", i);
                model_free(model);
                fclose(file);
                free(faces_per_shape);
                return NULL;
            }
            // Preallocate indices arrays
            if (model->shapes[i].vertex_indices)
            {
                model->shapes[i].vertex_indices->data = (uint32_t *)malloc(sizeof(uint32_t) * faces_per_shape[i] * 3);
                if (!model->shapes[i].vertex_indices->data)
                {
                    fprintf(stderr, "Failed to allocate vertex_indices for Shape %d.\n", i);
                    model_free(model);
                    fclose(file);
                    free(faces_per_shape);
                    return NULL;
                }
                model->shapes[i].vertex_indices->length = 0;
            }
            if (model->shapes[i].texcoord_indices)
            {
                model->shapes[i].texcoord_indices->data = (uint32_t *)malloc(sizeof(uint32_t) * faces_per_shape[i] * 3);
                if (!model->shapes[i].texcoord_indices->data)
                {
                    fprintf(stderr, "Failed to allocate texcoord_indices for Shape %d.\n", i);
                    model_free(model);
                    fclose(file);
                    free(faces_per_shape);
                    return NULL;
                }
                model->shapes[i].texcoord_indices->length = 0;
            }
            if (model->shapes[i].normal_indices)
            {
                model->shapes[i].normal_indices->data = (uint32_t *)malloc(sizeof(uint32_t) * faces_per_shape[i] * 3);
                if (!model->shapes[i].normal_indices->data)
                {
                    fprintf(stderr, "Failed to allocate normal_indices for Shape %d.\n", i);
                    model_free(model);
                    fclose(file);
                    free(faces_per_shape);
                    return NULL;
                }
                model->shapes[i].normal_indices->length = 0;
            }
        }
    }

    free(faces_per_shape); // No longer needed

    // Counters to keep track of current indices
    int current_vertex = 0;
    int current_texcoord = 0;
    int current_normal = 0;

    // Shape tracking variables
    int current_shape_index = -1;

    // Second Pass: Populate Mesh and Shapes
    while (fgets(line, sizeof(line), file))
    {
        char *trimmed = trim_whitespace(line);

        if (strlen(trimmed) == 0 || trimmed[0] == '#')
        {
            continue;
        }

        if (strncmp(trimmed, "mtllib", 6) == 0 && isspace(trimmed[6]))
        {
            // Material library (already handled in first pass)
            char mtl_filename[MAX_LINE_LENGTH];
            if (sscanf(trimmed, "mtllib %s", mtl_filename) == 1)
            {
                model->material_library_name = strdup(mtl_filename);
                if (!model->material_library_name)
                {
                    fprintf(stderr, "Failed to duplicate material library name.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "Failed to parse mtllib line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "o ", 2) == 0 || strncmp(trimmed, "g ", 2) == 0)
        {
            // Object or Group name, start a new Shape
            char shape_name[MAX_LINE_LENGTH];
            if (sscanf(trimmed, "%*s %[^\n]", shape_name) == 1)
            { // %*s skips "o" or "g"
                // Find the next shape in the shapes array
                current_shape_index++;
                if (current_shape_index >= model->shape_count)
                {
                    fprintf(stderr, "More shapes in file than allocated.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                // Set the shape's name
                if (model->shapes[current_shape_index].name)
                {
                    free(model->shapes[current_shape_index].name);
                }
                model->shapes[current_shape_index].name = strdup(shape_name);
                if (!model->shapes[current_shape_index].name)
                {
                    fprintf(stderr, "Failed to duplicate shape name.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "Failed to parse shape name line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "usemtl", 6) == 0 && isspace(trimmed[6]))
        {
            // Material usage
            char material_name[MAX_LINE_LENGTH];
            if (sscanf(trimmed, "usemtl %s", material_name) == 1)
            {
                if (current_shape_index < 0 || current_shape_index >= model->shape_count)
                {
                    fprintf(stderr, "usemtl encountered before any shape definition.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                Shape *current_shape = &model->shapes[current_shape_index];
                // Assign material name
                if (current_shape->material_name)
                {
                    free(current_shape->material_name);
                }
                current_shape->material_name = strdup(material_name);
                if (!current_shape->material_name)
                {
                    fprintf(stderr, "Failed to duplicate material name string.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "Failed to parse usemtl line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "v ", 2) == 0)
        {
            // Vertex position (already allocated in first pass)
            float x, y, z;
            if (sscanf(trimmed, "v %f %f %f", &x, &y, &z) == 3)
            {
                if (current_vertex >= vertex_count)
                {
                    fprintf(stderr, "Vertex index out of bounds during second pass.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                model->mesh->vertices->data[current_vertex * 3] = x;
                model->mesh->vertices->data[current_vertex * 3 + 1] = y;
                model->mesh->vertices->data[current_vertex * 3 + 2] = z;
                current_vertex++;
            }
            else
            {
                fprintf(stderr, "Invalid vertex line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "vt ", 3) == 0)
        {
            // Texture coordinate (already allocated in first pass)
            float u, v;
            if (sscanf(trimmed, "vt %f %f", &u, &v) == 2)
            {
                if (current_texcoord >= texcoord_count)
                {
                    fprintf(stderr, "Texcoord index out of bounds during second pass.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                model->mesh->texcoords->data[current_texcoord * 2] = u;
                model->mesh->texcoords->data[current_texcoord * 2 + 1] = v;
                current_texcoord++;
            }
            else
            {
                fprintf(stderr, "Invalid texture coordinate line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "vn ", 3) == 0)
        {
            // Vertex normal (already allocated in first pass)
            float nx, ny, nz;
            if (sscanf(trimmed, "vn %f %f %f", &nx, &ny, &nz) == 3)
            {
                if (current_normal >= normal_count)
                {
                    fprintf(stderr, "Normal index out of bounds during second pass.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                model->mesh->normals->data[current_normal * 3] = nx;
                model->mesh->normals->data[current_normal * 3 + 1] = ny;
                model->mesh->normals->data[current_normal * 3 + 2] = nz;
                current_normal++;
            }
            else
            {
                fprintf(stderr, "Invalid normal line: %s\n", trimmed);
            }
        }
        else if (strncmp(trimmed, "f ", 2) == 0)
        {
            // Face (already allocated memory for indices)
            // Expecting triangular faces: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            uint32_t v_idx[3], vt_idx[3], vn_idx[3];
            int matches = sscanf(trimmed, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                                 &v_idx[0], &vt_idx[0], &vn_idx[0],
                                 &v_idx[1], &vt_idx[1], &vn_idx[1],
                                 &v_idx[2], &vt_idx[2], &vn_idx[2]);

            if (matches == 9)
            {
                // Convert to 0-based indices
                for (int i = 0; i < 3; i++)
                {
                    if (v_idx[i] == 0 || vt_idx[i] == 0 || vn_idx[i] == 0)
                    {
                        fprintf(stderr, "OBJ indices are 1-based and must be positive.\n");
                        model_free(model);
                        fclose(file);
                        return NULL;
                    }
                    v_idx[i] -= 1;
                    vt_idx[i] -= 1;
                    vn_idx[i] -= 1;
                }

                // Ensure there is a current shape
                if (current_shape_index < 0 || current_shape_index >= model->shape_count)
                {
                    fprintf(stderr, "Face encountered before any shape definition.\n");
                    model_free(model);
                    fclose(file);
                    return NULL;
                }
                Shape *current_shape = &model->shapes[current_shape_index];

                // Append indices to the current shape
                // Removed incorrect boundary checks
                current_shape->vertex_indices->data[current_shape->vertex_indices->length] = v_idx[0];
                current_shape->vertex_indices->data[current_shape->vertex_indices->length + 1] = v_idx[1];
                current_shape->vertex_indices->data[current_shape->vertex_indices->length + 2] = v_idx[2];
                current_shape->vertex_indices->length += 3;

                current_shape->texcoord_indices->data[current_shape->texcoord_indices->length] = vt_idx[0];
                current_shape->texcoord_indices->data[current_shape->texcoord_indices->length + 1] = vt_idx[1];
                current_shape->texcoord_indices->data[current_shape->texcoord_indices->length + 2] = vt_idx[2];
                current_shape->texcoord_indices->length += 3;

                current_shape->normal_indices->data[current_shape->normal_indices->length] = vn_idx[0];
                current_shape->normal_indices->data[current_shape->normal_indices->length + 1] = vn_idx[1];
                current_shape->normal_indices->data[current_shape->normal_indices->length + 2] = vn_idx[2];
                current_shape->normal_indices->length += 3;
            }
            else
            {
                // Handle other face formats (e.g., f v//vn, f v/vt, f v)
                // For simplicity, we can skip or implement additional parsing
                fprintf(stderr, "Unsupported face format: %s\n", trimmed);
                // Optionally, implement additional parsing here
            }
        }
        else
        {
            // Other prefixes can be handled here if needed
            // For example, 's' for smoothing groups
            // Currently, we skip them
            continue;
        }
    }

    fclose(file);

    // Optionally, set the model's name based on the filename
    const char *base_filename = strrchr(filename, '/');
    if (!base_filename)
    {
        base_filename = filename;
    }
    else
    {
        base_filename += 1;
    }
    model->name = strdup(base_filename);
    if (!model->name)
    {
        fprintf(stderr, "Failed to duplicate model name.\n");
        model_free(model);
        return NULL;
    }

    return model;
}

void model_free(Model *model)
{
    if (model)
    {
        if (model->name)
        {
            free(model->name);
        }
        if (model->mesh)
        {
            mesh_free(model->mesh);
        }
        if (model->shapes)
        {
            for (int i = 0; i < model->shape_count; i++)
            {
                shape_free(&model->shapes[i]);
            }
            free(model->shapes);
        }
        free(model);
    }
}

void model_print(const Model *model)
{
    if (!model)
    {
        printf("Model is NULL.\n");
        return;
    }

    printf("Model: %s\n", model->name ? model->name : "Unknown Name");
    printf("Material Library: %s\n", model->material_library_name ? model->material_library_name : "Unknown Library");

    if (model->mesh)
    {
        printf("Mesh:\n");
        printf("  Vertices: %d\n", model->mesh->vertices ? model->mesh->vertices->length / 3 : 0);
        printf("  Normals: %d\n", model->mesh->normals ? model->mesh->normals->length / 3 : 0);
        printf("  Texcoords: %d\n", model->mesh->texcoords ? model->mesh->texcoords->length / 2 : 0);
    }

    printf("Shapes:\n");
    for (int i = 0; i < model->shape_count; i++)
    {
        printf("  Shape %d:\n", i + 1);
        printf("    Name: %s\n", model->shapes[i].name ? model->shapes[i].name : "Unknown Shape");
        printf("    Material: %s\n", model->shapes[i].material_name ? model->shapes[i].material_name : "Unknown Material");
        printf("    Num_Faces: %d\n", model->shapes[i].vertex_indices ? model->shapes[i].vertex_indices->length / 3 : 0);
    }
}

Shape *model_get_shape(const Model *model, const char *name)
{
    for (size_t i = 0; i < model->shape_count; i++)
    {
        if (strcmp(model->shapes[i].name, name) == 0)
        {
            return &model->shapes[i];
        }
    }
    return NULL;
}
