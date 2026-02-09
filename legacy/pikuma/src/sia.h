#ifndef SIA_H
#define SIA_H

// same for sized int array
// SIA: Sized Int Array
typedef struct
{
    int length;
    int *data;
} SIA;

SIA *sia_new(int length);
void sia_free(SIA *sia);

#endif // SIA_H