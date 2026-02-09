#ifndef SHAPE_H
#define SHAPE_H

#include "su32a.h"

// Shape (submesh) struct to hold indices and material info
typedef struct
{
    char *name;          // Shape name
    char *material_name; // Material reference

    SU32A *vertex_indices;   // Indices into MeshData->vertices
    SU32A *normal_indices;   // Indices into MeshData->normals
    SU32A *texcoord_indices; // Indices into MeshData->texcoords
} Shape;

Shape *shape_new(void);
void shape_free(Shape *shape);

#endif // SHAPE_H