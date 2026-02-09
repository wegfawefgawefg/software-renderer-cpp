#include "shape.h"

#include <stdlib.h>
#include <stdio.h>

Shape *shape_new(void)
{
    Shape *shape = (Shape *)malloc(sizeof(Shape));
    if (!shape)
    {
        fprintf(stderr, "Failed to allocate memory for Shape.\n");
        return NULL;
    }
    shape->name = NULL;
    shape->material_name = NULL;
    shape->vertex_indices = NULL;
    shape->normal_indices = NULL;
    shape->texcoord_indices = NULL;
    return shape;
}

void shape_free(Shape *shape)
{
    if (shape)
    {
        if (shape->name)
        {
            free(shape->name);
        }
        if (shape->material_name)
        {
            free(shape->material_name);
        }
        if (shape->vertex_indices)
        {
            su32a_free(shape->vertex_indices);
        }
        if (shape->normal_indices)
        {
            su32a_free(shape->normal_indices);
        }
        if (shape->texcoord_indices)
        {
            su32a_free(shape->texcoord_indices);
        }
        free(shape);
    }
}
