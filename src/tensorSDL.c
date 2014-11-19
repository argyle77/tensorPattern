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
#define FORWARDS 0
#define BACKWARDS 1
#define FOREVER for(;;)
#define CYCLE_FRAME_LIMIT 1000

// Tensor output
#define USE_TENSOR
#define DEF_IMAGE "Fbyte-01.jpg"

// Preview window definitions
#define PREVIEW_PIXEL_WIDTH 10
#define PREVIEW_PIXEL_HEIGHT 10
#define PREVIEW_BORDER_THICKNESS 10
#define PREVIEW_FONT_SIZE 14
#define PREVIEW_FONT_HEIGHT 14
#define TENSOR_PREVIEW_WIDTH (TENSOR_WIDTH * PREVIEW_PIXEL_WIDTH)
#define TENSOR_PREVIEW_HEIGHT (TENSOR_WIDTH * PREVIEW_PIXEL_HEIGHT)

#define TEXT_BUFFER_SIZE 2048

// Initial Values
#define MOMENT_COUNT 10
#define INITIAL_COLOR_MULTIPLIER 1.0
#define INITIAL_DIFF_COEF 0.002
#define INITIAL_FADEOUT_DEC 1
#define INITIAL_RAND_MOD 100
#define INITIAL_RAINBOW_INC 8
#define INITIAL_EXPAND 1
#define INITIAL_FLOAT_INC 0.1
#define INITIAL_ROTATION_DELTA 1.0
#define INITIAL_ROTATION_ANGLE 0
#define INITIAL_ROTATION_ANGLE2 10

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


// Colors / palettes

// 32 bit per pixel color type - Named or array access through union.
typedef union color_t {
  struct {
    unsigned char r, g, b, a;
  };
  unsigned char rgba[4];
} color_t;

// Named color definitions
#define CD_RED        {.r = 255, .g = 0,   .b = 0,   .a = 255}
#define CD_ORANGE     {.r = 255, .g = 127, .b = 0,   .a = 255}
#define CD_YELLOW     {.r = 255, .g = 255, .b = 0,   .a = 255}
#define CD_CHARTREUSE {.r = 127, .g = 255, .b = 0,   .a = 255}
#define CD_GREEN      {.r = 0,   .g = 255, .b = 0,   .a = 255}
#define CD_AQUA       {.r = 0,   .g = 255, .b = 127, .a = 255}
#define CD_CYAN       {.r = 0,   .g = 255, .b = 255, .a = 255}
#define CD_AZURE      {.r = 0  , .g = 127, .b = 255, .a = 255}
#define CD_BLUE       {.r = 0,   .g = 0,   .b = 255, .a = 255}
#define CD_VIOLET     {.r = 127, .g = 0,   .b = 255, .a = 255}
#define CD_MAGENTA    {.r = 255, .g = 0,   .b = 255, .a = 255}
#define CD_ROSE       {.r = 255, .g = 0,   .b = 127, .a = 255}
#define CD_WHITE      {.r = 255, .g = 255, .b = 255, .a = 255}
#define CD_LTGRAY     {.r = 191, .g = 191, .b = 191, .a = 255}
#define CD_GRAY       {.r = 127, .g = 127, .b = 127, .a = 255}
#define CD_DKGRAY     {.r = 63,  .g = 63,  .b = 63,  .a = 255}
#define CD_BLACK      {.r = 0,   .g = 0,   .b = 0,   .a = 255}

// Named color constants
const color_t cRed = CD_RED;
const color_t cOrange = CD_ORANGE;
const color_t cYellow = CD_YELLOW;
const color_t cChartreuse = CD_CHARTREUSE;
const color_t cGreen = CD_GREEN;
const color_t cAqua = CD_AQUA;
const color_t cCyan = CD_CYAN;
const color_t cAzure = CD_AZURE;
const color_t cBlue = CD_BLUE;
const color_t cViolet = CD_VIOLET;
const color_t cMagenta = CD_MAGENTA;
const color_t cRose = CD_ROSE;
const color_t cWhite = CD_WHITE;
const color_t cLtGray = CD_LTGRAY;
const color_t cGray = CD_GRAY;
const color_t cDkGray = CD_DKGRAY;
const color_t cBlack = CD_BLACK;

#define PARAMETER_COLOR cAzure

// Named color palette structures and constants
typedef struct palette_t {
  char *name;
  color_t color;
} palette_t;

// If you change this, change palette too.
typedef enum color_e {
  CE_RED = 0, CE_ORANGE, CE_YELLOW, CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN,
  CE_AZURE, CE_BLUE, CE_VIOLET, CE_MAGENTA, CE_ROSE, CE_WHITE, CE_LTGRAY,
  CE_GRAY, CE_DKGRAY, CE_BLACK,
  CE_COUNT // Last.
} color_e;

// This palette order should line up with color_e enumeration values.
const palette_t namedPalette[CE_COUNT] = {
  { "red", CD_RED },
  { "orange", CD_ORANGE },
  { "yellow", CD_YELLOW },
  { "chartreuse", CD_CHARTREUSE },
  { "green", CD_GREEN },
  { "aqua", CD_AQUA },
  { "cyan", CD_CYAN },
  { "azure", CD_AZURE },
  { "blue", CD_BLUE },
  { "violet", CD_VIOLET },
  { "magenta", CD_MAGENTA },
  { "rose", CD_ROSE },
  { "white", CD_WHITE },
  { "lt gray", CD_LTGRAY },
  { "gray", CD_GRAY },
  { "dk gray", CD_DKGRAY },
  { "black", CD_BLACK }
};


// Enumerations and Strings.

// Scroller directions
typedef enum dir_e {
  DIR_UP = 0, DIR_LEFT, DIR_DOWN, DIR_RIGHT,
  DIR_COUNT // Last.
} dir_e;
const char *dirText[] = { "Up", "Left", "Down", "Right" };

// Fade mode enumerations.
typedef enum fadeModes_e{
  FM_LIMIT = 0, FM_MODULAR,
  FM_COUNT // Last.
} fadeModes_e;
const char *fadeModeText[] = { "Limited", "Modular" };

// Color plane shift modes.
typedef enum shiftModes_e {
  SM_HOLD = 0, SM_UP, SM_LEFT, SM_DOWN, SM_RIGHT,
  SM_COUNT // Last.
} shiftModes_e;
const char *shiftText[] = { "Hold", "Up", "Left", "Down", "Right" };

// Text background modes
typedef enum textStaggerMode_e {
  TS_8ROW, TS_9ROWSTAGGER, TS_10ROWBORDER, TS_FULLBG,
  TS_COUNT // Last.
} textStaggerMode_e;
char *staggerText[] = { "8 Row", "9RowStagger", "10RowBorder", "FullBG" };

// Color palette cycle modes
typedef enum colorCycleModes_e{
  CM_RGB, CM_CMY, CM_SECONDARY, CM_TERTIARY, CM_GRAY, CM_RAINBOW, CM_RANDOM,
  CM_COUNT // Last.
} colorCycleModes_e;
const char *colorCycleText[] = { "R-G-B", "C-M-Y", "Secondary", "Tertiary", "Gray", "Rainbow", "Random" };

// A way to keep track of palette size.
typedef struct cyclePalette_t {
  const color_e *palette;
  int size;
} cyclePalette_t;

// Some color palettes for cycling
const color_e paletteRGBcolors[] = { CE_RED, CE_GREEN, CE_BLUE };
const cyclePalette_t paletteRGB = { paletteRGBcolors, (sizeof(paletteRGBcolors) / sizeof(color_e)) };

const color_e paletteCMYcolors[] = { CE_CYAN, CE_MAGENTA, CE_YELLOW };
const cyclePalette_t paletteCMY = { paletteCMYcolors, (sizeof(paletteCMYcolors) / sizeof(color_e)) };

const color_e paletteSeccolors[] = { CE_RED, CE_YELLOW, CE_GREEN, CE_CYAN, CE_BLUE, CE_MAGENTA };
const cyclePalette_t paletteSec = { paletteSeccolors, (sizeof(paletteSeccolors) / sizeof(color_e)) };

const color_e paletteTercolors[] = { CE_RED, CE_ORANGE, CE_YELLOW, CE_CHARTREUSE, CE_GREEN, CE_AQUA,
  CE_CYAN, CE_AZURE, CE_BLUE, CE_VIOLET, CE_MAGENTA, CE_ROSE };
const cyclePalette_t paletteTer = { paletteTercolors, (sizeof(paletteTercolors) / sizeof(color_e)) };

const color_e paletteGrycolors[] = { CE_WHITE, CE_LTGRAY, CE_GRAY, CE_DKGRAY };
const cyclePalette_t paletteGry = { paletteGrycolors, (sizeof(paletteGrycolors) / sizeof(color_e)) };

// Types

// Pattern modes
typedef struct modes_t {
  unsigned char textSeed;
  unsigned char cellAutoFun;
  unsigned char bouncer;
  unsigned char fadeout;
  unsigned char diffuse;
  unsigned char roller;
  unsigned char scroller;
  unsigned char horizontalBars;
  unsigned char verticalBars;
  unsigned char colorAll;
  unsigned char clearAll;
  unsigned char randDots;
  unsigned char cycleForeground;
  unsigned char cycleBackground;
  fadeModes_e fadeMode;
  unsigned char postRotateZoom;
  unsigned char preRotateZoom;
  unsigned char alias;
  unsigned char multiply;
  textStaggerMode_e textStagger;
  unsigned char sidebar;
  unsigned char clearRed;
  unsigned char clearGreen;
  unsigned char clearBlue;
  shiftModes_e shiftRed;
  shiftModes_e shiftGreen;
  shiftModes_e shiftBlue;
  shiftModes_e shiftCyan;
  shiftModes_e shiftYellow;
  shiftModes_e shiftMagenta;
  unsigned char postImage;
  unsigned char fontFlip;
  unsigned char fontDirection;
} modes_t;

// Parameters
typedef struct parms_t{
  color_e foreground;
  color_e background;
  color_t fg;
  color_t bg;
  int fadeout_dec;
  float diffusion_coef;
  dir_e scrollerDir;
  int randMod;
  colorCycleModes_e colorCycleMode;
  int rainbowInc;
  float expand;
  float floatinc;
  float rotation;
  float rotation2;
  float rotationDelta;
  float colorMultiplier;
  int cycleSaveForegound;
  int cycleSaveBackground;
  int cellAutoCount;
  int textOffset;
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

// Display menu items
typedef struct displayText_t {
  int line;
  int col;
  color_t color;
  char *text;
} displayText_t;

const displayText_t displayText[] = {
  {21, 0, CD_YELLOW,  "Modes:                       FPS:"},
  {23, 0, CD_LTGRAY, "<ctrl> q - Cell pattern"},
  {24, 0, CD_LTGRAY, "<ctrl> w - Bouncer"},
  {25, 0, CD_LTGRAY, "<ctrl> e - Fader"},
  {26, 0, CD_LTGRAY, "<ctrl> r - Diffuser"},
  {27, 0, CD_LTGRAY, "<ctrl> t - Text seed"},
  {28, 0, CD_LTGRAY, "<ctrl> y - Scroll rollover"},
  {29, 0, CD_LTGRAY, "<ctrl> u - Scroller"},
  {30, 0, CD_LTGRAY, "<ctrl> i - Horizontal bars!"},
  {31, 0, CD_LTGRAY, "<ctrl> o - Vertical bars!"},
  {32, 0, CD_LTGRAY, "<crtl> p - Foreground color all!"},
  {33, 0, CD_LTGRAY, "<crtl> a - Background color all!"},
  {34, 0, CD_LTGRAY, "<ctrl> s - Random dot seed"},
  {35, 0, CD_LTGRAY, "<ctrl> d - FG cycle"},
  {36, 0, CD_LTGRAY, "<ctrl> f - BG cycle"},
  {37, 0, CD_LTGRAY, "<ctrl> g - Cycle moments"},
  {38, 0, CD_LTGRAY, "<ctrl> h - Fader mode"},
  {39, 0, CD_LTGRAY, "<ctrl> j - Reset text to start"},
  {40, 0, CD_LTGRAY, "<ctrl> k - Post-rotate/zoom"},
  {41, 0, CD_LTGRAY, "<ctrl> l - All modes off"},
  {42, 0, CD_LTGRAY, "<ctrl> z - Pre-rotate/zoom"},
  {43, 0, CD_LTGRAY, "<ctrl> x - Rotate/zoom anti-alias"},
  {44, 0, CD_LTGRAY, "<ctrl> c - Multiplier"},
  {45, 0, CD_LTGRAY, "<ctrl> n - Sidebar seed"},
  {46, 0, CD_LTGRAY, "<ctrl> m - Supress red"},
  {47, 0, CD_LTGRAY, "<ctrl> , - Supress green"},
  {48, 0, CD_LTGRAY, "<ctrl> . - Supress blue"},
  {49, 0, CD_LTGRAY, "<ctrl> <alt> p - Image overlay"},
  {51, 0, CD_YELLOW,  "Text buffer:"},
  {0, 2,  CD_YELLOW,  "Coeffs: (3 keys = increment / reset / decrement)"},
  {2, 2, CD_LTGRAY, "<alt> m  ,  . - Float step size"},
  {4, 2, CD_LTGRAY, "<alt> q  w  e - Diffusion"},
  {5, 2, CD_LTGRAY, "<alt> a  s  d - Expansion"},
  {6, 2, CD_LTGRAY, "<alt> z  x  c - Fade amount"},
  {7, 2, CD_LTGRAY, "<alt> r  t  y - Pre rotation angle"},
  {8, 2, CD_LTGRAY, "<alt> f  g  h - Rainbow Speed"},
  {9, 2, CD_LTGRAY, "<alt> v  b  n - Post rotation angle"},
  {10, 2, CD_LTGRAY, "<alt> u  i  o - Multiplier"},
  {11, 2, CD_LTGRAY, "<alt> j  k  l - Post rotation incr"},
  {12, 2, CD_LTGRAY, "<alt> p  [  ] - Dot randomness"},
  {13, 2, CD_LTGRAY, "<alt> ;       - Color cycle mode"},
  {14, 2, CD_LTGRAY, "(<alt>) <arrows> - Scroll direction"},
  {15, 2, CD_LTGRAY, "<ctrl> <alt> u i o - Frame delay(ms)"},
  {18, 2, CD_YELLOW, "Colors:"},
  {20, 2, CD_LTGRAY, "<ctrl> <alt> q w - foreground:"},
  {21, 2, CD_LTGRAY, "<ctrl> <alt> a s - background:"},
  {24, 2, CD_YELLOW, "Moments:"},
  {26, 2, CD_LTGRAY, "<ctrl> 0-9 - Load Moment.    Current:"},
  {27, 2, CD_LTGRAY, "<ctrl> <alt> 0-9 - Copy moment over current."},
  {28, 2, CD_LTGRAY, "<alt> 0-9 - Load another moment's image buffer."},
  {31, 2, CD_YELLOW, "Text entry:"},
  {32, 2, CD_LTGRAY, "<Unmodified keys> - Add text."},
  {33, 2, CD_LTGRAY, "<backspace> - Delete last letter."},
  {34, 2, CD_LTGRAY, "<delete> - Erase all text."},
  {35, 2, CD_LTGRAY, "<ctrl> ' - Reverse text."},
  {36, 2, CD_LTGRAY, "<ctrl> ; - Flip text."},
  {37, 2, CD_LTGRAY, "<crtl> <alt> z x - Offset:"},
  {38, 2, CD_LTGRAY, "<ctrl> b - Text mode:"},
  {39, 2, CD_LTGRAY, "Text buffer size:"},
  {42, 2, CD_YELLOW, "More modes:"},
  {44, 2, CD_LTGRAY, "<ctrl> [ - Red plane:"},
  {45, 2, CD_LTGRAY, "<ctrl> ] - Green plane:"},
  {46, 2, CD_LTGRAY, "<ctrl> \\ - Blue plane:"},
  {47, 2, CD_LTGRAY, "<ctrl> <alt> [ - Cyan plane:"},
  {48, 2, CD_LTGRAY, "<ctrl> <alt> ] - Yellow plane:"},
  {49, 2, CD_LTGRAY, "<ctrl> <alt> \\ - Magenta plane:"},
  {51, 2, CD_YELLOW, "<esc> - Quit."}
};
#define DISPLAY_TEXT_SIZE (sizeof(displayText) / sizeof(displayText_t))



// Globals - We do love our globals.
TTF_Font *screenFont;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;
SDL_Surface *previewSurface = NULL;
SDL_Surface *imageSeed = NULL;
int TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF;
int cycleMomentsMode = NO;  // cycleMomentsMode mode is a global (for now).
moment_t moments[MOMENT_COUNT];
int now;
moment_t *currentMoment;
modes_t *currentMode;
parms_t *currentParms;
text_info_t *currentText;
unsigned char *currentFB;
float global_intensity_limit;
int frames, fps;

// Prototypes
void MainLoop(void);
void ProcessModes(void);
Uint32 TriggerProcessing(Uint32 interval, void *param);
Uint32 FrameCounter(Uint32 interval, void *param);
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int dec, fadeModes_e fadeMode, unsigned char *buffer);
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane);
void WriteSlice(moment_t *moment);
void CellFun(moment_t *moment);
void DrawSideBar(moment_t *moment);
void Diffuse(float diffusion_coeff, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
int HandleInput(SDL_Event *key_event);
int ColorAlter(int thiskey, moment_t *moments,int now);
int ModeAlter(int thiskey, moment_t *moments, int now);
void ParmAlter(int thiskey, moment_t *moments, int now);
void SaveMoments(SDL_KeyboardEvent *key, moment_t *moments, int now);
void LoadMoments(SDL_KeyboardEvent *key, moment_t *moments, char *fn);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
void ColorCycle(parms_t *parms, int fb_mode);
void ColorAll(color_t color, unsigned char *fb);
void SetDims(void);
void Update(float intensity_limit, moment_t *moment);
void UpdateAll(void);
void UpdateTensor(unsigned char *buffer);
void DrawPreviewBorder(void);
void UpdatePreview(unsigned char *buffer);
void InitInfoDisplay(void);
void UpdateInfoDisplay(moment_t *moment, int now);
void ClearAll(void);
void WriteLine(char * thisText, int line, int col, color_t color);
void WriteBool(int value, int row, int col);
void WriteInt(int value, int row, int col, int width);
void WriteFloat(float value, int row, int col, int width, int precision);
void WriteString(const char *text, int line, int col, int width);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src);
void Multiply(float multiplier, unsigned char *buffer);
void DrawRectangle(int x, int y, int w, int h, color_t color);
void DrawBox(int x, int y, int w, int h, color_t color);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void ClearRed(moment_t *currentMoment);
void ClearGreen(moment_t *currentMoment);
void ClearBlue(moment_t *currentMoment);
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void DrawImage(double angle, double expansion, int aliasmode, unsigned char *fb_dst);
int min(int a, int b);
int max(int a, int b);


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
  atexit(SDL_Quit);  // Register the SDL cleanup function.

  // Initialize the SDL window and renderer.
  SDL_CreateWindowAndRenderer(900, 768, SDL_WINDOW_RESIZABLE, &mainWindow, &mwRenderer);
  if ((!mainWindow) || (!mwRenderer)) {
    fprintf(stderr, "Unable to create main window or renderer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Window scaling hints:
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
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

  // Load the font.  Should be a fixed width font.
  if(TTF_Init() < 0) {
    printf("Couldn't initialize TTF: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  } else {
    screenFont = TTF_OpenFont("font.ttf", PREVIEW_FONT_SIZE);
    if(!screenFont) {
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
  currentMode->fadeMode = FM_LIMIT;
  currentMode->fadeout = NO;
  currentMode->horizontalBars = NO;
  currentMode->multiply = NO;
  currentMode->randDots = NO;
  currentMode->roller = NO;
  currentMode->postRotateZoom = NO;
  currentMode->preRotateZoom = NO;
  currentMode->scroller = YES;
  currentMode->textSeed = YES;
  currentMode->verticalBars = NO;
  currentMode->textStagger = TS_FULLBG;
  currentMode->sidebar = NO;
  currentMode->clearBlue = NO;
  currentMode->clearGreen = NO;
  currentMode->clearRed = NO;
  currentMode->shiftRed = SM_HOLD;
  currentMode->shiftGreen = SM_HOLD;
  currentMode->shiftBlue = SM_HOLD;
  currentMode->shiftCyan = SM_HOLD;
  currentMode->shiftMagenta = SM_HOLD;
  currentMode->shiftYellow = SM_HOLD;
  currentMode->postImage = NO;
  currentMode->fontFlip = NO;
  currentMode->fontDirection = FORWARDS;

  currentParms->background = CE_BLACK;
  currentParms->bg = namedPalette[currentParms->background].color;
  currentParms->colorCycleMode = CM_RAINBOW;
  currentParms->colorMultiplier = INITIAL_COLOR_MULTIPLIER;
  currentParms->cycleSaveForegound = 0;
  currentParms->cycleSaveBackground = 0;
  currentParms->diffusion_coef = INITIAL_DIFF_COEF;
  currentParms->expand = INITIAL_EXPAND;
  currentParms->fadeout_dec = INITIAL_FADEOUT_DEC;
  currentParms->floatinc = INITIAL_FLOAT_INC;
  currentParms->foreground = CE_RED;
  currentParms->fg = namedPalette[currentParms->foreground].color;
  currentParms->rainbowInc = INITIAL_RAINBOW_INC;
  currentParms->randMod = INITIAL_RAND_MOD;
  currentParms->rotation = INITIAL_ROTATION_ANGLE;
  currentParms->rotation2 = INITIAL_ROTATION_ANGLE2;
  currentParms->rotationDelta = INITIAL_ROTATION_DELTA;
  currentParms->scrollerDir = DIR_LEFT;
  currentParms->cellAutoCount = 0;
  currentParms->textOffset = TENSOR_HEIGHT_EFF / 3 - 1;
  currentParms->delay = 60;

  snprintf(currentText->textBuffer, TEXT_BUFFER_SIZE, "Frostbyte was an engineer. ");
  currentText->tindex = strlen(currentText->textBuffer);
  currentText->imaginaryIndex = -1;
  currentText->lastDirection = DIR_LEFT;
  currentText->textBufferRand[0] = 'a';  // Initializing value.

  ColorAll(cBlack, currentFB);

  // There is a secret hidden bug in the moment copying or changing or disk read / write
  // that I haven't found yet...
  for (i = 1; i < MOMENT_COUNT; i++) {
    memcpy(&moments[i], currentMoment, sizeof(moment_t));
  }

  // Attempt to load a file...
  LoadMoments(NULL, &moments[0], "1.now");

  // Bam!
  ColorAll(cBlack, currentFB);
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
          if (!HandleInput(&event)) {
            exitProgram = 1;
          }
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
         if ((event.window.event == SDL_WINDOWEVENT_RESIZED) ||
             (event.window.event == SDL_WINDOWEVENT_EXPOSED)) {
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
    } else {
      // Large delays make the CPU go way up.  Its because were stuck in a
      // tight loop, polling for events that never occur.  Best to get out of
      // the way...
      if (currentParms->delay > 30) {
        nanosleep((struct timespec[]) {{0,1000000}}, NULL);  // 1ms.
      }
    }

    if (exitProgram) {
      break;
    }
  }  // End program loop

  // Cleanup.
  TTF_CloseFont(screenFont);
  screenFont = NULL; // to be safe...?
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
  static int cycleFrameCount = 0;    // Frame Count.
  static dir_e scrDir = DIR_UP;

  // Random mode isn't random.  It cycles through the moments.  It allows you
  // to set up a bunch of pattern sets and switch between them one at a time at
  // some interval.
  if (cycleMomentsMode) {
    cycleFrameCount++;
    if (cycleFrameCount >= CYCLE_FRAME_LIMIT) {
      cycleFrameCount = 0;

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
  if (currentMode->cycleForeground) {
    ColorCycle(currentParms, FOREGROUND);
  }

  // Change background color.
  if (currentMode->cycleBackground) {
    ColorCycle(currentParms, BACKGROUND);
  }

  // Seed the entire array with the foreground color.
  if (currentMode->colorAll) {
    ColorAll(currentParms->fg, currentFB);
    currentMode->colorAll = NO;
  }

  // Scroller.
  if (currentMode->scroller) {
    Scroll(currentParms->scrollerDir, currentMode->roller, currentFB, PLANE_ALL);
  }

  // Scroll red.
  if (currentMode->shiftRed) {
    Scroll(currentMode->shiftRed - 1, currentMode->roller, currentFB, PLANE_RED);
  }

  // Scroll green.
  if (currentMode->shiftGreen) {
    Scroll(currentMode->shiftGreen - 1, currentMode->roller, currentFB, PLANE_GREEN);
  }

  // Scroll blue.
  if (currentMode->shiftBlue) {
    Scroll(currentMode->shiftBlue - 1, currentMode->roller, currentFB, PLANE_BLUE);
  }

  // Scroll blue and green
  if (currentMode->shiftCyan) {
      Scroll(currentMode->shiftCyan - 1, currentMode->roller, currentFB, PLANE_CYAN);
  }

  // Scroll red and blue.
  if (currentMode->shiftMagenta) {
    Scroll(currentMode->shiftMagenta - 1, currentMode->roller, currentFB, PLANE_MAGENTA);
  }

  // Scroll green and red.
  if (currentMode->shiftYellow) {
    Scroll(currentMode->shiftYellow - 1, currentMode->roller, currentFB, PLANE_YELLOW);
  }

  // Draw a solid bar up the side we are scrolling from.
  if (currentMode->sidebar) {
    DrawSideBar(currentMoment);
  }

  // First stab experimental image drawing.  Needs work.
  if (currentMode->postImage) {
    DrawImage(currentParms->rotation, currentParms->expand, currentMode->alias, currentFB);
    currentParms->rotation = currentParms->rotation + currentParms->rotationDelta;
  }

  // I think this just writes a column of text on the right or left.
  if (currentMode->textSeed) {
    // Scroll provided by scroller if its on.
    WriteSlice(currentMoment);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (currentMode->cellAutoFun) {
    // Give each pixel a color value.
    CellFun(currentMoment);
  }

  // Bouncy bouncy (ick).
  if (currentMode->bouncer) {
    scrDir = (scrDir + 1) % 4;
    Scroll(scrDir, currentMode->roller, currentFB, PLANE_ALL);
  }

  // Bam!  Draw some horizontal bars.
  if (currentMode->horizontalBars) {
    HorizontalBars(currentParms->fg, currentFB);
    currentMode->horizontalBars = NO;
  }

  // Bam!
  if (currentMode->verticalBars) {
    VerticalBars(currentParms->fg, currentFB);
    currentMode->verticalBars = NO;
  }

  // Random dots.  Most useful seed ever.
  if (currentMode->randDots) {
    RandomDots(currentParms->fg, currentParms->randMod, currentFB);
  }

  // Fader
  if (currentMode->fadeout) {
    FadeAll(currentParms->fadeout_dec, currentMode->fadeMode, currentFB);
  }

  // Averager
  if (currentMode->diffuse) {
    Diffuse(currentParms->diffusion_coef, currentFB);
  }

  // Clear screen.
  if (currentMode->clearAll) {
    ColorAll(currentParms->bg, currentFB);
    currentMode->clearAll = NO;
  }

  // Multiplier
  if (currentMode->multiply) {
    Multiply(currentParms->colorMultiplier, currentFB);
  }

  // Experimental Rotozoomer
  if (currentMode->preRotateZoom) {
    Rotate(currentParms->rotation2, currentParms->expand, currentMode->alias, currentFB, currentFB);
  }

  // Zero the red.
  if (currentMode->clearRed) {
    ClearRed(currentMoment);
  }

  // Zero the blue.
  if (currentMode->clearBlue) {
    ClearBlue(currentMoment);
  }

  // Zero the green.
  if (currentMode->clearGreen) {
    ClearGreen(currentMoment);
  }
}

// Key press processing.
int HandleInput(SDL_Event *key_event) {

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
    // Check for arrows here.
    switch (key_event->key.keysym.sym) {
      case SDLK_UP:
        currentParms->scrollerDir = DIR_UP;
        break;

      case SDLK_DOWN:
        currentParms->scrollerDir = DIR_DOWN;
        break;

      case SDLK_LEFT:
        currentParms->scrollerDir = DIR_LEFT;
        break;

      case SDLK_RIGHT:
        currentParms->scrollerDir = DIR_RIGHT;
        break;
      default:
        break;
    }
    if (key_event->key.keysym.sym < 0x80 && key_event->key.keysym.sym > 0) {
      switch(key_event->key.keysym.sym) {
        case SDLK_ESCAPE:
          // This will exit the program.
          return 0;
          break;

        case SDLK_BACKSPACE:
          // Reduce the buffer by 1 character off the end.
          currentText->tindex--;
          if (currentText->tindex < 0) {
            currentText->tindex = 0;
          }
          currentText->textBuffer[currentText->tindex] = 0x00;
          break;

        case SDLK_RETURN:
          // Um, place a carriage return in the buffer?
          currentText->textBuffer[currentText->tindex] = '\n';
          currentText->textBuffer[currentText->tindex + 1] = 0x00;
          currentText->tindex++;
          if (currentText->tindex >= (sizeof(currentText->textBuffer) - 2)) {
            currentText->tindex--;
          }
          break;

        case SDLK_DELETE:
          // Erase the entire buffer by settings its size to 0 and terminating it.
          currentText->tindex = 0;
          currentText->textBuffer[0] = 0x00;
          break;

        default:
          // Place the pressed key into the text buffer.
          if (key_event->key.keysym.mod & KMOD_SHIFT) {
            // Shift was pushed.
            if (key_event->key.keysym.sym <= 122 && key_event->key.keysym.sym >= 97) {
              // Capitalize for a - z.
              currentText->textBuffer[currentText->tindex] = (char)key_event->key.keysym.sym - 32;
            } else {
              // Lookup the symbols for the rest of the keys.
              switch (key_event->key.keysym.sym) {
                case SDLK_1:
                  currentText->textBuffer[currentText->tindex] = '!';
                  break;
                case SDLK_2:
                  currentText->textBuffer[currentText->tindex] = '@';
                  break;
                case SDLK_3:
                  currentText->textBuffer[currentText->tindex] = '#';
                  break;
                case SDLK_4:
                  currentText->textBuffer[currentText->tindex] = '$';
                  break;
                case SDLK_5:
                  currentText->textBuffer[currentText->tindex] = '%';
                  break;
                case SDLK_6:
                  currentText->textBuffer[currentText->tindex] = '^';
                  break;
                case SDLK_7:
                  currentText->textBuffer[currentText->tindex] = '&';
                  break;
                case SDLK_8:
                  currentText->textBuffer[currentText->tindex] = '*';
                  break;
                case SDLK_9:
                  currentText->textBuffer[currentText->tindex] = '(';
                  break;
                case SDLK_0:
                  currentText->textBuffer[currentText->tindex] = ')';
                  break;
                case SDLK_BACKSLASH:
                  currentText->textBuffer[currentText->tindex] = '|';
                  break;
                case SDLK_BACKQUOTE:
                  currentText->textBuffer[currentText->tindex] = '~';
                  break;
                case SDLK_MINUS:
                  currentText->textBuffer[currentText->tindex] = '_';
                  break;
                case SDLK_EQUALS:
                  currentText->textBuffer[currentText->tindex] = '+';
                  break;
                case SDLK_LEFTBRACKET:
                  currentText->textBuffer[currentText->tindex] = '{';
                  break;
                case SDLK_RIGHTBRACKET:
                  currentText->textBuffer[currentText->tindex] = '}';
                  break;
                case SDLK_SEMICOLON:
                  currentText->textBuffer[currentText->tindex] = ':';
                  break;
                case SDLK_COMMA:
                  currentText->textBuffer[currentText->tindex] = '<';
                  break;
                case SDLK_PERIOD:
                  currentText->textBuffer[currentText->tindex] = '>';
                  break;
                case SDLK_SLASH:
                  currentText->textBuffer[currentText->tindex] = '?';
                  break;
                case SDLK_QUOTE:
                  currentText->textBuffer[currentText->tindex] = '"';
                  break;
                default:
                  break;
              }
            }
          } else {
            // Unmodified key entry.
            currentText->textBuffer[currentText->tindex] = (char)key_event->key.keysym.sym;
          }
          // Advance the terminating null and increase the buffer size.
          currentText->textBuffer[currentText->tindex + 1] = 0x00;
          currentText->tindex++;

          // But not too much.
          if (currentText->tindex >= (sizeof(currentText->textBuffer) - 2)) {
            currentText->tindex--;
          }
          break;  // End the default case (most keys).
      } // End normal key switch.
    }  // End normal key else.
  }  // End elses between modifier keys.
  return 1;
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
void FadeAll(int dec, fadeModes_e fadeMode, unsigned char *buffer) {
  int x, y, r, g, b;
  color_t colorTemp;

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {

      // Grab the pixel color.
      colorTemp = GetPixel(x, y, buffer);

      if (fadeMode == FM_MODULAR) {
        // Modular color fade is easy regardless of dec sign.
        // Unsigned chars rollover.
        colorTemp.r -= dec;
        colorTemp.g -= dec;
        colorTemp.b -= dec;
      } else {
        // For the limiter, we'll do the math with ints.
        r = colorTemp.r;
        g = colorTemp.g;
        b = colorTemp.b;
        r -= dec;
        g -= dec;
        b -= dec;
        if ( r < 0 ) r = 0;
        if ( g < 0 ) g = 0;
        if ( b < 0 ) b = 0;
        if ( r > 255 ) r = 255;
        if ( g > 255 ) g = 255;
        if ( b > 255 ) b = 255;
        colorTemp.r = r;
        colorTemp.g = g;
        colorTemp.b = b;
      }

      SetPixel(x, y, colorTemp, buffer);
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
  if (mode->postRotateZoom) {
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
  rotatedSurface = rotozoomSurface (previewSurface, angle, expansion, aliasmode ? 1 : 0);
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
      case DIR_UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, 0, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          rollSave[i] = GetPixel(i, TENSOR_HEIGHT_EFF - 1, fb);
        }
        break;

      case DIR_RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          rollSave[i] = GetPixel(TENSOR_WIDTH_EFF - 1, i, fb);
        }
      break;

      case DIR_LEFT:
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
    case DIR_UP:
      for (y = 0; y < (TENSOR_HEIGHT_EFF - 1); y++) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y+1, fb), plane, fb);
        }
      }
      break;

    case DIR_DOWN:
      for (y = (TENSOR_HEIGHT_EFF - 1); y > 0; y--) {
        for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y - 1, fb), plane, fb);
        }
      }
      break;

    case DIR_LEFT:
      for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
        for (x = 0; x < (TENSOR_WIDTH_EFF - 1); x++) {
          SetPixelByPlane(x, y, GetPixel(x + 1, y, fb),plane, fb);
        }
      }
      break;

    case DIR_RIGHT:
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
      case DIR_UP:
        for(i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixelByPlane(i, TENSOR_HEIGHT_EFF - 1, rollSave[i], plane, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixelByPlane(i, 0, rollSave[i],plane, fb);
        }
        break;

      case DIR_RIGHT:
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixelByPlane(0, i, rollSave[i],plane, fb);
        }
      break;

      case DIR_LEFT:
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
  const unsigned char charColMasks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  int sliceColOrRow;  // The seed target column / row (depending on direction).
  int sliceIndex;     // A pixel of the target slice we are working on.
  char character;     // The current character being output.
  int imaginaryBufferSize;  // Number of pixel columns in the text buffer.
  int fontPixelIndex;  // Index into the font to a selected pixel.
  int bufferIndex;  // Index into the text buffer of the current character.
  int charCol;  // Index into the font pixels column of the current character.
  int i, j;
  int useRand;  // The random number used during staggering (if at all).
  unsigned char horizontal;
  unsigned char pixelOn;
  unsigned char textDirection;

  // Figure out which row or column to place the seed slice into depending on direction.
  horizontal = YES;
  switch (moment->coefs.scrollerDir) {
    case DIR_LEFT:
      sliceColOrRow = TENSOR_WIDTH_EFF - 1;
      break;

    case DIR_UP:
      horizontal = NO;
      sliceColOrRow = TENSOR_HEIGHT_EFF - 1;
      break;

    case DIR_DOWN:
      horizontal = NO;
      // Fallthrough...
    case DIR_RIGHT:
    default:
      sliceColOrRow = 0;
      break;
  }

  // Initialize textBufferRand, if necessary.  This is used for 9 row stagger,
  // which is meant for an 8 pixel font height on a 9 pixel high display.
  if (moment->text.textBufferRand[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      moment->text.textBufferRand[i] = rand() % 2;
    }
  }

  // The imaginary buffer correspends to the real text buffer, but further
  // subdivides each array index into a part for every pixel of the text width.
  imaginaryBufferSize = strlen(moment->text.textBuffer) * FONT_WIDTH;

  // Trivially, no text in the buffer.
  if (imaginaryBufferSize == 0) {
    // Just exit.  This actually causes the background to stop scrolling, which
    // might not be desireable.
    return;
  }

  // Prior to writing out a line, we increment the imaginary buffer index to
  // point to the next part of the line to be written.  This depends on
  // scroll direction, text direction.
  textDirection = moment->coefs.scrollerDir;
  if (moment->mode.fontDirection == BACKWARDS) {
    switch (textDirection) {
      case DIR_LEFT:
        textDirection = DIR_RIGHT;
        break;
      case DIR_RIGHT:
        textDirection = DIR_LEFT;
        break;
      case DIR_DOWN:
        textDirection = DIR_UP;
        break;
      case DIR_UP:
        textDirection = DIR_DOWN;
        break;
      default:
        break;
    }
  }
  if (moment->text.lastDirection != moment->coefs.scrollerDir) {

    // Looks like we changed direction.
    moment->text.lastDirection = moment->coefs.scrollerDir;

    // TENSOR_WIDTH_EFF is used here to preserve continuity of the text, even
    // across text buffer wrap-around, when the direction changes (but only
    // really between UP - DOWN or RIGHT - LEFT changes).
    switch(textDirection) {
      case DIR_LEFT:
        moment->text.imaginaryIndex = (moment->text.imaginaryIndex + TENSOR_WIDTH_EFF) % imaginaryBufferSize;
        break;
      case DIR_RIGHT:
        moment->text.imaginaryIndex = moment->text.imaginaryIndex - TENSOR_WIDTH_EFF;
        if (moment->text.imaginaryIndex < 0) {
          moment->text.imaginaryIndex = imaginaryBufferSize + moment->text.imaginaryIndex;
        }
        break;
      case DIR_DOWN:
        moment->text.imaginaryIndex = (moment->text.imaginaryIndex + TENSOR_HEIGHT_EFF) % imaginaryBufferSize;
        break;
      case DIR_UP:
        moment->text.imaginaryIndex = moment->text.imaginaryIndex - TENSOR_HEIGHT_EFF;
        if (moment->text.imaginaryIndex < 0) {
          moment->text.imaginaryIndex = imaginaryBufferSize + moment->text.imaginaryIndex;
        }
        break;
      default:
        break;
    }
  } else {
    // No change in direction, so we increment without worrying about continuity
    // preservation.
    switch(textDirection) {
      case DIR_LEFT:
      case DIR_DOWN:
        moment->text.imaginaryIndex = (moment->text.imaginaryIndex + 1) % imaginaryBufferSize;
        break;
      case DIR_RIGHT:
      case DIR_UP:
        moment->text.imaginaryIndex--;
        if ((moment->text.imaginaryIndex < 0) || (moment->text.imaginaryIndex >= imaginaryBufferSize)) {
          moment->text.imaginaryIndex = imaginaryBufferSize - 1;
        }
        break;
      default:
        break;
    }
  }

  // Now using the imaginary index, we find out where in the real buffer we are.
  // (Integer division.)
  bufferIndex = moment->text.imaginaryIndex / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (moment->text.imaginaryIndex % FONT_WIDTH);

  // What the character is.  What it is, man.
  character = moment->text.textBuffer[bufferIndex];

  // First we draw the seed's background according to our textStagger mode.
  useRand = 0;
  switch (moment->mode.textStagger) {
    case TS_9ROWSTAGGER:
      // Stagger.  For landscape - 8 pixel font on 9 pixels high display.
      // Stagger the letter and fill in the pixel that isn't covered by the
      // letter with our background.
      sliceIndex = moment->coefs.textOffset;
      if (!moment->text.textBufferRand[bufferIndex]) sliceIndex += 8;
      if (horizontal) {
        sliceIndex = sliceIndex % TENSOR_HEIGHT_EFF;
        SetPixelA(sliceColOrRow, sliceIndex, moment->coefs.bg, moment->fb);
      } else {
        sliceIndex = sliceIndex % TENSOR_WIDTH_EFF;
        SetPixelA(sliceIndex, sliceColOrRow, moment->coefs.bg, moment->fb);
      }
      useRand = moment->text.textBufferRand[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = moment->coefs.textOffset - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = TENSOR_HEIGHT_EFF - 1;
        sliceIndex %= TENSOR_HEIGHT_EFF;
        SetPixelA(sliceColOrRow, sliceIndex, moment->coefs.bg, moment->fb);
      } else {
        if (sliceIndex < 0) sliceIndex = TENSOR_WIDTH_EFF - 1;
        sliceIndex %= TENSOR_WIDTH_EFF;
        SetPixelA(sliceIndex, sliceColOrRow, moment->coefs.bg, moment->fb);
      }
      sliceIndex = moment->coefs.textOffset + 8;
      if (horizontal) {
        sliceIndex %= TENSOR_HEIGHT_EFF;
        SetPixelA(sliceColOrRow, sliceIndex, moment->coefs.bg, moment->fb);
      } else {
        sliceIndex %= TENSOR_WIDTH_EFF;
        SetPixelA(sliceIndex, sliceColOrRow, moment->coefs.bg, moment->fb);
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
          SetPixelA(sliceColOrRow, i, moment->coefs.bg, moment->fb);
        }
      } else {
        for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
          SetPixelA(i, sliceColOrRow, moment->coefs.bg, moment->fb);
        }
      }
      break;

    default:
      // textStagger == TS_8ROW, which means no border, no stagger, no extra
      // background to draw, so do nothing.
      break;
  }

  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    if (moment->mode.fontFlip) {
      j = (FONT_HEIGHT - 1) - i;
    } else {
      j = i;
    }
    fontPixelIndex = (j * FONT_CHARACTER_COUNT) + character;
    sliceIndex = i + useRand + moment->coefs.textOffset;
    pixelOn = myfont[fontPixelIndex] & charColMasks[charCol];
    if (horizontal) {
      sliceIndex = sliceIndex % TENSOR_HEIGHT_EFF;
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? moment->coefs.fg : moment->coefs.bg, moment->fb);
    } else {
      sliceIndex = sliceIndex % TENSOR_WIDTH_EFF;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? moment->coefs.fg : moment->coefs.bg, moment->fb);
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
      parms->foreground--;
      if (parms->foreground < 0 || parms->foreground >= CE_COUNT) {
        parms->foreground = CE_COUNT - 1;
      }
      parms->fg = namedPalette[parms->foreground].color;
      break;

    case SDLK_w:
      parms->foreground++;
      if (parms->foreground >= CE_COUNT) {
        parms->foreground = 0;
      }
      parms->fg = namedPalette[parms->foreground].color;
      break;

    case SDLK_a:
      parms->background--;
      if (parms->background < 0 || parms->background >= CE_COUNT) {
        parms->background = CE_COUNT - 1;
      }
      parms->bg = namedPalette[parms->background].color;
      break;

    case SDLK_s:
      parms->background++;
      if (parms->background >= CE_COUNT) {
        parms->background = 0;
      }
      parms->bg = namedPalette[parms->background].color;
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      memcpy(&moments[thiskey - SDLK_0], moment, sizeof(moment_t));
      printf("Current moment (%i) saved in box %i\n", now, thiskey - SDLK_0);
      break;

    case SDLK_z:
      parms->textOffset--;
      if (parms->textOffset < 0) {
        if ((parms->scrollerDir == DIR_LEFT) || (parms->scrollerDir == DIR_RIGHT)) {
          parms->textOffset = TENSOR_HEIGHT_EFF - 1;
        } else {
          parms->textOffset = TENSOR_WIDTH_EFF - 1;
        }
      }
      break;

    case SDLK_x:
      parms->textOffset++;
      if ((parms->scrollerDir == DIR_LEFT) || (parms->scrollerDir == DIR_RIGHT)) {
        if (parms->textOffset >= TENSOR_HEIGHT_EFF) {
          parms->textOffset = 0;
        }
      } else {
        if (parms->textOffset >= TENSOR_WIDTH_EFF) {
          parms->textOffset = 0;
        }
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
      mode->shiftCyan = (mode->shiftCyan + 1) % SM_COUNT;
      break;

    case SDLK_RIGHTBRACKET:
      mode->shiftYellow = (mode->shiftYellow + 1) % SM_COUNT;
      break;

    case SDLK_BACKSLASH:
      mode->shiftMagenta = (mode->shiftMagenta + 1) % SM_COUNT;
      break;

    case SDLK_p:
      mode->postImage = !mode->postImage;
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

    case SDLK_SEMICOLON:
      mode->fontFlip = !mode->fontFlip;
      break;

    case SDLK_QUOTE:
      mode->fontDirection = !mode->fontDirection;
      break;

    case SDLK_t:
      mode->textSeed = !mode->textSeed;
      break;

    case SDLK_q:
      mode->cellAutoFun = !mode->cellAutoFun;
      break;

    case SDLK_w:
      mode->bouncer = !mode->bouncer;
      break;

    case SDLK_e:
      mode->fadeout = !mode->fadeout;
      break;

    case SDLK_r:
      mode->diffuse = !mode->diffuse;
      break;

    case SDLK_y:
      mode->roller = !mode->roller;
      break;

    case SDLK_u:
      mode->scroller = !mode->scroller;
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
      mode->randDots = !mode->randDots;
      break;

    case SDLK_d:
      mode->cycleForeground = !mode->cycleForeground;
      break;

    case SDLK_f:
      mode->cycleBackground = !mode->cycleBackground;
      break;

    case SDLK_g:
      cycleMomentsMode = !cycleMomentsMode;
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
      mode->fadeMode = FM_LIMIT;
      mode->fadeout = NO;
      mode->horizontalBars = NO;
      mode->multiply = NO;
      mode->randDots = NO;
      mode->roller = NO;
      mode->postRotateZoom = NO;
      mode->preRotateZoom = NO;
      mode->scroller = NO;
      mode->textSeed = NO;
      mode->verticalBars = NO;
      mode->sidebar = NO;
      mode->clearRed = NO;
      mode->clearBlue = NO;
      mode->clearGreen = NO;
      mode->shiftBlue = SM_HOLD;
      mode->shiftCyan = SM_HOLD;
      mode->shiftGreen = SM_HOLD;
      mode->shiftMagenta = SM_HOLD;
      mode->shiftYellow = SM_HOLD;
      mode->shiftRed = SM_HOLD;
      break;

    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
    case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
      now = thiskey - SDLK_0;
      printf("Moment %i loaded.\n", now);
      break;

    case SDLK_h:
      mode->fadeMode = !mode->fadeMode;
      break;

    case SDLK_m:
      mode->clearRed = !mode->clearRed;
      break;

    case SDLK_COMMA:
         mode->clearGreen = !mode->clearGreen;
         break;

    case SDLK_PERIOD:
         mode->clearBlue = !mode->clearBlue;
         break;

    case SDLK_j:
      text->imaginaryIndex = -1;
      break;

    case SDLK_k:
      mode->postRotateZoom = !mode->postRotateZoom;
      parms->rotation = 0;
      break;

    case SDLK_z:
      mode->preRotateZoom = !mode->preRotateZoom;
      break;

    case SDLK_x:
      mode->alias = !mode->alias;
      break;

    case SDLK_c:
      mode->multiply = !mode->multiply;
      break;

    case SDLK_n:
      mode->sidebar = !mode->sidebar;
      break;

    case SDLK_v:
      tensor_landscape_p = !tensor_landscape_p;
      SetDims();
      DrawPreviewBorder();
      break;

    case SDLK_b:
      mode->textStagger = (mode->textStagger + 1) % TS_COUNT;
      break;

    case SDLK_LEFTBRACKET:
      mode->shiftRed = (mode->shiftRed + 1) % SM_COUNT;
      break;

    case SDLK_RIGHTBRACKET:
      mode->shiftGreen = (mode->shiftGreen + 1) % SM_COUNT;
      break;

    case SDLK_BACKSLASH:
      mode->shiftBlue = (mode->shiftBlue + 1) % SM_COUNT;
      break;

    default:
      break;
  }  // End <ctrl> mode switch

  return(now);
}


void ParmAlter(int thiskey, moment_t *moments, int now) {
  moment_t *moment = &moments[now];
  parms_t *parms = &moment->coefs;
  modes_t *modes = &moment->mode;

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
      parms->randMod = parms->randMod - max(1, parms->randMod / 50);
      if (parms->randMod < 1) {
        parms->randMod = 1;
      }
      break;

    case SDLK_LEFTBRACKET:
      parms->randMod = INITIAL_RAND_MOD;
      break;

    case SDLK_RIGHTBRACKET:
      parms->randMod = parms->randMod + max(1, parms->randMod / 50);
      if (parms->randMod > 20000) {
        parms->randMod = 20000;
      }
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
      parms->colorCycleMode = (parms->colorCycleMode + 1) % CM_COUNT;
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
      parms->scrollerDir = DIR_UP;
      modes->scroller = YES;
      break;

    case SDLK_DOWN:
      parms->scrollerDir = DIR_DOWN;
      modes->scroller = YES;
      break;

    case SDLK_LEFT:
      parms->scrollerDir = DIR_LEFT;
      modes->scroller = YES;
      break;

    case SDLK_RIGHT:
      parms->scrollerDir = DIR_RIGHT;
      modes->scroller = YES;
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

  for (x = 0; x < TENSOR_WIDTH_EFF; x++) {
    for (y = 0; y < TENSOR_HEIGHT_EFF; y++) {
      if (!(rand() % rFreq)) {
        SetPixel(x, y, color, buffer);
      }
    }
  }
  //~ if (rFreq == 0) {
    //~ return;
  //~ }

  //~ if ((rand() % rFreq) == 0) {
    //~ x = rand() % TENSOR_WIDTH_EFF;
    //~ y = rand() % TENSOR_HEIGHT_EFF;
    //~ SetPixel(x,y,color, buffer);
  //~ }

}



//void ColorCycle(color_t originalColor, colorCycles_e cycleMode, int rainbowInc, int *cycleSaver) {
void ColorCycle(parms_t *parms, int fb_mode) {
  color_t colorTemp = cBlack;
  int inpos, inposo;
  colorCycleModes_e cycleMode = parms->colorCycleMode;
  int *cycleSaver;
  int rainbowInc = parms->rainbowInc;

  // Position in the cycle must be saved in the moment to ensure reload is
  // identical to save.
  if (fb_mode == FOREGROUND) {
    cycleSaver = &parms->cycleSaveForegound;
  } else {
    cycleSaver = &parms->cycleSaveBackground;
  }

  // If the mode changed, we should reset our position in the cycle.
  switch(cycleMode) {
    case CM_RGB:
      *cycleSaver = (*cycleSaver + 1) % paletteRGB.size;
      colorTemp = namedPalette[paletteRGB.palette[*cycleSaver]].color;
      break;

    case CM_CMY:
      *cycleSaver = (*cycleSaver + 1) % paletteCMY.size;
      colorTemp = namedPalette[paletteCMY.palette[*cycleSaver]].color;
      break;

    case CM_SECONDARY:
      *cycleSaver = (*cycleSaver + 1) % paletteSec.size;
      colorTemp = namedPalette[paletteSec.palette[*cycleSaver]].color;
      break;

    case CM_TERTIARY:
      *cycleSaver = (*cycleSaver + 1) % paletteTer.size;
      colorTemp = namedPalette[paletteTer.palette[*cycleSaver]].color;
      break;

    case CM_GRAY:
      *cycleSaver = (*cycleSaver + 1) % paletteGry.size;
      colorTemp = namedPalette[paletteGry.palette[*cycleSaver]].color;
      break;

    case CM_RAINBOW:
      *cycleSaver = (*cycleSaver + rainbowInc) % (256 * 6);
      inposo = *cycleSaver % 256;
      inpos = *cycleSaver / 256;
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

    case CM_RANDOM:
      colorTemp.r = rand() % 255;
      colorTemp.g = rand() % 255;
      colorTemp.b = rand() % 255;
      break;

    default:
      break;
  }

  // We used to return it...
  if (fb_mode == FOREGROUND) {
    parms->fg = colorTemp;
  } else {
    parms->bg = colorTemp;
  }
}


// Write the static menu information to the display.
void InitInfoDisplay(void) {
  int i;
  for (i = 0; i < DISPLAY_TEXT_SIZE; i++) {
    WriteLine(displayText[i].text, displayText[i].line, displayText[i].col, displayText[i].color);
  }
}

// Update the values of the text on the display window.
void UpdateInfoDisplay(moment_t *moment, int now) {

  char text[102], mybuff[102];
  int length;
  parms_t *parms = &moment->coefs;
  modes_t *mode = &moment->mode;
  text_info_t *scrollText = &moment->text;

  // First column
  WriteInt(fps, 21, 1, 3);
  WriteBool(mode->cellAutoFun, 23, 1);
  WriteBool(mode->bouncer, 24, 1);
  WriteBool(mode->fadeout, 25, 1);
  WriteBool(mode->diffuse, 26, 1);
  WriteBool(mode->textSeed, 27, 1);
  WriteBool(mode->roller, 28, 1);
  WriteBool(mode->scroller, 29, 1);
  WriteBool(mode->randDots, 34, 1);
  WriteBool(mode->cycleForeground, 35, 1);
  WriteBool(mode->cycleBackground, 36, 1);
  WriteBool(cycleMomentsMode, 37, 1);
  WriteString(fadeModeText[mode->fadeMode], 38, 1, 3);
  WriteBool(mode->postRotateZoom, 40, 1);
  WriteBool(mode->preRotateZoom, 42, 1);
  WriteBool(mode->alias, 43, 1);
  WriteBool(mode->multiply, 44, 1);
  WriteBool(mode->sidebar, 45, 1);
  WriteBool(mode->clearRed, 46, 1);
  WriteBool(mode->clearGreen, 47, 1);
  WriteBool(mode->clearBlue, 48, 1);
  WriteBool(mode->postImage, 49, 1);

  // Second column
  WriteFloat(parms->floatinc, 2, 3, 14, 6);
  WriteFloat(parms->diffusion_coef, 4, 3, 14, 6);
  WriteFloat(parms->expand, 5, 3, 14, 6);
  WriteInt(parms->fadeout_dec, 6, 3, 14);
  WriteFloat(parms->rotation2, 7, 3, 14, 6);
  WriteInt(parms->rainbowInc, 8, 3, 14);
  WriteFloat(parms->rotation, 9, 3, 14, 6);
  WriteFloat(parms->colorMultiplier, 10, 3, 14, 6);
  WriteFloat(parms->rotationDelta, 11, 3, 14, 6);
  WriteFloat((1.0 / parms->randMod), 12, 3, 14, 6);
  WriteString(colorCycleText[parms->colorCycleMode], 13, 3, 14);
  WriteString(dirText[parms->scrollerDir], 14, 3, 14);
  WriteInt(parms->delay, 15, 3, 14);
  WriteString(namedPalette[parms->foreground].name, 20, 3, 14);
  WriteString(namedPalette[parms->background].name, 21, 3, 14);
  WriteInt(now, 26, 3, 14);
  WriteBool(mode->fontDirection, 35, 4);
  WriteBool(mode->fontFlip, 36, 4);
  WriteInt(parms->textOffset, 37, 3, 14);
  WriteString(staggerText[mode->textStagger], 38, 3, 14);
  WriteInt((int) strlen(scrollText->textBuffer), 39, 3, 14);
  WriteString(shiftText[mode->shiftRed], 44, 3, 14);
  WriteString(shiftText[mode->shiftGreen], 45, 3, 14);
  WriteString(shiftText[mode->shiftBlue], 46, 3, 14);
  WriteString(shiftText[mode->shiftCyan], 47, 3, 14);
  WriteString(shiftText[mode->shiftYellow], 48, 3, 14);
  WriteString(shiftText[mode->shiftMagenta], 49, 3, 14);

  // Show the last 100 bytes of the text buffer.
  length = strlen(scrollText->textBuffer);
  strncpy(mybuff, scrollText->textBuffer + (length > 100 ? length - 100 : 0), 101);
  // The extra snprintf takes care of overwriting longer lines.
  snprintf(text, sizeof(text), "%-100s", mybuff );
  WriteLine(text, 52, 0, PARAMETER_COLOR);

  UpdateAll();
}

void WriteBool(int value, int row, int col) {
  if (value) {
    WriteLine("YES", row, col, PARAMETER_COLOR);
  } else {
    WriteLine(" NO", row, col, PARAMETER_COLOR);
  }
}

void WriteInt(int value, int row, int col, int width) {
  char text[100];
  snprintf(text, sizeof(text), "%*i", width, value);
  WriteLine(text, row, col, PARAMETER_COLOR);
}

void WriteFloat(float value, int row, int col, int width, int precision) {
  char text[100];
  snprintf(text, sizeof(text), "%*.*f", width, precision, value);
  WriteLine(text, row, col, PARAMETER_COLOR);
}

void WriteString(const char *text, int line, int col, int width) {
  char temp[200];
  snprintf(temp, sizeof(temp), "%*.*s", width, width, text);
  WriteLine(temp, line, col, PARAMETER_COLOR);
}

// Writes a line of text to the selected line and column of the
// output window with the selected color.
void WriteLine(char * thisText, int line, int col, color_t color) {

  // Vars
  const SDL_Color fontColor = {color.r, color.g, color.b};
  const SDL_Color bgColor = {0, 0, 0};
  const int colstart[] = {0, 275, 375, 675, 763};
  SDL_Surface *textSurface;
  SDL_Texture *textTexture;
  SDL_Rect rect;

  // Set the position of the output text.
  rect.x = colstart[col];
  rect.y = line * (PREVIEW_FONT_HEIGHT);

  // Render the text to a surface.
  textSurface = TTF_RenderText_Shaded(screenFont, thisText, fontColor, bgColor);
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

  if (previewSurface) SDL_FreeSurface(previewSurface);

  previewSurface = SDL_CreateRGBSurface(0, TENSOR_WIDTH_EFF, TENSOR_HEIGHT_EFF,
    32, 0, 0, 0, 0);
  if (!previewSurface) {
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
  rotatedImage = rotozoomSurface (imageSeed, angle, expansion, aliasmode ? 1 : 0);
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
  DrawBox(0, 0, h, w, cBlack);

  // Draw the new border.
  DrawRectangle(0, 0, w, h, cWhite);

  // Draw the panel indicators
  w2 = (TENSOR_WIDTH_EFF * PREVIEW_PIXEL_WIDTH) + 1;
  h2 = (TENSOR_HEIGHT_EFF * PREVIEW_PIXEL_HEIGHT) + 1;

  if (tensor_landscape_p) {
    // Landscape - Horizontal indicators.
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 0 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, cGray);
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 1 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, cGray);
    DrawRectangle(PREVIEW_BORDER_THICKNESS - 1, 2 * (h2 / 3) + PREVIEW_BORDER_THICKNESS - 1,
                  w2, (h2 / 3) + 1, cGray);
  } else {
    // Portrait - Vertical indicators
    DrawRectangle(0 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, cGray);
    DrawRectangle(1 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, cGray);
    DrawRectangle(2 * (w2 / 3) + PREVIEW_BORDER_THICKNESS - 1, PREVIEW_BORDER_THICKNESS - 1,
                  (w2 / 3) + 1, h2, cGray);
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
    case DIR_LEFT:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
              SetPixel(TENSOR_WIDTH_EFF - 1, i, moment->coefs.fg, moment->fb);
            }
            break;

    case DIR_RIGHT:
      for (i = 0; i < TENSOR_HEIGHT_EFF; i++) {
              SetPixel(0, i, moment->coefs.fg, moment->fb);
            }
            break;

    case DIR_UP:
      for (i = 0; i < TENSOR_WIDTH_EFF; i++) {
              SetPixel(i, TENSOR_HEIGHT_EFF - 1, moment->coefs.fg, moment->fb);
            }
            break;

    case DIR_DOWN:
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


