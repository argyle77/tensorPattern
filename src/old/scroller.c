// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "drv-tensor.h"
#include <SDL.h>

typedef enum {
  UP, DOWN, LEFT, RIGHT
} dir_t;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color_t;

#define PXSIZE 10
#define PYSIZE 3

void DrawPix(int x, int y, color_t mycolor);
void scroll(dir_t direction);
void DrawTensor(int x, int y, color_t color);
unsigned char fb[TENSOR_BYTES];  // Declare tensor frame buffer
color_t GetColor(int x, int y);
SDL_Surface *screen;

// Main
int main(void) {
  
  // Vars
  int x,y;
  int sdl_init_okay;
  dir_t direction = UP;
  color_t color = {0, 0, 0};
  int i;
  
  
  // Init tensor
  tensor_init();
  tensor_landscape_p = 1;  // Landscape mode flag


  // Init SDL
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    sdl_init_okay = 0;
  } else {
    sdl_init_okay = 1;
    atexit(SDL_Quit);
  }

  screen = SDL_SetVideoMode(TENSOR_WIDTH * PXSIZE, TENSOR_HEIGHT * PYSIZE, 32, SDL_SWSURFACE);
  if ( screen == NULL ) {
    fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
    exit(1);
  }



  // Program loop
  
  // Generate a color for each pixel.
  for(y=0;y<TENSOR_HEIGHT;y++) {
    for(x=0;x<TENSOR_WIDTH;x++) {
      
      // Determine the color for this pixel.
      color.r = color.r + 1;
      color.g = color.g + 1;
      color.b = color.b - 1;
      
      // Write the pixel into the approp place in the frame buffer, write to window
      DrawTensor(x, y, color);
      
      DrawPix(x, y, GetColor(x, y));
    }
  }


  // Scroll up
  for (i = 0; i < 10; i++) {
    scroll(RIGHT);
  }
  
  // Send out the frame buffer.
  // tensor_send(fb);

  // Delay
  // usleep(50000);
  sleep(2);
  exit(EXIT_SUCCESS);
}


// Scroller buffer manipulation
void scroll (dir_t direction) {
  int x, y;
  // 
  switch(direction) {
    case UP:
      for (y = 0; y < (TENSOR_HEIGHT - 1); y++) {
        for (x = 0; x < TENSOR_WIDTH; x++) {
          DrawTensor(x, y, GetColor(x, y+1));
          DrawPix(x, y, GetColor(x, y));
        }
      }
      break;
      
    case DOWN:
      for (y = (TENSOR_HEIGHT - 1); y > 0; y--) {
        for (x = 0; x < TENSOR_WIDTH; x++) {
          DrawTensor(x, y, GetColor(x, y-1));
          DrawPix(x, y, GetColor(x, y));
        }
      }
      break;
      
    case LEFT:
      for (y = 0; y < TENSOR_HEIGHT; y++) {
        for (x = 0; x < (TENSOR_WIDTH - 1); x++) {
          DrawTensor(x, y, GetColor(x+1, y));
          DrawPix(x, y, GetColor(x, y));
        }
      }
      break;
      
    case RIGHT:
      for (y = 0; y < TENSOR_HEIGHT; y++) {
        for (x = (TENSOR_WIDTH - 1); x > 0; x--) {
          DrawTensor(x, y, GetColor(x-1, y));
          DrawPix(x, y, GetColor(x, y));
        }
      }
      break;
      
    default:
      return;
  }
}



void DrawPix(int x, int y, color_t mycolor) {
  Uint32 color = SDL_MapRGB(screen->format, mycolor.r, mycolor.g, mycolor.b);
  Uint32 *bufp;
  int i, j;
  
  for (i = 0; i < PYSIZE; i++) {
    for (j = 0; j < PXSIZE; j++) {
      bufp = (Uint32 *)screen->pixels + ((y * PYSIZE) + i)*screen->pitch/4 + ((x * PXSIZE) + j);
      *bufp = color;
    }
  }

  SDL_Flip(screen);
}



void DrawTensor(int x, int y, color_t color) {
  fb[x*TENSOR_HEIGHT*3 + y*3 + 0] = color.r;
  fb[x*TENSOR_HEIGHT*3 + y*3 + 1] = color.g;
  fb[x*TENSOR_HEIGHT*3 + y*3 + 2] = color.b;
}



color_t GetColor(int x, int y) {
  color_t colorTemp;
  
  colorTemp.r = fb[x*TENSOR_HEIGHT*3 + y*3 + 0];
  colorTemp.g = fb[x*TENSOR_HEIGHT*3 + y*3 + 1];
  colorTemp.b = fb[x*TENSOR_HEIGHT*3 + y*3 + 2];
  return(colorTemp);
}
