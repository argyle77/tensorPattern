// Tensor Pattern
// Joshua Krueger



// Defines
#define MAIN_C_

#define YES 0
#define NO 1
#define OFF 0
#define ON 1
#define PASS 0
#define FAIL -1
#define AUTO 0
#define MANUAL 1

#define FOREVER for(;;)
#define PSIZE_X 10
#define PSIZE_Y 10
#define BORDER 10
#define TEXT_BUFFER_SIZE 4096

#define USE_TENSOR
//#undef USE_TENSOR

#define INITIAL_DIFF_COEF 0.016
#define INITIAL_DIFF_INC 0.001
#define INITIAL_FADEOUT_DEC 4
#define INITIAL_RAND_MOD 4
#define INITIAL_RAINBOW_INC 8


// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "drv-tensor.h"
#include "my_font.h"
#include "SDL.h"



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
  unsigned char a;
} color_t;

typedef struct {
  color_t foreground;
  color_t background;
} colors_t;

// Scroller directions
typedef enum {
  UP, LEFT, DOWN, RIGHT
} dir_t;

// Pattern modes
typedef struct {
  int textScroller;
  int cellAutoFun;
  int bouncer;
  int fadeout;
  int diffuse;
  int roller;
  int scroller;
  int horizontalBars;
  int verticalBars;
  int colorAll;
  int clearAll;
  int randDots;
  int cycleForeground;
  int cycleBackground;
} modes_t;

typedef enum {
  RGB, CMY, RYGCBM, RAINBOW, RANDOM
} colorCycles_e;

// Various parameters
typedef struct {
  int fadeout_dec;
  float diffusion_coef;
  float diffusion_increment;
  dir_t scrollerDir;
  unsigned int randMod;
  colorCycles_e colorCycleMode;
  int rainbowInc;
} mode_parms_t;


// Globals
const color_t white = {.r = 255, .g = 255, .b = 255, .a = 0};
const color_t black = {.r = 0, .g = 0, .b = 0, .a = 0};
const color_t red = {.r = 255, .g = 0, .b = 0, .a = 0};
const color_t green = {.r = 0, .g = 255, .b = 0, .a = 0};
const color_t blue = {.r = 0, .g = 0, .b = 255, .a = 0};
const color_t cyan = {.r = 0, .g = 255, .b = 255, .a = 0};
const color_t magenta = {.r = 255, .g = 0, .b = 255, .a = 0};
const color_t yellow = {.r = 255, .g = 255, .b = 0, .a = 0};
const color_t orange = {.r = 255, .g = 127, .b = 0, .a = 0};
const color_t grey = {.r = 127, .g = 127, .b = 127, .a = 0};
const unsigned char masks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
unsigned char fb[TENSOR_BYTES_EFF];  // Tensor frame buffer
double color_multiplier;
volatile int do_an_update = NO;
SDL_Surface *screen;



// Protos
void SetPixel(int x, int y, color_t color);
color_t GetPixel(int x, int y);
void ColorAll(color_t color);
void Update(void);
void FadeAll(int dec);
void Scroll (dir_t direction, int rollovermode);
void ScrollErase(dir_t direction, color_t eraseColor);
void WriteSlice(char *buffer, colors_t font_colors, dir_t direction, int column);
void UpdateTensor(void);
void UpdatePreview(void);
void Diffuse(float diffusion_coeff);
void HorizontalBars(colors_t colors);
void VerticalBars(colors_t colors);
colors_t ColorAlter(int thiskey, colors_t colors);
modes_t ModeAlter(int thiskey, modes_t mode);
mode_parms_t ParmAlter(int thiskey, mode_parms_t parms);
void RandomDots(color_t color, unsigned int rFreq);
color_t ColorCycle(color_t originalColor, colorCycles_e cycleMode, int rainbowInc, int *cycleSaver);

// Functions

// Main
int main(int argc, char *argv[]) {
  char textBuffer[TEXT_BUFFER_SIZE] = "beep ";
  // int textBufferRand[TEXT_BUFFER_SIZE];
  int tindex = strlen(textBuffer);
  SDL_Event key_event;
  color_t pixelColor, oldColor;
  int x,y;
  dir_t scrDir = UP;
  int saveCycleF = 0;
  int saveCycleB = 0;
  modes_t mode = {
    .textScroller = NO,
    .cellAutoFun = NO,
    .bouncer = NO,
    .fadeout = NO,
    .diffuse = NO,
    .roller = NO,
    .scroller = NO,
    .horizontalBars = NO,
    .verticalBars = NO,
    .colorAll = NO,
    .clearAll = NO,
    .randDots = NO,
    .cycleForeground = NO,
    .cycleBackground = NO
  };
  
  mode_parms_t parms = {
    .fadeout_dec = INITIAL_FADEOUT_DEC,
    .diffusion_coef = INITIAL_DIFF_COEF,
    .diffusion_increment = INITIAL_DIFF_INC,
    .scrollerDir = LEFT,
    .randMod = INITIAL_RAND_MOD,
    .colorCycleMode = RGB,
    .rainbowInc = INITIAL_RAINBOW_INC
  };
  
  colors_t colors = {
    .foreground = red, 
    .background = black
  };
  
  int count = 0;
  
//  for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
//    textBufferRand[i] = rand() % 2;
//  }

  // Unbuffer stdout...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
  
  // For toning it down.
  if (argc == 2) {
    color_multiplier = atof(argv[1]);
  } else {
    color_multiplier = 1.0;
  }
  
  // Init the display window.
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
  
  
  // Init tensor.
  #ifdef USE_TENSOR
    tensor_init();
    tensor_landscape_p = 1;  // Landscape mode.
  #endif


  // Pattern
  ColorAll(white);
  Update();
 
  // Program loop...
  FOREVER {
    
    // Read all the key events queued up.
    while (SDL_PollEvent(&key_event)) {
      if (key_event.type == SDL_KEYDOWN) {
        
        // <ctrl> <alt>
        if ((key_event.key.keysym.mod & KMOD_ALT) && (key_event.key.keysym.mod & KMOD_CTRL)) {
          colors = ColorAlter(key_event.key.keysym.sym, colors);
          
        // <ctrl>
        } else if (key_event.key.keysym.mod & KMOD_CTRL) {
          mode = ModeAlter(key_event.key.keysym.sym, mode);
          
        // <alt>
        } else if (key_event.key.keysym.mod & KMOD_ALT) {
          parms = ParmAlter(key_event.key.keysym.sym, parms);

        // No extra keys.  Text buffer stuff.
        } else {
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
                printf("Text buffer erased.\n");
                break;
                
              default:
                textBuffer[tindex] = (char)key_event.key.keysym.unicode;
                textBuffer[tindex + 1] = 0x00;
                
                tindex++;
                if (tindex >= (sizeof(textBuffer) - 2)) {
                  tindex--;
                }
                break;
            }                    
            
            printf("Text buffer (%04i): \"%s\"\n", tindex, textBuffer);
          }  // End normal keys.
        }  // End elses between modifier keys.
      }  // End key event occurred.
    } // End event poll.

    if (mode.cycleForeground == YES) {
      colors.foreground = ColorCycle(colors.foreground, parms.colorCycleMode, parms.rainbowInc, &saveCycleF);
    }
    
    if (mode.cycleBackground == YES) {
      colors.background = ColorCycle(colors.background, parms.colorCycleMode, parms.rainbowInc, &saveCycleB);
    }
    
    // Color all
    if (mode.colorAll == YES) {
      ColorAll(colors.foreground);
      mode.colorAll = NO;
    }
    
    // Scroller.
    if (mode.scroller == YES) {
      Scroll(parms.scrollerDir, mode.roller);
    }
    
    // Pixel manips by mode.    
    if (mode.textScroller == YES) {
      // Scroll provided by scroller if its on.
      WriteSlice(textBuffer, colors, parms.scrollerDir, (parms.scrollerDir == RIGHT) ? 0 : TENSOR_WIDTH_EFF - 1);
    }
    
    if (mode.cellAutoFun == YES) {
      // Give each pixel a color value.
      count++;
      for(x = 0 ; x < TENSOR_WIDTH_EFF ; x++) {
        for(y = 0 ; y < TENSOR_HEIGHT_EFF ; y++) {
          oldColor = GetPixel(x,y);
            
          pixelColor.r = (((x + 1) * (y + 1)) - count) + (oldColor.r / 2);
          pixelColor.g = oldColor.g + pixelColor.r;
          pixelColor.b = oldColor.b - 1;
    
          SetPixel(x, y, pixelColor);
        }
      }
    }
    
    if (mode.bouncer == YES) {
      scrDir = (scrDir + 1) % 4;
      Scroll(scrDir, mode.roller);
    }
    
    if (mode.horizontalBars == YES) {
      HorizontalBars(colors);
      mode.horizontalBars = NO;
    }
    
    if (mode.verticalBars == YES) {
      VerticalBars(colors);
      mode.verticalBars = NO;
    }
    
    if (mode.randDots == YES) {
      RandomDots(colors.foreground, parms.randMod);
    }
    
    if (mode.fadeout == YES) {
      FadeAll(parms.fadeout_dec);
    }
    
    if (mode.diffuse == YES) {
      Diffuse(parms.diffusion_coef);
    }
    
    if (mode.clearAll == YES) {
      ColorAll(colors.background);
      mode.clearAll = NO;
    }

    Update();
  }  // End program loop
              
        
  exit(EXIT_SUCCESS);
}



void Diffuse(float diffusion_coeff) {
  int x,y,i,j,k,l;
  color_t colorTemp, finalColor[TENSOR_WIDTH_EFF * TENSOR_HEIGHT_EFF];
  float weightSumR, weightSumG, weightSumB;
  float divisor;
  
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      divisor = 0;
      weightSumR = 0;
      weightSumG = 0;
      weightSumB = 0;
      
      for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
          k = x + i;
          l = y + j;
          if ((k >= 0) && (k < TENSOR_WIDTH_EFF) && (l >= 0) && (l < TENSOR_HEIGHT_EFF)) {
            colorTemp = GetPixel(k, l);
            if ((i == 0) && (j == 0)) {
              weightSumR += colorTemp.r;
              weightSumG += colorTemp.g;
              weightSumB += colorTemp.b;
              divisor = divisor + 1;
            } else {
              weightSumR += (diffusion_coeff * colorTemp.r);
              weightSumG += (diffusion_coeff * colorTemp.g);
              weightSumB += (diffusion_coeff * colorTemp.b);
              divisor += diffusion_coeff;
            }
          }
        }
      }
      
      finalColor[(y * TENSOR_WIDTH_EFF) + x].r = weightSumR / divisor;
      finalColor[(y * TENSOR_WIDTH_EFF) + x].g = weightSumG / divisor;
      finalColor[(y * TENSOR_WIDTH_EFF) + x].b = weightSumB / divisor;
    }
  }
  
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      SetPixel(x,y,finalColor[(y * TENSOR_WIDTH_EFF) + x]);
    }
  }
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
  colorTemp.a = 0; // We don't return an alpha for a written pixel.
  return colorTemp;
}



// Send out the frame buffer to tensor and/or the display window.
void Update(void) {
  UpdatePreview();
  
  #ifdef USE_TENSOR
    UpdateTensor();
  #endif
  
  usleep(50000);
  return;
}



// Send frame buffer to display window
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



// Scroller buffer manipulation
void Scroll (dir_t direction, int rollovermode) { 
  int x, y, i;
  color_t rollSave[TENSOR_WIDTH_EFF + TENSOR_HEIGHT_EFF];
  
  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, 0);
        }
        break;
        
      case DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, TENSOR_HEIGHT_EFF - 1);
        }
        break;
      
      case RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          rollSave[i] = GetPixel(TENSOR_WIDTH_EFF - 1, i);
        }
      break;
      
      case LEFT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          rollSave[i] = GetPixel(0, i);
        }
        break;
        
      default:
        break;
    }
  }
  
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
  
  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixel(i, TENSOR_HEIGHT_EFF - 1, rollSave[i]);
        }
        break;
        
      case DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixel(i, 0, rollSave[i]);
        }
        break;
      
      case RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixel(0, i, rollSave[i]);
        }
      break;
      
      case LEFT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixel(TENSOR_WIDTH_EFF - 1, i, rollSave[i]);
        }
        break;
        
      default:
        break;
    }
  }
  
  return;
}



// Erase while backfilling with a color.
void ScrollErase(dir_t direction, color_t eraseColor) {
  int i;
  
  Scroll(direction, NO);
  
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



//void WriteSlice(char *buffer, color_t font_color, color_t bg_color, dir_t direction, int column) {
void WriteSlice(char *buffer, colors_t font_colors, dir_t direction, int column) {
  static char textBufferRand[TEXT_BUFFER_SIZE] = "a";
  static int imaginaryBufferIndex = -1;
  static dir_t lastDirection = RIGHT;  // Could be.  Who knows?
  unsigned char character;
  int imaginaryBufferSize;
  int fontPixelIndex;
  int bufferIndex;
  int charCol;
  int i;
  
  
  // Some inits just in case.
  if (textBufferRand[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      textBufferRand[i] = rand() % 2;
    }
  }
  
  // The imaginary buffer correspends to the real buffer, but further subdivides
  // each array index into 8 parts.
  imaginaryBufferSize = strlen(buffer) * FONT_WIDTH;
  
  if (imaginaryBufferSize == 0) {
    return;
  }
  
  // Prior to writing out a lines, we increment the imaginary buffer index to 
  // point to the next part of the line to be written.  This depends on a couple
  // of things, i.e. direction (for instance).
  if (lastDirection != direction) {
    // Looks like we changed direction.
    lastDirection = direction;
    printf("Scroll %s\n", direction == LEFT ? "LEFT" : "RIGHT");
    if (direction == LEFT) {
      imaginaryBufferIndex = (imaginaryBufferIndex + TENSOR_WIDTH_EFF) % imaginaryBufferSize;
    } else {
      imaginaryBufferIndex = imaginaryBufferIndex - TENSOR_WIDTH_EFF;
      if (imaginaryBufferIndex < 0) {
        imaginaryBufferIndex = imaginaryBufferSize + imaginaryBufferIndex;
      }
    }  // +/- 1
  } else {
    if (direction == LEFT) {
      imaginaryBufferIndex = (imaginaryBufferIndex + 1) % imaginaryBufferSize;
    } else {
      imaginaryBufferIndex = imaginaryBufferIndex - 1;
      if (imaginaryBufferIndex < 0) {
        imaginaryBufferIndex = imaginaryBufferSize - 1;
      }
    }
  }
      
  // Now using the imaginary index, we find out where in the real buffer we are.
  bufferIndex = imaginaryBufferIndex / FONT_WIDTH;
  
  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (imaginaryBufferIndex % FONT_WIDTH);

  // What the character is. (What it is, man.)
  character = buffer[bufferIndex];
  //printf("%c ", character);
  
  // Up or down justified (zig zagger)
  if (font_colors.background.a == 0) {
    if (textBufferRand[bufferIndex] == 1) {
      SetPixel(column, 0, font_colors.background);
    } else {
      SetPixel(column, TENSOR_HEIGHT_EFF - 1, font_colors.background);
    }
  }
  
  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    fontPixelIndex = (i * FONT_CHARACTER_COUNT) + character;
    
    if ((myfont[fontPixelIndex] & masks[charCol]) != 0) {
      SetPixel(column, i + textBufferRand[bufferIndex], font_colors.foreground);
    } else {
      if (font_colors.background.a == 0) {
        SetPixel(column, i + textBufferRand[bufferIndex], font_colors.background);
      }
    }
  }
  
  return;
}



// Alter foreground and background colors according to key press.
colors_t ColorAlter(int thiskey, colors_t colors) {
  int alphaF = colors.foreground.a;
  int alphaB = colors.background.a;
  
  switch (thiskey) {
    case SDLK_q:
      colors.foreground = red;
      printf("Foreground red.\n");
      break;
      
    case SDLK_w:
      colors.foreground = orange;
      printf("Foreground orange.\n");
      break;
      
    case SDLK_e:
      colors.foreground = yellow;
      printf("Foreground yellow.\n");
      break;
      
    case SDLK_r:
      colors.foreground = green;
      printf("Foreground green.\n");
      break;
      
    case SDLK_t:
      colors.foreground = cyan;
      printf("Foreground cyan.\n");
      break;
      
    case SDLK_y:
      colors.foreground = blue;
      printf("Foreground blue.\n");
      break;
    
    case SDLK_u:
      colors.foreground = magenta;
      printf("Foreground magenta.\n");
      break;
      
    case SDLK_i:
      colors.foreground = white;
      printf("Foreground white.\n");
      break;
      
    case SDLK_o: 
      colors.foreground = grey;
      printf("Foreground grey.\n");
      break;

    case SDLK_p:
      colors.foreground = black;
      printf("Foreground black.\n");
      break;
      
    case SDLK_a:
      colors.background = red;
      printf("Background red.\n");
      break;
      
    case SDLK_s:
      colors.background = orange;
      printf("Background orange.\n");
      break;
      
    case SDLK_d:
      colors.background = yellow;
      printf("Background yellow.\n");
      break;
      
    case SDLK_f:
      colors.background = green;
      printf("Background green.\n");
      break;
      
    case SDLK_g:
      colors.background = cyan;
      printf("Background cyan.\n");
      break;
      
    case SDLK_h:
      colors.background = blue;
      printf("Background blue.\n");
      break;
    
    case SDLK_j:
      colors.background = magenta;
      printf("Background magenta.\n");
      break;
      
    case SDLK_k:
      colors.background = white;
      printf("Background white.\n");
      break;
      
    case SDLK_l: 
      colors.background = grey;
      printf("Background grey.\n");
      break;

    case SDLK_SEMICOLON:
      colors.background = black;
      printf("Background black.\n");
      break;
      
    case SDLK_z:
      if (alphaF == 0) {
        alphaF = 255;
      } else {
        alphaF = 0;
      }
      printf("Foreground alpha set to %i\n", alphaF);
      break;
      
    case SDLK_x:
      if (alphaB == 0) {
        alphaB = 255;
      } else {
        alphaB = 0;
      }
      printf("Background alpha set to %i\n", alphaB);
      break;  

    case SDLK_1:
      printf("<ctrl> <alt> q - foreground red\n");
      printf("<ctrl> <alt> w - foreground orange\n");
      printf("<ctrl> <alt> e - foreground yellow\n");
      printf("<ctrl> <alt> r - foreground green\n");
      printf("<ctrl> <alt> t - foreground cyan\n");
      printf("<ctrl> <alt> y - foreground blue\n");
      printf("<ctrl> <alt> u - foreground magenta\n");
      printf("<ctrl> <alt> i - foreground white\n");
      printf("<ctrl> <alt> o - foreground grey\n");
      printf("<ctrl> <alt> p - foreground black\n");
      printf("<ctrl> <alt> a - background red\n");
      printf("<ctrl> <alt> s - background orange\n");
      printf("<ctrl> <alt> d - background yellow\n");
      printf("<ctrl> <alt> f - background green\n");
      printf("<ctrl> <alt> g - background cyan\n");
      printf("<ctrl> <alt> h - background blue\n");
      printf("<ctrl> <alt> j - background magenta\n");
      printf("<ctrl> <alt> k - background white\n");
      printf("<ctrl> <alt> l - background grey\n");
      printf("<ctrl> <alt> ; - background black\n");
      printf("<ctrl> <alt> z - foreground alpha toggle (currently %i)\n", alphaF);
      printf("<ctrl> <alt> x - backfround alpha toggle (currently %i)\n", alphaB);      
      break;
      
    default:
      break;
  }
  
  colors.foreground.a = alphaF;
  colors.background.a = alphaB;
  
  return(colors);
}



modes_t ModeAlter(int thiskey, modes_t mode) {
  switch(thiskey) {
    case SDLK_c:
      printf("CTRL-c pushed. Exiting\n");
      exit(EXIT_SUCCESS);
      break;
      
    case SDLK_t:
      mode.textScroller = (mode.textScroller + 1) % 2;
      printf("<ctrl> t - Text set to %s\n", (mode.textScroller == YES) ? "ON" : "OFF");
      break;
                
    case SDLK_q:
      mode.cellAutoFun = (mode.cellAutoFun + 1) % 2;
      printf("<ctrl> q - CellAutoFun set to %s\n", (mode.cellAutoFun == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_w:
      mode.bouncer = (mode.bouncer + 1) % 2;
      printf("<ctrl> w - Bouncer set to %s\n", (mode.bouncer == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_e:
      mode.fadeout = (mode.fadeout + 1) % 2;
      printf("<ctrl> e - Fader set to %s\n", (mode.fadeout == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_r:
      mode.diffuse = (mode.diffuse + 1) % 2;
      printf("<ctrl> r - Diffuse set to %s\n", (mode.diffuse == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_y:
      mode.roller = (mode.roller + 1) % 2;
      printf("<ctrl> y - Roller set to %s\n", (mode.roller == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_u:
      mode.scroller = (mode.scroller + 1) %2;
      printf("<ctrl> u - Scroller set to %s\n", (mode.scroller == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_i:
      mode.horizontalBars = YES;
      printf("<ctrl> i - Horizontal bars!\n");
      break;
      
    case SDLK_o:
      mode.verticalBars = YES;
      printf("<ctrl> o - Vertical bars!\n");
      break;
      
    case SDLK_p:
      mode.colorAll = YES;
      printf("<crtl> p - Foreground color all!\n");
      break;
      
    case SDLK_a:
      mode.clearAll = YES;
      printf("<crtl> a - Background color all!\n");
      break;
      
    case SDLK_s:
      mode.randDots = (mode.randDots + 1) % 2;
      printf("<ctrl> s - Random dots set to %s\n", (mode.randDots == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_d:
      mode.cycleForeground = (mode.cycleForeground + 1) % 2;
      printf ("<ctrl> d - Cycle foreground color set to %s\n", (mode.cycleForeground == YES) ? "ON" : "OFF");
      break;
      
    case SDLK_f:
      mode.cycleBackground = (mode.cycleBackground + 1) % 2;
      printf ("<ctrl> f - Cycle background color set to %s\n", (mode.cycleBackground == YES) ? "ON" : "OFF");
      break;

    case SDLK_0:
      mode.bouncer = NO;
      mode.cellAutoFun = NO;
      mode.diffuse = NO;
      mode.fadeout = NO;
      mode.roller = NO;
      mode.textScroller = NO;
      mode.scroller = NO;
      mode.horizontalBars = NO;
      mode.verticalBars = NO;
      mode.randDots = NO;
      mode.colorAll = NO;
      mode.clearAll = NO;
      mode.cycleForeground = NO;
      mode.cycleBackground = NO;
      // No break.  Allow passthrough...
      
    case SDLK_1:
      printf("\n\n");
      printf("<ctrl> t - Text set to %s\n", (mode.textScroller == YES) ? "ON" : "OFF");
      printf("<ctrl> q - CellAutoFun set to %s\n", (mode.cellAutoFun == YES) ? "ON" : "OFF");
      printf("<ctrl> w - Bouncer set to %s\n", (mode.bouncer == YES) ? "ON" : "OFF");
      printf("<ctrl> e - Fader set to %s\n", (mode.fadeout == YES) ? "ON" : "OFF");
      printf("<ctrl> r - Diffuse set to %s\n", (mode.diffuse == YES) ? "ON" : "OFF");
      printf("<ctrl> y - Roller set to %s\n", (mode.roller == YES) ? "ON" : "OFF");
      printf("<ctrl> u - Scroller set to %s\n", (mode.scroller == YES) ? "ON" : "OFF");
      printf("<ctrl> i - Horizontal bars!\n");
      printf("<ctrl> o - Vertical bars!\n");
      printf("<crtl> p - Foreground color all!\n");      
      printf("<crtl> a - Background color all!\n");
      printf("<ctrl> s - Random dots set to %s\n", (mode.randDots == YES) ? "ON" : "OFF");
      printf("<ctrl> d - Cycle foreground color set to %s\n", (mode.cycleForeground == YES) ? "ON" : "OFF");
      printf("<ctrl> f - Cycle background color set to %s\n", (mode.cycleBackground == YES) ? "ON" : "OFF");
      printf("<ctrl> 0 - All off.\n");
      break;
      
    default:
      break;
  }  // End <ctrl> mode switch

  return(mode);
}



mode_parms_t ParmAlter(int thiskey, mode_parms_t parms) {
  switch(thiskey) {
    case SDLK_z:
      parms.fadeout_dec++;
      printf("<alt> z - Fadeout decrement set to %i\n", parms.fadeout_dec);
      break;
      
    case SDLK_c:
      parms.fadeout_dec--;
      printf("<alt> c - Fadeout decrement set to %i\n", parms.fadeout_dec);
      break;
      
    case SDLK_x:
      parms.fadeout_dec = 0;
      printf("<alt> x - Fadeout decrement set to %i\n", parms.fadeout_dec);
      break;
      
    case SDLK_q:
      parms.diffusion_coef = parms.diffusion_coef - parms.diffusion_increment;
      printf("<alt> q - Diffusion coef set to %f\n", parms.diffusion_coef);
      break;
      
    case SDLK_w:
      parms.diffusion_coef = INITIAL_DIFF_COEF;
      printf("<alt> w - Diffusion coef set to %f\n", parms.diffusion_coef);
      break;
      
    case SDLK_e:
      parms.diffusion_coef = parms.diffusion_coef + parms.diffusion_increment;
      printf("<alt> e - Diffusion coef set to %f\n", parms.diffusion_coef);
      break;
      
    case SDLK_a:
      parms.diffusion_increment = parms.diffusion_increment / 10;
      printf("<alt> a - Diffusion coef increment set to %f\n", parms.diffusion_increment);
      break;
      
    case SDLK_s:
      parms.diffusion_increment = INITIAL_DIFF_INC;
      printf("<alt> s - Diffusion coef increment set to %f\n", parms.diffusion_increment);
      break;
      
    case SDLK_d:
      parms.diffusion_increment = parms.diffusion_increment * 10;
      printf("<alt> d - Diffusion coef increment set to %f\n", parms.diffusion_increment);
      break;
      
    case SDLK_2:
      parms.colorCycleMode = (parms.colorCycleMode + 1) % 5;
      printf("<alt> 2 - Color cycle mode set to %s\n", (parms.colorCycleMode == RGB) ? "RGB" : (parms.colorCycleMode == CMY) ? "CMY" : (parms.colorCycleMode == RYGCBM) ? "RYGCBM" : (parms.colorCycleMode == RAINBOW) ? "RAINBOW" : "RANDOM");
      break;
      
    case SDLK_1:
      printf("\n\n");
      printf("<alt> zxc - Fadeout decrement set to %i\n", parms.fadeout_dec);
      printf("<alt> qwe - Diffusion coef set to %f\n", parms.diffusion_coef);
      printf("<alt> asd - Diffusion coef increment set to %f\n", parms.diffusion_increment);
      printf("<alt> <arrows> - Scroll direction set to %s\n", (parms.scrollerDir == UP) ? "UP" : (parms.scrollerDir == DOWN) ? "DOWN" : (parms.scrollerDir == LEFT) ? "LEFT" : "RIGHT");
      printf("<alt> rty - Frequency of random dots set to %i\n", parms.randMod);
      printf("<alt> fgh - Rainbow increment set to %i\n", parms.rainbowInc);
      printf("<alt> 2 - Color cycle mode set to %s\n", (parms.colorCycleMode == RGB) ? "RGB" : (parms.colorCycleMode == CMY) ? "CMY" : (parms.colorCycleMode == RYGCBM) ? "RYGCBM" : (parms.colorCycleMode == RAINBOW) ? "RAINBOW" : "RANDOM");
      break;
      
    case SDLK_f:
      parms.rainbowInc--;
      printf("<alt> f - Rainbow increment set to %i\n", parms.rainbowInc); 
      break;
      
    case SDLK_g:
      parms.rainbowInc = INITIAL_RAINBOW_INC;
      printf("<alt> g - Rainbow increment set to %i\n", parms.rainbowInc); 
      break;

    case SDLK_h:
      parms.rainbowInc++;
      printf("<alt> h - Rainbow increment set to %i\n", parms.rainbowInc); 
      break;
            
    case SDLK_r:
      parms.randMod--;
      printf("<alt> r - Frequency of random dots set to %i\n", parms.randMod);
      break;
      
    case SDLK_t:
      parms.randMod = INITIAL_RAND_MOD;
      printf("<alt> t - Frequency of random dots set to %i\n", parms.randMod);
      break;
      
    case SDLK_y:
      parms.randMod++;
      printf("<alt> y - Frequency of random dots set to %i\n", parms.randMod);
      break;
      
    case SDLK_UP:
      parms.scrollerDir = UP;
      printf("<alt> <up> - Scroller direction set to up.\n");
      break;
      
    case SDLK_DOWN:
      parms.scrollerDir = DOWN;
      printf("<alt> <down> - Scroller direction set to down.\n");
      break;
      
    case SDLK_LEFT:
      parms.scrollerDir = LEFT;
      printf("<alt> <right> - Scroller direction set to left.\n");
      break;
      
    case SDLK_RIGHT:
      parms.scrollerDir = RIGHT;
      printf("<alt> <left> - Scroller direction set to right.\n");
      break;
      
    default:
      break;
  }
  
  return(parms);
}



void HorizontalBars(colors_t colors) {
  int i, j;
  for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
    for (j = 0; j < TENSOR_HEIGHT_EFF; j += 2) {
      SetPixel(i, j, colors.foreground);
    }
  }
}


void VerticalBars(colors_t colors) {
  int i, j;
  for (i = 0; i < TENSOR_WIDTH_EFF; i+=2) {
    for (j = 0; j < TENSOR_HEIGHT_EFF; j++) {
      SetPixel(i,j, colors.foreground);
    }
  }
}


void RandomDots(color_t color, unsigned int rFreq) {
  int x,y;
  if (rFreq == 0) {
    return;
  }
  
  if ((rand() % rFreq) == 0) {
    x = rand() % TENSOR_WIDTH_EFF;
    y = rand() % TENSOR_HEIGHT_EFF;
    SetPixel(x,y,color);
  }
  
}



color_t ColorCycle(color_t originalColor, colorCycles_e cycleMode, int rainbowInc, int *cycleSaver) {
  color_t colorTemp = black;
  int inpos;
  int inposo;
  
  switch(cycleMode) {
    case RGB:
      *cycleSaver = (*cycleSaver + 1) % 3;
      switch(*cycleSaver) {
        case 0:
          colorTemp = red;
          break;
          
        case 1:
          colorTemp = green;
          break;
          
        case 2:
          colorTemp = blue;
          break;
          
        default:
          break;
      }
      break;

    case CMY:
      *cycleSaver = (*cycleSaver + 1) % 3;
      switch(*cycleSaver) {
        case 0:
          colorTemp = cyan;
          break;
          
        case 1:
          colorTemp = magenta;
          break;
          
        case 2:
          colorTemp = yellow;
          break;
          
        default:
          break;
      }
      break;
      
    case RYGCBM:
     *cycleSaver = (*cycleSaver + 1) % 6;
      switch(*cycleSaver) {
        case 0:
          colorTemp = red;
          break;
          
        case 1:
          colorTemp = yellow;
          break;
          
        case 2:
          colorTemp = green;
          break;
          
        case 3:
          colorTemp = cyan;
          break;
          
        case 4:
          colorTemp = blue;
          break;
          
        case 5:
          colorTemp = magenta;
          break;
                   
        default:
          break;
      }
      break;    

    case RAINBOW:
      *cycleSaver = (*cycleSaver + rainbowInc) % 1536;
      inposo = *cycleSaver % 256;
      inpos = *cycleSaver / 256;
      //printf("Csave: %i, inposo: %i, inpos: %i\n", *cycleSaver, inposo, inpos); 
      switch(inpos) {
        case 0: // R -> Y
          colorTemp.r = 255;
          colorTemp.g = inposo;
          colorTemp.b = 0;
          break;
          
        case 1: // Y -> G
          colorTemp.r = 255 - inposo;
          colorTemp.g = 255;
          colorTemp.b = 0;
          break;
          
        case 2: // G -> C
          colorTemp.r = 0;
          colorTemp.g = 255;
          colorTemp.b = inposo;
          break;
          
        case 3: // C -> B
          colorTemp.r = 0;
          colorTemp.g = 255 - inposo;
          colorTemp.b = 255;
          break;
          
        case 4: // B -> M
          colorTemp.r = inposo;
          colorTemp.g = 0;
          colorTemp.b = 255;
          break;
          
        case 5: // M -> R
          colorTemp.r = 255;
          colorTemp.g = 0;
          colorTemp.b = 255 - inposo;
          break;
          
        default:
          break;
      }
      break;
      
    case RANDOM:
      colorTemp.r = rand() % 255;
      colorTemp.g = rand() % 255;
      colorTemp.b = rand() % 255;
      break;
  }
  
  return(colorTemp);
}
