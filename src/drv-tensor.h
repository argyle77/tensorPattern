#ifndef _TENSOR_H
#define _TENSOR_H

#include "kinet.h"

#define TENSOR_WIDTH 27
#define TENSOR_HEIGHT 20

#define TENSOR_BYTES (TENSOR_WIDTH*TENSOR_HEIGHT*3)

int tensor_init(const char **Tensor_Section1, const char **Tensor_Section2, const char **Tensor_Section3);
void tensor_send(unsigned char *fb);

extern int tensor_landscape_p;

extern const char *Tensor_Section1_def[];
extern const char *Tensor_Section2_def[];
extern const char *Tensor_Section3_def[];


#endif
