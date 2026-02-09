#include "f_texture.h"
#include "texture.h"
#include "colors.h"
#include "utils.h"
#include "draw_lib.h"

void debug_draw_z_buffer(Texture *pb, FTexture *z_buffer)
{
    // get min and max z
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;
    for (int i = 0; i < z_buffer->width * z_buffer->height; i++)
    {
        float z = z_buffer->data[i];
        if (z < min_z)
        {
            min_z = z;
        }
        if (z > max_z)
        {
            max_z = z;
        }
    }

    float mapscale = 4.0f;
    for (int y = 0; y < z_buffer->height; y += (int)mapscale)
    {
        for (int x = 0; x < z_buffer->width; x += (int)mapscale)
        {
            float z = f_texture_get(z_buffer, x, y);
            // z = map_range(z, min_z, max_z, 0.0f, 1.0f);
            // z = 1.0f - z;
            // high pow
            // z = pow(z, 1000.0f);
            z /= 500.0f;
            uint8_t z8 = (uint8_t)(z * 255.0f);
            uint32_t color = color_from_rgb(z8, z8, z8);
            texture_set(pb, x / mapscale, y / mapscale, color);

            // if (z < 500.0f)
            // {
            //     // write out in text the z value on screen
            //     char z_str[10];
            //     sprintf(z_str, "%.1f", z);
            //     blit_string(pb, assets->charmap_white, z_str, x, y, 1, COLOR_WHITE);
            // }
        }
    }
}

//  // debug the face buffer
//     {
//         // get min and max face
//         uint32_t min_face = UINT32_MAX;
//         uint32_t max_face = 0;
//         for (int i = 0; i < face_buffer->width * face_buffer->height; i++)
//         {
//             uint32_t face = face_buffer->pixels[i];
//             if (face < min_face)
//             {
//                 min_face = face;
//             }
//             if (face > max_face)
//             {
//                 max_face = face;
//             }
//         }

//         float mapscale = 4.0f;
//         for (int y = 0; y < face_buffer->height; y += (int)mapscale)
//         {
//             for (int x = 0; x < face_buffer->width; x += (int)mapscale)
//             {
//                 uint32_t face = texture_get(face_buffer, x, y);
//                 face = map_range(face, min_face, max_face, 100, 255);
//                 uint32_t color = color_from_rgb(face, face, face);
//                 texture_set(pb, x / mapscale, y / mapscale, color);
//             }
//         }
//     }
