// #define _DEFAULT_SOURCE // Ensure feature test macros are set

#include "material_management.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For directory operations
#include <sys/types.h>
#include <errno.h>

#include "material_library.h"
#include "utils.h"

// Helper function to check if a file has a .obj extension (case-insensitive)
static int has_mtl_extension(const char *filename)
{
    if (!filename)
        return 0;
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return 0;
    return (strcasecmp(dot, ".mtl") == 0); // Case-insensitive comparison
}

MaterialManager *material_manager_load_from_directory(const char *directory_path)
{
    if (!directory_path)
    {
        fprintf(stderr, "Invalid arguments to material_manager_load_from_directory.\n");
        return NULL;
    }

    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory_path);
    if (!dir)
    {
        perror("opendir");
        fprintf(stderr, "Failed to open directory: %s\n", directory_path);
        return NULL;
    }

    // allocate memory for the MaterialManager
    MaterialManager *manager = (MaterialManager *)malloc(sizeof(MaterialManager));
    if (!manager)
    {
        fprintf(stderr, "Memory allocation failed for MaterialManager.\n");
        closedir(dir);
        return NULL;
    }
    manager->count = 0;
    manager->libraries = NULL;

    // Count the number of .mtl files in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_mtl_extension(filename))
        {
            manager->count += 1;
        }
    }
    rewinddir(dir);

    // Allocate memory for the Model array
    manager->libraries = (MaterialLibrary *)malloc(manager->count * sizeof(MaterialLibrary));
    if (manager->libraries == NULL)
    {
        fprintf(stderr, "Memory allocation failed for MaterialLibrary array.\n");
        closedir(dir);
        free(manager);
        return NULL;
    }

    int current_library_index = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_mtl_extension(filename))
        {
            // Construct the full path for loading the model
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, filename);

            // Load the material library
            MaterialLibrary library = material_library_load(full_path);
            if (library.materials == NULL)
            {
                fprintf(stderr, "Failed to load material library from file: %s\n", full_path);
                // Continue to next file
                continue;
            }

            // Assign to MaterialManager
            manager->libraries[current_library_index] = library;
            current_library_index += 1;
        }
    }

    closedir(dir);
    return manager;
}

MaterialLibrary *material_manager_get_library(const MaterialManager *manager, const char *name)
{
    if (manager == NULL || name == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < manager->count; i++)
    {
        if (strcmp(manager->libraries[i].filename, name) == 0)
        {
            return &manager->libraries[i];
        }
    }

    return NULL; // Not found
}

void material_manager_print(const MaterialManager *manager)
{
    if (manager == NULL)
    {
        printf("Null MaterialManager.\n");
        return;
    }

    printf("Material Manager: %zu material libraries loaded.\n\n", manager->count);
    for (size_t i = 0; i < manager->count; i++)
    {
        printf("Material Library %zu:\n", i + 1);
        material_library_print(&manager->libraries[i]);
    }
}

void material_manager_free(MaterialManager *manager)
{
    if (manager == NULL || manager->libraries == NULL)
        return;

    for (size_t i = 0; i < manager->count; i++)
    {
        material_library_free(&manager->libraries[i]);
    }
    free(manager->libraries);
    manager->libraries = NULL;
    manager->count = 0;
}
