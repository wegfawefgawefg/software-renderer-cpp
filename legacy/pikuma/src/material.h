#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"

typedef struct
{
    char *name;             // Material name
    float shininess;        // Shininess (Ns)
    Vec3 ambient_color;     // Ambient color (Ka)
    Vec3 diffuse_color;     // Diffuse color (Kd)
    Vec3 specular_color;    // Specular color (Ks)
    Vec3 emissive_color;    // Emissive color (Ke)
    float optical_density;  // Optical density (Ni)
    float transparency;     // Transparency (d)
    int illumination_model; // Illumination model (illum)
    char *diffuse_map;      // Diffuse texture map (map_Kd)
} Material;

void material_print(const Material *mat);

#endif
