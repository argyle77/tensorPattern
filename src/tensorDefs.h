/*
 * tensorDefs.h
 *
 *  Created on: Jul 17, 2010
 *      Author: argyle
 */

#ifndef TENSORDEFS_H_
#define TENSORDEFS_H_

// Includes
#include "drv-tensor.h"

// Defs
#define MOMENT_COUNT 10
#define TEXT_BUFFER_SIZE 4096

#define YES 0
#define NO 1
#define OFF 0
#define ON 1
#define PASS 0
#define FAIL -1
#define AUTO 0
#define MANUAL 1
#define FOREGROUND 0
#define BACKGROUND 1
#define FOREVER for(;;)
#define PI 3.14159265
#define RANDOM_LIMIT 1000

// Types

// 24 bit Pixel Color
typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} color_t;

// Pattern modes
typedef struct {
  int randDots;
} modes_t;

typedef enum {RED_E, ORANGE_E, YELLOW_E, GREEN_E, CYAN_E, BLUE_E, MAGENTA_E,
              WHITE_E, LIGHT_GRAY_E, GRAY_E, DARK_GREY_E, BLACK_E} color_e;

// Parameters
typedef struct {
  color_e foreground;
  unsigned int randMod;
} parms_t;

typedef struct {
  char textBuffer[TEXT_BUFFER_SIZE];
  int tindex;
  int imaginaryIndex;
} text_info_t;

typedef struct {
  modes_t mode;
  parms_t coefs;
  text_info_t text;
  unsigned char fb[TENSOR_BYTES];  // Tensor frame buffer
} moment_t;

// Some colors
#define PALETTE_COUNT 12
#define COLOR_WHITE {.r = 255, .g = 255, .b = 255, .a = 255}
#define COLOR_LIGHT_GRAY {.r = 191, .g = 191, .b = 191, .a = 255}
#define COLOR_GRAY {.r = 127, .g = 127, .b = 127, .a = 255}
#define COLOR_DARK_GRAY {.r = 63, .g = 63, .b = 63, .a = 255}
#define COLOR_BLACK {.r = 0, .g = 0, .b = 0, .a = 255}
#define COLOR_RED {.r = 255, .g = 0, .b = 0, .a = 255}
#define COLOR_ORANGE {.r = 255, .g = 127, .b = 0, .a = 255}
#define COLOR_YELLOW {.r = 255, .g = 255, .b = 0, .a = 255}
#define COLOR_GREEN {.r = 0, .g = 255, .b = 0, .a = 255}
#define COLOR_CYAN {.r = 0, .g = 255, .b = 255, .a = 255}
#define COLOR_BLUE {.r = 0, .g = 0, .b = 255, .a = 255}
#define COLOR_MAGENTA {.r = 255, .g = 0, .b = 255, .a = 255}
#define OPAQUE 255

// Globals
extern int TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF;
extern const color_t red, orange, yellow, green, cyan, blue, magenta, white,
                     light_gray, gray, dark_gray, black;
extern moment_t moments[MOMENT_COUNT];
extern int now;
extern moment_t *currentMoment;
extern modes_t *currentMode;
extern parms_t *currentParms;
extern text_info_t *currentText;
extern unsigned char *currentFB;

#endif /* TENSORDEFS_H_ */
