#include "state.h"
#include "utils.h"
#include "globals.h"

State *new_state(void)
{
    State *state = malloc(sizeof(State));
    if (!state)
    {
        return NULL;
    }

    state->quit = false;
    state->frame_count = 0;

    state->camera_pos = vec3_create(0, 0, -100);
    state->camera_target = vec3_create(0, 0, 0);
    state->camera_up = vec3_create(0, 1, 0);

    state->pointer_pos = vec2_create(0, 0);

    state->color = 0xFFFFFFFF;
    state->hue = 0.0f;
    state->saturation = 1.0f;
    state->brightness = 1.0f;

    return state;
}

void free_state(State *state)
{

    free(state);
}