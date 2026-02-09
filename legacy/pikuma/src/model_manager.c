#include "model_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

// Helper function to check if a file has a .obj extension (case-insensitive)
static int has_obj_extension(const char *filename)
{
    if (!filename)
        return 0;
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return 0;
    return (strcasecmp(dot, ".obj") == 0); // Case-insensitive comparison
}

ModelManager *model_manager_load_from_directory(const char *directory_path)
{
    if (!directory_path)
    {
        fprintf(stderr, "Invalid arguments to model_manager_load_from_directory.\n");
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

    // Allocate memory for the ModelManager
    ModelManager *manager = (ModelManager *)malloc(sizeof(ModelManager));
    if (!manager)
    {
        fprintf(stderr, "Failed to allocate memory for ModelManager.\n");
        closedir(dir);
        return NULL;
    }
    manager->count = 0;
    manager->models = NULL;

    // Count the number of .obj files in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_obj_extension(filename))
        {
            manager->count += 1;
        }
    }
    rewinddir(dir);

    // Allocate memory for the Model array
    manager->models = (Model *)malloc(manager->count * sizeof(Model));
    if (!manager->models)
    {
        fprintf(stderr, "Failed to allocate memory for Model array.\n");
        closedir(dir);
        free(manager);
        return NULL;
    }

    int current_model_index = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_obj_extension(filename))
        {
            // Construct the full path for loading the model
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, filename);

            // Load the model using model_load_from_file
            Model *loaded_model = model_load_from_file(full_path);
            if (!loaded_model)
            {
                fprintf(stderr, "Failed to load model from %s.\n", full_path);
                // Continue loading other models despite failure
                continue;
            }

            // Assign to the next available entry in the array
            manager->models[current_model_index] = *loaded_model;
            current_model_index += 1;
        }
    }

    closedir(dir);
    return manager;
}

// get a model by its filename (ex: peaches_castle.obj), or NULL if not found
Model *model_manager_get_model(const ModelManager *manager, const char *name)
{
    for (size_t i = 0; i < manager->count; i++)
    {
        if (strcmp(manager->models[i].name, name) == 0)
        {
            return &manager->models[i];
        }
    }
    return NULL;
}

void model_manager_free(ModelManager *manager)
{
    if (!manager)
    {
        printf("model_manager_free: manager is NULL\n");
        return;
    }

    for (size_t i = 0; i < manager->count; i++)
    {
        model_free(&manager->models[i]);
    }
    free(manager->models);
    manager->models = NULL;
    manager->count = 0;

    free(manager);
}

void model_manager_print(const ModelManager *manager)
{
    printf("Model Manager:\n");
    printf("Model Count: %zu\n", manager->count);
    for (size_t i = 0; i < manager->count; i++)
    {
        model_print(&manager->models[i]);
    }
}
