
#ifndef MATERIAL_LIBRARY_H
#define MATERIAL_LIBRARY_H

#include "material.h"

#include <stddef.h>

// represents a material library loaded from a.mtl file
// a mtl file for a model probably contains many materials, so all of them are here in a group
typedef struct
{
    char *filename;      // Name of the .mtl file
    Material *materials; // Dynamic array of materials
    size_t count;        // Number of materials loaded
} MaterialLibrary;

// Loads materials from a .mtl file into a MaterialLibrary.
// If loading fails, filename is NULL, materials is NULL, and count is 0.
MaterialLibrary material_library_load(const char *path);
void material_library_free(MaterialLibrary *library);
void material_library_print(const MaterialLibrary *library);

// gets material by name from the library
// names could be anything. not file names, and are defined in the .mtl file and referenced in the .obj file
Material *material_library_get_material(const MaterialLibrary *library, const char *name);

#endif