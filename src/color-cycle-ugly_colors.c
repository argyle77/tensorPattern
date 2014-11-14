#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "drv-tensor.h"

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color_t;

void blit(int x, int y, unsigned char *fb, color_t color);
void reset(unsigned char *fb, color_t color);


int main(void)
{
  int i,count;
  // color_t black = {0, 0, 0};
  // color_t white = {255, 255, 255};
  color_t red = {255, 0, 0};
  color_t green = {0,255,0};
  color_t blue = {0,0,255};
  color_t purple = {255, 0, 255};
  color_t cyan = {0, 255,255};
  color_t yellow = {255, 255, 0};
  color_t orange = {255,127,0};

  color_t array[7] = { red, orange, yellow, green, cyan, blue, purple};

  // unsigned char r,g,b;
  // double f;

  /* this is how you should declare your tensor frame buffer */

  //unsigned char fb[TENSOR_BYTES];
  unsigned char fb[TENSOR_WIDTH][TENSOR_HEIGHT][3];
  unsigned char *fb2 = &fb[0][0][0];

  /* call tensor_init() once, before you call tensor_send() */

  tensor_init();

  /* set this flag if you're going to use tensor in landscape mode */

  tensor_landscape_p = 1;


  printf("Height: %i, Width: %i, Bytes: %i\n", TENSOR_HEIGHT, TENSOR_WIDTH, TENSOR_BYTES);

  printf("H x W: %i, /3: %i\n", TENSOR_HEIGHT * TENSOR_WIDTH, (TENSOR_HEIGHT * TENSOR_WIDTH) / 3);
  count = 0;
  while(1) {
    count ++;

    /* NOTICE: in landscape mode, x ranges between 0 and TENSOR_HEIGHT.
     * normally, when not in landscape mode, x would range between 0
     * and TENSOR_HEIGHT. similarly, y is constrained by TENSOR_WIDTH
     * instead of TENSOR_HEIGHT. */
    
    for(i=0; i<7; i++) {
      reset(fb2, array[i]);
      tensor_send(fb2);
      usleep(500000);
     }

    /* after you've completed a frame buffer, call tensor_send() with
     * that framebuffer */

  }
  return(0);
}


void blit(int x, int y, unsigned char *fb, color_t color) {
}

void reset(unsigned char *fb, color_t color) {
  int i;
  for (i = 0; i < (20 * 9 * 3) ; i+=3) {
    fb[i+0] = color.r;
    fb[i+1] = color.g;
    fb[i+2] = color.b;
  }
}
