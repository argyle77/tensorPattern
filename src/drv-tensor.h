#ifndef _TENSOR_H
#define _TENSOR_H

#include "kinet.h"

#define TENSOR_WIDTH 27
#define TENSOR_HEIGHT 20

#define TENSOR_BYTES (TENSOR_WIDTH*TENSOR_HEIGHT*3)

int tensor_init(void);
void tensor_send(unsigned char *fb);

extern int tensor_landscape_p;

#endif
