#define _DEFAULT_SOURCE // Ensure feature test macros are set

#include "texture_management.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // For directory traversal
#include <sys/stat.h> // For file information
#include <errno.h>    // For error handling

#include <gif_lib.h>

#include "texture.h"
#include "utils.h"

// Helper function to check if a file has a .png extension (case-insensitive)
static int has_png_extension(const char *filename)
{
    if (!filename)
        return 0;
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return 0;
    return (strcasecmp(dot, ".png") == 0); // Case-insensitive comparison
}

// Free all memory associated with a TextureManager structure
void texture_manager_free(TextureManager *textures)
{
    if (!textures)
        return;

    for (int i = 0; i < textures->num_entries; i++)
    {
        // Free the path and filename strings
        if (textures->entries[i].path)
            free(textures->entries[i].path);
        if (textures->entries[i].filename)
            free(textures->entries[i].filename);

        // Free the Texture
        if (textures->entries[i].texture)
            texture_free(textures->entries[i].texture);
    }

    // Free the entries array
    free(textures->entries);

    // Free the TextureManager struct
    free(textures);
}

// Load all .png textures from a specified directory into the TextureManager array
TextureManager *texture_manager_load_from_directory(const char *directory_path)
{
    if (!directory_path)
    {
        fprintf(stderr, "Invalid arguments to textures_load_from_directory.\n");
        return NULL;
    }

    // do your initial allocation
    TextureManager *texture_manager = (TextureManager *)malloc(sizeof(TextureManager));
    if (!texture_manager)
    {
        fprintf(stderr, "Failed to allocate memory for TextureManager.\n");
        return NULL;
    }
    texture_manager->num_entries = 0;
    texture_manager->entries = NULL;

    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory_path);
    if (!dir)
    {
        perror("opendir");
        fprintf(stderr, "Failed to open directory: %s\n", directory_path);
        return NULL;
    }

    // go through the filenames and determine how many textures we need to allocate for
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_png_extension(filename))
        {
            texture_manager->num_entries += 1;
        }
    }
    texture_manager->entries = (TextureManagerEntry *)malloc(texture_manager->num_entries * sizeof(TextureManagerEntry));
    rewinddir(dir);

    int current_entry_index = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories
        if (entry->d_type == DT_DIR)
            continue;

        const char *filename = entry->d_name;

        if (has_png_extension(filename))
        {

            // Allocate and duplicate the path and filename strings
            char *path_dup = strdup(directory_path);
            if (!path_dup)
            {
                fprintf(stderr, "Failed to duplicate directory path string.\n");
                closedir(dir);
                return NULL;
            }

            char *filename_dup = strdup(filename);
            if (!filename_dup)
            {
                fprintf(stderr, "Failed to duplicate filename string.\n");
                free(path_dup);
                closedir(dir);
                return NULL;
            }

            // Construct the full path for loading the texture
            size_t path_len = strlen(path_dup);
            bool needs_slash = (path_dup[path_len - 1] != '/');
            size_t full_path_length = path_len + (needs_slash ? 1 : 0) + strlen(filename_dup) + 1; // '/' + filename + '\0'
            char *full_path = (char *)malloc(full_path_length);
            if (!full_path)
            {
                fprintf(stderr, "Failed to allocate memory for full path.\n");
                free(path_dup);
                free(filename_dup);
                closedir(dir);
                return NULL;
            }

            // Concatenate path and filename with '/' if necessary
            if (needs_slash)
            {
                snprintf(full_path, full_path_length, "%s/%s", path_dup, filename_dup);
            }
            else
            {
                snprintf(full_path, full_path_length, "%s%s", path_dup, filename_dup);
            }

            // Load the texture using texture_load_from_png
            Texture *loaded_texture = texture_load_from_png(full_path);
            if (!loaded_texture)
            {
                fprintf(stderr, "Failed to load texture from %s.\n", full_path);
                free(full_path);
                free(path_dup);
                free(filename_dup);
                // Continue loading other textures despite failure
                continue;
            }
            free(full_path);

            // Assign to the next available entry in the array
            texture_manager->entries[current_entry_index].path = path_dup;
            texture_manager->entries[current_entry_index].filename = filename_dup;
            texture_manager->entries[current_entry_index].texture = loaded_texture;

            current_entry_index += 1;
        }
    }

    closedir(dir);
    return texture_manager;
}

// Retrieve a Texture by its filename
Texture *texture_manager_get(TextureManager *textures, const char *filename)
{
    if (!textures || !filename)
        return NULL;

    for (int i = 0; i < textures->num_entries; i++)
    {
        if (strcmp(textures->entries[i].filename, filename) == 0)
        {
            return textures->entries[i].texture;
        }
    }

    // If not found, return NULL
    return NULL;
}

// Print details of all loaded textures
void texture_manager_print(TextureManager *textures)
{
    if (!textures)
    {
        printf("TextureManager is NULL.\n");
        return;
    }

    printf("Total TextureManager: %d\n", textures->num_entries);
    printf("TextureManager List:\n");
    printf("----------------------------\n");

    for (int i = 0; i < textures->num_entries; i++)
    {
        printf("Texture %d:\n", i + 1);
        printf("  Path: %s\n", textures->entries[i].path ? textures->entries[i].path : "Unknown Path");
        printf("  Filename: %s\n", textures->entries[i].filename ? textures->entries[i].filename : "Unknown Filename");
        if (textures->entries[i].texture)
        {
            printf("  Dimensions: %dx%d\n", textures->entries[i].texture->width, textures->entries[i].texture->height);
        }
        else
        {
            printf("  Dimensions: Unknown\n");
        }
        printf("----------------------------\n");
    }
}
