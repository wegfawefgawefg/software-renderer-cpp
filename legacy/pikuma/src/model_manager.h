#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "model.h"

#include <stddef.h>

// the model manager holds all the models
typedef struct
{
    Model *models; // Array of models
    size_t count;  // Number of models loaded
} ModelManager;

ModelManager *model_manager_load_from_directory(const char *directory_path);

// get a model by its filename (ex: peaches_castle.obj), or NULL if not found
Model *model_manager_get_model(const ModelManager *manager, const char *name);
void model_manager_free(ModelManager *manager);
void model_manager_print(const ModelManager *manager);

#endif
