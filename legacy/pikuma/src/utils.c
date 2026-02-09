#include "utils.h"

#include "primitives.h"
#include "globals.h"
#include "math.h"

float rand_range(float min, float max)
{
    // Generate a random float between 0 and 1
    float scale = rand() / (float)RAND_MAX;
    // Scale and shift it to the desired range
    return min + scale * (max - min);
}

float rand_range_int(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

int imin(int a, int b) { return (a < b) ? a : b; }
int imax(int a, int b) { return (a > b) ? a : b; }

// Helper function to convert RGB to HSV
void rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *v)
{
    float r_prime = r / 255.0f;
    float g_prime = g / 255.0f;
    float b_prime = b / 255.0f;

    float cmax = fmaxf(r_prime, fmaxf(g_prime, b_prime));
    float cmin = fminf(r_prime, fminf(g_prime, b_prime));
    float diff = cmax - cmin;

    *v = cmax;

    if (cmax == 0.0f)
    {
        *s = 0.0f;
    }
    else
    {
        *s = diff / cmax;
    }

    if (diff == 0.0f)
    {
        *h = 0.0f;
    }
    else if (cmax == r_prime)
    {
        *h = 60.0f * fmodf((g_prime - b_prime) / diff, 6.0f);
    }
    else if (cmax == g_prime)
    {
        *h = 60.0f * ((b_prime - r_prime) / diff + 2.0f);
    }
    else
    {
        *h = 60.0f * ((r_prime - g_prime) / diff + 4.0f);
    }

    if (*h < 0.0f)
    {
        *h += 360.0f;
    }
}

// Helper function to convert HSV to RGB
void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2.0f) - 1));
    float m = v - c;

    float r_prime, g_prime, b_prime;

    if (h < 60.0f)
    {
        r_prime = c;
        g_prime = x;
        b_prime = 0;
    }
    else if (h < 120.0f)
    {
        r_prime = x;
        g_prime = c;
        b_prime = 0;
    }
    else if (h < 180.0f)
    {
        r_prime = 0;
        g_prime = c;
        b_prime = x;
    }
    else if (h < 240.0f)
    {
        r_prime = 0;
        g_prime = x;
        b_prime = c;
    }
    else if (h < 300.0f)
    {
        r_prime = x;
        g_prime = 0;
        b_prime = c;
    }
    else
    {
        r_prime = c;
        g_prime = 0;
        b_prime = x;
    }

    *r = (uint8_t)((r_prime + m) * 255.0f);
    *g = (uint8_t)((g_prime + m) * 255.0f);
    *b = (uint8_t)((b_prime + m) * 255.0f);
}

double degrees_to_radians(double degrees)
{
    return degrees * (PI / 180.0);
}
double radians_to_degrees(double radians)
{
    return radians * (180.0 / PI);
}

// Helper function to trim whitespace
char *trim_whitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

// Function to duplicate a string (similar to strdup)
char *duplicate_string(const char *src)
{
    if (src == NULL)
        return NULL;
    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (dest != NULL)
    {
        strcpy(dest, src);
    }
    return dest;
}

float map_range(float value, float in_min, float in_max, float out_min, float out_max)
{
    return out_min + (out_max - out_min) * ((value - in_min) / (in_max - in_min));
}

// Simple hash function to generate pseudo-random float between 0 and 1
float hash(int x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return ((float)x) / 4294967295.0f;
}
