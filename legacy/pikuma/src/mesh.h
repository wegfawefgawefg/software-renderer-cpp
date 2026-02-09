#ifndef MESH_H
#define MESH_H

#include "sfa.h"
#include "mat4.h"
#include "su32a.h"

//////////////////////// PRIMITIVES ////////////////////////
// MESH: Combined Vertex and Index Data
typedef struct
{
    SFA *vertices;  // Vertex data
    SFA *normals;   // Normal data
    SFA *texcoords; // Texture coordinate data

    // SU32A *vertex_indices;   // Index data
    // SU32A *normal_indices;   // Normal index data
    // SU32A *texcoord_indices; // Texture coordinate index data
    // SU32A *colors;           // Color data
} Mesh;

Mesh *mesh_new(void);
void mesh_free(Mesh *mesh);
Mesh *mesh_copy(const Mesh *mesh);

#endif