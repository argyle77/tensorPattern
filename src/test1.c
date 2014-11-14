// Tensor Pattern


// Defines
#define YES 0
#define NO 1
#define USE_SDL
#define PSIZE_X 40
#define PSIZE_Y 40
#define BORDER 10
#define USE_SDL
#define USE_TENSOR
//#undef USE_SDL
//#undef USE_TENSOR

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "drv-tensor.h"
#ifdef USE_SDL
  #include "SDL.h"
#endif

// Use 1/3
#define TENSOR_WIDTH_EFF (TENSOR_HEIGHT)
#define TENSOR_HEIGHT_EFF (TENSOR_WIDTH / 3)
#define TENSOR_BYTES_EFF (TENSOR_WIDTH_EFF * TENSOR_HEIGHT_EFF * 3)
#define TENSOR_PREVIEW_WIDTH (TENSOR_WIDTH_EFF * PSIZE_X)
#define TENSOR_PREVIEW_HEIGHT (TENSOR_HEIGHT_EFF * PSIZE_Y)

// Typedefs
typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color_t;

typedef enum {
  UP, DOWN, LEFT, RIGHT
} dir_t;


// Globals
unsigned char fb[TENSOR_BYTES_EFF];  // Tensor frame buffer
const color_t white = {.r = 255, .g = 255, .b = 255};
const color_t black = {.r = 0, .g = 0, .b = 0};
#ifdef USE_SDL
SDL_Surface *screen;
#endif  


// Protos
void SetPixel(int x, int y, color_t color);
color_t GetPixel(int x, int y);
void ColorAll(color_t color);
void Update(void);
void FadeAll(int dec);
#ifdef USE_SDL
  void UpdatePreview(void);
#endif



// Functions
int main(void) {
  int x,y,count;
  color_t pixel_color;
  color_t oldColor;
  
  // Init the display window.
  #ifdef USE_SDL
  if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  } else {
    atexit(SDL_Quit);
  }
  
  screen = SDL_SetVideoMode(TENSOR_PREVIEW_WIDTH + (BORDER * 2), TENSOR_PREVIEW_HEIGHT + (BORDER * 2), 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video size: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  #endif
  
  // Init tensor.
  tensor_init();
  tensor_landscape_p = 1;  // Landscape mode.


  // Pattern loop.
  ColorAll(white);
  Update();
  
  int i;
  for (i = 0; i < 255; i+=4) {
    FadeAll(4);
    Update();
  }
  
  count = 0;
  while(count < 200) {
    count++;

    // Give each pixel a color value.
    for(x = 0 ; x < TENSOR_WIDTH_EFF ; x++) {
      for(y = 0 ; y < TENSOR_HEIGHT_EFF ; y++) {
        oldColor = GetPixel(x,y);
        
	      pixel_color.r = ((x * y) - count) + (oldColor.r / 2);
	      pixel_color.g = oldColor.g + pixel_color.r;
	      pixel_color.b = 0;

        SetPixel(x, y, pixel_color);
        }
      }

    Update();
  }
  
  
  for (i = 0; i < 255; i+=4) {
    FadeAll(4);
    Update();
  }
  
  exit(EXIT_SUCCESS);
}



void ColorAll(color_t color) {
  int x,y;
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      SetPixel(x,y,color);
    }
  }
}

void FadeAll(int dec) {
  int x,y;
  color_t oldColor;
    
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      oldColor = GetPixel(x,y);
      if (oldColor.r < dec) {
        oldColor.r = 0;
      } else {
        oldColor.r -= dec;
      }
      
      if (oldColor.g < dec) {
        oldColor.g = 0;
      } else {
        oldColor.g -= dec;
      }
      
      if (oldColor.b < dec) {
        oldColor.b = 0;
      } else {
        oldColor.b -= dec;
      }
      
      SetPixel(x,y,oldColor);
    }
  }
  
  return;
}

void SetPixel(int x, int y, color_t color) {
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
}



color_t GetPixel(int x, int y) {
  color_t colorTemp;
  
  colorTemp.r = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0];
  colorTemp.g = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1];
  colorTemp.b = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2];
  
  return colorTemp;
}


void Update(void) {
  #ifdef USE_SDL
    UpdatePreview();
  #endif
  
  #ifdef USE_TENSOR
    tensor_send(fb);
  #endif
  
  usleep(50000);
  return;
}



#ifdef USE_SDL
void UpdatePreview(void) {
  Uint32 color_sdl;
  Uint32 *bufp;
  color_t thisColor;
  int x,y;
  int i,j;
  
  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0) {
      return;
    }
  }
  
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      thisColor = GetPixel(x,y);
      color_sdl = SDL_MapRGB(screen->format, (Uint8) thisColor.r, (Uint8) thisColor.g, (Uint8) thisColor.b);
      for (i = 0; i < PSIZE_X; i++) {
        for (j = 0; j < PSIZE_Y; j++) {
          bufp = (Uint32 *) screen->pixels + ( (((y * PSIZE_Y) + j + BORDER) * screen->pitch / 4) + ((x * PSIZE_X) + i + BORDER));
          *bufp = color_sdl;
        }
      }
    }
  }
  
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
  
  SDL_Flip(screen);
}
#endif
