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

#define INITIAL_DIFF_COEF 0.002
#define INITIAL_DIFF_INC 0.001
#define INITIAL_FADEOUT_DEC 1
#define INITIAL_RAND_MOD 4
#define INITIAL_RAINBOW_INC 8

#define THISKEY_OFFSET 48
#define INITIAL_RANDOM_FRAME_COUNT 10

#define FONT_SIZE 14

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> 
#include "drv-tensor.h"
#include "my_font.h"
#include "SDL.h"  
#include "SDL_ttf.h" 


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
} dir_e;
typedef enum {
  FADE_TO_ZERO, FADE_MODULAR
} fade_mode_e;

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
  int random;
  fade_mode_e fade_mode;
} modes_t;

typedef enum {
  RGB, CMY, RYGCBM, RAINBOW, RANDOM
} colorCycles_e;


// Various parameters
typedef struct {
  int fadeout_dec;
  float diffusion_coef;
  float diffusion_increment;
  dir_e scrollerDir;
  unsigned int randMod;
  colorCycles_e colorCycleMode;
  int rainbowInc;
} mode_parms_t;


typedef struct {
  char textBuffer[TEXT_BUFFER_SIZE];
  int tindex;
} text_info_t;

  
typedef struct {
  modes_t mode;
  mode_parms_t coefs;
  text_info_t text;
  unsigned char fb[TENSOR_BYTES_EFF];  // Tensor frame buffer
} moment_t;

// Globals
const color_t border_c = {.r = 255, .g = 255, .b = 255, .a = 0};
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
double color_increment = .1;
double color_mod_save;
volatile int do_an_update = NO;
TTF_Font *font;
SDL_Color font_color = {255,255,255};
SDL_Color font_back = {0,0,0};
SDL_Surface *screen;
int imaginaryBufferIndex = -1;

moment_t moments[10];
modes_t mode = {
    .textScroller = YES,
    .cellAutoFun = NO,
    .bouncer = NO,
    .fadeout = NO,
    .diffuse = YES,
    .roller = NO,
    .scroller = YES,
    .horizontalBars = NO,
    .verticalBars = NO,
    .colorAll = NO,
    .clearAll = NO,
    .randDots = NO,
    .cycleForeground = YES,
    .cycleBackground = NO,
    .fade_mode = FADE_TO_ZERO,
    .random = NO
  };
  
  mode_parms_t parms = {
    .fadeout_dec = INITIAL_FADEOUT_DEC,
    .diffusion_coef = INITIAL_DIFF_COEF,
    .diffusion_increment = INITIAL_DIFF_INC,
    .scrollerDir = LEFT,
    .randMod = INITIAL_RAND_MOD,
    .colorCycleMode = RAINBOW,
    .rainbowInc = INITIAL_RAINBOW_INC
  };
  
text_info_t text = { .textBuffer = "Be the light you wish to see in the world. ",
                       .tindex = sizeof("Be the light you wish to see in the world. ") - 1 };
int randomCount = 0;
int randomLimit = INITIAL_RANDOM_FRAME_COUNT;

colors_t colors = {.foreground = {.r = 255, .g = 0, .b = 0, .a = 0}, .background = {.r = 0, .g = 0, .b = 0, .a = 0}};


  
// Protos
void SetPixel(int x, int y, color_t color);
color_t GetPixel(int x, int y);
void ColorAll(color_t color);
void Update(void);
void FadeAll(int dec, fade_mode_e fademode);
void Scroll (dir_e direction, int rollovermode);
void ScrollErase(dir_e direction, color_t eraseColor);
void WriteSlice(char *buffer, colors_t font_colors, dir_e direction, int column);
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
moment_t SaveAside(modes_t mode, mode_parms_t coefs, text_info_t text, unsigned char *fb);
void UpdateInfoDisplay(void);
void PreviewBorder(int width, int height);
void WriteLine(int line, int col, char * thistext);
void UpdateAll(void);
void ClearAll(void);



// Main
int main(int argc, char *argv[]) {
  
  SDL_Event key_event;
  color_t pixelColor, oldColor;


  int x,y;
  dir_e scrDir = UP;
  int saveCycleF = 0;
  int saveCycleB = 0;
  int currentMoment = 0;
  
  int frameCount = 0;
  int updateNeeded = YES;

  
  int count = 0;
  
//  for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
//    text.textBufferRand[i] = rand() % 2;
//  }

  // Unbuffer stdout...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
  
  // For toning it down.
  if (argc == 2) {
    color_multiplier = atof(argv[1]);
  } else {
    color_multiplier = 1.0;
  }

  color_mod_save = color_multiplier;
  
  // Init the display window.
  if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  } else {
    atexit(SDL_Quit);
  }
  
  // screen = SDL_SetVideoMode(TENSOR_PREVIEW_WIDTH + (BORDER * 2), TENSOR_PREVIEW_HEIGHT + (BORDER * 2), 32, SDL_SWSURFACE);
    screen = SDL_SetVideoMode(900, 768, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video size: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  
  // Draw a border around the preview
  PreviewBorder(TENSOR_PREVIEW_WIDTH + (BORDER * 2), TENSOR_PREVIEW_HEIGHT + (BORDER * 2));
  sleep(1);
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_WM_SetCaption("Tensor Control", "Click here bitch!");
  // True type...
  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  }

  // load font.ttf at size 16 into font
  
  font=TTF_OpenFont("font.ttf", FONT_SIZE);
  if(!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    // handle error
  }

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
        updateNeeded = YES;
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
                text.tindex--;
                if (text.tindex < 0) {
                  text.tindex = 0;
                }
                text.textBuffer[text.tindex] = 0x00;
                break;
                
              case SDLK_RETURN:
                text.textBuffer[text.tindex] = '\n';
                text.textBuffer[text.tindex + 1] = 0x00;
                text.tindex++;
                if (text.tindex >= (sizeof(text.textBuffer) - 2)) {
                  text.tindex--;
                }
                break;

              case SDLK_ESCAPE:
                text.tindex = 0;
                text.textBuffer[0] = 0x00;
                break;
                
              default:
                text.textBuffer[text.tindex] = (char)key_event.key.keysym.unicode;
                text.textBuffer[text.tindex + 1] = 0x00;
                text.tindex++;
                if (text.tindex >= (sizeof(text.textBuffer) - 2)) {
                  text.tindex--;
                }
                break;
            }                    
            
          }  // End normal keys.
        }  // End elses between modifier keys.
        
     
      }  // End key event occurred.
    } // End event poll.

    if (mode.random == YES) {
      randomCount++;
      if (randomCount >= randomLimit) {
        randomCount = 0;
        currentMoment = (currentMoment + 1)%10;
        memcpy(&mode, &moments[currentMoment].mode, sizeof(modes_t));
        memcpy(&parms, &moments[currentMoment].coefs, sizeof(mode_parms_t));
        memcpy(&text, &moments[currentMoment].text, sizeof(text_info_t));
      }
    }

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
      WriteSlice(text.textBuffer, colors, parms.scrollerDir, (parms.scrollerDir == RIGHT) ? 0 : TENSOR_WIDTH_EFF - 1);
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
      FadeAll(parms.fadeout_dec, mode.fade_mode);
    }
    
    if (mode.diffuse == YES) {
      Diffuse(parms.diffusion_coef);
    }
    
    if (mode.clearAll == YES) {
      ColorAll(colors.background);
      mode.clearAll = NO;
    }

    Update();
    
    if (updateNeeded == YES) {
      // Should put update screen here.  That way, update everytime something changes
      // (key press-wise).
      if (frameCount > 5) {
        UpdateInfoDisplay();
        updateNeeded = NO;
        frameCount = 0;
      }
    }
    
    frameCount++;
  }  // End program loop
              

  TTF_CloseFont(font);
  font=NULL; // to be safe...

  TTF_Quit();
  
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
void FadeAll(int dec, fade_mode_e fade_mode) {
  int x,y;
  color_t oldColor;
    
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      oldColor = GetPixel(x,y);
      if ((oldColor.r < dec) && (fade_mode == FADE_TO_ZERO)) {
        oldColor.r = 0;
      } else {
        oldColor.r -= dec;
      }
      
      if ((oldColor.g < dec) && (fade_mode == FADE_TO_ZERO)) {
        oldColor.g = 0;
      } else {
        oldColor.g -= dec;
      }
      
      if ((oldColor.b < dec) && (fade_mode == FADE_TO_ZERO)) {
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
  
//  SDL_Flip(screen);
  SDL_UpdateRect(screen, 0, 0, TENSOR_PREVIEW_WIDTH + (BORDER * 2), TENSOR_PREVIEW_HEIGHT + (BORDER * 2));
}



// Scroller buffer manipulation
void Scroll (dir_e direction, int rollovermode) { 
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
void ScrollErase(dir_e direction, color_t eraseColor) {
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
void WriteSlice(char *buffer, colors_t font_colors, dir_e direction, int column) {
  static char textBufferRand[TEXT_BUFFER_SIZE] = "a";
  static dir_e lastDirection = LEFT;  // Could be.  Who knows?
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
colors_t ColorAlter(int thiskey, colors_t incolor) {
  
  switch (thiskey) {
    case SDLK_q:
      incolor.foreground = red;
      break;
      
    case SDLK_w:
      incolor.foreground = orange;
      break;
      
    case SDLK_e:
      incolor.foreground = yellow;
      break;
      
    case SDLK_r:
      incolor.foreground = green;
      break;
      
    case SDLK_t:
      incolor.foreground = cyan;
      break;
      
    case SDLK_y:
      incolor.foreground = blue;
      break;
    
    case SDLK_u:
      incolor.foreground = magenta;
      break;
      
    case SDLK_i:
      incolor.foreground = white;
      break;
      
    case SDLK_o: 
      incolor.foreground = grey;
      break;

    case SDLK_p:
      incolor.foreground = black;
      break;
      
    case SDLK_a:
      incolor.background = red;
      break;
      
    case SDLK_s:
      incolor.background = orange;
      break;
      
    case SDLK_d:
      incolor.background = yellow;
      break;
      
    case SDLK_f:
      incolor.background = green;
      break;
      
    case SDLK_g:
      incolor.background = cyan;
      break;
      
    case SDLK_h:
      incolor.background = blue;
      break;
    
    case SDLK_j:
      incolor.background = magenta;
      break;
      
    case SDLK_k:
      incolor.background = white;
      break;
      
    case SDLK_l: 
      incolor.background = grey;
      break;

    case SDLK_SEMICOLON:
      incolor.background = black;
      break;
      
    case SDLK_z:
      if (incolor.foreground.a == 0) {
        incolor.foreground.a = 255;
      } else {
        incolor.foreground.a = 0;
      }
      break;
      
    case SDLK_x:
      if (incolor.background.a == 0) {
        incolor.background.a = 255;
      } else {
        incolor.background.a = 0;
      }
      break;  

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
printf("thiskey: %i\n", thiskey);
printf("1\n");      
memcpy(&mode, &moments[thiskey - THISKEY_OFFSET].mode, sizeof(modes_t));
printf("1\n");      
      memcpy(&parms, &moments[thiskey - THISKEY_OFFSET].coefs, sizeof(mode_parms_t));
printf("1\n");      
      memcpy(&text, &moments[thiskey - THISKEY_OFFSET].text, sizeof(text_info_t));
      break;
      
    default:
      break;
  }
  
  return(incolor);
}



modes_t ModeAlter(int thiskey, modes_t mode) {
  moment_t aMoment;
  switch(thiskey) {
    case SDLK_c:
      printf("CTRL-c pushed. Exiting\n");
      TTF_CloseFont(font);
      font=NULL; // to be safe...
      
      TTF_Quit();
      exit(EXIT_SUCCESS);
      break;
      
    case SDLK_t:
      mode.textScroller = (mode.textScroller + 1) % 2;
      break;
                
    case SDLK_q:
      mode.cellAutoFun = (mode.cellAutoFun + 1) % 2;
      break;
      
    case SDLK_w:
      mode.bouncer = (mode.bouncer + 1) % 2;
      break;
      
    case SDLK_e:
      mode.fadeout = (mode.fadeout + 1) % 2;
      break;
      
    case SDLK_r:
      mode.diffuse = (mode.diffuse + 1) % 2;
      break;
      
    case SDLK_y:
      mode.roller = (mode.roller + 1) % 2;
      break;
      
    case SDLK_u:
      mode.scroller = (mode.scroller + 1) %2;
      break;
      
    case SDLK_i:
      mode.horizontalBars = YES;
      break;
      
    case SDLK_o:
      mode.verticalBars = YES;
      break;
      
    case SDLK_p:
      mode.colorAll = YES;
      break;
      
    case SDLK_a:
      mode.clearAll = YES;
      break;
      
    case SDLK_s:
      mode.randDots = (mode.randDots + 1) % 2;
      break;
      
    case SDLK_d:
      mode.cycleForeground = (mode.cycleForeground + 1) % 2;
      break;
      
    case SDLK_f:
      mode.cycleBackground = (mode.cycleBackground + 1) % 2;
      break;

    case SDLK_l:
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
      mode.fade_mode = FADE_TO_ZERO;
      mode.random = NO;
      // No break.  Allow passthrough...
      
    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      printf("Moment #: %i\n", thiskey);
      aMoment = SaveAside(mode, parms, text, fb);
      memcpy(&moments[thiskey - THISKEY_OFFSET], &aMoment, sizeof(moment_t) );
      break;

    case SDLK_g:
      //mode.random = (mode.random + 1) % 2;
      break;
      
    case SDLK_h:
      mode.fade_mode = (mode.fade_mode + 1) % 2;
      break;
      
    case SDLK_j:
      imaginaryBufferIndex = -1;
      break;
      
    default:
      break;
  }  // End <ctrl> mode switch

  return(mode);
}



mode_parms_t ParmAlter(int thiskey, mode_parms_t parms) {
  switch(thiskey) {
    case SDLK_z:
      parms.fadeout_dec--;
      break;
      
    case SDLK_c:
      parms.fadeout_dec++;
      break;
      
    case SDLK_x:
      parms.fadeout_dec = 0;
      break;
      
    case SDLK_q:
      parms.diffusion_coef = parms.diffusion_coef - parms.diffusion_increment;
      break;
      
    case SDLK_w:
      parms.diffusion_coef = INITIAL_DIFF_COEF;
      break;
      
    case SDLK_e:
      parms.diffusion_coef = parms.diffusion_coef + parms.diffusion_increment;
      break;
      
    case SDLK_a:
      parms.diffusion_increment = parms.diffusion_increment * 10;
      break;
      
    case SDLK_s:
      parms.diffusion_increment = INITIAL_DIFF_INC;
      break;
      
    case SDLK_d:
      parms.diffusion_increment = parms.diffusion_increment / 10;
      break;
      
    case SDLK_p:
      parms.colorCycleMode = (parms.colorCycleMode + 1) % 5;
      break;

    case SDLK_v:
      randomLimit -= 10;
      break;

    case SDLK_b:
      randomLimit = INITIAL_RANDOM_FRAME_COUNT;
      break;

    case SDLK_n:
      randomLimit += 10;
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      memcpy(&mode, &moments[thiskey - THISKEY_OFFSET].mode, sizeof(modes_t));
      memcpy(&parms, &moments[thiskey - THISKEY_OFFSET].coefs,sizeof(mode_parms_t));
      memcpy(&text, &moments[thiskey - THISKEY_OFFSET].text,sizeof(text_info_t));
      memcpy(&fb, &moments[thiskey - THISKEY_OFFSET].fb,TENSOR_BYTES_EFF);
      break;

    case SDLK_f:
      parms.rainbowInc--;
      break;
      
    case SDLK_g:
      parms.rainbowInc = INITIAL_RAINBOW_INC;
      break;

    case SDLK_h:
      parms.rainbowInc++;
      break;
            
    case SDLK_r:
      parms.randMod--;
      break;
      
    case SDLK_t:
      parms.randMod = INITIAL_RAND_MOD;
      break;
      
    case SDLK_y:
      parms.randMod++;
      break;
      
    case SDLK_UP:
      parms.scrollerDir = UP;
      break;
      
    case SDLK_DOWN:
      parms.scrollerDir = DOWN;
      break;
      
    case SDLK_LEFT:
      parms.scrollerDir = LEFT;
      break;
      
    case SDLK_RIGHT:
      parms.scrollerDir = RIGHT;
      break;
      
    case SDLK_u:
      color_multiplier = color_multiplier - color_increment;
      break;
      
    case SDLK_i:
      color_multiplier = color_mod_save;
      break;
      
    case SDLK_o:
      color_multiplier = color_multiplier + color_increment;
      break;
      
    case SDLK_j:
      color_increment = color_increment * 10;
      break;
      
    case SDLK_k:
      color_increment = .1;
      break;
      
    case SDLK_l:
      color_increment = color_increment / 10;
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
  
  //color_t ct2;
  //ct2 = originalColor;
  
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
  
  colorTemp.a = originalColor.a;

  return(colorTemp);
}



moment_t SaveAside(modes_t mode, mode_parms_t coefs, text_info_t text, unsigned char *fb) {
  moment_t thisMoment;
  thisMoment.mode = mode;
  thisMoment.coefs = coefs;
  thisMoment.text = text;
  strncpy(thisMoment.text.textBuffer, text.textBuffer, TEXT_BUFFER_SIZE);
  thisMoment.text.tindex = text.tindex;
  memcpy(&thisMoment.fb, &fb, TENSOR_BYTES_EFF);

  return(thisMoment);
}


void UpdateInfoDisplay(void) {
  ClearAll();
  
  char thisline[1024];
  char thisparm[1024];
  
  int length;
  int line;
  
  line = 8;
  
  snprintf(thisline, sizeof(thisline), "Modes: ");
  WriteLine(line++, 0, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> t - Text");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.textScroller == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> q - CellAutoFun");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.cellAutoFun == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> w - Bouncer");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.bouncer == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> e - Fader");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.fadeout == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> r - Diffuse");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.diffuse == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> y - Roller");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.roller == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> u - Scroller");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.scroller == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> i - Horizontal bars!");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> o - Vertical bars!");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<crtl> p - Foreground color all!");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<crtl> a - Background color all!");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> s - Random dots");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.randDots == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> d - FG cycle");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.cycleForeground == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> f - BG cycle");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.cycleBackground == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
//  snprintf(thisline, sizeof(thisline), "<ctrl> g - Random mode");
//  snprintf(thisparm, sizeof(thisparm), "%s", mode.random == YES ? "YES" : "NO");
//  WriteLine(line, 0, thisline);
//  WriteLine(line++, 1, thisparm);  

  snprintf(thisline, sizeof(thisline), "<ctrl> h - Fader mode");
  snprintf(thisparm, sizeof(thisparm), "%s", mode.fade_mode == FADE_MODULAR ? "MODULAR" : "ZERO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);  
  
  snprintf(thisline, sizeof(thisline), "<ctrl> j - Reset text to start.");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> l - All modes off.");
  WriteLine(line++, 0, thisline);
 
  
  line++;
  line++;
   
  snprintf(thisline, sizeof(thisline), "Other keys:");
  WriteLine(line++, 0, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<Unmodified keys> - Add text.");
  WriteLine(line++, 0, thisline);          
  
  snprintf(thisline, sizeof(thisline), "<backspace> - Delete last letter.");
  WriteLine(line++, 0, thisline);  
  
  snprintf(thisline, sizeof(thisline), "<esc> - Erase all text.");
  WriteLine(line++, 0, thisline);
  

  


  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> c - Quit.");
  WriteLine(line++, 0, thisline);  

  // Alt
  line = 1;
  snprintf(thisline, sizeof(thisline), "Coeffs (3 keys means inc, rst, dec):");
  WriteLine(line++, 2, thisline);
  line++;
  
  snprintf(thisline, sizeof(thisline), "<alt> z, x, c - fader decrementer");
  snprintf(thisparm, sizeof(thisparm), "%i", parms.fadeout_dec);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> q, w, e - Diffusion");
  snprintf(thisparm, sizeof(thisparm), "%f", parms.diffusion_coef);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> a, s, d - Diffusion inc");
  snprintf(thisparm, sizeof(thisparm), "%f", parms.diffusion_increment);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<alt> <arrows> - Scroller direction");
  snprintf(thisparm, sizeof(thisparm), "%s", parms.scrollerDir == UP ? "UP" :
                                             parms.scrollerDir == DOWN ? "DOWN" :
                                             parms.scrollerDir == LEFT ? "LEFT" :
                                             parms.scrollerDir == RIGHT ? "RIGHT" : "UNKNOWN");
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);  

  snprintf(thisline, sizeof(thisline), "<alt> r, t, y - Random dot modulus");
  snprintf(thisparm, sizeof(thisparm), "%i", parms.randMod);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> f, g, h - Rainbow increment");
  snprintf(thisparm, sizeof(thisparm), "%i", parms.rainbowInc);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> p - Color cycle type");
  snprintf(thisparm, sizeof(thisparm), "%s", parms.colorCycleMode == RGB ? "R-G-B" :
                                             parms.colorCycleMode == CMY ? "C-M-Y" :
                                             parms.colorCycleMode == RYGCBM ? "R-Y-G-C-B-M" :
                                             parms.colorCycleMode == RAINBOW ? "RAINBOW" :
                                             parms.colorCycleMode == RANDOM ? "RANDOM" : "UNKNOWN");
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> v, b, n - Random mode counter");
  snprintf(thisparm, sizeof(thisparm), "%i", randomLimit);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<alt> u, i, o - Multiplier");
  snprintf(thisparm, sizeof(thisparm), "%f", color_multiplier);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> j, k, l - Multiplier inc");
  snprintf(thisparm, sizeof(thisparm), "%f", color_increment);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  line++;
  line++;

  snprintf(thisline, sizeof(thisline), "Colors:");
  WriteLine(line++, 2, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> q - foreground red");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> w - foreground orange");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> e - foreground yellow");
  WriteLine(line++, 2, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> r - foreground green");
  WriteLine(line++, 2, thisline);  
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> t - foreground cyan");
  WriteLine(line++, 2, thisline);  
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> y - foreground blue");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> u - foreground magenta");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> i - foreground white");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> o - foreground grey");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> p - foreground black");
  WriteLine(line++, 2, thisline);
  line++;
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> a - background red");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> s - background orange");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> d - background yellow");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> f - background green");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> g - background cyan");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> h - background blue");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> j - background magenta");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> k - background white");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> l - background grey");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> ; - background black");
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> z - foreground alpha");
  snprintf(thisparm, sizeof(thisparm), "%i", colors.foreground.a);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> x - backfround alpha");
  snprintf(thisparm, sizeof(thisparm), "%i", colors.background.a);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  length=strlen(text.textBuffer);
  line++;
  snprintf(thisline, sizeof(thisline), "Text Buffer:");
  snprintf(thisparm, sizeof(thisparm), "Length: %i", length);
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  char mybuff[1024];

  strncpy(mybuff, text.textBuffer + (length > 100 ? length - 100 : 0), 101);
  snprintf(thisline, sizeof(thisline), "%s", mybuff );
  WriteLine(line++, 0, thisline);  

  UpdateAll();
  
}


void PreviewBorder(int width, int height) {
  
  int i;
  Uint32 color_sdl;
  Uint32 *bufp;
  
  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0) {
      return;
    }
  }
  
  color_sdl = SDL_MapRGB(screen->format, (Uint8) border_c.r, (Uint8) border_c.g, (Uint8) border_c.b);
  
  // Vert
  for(i = 0; i <= height; i++) { 
    bufp= (Uint32 *) screen->pixels + (i * screen->pitch / 4) + width;
    *bufp = color_sdl;
  }
  
  // Horz
  for(i = 0; i <= width; i++) { 
    bufp= (Uint32 *) screen->pixels + (height * screen->pitch / 4) + i;
    *bufp = color_sdl;
  }  
  
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
  
  SDL_UpdateRect(screen, 0, 0, width + 1, height + 1);
}



void WriteLine(int line, int col, char * thistext) {
  SDL_Surface *text_surface;
  SDL_Rect rect;
  const int colstart[4] = {0, 275, 375, 675};

  rect.x = colstart[col];
  rect.y = line * (FONT_SIZE);
  rect.w = 0;
  rect.h = 0;
  
  text_surface = TTF_RenderText_Solid(font, thistext, font_color);
  if (text_surface != NULL) {
   SDL_BlitSurface(text_surface, NULL, screen, &rect);
   SDL_FreeSurface(text_surface);
  }
}



void UpdateAll(void) {
  SDL_UpdateRect(screen, 0, 0, 900, 768);
}


void ClearAll(void) {
  SDL_Rect rect1={0,
                 FONT_SIZE * 8,
                 TENSOR_PREVIEW_WIDTH + (BORDER * 2) + 1,
                 768 - (FONT_SIZE * 8)};
                
  SDL_Rect rect2={TENSOR_PREVIEW_WIDTH + (BORDER * 2) + 1,
                  0,
                  900 - TENSOR_PREVIEW_WIDTH + (BORDER * 2),
                  768};
                 
  SDL_FillRect(screen, &rect1, 0);
  SDL_FillRect(screen, &rect2, 0);
}
