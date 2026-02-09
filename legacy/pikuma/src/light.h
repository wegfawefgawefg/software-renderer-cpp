#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include "vec3.h"
#include "sfa.h"
#include "su32a.h"

typedef struct Light
{
    Vec3 pos;
    uint32_t color;
    float brightness;
} Light;

Light light_new(Vec3 pos, uint32_t color, float brightness);

SU32A *lighting_get_face_colors(
    SFA *vertices,
    SU32A *indices,
    SU32A *colors,
    Light *lights,
    int num_lights);
#endif