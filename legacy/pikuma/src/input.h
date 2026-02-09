#ifndef INPUT_H
#define INPUT_H

#include "state.h"
#include "primitives.h"

typedef struct
{
    Uint32 buttons;
    IVec2 pos;
} MouseState;

IVec2 get_mouse_pos(void);
IVec2 get_mouse_pos_in_gba_window(void);
bool is_left_mouse_button_down(MouseState *mouseState);
bool is_right_mouse_button_down(MouseState *mouseState);
Uint32 get_mouse_buttons(void);
void process_input(State *state);

#endif