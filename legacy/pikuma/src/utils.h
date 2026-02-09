#ifndef UTILS_H
#define UTILS_H

#include "primitives.h"

#include <SDL2/SDL.h>

#define M_PI 3.14159265358979323846

#define SWAP(a, b)          \
    do                      \
    {                       \
        typeof(a) temp = a; \
        a = b;              \
        b = temp;           \
    } while (0)
float rand_range(float min, float max);
float rand_range_int(int min, int max);

int imin(int a, int b);
int imax(int a, int b);

void rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *v);
void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

#define PI 3.14159265358979323846

double degrees_to_radians(double degrees);
double radians_to_degrees(double radians);

char *trim_whitespace(char *str);
char *duplicate_string(const char *src);

float map_range(float value, float in_min, float in_max, float out_min, float out_max);

float hash(int x);

#endif