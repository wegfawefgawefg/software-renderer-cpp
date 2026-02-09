#ifndef MATERIAL_MANAGEMENT_H
#define MATERIAL_MANAGEMENT_H

#include "material.h"
#include "material_library.h"

// objects will generally have one mtl collection they use, we have to store them somewhere
typedef struct
{
    MaterialLibrary *libraries; // Array of material libraries
    size_t count;               // Number of libraries loaded
} MaterialManager;

// loads all material libraries in directory, returns 0 on success, -1 on failure.
MaterialManager *material_manager_load_from_directory(const char *directory_path);
void material_manager_free(MaterialManager *manager);

// get a material library by its filename (ex: peaches_castle.mtl), or NULL if not found
MaterialLibrary *material_manager_get_library(const MaterialManager *manager, const char *name);
void material_manager_print(const MaterialManager *manager);

#endif
