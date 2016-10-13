#ifndef _DRVTENSOR_H
#define _DRVTENSOR_H

#include "kinet.h"

int tensor_init(const char **Tensor_Section1, const char **Tensor_Section2, const char **Tensor_Section3);
void tensor_send(unsigned char *fb);

extern int tensor_landscape_p;

extern const char *Tensor_Section1_def[];
extern const char *Tensor_Section2_def[];
extern const char *Tensor_Section3_def[];


#endif
