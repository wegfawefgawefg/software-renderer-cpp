#ifndef PROJECTION_H
#define PROJECTION_H

#include "sfa.h"

void map_to_screen(const SFA *transformed_sfa, SFA *screen_sfa, int screen_width, int screen_height);
void map_to_screen_keep_z(const SFA *transformed_sfa, SFA *screen_sfa, int screen_width, int screen_height);
void perspective_divide(SFA *transformed_sfa);

#endif