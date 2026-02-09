#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include "shape.h"

#include <stddef.h>

// Model struct holds shape shared vertex data and an array of shapes
typedef struct
{
    char *name;                  // model name
    char *material_library_name; // Material library

    Mesh *mesh;    // vertices/normals/texcoords
    Shape *shapes; // array of shapes (submeshes)
    size_t shape_count;
} Model;

Model *model_load_from_file(const char *filename);
void model_free(Model *model);
void model_print(const Model *model);

Shape *model_get_shape(const Model *model, const char *name);

#endif