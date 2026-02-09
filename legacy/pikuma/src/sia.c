#include "sia.h"

#include <stdlib.h>

SIA *sia_new(int length)
{
    SIA *sia = (SIA *)malloc(sizeof(SIA));
    sia->length = length;
    sia->data = (int *)malloc(sizeof(int) * length);
    return sia;
}
void sia_free(SIA *sia)
{
    free(sia->data);
    free(sia);
}
