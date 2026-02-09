#include "material.h"

#include "utils.h"

void material_print(const Material *mat)
{
       if (mat == NULL)
       {
              printf("Null Material.\n");
              return;
       }

       printf("  Material: %s\n", mat->name ? mat->name : "Unnamed");
       printf("    Shininess (Ns): %.2f\n", mat->shininess);
       printf("    Ambient Color (Ka): %.2f %.2f %.2f\n",
              mat->ambient_color.x, mat->ambient_color.y, mat->ambient_color.z);
       printf("    Diffuse Color (Kd): %.2f %.2f %.2f\n",
              mat->diffuse_color.x, mat->diffuse_color.y, mat->diffuse_color.z);
       printf("    Specular Color (Ks): %.2f %.2f %.2f\n",
              mat->specular_color.x, mat->specular_color.y, mat->specular_color.z);
       printf("    Emissive Color (Ke): %.2f %.2f %.2f\n",
              mat->emissive_color.x, mat->emissive_color.y, mat->emissive_color.z);
       printf("    Optical Density (Ni): %.2f\n", mat->optical_density);
       printf("    Transparency (d): %.2f\n", mat->transparency);
       printf("    Illumination Model (illum): %d\n", mat->illumination_model);
       printf("    Diffuse Map (map_Kd): %s\n", mat->diffuse_map ? mat->diffuse_map : "None");
       printf("\n");
}