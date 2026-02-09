#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL.h>

#include "state.h"
#include "draw_lib.h"
#include "texture.h"
#include "assets.h"
#include "f_texture.h"

void draw(Texture *pb, FTexture *z_buffer, State *state, Assets *assets);
#endif
