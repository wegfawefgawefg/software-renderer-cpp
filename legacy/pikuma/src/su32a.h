#ifndef SU32A_H
#define SU32A_H

#include <stdint.h>

// SU32A: Sized U32 Array
typedef struct
{
    int length;
    uint32_t *data;
} SU32A;

SU32A *su32a_new(int length);
void su32a_free(SU32A *su32a);

void su32a_set(SU32A *su32a, int index, uint32_t value);
uint32_t su32a_get(SU32A *su32a, int index);

#endif