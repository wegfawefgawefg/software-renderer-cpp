#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "primitives.h"
#include "texture.h"

typedef struct
{
    bool quit;
    uint32_t frame_count;

    Vec3 camera_pos;
    Vec3 camera_target;
    Vec3 camera_up;

    // current color
    uint32_t color;
    float hue;        // Hue component (0° to 360°)
    float saturation; // Saturation component (0.0 to 1.0)
    float brightness; // Brightness (Value) component (0.0 to 1.0)

    Vec2 pointer_pos;

} State;

State *new_state(void);
void free_state(State *state);

#endif // STATE_H