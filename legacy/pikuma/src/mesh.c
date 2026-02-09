#include <stdlib.h>
#include <math.h>

#include "mesh.h"
#include "utils.h"
#include "mat4.h"

Mesh *mesh_new(void)
{
    Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
    if (!mesh)
        return NULL;

    mesh->vertices = NULL;
    mesh->normals = NULL;
    mesh->texcoords = NULL;

    return mesh;
}

void mesh_free(Mesh *mesh)
{
    if (!mesh)
        return;

    if (mesh->vertices)
        sfa_free(mesh->vertices);
    if (mesh->normals)
        sfa_free(mesh->normals);
    if (mesh->texcoords)
        sfa_free(mesh->texcoords);

    free(mesh);
}

// Function to create a deep copy of the given mesh
Mesh *mesh_copy(const Mesh *mesh)
{
    if (!mesh)
    {
        return NULL; // Handle invalid mesh
    }

    // Create a new mesh with the same number of vertices and indices
    Mesh *copy = mesh_new();
    if (!copy)
    {
        return NULL; // Handle allocation failure
    }

    // Copy vertices
    if (mesh->vertices)
    {
        copy->vertices = sfa_new(mesh->vertices->length);
        if (!copy->vertices)
        {
            mesh_free(copy);
            return NULL; // Handle allocation failure
        }
        memcpy(copy->vertices->data, mesh->vertices->data, sizeof(float) * mesh->vertices->length);
    }

    // Copy normals
    if (mesh->normals)
    {
        copy->normals = sfa_new(mesh->normals->length);
        if (!copy->normals)
        {
            mesh_free(copy);
            return NULL; // Handle allocation failure
        }
        memcpy(copy->normals->data, mesh->normals->data, sizeof(float) * mesh->normals->length);
    }

    // Copy texture coordinates
    if (mesh->texcoords)
    {
        copy->texcoords = sfa_new(mesh->texcoords->length);
        if (!copy->texcoords)
        {
            mesh_free(copy);
            return NULL; // Handle allocation failure
        }
        memcpy(copy->texcoords->data, mesh->texcoords->data, sizeof(float) * mesh->texcoords->length);
    }

    return copy;
}

// mesh_transform, takes in a mesh and a transformation mat4 and applies it
void mesh_transform(Mesh *mesh, Mat4 transform)
{
    if (!mesh || !mesh->vertices || !mesh->vertices->data)
    {
        return; // Handle invalid mesh
    }

    // Iterate over each vertex and apply transformation
    for (int i = 0; i < mesh->vertices->length; i += 3)
    {
        Vec3 vertex = {mesh->vertices->data[i], mesh->vertices->data[i + 1], mesh->vertices->data[i + 2]};
        Vec3 transformed = mat4_multiply_vec3(transform, vertex);

        mesh->vertices->data[i] = transformed.x;
        mesh->vertices->data[i + 1] = transformed.y;
        mesh->vertices->data[i + 2] = transformed.z;
    }
}
