// Tensor Pattern
// For Kevin (FB) McCormick (w/o you, there may be nothing) <3
// Blau
// tensor@core9.org

#define MAIN_C_


// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL2_rotozoom.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <time.h>
#include "drv-tensor.h"
#include "my_font.h"


// Defines
#define NO 0
#define YES 1
#define OFF 0
#define ON 1
#define FOREGROUND 0
#define BACKGROUND 1
#define FOREVER for(;;)
#define RANDOM_LIMIT 1000

// Tensor output
#define USE_TENSOR
#define DEF_IMAGE "Fbyte-01.jpg"

// Preview window definitions
#define PREVIEW_PIXEL_WIDTH 10
#define PREVIEW_PIXEL_HEIGHT 10
#define PREVIEW_BORDER_THICKNESS 10
#define PREVIEW_FONT_SIZE 14
#define PREVIEW_FONT_WIDTH 14
#define PREVIEW_FONT_HEIGHT 14
#define TENSOR_PREVIEW_WIDTH (TENSOR_WIDTH * PREVIEW_PIXEL_WIDTH)
#define TENSOR_PREVIEW_HEIGHT (TENSOR_WIDTH * PREVIEW_PIXEL_HEIGHT)

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


// Types

// 32 bit Pixel Color, which is, as yet, sadly underused.
typedef struct color_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;  // right
} color_t;

// Scroller directions
typedef enum dir_e {
  UP = 0, LEFT, DOWN, RIGHT
} dir_e;

// Better scroller directions
typedef enum scr_e{
  SCR_HOLD, SCR_UL, SCR_U, SCR_UR, SCR_L, SCR_R, SCR_DL, SCR_D, SCR_DR, SCR_COUNT
} scr_e;

// Color plane flags - I'm unsatified with the way the cym color planes worked
// out.  I should be using a different color space or something.
#define PLANE_NONE    0x00
#define PLANE_RED     0x01
#define PLANE_GREEN   0x02
#define PLANE_BLUE    0x04
#define PLANE_CYAN    (PLANE_BLUE | PLANE_GREEN)
#define PLANE_YELLOW  (PLANE_RED | PLANE_GREEN)
#define PLANE_MAGENTA (PLANE_RED | PLANE_BLUE)
#define PLANE_ALL     (PLANE_RED | PLANE_BLUE | PLANE_GREEN)

typedef enum fade_mode_e{
  FADE_TO_ZERO, FADE_MODULAR, FADE_HALF
} fade_mode_e;

// Pattern modes
typedef struct modes_t {
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
  int stagger;
  int sidebar;
  int clearRed;
  int clearGreen;
  int clearBlue;
  int shiftred;
  int shiftgreen;
  int shiftblue;
  int shiftcyan;
  int shiftyellow;
  int shiftmagenta;
  int image;
} modes_t;

typedef enum colorCycles_e{
  RGB, CMY, RYGCBM, RAINBOW, RANDOM
} colorCycles_e;

typedef enum color_e {RED_E, ORANGE_E, YELLOW_E, GREEN_E, CYAN_E, BLUE_E, MAGENTA_E,
              WHITE_E, LIGHT_GRAY_E, GRAY_E, DARK_GREY_E, BLACK_E} color_e;

// Parameters
typedef struct parms_t{
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
  int cellAutoCount;
  int textRow;
  int delay;
} parms_t;

typedef struct text_info_t{
  char textBuffer[TEXT_BUFFER_SIZE];
  int tindex;  // How many chars in.
  int imaginaryIndex;  // How many pixel cols in?
  char textBufferRand[TEXT_BUFFER_SIZE];
  dir_e lastDirection;
} text_info_t;

typedef struct moment_t{
  modes_t mode;
  parms_t coefs;
  text_info_t text;
  unsigned char fb[TENSOR_BYTES];  // Tensor frame buffer
} moment_t;

// A list of the items to update for more information.
typedef enum infoList_e {
  INFO_CELL, INFO_BOUNCE, INFO_FADE, INFO_DIFFUSE, INFO_TEXTMODE, INFO_ROLL,
  INFO_SCROLL, INFO_DOTS, INFO_FG, INFO_BG, INFO_RANDOM, INFO_FADEMODE,
  INFO_ROTOZOOM, INFO_ROTOZOOM2, INFO_ALIAS, INFO_MULTIPLY, INFO_TEXTLEN,
  INFO_TEXT, INFO_INC, INFO_DIFF, INFO_EXP, INFO_DEC, INFO_ANGLE2,
  INFO_RSPEED, INFO_ANGLE1, INFO_MULT, INFO_ROT1SPEED, INFO_DOTF, INFO_CYCLE,
  INFO_DIR, INFO_FGC, INFO_BGC, INFO_NOW, INFO_TEXTROW, INFO_TEXTSTAG,
  INFO_SIDEBAR, INFO_CLEARRED, INFO_CLEARGREEN, INFO_CLEARBLUE,
  INFO_SHIFTRED, INFO_SHIFTGREEN, INFO_SHIFTBLUE, INFO_FPS, INFO_DELAY,
  INFO_SHIFTCYAN, INFO_SHIFTYELLOW, INFO_SHIFTMAGENTA, INFO_IMAGE,
  INFO_COUNT  // Always last.
} infoList_e;

// The location of those items on screen (row, col#)
// What a mess.
const int infoLoc[INFO_COUNT][2] = {
  /* INFO_CELL */      {23,1},
  /* INFO_BOUNCE */    {24,1},
  /* INFO_FADE */      {25,1},
  /* INFO_DIFFUSE */   {26,1},
  /* INFO_TEXTMODE */  {27,1},
  /* INFO_ROLL */      {28,1},
  /* INFO_SCROLL */    {29,1},
  /* INFO_DOTS */      {34,1},
  /* INFO_FG */        {35,1},
  /* INFO_BG */        {36,1},
  /* INFO_RANDOM */    {37,1},
  /* INFO_FADEMODE */  {38,1},
  /* INFO_ROTOZOOM */  {40,1},
  /* INFO_ROTOZOOM2 */ {42,1},
  /* INFO_ALIAS */     {43,1},
  /* INFO_MULTIPLY */  {44,1},
  /* INFO_TEXTLEN */   {39,3},
  /* INFO_TEXT */      {52,0},
  /* INFO_INC */       {3,3},
  /* INFO_DIFF */      {5,3},
  /* INFO_EXP */       {6,3},
  /* INFO_DEC */       {7,3},
  /* INFO_ANGLE2 */    {8,3},
  /* INFO_RSPEED */    {9,3},
  /* INFO_ANGLE1 */    {10,3},
  /* INFO_MULT */      {11,3},
  /* INFO_ROT1SPEED */ {12,3},
  /* INFO_DOTF */      {13,3},
  /* INFO_CYCLE */     {14,3},
  /* INFO_DIR */       {15,3},
  /* INFO_FGC */       {21,3},
  /* INFO_BGC */       {22,3},
  /* INFO_NOW */       {27,3},
  /* INFO_TEXTROW */   {37,3},
  /* INFO_TEXTSTAG */  {38,3},
  /* INFO_SIDEBAR */   {45,1},
  /* INFO_CLEARRED */  {46, 1},
  /* INFO_CLEARGREEN */ {47, 1},
  /* INFO_CLEARBLUE */ {48, 1},
  /* INFO_SHIFTRED */ {44,3},
  /* INFO_SHIFTGREEN */ {45,3},
  /* INFO_SHIFTBLUE */ {46,3},
  /* INFO_FPS */       {21,1},
  /* INFO_DELAY */     {16, 3},
  /* INFO_SHIFTCYAN */ {47, 3},
  /* INFO_SHIFTYELLOW */ {48, 3},
  /* INFO_SHIFTMAGENTA */ {49,3},
  /* INFO_IMAGE */  {49,1 }
};


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
const color_t palette[PALETTE_COUNT] = {
  COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_CYAN, COLOR_BLUE,
  COLOR_MAGENTA, COLOR_WHITE, COLOR_LIGHT_GRAY, COLOR_GRAY, COLOR_DARK_GRAY,
  COLOR_BLACK };
const char *palette_char[] = {
  "red", "orange", "yellow", "green", "cyan", "blue", "magenta", "white", "3/4",
  "1/2", "1/4", "black" };

// Globals - We do love our globals.
TTF_Font *font;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;
SDL_Surface *previewSurface = NULL;
SDL_Surface *imageSeed = NULL;
int TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF;
int randomMode = NO;  // Random mode is a global (for now).
moment_t moments[MOMENT_COUNT];
int now;
moment_t *currentMoment;
modes_t *currentMode;
parms_t *currentParms;
text_info_t *currentText;
unsigned char *currentFB;
float global_intensity_limit;
time_t mnow;
int frames, fps;

// Prototypes
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int dec, fade_mode_e fade_mode, unsigned char *buffer);
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane);
void WriteSlice(moment_t *moment);
void CellFun(moment_t *moment);
void DrawSideBar(moment_t *moment);
void UpdateTensor(unsigned char *buffer);
void UpdatePreview(unsigned char *buffer);
void Diffuse(float diffusion_coeff, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
int ColorAlter(int thiskey, moment_t *moments,int now);
int ModeAlter(int thiskey, moment_t *moments, int now);
void ParmAlter(int thiskey, moment_t *moments, int now);
void SaveMoments(SDL_KeyboardEvent *key, moment_t *moments, int now);
void LoadMoments(SDL_KeyboardEvent *key, moment_t *moments, char *fn);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
void ColorCycle(parms_t *parms, int fb_mode);
void ColorAll(color_t color, unsigned char *fb);
void UpdateInfoDisplay(moment_t *moment, int now);
void qDrawRectangle(int x, int y, int width, int height, color_t color);
void WriteLine(char * thisText, int line, int col, color_t color);
void Update(float intensity_limit, moment_t *moment);
void UpdateAll(void);
void ClearAll(void);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src);
void Multiply(float multiplier, unsigned char *buffer);
void InitInfoDisplay(void);
void SetDims(void);
void DrawPreviewBorder(void);
void DrawRectangle(int x, int y, int w, int h, color_t color);
void DrawBox(int x, int y, int w, int h, color_t color);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void ClearRed(moment_t *currentMoment);
void ClearGreen(moment_t *currentMoment);
void ClearBlue(moment_t *currentMoment);
void HandleInput(SDL_Event *key_event);
void ProcessModes(void);
void MainLoop(void);
Uint32 TriggerProcessing(Uint32 interval, void *param);
Uint32 FrameCounter(Uint32 interval, void *param);
int min(int a, int b);
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void DrawImage(double angle, double expansion, int aliasmode, unsigned char *fb_dst);
int max(int a, int b);
int min(int a, int b);

// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  int i;
  unsigned char exitProgram = 0;
  char caption_temp[100];
  SDL_Event event;

  // Some inits
  now = 0;
  currentMoment = &moments[now];
  currentMode = &currentMoment->mode;
  currentParms = &currentMoment->coefs;
  currentText = &currentMoment->text;
  currentFB = currentMoment->fb;

  frames = 0;
  fps = 0;

  // Set the times
  mnow = time(NULL);

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
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  atexit(SDL_Quit);

  // Initialize the SDL window and renderer.
  SDL_CreateWindowAndRenderer(900, 768, SDL_WINDOW_RESIZABLE, &mainWindow, &mwRenderer);
  if (!mainWindow) {
    fprintf(stderr, "Unable to create main window: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (!mwRenderer) {
    fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(mwRenderer, 900, 768);

  // Set the window title
  snprintf(caption_temp, sizeof(caption_temp), "Tensor Control - Output: %i%%", (int) (global_intensity_limit * 100));
  SDL_SetWindowTitle(mainWindow, caption_temp);

  // Load the image.
  imageSeed = IMG_Load(DEF_IMAGE);
  if (!imageSeed) {
    fprintf(stderr, "Unable to load image: %s\n", DEF_IMAGE);
    exit(EXIT_FAILURE);
  }

  // Set the widths / heights
  SetDims();

  // Draw a border around the preview
  DrawPreviewBorder();

  // Load the font.
  if(TTF_Init() < 0) {
    printf("Couldn't initialize TTF: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  } else {
    font=TTF_OpenFont("font.ttf", PREVIEW_FONT_SIZE);
    if(!font) {
      fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
      exit(EXIT_FAILURE);
    }
  }

  // Init tensor.
#ifdef USE_TENSOR
  tensor_init();
  //tensor_landscape_p = 1;  // Landscape mode.
  tensor_landscape_p = 0;  // Portrait mode.
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
  currentMode->stagger = 3;
  currentMode->sidebar = NO;
  currentMode->clearBlue = NO;
  currentMode->clearGreen = NO;
  currentMode->clearRed = NO;
  currentMode->shiftred = 0;
  currentMode->shiftgreen = 0;
  currentMode->shiftblue = 0;
  currentMode->shiftcyan = 0;
  currentMode->shiftmagenta = 0;
  currentMode->shiftyellow = 0;
  currentMode->image = NO;

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
  currentParms->cellAutoCount = 0;
  currentParms->textRow = TENSOR_HEIGHT_EFF / 3 - 1;
  currentParms->delay = 60;

  snprintf(currentText->textBuffer, TEXT_BUFFER_SIZE, "Frostbyte was an engineer. ");
  currentText->tindex = strlen(currentText->textBuffer);
  currentText->imaginaryIndex = -1;
  currentText->lastDirection = LEFT;
  currentText->textBufferRand[0] = 'a';

  ColorAll(black, currentFB);

  // There is a secret hidden bug in the moment copying or changing or disk read / write
  // that I haven't found yet...
  for (i = 1; i < MOMENT_COUNT; i++) {
    memcpy(&moments[i], currentMoment, sizeof(moment_t));
  }

  // Attempt to load a file...
  LoadMoments(NULL, &moments[0], "1.now");

  // Bam!
  ColorAll(black, currentFB);
  Update(global_intensity_limit, currentMoment);

  // Add the text.
  InitInfoDisplay();

  // Init the processing timer.
  if (SDL_AddTimer(currentParms->delay, TriggerProcessing, NULL) == 0) {
    printf("Process Timer problem!\n");
    exit(0);
  }

  // FPS counter.
  if (SDL_AddTimer(1000, FrameCounter, NULL) == 0) {
    printf("Frame Count Timer problem!\n");
    exit(0);
  }

  unsigned char doprocess = NO;

  // Program loop...
  FOREVER {

    // Act on queued events.
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          HandleInput(&event);
          break;

        // User events are either an fps counter update, or calculating and writing a new frame.
        case SDL_USEREVENT:
          switch (event.user.code) {
            case 0:
              doprocess = YES;
              break;
            case 1:
              fps = frames;
              frames = 0;
              break;
            default:
              break;
          }
          break;

        case SDL_QUIT:
          exitProgram = 1;
          break;

       case SDL_WINDOWEVENT:
         if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
           ClearAll();
           DrawPreviewBorder();
           InitInfoDisplay();
         }
         break;


        default:
          break;
      }
    }

    // Best not to do this in the interrupt.
    if (doprocess == YES) {
      doprocess = NO;
      frames++;
      MainLoop();
    }

    if (exitProgram) {
      break;
    }
  }  // End program loop

  // Cleanup.
  TTF_CloseFont(font);
  font=NULL; // to be safe...?
  TTF_Quit();
  exit(EXIT_SUCCESS);  // Is it?
}

// Event that triggers a frame to be drawn.
Uint32 TriggerProcessing(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);

  return(currentParms->delay);
}

// Event that triggers the fps to be updated.
Uint32 FrameCounter(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 1;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);

  return(interval);
}

// The thing that happens at every frame.
void MainLoop(void) {
  // Update the buffer. (I.E. make pattern)
  ProcessModes();

  // Update the preview and tensor.
  Update(global_intensity_limit, currentMoment);

  // Update the text display.
  UpdateInfoDisplay(currentMoment, now);
}

// Time to make a mess of the array.
void ProcessModes(void) {
  static int randomCount = 0;    // Frame Count.
  static dir_e scrDir = UP;

  // Random mode isn't random.  It cycles through the moments.  It allows you
  // to set up a bunch of pattern sets and switch between them one at a time at
  // some interval.
  if (randomMode == YES) {
    randomCount++;
    if (randomCount >= RANDOM_LIMIT) {
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

  // Change foreground color.
  if (currentMode->cycleForeground == YES) {
    ColorCycle(currentParms, FOREGROUND);
  }

  // Change background color.
  if (currentMode->cycleBackground == YES) {
    ColorCycle(currentParms, BACKGROUND);
  }

  // Seed the entire array with the foreground color.
  if (currentMode->colorAll == YES) {
    ColorAll(currentParms->fg, currentFB);
    currentMode->colorAll = NO;
  }

  // Scroller.
  if (currentMode->scroller == YES) {
    Scroll(currentParms->scrollerDir, currentMode->roller, currentFB, PLANE_ALL);
  }

  // Scroll red.
  if ((currentMode->shiftred != 0)) {
    Scroll(currentMode->shiftred - 1, currentMode->roller, currentFB, PLANE_RED);
  }

  // Scroll green.
  if ((currentMode->shiftgreen != 0)) {
    Scroll(currentMode->shiftgreen - 1, currentMode->roller, currentFB, PLANE_GREEN);
  }

  // Scroll blue.
  if ((currentMode->shiftblue != 0)) {
    Scroll(currentMode->shiftblue - 1, currentMode->roller, currentFB, PLANE_BLUE);
  }

  // Scroll blue and green
  if ((currentMode->shiftcyan != 0)) {
      Scroll(currentMode->shiftcyan - 1, currentMode->roller, currentFB, PLANE_CYAN);
  }

  // Scroll red and blue.
  if ((currentMode->shiftmagenta != 0)) {
    Scroll(currentMode->shiftmagenta - 1, currentMode->roller, currentFB, PLANE_MAGENTA);
  }

  // Scroll green and red.
  if ((currentMode->shiftyellow != 0)) {
    Scroll(currentMode->shiftyellow - 1, currentMode->roller, currentFB, PLANE_YELLOW);
  }

  // Draw a solid bar up the side we are scrolling from.
  if (currentMode->sidebar == YES) {
    DrawSideBar(currentMoment);
  }

  // First stab experimental image drawing.  Needs work.
  if (currentMode->image == YES) {
    DrawImage(currentParms->rotation, currentParms->expand, currentMode->alias, currentFB);
    currentParms->rotation = currentParms->rotation + currentParms->rotationDelta;
  }

  // I think this just writes a column of text on the right or left.
  if (currentMode->textScroller == YES) {
    // Scroll provided by scroller if its on.
    WriteSlice(currentMoment);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (currentMode->cellAutoFun == YES) {
    // Give each pixel a color value.
    CellFun(currentMoment);
  }

  // Bouncy bouncy (ick).
  if (currentMode->bouncer == YES) {
    scrDir = (scrDir + 1) % 4;
    Scroll(scrDir, currentMode->roller, currentFB, PLANE_ALL);
  }

  // Bam!  Draw some horizontal bars.
  if (currentMode->horizontalBars == YES) {
    HorizontalBars(currentParms->fg, currentFB);
    currentMode->horizontalBars = NO;
  }

  // Bam!
  if (currentMode->verticalBars == YES) {
    VerticalBars(currentParms->fg, currentFB);
    currentMode->verticalBars = NO;
  }

  // Random dots.  Most useful seed ever.
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

  // Zero the red.
  if (currentMode->clearRed == YES) {
    ClearRed(currentMoment);
  }

  // Zero the blue.
  if (currentMode->clearBlue == YES) {
    ClearBlue(currentMoment);
  }

  // Zero the green.
  if (currentMode->clearGreen == YES) {
    ClearGreen(currentMoment);
  }
}

// Key press processing.
void HandleInput(SDL_Event *key_event) {

  // <ctrl> <alt>
  if ((key_event->key.keysym.mod & KMOD_ALT) && (key_event->key.keysym.mod & KMOD_CTRL)) {
    now = ColorAlter(key_event->key.keysym.sym, &moments[0], now);
    currentMoment = &moments[now];
    currentMode = &currentMoment->mode;
    currentParms = &currentMoment->coefs;
    currentText = &currentMoment->text;
    currentFB = currentMoment->fb;

  // <alt> <shift>
  } else if ((key_event->key.keysym.mod & KMOD_ALT) && (key_event->key.keysym.mod & KMOD_SHIFT)) {
    SaveMoments(&key_event->key, &moments[0], now);

  // <ctrl> <shift>
  } else if ((key_event->key.keysym.mod & KMOD_CTRL) && (key_event->key.keysym.mod & KMOD_SHIFT)) {
    LoadMoments(&key_event->key, &moments[0], NULL);

  // <ctrl>
  } else if (key_event->key.keysym.mod & KMOD_CTRL) {
    now = ModeAlter(key_event->key.keysym.sym, &moments[0], now);
    currentMoment = &moments[now];
    currentMode = &currentMoment->mode;
    currentParms = &currentMoment->coefs;
    currentText = &currentMoment->text;
    currentFB = currentMoment->fb;
  // <alt>
  } else if (key_event->key.keysym.mod & KMOD_ALT) {
    ParmAlter(key_event->key.keysym.sym, &moments[0], now);

  // No modifier keys.  Text buffer stuff.
  } else {
    if (key_event->key.keysym.sym < 0x80 && key_event->key.keysym.sym > 0) {
      switch(key_event->key.keysym.sym) {
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
          currentText->textBuffer[currentText->tindex] = (char)key_event->key.keysym.sym;
          currentText->textBuffer[currentText->tindex + 1] = 0x00;
          currentText->tindex++;
          if (currentText->tindex >= (sizeof(currentText->textBuffer) - 2)) {
            currentText->tindex--;
          }
          break;
      }
    }  // End normal keys.
  }  // End elses between modifier keys.
}

// Weighted Averager.
void Diffuse(float diffusion_coeff, unsigned char *buffer) {
  int x,y,i,j,k,l;
  color_t colorTemp, finalColor[TENSOR_WIDTH * TENSOR_HEIGHT];
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

// Multiply all pixels by a value.
void Multiply(float multiplier, unsigned char *buffer) {
  int i;

  for (i = 0; i < TENSOR_BYTES; i++) {
    buffer[i] = (unsigned char)((float) buffer[i] * multiplier);
  }
}


// Darken (or lighten) all the pixels by a certain value.
void FadeAll(int dec, fade_mode_e fade_mode, unsigned char *buffer) {
  int x,y;
  color_t oldColor;

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      oldColor = GetPixel(x, y, buffer);
      if ((oldColor.r < dec) && (fade_mode == FADE_TO_ZERO)) {
        oldColor.r = 0;
      } else {
        // 8 bit modular fade.
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

// Set a single pixel a particular color by color plane.
// Rediculous.
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer) {
  switch(plane) {
    case PLANE_ALL:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_RED:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
      break;

    case PLANE_GREEN:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_BLUE:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
      break;

    // More complicated.
    case PLANE_CYAN:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_YELLOW:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_MAGENTA:
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 0] = color.r;
      buffer[(y * TENSOR_WIDTH_EFF * 3) + (x * 3) + 2] = color.b;
      break;
    default:
      break;

  }
}

// Set a single pixel a particular color with alpha blending.
// Is this ever used?  Does it work?
void SetPixelA(int x, int y, color_t color, unsigned char *buffer) {
  color_t colorTemp;
  float a, r1, g1, b1, r2, g2, b2;

  // Input colors.
  r1 = color.r;
  g1 = color.g;
  b1 = color.b;

  // Get the current color.
  colorTemp = GetPixel(x, y, buffer);
  r2 = colorTemp.r;
  g2 = colorTemp.g;
  b2 = colorTemp.b;

  // Normalize the alpha value. Normal = (in - min) / (max - min)
  a = color.a;
  a = a / 255;

  // Calculate the blended outputs = value = (1 - a) Value0 + a Value1
  color.r = (unsigned char) (a * r1) + ((1 - a) * r2);
  color.g = (unsigned char) (a * g1) + ((1 - a) * g2);
  color.b = (unsigned char) (a * b1) + ((1 - a) * b2);

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
  colorTemp.a = 0; // We don't return an alpha for a written pixel?
  return colorTemp;
}





// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
void Update(float intensity_limit, moment_t *moment) {
  modes_t *mode = &moment->mode;
  parms_t *parms = &moment->coefs;
  unsigned char *buffer = moment->fb;
  unsigned char fba[TENSOR_BYTES];
  int i;

  // Output rotate (doesn't effect array values, but does effect the final image).
  if (mode->rotozoom == YES) {
    parms->rotation = parms->rotation + parms->rotationDelta;
    Rotate(parms->rotation, parms->expand, mode->alias, &fba[0], &buffer[0]);

    UpdatePreview(fba);

    // Output diminish - no reason to do this to the preview (or is there?)
    for (i = 0; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
    }

  } else {
    UpdatePreview(buffer);

    // Output diminish
    for (i = 0; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) buffer[i] * intensity_limit);
    }
  }

#ifdef USE_TENSOR
  UpdateTensor(fba);
#endif

  // I can't remember if this is necessary - it would be frame limiting, huh?
  usleep(50000);

  return;
}


// Rotate
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src) {
  SDL_Surface *rotatedSurface;
  SDL_Rect offset;

  FBToSurface(previewSurface, fb_src);

  // Rotate / scale it.
  rotatedSurface = rotozoomSurface (previewSurface, angle, expansion, (aliasmode == YES) ? 1 : 0);
  if (!rotatedSurface) {
    fprintf(stderr, "Error rotating surface: %s\n", SDL_GetError());
    return;
  }

  // Recenter and copy it back to the previewSurface
  offset.x = 0 - (rotatedSurface->w - previewSurface->w) / 2;
  offset.y = 0 - (rotatedSurface->h - previewSurface->h) / 2;
  SDL_FillRect(previewSurface, NULL, 0);
  SDL_BlitSurface(rotatedSurface, NULL, previewSurface, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, previewSurface);
  SDL_FreeSurface(rotatedSurface);
}


// Frame buffer to SDL surface
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB) {
  color_t pixel;
  unsigned char *surfacePixels;
  int x, y;

  surfacePixels = surface->pixels;
  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {

      // Get pixel color.
      pixel = GetPixel(x, y, FB);
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 0] = pixel.b;
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 1] = pixel.g;
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 2] = pixel.r;
    }
  }
  return surface;
}

// SDL Surface to frame buffer
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface) {
  Uint32 *pixel;
  Uint8 r,g,b;
  int x,y;

  for(x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      pixel = surface->pixels + y * surface->pitch + x * (surface->pitch / surface->w);
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

      boxRGBA(mwRenderer, x1, y1, x2, y2,
             (Uint8) pixel.r,
             (Uint8) pixel.g,
             (Uint8) pixel.b,
             (Uint8) 255);
    }
  }
  SDL_RenderPresent(mwRenderer);
}


// Scroller buffer manipulation
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane) {
  int x, y, i;
  color_t rollSave[TENSOR_WIDTH + TENSOR_HEIGHT];  // Size = Why?

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
          SetPixelByPlane(x, y, GetPixel(x, y+1, fb), plane, fb);
        }
      }
      break;

    case DOWN:
      for (y = (TENSOR_HEIGHT_EFF - 1); y > 0; y--) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y - 1, fb), plane, fb);
        }
      }
      break;

    case LEFT:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = 0; x < (TENSOR_WIDTH_EFF - 1); x++) {
          SetPixelByPlane(x, y, GetPixel(x + 1, y, fb),plane, fb);
        }
      }
      break;

    case RIGHT:
    default:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = (TENSOR_WIDTH_EFF - 1); x > 0; x--) {
          SetPixelByPlane(x, y, GetPixel(x - 1, y, fb),plane, fb);
        }
      }
      break;
  }

  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixelByPlane(i, TENSOR_HEIGHT_EFF - 1, rollSave[i], plane, fb);
        }
        break;

      case DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixelByPlane(i, 0, rollSave[i],plane, fb);
        }
        break;

      case RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixelByPlane(0, i, rollSave[i],plane, fb);
        }
      break;

      case LEFT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixelByPlane(TENSOR_WIDTH_EFF - 1, i, rollSave[i],plane, fb);
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
  int textRow = moment->coefs.textRow;
  int effRow;
  int column;
  unsigned char character;
  int imaginaryBufferSize;
  int *imaginaryBufferIndex = &moment->text.imaginaryIndex;
  int fontPixelIndex;
  int bufferIndex;
  int charCol;
  int i;
  int useRand;
  color_t bg_color = moment->coefs.bg;
  color_t fg_color = moment->coefs.fg;

  if (direction == RIGHT) {
    column = 0;
  } else {
    column = TENSOR_WIDTH_EFF - 1;
  }

  // Some inits just in case.
  if (moment->text.textBufferRand[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      moment->text.textBufferRand[i] = rand() % 2;
    }
  }

  // The imaginary buffer correspends to the real buffer, but further subdivides
  // each array index into 8 parts.
  imaginaryBufferSize = strlen(buffer) * FONT_WIDTH;

  if (imaginaryBufferSize == 0) {
    return;
  }

  // Prior to writing out a line, we increment the imaginary buffer index to
  // point to the next part of the line to be written.  This depends on a couple
  // of things, i.e. direction (for instance).
  if (moment->text.lastDirection != direction) {
    // Looks like we changed direction.
    moment->text.lastDirection = direction;
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
  // Integer division.
  bufferIndex = *imaginaryBufferIndex / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (*imaginaryBufferIndex % FONT_WIDTH);

  // What the character is. (What it is, man.)
  character = buffer[bufferIndex];
  //printf("%c ", character);

  // Up or down justified (zig zagger) for better fit on landscape panel.
  useRand = 0;
  if (moment->mode.stagger == 1) {
    // Stagger.  For landscape.  Fill in the row that isn't covered by the letter.
    if (moment->text.textBufferRand[bufferIndex] == 1) {
      effRow = textRow;
    } else {
      effRow = textRow + 8;
    }

    if ((effRow >= 0) && (effRow < TENSOR_HEIGHT_EFF)) {
      SetPixelA(column, effRow, bg_color,fb);
    }

    useRand = moment->text.textBufferRand[bufferIndex];
  } else if (moment->mode.stagger == 2) {
    // No stagger, but draw a single line border on top and bottom of text with bg.
    effRow = textRow - 1;
    if ((effRow >= 0) && (effRow < TENSOR_HEIGHT_EFF)) {
      SetPixelA(column, effRow, bg_color,fb);
    }
    effRow = textRow + 8;
    if ((effRow >= 0) && (effRow < TENSOR_HEIGHT_EFF)) {
      SetPixelA(column, effRow, bg_color,fb);
    }
  } else if (moment->mode.stagger == 3) {
    // No stagger, but background the whole image.
    for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
      SetPixelA(column, i, bg_color, fb);
    }
  }

  // stagger == 0 means no border, no stagger.

  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    fontPixelIndex = (i * FONT_CHARACTER_COUNT) + character;
    effRow = i + useRand + textRow;
    if ((myfont[fontPixelIndex] & char_masks[charCol]) != 0) {
      SetPixelA(column, effRow, fg_color,fb);
    } else {
      SetPixelA(column, effRow, bg_color, fb);
    }
  }

  return;
}



// Alter foreground and background colors according to key press.
// There's more here than just that, though.
int ColorAlter(int thiskey, moment_t *moments, int now) {
  moment_t *moment = &moments[now];
  parms_t *parms = &moment->coefs;
  modes_t *mode = &moment->mode;

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

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      memcpy(&moments[thiskey - SDLK_0], moment, sizeof(moment_t));
      printf("Current moment (%i) saved in box %i\n", now, thiskey - SDLK_0);
      break;

    case SDLK_z:
      parms->textRow--;
      if (parms->textRow < (0 - (FONT_HEIGHT + 1))) {
        parms->textRow = TENSOR_HEIGHT_EFF;
      }
      break;

    case SDLK_x:
      parms->textRow++;
      if (parms->textRow > TENSOR_HEIGHT_EFF) {
        parms->textRow = 0 - (FONT_HEIGHT + 1);
      }
      break;

    case SDLK_u:
      parms->delay--;
      if (parms->delay < 1) {
        parms->delay = 1;
      }
      break;

    case SDLK_i:
      parms->delay = 60;
      break;

    case SDLK_o:
      parms->delay++;
      break;

    case SDLK_LEFTBRACKET:
      mode->shiftcyan = (mode->shiftcyan + 1) % 5;
      break;

    case SDLK_RIGHTBRACKET:
      mode->shiftyellow = (mode->shiftyellow + 1) % 5;
      break;

    case SDLK_BACKSLASH:
      mode->shiftmagenta = (mode->shiftmagenta + 1) % 5;
      break;

    case SDLK_p:
      mode->image = (mode->image + 1) % 2;
      break;

    default:
      break;
  }
  return now;
}



int ModeAlter(int thiskey, moment_t *moments, int now) {
  moment_t *moment = &moments[now];
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
      mode->sidebar = NO;
      mode->clearRed = NO;
      mode->clearBlue = NO;
      mode->clearGreen = NO;
      mode->shiftblue = 0;  // Oh shit, I just realized that NO = 1.
      mode->shiftcyan = 0;
      mode->shiftgreen = 0;
      mode->shiftmagenta = 0;
      mode->shiftyellow = 0;
      mode->shiftred = 0;
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      now = thiskey - SDLK_0;
      printf("Moment %i loaded.\n", now);
      break;

    case SDLK_h:
      mode->fade_mode = (mode->fade_mode + 1) % 2;
      break;

    case SDLK_m:
      mode->clearRed = (mode->clearRed + 1) % 2;
      break;

    case SDLK_COMMA:
         mode->clearGreen = (mode->clearGreen + 1) % 2;
         break;

    case SDLK_PERIOD:
         mode->clearBlue = (mode->clearBlue + 1) % 2;
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

    case SDLK_n:
      mode->sidebar = (mode->sidebar + 1) % 2;
      break;

    case SDLK_v:
      tensor_landscape_p = (tensor_landscape_p + 1) % 2;
      SetDims();
      DrawPreviewBorder();
      break;

    case SDLK_b:
      mode->stagger = (mode->stagger + 1) % 4;
      break;

    case SDLK_LEFTBRACKET:
      mode->shiftred = (mode->shiftred + 1) % 5;
      break;
    case SDLK_RIGHTBRACKET:
      mode->shiftgreen = (mode->shiftgreen + 1) % 5;
      break;
    case SDLK_BACKSLASH:
      mode->shiftblue = (mode->shiftblue + 1) % 5;
      break;

    default:
      break;
  }  // End <ctrl> mode switch

  return(now);
}


void ParmAlter(int thiskey, moment_t *moments, int now) {
  moment_t *moment = &moments[now];
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
      memcpy(moment->fb, moments[thiskey - SDLK_0].fb, sizeof(unsigned char) * TENSOR_BYTES);
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

// You can put your head in your hands here.
void InitInfoDisplay(void) {

    char text[1024];
    int line;

    line = 21;

    snprintf(text, sizeof(text), "Modes: ");
    WriteLine(text, line++, 0, white);

    line++;

    snprintf(text, sizeof(text), "<ctrl> q - CellAutoFun");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> w - Bouncer");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> e - Fader");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> r - Diffuse");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> t - Text");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> y - Roller");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> u - Scroller");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> i - Horizontal bars!");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> o - Vertical bars!");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<crtl> p - Foreground color all!");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<crtl> a - Background color all!");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> s - Random dots");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> d - FG cycle");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> f - BG cycle");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> g - Random mode");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> h - Fader mode");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> j - Reset text to start.");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> k - Roto1 mode");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> l - All modes off.");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> z - Roto2 mode");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> x - Roto anti-alias");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> c - Multiplier");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> n - SideBar");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> m - No Red");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> , - No Green");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> . - No Blue");
    WriteLine(text, line++, 0, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> p - Image Toggle");
    WriteLine(text, line++, 0, white);


    line++;
    snprintf(text, sizeof(text), "Text Buffer:");
    WriteLine(text, line++, 0, white);

    // Alt
    line = 1;
    snprintf(text, sizeof(text), "Coeffs: (3 Keys = Increment, Reset, Decrement)");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<alt> m  ,  . - Float step size");
    WriteLine(text, line++, 2, white);
    line++;

    snprintf(text, sizeof(text), "<alt> q  w  e - Diffusion");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> a  s  d - Expansion");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> z  x  c - Fade Decrement");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> r  t  y - Roto2 Angle");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> f  g  h - Rainbow Speed");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> v  b  n - Roto1 Angle");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> u  i  o - Multiplier");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> j  k  l - Roto1 Speed");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> ; - Random dot period");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> p - Color cycle type");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> <arrows> - Scroller direction");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> u i o - Frame delay(ms)");
    WriteLine(text, line++, 2, white);

    line++;
    line++;

    snprintf(text, sizeof(text), "Colors:");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<ctrl> <alt> q w - foreground:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> a s - background:");
    WriteLine(text, line++, 2, white);

    line++;
    line++;

    snprintf(text, sizeof(text), "Moments:");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<ctrl> 0-9 - Load Moment. Current:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> 0-9 - Copy Moment.");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<alt> 0-9 - Load another moment's image buffer.");
    WriteLine(text, line++, 2, white);

    line++;
    line++;

    snprintf(text, sizeof(text), "Text Entry:");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<Unmodified keys> - Add text.");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<backspace> - Delete last letter.");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<esc> - Erase all text.");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<crtl> <alt> z x - text row:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> b - text mod:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "Text Buffer Size:");
    WriteLine(text, line++, 2, white);

    line++;
    line++;

    snprintf(text, sizeof(text), "More Modes:");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<ctrl> [ - Red Plane:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> ] - Green Plane:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> \\ - Blue Plane:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> [ - Cyan Plane:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> ] - Yellow Plane:");
    WriteLine(text, line++, 2, white);

    snprintf(text, sizeof(text), "<ctrl> <alt> \\ - Magenta Plane:");
    WriteLine(text, line++, 2, white);

    line++;

    snprintf(text, sizeof(text), "<ctrl> <esc> - Quit.");
    WriteLine(text, line++, 2, white);

    UpdateAll();
}

// And here.
void UpdateInfoDisplay(moment_t *moment, int now) {
  char text[1024], mybuff[102];
  int length;
  parms_t *parms = &moment->coefs;
  modes_t *mode = &moment->mode;
  text_info_t *scrollText = &moment->text;

  snprintf(text, sizeof(text), "FPS: %4i", fps);
  WriteLine(text, infoLoc[INFO_FPS][0], infoLoc[INFO_FPS][1], yellow);

  snprintf(text, sizeof(text), "%6i", parms->delay);
  WriteLine(text, infoLoc[INFO_DELAY][0], infoLoc[INFO_DELAY][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->image == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_IMAGE][0], infoLoc[INFO_IMAGE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->cellAutoFun == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_CELL][0], infoLoc[INFO_CELL][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->bouncer == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_BOUNCE][0], infoLoc[INFO_BOUNCE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->fadeout == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_FADE][0], infoLoc[INFO_FADE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->diffuse == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_DIFFUSE][0], infoLoc[INFO_DIFFUSE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->textScroller == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_TEXTMODE][0], infoLoc[INFO_TEXTMODE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->roller == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_ROLL][0], infoLoc[INFO_ROLL][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->scroller == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_SCROLL][0], infoLoc[INFO_SCROLL][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->randDots == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_DOTS][0], infoLoc[INFO_DOTS][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->cycleForeground == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_FG][0], infoLoc[INFO_FG][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->cycleBackground == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_BG][0], infoLoc[INFO_BG][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->clearRed == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_CLEARRED][0], infoLoc[INFO_CLEARRED][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->clearGreen == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_CLEARGREEN][0], infoLoc[INFO_CLEARGREEN][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->clearBlue == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_CLEARBLUE][0], infoLoc[INFO_CLEARBLUE][1], yellow);

  snprintf(text, sizeof(text), "%3s", randomMode == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_RANDOM][0], infoLoc[INFO_RANDOM][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->fade_mode == FADE_MODULAR ? "MOD" : "LIM");
  WriteLine(text, infoLoc[INFO_FADEMODE][0], infoLoc[INFO_FADEMODE][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->rotozoom == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_ROTOZOOM][0], infoLoc[INFO_ROTOZOOM][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->rotozoom2 == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_ROTOZOOM2][0], infoLoc[INFO_ROTOZOOM2][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->alias == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_ALIAS][0], infoLoc[INFO_ALIAS][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->multiply == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_MULTIPLY][0], infoLoc[INFO_MULTIPLY][1], yellow);

  snprintf(text, sizeof(text), "%3s", mode->sidebar == YES ? "YES" : "NO");
  WriteLine(text, infoLoc[INFO_SIDEBAR][0], infoLoc[INFO_SIDEBAR][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->floatinc);
  WriteLine(text, infoLoc[INFO_INC][0], infoLoc[INFO_INC][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->diffusion_coef);
  WriteLine(text, infoLoc[INFO_DIFF][0], infoLoc[INFO_DIFF][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->expand);
  WriteLine(text, infoLoc[INFO_EXP][0], infoLoc[INFO_EXP][1], yellow);

  snprintf(text, sizeof(text), "%14i", parms->fadeout_dec);
  WriteLine(text, infoLoc[INFO_DEC][0], infoLoc[INFO_DEC][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftred == 0 ? " Hold" :
                                             mode->shiftred == 1 ? "   Up" :
                                             mode->shiftred == 2 ? " Left" :
                                             mode->shiftred == 3 ? " Down" :
                                             mode->shiftred == 4 ? "Right" :
                                                                   "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTRED][0], infoLoc[INFO_SHIFTRED][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftgreen == 0 ? " Hold" :
                                             mode->shiftgreen == 1 ? "   Up" :
                                             mode->shiftgreen == 2 ? " Left" :
                                             mode->shiftgreen == 3 ? " Down" :
                                             mode->shiftgreen == 4 ? "Right" :
                                                                     "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTGREEN][0], infoLoc[INFO_SHIFTGREEN][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftblue == 0 ? " Hold" :
                                             mode->shiftblue == 1 ? "   Up" :
                                             mode->shiftblue == 2 ? " Left" :
                                             mode->shiftblue == 3 ? " Down" :
                                             mode->shiftblue == 4 ? "Right" :
                                                                    "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTBLUE][0], infoLoc[INFO_SHIFTBLUE][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftcyan == 0 ? " Hold" :
                                             mode->shiftcyan == 1 ? "   Up" :
                                             mode->shiftcyan == 2 ? " Left" :
                                             mode->shiftcyan == 3 ? " Down" :
                                             mode->shiftcyan == 4 ? "Right" :
                                                                    "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTCYAN][0], infoLoc[INFO_SHIFTCYAN][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftyellow == 0 ? " Hold" :
                                             mode->shiftyellow == 1 ? "   Up" :
                                             mode->shiftyellow == 2 ? " Left" :
                                             mode->shiftyellow == 3 ? " Down" :
                                             mode->shiftyellow == 4 ? "Right" :
                                                                    "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTYELLOW][0], infoLoc[INFO_SHIFTYELLOW][1], yellow);

  snprintf(text, sizeof(text), "%s", mode->shiftmagenta == 0 ? " Hold" :
                                             mode->shiftmagenta == 1 ? "   Up" :
                                             mode->shiftmagenta == 2 ? " Left" :
                                             mode->shiftmagenta == 3 ? " Down" :
                                             mode->shiftmagenta == 4 ? "Right" :
                                                                    "Unk!!" );
  WriteLine(text, infoLoc[INFO_SHIFTMAGENTA][0], infoLoc[INFO_SHIFTMAGENTA][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->rotation2);
  WriteLine(text, infoLoc[INFO_ANGLE2][0], infoLoc[INFO_ANGLE2][1], yellow);

  snprintf(text, sizeof(text), "%14i", parms->rainbowInc);
  WriteLine(text, infoLoc[INFO_RSPEED][0], infoLoc[INFO_RSPEED][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->rotation);
  WriteLine(text, infoLoc[INFO_ANGLE1][0], infoLoc[INFO_ANGLE1][1], yellow);


  snprintf(text, sizeof(text), "%14.6f", parms->colorMultiplier);
  WriteLine(text, infoLoc[INFO_MULT][0], infoLoc[INFO_MULT][1], yellow);

  snprintf(text, sizeof(text), "%14.6f", parms->rotationDelta);
  WriteLine(text, infoLoc[INFO_ROT1SPEED][0], infoLoc[INFO_ROT1SPEED][1], yellow);

  snprintf(text, sizeof(text), "%14i", parms->randMod);
  WriteLine(text, infoLoc[INFO_DOTF][0], infoLoc[INFO_DOTF][1], yellow);

  snprintf(text, sizeof(text), "%11s", parms->colorCycleMode == RGB ? "R-G-B" :
                                             parms->colorCycleMode == CMY ? "C-M-Y" :
                                             parms->colorCycleMode == RYGCBM ? "R-Y-G-C-B-M" :
                                             parms->colorCycleMode == RAINBOW ? "RAINBOW" :
                                             parms->colorCycleMode == RANDOM ? "RANDOM" : "UNKNOWN");
  WriteLine(text, infoLoc[INFO_CYCLE][0], infoLoc[INFO_CYCLE][1], yellow);


  snprintf(text, sizeof(text), "%5s", parms->scrollerDir == UP ? "UP" :
                                             parms->scrollerDir == DOWN ? "DOWN" :
                                             parms->scrollerDir == LEFT ? "LEFT" :
                                             parms->scrollerDir == RIGHT ? "RIGHT" : "UNK");
  WriteLine(text, infoLoc[INFO_DIR][0], infoLoc[INFO_DIR][1], yellow);


  snprintf(text, sizeof(text), "%10s", palette_char[parms->foreground]);
  WriteLine(text, infoLoc[INFO_FGC][0], infoLoc[INFO_FGC][1], yellow);

  snprintf(text, sizeof(text), "%10s", palette_char[parms->background]);
  WriteLine(text, infoLoc[INFO_BGC][0], infoLoc[INFO_BGC][1], yellow);

  snprintf(text, sizeof(text), "%1i", now);
  WriteLine(text, infoLoc[INFO_NOW][0], infoLoc[INFO_NOW][1], yellow);

  snprintf(text, sizeof(text), "%3i", parms->textRow);
  WriteLine(text, infoLoc[INFO_TEXTROW][0], infoLoc[INFO_TEXTROW][1], yellow);

  snprintf(text, sizeof(text), "%11s", mode->stagger == 1 ? "9RowStagger" :
                                             mode->stagger == 2 ? "10RowBorder" :
                                             mode->stagger == 0 ? "8Row" :
                                             mode->stagger == 3 ? "FullBG" : "Unknown");

  WriteLine(text, infoLoc[INFO_TEXTSTAG][0], infoLoc[INFO_TEXTSTAG][1], yellow);

  snprintf(text, sizeof(text), "%4i", (int) strlen(scrollText->textBuffer));
  WriteLine(text, infoLoc[INFO_TEXTLEN][0], infoLoc[INFO_TEXTLEN][1], yellow);

  length = strlen(scrollText->textBuffer);
  strncpy(mybuff, scrollText->textBuffer + (length > 100 ? length - 100 : 0), 101);
  snprintf(text, sizeof(text), "%-100s", mybuff );
  WriteLine(text, infoLoc[INFO_TEXT][0], infoLoc[INFO_TEXT][1], yellow);

  UpdateAll();
}


// Writes a line of text to the selected line and column of the
// output window with the selected color.
void WriteLine(char * thisText, int line, int col, color_t color) {

  // Vars
  const SDL_Color fontColor = {color.r, color.g, color.b};
  const SDL_Color bgColor = {0, 0, 0};
  SDL_Surface *textSurface;
  SDL_Texture *textTexture;
  SDL_Rect rect;

  const int colstart[4] = {0, 275, 375, 675};

  // Set the position of the output text.
  rect.x = colstart[col];
  rect.y = line * (PREVIEW_FONT_HEIGHT);

  // Render the text to a surface.
  textSurface = TTF_RenderText_Shaded(font, thisText, fontColor, bgColor);
  if (textSurface) {
    // Set the width and height of the output text.
    rect.w = textSurface->w;
    // rect.h = textSurface->h;
    rect.h = PREVIEW_FONT_HEIGHT;

    // Create a texture in video memory from the surface.
    textTexture = SDL_CreateTextureFromSurface(mwRenderer, textSurface);
    if (textTexture) {

      // Copy it to the rendering context.
      SDL_RenderCopy(mwRenderer, textTexture, NULL, &rect);
      SDL_DestroyTexture(textTexture);  // Free video memory
    } else {
      fprintf(stderr, "Failed to create texture for \"%s\"!", thisText);
    }
    SDL_FreeSurface(textSurface);  // Free RAM.
  } else {
    fprintf(stderr, "Failed to create surface for \"%s\"!", thisText);
  }
}


void UpdateAll(void) {
  SDL_RenderPresent(mwRenderer);
}


void ClearAll(void) {
  SDL_SetRenderDrawColor(mwRenderer, 0, 0, 0, 255);
  SDL_RenderClear(mwRenderer);
}

// Set some dimensions according to orientation
void SetDims(void) {
   if (tensor_landscape_p) {
    TENSOR_WIDTH_EFF = TENSOR_HEIGHT;
    TENSOR_HEIGHT_EFF = TENSOR_WIDTH;

  } else {
    TENSOR_WIDTH_EFF = TENSOR_WIDTH;
    TENSOR_HEIGHT_EFF = TENSOR_HEIGHT;
  }

  if (!previewSurface) {
    SDL_FreeSurface(previewSurface);
  }

  previewSurface = SDL_CreateRGBSurface(0, TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF,
    32, 0, 0, 0, 0);
  if (previewSurface == NULL) {
    fprintf(stderr, "Unable to allocate a temp buffer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

void DrawRectangle(int x, int y, int w, int h, color_t color) {
  rectangleRGBA(mwRenderer, x, y, x + (w - 1), y + (h - 1),
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

void DrawBox(int x, int y, int w, int h, color_t color) {
  boxRGBA(mwRenderer, x, y, x + (w - 1), y + (h - 1),
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}


// Draw an image to the output with appropriate rotation and expansion.
// Rotation is acheived using SDL_gfx primitive rotozoom.
void DrawImage(double angle, double expansion, int aliasmode, unsigned char *fb_dst) {
  SDL_Surface *rotatedImage;
  SDL_Rect offset;

  // Rotate / scale it.
  rotatedImage = rotozoomSurface (imageSeed, angle, expansion, (aliasmode == YES) ? 1 : 0);
  if (!rotatedImage) {
    fprintf(stderr, "Error rotating image: %s\n", SDL_GetError());
    return;
  }

  // This clips the image and recenters it on the preview window.
  offset.x = 0 - (rotatedImage->w - previewSurface->w) / 2;
  offset.y = 0 - (rotatedImage->h - previewSurface->h) / 2;
  SDL_BlitSurface(rotatedImage, NULL, previewSurface, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, previewSurface);
  SDL_FreeSurface(rotatedImage);
}


// Draw the borders around the preview output.
void DrawPreviewBorder(void) {
  int w, h, w2, h2;

  // Get the border dimensions/
  w = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_WIDTH) + (PREVIEW_BORDER_THICKNESS * 2);
  h = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_HEIGHT) + (PREVIEW_BORDER_THICKNESS * 2);

  // Erase the old preview.
  DrawBox(0, 0, h, w, black);

  // Draw the new border.
  DrawRectangle(0, 0, w, h, white);

  // Draw the panel indicators
  w2 = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_WIDTH) + 1;
  h2 = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_HEIGHT) + 1;

  if (tensor_landscape_p) {
    // Landscape - Horizontal indicators.
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 0 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, gray);
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 1 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, gray);
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 2 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, gray);
  } else {
    // Portrait - Vertical indicators
    DrawRectangle(0 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, gray);
    DrawRectangle(1 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, gray);
    DrawRectangle(2 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, gray);
  }

  SDL_RenderPresent(mwRenderer);
}


// Or not.
void CellFun(moment_t *moment) {
  int x, y;
  color_t pixelColor, oldColor;

  moment->coefs.cellAutoCount++;

  for(x = 0 ; x < TENSOR_WIDTH_EFF ; x++) {
    for(y = 0 ; y < TENSOR_HEIGHT_EFF ; y++) {
      oldColor = GetPixel(x, y, moment->fb);
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2);
      pixelColor.g = oldColor.g + pixelColor.r;
      pixelColor.b = moment->coefs.cellAutoCount;
      SetPixel(x, y, pixelColor,  moment->fb);
    }
  }
}


void SaveMoments(SDL_KeyboardEvent *key, moment_t *moments, int now) {
  int thiskey = key->keysym.sym;
  FILE *fp;
  char filename[6];
  size_t count;

  if (((thiskey >= SDLK_0) && (thiskey <= SDLK_9)) ||
      ((thiskey >= SDLK_a) && (thiskey <= SDLK_z))) {

    snprintf(filename, sizeof(filename), "%s.now", SDL_GetKeyName(key->keysym.sym));
    printf( "Saving moment set to file \"%s\"\n", filename);

    fp = fopen(filename, "wb");
    if (fp == NULL) {
      perror("Failed to open file.");
      return;
    }

    count = fwrite(moments, sizeof(moment_t), MOMENT_COUNT, fp);
    printf("Wrote %i records of %lu bytes each for a total of %lu bytes.\n", (int) count, sizeof(moment_t), count * sizeof(moment_t));
    printf("fclose(fp) %s.\n", fclose(fp) == 0 ? "succeeded" : "failed");
  }
}

void LoadMoments(SDL_KeyboardEvent *key, moment_t *moments, char *fn) {
  int thiskey;
  FILE *fp;
  char filename[6] = "";
  size_t count;
  if (fn != NULL) {

    snprintf(filename, sizeof(filename), "%s", fn);
    thiskey = SDLK_0;
  } else {
    thiskey = key->keysym.sym;
    snprintf(filename, sizeof(filename), "%s.now", SDL_GetKeyName(key->keysym.sym));
  }
  if (((thiskey >= SDLK_0) && (thiskey <= SDLK_9)) ||
      ((thiskey >= SDLK_a) && (thiskey <= SDLK_z))) {

    printf( "Loading moment set from file \"%s\"\n", filename);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
      perror("Failed to open file!");
      return;
    }

    count = fread(moments, sizeof(moment_t), MOMENT_COUNT, fp);
    printf("Read %i records of %lu bytes each for a total of %lu bytes.\n", (int) count, sizeof(moment_t), count * sizeof(moment_t));
    printf("fclose(fp) %s.\n", fclose(fp) == 0 ? "succeeded" : "failed");
  }
}


void DrawSideBar(moment_t *moment) {
  int i;

  switch (moment->coefs.scrollerDir) {
    case LEFT:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
              SetPixel(TENSOR_WIDTH_EFF - 1, i, moment->coefs.fg, moment->fb);
            }
            break;

    case RIGHT:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
              SetPixel(0, i, moment->coefs.fg, moment->fb);
            }
            break;

    case UP:
      for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
              SetPixel(i, TENSOR_HEIGHT_EFF - 1, moment->coefs.fg, moment->fb);
            }
            break;

    case DOWN:
      for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
              SetPixel(i, 0, moment->coefs.fg, moment->fb);
            }
            break;


    default:
      break;
  }
}

void ClearRed(moment_t *currentMoment) {
  int i;
  for(i = 0; i < TENSOR_BYTES; i++) {
    currentMoment->fb[(i * 3) + 0] = 0;
  }
}

void ClearGreen(moment_t *currentMoment) {
  int i;
  for(i = 0; i < TENSOR_BYTES; i++) {
    currentMoment->fb[(i * 3) + 1] = 0;
  }
}

void ClearBlue(moment_t *currentMoment) {
  int i;
  for(i = 0; i < TENSOR_BYTES; i++) {
    currentMoment->fb[(i * 3) + 2] = 0;
  }
}

int min(int a, int b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

int max(int a, int b) {
  if (b < a) {
    return a;
  } else {
    return b;
  }
}

