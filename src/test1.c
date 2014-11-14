// Tensor Pattern
// Joshua Krueger



// Defines
#define MAIN_C_

#define YES 0
#define NO 1
#define PASS 0
#define FAIL -1
#define AUTO 0
#define MANUAL 1

#define FOREVER for(;;)
#define PSIZE_X 10
#define PSIZE_Y 10
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
#include "my_font.h"

#ifdef USE_SDL
  #include "SDL.h"
#endif



// Use 1/3 of tensor.  Since we are in landscape, we switch the height and
// width.
#define TENSOR_WIDTH_EFF (TENSOR_HEIGHT)
#define TENSOR_HEIGHT_EFF (TENSOR_WIDTH / 3)
#define TENSOR_BYTES_EFF (TENSOR_WIDTH_EFF * TENSOR_HEIGHT_EFF * 3)
#define TENSOR_PREVIEW_WIDTH (TENSOR_WIDTH_EFF * PSIZE_X)
#define TENSOR_PREVIEW_HEIGHT (TENSOR_HEIGHT_EFF * PSIZE_Y)



// Typedefs

// 24 bit Pixel Color
typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color_t;

// Scroller directions
typedef enum {
  UP, DOWN, LEFT, RIGHT
} dir_t;

// Pattern modes
//typedef enum {
//  TEXT
//} mode_t;



// Globals
const color_t white = {.r = 255, .g = 255, .b = 255};
const color_t black = {.r = 0, .g = 0, .b = 0};
const color_t red = {.r = 255, .g = 0, .b = 0};
const color_t green = {.r = 0, .g = 255, .b = 0};
const color_t blue = {.r = 0, .g = 0, .b = 255};
const color_t cyan = {.r = 0, .g = 255, .b = 255};
const color_t magenta = {.r = 255, .g = 0, .b = 255};
const color_t yellow = {.r = 255, .g = 255, .b = 0};
const color_t orange = {.r = 255, .g = 127, .b = 0};
const color_t grey = {.r = 127, .g = 127, .b = 127};
const unsigned char masks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
unsigned char fb[TENSOR_BYTES_EFF];  // Tensor frame buffer
double color_multiplier;
volatile int do_an_update = NO;
#ifdef USE_SDL
SDL_Surface *screen;
#endif  



// Protos
void SetPixel(int x, int y, color_t color);
color_t GetPixel(int x, int y);
void ColorAll(color_t color);
void Update(void);
void FadeAll(int dec);
void Scroll (dir_t direction);
void ScrollErase(dir_t direction, color_t eraseColor);
void WriteSlice(char *buffer, color_t font_color, color_t bg_color, dir_t side);
void UpdateTensor(void);
#ifdef USE_SDL
  void UpdatePreview(void);
#endif



// Functions

// Main
int main(int argc, char *argv[]) {
  char textBuffer[1024] = "beep ";
  int tindex = strlen(textBuffer);
  SDL_Event key_event;
  
  
  // Unbuffer stdout...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
  // For toning it down.
  if (argc == 2) {
    color_multiplier = atof(argv[1]);
  } else {
    color_multiplier = 1.0;
  }
  
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
  
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  #endif
  
  
  // Init tensor.
  tensor_init();
  tensor_landscape_p = 1;  // Landscape mode.


  // Pattern
  ColorAll(grey);
  Update();
 
  // Program loop...
  FOREVER {
    
    // Read all the key event queued up and update the buffer.
    while (SDL_PollEvent(&key_event)) {
      switch(key_event.type) {
        case SDL_KEYDOWN:
          if (key_event.key.keysym.unicode < 0x80 && key_event.key.keysym.unicode > 0) {
            switch(key_event.key.keysym.sym) {
              case SDLK_BACKSPACE:
                tindex--;
                if (tindex < 0) {
                  tindex = 0;
                }
                textBuffer[tindex] = 0x00;
                break;
                
              case SDLK_RETURN:
                textBuffer[tindex] = '\n';
                textBuffer[tindex + 1] = 0x00;
                tindex++;
                if (tindex >= (sizeof(textBuffer) - 2)) {
                  tindex--;
                }
                break;
                
              case SDLK_ESCAPE:
                tindex = 0;
                textBuffer[0] = 0x00;
                break;
                
              default:
                if (key_event.key.keysym.mod & KMOD_CTRL) {
                  if (key_event.key.keysym.sym == SDLK_c) {
                    printf("CTRL-c pushed.\n");
                    exit(EXIT_SUCCESS);
                  }
                }
                textBuffer[tindex] = (char)key_event.key.keysym.unicode;
                textBuffer[tindex + 1] = 0x00;
                
                tindex++;
                if (tindex >= (sizeof(textBuffer) - 2)) {
                  tindex--;
                }
                
            }
            printf("Text buffer (%04i): \"%s\"\n\n", tindex, textBuffer);
          }
          break;
        default:
          break;
      }  // End key event type switch / case statement.
    }  // End key event polling loop.
    
    // Scroll what we got.
    Scroll(LEFT);
    WriteSlice(textBuffer, red, black, RIGHT);
    
    Update();
    usleep(30000);
   
  }  // End program loop
              
        
  exit(EXIT_SUCCESS);
}



// Color all the pixels a certain color.
void ColorAll(color_t color) {
  int x,y;
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      SetPixel(x,y,color);
    }
  }
}



// Darken all the pixels by a certain value.
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



// Set a single pixel a particular color.
void SetPixel(int x, int y, color_t color) {
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
  fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
}



// Get the color of a particular pixel.
color_t GetPixel(int x, int y) {
  color_t colorTemp;
  
  colorTemp.r = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0];
  colorTemp.g = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1];
  colorTemp.b = fb[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2];
  
  return colorTemp;
}



// Send out the frame buffer to tensor and/or the display window.
void Update(void) {
  #ifdef USE_SDL
    UpdatePreview();
  #endif
  
  #ifdef USE_TENSOR
    UpdateTensor();
  #endif
  
  usleep(50000);
  return;
}



// Send frame buffer to display window
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
      thisColor.r = (unsigned char)((float) thisColor.r * color_multiplier);
      thisColor.g = (unsigned char)((float) thisColor.g * color_multiplier);
      thisColor.b = (unsigned char)((float) thisColor.b * color_multiplier);
      
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



// Scroller buffer manipulation
void Scroll (dir_t direction) {
  int x, y;
  // 
  switch(direction) {
    case UP:
      for (y = 0; y < (TENSOR_HEIGHT_EFF - 1); y++) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixel(x, y, GetPixel(x, y+1));
        }
      }
      break;
      
    case DOWN:
      for (y = (TENSOR_HEIGHT_EFF - 1); y > 0; y--) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixel(x, y, GetPixel(x, y - 1));
        }
      }
      break;
      
    case LEFT:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = 0; x < (TENSOR_WIDTH_EFF - 1); x++) {
          SetPixel(x, y, GetPixel(x + 1, y));
        }
      }
      break;
      
    case RIGHT:
    default:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = (TENSOR_WIDTH_EFF - 1); x > 0; x--) {
          SetPixel(x, y, GetPixel(x - 1, y));
        }
      }
      break;
  }
  
  return;
}



// Erase while backfilling with a color.
void ScrollErase(dir_t direction, color_t eraseColor) {
  int i;
  
  Scroll(direction);
  
  switch (direction) {
    case UP:
      for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
        SetPixel(i, TENSOR_HEIGHT_EFF - 1, eraseColor);
      }
      break;
      
    case DOWN:
      for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
        SetPixel(i, 0, eraseColor);
      }
      break;
      
    case LEFT:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
        SetPixel(TENSOR_WIDTH_EFF - 1, i, eraseColor);
      }
      break;
      
    case RIGHT:
    default:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
        SetPixel(0, i, eraseColor);
      }
      break;
  }
  
  return;
}



void UpdateTensor(void) {
  int i;
  unsigned char fba[TENSOR_BYTES_EFF];
  
  for (i = 0; i < TENSOR_BYTES_EFF; i++) {
    fba[i] = (unsigned char)((double) fb[i] * color_multiplier);
  }
  
  tensor_send(fba);
  return;
}



void WriteSlice(char *buffer, color_t font_color, color_t bg_color, dir_t side) {
  static int bufferIndex = 0;
  static int charCol = 0;
  int actualCol;
  static int charRand = 0;
  unsigned char character;
  int stringLen = strlen(buffer);
  int charIndex;
  int i;
  int tcol;
  
  // Start over if not enough data.
  if (bufferIndex >= stringLen) {
    bufferIndex = 0;
    charCol = 0;
  } else if (bufferIndex < 0) {
    bufferIndex = stringLen - 1;
    charCol = 0;
  }
  
  character = buffer[bufferIndex];
  //printf("%c ", character);
  
  if (side == LEFT) {
    tcol = 0;
  } else {
    tcol = TENSOR_WIDTH_EFF - 1;
  }
  
  if (charRand == 1) {
    SetPixel(tcol, 0, bg_color);
  } else {
    SetPixel(tcol, TENSOR_HEIGHT_EFF - 1, bg_color);
  }
  
  for (i = 0; i < 8; i++) {
    charIndex = (i * 128) + character;
    if (side == LEFT) {
      actualCol = charCol;
    } else {
      actualCol = 7 - charCol;
    }
    
    if ((myfont[charIndex] & masks[actualCol]) != 0) {
      SetPixel(tcol, i + charRand, font_color);
    } else {
      SetPixel(tcol, i + charRand, bg_color);
    }
  }
    
  charCol = charCol + 1;
  if (charCol > 7) {
    if (side == RIGHT) {
      bufferIndex++;
    } else {
      bufferIndex--;
    }
    
    charCol = 0;
    charRand = rand() % 2;
  }
  
  return;
}
