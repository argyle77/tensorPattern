// Tensor Pattern
// Joshua Krueger

// Definitions
#define MAIN_C_

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

#define PREVIEW_PIXEL_WIDTH 10
#define PREVIEW_PIXEL_HEIGHT 10
#define PREVIEW_BORDER_THICKNESS 10

#define TEXT_BUFFER_SIZE 4096

// Initial Values
#define MOMENT_COUNT 10
#define INITIAL_COLOR_MULTIPLIER 1.0
#define INITIAL_DIFF_COEF 0.002
#define INITIAL_FADEOUT_DEC 1
#define INITIAL_RAND_MOD 4
#define INITIAL_RAINBOW_INC 8
#define INITIAL_EXPAND 1
#define INITIAL_FLOAT_INC 0.1
#define INITIAL_ROTATION_DELTA 1.0
#define INITIAL_ROTATION_ANGLE 0
#define INITIAL_ROTATION_ANGLE2 10

#define THISKEY_OFFSET 48
#define FONT_SIZE 14

// Tensor output
#define USE_TENSOR
#define TENSOR_WIDTH_EFF (TENSOR_HEIGHT)
#define TENSOR_HEIGHT_EFF (TENSOR_WIDTH / 3)
#define TENSOR_BYTES_EFF (TENSOR_WIDTH_EFF * TENSOR_HEIGHT_EFF * 3)
#define TENSOR_PREVIEW_WIDTH (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_WIDTH)
#define TENSOR_PREVIEW_HEIGHT (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_HEIGHT)

// Some colors
#define PALETTE_COUNT 12
#define COLOR_WHITE {.r = 255, .g = 255, .b = 255, .a = 0}
#define COLOR_LIGHT_GRAY {.r = 191, .g = 191, .b = 191, .a = 0}
#define COLOR_GRAY {.r = 127, .g = 127, .b = 127, .a = 0}
#define COLOR_DARK_GRAY {.r = 63, .g = 63, .b = 63, .a = 0}
#define COLOR_BLACK {.r = 0, .g = 0, .b = 0, .a = 0}
#define COLOR_RED {.r = 255, .g = 0, .b = 0, .a = 0}
#define COLOR_ORANGE {.r = 255, .g = 127, .b = 0, .a = 0}
#define COLOR_YELLOW {.r = 255, .g = 255, .b = 0, .a = 0}
#define COLOR_GREEN {.r = 0, .g = 255, .b = 0, .a = 0}
#define COLOR_CYAN {.r = 0, .g = 255, .b = 255, .a = 0}
#define COLOR_BLUE {.r = 0, .g = 0, .b = 255, .a = 0}
#define COLOR_MAGENTA {.r = 255, .g = 0, .b = 255, .a = 0}


// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> 
#include "drv-tensor.h"
#include "my_font.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>


// Types

// 24 bit Pixel Color
typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} color_t;

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
  fade_mode_e fade_mode;
  int rotozoom;
  int rotozoom2;
  int alias;
  int multiply;
} modes_t;

typedef enum {
  RGB, CMY, RYGCBM, RAINBOW, RANDOM
} colorCycles_e;

typedef enum {RED_E, ORANGE_E, YELLOW_E, GREEN_E, CYAN_E, BLUE_E, MAGENTA_E,
              WHITE_E, LIGHT_GRAY_E, GRAY_E, DARK_GREY_E, BLACK_E} color_e;

// Parameters
typedef struct {
  color_e foreground;
  color_e background;
  int fadeout_dec;
  float diffusion_coef;
  dir_e scrollerDir;
  unsigned int randMod;
  colorCycles_e colorCycleMode;
  int rainbowInc;
  float expand;
  float floatinc;
  float rotation;
  float rotation2;
  float rotationDelta;
  float colorMultiplier;
  int cycleF;
  int cycleB;
  color_t fg;
  color_t bg;
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
  unsigned char fb[TENSOR_BYTES_EFF];  // Tensor frame buffer
} moment_t;


// Constants
const color_t red = COLOR_RED;
const color_t orange = COLOR_ORANGE;
const color_t yellow = COLOR_YELLOW;
const color_t green = COLOR_GREEN;
const color_t cyan = COLOR_CYAN;
const color_t blue = COLOR_BLUE;
const color_t magenta = COLOR_MAGENTA;
const color_t white = COLOR_WHITE;
const color_t light_gray = COLOR_LIGHT_GRAY;
const color_t gray = COLOR_GRAY;
const color_t dark_gray = COLOR_DARK_GRAY;
const color_t black = COLOR_BLACK;
const color_t palette[PALETTE_COUNT] = {COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN,
                                        COLOR_CYAN, COLOR_BLUE, COLOR_MAGENTA, COLOR_WHITE,
                                        COLOR_LIGHT_GRAY, COLOR_GRAY, COLOR_DARK_GRAY, COLOR_BLACK };
const char *palette_char[] = {"red", "orange", "yellow", "green", "cyan", "blue", "magenta",
                             "white", "3/4", "1/2", "1/4", "black"};


// Globals
TTF_Font *font;
SDL_Surface *screen = NULL;
SDL_Surface *ttemp = NULL;

int randomMode = NO;  // Random mode is a global (for now).



// Prototypes
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int dec, fade_mode_e fade_mode, unsigned char *buffer);
void Scroll (dir_e direction, int rollovermode, unsigned char *fb);
void WriteSlice(moment_t *moment);
void UpdateTensor(unsigned char *buffer);
void UpdatePreview(unsigned char *buffer);
void Diffuse(float diffusion_coeff, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
int ColorAlter(int thiskey, moment_t *moment,int now);
void ModeAlter(int thiskey, moment_t *moment);
void ParmAlter(int thiskey, moment_t *moment);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
void ColorCycle(parms_t *parms, int fb_mode);
void ColorAll(color_t color, unsigned char *fb);
void UpdateInfoDisplay(moment_t *moment, int now);
void PreviewBorder(int width, int height);
void WriteLine(int line, int col, char * thistext);
void Update(float intensity_limit, moment_t *moment);
void UpdateAll(void);
void ClearAll(void);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src);
void Multiply(float multiplier, unsigned char *buffer);


// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  SDL_Event key_event;
  int randomLimit = 1000; // Frame Count Limit.
  int randomCount = 0;    // Frame Count.
  color_t pixelColor, oldColor;
  int x,y,i;
  dir_e scrDir = UP;
  int frameCount = 0;
  int textUpdateNeeded = YES;
  int cellAutoCount = 0;
  float global_intensity_limit;
  moment_t moments[MOMENT_COUNT];
  int now = 0;
  moment_t *currentMoment = &moments[now];
  modes_t *currentMode = &currentMoment->mode;
  parms_t *currentParms = &currentMoment->coefs;
  text_info_t *currentText = &currentMoment->text;
  unsigned char *currentFB = currentMoment->fb;
  char caption_temp[100];
  
  // Unbuffer the console...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  setvbuf(stderr, (char *)NULL, _IONBF, 0);
  
  // Commandline parameter for limiting the intensity.
  if (argc == 2) {
    global_intensity_limit = atof(argv[1]);
    if (global_intensity_limit < -0.0001 || global_intensity_limit > 1.0001) {
      global_intensity_limit = 1.0;
    }
  } else {
    global_intensity_limit = 1.0;
  }

  // Init the display window.
  if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  } else {
    atexit(SDL_Quit);
  }
  
  // Initialize the SDL surfaces.
  screen = SDL_SetVideoMode(900, 768, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video size: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  ttemp = SDL_CreateRGBSurface(SDL_SWSURFACE, TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF, 32, 0,0,0,0);
  if (ttemp == NULL) {
    fprintf(stderr, "Unable to allocate a temp buffer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  
  // Draw a border around the preview
  PreviewBorder(TENSOR_PREVIEW_WIDTH + (PREVIEW_BORDER_THICKNESS * 2),
                TENSOR_PREVIEW_HEIGHT + (PREVIEW_BORDER_THICKNESS * 2));

  sleep(1);  // I don't remember.

  // Set up the keyboard and window access.
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  snprintf(caption_temp, sizeof(caption_temp), "Tensor Control - Output: %i%%", (int) (global_intensity_limit * 100));
  SDL_WM_SetCaption(caption_temp, "Tensor Control");

  // Load the font.
  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  } else {
    font=TTF_OpenFont("font.ttf", FONT_SIZE);
    if(!font) {
      fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
      exit(EXIT_FAILURE);
    }
  }

  // Init tensor.
  #ifdef USE_TENSOR
    tensor_init();
    tensor_landscape_p = 1;  // Landscape mode.
  #endif

  // Set up moment 0.
  currentMode->alias = NO;
  currentMode->bouncer = NO;
  currentMode->cellAutoFun = NO;
  currentMode->clearAll = NO;
  currentMode->colorAll = NO;
  currentMode->cycleBackground = NO;
  currentMode->cycleForeground = YES;
  currentMode->diffuse = NO;
  currentMode->fade_mode = FADE_TO_ZERO;
  currentMode->fadeout = NO;
  currentMode->horizontalBars = NO;
  currentMode->multiply = NO;
  currentMode->randDots = NO;
  currentMode->roller = NO;
  currentMode->rotozoom = NO;
  currentMode->rotozoom2 = NO;
  currentMode->scroller = YES;
  currentMode->textScroller = YES;
  currentMode->verticalBars = NO;

  currentParms->background = BLACK_E;
  currentParms->bg = palette[currentParms->background];
  currentParms->colorCycleMode = RAINBOW;
  currentParms->colorMultiplier = INITIAL_COLOR_MULTIPLIER;
  currentParms->cycleF = 0;
  currentParms->cycleB = 0;
  currentParms->diffusion_coef = INITIAL_DIFF_COEF;
  currentParms->expand = INITIAL_EXPAND;
  currentParms->fadeout_dec = INITIAL_FADEOUT_DEC;
  currentParms->floatinc = INITIAL_FLOAT_INC;
  currentParms->foreground = RED_E;
  currentParms->fg = palette[currentParms->foreground];
  currentParms->rainbowInc = INITIAL_RAINBOW_INC;
  currentParms->randMod = INITIAL_RAND_MOD;
  currentParms->rotation = INITIAL_ROTATION_ANGLE;
  currentParms->rotation2 = INITIAL_ROTATION_ANGLE2;
  currentParms->rotationDelta = INITIAL_ROTATION_DELTA;
  currentParms->scrollerDir = LEFT;

  snprintf(currentText->textBuffer, TEXT_BUFFER_SIZE, "Be the light you wish to see in the world. ");
  currentText->tindex = sizeof("Be the light you wish to see in the world. ") - 1;
  currentText->imaginaryIndex = -1;

  for (i = 1; i < MOMENT_COUNT; i++) {
    memcpy(&moments[i].mode, currentMode, sizeof(modes_t));
    memcpy(&moments[i].coefs, currentParms, sizeof(parms_t));
    memcpy(&moments[i].text, currentText, sizeof(text_info_t));
  }

  // Bam!
  ColorAll(black, currentFB);
  Update(global_intensity_limit, currentMoment);

  // Program loop...
  FOREVER {
    
    // Read all the key events queued up.
    while (SDL_PollEvent(&key_event)) {
      if (key_event.type == SDL_KEYDOWN) {
        textUpdateNeeded = YES;

        // <ctrl> <alt>
        if ((key_event.key.keysym.mod & KMOD_ALT) && (key_event.key.keysym.mod & KMOD_CTRL)) {
          now = ColorAlter(key_event.key.keysym.sym, currentMoment, now);
          currentMoment = &moments[now];
                 currentMode = &currentMoment->mode;
                 currentParms = &currentMoment->coefs;
                 currentText = &currentMoment->text;
                 currentFB = currentMoment->fb;

        // <ctrl>
        } else if (key_event.key.keysym.mod & KMOD_CTRL) {
          ModeAlter(key_event.key.keysym.sym, currentMoment);
          
        // <alt>
        } else if (key_event.key.keysym.mod & KMOD_ALT) {
          ParmAlter(key_event.key.keysym.sym, currentMoment);

        // No extra keys.  Text buffer stuff.
        } else {
          if (key_event.key.keysym.unicode < 0x80 && key_event.key.keysym.unicode > 0) {
            switch(key_event.key.keysym.sym) {
              case SDLK_BACKSPACE:
                currentText->tindex--;
                if (currentText->tindex < 0) {
                  currentText->tindex = 0;
                }
                currentText->textBuffer[currentText->tindex] = 0x00;
                break;
                
              case SDLK_RETURN:
                currentText->textBuffer[currentText->tindex] = '\n';
                currentText->textBuffer[currentText->tindex + 1] = 0x00;
                currentText->tindex++;
                if (currentText->tindex >= (sizeof(currentText->textBuffer) - 2)) {
                  currentText->tindex--;
                }
                break;

              case SDLK_ESCAPE:
                currentText->tindex = 0;
                currentText->textBuffer[0] = 0x00;
                break;
                
              default:
                currentText->textBuffer[currentText->tindex] = (char)key_event.key.keysym.unicode;
                currentText->textBuffer[currentText->tindex + 1] = 0x00;
                currentText->tindex++;
                if (currentText->tindex >= (sizeof(currentText->textBuffer) - 2)) {
                  currentText->tindex--;
                }
                break;
            }
          }  // End normal keys.
        }  // End elses between modifier keys.
      }  // End key event occurred.
    } // End event poll.

    if (randomMode == YES) {
      randomCount++;
      if (randomCount >= randomLimit) {
        randomCount = 0;

        now = (now + 1) % MOMENT_COUNT;
        // Re-appoint the pointers.
        currentMoment = &moments[now];
        currentMode = &currentMoment->mode;
        currentParms = &currentMoment->coefs;
        currentText = &currentMoment->text;
        currentFB = currentMoment->fb;
      }
    }

    // Manipulate the buffer based on the mode.
    if (currentMode->cycleForeground == YES) {
      ColorCycle(currentParms, FOREGROUND);
    }
    
    if (currentMode->cycleBackground == YES) {
      ColorCycle(currentParms, BACKGROUND);
    }
    
    // Color all
    if (currentMode->colorAll == YES) {
      ColorAll(currentParms->fg, currentFB);
      currentMode->colorAll = NO;
    }
    
    // Scroller.
    if (currentMode->scroller == YES) {
      Scroll(currentParms->scrollerDir, currentMode->roller, currentFB);
    }
    
    // Pixel manips by mode.    
    if (currentMode->textScroller == YES) {
      // Scroll provided by scroller if its on.
      WriteSlice(currentMoment);
    }
    
    // Cellular automata manips?  Fix these.
    if (currentMode->cellAutoFun == YES) {
      // Give each pixel a color value.
      cellAutoCount++;
      for(x = 0 ; x < TENSOR_WIDTH_EFF ; x++) {
        for(y = 0 ; y < TENSOR_HEIGHT_EFF ; y++) {
          oldColor = GetPixel(x, y, currentFB);
            
          pixelColor.r = (((x + 1) * (y + 1)) - cellAutoCount) + (oldColor.r / 2);
          pixelColor.g = oldColor.g + pixelColor.r;
          pixelColor.b = oldColor.b - 1;
    
          SetPixel(x, y, pixelColor, currentFB);
        }
      }
    }
    
    // Bouncy bouncy (ick).
    if (currentMode->bouncer == YES) {
      scrDir = (scrDir + 1) % 4;
      Scroll(scrDir, currentMode->roller, currentFB);
    }
    
    // Bam!
    if (currentMode->horizontalBars == YES) {
      HorizontalBars(currentParms->fg, currentFB);
      currentMode->horizontalBars = NO;
    }
    
    // Bam!
    if (currentMode->verticalBars == YES) {
      VerticalBars(currentParms->fg, currentFB);
      currentMode->verticalBars = NO;
    }
    
    // Random dots.  Most useful mode ever.
    if (currentMode->randDots == YES) {
      RandomDots(currentParms->fg, currentParms->randMod, currentFB);
    }
    
    // Fader
    if (currentMode->fadeout == YES) {
      FadeAll(currentParms->fadeout_dec, currentMode->fade_mode, currentFB);
    }
    
    // Averager
    if (currentMode->diffuse == YES) {
      Diffuse(currentParms->diffusion_coef, currentFB);
    }
    
    // Clear screen.
    if (currentMode->clearAll == YES) {
      ColorAll(currentParms->bg, currentFB);
      currentMode->clearAll = NO;
    }

    // Multiplier
    if (currentMode->multiply == YES) {
      Multiply(currentParms->colorMultiplier, currentFB);
    }

    // Experimental Rotozoomer
    if (currentMode->rotozoom2 == YES) {
      Rotate(currentParms->rotation2, currentParms->expand, currentMode->alias, currentFB, currentFB);
    }

    // Update the preview and tensor.
    Update(global_intensity_limit, currentMoment);
    
    // Update the text display, but not more than once in 5 frames.
    if (textUpdateNeeded == YES) {
      if (frameCount > 5) {
        UpdateInfoDisplay(currentMoment, now);
        textUpdateNeeded = NO;
        frameCount = 0;
      }
    }
    
    frameCount++;
  }  // End program loop
              
  // Cleanup. Technically, we never get here.
  TTF_CloseFont(font);
  font=NULL; // to be safe...
  TTF_Quit();
  exit(EXIT_SUCCESS);  // Is it?
}



void Diffuse(float diffusion_coeff, unsigned char *buffer) {
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
            colorTemp = GetPixel(k, l, buffer);
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
      SetPixel(x,y,finalColor[(y * TENSOR_WIDTH_EFF) + x], buffer);
    }
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

// Multiplier
void Multiply(float multiplier, unsigned char *buffer) {
  int i;

  for (i = 0; i < TENSOR_BYTES_EFF; i++) {
    buffer[i] = (unsigned char)((float) buffer[i] * multiplier);
  }
}


// Darken all the pixels by a certain value.
void FadeAll(int dec, fade_mode_e fade_mode, unsigned char *buffer) {
  int x,y;
  color_t oldColor;
    
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      oldColor = GetPixel(x, y, buffer);
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
      
      SetPixel(x,y,oldColor, buffer);
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
  colorTemp.a = 0; // We don't return an alpha for a written pixel.
  return colorTemp;
}





// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
void Update(float intensity_limit, moment_t *moment) {
  modes_t *mode = &moment->mode;
  parms_t *parms = &moment->coefs;
  unsigned char *buffer = moment->fb;
  unsigned char fba[TENSOR_BYTES_EFF];
  int i;

  // Output rotate.
  if (mode->rotozoom == YES) {
    parms->rotation = parms->rotation + parms->rotationDelta;
    Rotate(parms->rotation, parms->expand, mode->alias, &fba[0], &buffer[0]);

    UpdatePreview(fba);

    // Output diminish - no reason to do this to the preview.
    for (i = 0; i < TENSOR_BYTES_EFF; i++) {
      fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
    }

  } else {
    UpdatePreview(buffer);

    // Output diminish
    for (i = 0; i < TENSOR_BYTES_EFF; i++) {
      fba[i] = (unsigned char)((float) buffer[i] * intensity_limit);
    }
  }

  #ifdef USE_TENSOR
    UpdateTensor(fba);
  #endif
  
  usleep(50000);
  return;
}



// Rotate
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src) {
  SDL_Surface *tttemp;
  SDL_Rect offset;

  // Rotation is acheived using SDL_gfx primitive rotozoom.
  // Copy the frame buffer to the sdl surface.
  FBToSurface(ttemp, &fb_src[0]);

  // Rotate / scale it.
  tttemp = rotozoomSurface (ttemp, angle, expansion, (aliasmode == YES) ? 1 : 0);

  offset.x = 0 - (tttemp->w - ttemp->w) / 2;
  offset.y = 0 - (tttemp->h - ttemp->h) / 2;
  //dst.x = UserCenterX - ((MposX - CposX) * BlockRectW) - (W / 2);

   // dst.y = UserCenter_y
     // - (Me.pos.y-CurBullet->pos.y)*Block_Rect.w-CurBullet->SurfacePointer[ PhaseOfBullet ]->h/2;

  //SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
  boxRGBA(ttemp, 0, 0, TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF, 0, 0, 0, 255);
  SDL_BlitSurface(tttemp, NULL, ttemp, &offset);

  //SDL_BlitSurface()
  //SDL_BlitSurface(temp2, NULL, visuals->Screen, &destinationRectangle);
  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, ttemp);
  SDL_FreeSurface(tttemp);
}


// Frame buffer to surface
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB) {
  color_t pixel;
  int x, y;

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {

      // Get pixel color.
      pixel = GetPixel(x, y, FB);

      // Write it to the output surface as a 10x10 square. (PSIZE_X x PSIZE_Y).
      pixelRGBA(surface, x, y, pixel.r, pixel.g, pixel.b, 255);
    }
  }

  return surface;
}

// Surface to frame buffer
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface) {
  int bpp = 4;
  Uint32 *pixel;
  Uint8 r,g,b;
  int x,y;

  for(x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      pixel = surface->pixels + y * surface->pitch + x * bpp;
      SDL_GetRGB(*pixel, surface->format, &r, &g, &b);
      FB[((y * TENSOR_WIDTH_EFF * 3) + (x * 3)) + 0] = r;
      FB[((y * TENSOR_WIDTH_EFF * 3) + (x * 3)) + 1] = g;
      FB[((y * TENSOR_WIDTH_EFF * 3) + (x * 3)) + 2] = b;
    }
  }

  return FB;
}


// Send frame buffer to display window
void UpdatePreview(unsigned char *buffer) {
  color_t pixel;
  int x,y;
  int x1, y1, x2, y2;

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {

      // Get pixel color.
      pixel = GetPixel(x, y, buffer);

      // Write it to the output surface as a 10x10 square. (PSIZE_X x PSIZE_Y).
      x1 = (x * PREVIEW_PIXEL_WIDTH) + PREVIEW_BORDER_THICKNESS;
      y1 = (y * PREVIEW_PIXEL_HEIGHT) + PREVIEW_BORDER_THICKNESS;
      x2 = x1 + PREVIEW_PIXEL_WIDTH - 2;
      y2 = y1 + PREVIEW_PIXEL_HEIGHT - 2;

      boxRGBA(screen, x1, y1, x2, y2,
             (Uint8) pixel.r,
             (Uint8) pixel.g,
             (Uint8) pixel.b,
             (Uint8) 255);
    }
  }
  
  SDL_UpdateRect(screen, 0, 0, TENSOR_PREVIEW_WIDTH + (PREVIEW_BORDER_THICKNESS * 2), TENSOR_PREVIEW_HEIGHT + (PREVIEW_BORDER_THICKNESS * 2));
}



// Scroller buffer manipulation
void Scroll (dir_e direction, int rollovermode, unsigned char *fb) {
  int x, y, i;
  color_t rollSave[TENSOR_WIDTH_EFF + TENSOR_HEIGHT_EFF];
  
  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, 0, fb);
        }
        break;
        
      case DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, TENSOR_HEIGHT_EFF - 1, fb);
        }
        break;
      
      case RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          rollSave[i] = GetPixel(TENSOR_WIDTH_EFF - 1, i, fb);
        }
      break;
      
      case LEFT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          rollSave[i] = GetPixel(0, i, fb);
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
          SetPixel(x, y, GetPixel(x, y+1, fb), fb);
        }
      }
      break;
      
    case DOWN:
      for (y = (TENSOR_HEIGHT_EFF - 1); y > 0; y--) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixel(x, y, GetPixel(x, y - 1, fb),fb);
        }
      }
      break;
      
    case LEFT:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = 0; x < (TENSOR_WIDTH_EFF - 1); x++) {
          SetPixel(x, y, GetPixel(x + 1, y, fb),fb);
        }
      }
      break;
      
    case RIGHT:
    default:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = (TENSOR_WIDTH_EFF - 1); x > 0; x--) {
          SetPixel(x, y, GetPixel(x - 1, y, fb),fb);
        }
      }
      break;
  }
  
  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixel(i, TENSOR_HEIGHT_EFF - 1, rollSave[i],fb);
        }
        break;
        
      case DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixel(i, 0, rollSave[i],fb);
        }
        break;
      
      case RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixel(0, i, rollSave[i],fb);
        }
      break;
      
      case LEFT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixel(TENSOR_WIDTH_EFF - 1, i, rollSave[i],fb);
        }
        break;
        
      default:
        break;
    }
  }
  
  return;
}


void UpdateTensor(unsigned char *buffer) {
  tensor_send(buffer);
}


// Write a slice of text.
void WriteSlice(moment_t *moment) {
  const unsigned char char_masks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  char *buffer = moment->text.textBuffer;
  unsigned char *fb = moment->fb;
  dir_e direction = moment->coefs.scrollerDir;

  int column;
  static char textBufferRand[TEXT_BUFFER_SIZE] = "a";
  static dir_e lastDirection = LEFT;  // Could be.  Who knows?
  unsigned char character;
  int imaginaryBufferSize;
  int *imaginaryBufferIndex = &moment->text.imaginaryIndex;
  int fontPixelIndex;
  int bufferIndex;
  int charCol;
  int i;
  color_t bg_color = moment->coefs.bg;
  color_t fg_color = moment->coefs.fg;

  if (direction == RIGHT) {
    column = 0;
  } else {
    column = TENSOR_WIDTH_EFF - 1;
  }
  
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
      *imaginaryBufferIndex = (*imaginaryBufferIndex + TENSOR_WIDTH_EFF) % imaginaryBufferSize;
    } else {
      *imaginaryBufferIndex = *imaginaryBufferIndex - TENSOR_WIDTH_EFF;
      if (*imaginaryBufferIndex < 0) {
        *imaginaryBufferIndex = imaginaryBufferSize + *imaginaryBufferIndex;
      }
    }  // +/- 1
  } else {
    if (direction == LEFT) {
      *imaginaryBufferIndex = (*imaginaryBufferIndex + 1) % imaginaryBufferSize;
    } else {
      *imaginaryBufferIndex = *imaginaryBufferIndex - 1;
      if (*imaginaryBufferIndex < 0) {
        *imaginaryBufferIndex = imaginaryBufferSize - 1;
      }
    }
  }
      
  // Now using the imaginary index, we find out where in the real buffer we are.
  bufferIndex = *imaginaryBufferIndex / FONT_WIDTH;
  
  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (*imaginaryBufferIndex % FONT_WIDTH);

  // What the character is. (What it is, man.)
  character = buffer[bufferIndex];
  //printf("%c ", character);
  
  // Up or down justified (zig zagger)
  if (bg_color.a == 0) {
    if (textBufferRand[bufferIndex] == 1) {
      SetPixel(column, 0, bg_color,fb);
    } else {
      SetPixel(column, TENSOR_HEIGHT_EFF - 1, bg_color,fb);
    }
  }
  
  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    fontPixelIndex = (i * FONT_CHARACTER_COUNT) + character;
    
    if ((myfont[fontPixelIndex] & char_masks[charCol]) != 0) {
      SetPixel(column, i + textBufferRand[bufferIndex], fg_color,fb);
    } else {
      if (bg_color.a == 0) {
        SetPixel(column, i + textBufferRand[bufferIndex], bg_color,fb);
      }
    }
  }
  
  return;
}



// Alter foreground and background colors according to key press.
int ColorAlter(int thiskey, moment_t *moment, int now) {
  parms_t *parms = &moment->coefs;
  
  switch (thiskey) {
    case SDLK_q:
      parms->foreground = parms->foreground - 1;
      if (parms->foreground < 0 || parms->foreground >= PALETTE_COUNT) {
        parms->foreground = PALETTE_COUNT - 1;
      }
      parms->fg = palette[parms->foreground];
      break;
      
    case SDLK_w:
      parms->foreground++;
      if (parms->foreground >= PALETTE_COUNT) {
        parms->foreground = 0;
      }
      parms->fg = palette[parms->foreground];
      break;
      
    case SDLK_a:
      parms->background--;
      if (parms->background < 0 || parms->background >= PALETTE_COUNT) {
        parms->background = PALETTE_COUNT - 1;
      }
      parms->bg = palette[parms->background];
      break;
      
    case SDLK_s:
      parms->background++;
      if (parms->background >= PALETTE_COUNT) {
        parms->background = 0;
      }
      parms->bg = palette[parms->background];
      break;

    case SDLK_x:
      now = (now + 1) % MOMENT_COUNT;
      break;

    case SDLK_z:
      now = (now - 1);
      if (now < 0) {
        now = MOMENT_COUNT - 1;
      }
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      printf("This saves the current moment in box %i\n", thiskey - THISKEY_OFFSET);
      printf("Or would, if it worked.\n");
      break;
      
    default:
      break;
  }
  return now;
}



void ModeAlter(int thiskey, moment_t *moment) {
  modes_t *mode = &moment->mode;
  parms_t *parms = &moment->coefs;
  text_info_t *text = &moment->text;

  switch(thiskey) {
    case SDLK_ESCAPE:
      printf("CTRL-ESC pushed. Exiting Program.\n");
      TTF_CloseFont(font);
      font=NULL; // to be safe...
      TTF_Quit();
      exit(EXIT_SUCCESS);
      break;
      
    case SDLK_t:
      mode->textScroller = (mode->textScroller + 1) % 2;
      break;
                
    case SDLK_q:
      mode->cellAutoFun = (mode->cellAutoFun + 1) % 2;
      break;
      
    case SDLK_w:
      mode->bouncer = (mode->bouncer + 1) % 2;
      break;
      
    case SDLK_e:
      mode->fadeout = (mode->fadeout + 1) % 2;
      break;
      
    case SDLK_r:
      mode->diffuse = (mode->diffuse + 1) % 2;
      break;
      
    case SDLK_y:
      mode->roller = (mode->roller + 1) % 2;
      break;
      
    case SDLK_u:
      mode->scroller = (mode->scroller + 1) %2;
      break;
      
    case SDLK_i:
      mode->horizontalBars = YES;
      break;
      
    case SDLK_o:
      mode->verticalBars = YES;
      break;
      
    case SDLK_p:
      mode->colorAll = YES;
      break;
      
    case SDLK_a:
      mode->clearAll = YES;
      break;
      
    case SDLK_s:
      mode->randDots = (mode->randDots + 1) % 2;
      break;
      
    case SDLK_d:
      mode->cycleForeground = (mode->cycleForeground + 1) % 2;
      break;
      
    case SDLK_f:
      mode->cycleBackground = (mode->cycleBackground + 1) % 2;
      break;

    case SDLK_g:
      randomMode = (randomMode + 1) % 2;
      break;

    case SDLK_l:
      mode->alias = NO;
      mode->bouncer = NO;
      mode->cellAutoFun = NO;
      mode->clearAll = NO;
      mode->colorAll = NO;
      mode->cycleBackground = NO;
      mode->cycleForeground = NO;
      mode->diffuse = NO;
      mode->fade_mode = FADE_TO_ZERO;
      mode->fadeout = NO;
      mode->horizontalBars = NO;
      mode->multiply = NO;
      mode->randDots = NO;
      mode->roller = NO;
      mode->rotozoom = NO;
      mode->rotozoom2 = NO;
      mode->scroller = NO;
      mode->textScroller = NO;
      mode->verticalBars = NO;
      break;
      
    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      printf("This should load moment %i, but doesn't.\n", thiskey + THISKEY_OFFSET);
      break;

    case SDLK_h:
      mode->fade_mode = (mode->fade_mode + 1) % 2;
      break;
      
    case SDLK_j:
      text->imaginaryIndex = -1;
      break;
      
    case SDLK_k:
      mode->rotozoom = (mode->rotozoom + 1) % 2;
      parms->rotation = 0;
      break;

    case SDLK_z:
      mode->rotozoom2 = (mode->rotozoom2 + 1) % 2;
      break;

    case SDLK_x:
      mode->alias = (mode->alias + 1) % 2;
      break;

    case SDLK_c:
      mode->multiply = (mode->multiply + 1) % 2;
      break;

    default:
      break;
  }  // End <ctrl> mode switch
}



void ParmAlter(int thiskey, moment_t *moment) {
  parms_t *parms = &moment->coefs;

  switch(thiskey) {
    case SDLK_z:
      parms->fadeout_dec--;
      break;
      
    case SDLK_c:
      parms->fadeout_dec++;
      break;
      
    case SDLK_x:
      parms->fadeout_dec = 0;
      break;
      
    case SDLK_q:
      parms->diffusion_coef = parms->diffusion_coef - parms->floatinc;
      break;
      
    case SDLK_w:
      parms->diffusion_coef = INITIAL_DIFF_COEF;
      break;
      
    case SDLK_e:
      parms->diffusion_coef = parms->diffusion_coef + parms->floatinc;
      break;
      
    case SDLK_a:
      parms->expand = parms->expand - parms->floatinc;
      break;
      
    case SDLK_s:
      parms->expand = INITIAL_EXPAND;
      break;
      
    case SDLK_d:
      parms->expand = parms->expand + parms->floatinc;
      break;
      
    case SDLK_p:
      parms->colorCycleMode = (parms->colorCycleMode + 1) % 5;
      break;

    case SDLK_v:
      parms->rotation = parms->rotation - parms->floatinc;
      break;

    case SDLK_b:
      parms->rotation = INITIAL_ROTATION_ANGLE;
      break;

    case SDLK_n:
      parms->rotation = parms->rotation + parms->floatinc;
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      printf("Normally, you'd be looking at the FB of moment %i, but what is normal?\n", thiskey - THISKEY_OFFSET);
      break;

    case SDLK_f:
      parms->rainbowInc--;
      break;
      
    case SDLK_g:
      parms->rainbowInc = INITIAL_RAINBOW_INC;
      break;

    case SDLK_h:
      parms->rainbowInc++;
      break;
            
    case SDLK_SEMICOLON:
      parms->randMod = (parms->randMod + 1) % 10;
      break;

    case SDLK_r:
      parms->rotation2-= parms->floatinc;
      break;
      
    case SDLK_t:
      parms->rotation2 = INITIAL_ROTATION_ANGLE2;
      break;
      
    case SDLK_y:
      parms->rotation2+= parms->floatinc;
      break;
      
    case SDLK_UP:
      parms->scrollerDir = UP;
      break;
      
    case SDLK_DOWN:
      parms->scrollerDir = DOWN;
      break;
      
    case SDLK_LEFT:
      parms->scrollerDir = LEFT;
      break;
      
    case SDLK_RIGHT:
      parms->scrollerDir = RIGHT;
      break;
      
    case SDLK_u:
      parms->colorMultiplier -= parms->floatinc;
      break;
      
    case SDLK_i:
      parms->colorMultiplier = INITIAL_COLOR_MULTIPLIER;
      break;
      
    case SDLK_o:
      parms->colorMultiplier += parms->floatinc;
      break;
      
    case SDLK_j:
      parms->rotationDelta -= parms->floatinc;
      break;
      
    case SDLK_k:
      parms->rotationDelta = INITIAL_ROTATION_DELTA;
      break;
      
    case SDLK_l:
      parms->rotationDelta += parms->floatinc;
      break; 
      
    case SDLK_m:
      parms->floatinc *= 10;
      break;

    case SDLK_COMMA:
      parms->floatinc = INITIAL_FLOAT_INC;
      break;

    case SDLK_PERIOD:
      parms->floatinc /= 10;
      break;

    default:
      break;
  }
}



void HorizontalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
    for (j = 0; j < TENSOR_HEIGHT_EFF; j += 2) {
      SetPixel(i, j, color, buffer);
    }
  }
}


void VerticalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < TENSOR_WIDTH_EFF; i+=2) {
    for (j = 0; j < TENSOR_HEIGHT_EFF; j++) {
      SetPixel(i,j, color, buffer);
    }
  }
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



//void ColorCycle(color_t originalColor, colorCycles_e cycleMode, int rainbowInc, int *cycleSaver) {
void ColorCycle(parms_t *parms, int fb_mode) {
  color_t colorTemp = black;
  int inpos;
  int inposo;
  colorCycles_e cycleMode = parms->colorCycleMode;
  int *cycleSaver;
  //color_t ct2;
  int rainbowInc = parms->rainbowInc;

  if (fb_mode == FOREGROUND) {
    //ct2 = parms->fg;
    cycleSaver = &parms->cycleF;
  } else {
    //ct2 = parms->bg;
    cycleSaver = &parms->cycleB;
  }
  
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

  // We used to return it...
  if (fb_mode == FOREGROUND) {
    parms->fg = colorTemp;
  } else {
    parms->bg = colorTemp;
  }
}


void UpdateInfoDisplay(moment_t *moment, int now) {
  parms_t *parms = &moment->coefs;
  modes_t *mode = &moment->mode;
  text_info_t *text = &moment->text;
  ClearAll();
  
  char thisline[1024];
  char thisparm[1024];
  
  int length;
  int line;
  
  line = 8;
  
  snprintf(thisline, sizeof(thisline), "Modes: ");
  WriteLine(line++, 0, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> q - CellAutoFun");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->cellAutoFun == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> w - Bouncer");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->bouncer == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> e - Fader");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->fadeout == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> r - Diffuse");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->diffuse == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> t - Text");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->textScroller == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> y - Roller");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->roller == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> u - Scroller");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->scroller == YES ? "YES" : "NO");
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
  snprintf(thisparm, sizeof(thisparm), "%s", mode->randDots == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> d - FG cycle");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->cycleForeground == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> f - BG cycle");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->cycleBackground == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> g - Random mode");
  snprintf(thisparm, sizeof(thisparm), "%s", randomMode == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> h - Fader mode");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->fade_mode == FADE_MODULAR ? "MODULAR" : "ZERO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);  
  
  snprintf(thisline, sizeof(thisline), "<ctrl> j - Reset text to start.");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> k - Roto1 mode");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->rotozoom == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> l - All modes off.");
  WriteLine(line++, 0, thisline);

  snprintf(thisline, sizeof(thisline), "<ctrl> z - Roto2 mode");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->rotozoom2 == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> x - Roto anti-alias");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->alias == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);

  snprintf(thisline, sizeof(thisline), "<ctrl> c - Multiplier");
  snprintf(thisparm, sizeof(thisparm), "%s", mode->multiply == YES ? "YES" : "NO");
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  line++;
  line++;
   
  snprintf(thisline, sizeof(thisline), "Text Entry:");
  WriteLine(line++, 0, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<Unmodified keys> - Add text.");
  WriteLine(line++, 0, thisline);          
  
  snprintf(thisline, sizeof(thisline), "<backspace> - Delete last letter.");
  WriteLine(line++, 0, thisline);  
  
  snprintf(thisline, sizeof(thisline), "<esc> - Erase all text.");
  WriteLine(line++, 0, thisline);

  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <esc> - Quit.");
  WriteLine(line++, 0, thisline);  

  // Alt
  line = 1;
  snprintf(thisline, sizeof(thisline), "Coeffs (3 keys means inc, rst, dec):");
  WriteLine(line++, 2, thisline);
  line++;
  
  snprintf(thisline, sizeof(thisline), "<alt> m  ,  . - Float increment");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->floatinc);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);
  line++;

  snprintf(thisline, sizeof(thisline), "<alt> q  w  e - Diffusion");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->diffusion_coef);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> a  s  d - Expansion");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->expand);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> z  x  c - Fade Decrement");
  snprintf(thisparm, sizeof(thisparm), "%i", parms->fadeout_dec);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);


  snprintf(thisline, sizeof(thisline), "<alt> r  t  y - Roto2 Angle");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->rotation2);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> f  g  h - Rainbow Speed");
  snprintf(thisparm, sizeof(thisparm), "%i", parms->rainbowInc);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> v  b  n - Roto1 Angle");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->rotation);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);


  snprintf(thisline, sizeof(thisline), "<alt> u  i  o - Multiplier");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->colorMultiplier);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> j  k  l - Roto1 Speed");
  snprintf(thisparm, sizeof(thisparm), "%f", parms->rotationDelta);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> ; - Random dot 1/freq");
  snprintf(thisparm, sizeof(thisparm), "%i", parms->randMod);
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  snprintf(thisline, sizeof(thisline), "<alt> p - Color cycle type");
  snprintf(thisparm, sizeof(thisparm), "%s", parms->colorCycleMode == RGB ? "R-G-B" :
                                             parms->colorCycleMode == CMY ? "C-M-Y" :
                                             parms->colorCycleMode == RYGCBM ? "R-Y-G-C-B-M" :
                                             parms->colorCycleMode == RAINBOW ? "RAINBOW" :
                                             parms->colorCycleMode == RANDOM ? "RANDOM" : "UNKNOWN");
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);

  
  snprintf(thisline, sizeof(thisline), "<alt> <arrows> - Scroller direction");
  snprintf(thisparm, sizeof(thisparm), "%s", parms->scrollerDir == UP ? "UP" :
                                             parms->scrollerDir == DOWN ? "DOWN" :
                                             parms->scrollerDir == LEFT ? "LEFT" :
                                             parms->scrollerDir == RIGHT ? "RIGHT" : "UNKNOWN");
  WriteLine(line, 2, thisline);
  WriteLine(line++, 3, thisparm);


  line++;
  line++;

  snprintf(thisline, sizeof(thisline), "Colors:");
  WriteLine(line++, 2, thisline);
  
  line++;
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> q w - foreground: %s", palette_char[parms->foreground]);
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> a s - background: %s", palette_char[parms->background]);
  WriteLine(line++, 2, thisline);
  
  snprintf(thisline, sizeof(thisline), "<ctrl> <alt> z x - now: %i", now);
  WriteLine(line++, 2, thisline);

  line = line + 17;

  length=strlen(text->textBuffer);
  line++;
  snprintf(thisline, sizeof(thisline), "Text Buffer:");
  snprintf(thisparm, sizeof(thisparm), "Length: %i", length);
  WriteLine(line, 0, thisline);
  WriteLine(line++, 1, thisparm);
  
  char mybuff[1024];

  strncpy(mybuff, text->textBuffer + (length > 100 ? length - 100 : 0), 101);
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
  
  color_sdl = SDL_MapRGB(screen->format, (Uint8) white.r, (Uint8) white.g, (Uint8) white.b);
  
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
  const SDL_Color font_color = {255, 255, 255};
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
                 TENSOR_PREVIEW_WIDTH + (PREVIEW_BORDER_THICKNESS * 2) + 1,
                 768 - (FONT_SIZE * 8)};
                
  SDL_Rect rect2={TENSOR_PREVIEW_WIDTH + (PREVIEW_BORDER_THICKNESS * 2) + 1,
                  0,
                  900 - TENSOR_PREVIEW_WIDTH + (PREVIEW_BORDER_THICKNESS * 2),
                  768};
                 
  SDL_FillRect(screen, &rect1, 0);
  SDL_FillRect(screen, &rect2, 0);
}
