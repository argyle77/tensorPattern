#ifndef _TENSOR_H
#define _TENSOR_H

#include "kinet.h"

//#define PANELS_WIDE 1
#define PANELS_WIDE 3
#define PANELS_HIGH 1
//#define PANEL_WIDTH 151
#define PANEL_WIDTH 9
//#define PANEL_HEIGHT 151
#define PANEL_HEIGHT 20
#define PIXEL_BYTES 3

#define TENSOR_WIDTH (PANELS_WIDE * PANEL_WIDTH)
#define TENSOR_HEIGHT (PANELS_HIGH * PANEL_HEIGHT)

#define TENSOR_BYTES (TENSOR_WIDTH * TENSOR_HEIGHT * PIXEL_BYTES)

#define TENSOR_ADDR_PER_PANEL 6
#define TENSOR_PANEL_COUNT (PANELS_WIDE * PANELS_HIGH)

int tensor_init(const char **Tensor_Section1, const char **Tensor_Section2, const char **Tensor_Section3);
void tensor_send(unsigned char *fb);

extern int tensor_landscape_p;

extern const char *Tensor_Section1_def[];
extern const char *Tensor_Section2_def[];
extern const char *Tensor_Section3_def[];


#endif
