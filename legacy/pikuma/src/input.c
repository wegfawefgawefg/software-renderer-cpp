#include <SDL2/SDL.h>

#include "input.h"
#include "globals.h"
#include "vec3.h"

IVec2 get_mouse_pos(void)
{
    int window_x, window_y;
    SDL_GetMouseState(&window_x, &window_y);
    int x, y;
    x = window_x * RENDER_WIDTH / WIDTH;
    y = window_y * RENDER_HEIGHT / HEIGHT;
    return (IVec2){x, y};
}

Uint32 get_mouse_buttons(void)
{
    return SDL_GetMouseState(NULL, NULL);
}

bool is_left_mouse_button_down(MouseState *mouseState)
{
    return mouseState->buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
}

bool is_right_mouse_button_down(MouseState *mouseState)
{
    return mouseState->buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
}
void process_input(State *state)
{
    SDL_Event event;

    // Handle all pending events
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            state->quit = true;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                state->quit = true;
            }
            break;
        // Handle other one-time events here if necessary
        default:
            break;
        }
    }

    // Get the current state of the keyboard
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    const float camera_speed = 20.0f;
    const float pointer_speed = 1.0f; // Adjust as needed

    // Handle camera movement with arrow keys and WASD
    if (keyboard_state[SDL_SCANCODE_A])
    {
        state->camera_pos.x -= camera_speed;
    }
    if (keyboard_state[SDL_SCANCODE_D])
    {
        state->camera_pos.x += camera_speed;
    }
    if (keyboard_state[SDL_SCANCODE_W])
    {
        state->camera_pos.z += camera_speed;
    }
    if (keyboard_state[SDL_SCANCODE_S])
    {
        state->camera_pos.z -= camera_speed;
    }
    // space for up, left shift for down
    if (keyboard_state[SDL_SCANCODE_SPACE])
    {
        state->camera_pos.y += camera_speed;
    }
    // or ctrl
    if (keyboard_state[SDL_SCANCODE_LSHIFT] || keyboard_state[SDL_SCANCODE_LCTRL])
    {
        state->camera_pos.y -= camera_speed;
    }

    // up and down should move towards camera target
    if (keyboard_state[SDL_SCANCODE_UP])
    {
        Vec3 dir = vec3_sub(state->camera_target, state->camera_pos);
        dir = vec3_normalize(dir);
        dir = vec3_fmul(dir, camera_speed);
        state->camera_pos = vec3_add(state->camera_pos, dir);
    }
    if (keyboard_state[SDL_SCANCODE_DOWN])
    {
        Vec3 dir = vec3_sub(state->camera_target, state->camera_pos);
        dir = vec3_normalize(dir);
        dir = vec3_fmul(dir, camera_speed);
        state->camera_pos = vec3_sub(state->camera_pos, dir);
    }

    // Handle camera movement along the z-axis with additional keys if needed
    // For example, you can use Q and E for z-axis movement
    if (keyboard_state[SDL_SCANCODE_Q])
    {
        state->camera_pos.z += camera_speed;
    }
    if (keyboard_state[SDL_SCANCODE_E])
    {
        state->camera_pos.z -= camera_speed;
    }

    // Print camera position for debugging
    printf("camera pos: %f, %f, %f\n", state->camera_pos.x, state->camera_pos.y, state->camera_pos.z);

    // Handle pointer movement separately to avoid conflicts with camera movement
    // For example, use IJKL or other keys for pointer control
    if (keyboard_state[SDL_SCANCODE_J])
    {
        state->pointer_pos.x -= pointer_speed;
    }
    if (keyboard_state[SDL_SCANCODE_L])
    {
        state->pointer_pos.x += pointer_speed;
    }
    if (keyboard_state[SDL_SCANCODE_I])
    {
        state->pointer_pos.y -= pointer_speed;
    }
    if (keyboard_state[SDL_SCANCODE_K])
    {
        state->pointer_pos.y += pointer_speed;
    }

    // Clamp pointer position between 0 and 9
    state->pointer_pos.x = (int)fmax(0, fmin(9, state->pointer_pos.x));
    state->pointer_pos.y = (int)fmax(0, fmin(9, state->pointer_pos.y));
}