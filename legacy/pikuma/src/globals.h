#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>

#define RENDER_SCALE 3.0

#define RENDER_WIDTH 240 * RENDER_SCALE
#define RENDER_HEIGHT 160 * RENDER_SCALE

#define WINDOW_SCALE 3
#define WINDOW_WIDTH 400 * WINDOW_SCALE
#define WINDOW_HEIGHT 240 * WINDOW_SCALE

#define FRAME_LIMITING true
#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

#define SHOW_FPS true

#define AMBIENT_LIGHT 0.4f

extern int WIDTH;
extern int HEIGHT;

#endif
