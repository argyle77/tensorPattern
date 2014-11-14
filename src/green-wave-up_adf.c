#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "drv-tensor.h"

int main(void)
{
  int x,y,count;
  //int i;
  unsigned char r,g,b;
  //double f;

  /* this is how you should declare your tensor frame buffer */

  unsigned char fb[TENSOR_BYTES];

  /* call tensor_init() once, before you call tensor_send() */

  tensor_init();

  /* set this flag if you're going to use tensor in landscape mode */

  tensor_landscape_p = 1;

  count = 0;
  while(1) {
    count++;

    /* NOTICE: in landscape mode, x ranges between 0 and TENSOR_HEIGHT.
     * normally, when not in landscape mode, x would range between 0
     * and TENSOR_HEIGHT. similarly, y is constrained by TENSOR_WIDTH
     * instead of TENSOR_HEIGHT. */
      for(y=0;y<TENSOR_WIDTH;y++) {
    for(x=0;x<TENSOR_HEIGHT;x++) {
        // here
	r = 0;
	g = g + 1;
	b = 0;

	fb[y*TENSOR_HEIGHT*3 + x*3 + 0] = r * 0.15;
	fb[y*TENSOR_HEIGHT*3 + x*3 + 1] = g * 0.15;
	fb[y*TENSOR_HEIGHT*3 + x*3 + 2] = b * 0.15;
      }
    }

    /* after you've completed a frame buffer, call tensor_send() with
     * that framebuffer */

    tensor_send(fb);

    usleep(50000);
  }
  return(0);
}
