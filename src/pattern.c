/*
 * pattern.c
 *
 *  Created on: Jul 17, 2010
 *      Author: argyle
 */

// Includes
#include "tensorDefs.h"
#include "pattern.h"
#include <stdlib.h> // rand

void PatternEngine(void) {
  // Random dots.  Most useful mode ever.
   if (currentMode->randDots == YES) {
     RandomDots(blue, currentParms->randMod, currentFB);
   }
}

// Color all the pixels a certain color.
void ColorAll(color_t color, unsigned char *fb) {
  int x,y;
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      SetPixel(x, y, color, fb);
    }
  }
}

// Set a single pixel a particular color.
void SetPixel(int x, int y, color_t color, unsigned char *buffer) {
  buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
  buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
  buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
}

// Get the color of a particular pixel.
color_t GetPixel(int x, int y, unsigned char *buffer) {
  color_t colorTemp;
  colorTemp.r = buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0];
  colorTemp.g = buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1];
  colorTemp.b = buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2];
  colorTemp.a = OPAQUE; // We don't return an alpha for a written pixel.
  return colorTemp;
}

void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer) {
  int x,y;
  if (rFreq == 0) {
    return;
  }

  if ((rand() % rFreq) == 0) {
    x = rand() % TENSOR_WIDTH_EFF;
    y = rand() % TENSOR_HEIGHT_EFF;
    SetPixel(x,y,color, buffer);
  }

}
