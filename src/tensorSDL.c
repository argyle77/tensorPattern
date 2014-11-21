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
#define INVALID (-1)
#define FOREVER for(;;)
#define CYCLE_FRAME_LIMIT 1000
#define GUIFRAMEDELAY 25

// Tensor output
#define USE_TENSOR
#define DEF_IMAGE "Fbyte-01.jpg"

// Preview window definitions
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define PREVIEW_PIXEL_SIZE 10
#define PREVIEW_BORDER_THICKNESS 10

#define PREVIEW_A_POSITION_X 0
#define PREVIEW_A_POSITION_Y 0
#define PREVIEW_B_POSITION_X (WINDOW_WIDTH - (TENSOR_WIDTH * PREVIEW_PIXEL_SIZE) - (PREVIEW_BORDER_THICKNESS * 2))
#define PREVIEW_B_POSITION_Y 0
#define DISPLAY_FONT_SIZE 14
#define DISPLAY_TEXT_HEIGHT 14

#define TEXT_BUFFER_SIZE 2048
#define PATTERN_SET_COUNT 10

// Initial Values
#define INITIAL_COLOR_MULTIPLIER 1.0
#define INITIAL_DIFF_COEF 0.002
#define INITIAL_FADEOUT_DEC -1
#define INITIAL_RAND_MOD 100
#define INITIAL_RAINBOW_INC 8
#define INITIAL_EXPAND 1
#define INITIAL_FLOAT_INC 0.1
#define INITIAL_POSTROT_INC 0
#define INITIAL_POSTROT_ANGLE 0
#define INITIAL_PREROT_ANGLE 10
#define INITIAL_DELAY 60
#define INITIAL_TEXT "Frostbyte was an engineer. "
#define INITIAL_DIR DIR_LEFT

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

// Colors for the info display texts.
#define DISPLAY_COLOR_PARMS cAzure
#define DISPLAY_COLOR_PARMSBG cBlack
#define DISPLAY_COLOR_TITLES cYellow
#define DISPLAY_COLOR_TITLESBG cBlack
#define DISPLAY_COLOR_TEXTS cWhite
#define DISPLAY_COLOR_TEXTSBG cBlack
#define DISPLAY_COLOR_TEXTS_HL cBlack
#define DISPLAY_COLOR_TEXTSBG_HL cWhite

// Named color palette structures and constants
typedef struct palette_t {
  char *name;
  color_t color;
} palette_t;

// If you change this, change palette too.
typedef enum color_e {
  CE_INVALID = -1,
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
  DIR_INVALID = -1,
  DIR_UP = 0, DIR_LEFT, DIR_DOWN, DIR_RIGHT,
  DIR_COUNT // Last.
} dir_e;
const char *dirText[] = { "Up", "Left", "Down", "Right" };

// Fade modes
typedef enum fadeModes_e{
  FM_INVALID = -1,
  FM_LIMIT = 0, FM_MODULAR,
  FM_COUNT // Last.
} fadeModes_e;
const char *fadeModeText[] = { "Limited", "Modular" };

// Color plane shift modes.
typedef enum shiftModes_e {
  SM_INVALID = -1,
  SM_HOLD = 0, SM_UP, SM_LEFT, SM_DOWN, SM_RIGHT,
  SM_COUNT // Last.
} shiftModes_e;
const char *shiftText[] = { "Hold", "Up", "Left", "Down", "Right" };

// Text background modes
typedef enum textStaggerMode_e {
  TS_INVALID = -1,
  TS_8ROW = 0, TS_9ROWSTAGGER, TS_10ROWBORDER, TS_FULLBG,
  TS_COUNT // Last.
} textStaggerMode_e;
char *staggerText[] = { "8 Row", "9RowStagger", "10RowBorder", "FullBG" };

// Color palette cycle modes
typedef enum colorCycleModes_e{
  CM_INVALID = -1,
  CM_RGB = 0, CM_CMY, CM_SECONDARY, CM_TERTIARY, CM_GRAY, CM_RAINBOW, CM_RANDOM,
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
  unsigned char rollOver;
  unsigned char scroller;
  unsigned char horizontalBars;
  unsigned char verticalBars;
  unsigned char colorAll;
  unsigned char clearAll;
  unsigned char randomDots;
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
  // Use parms
  color_e foreground;
  color_e background;
  int fadeAllIncr;
  float diffusionCoef;
  dir_e scrollDirection;
  int dotRandomness;
  colorCycleModes_e colorCycleMode;
  int rainbowInc;
  float expand;
  float floatIncr;
  float postRotationAngle;
  float preRotationAngle;
  float postRotationIncr;
  float colorMultiplier;
  int textOffset;
  int delay;
  // Internals
  int cycleSaveForegound;
  int cycleSaveBackground;
  color_t fg;  // For keeping colors other than the color_e named ones.
  color_t bg;  // For keeping colors other than the color_e named ones.
  int cellAutoCount;
} parms_t;

typedef struct textBuffer_t{
  char textBuffer[TEXT_BUFFER_SIZE];
  int textIndex;  // How many chars in.
  int pixelIndex;  // How many pixel cols in?
  unsigned char textStaggerFlag[TEXT_BUFFER_SIZE];
  dir_e prevScrollDir;
} textBuffer_t;

typedef struct patternSet_t{
  modes_t mode;
  parms_t parm;
  textBuffer_t text;
  unsigned char fb[TENSOR_BYTES];  // Tensor frame buffer image
} patternSet_t;

// User Commands
typedef enum command_e {
  COM_INVALID = -1, COM_NONE = 0,
  COM_CELL = 1, COM_BOUNCE, COM_FADE, COM_DIFFUSE, COM_TEXT, COM_ROLL,
  COM_SCROLL, COM_HBAR, COM_VBAR, COM_FGALL, COM_BGALL, COM_RDOT, COM_FGCYCLE,
  COM_BGCYCLE, COM_CYCLESET, COM_FADEMODE, COM_TEXTRESET, COM_POSTROTATE,
  COM_MODEOFF, COM_PREROTATE, COM_AA, COM_MULTIPLY, COM_SIDEBAR, COM_NORED,
  COM_NOGREEN, COM_NOBLUE, COM_IMAGE, COM_STEP_INC, COM_STEP_DEC, COM_STEP_RST,
  COM_DIFFUSE_INC, COM_DIFFUSE_DEC, COM_DIFFUSE_RST, COM_EXPAND_INC,
  COM_EXPAND_DEC, COM_EXPAND_RST, COM_FADE_INC, COM_FADE_DEC, COM_FADE_RST,
  COM_PREROT_INC, COM_PREROT_DEC, COM_PREROT_RST, COM_RAINDBOW_INC,
  COM_RAINDBOW_DEC, COM_RAINDBOW_RST, COM_POSTROT_INC, COM_POSTROT_DEC,
  COM_POSTROT_RST, COM_MULT_INC, COM_MULT_DEC, COM_MULT_RST, COM_POSTSPEED_INC,
  COM_POSTSPEED_DEC, COM_POSTSPEED_RST, COM_RANDOM_INC, COM_RANDOM_DEC,
  COM_RANDOM_RST, COM_CYCLE_MODE, COM_CYCLE_MODE_DOWN,
  COM_SCROLL_UP, COM_SCROLL_DOWN, COM_SCROLL_LEFT, COM_SCROLL_RIGHT, COM_SCROLL_DOWNC, COM_SCROLL_LEFTC,
  COM_SCROLL_RIGHTC, COM_SCROLL_UPC, COM_DELAY_INC, COM_DELAY_DEC,
  COM_DELAY_RST, COM_FG_INC, COM_FG_DEC, COM_BG_INC, COM_BG_DEC, COM_BACKSPACE,
  COM_DELETE, COM_TEXT_REVERSE, COM_TEXT_FLIP, COM_TEXTO_INC, COM_TEXTO_DEC,
  COM_TEXT_MODE_UP, COM_TEXT_MODE_DOWN, COM_RP_UP, COM_RP_DOWN, COM_GP_UP, COM_GP_DOWN, COM_BP_UP, COM_BP_DOWN,
  COM_CP_UP, COM_CP_DOWN, COM_YP_UP, COM_YP_DOWN, COM_MP_UP, COM_MP_DOWN, COM_EXIT,
  COM_ORIENTATION, COM_RETURN, COM_SCROLL_CYCLE_UP, COM_SCROLL_CYCLE_DOWN, COM_LOAD0, COM_LOAD1,
  COM_LOAD2, COM_LOAD3, COM_LOAD4, COM_LOAD5, COM_LOAD6, COM_LOAD7, COM_LOAD8,
  COM_LOAD9, COM_LOADSET0, COM_LOADSET1, COM_LOADSET2, COM_LOADSET3,
  COM_LOADSET4, COM_LOADSET5, COM_LOADSET6, COM_LOADSET7, COM_LOADSET8,
  COM_LOADSET9, COM_COPYSET0, COM_COPYSET1, COM_COPYSET2, COM_COPYSET3,
  COM_COPYSET4, COM_COPYSET5, COM_COPYSET6, COM_COPYSET7, COM_COPYSET8,
  COM_COPYSET9,
  COM_COUNT // Last
} command_e;

//
typedef struct keyCommand_t {
  SDL_Keymod mod;
  SDL_Keycode key;
  command_e command;
} keyCommand_t;

// Single key commands
typedef enum mouseCommand_e {
  MOUSE_INVALID = -1,
  MOUSE_CLICK = 0, MOUSE_WHEEL_UP, MOUSE_WHEEL_DOWN,
  MOUSE_COUNT // Last
} mouseCommand_e;

typedef struct command_t {
  int line;
  int col;
  char *text;
  keyCommand_t commands[MOUSE_COUNT];
} command_t;

const command_t mouseCommands[] = {
  {23, 0, "Cell pattern", {{KMOD_CTRL, SDLK_q, COM_CELL}}},
  {24, 0, "Bouncer", {{KMOD_CTRL, SDLK_w, COM_BOUNCE}}},
  {25, 0, "Fader", {{KMOD_CTRL, SDLK_e,  COM_FADE}}},
  {26, 0, "Diffuser", {{KMOD_CTRL, SDLK_r, COM_DIFFUSE}}},
  {27, 0, "Text seed", {{KMOD_CTRL, SDLK_t, COM_TEXT}}},
  {28, 0, "Scroll rollover", {{KMOD_CTRL, SDLK_y, COM_ROLL}}},
  {29, 0, "Scroller", {{KMOD_CTRL, SDLK_u, COM_SCROLL}}},
  {30, 0, "Horizontal bars!", {{KMOD_CTRL, SDLK_i, COM_HBAR}}},
  {31, 0, "Vertical bars!", {{KMOD_CTRL, SDLK_o, COM_VBAR}}},
  {32, 0, "Foreground color all!", {{KMOD_CTRL, SDLK_p, COM_FGALL}}},
  {33, 0, "Background color all!", {{KMOD_CTRL, SDLK_a, COM_BGALL}}},
  {34, 0, "Random dot seed", {{KMOD_CTRL, SDLK_s, COM_RDOT}}},
  {35, 0, "FG cycle", {{KMOD_CTRL, SDLK_d, COM_FGCYCLE}}},
  {36, 0, "BG cycle", {{KMOD_CTRL, SDLK_f, COM_BGCYCLE}}},
  {37, 0, "Cycle pattern sets", {{KMOD_CTRL, SDLK_g, COM_CYCLESET}}},
  {38, 0, "Fader mode", {{KMOD_CTRL, SDLK_h, COM_FADEMODE}}},
  {39, 0, "Reset text to start", {{KMOD_CTRL, SDLK_j, COM_TEXTRESET}}},
  {40, 0, "Post-rotate/zoom", {{KMOD_CTRL, SDLK_k, COM_POSTROTATE}}},
  {41, 0, "All modes off", {{KMOD_CTRL, SDLK_l, COM_MODEOFF}}},
  {42, 0, "Pre-rotate/zoom", {{KMOD_CTRL, SDLK_z, COM_PREROTATE}}},
  {43, 0, "Rotate/zoom anti-alias", {{KMOD_CTRL, SDLK_x, COM_AA}}},
  {44, 0, "Multiplier", {{KMOD_CTRL, SDLK_c, COM_MULTIPLY}}},
  {45, 0, "Sidebar seed", {{KMOD_CTRL, SDLK_n, COM_SIDEBAR}}},
  {46, 0, "Supress red", {{KMOD_CTRL, SDLK_m, COM_NORED}}},
  {47, 0, "Supress green", {{KMOD_CTRL, SDLK_COMMA, COM_NOGREEN}}},
  {48, 0, "Supress blue", {{KMOD_CTRL, SDLK_PERIOD, COM_NOBLUE}}},
  {49, 0, "Image overlay", {{KMOD_CTRL | KMOD_ALT, SDLK_p, COM_IMAGE}}},
  {13, 2, "Color cycle mode", {{KMOD_ALT, SDLK_SEMICOLON, COM_CYCLE_MODE}, {}, {0, 0, COM_CYCLE_MODE_DOWN}}},
  {14, 2, "(<alt>) <arrows> - Scroll direction", {{0, 0, COM_SCROLL_CYCLE_UP}, {}, {0, 0, COM_SCROLL_CYCLE_DOWN}}},
  {33, 2, "<backspace> - Delete last letter.", {{KMOD_NONE, SDLK_BACKSPACE, COM_BACKSPACE}}},
  {34, 2, "Erase all text.", {{KMOD_NONE, SDLK_DELETE, COM_DELETE}}},
  {35, 2, "Reverse text.", {{KMOD_CTRL, SDLK_QUOTE, COM_TEXT_REVERSE}}},
  {36, 2, "Flip text.", {{KMOD_CTRL, SDLK_SEMICOLON, COM_TEXT_FLIP}}},
  {38, 2, "Text mode:", {{KMOD_CTRL, SDLK_b, COM_TEXT_MODE_UP}, {}, {0, 0, COM_TEXT_MODE_DOWN}}},
  {44, 2, "Red plane:", {{KMOD_CTRL, SDLK_LEFTBRACKET, COM_RP_UP}, {}, {0, 0, COM_RP_DOWN}}},
  {45, 2, "Green plane:", {{KMOD_CTRL, SDLK_RIGHTBRACKET, COM_GP_UP}, {}, {0, 0, COM_GP_DOWN}}},
  {46, 2, "Blue plane:", {{KMOD_CTRL, SDLK_BACKSLASH, COM_BP_UP}, {}, {0, 0, COM_BP_DOWN}}},
  {47, 2, "Cyan plane:", {{KMOD_CTRL | KMOD_ALT, SDLK_LEFTBRACKET, COM_CP_UP}, {}, {0, 0, COM_CP_DOWN}}},
  {48, 2, "Yellow plane:", {{KMOD_CTRL | KMOD_ALT, SDLK_RIGHTBRACKET, COM_YP_UP}, {}, {0, 0, COM_YP_DOWN}}},
  {49, 2, "Magenta plane:", {{KMOD_CTRL | KMOD_ALT, SDLK_BACKSLASH, COM_MP_UP}, {}, {0, 0, COM_MP_DOWN}}},
  {51, 2, "Quit.", {{KMOD_NONE, SDLK_ESCAPE, COM_EXIT}}},
  {2, 2, "Float step size", {{KMOD_ALT, SDLK_COMMA, COM_STEP_RST}, {KMOD_ALT, SDLK_m, COM_STEP_INC}, {KMOD_ALT, SDLK_PERIOD, COM_STEP_DEC}}},
  {4, 2, "Diffusion", {{KMOD_ALT, SDLK_w, COM_DIFFUSE_RST}, {KMOD_ALT, SDLK_q, COM_DIFFUSE_INC}, {KMOD_ALT, SDLK_e, COM_DIFFUSE_DEC}}},
  {5, 2, "Expansion", {{KMOD_ALT, SDLK_s, COM_EXPAND_RST}, {KMOD_ALT, SDLK_a, COM_EXPAND_INC}, {KMOD_ALT, SDLK_d, COM_EXPAND_DEC}}},
  {6, 2, "Fade amount", {{KMOD_ALT, SDLK_x, COM_FADE_RST}, {KMOD_ALT, SDLK_z, COM_FADE_INC}, {KMOD_ALT, SDLK_c, COM_FADE_DEC}}},
  {7, 2, "Pre rotation angle", {{KMOD_ALT, SDLK_t, COM_PREROT_RST}, {KMOD_ALT, SDLK_r, COM_PREROT_INC}, {KMOD_ALT, SDLK_y, COM_PREROT_DEC}}},
  {8, 2, "Rainbow Speed", {{KMOD_ALT, SDLK_g, COM_RAINDBOW_RST}, {KMOD_ALT, SDLK_f, COM_RAINDBOW_INC}, {KMOD_ALT, SDLK_h, COM_RAINDBOW_DEC}}},
  {9, 2, "Post rotation angle", {{KMOD_ALT, SDLK_b, COM_POSTROT_RST}, {KMOD_ALT, SDLK_v, COM_POSTROT_INC}, {KMOD_ALT, SDLK_n, COM_POSTROT_DEC}}},
  {10, 2, "Multiplier", {{KMOD_ALT, SDLK_i, COM_MULT_RST}, {KMOD_ALT, SDLK_u, COM_MULT_INC}, {KMOD_ALT, SDLK_o, COM_MULT_DEC}}},
  {11, 2, "Post rotation incr", {{KMOD_ALT, SDLK_k, COM_POSTSPEED_RST}, {KMOD_ALT, SDLK_j, COM_POSTSPEED_INC}, {KMOD_ALT, SDLK_l, COM_POSTSPEED_DEC}}},
  {12, 2, "Dot randomness", {{KMOD_ALT, SDLK_LEFTBRACKET, COM_RANDOM_RST}, {KMOD_ALT, SDLK_p, COM_RANDOM_DEC}, {KMOD_ALT, SDLK_RIGHTBRACKET, COM_RANDOM_INC}}},
  {15, 2, "Frame delay(ms)", {{KMOD_ALT | KMOD_CTRL, SDLK_i, COM_DELAY_RST}, {KMOD_ALT | KMOD_CTRL, SDLK_u, COM_DELAY_INC}, {KMOD_ALT | KMOD_CTRL, SDLK_o, COM_DELAY_DEC}}},
  {20, 2, "foreground:", {{KMOD_ALT | KMOD_CTRL, SDLK_q, COM_FG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_w, COM_FG_DEC}}},
  {21, 2, "background:", {{KMOD_ALT | KMOD_CTRL, SDLK_a, COM_BG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_s, COM_BG_DEC}}},
  {37, 2, "Offset:", {{KMOD_ALT | KMOD_CTRL, SDLK_z, COM_TEXTO_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_x, COM_TEXTO_DEC}}},
};
#define DISPLAY_COMMAND_SIZE (sizeof(mouseCommands) / sizeof(command_t))

const command_t otherCommands[] = {
  {-1, 0, "Orientation", {{KMOD_CTRL, SDLK_v, COM_ORIENTATION}}},
  {-1, 2, "Scroll Up", {{KMOD_NONE, SDLK_UP, COM_SCROLL_UP}}},
  {-1, 2, "Scroll Down", {{KMOD_NONE, SDLK_DOWN, COM_SCROLL_DOWN}}},
  {-1, 2, "Scroll Left", {{KMOD_NONE, SDLK_LEFT, COM_SCROLL_LEFT}}},
  {-1, 2, "Scroll Right", {{KMOD_NONE, SDLK_RIGHT, COM_SCROLL_RIGHT}}},
  {-1, 2, "Activate & Scroll Up", {{KMOD_ALT, SDLK_UP, COM_SCROLL_UPC}}},
  {-1, 2, "Activate & Scroll Down", {{KMOD_ALT, SDLK_DOWN, COM_SCROLL_DOWNC}}},
  {-1, 2, "Activate & Scroll Left", {{KMOD_ALT, SDLK_LEFT, COM_SCROLL_LEFTC}}},
  {-1, 2, "Activate & Scroll Right", {{KMOD_ALT, SDLK_RIGHT, COM_SCROLL_RIGHTC}}},
  {-1, 0, "Load image buffer 0", {{KMOD_ALT,  SDLK_0, COM_LOAD0 }}},
  {-1, 0, "Load image buffer 1", {{KMOD_ALT,  SDLK_1, COM_LOAD1 }}},
  {-1, 0, "Load image buffer 2", {{KMOD_ALT,  SDLK_2, COM_LOAD2 }}},
  {-1, 0, "Load image buffer 3", {{KMOD_ALT,  SDLK_3, COM_LOAD3 }}},
  {-1, 0, "Load image buffer 4", {{KMOD_ALT,  SDLK_4, COM_LOAD4 }}},
  {-1, 0, "Load image buffer 5", {{KMOD_ALT,  SDLK_5, COM_LOAD5 }}},
  {-1, 0, "Load image buffer 6", {{KMOD_ALT,  SDLK_6, COM_LOAD6 }}},
  {-1, 0, "Load image buffer 7", {{KMOD_ALT,  SDLK_7, COM_LOAD7 }}},
  {-1, 0, "Load image buffer 8", {{KMOD_ALT,  SDLK_8, COM_LOAD8 }}},
  {-1, 0, "Load image buffer 9", {{KMOD_ALT,  SDLK_9, COM_LOAD9 }}},
  {-1, 0, "Load pattern set 0", {{KMOD_CTRL,  SDLK_0, COM_LOADSET0 }}},
  {-1, 0, "Load pattern set 1", {{KMOD_CTRL,  SDLK_1, COM_LOADSET1 }}},
  {-1, 0, "Load pattern set 2", {{KMOD_CTRL,  SDLK_2, COM_LOADSET2 }}},
  {-1, 0, "Load pattern set 3", {{KMOD_CTRL,  SDLK_3, COM_LOADSET3 }}},
  {-1, 0, "Load pattern set 4", {{KMOD_CTRL,  SDLK_4, COM_LOADSET4 }}},
  {-1, 0, "Load pattern set 5", {{KMOD_CTRL,  SDLK_5, COM_LOADSET5 }}},
  {-1, 0, "Load pattern set 6", {{KMOD_CTRL,  SDLK_6, COM_LOADSET6 }}},
  {-1, 0, "Load pattern set 7", {{KMOD_CTRL,  SDLK_7, COM_LOADSET7 }}},
  {-1, 0, "Load pattern set 8", {{KMOD_CTRL,  SDLK_8, COM_LOADSET8 }}},
  {-1, 0, "Load pattern set 9", {{KMOD_CTRL,  SDLK_9, COM_LOADSET9 }}},
  {-1, 0, "Copy to set 0", {{KMOD_CTRL | KMOD_ALT,  SDLK_0, COM_COPYSET0 }}},
  {-1, 0, "Copy to set 1", {{KMOD_CTRL | KMOD_ALT,  SDLK_1, COM_COPYSET1 }}},
  {-1, 0, "Copy to set 2", {{KMOD_CTRL | KMOD_ALT,  SDLK_2, COM_COPYSET2 }}},
  {-1, 0, "Copy to set 3", {{KMOD_CTRL | KMOD_ALT,  SDLK_3, COM_COPYSET3 }}},
  {-1, 0, "Copy to set 4", {{KMOD_CTRL | KMOD_ALT,  SDLK_4, COM_COPYSET4 }}},
  {-1, 0, "Copy to set 5", {{KMOD_CTRL | KMOD_ALT,  SDLK_5, COM_COPYSET5 }}},
  {-1, 0, "Copy to set 6", {{KMOD_CTRL | KMOD_ALT,  SDLK_6, COM_COPYSET6 }}},
  {-1, 0, "Copy to set 7", {{KMOD_CTRL | KMOD_ALT,  SDLK_7, COM_COPYSET7 }}},
  {-1, 0, "Copy to set 8", {{KMOD_CTRL | KMOD_ALT,  SDLK_8, COM_COPYSET8 }}},
  {-1, 0, "Copy to set 9", {{KMOD_CTRL | KMOD_ALT,  SDLK_9, COM_COPYSET9 }}},
};
#define OTHER_COMMAND_SIZE (sizeof(otherCommands) / sizeof(command_t))

// Display titles
typedef struct displayText_t {
  int line;
  int col;
  char *text;
} displayText_t;

displayText_t otherText[] = {
  {39, 2, "Text buffer size:"},
  {32, 2, "<Unmodified keys> - Add text."},
  {28, 2, "<alt> 0-9 - Load image buffer from set #"},
  {26, 2, "<ctrl> 0-9 - Load set #.  Current:"},
  {27, 2, "<ctrl> <alt> 0-9 - Copy to set #"}
};
#define OTHER_TEXT_SIZE (sizeof(otherText) / sizeof(displayText_t))

displayText_t headerText[] = {
  {21, 0,  "Modes:               PREVIEW FPS:"},
  {22, 0, "                         GUI FPS:"},
  {51, 0,  "Text buffer:"},
  {0, 2,   "Coeffs: (3 keys = increment / reset / decrement)"},
  {18, 2, "Colors:"},
  {24, 2, "Pattern Sets:"},
  {31, 2, "Text entry:"},
  {42, 2, "More modes:"}
};
#define HEAD_TEXT_SIZE (sizeof(headerText) / sizeof(displayText_t))

// Globals - We do love our globals.
TTF_Font *screenFont;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;
SDL_Surface *scratchSurface = NULL;
SDL_Surface *imageSeed = NULL;
int tensorWidth, tensorHeight;
int cyclePatternSets = NO;  // cyclePatternSets mode is a global (for now).
int currentSet = 0;
float global_intensity_limit = 1.0;
int previewFrameCount = 0, previewFPS = 0;
int guiFrameCount = 0, guiFPS = 0;
const int colToPixel[] = {0, 275, 375, 675, 763, WINDOW_WIDTH};
Uint32 FPSEventType, DRAWEventType, GUIEventType;

// We hold PATTERN_SET_COUNT pattern sets in memory at a time.  We'll put some
// intial values in the first one and then copy that to the others.  Note, c
// globals are initialized to 0 at startup, so the rest should come up 0.
patternSet_t patternSets[PATTERN_SET_COUNT] = { {
  .mode.alias = NO, .mode.bouncer = NO, .mode.cellAutoFun = NO,
  .mode.clearAll = NO, .mode.colorAll = NO, .mode.cycleBackground = NO,
  .mode.cycleForeground = YES, .mode.diffuse = NO, .mode.fadeMode = FM_LIMIT,
  .mode.fadeout = NO, .mode.horizontalBars = NO, .mode.multiply = NO,
  .mode.randomDots = NO, .mode.rollOver = NO, .mode.postRotateZoom = NO,
  .mode.preRotateZoom = NO, .mode.scroller = YES, .mode.textSeed = YES,
  .mode.verticalBars = NO, .mode.textStagger = TS_FULLBG, .mode.sidebar = NO,
  .mode.clearBlue = NO, .mode.clearGreen = NO, .mode.clearRed = NO,
  .mode.shiftRed = SM_HOLD, .mode.shiftGreen = SM_HOLD,
  .mode.shiftBlue = SM_HOLD, .mode.shiftCyan = SM_HOLD,
  .mode.shiftMagenta = SM_HOLD, .mode.shiftYellow = SM_HOLD,
  .mode.postImage = NO, .mode.fontFlip = NO, .mode.fontDirection = FORWARDS,
  .parm.background = CE_BLACK, .parm.bg = CD_BLACK,
  .parm.colorCycleMode = CM_RAINBOW, .parm.colorMultiplier = INITIAL_COLOR_MULTIPLIER,
  .parm.cycleSaveForegound = 0, .parm.cycleSaveBackground = 0,
  .parm.diffusionCoef = INITIAL_DIFF_COEF, .parm.expand = INITIAL_EXPAND,
  .parm.fadeAllIncr = INITIAL_FADEOUT_DEC, .parm.floatIncr = INITIAL_FLOAT_INC,
  .parm.foreground = CE_RED, .parm.fg = CD_RED, .parm.rainbowInc = INITIAL_RAINBOW_INC,
  .parm.dotRandomness = INITIAL_RAND_MOD, .parm.postRotationAngle = INITIAL_POSTROT_ANGLE,
  .parm.preRotationAngle = INITIAL_PREROT_ANGLE, .parm.postRotationIncr = INITIAL_POSTROT_INC,
  .parm.scrollDirection = INITIAL_DIR, .parm.cellAutoCount = 0,
  .parm.textOffset = TENSOR_HEIGHT / 3 - 1, .parm.delay = INITIAL_DELAY,
  .text.textBuffer = INITIAL_TEXT, .text.textIndex = sizeof(INITIAL_TEXT),
  .text.pixelIndex = -1, .text.prevScrollDir = INITIAL_DIR,
  .text.textStaggerFlag = "a"
} };


// Prototypes
void DrawNewFrame(void);
void ProcessModes(void);
Uint32 TriggerFrameDraw(Uint32 interval, void *param);
Uint32 TriggerFrameCount(Uint32 interval, void *param);
Uint32 TriggerGUIUpdate(Uint32 interval, void *param);
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer);
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane);
void WriteSlice(patternSet_t *patternSet);
void CellFun(patternSet_t *patternSet);
void DrawSideBar(patternSet_t *patternSet);
void Diffuse(float diffusionCoeff, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
void SavePatternSet(SDL_KeyboardEvent *key, patternSet_t *patternSet, int setIndex);
void LoadPatternSet(SDL_KeyboardEvent *key, patternSet_t *patternSet, char *fn);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
void ColorCycle(parms_t *parms, int fb_mode);
void ColorAll(color_t color, unsigned char *fb);
void SetDims(void);
void UpdateDisplays(float intensity_limit, patternSet_t *patternSet);
void UpdateGUI(void);
void UpdateTensor(unsigned char *buffer);
void DrawPreviewBorder(int x, int y);
void UpdatePreview(int xOffset, int yOffset, unsigned char *buffer);
void InitDisplayTexts(void);
void UpdateInfoDisplay(patternSet_t *patternSet, int setIndex);
void ClearWindow(void);
void WriteLine(char * thisText, int line, int col, color_t color, color_t bgColor);
void WriteBool(int value, int row, int col);
void WriteInt(int value, int row, int col, int width);
void WriteFloat(float value, int row, int col, int width, int precision);
void WriteString(const char *text, int line, int col, int width);
void WriteCommand(int index, const command_t *commands, color_t fg, color_t bg);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src);
void Multiply(float multiplier, unsigned char *buffer);
void DrawRectangle(int x, int y, int w, int h, color_t color);
void DrawBox(int x, int y, int w, int h, color_t color);
void ClearRed(patternSet_t *currentMoment);
void ClearGreen(patternSet_t *currentMoment);
void ClearBlue(patternSet_t *currentMoment);
void DrawImage(double angle, double expansion, int aliasmode, unsigned char *fb_dst);
int min(int a, int b);
int max(int a, int b);
unsigned char SameRectangle(SDL_Rect a, SDL_Rect b);
int HandleCommand(command_e command);
int HandleKey(SDL_Keycode key, SDL_Keymod mod);

// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  int i;
  unsigned char exitProgram = NO;
  char caption_temp[100];
  SDL_Event event;
  unsigned char drawNewFrame = NO;
  int x, y;
  int thisHover = INVALID;
  int lastHover = INVALID;
  int mouseDownOn = INVALID;
  SDL_Rect box = {0, 0, 0, 0}, boxOld = {0, 0, 0, 0};

  // Unbuffer the console...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  setvbuf(stderr, (char *)NULL, _IONBF, 0);

  // Commandline parameter for limiting the intensity.
  if (argc == 2) {
    global_intensity_limit = atof(argv[1]);
    if (global_intensity_limit < -0.0001 || global_intensity_limit > 1.0001) {
      global_intensity_limit = 1.0;
    }
  }

  // Initialize SDL components.
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  atexit(SDL_Quit);  // Register the SDL cleanup function.

  // Initialize the SDL window and renderer.
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &mainWindow, &mwRenderer);
  if ((!mainWindow) || (!mwRenderer)) {
    fprintf(stderr, "Unable to create main window or renderer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Window scaling hints:
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(mwRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);

  // Set the window title
  snprintf(caption_temp, sizeof(caption_temp), "Tensor Control - Output: %i%%", (int) (global_intensity_limit * 100));
  SDL_SetWindowTitle(mainWindow, caption_temp);

  // Load the font.  Should be a fixed width font.
  if(TTF_Init() < 0) {
    fprintf(stderr, "Couldn't initialize TTF: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  } else {
    screenFont = TTF_OpenFont("font.ttf", DISPLAY_FONT_SIZE);
    if(!screenFont) {
      fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
      exit(EXIT_FAILURE);
    }
  }

  // Load an image. (Testing)
  imageSeed = IMG_Load(DEF_IMAGE);
  if (!imageSeed) {
    fprintf(stderr, "Unable to load image: %s\n", DEF_IMAGE);
    exit(EXIT_FAILURE);
  }

  // Initialize tensor communications and aspect.
#ifdef USE_TENSOR
  tensor_init();
  //tensor_landscape_p = 1;  // Landscape mode (good for single panel).
  tensor_landscape_p = 0;  // Portrait mode (normal).
#endif

  // Set the widths / heights
  SetDims();  // After set tensor_landscape_p.

  // Draw a border around the preview
  DrawPreviewBorder(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y);
  //~ DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);

  // Further patternSet 0 initializations
  patternSets[0].parm.textOffset = tensorHeight / 3 - 1;  // After SetDims()
  ColorAll(cBlack, patternSets[0].fb);

  // Initialize the rest of the pattern sets from the first one.
  for (i = 1; i < PATTERN_SET_COUNT; i++) {
    memcpy(&patternSets[i], &patternSets[0], sizeof(patternSet_t));
  }

  // Attempt to load startup pattern sets from disk.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    char filename[8];
    snprintf(filename, sizeof(filename), "%i.now", i);
    LoadPatternSet(NULL, &patternSets[i], filename);
  }

  // Bam - Show the (blank) preview.
  UpdateDisplays(global_intensity_limit, &patternSets[currentSet]);

  // Add the text to the window
  InitDisplayTexts();

  // Initialize the user events
  FPSEventType = SDL_RegisterEvents(1);
  DRAWEventType = SDL_RegisterEvents(1);
  GUIEventType = SDL_RegisterEvents(1);

  // Init the processing timer.
  if (!SDL_AddTimer(patternSets[currentSet].parm.delay, TriggerFrameDraw, NULL)) {
    fprintf(stderr, "Can't initialize the processing timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // FPS counter.
  if (!SDL_AddTimer(1000, TriggerFrameCount, NULL)) {
    fprintf(stderr, "Can't initialize the frame count timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // GUI update timer.  Normally the gui is updated by the preview frame update,
  // but this can be reduced in speed.  To ensure that the gui stays responsive
  // between frames, there is a third timer for this purpose.
  if (!SDL_AddTimer(GUIFRAMEDELAY, TriggerGUIUpdate, NULL)) {
    fprintf(stderr, "Can't initialize the gui update timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Main program loop...
  FOREVER {

    // Act on queued events.
    while (SDL_PollEvent(&event)) {

      switch(event.type) {

        case SDL_KEYDOWN:
          // Evaluate the keypress.
          exitProgram = HandleKey(event.key.keysym.sym, event.key.keysym.mod);
          break;

        case SDL_MOUSEMOTION:
          // The mouse moved.  See where it is.
          x = event.motion.x;
          y = event.motion.y;

          // Check if its hovering over a command.
          thisHover = INVALID;
          for (i = 0; i < DISPLAY_COMMAND_SIZE; i++) {
            // box is the rectangle encompassing the command text.  We could
            // precompute these if timing were important.
            box.x = colToPixel[mouseCommands[i].col];
            box.y = mouseCommands[i].line * DISPLAY_TEXT_HEIGHT;
            box.w = colToPixel[mouseCommands[i].col + 1] - box.x;
            box.h = (mouseCommands[i].line + 1) * DISPLAY_TEXT_HEIGHT - box.y + 1;

            // Is it in the rectangle of command i?
            if ((y >= box.y) && (y < box.y + box.h)) {
              if ((x >= box.x) && (x < box.x + box.w)) {

                // Yep.
                thisHover = i;

                // Is it hovering over a command is wasn't hovering over before?
                if ((!SameRectangle(box, boxOld)) || (lastHover == INVALID)) {

                  // Yeah, so draw the new highlight.
                  DrawBox(box.x, box.y, box.w, box.h, DISPLAY_COLOR_TEXTSBG_HL);
                  WriteCommand(i, mouseCommands, DISPLAY_COLOR_TEXTS_HL, DISPLAY_COLOR_TEXTSBG_HL);

                  // And if it came off a different command, remove that highlight.
                  if (lastHover != INVALID) {
                    DrawBox(boxOld.x, boxOld.y, boxOld.w, boxOld.h, DISPLAY_COLOR_TEXTSBG);
                    WriteCommand(lastHover, mouseCommands, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
                  }

                  // Keep track for next time.
                  boxOld = box;
                  lastHover = thisHover;

                  // Break the loop.  Can't be hovering over multiple commands.
                  break;
                }
              }
            }
          }

          // Not over a new command? May have to clear the old highlight anyway.
          if ((thisHover == INVALID) && (lastHover != INVALID)) {
            DrawBox(boxOld.x, boxOld.y, boxOld.w, boxOld.h, DISPLAY_COLOR_TEXTSBG);
            WriteCommand(lastHover, mouseCommands, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
            lastHover = INVALID;
          }
          break;

        case SDL_MOUSEWHEEL:
          // The mouse wheel moved.  See if we should act on that.
          if (thisHover != INVALID) {

            // Wheel down.
            if (event.wheel.y < 0) {

              // If there are no mouse wheel commands for this item, consider it a click.
              if (mouseCommands[thisHover].commands[MOUSE_WHEEL_DOWN].command == COM_NONE) {
                exitProgram = HandleCommand(mouseCommands[thisHover].commands[MOUSE_CLICK].command);
              } else {
                exitProgram = HandleCommand(mouseCommands[thisHover].commands[MOUSE_WHEEL_DOWN].command);
              }

            // Wheel up.
            } else {

              // If there are no mouse wheel commands for this item, consider it a click.
              if (mouseCommands[thisHover].commands[MOUSE_WHEEL_UP].command == COM_NONE) {
                exitProgram = HandleCommand(mouseCommands[thisHover].commands[MOUSE_CLICK].command);
              } else {
                exitProgram = HandleCommand(mouseCommands[thisHover].commands[MOUSE_WHEEL_UP].command);
              }
            }
          }
          break;

        case SDL_MOUSEBUTTONUP:
          // Mouse button unpushed.  Consider this a click.  If we're over
          // the same item we down clicked on, execute a command.
          if ((thisHover != INVALID) && (thisHover == mouseDownOn)) {
            exitProgram = HandleCommand(mouseCommands[thisHover].commands[MOUSE_CLICK].command);
          }
          break;

        case SDL_MOUSEBUTTONDOWN:
          // Mouse button pushed.  Make a note of the item it was pushed over.
          mouseDownOn = thisHover;
          break;

        case SDL_QUIT:
          // Window closed or <ctrl> c pushed in terminal.  Exit program.
          exitProgram = 1;
          break;

        case SDL_WINDOWEVENT:
          // We care about the window events that destroy our display.  Those
          // are probably resize and expose events.  We'll redraw the whole
          // display if they occur.
          if ((event.window.event == SDL_WINDOWEVENT_RESIZED) ||
              (event.window.event == SDL_WINDOWEVENT_EXPOSED)) {
            ClearWindow();
            InitDisplayTexts();
            DrawPreviewBorder(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y);
            //~ DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);
          }
          break;

        default:
          // There are two registered user events to check for here.  They can't
          // be tested in the case statement because they are not compile-time
          // integer constants.
          if (event.type == DRAWEventType) {
            // The frame timer expired.  Set a new frame to draw.
            drawNewFrame = YES;

          } else if (event.type == FPSEventType) {
            // The fps timer expired.  Set fps and reset the frame count.
            previewFPS = previewFrameCount;
            guiFPS = guiFrameCount;
            guiFrameCount = 0;
            previewFrameCount = 0;

          } else if (event.type == GUIEventType) {
            // Update the text display.
            UpdateInfoDisplay(&patternSets[currentSet], currentSet);
            UpdateGUI();
            guiFrameCount++;

          } else {
            //~ fprintf(stderr, "Unhandled SDL event: %i\n", event.type);
          }
          break;
      }
    }

    // Draw a new frame once every n ms, where n defaults to 60.
    if (drawNewFrame) {
      drawNewFrame = NO;
      previewFrameCount++;
      DrawNewFrame();

    } else {
      // Large delays make the CPU spin. Its because we;re stuck in a tight
      // loop, polling for events that never occur.  Best to get out of the way...
      if (patternSets[currentSet].parm.delay > 10) {

        // Idle the processor for 1 ms.
        nanosleep((struct timespec[]) {{0,1000000}}, NULL);
      }
    }

    if (exitProgram) break;

  } // End FOREVER program loop

  // Clean up a bit and exit.
  TTF_CloseFont(screenFont);
  screenFont = NULL; // to be safe...?
  TTF_Quit();
  exit(EXIT_SUCCESS);  // Is it?
}

// Event that triggers a frame to be drawn.
Uint32 TriggerFrameDraw(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = DRAWEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(patternSets[currentSet].parm.delay);
}

// Event that triggers the fps to be updated.
Uint32 TriggerFrameCount(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for fps measurement, and push it to the queue.
  SDL_zero(event);
  event.type = FPSEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the interval starts the timer again for the same period.
  return(interval);
}

// Gui update timer event pusher
Uint32 TriggerGUIUpdate(Uint32 interval, void *param) {
    SDL_Event event;

  // Make a new event for fps measurement, and push it to the queue.
  SDL_zero(event);
  event.type = GUIEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the interval starts the timer again for the same period.
  return(interval);
}

// The thing that happens at every frame.
void DrawNewFrame(void) {
  // Update the buffer. (I.E. make pattern)
  ProcessModes();

  // Update the preview and tensor.
  UpdateDisplays(global_intensity_limit, &patternSets[currentSet]);
}

// Time to make a mess of the array.
void ProcessModes(void) {
  static int cycleFrameCount = 0;    // Frame Count.
  static dir_e scrDir = DIR_UP;

  // Cycles through the pattern sets.  Allows you to set up a bunch of pattern
  // sets and switch between them one at a time at some interval.
  if (cyclePatternSets) {
    cycleFrameCount++;
    if (cycleFrameCount >= CYCLE_FRAME_LIMIT) {
      cycleFrameCount = 0;

      currentSet = (currentSet + 1) % PATTERN_SET_COUNT;
    }
  }

  // Change foreground color.
  if (patternSets[currentSet].mode.cycleForeground) {
    ColorCycle(&patternSets[currentSet].parm, FOREGROUND);
  }

  // Change background color.
  if (patternSets[currentSet].mode.cycleBackground) {
    ColorCycle(&patternSets[currentSet].parm, BACKGROUND);
  }

  // Seed the entire array with the foreground color.
  if (patternSets[currentSet].mode.colorAll) {
    ColorAll(patternSets[currentSet].parm.fg, patternSets[currentSet].fb);
    patternSets[currentSet].mode.colorAll = NO;
  }

  // Scroller.
  if (patternSets[currentSet].mode.scroller) {
    Scroll(patternSets[currentSet].parm.scrollDirection, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_ALL);
  }

  // Scroll red.
  if (patternSets[currentSet].mode.shiftRed) {
    Scroll(patternSets[currentSet].mode.shiftRed - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_RED);
  }

  // Scroll green.
  if (patternSets[currentSet].mode.shiftGreen) {
    Scroll(patternSets[currentSet].mode.shiftGreen - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_GREEN);
  }

  // Scroll blue.
  if (patternSets[currentSet].mode.shiftBlue) {
    Scroll(patternSets[currentSet].mode.shiftBlue - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_BLUE);
  }

  // Scroll blue and green
  if (patternSets[currentSet].mode.shiftCyan) {
      Scroll(patternSets[currentSet].mode.shiftCyan - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_CYAN);
  }

  // Scroll red and blue.
  if (patternSets[currentSet].mode.shiftMagenta) {
    Scroll(patternSets[currentSet].mode.shiftMagenta - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_MAGENTA);
  }

  // Scroll green and red.
  if (patternSets[currentSet].mode.shiftYellow) {
    Scroll(patternSets[currentSet].mode.shiftYellow - 1, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_YELLOW);
  }

  // Draw a solid bar up the side we are scrolling from.
  if (patternSets[currentSet].mode.sidebar) {
    DrawSideBar(&patternSets[currentSet]);
  }

  // First stab experimental image drawing.  Needs work.
  if (patternSets[currentSet].mode.postImage) {
    DrawImage(patternSets[currentSet].parm.postRotationAngle, patternSets[currentSet].parm.expand, patternSets[currentSet].mode.alias, patternSets[currentSet].fb);
    patternSets[currentSet].parm.postRotationAngle = patternSets[currentSet].parm.postRotationAngle + patternSets[currentSet].parm.postRotationIncr;
  }

  // I think this just writes a column of text on the right or left.
  if (patternSets[currentSet].mode.textSeed) {
    // Scroll provided by scroller if its on.
    WriteSlice(&patternSets[currentSet]);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (patternSets[currentSet].mode.cellAutoFun) {
    // Give each pixel a color value.
    CellFun(&patternSets[currentSet]);
  }

  // Bouncy bouncy (ick).
  if (patternSets[currentSet].mode.bouncer) {
    scrDir = (scrDir + 1) % 4;
    Scroll(scrDir, patternSets[currentSet].mode.rollOver, patternSets[currentSet].fb, PLANE_ALL);
  }

  // Bam!  Draw some horizontal bars.
  if (patternSets[currentSet].mode.horizontalBars) {
    HorizontalBars(patternSets[currentSet].parm.fg, patternSets[currentSet].fb);
    patternSets[currentSet].mode.horizontalBars = NO;
  }

  // Bam!
  if (patternSets[currentSet].mode.verticalBars) {
    VerticalBars(patternSets[currentSet].parm.fg, patternSets[currentSet].fb);
    patternSets[currentSet].mode.verticalBars = NO;
  }

  // Random dots.  Most useful seed ever.
  if (patternSets[currentSet].mode.randomDots) {
    RandomDots(patternSets[currentSet].parm.fg, patternSets[currentSet].parm.dotRandomness, patternSets[currentSet].fb);
  }

  // Fader
  if (patternSets[currentSet].mode.fadeout) {
    FadeAll(patternSets[currentSet].parm.fadeAllIncr, patternSets[currentSet].mode.fadeMode, patternSets[currentSet].fb);
  }

  // Averager
  if (patternSets[currentSet].mode.diffuse) {
    Diffuse(patternSets[currentSet].parm.diffusionCoef, patternSets[currentSet].fb);
  }

  // Clear screen.
  if (patternSets[currentSet].mode.clearAll) {
    ColorAll(patternSets[currentSet].parm.bg, patternSets[currentSet].fb);
    patternSets[currentSet].mode.clearAll = NO;
  }

  // Multiplier
  if (patternSets[currentSet].mode.multiply) {
    Multiply(patternSets[currentSet].parm.colorMultiplier, patternSets[currentSet].fb);
  }

  // Experimental Rotozoomer
  if (patternSets[currentSet].mode.preRotateZoom) {
    Rotate(patternSets[currentSet].parm.preRotationAngle, patternSets[currentSet].parm.expand, patternSets[currentSet].mode.alias, patternSets[currentSet].fb, patternSets[currentSet].fb);
  }

  // Zero the red.
  if (patternSets[currentSet].mode.clearRed) {
    ClearRed(&patternSets[currentSet]);
  }

  // Zero the blue.
  if (patternSets[currentSet].mode.clearBlue) {
    ClearBlue(&patternSets[currentSet]);
  }

  // Zero the green.
  if (patternSets[currentSet].mode.clearGreen) {
    ClearGreen(&patternSets[currentSet]);
  }
}

// Key press processing.
int HandleKey(SDL_Keycode key, SDL_Keymod mod) {
  textBuffer_t *text = &patternSets[currentSet].text;
  int i, j;

  // Prolly not necessary.
  if (key == SDLK_UNKNOWN) return 0;

  // To make exact comparisons to mod, left or right equivalents of modifier
  // keys must both be set if one is set.
  if (mod & KMOD_CTRL) mod |= KMOD_CTRL;
  if (mod & KMOD_ALT) mod |= KMOD_ALT;
  if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;

  // Check to see if the key combination activates a command.
  for ( i = 0 ; i < DISPLAY_COMMAND_SIZE; i++) {
    for (j = 0; j < MOUSE_COUNT; j++) {
      if ((mouseCommands[i].commands[j].key == key) && (mouseCommands[i].commands[j].mod == mod)) {
        return(HandleCommand(mouseCommands[i].commands[j].command));
      }
    }
  }

  // Check other place for commands.
  for ( i = 0 ; i < OTHER_COMMAND_SIZE; i++) {
    for (j = 0 ; j < MOUSE_COUNT; j++) {
      if ((otherCommands[i].commands[j].key == key) && (otherCommands[i].commands[j].mod == mod)) {
        return(HandleCommand(otherCommands[i].commands[j].command));
      }
    }
  }

  // If no command by now, then the key prolly goes in the text buffer.  First,
  // make sure we are dealing with printable characters:
  if ((key <= '~' && key >= ' ') && (!(mod & KMOD_ALT)) && (!(mod & KMOD_CTRL))) {
    if ((mod & KMOD_SHIFT) == KMOD_SHIFT) {
      // Keys with shift held down.
      if (key <= SDLK_z && key >= SDLK_a) {
        // Capitalize for a - z.
        text->textBuffer[text->textIndex] = key - ('a' - 'A');
      } else {
        // Lookup the symbols for the rest of the keys.
        switch (key) {
          case SDLK_1: text->textBuffer[text->textIndex] = '!'; break;
          case SDLK_2: text->textBuffer[text->textIndex] = '@'; break;
          case SDLK_3: text->textBuffer[text->textIndex] = '#'; break;
          case SDLK_4: text->textBuffer[text->textIndex] = '$'; break;
          case SDLK_5: text->textBuffer[text->textIndex] = '%'; break;
          case SDLK_6: text->textBuffer[text->textIndex] = '^'; break;
          case SDLK_7: text->textBuffer[text->textIndex] = '&'; break;
          case SDLK_8: text->textBuffer[text->textIndex] = '*'; break;
          case SDLK_9: text->textBuffer[text->textIndex] = '('; break;
          case SDLK_0: text->textBuffer[text->textIndex] = ')'; break;
          case SDLK_BACKSLASH: text->textBuffer[text->textIndex] = '|'; break;
          case SDLK_BACKQUOTE: text->textBuffer[text->textIndex] = '~'; break;
          case SDLK_MINUS: text->textBuffer[text->textIndex] = '_'; break;
          case SDLK_EQUALS: text->textBuffer[text->textIndex] = '+'; break;
          case SDLK_LEFTBRACKET: text->textBuffer[text->textIndex] = '{'; break;
          case SDLK_RIGHTBRACKET: text->textBuffer[text->textIndex] = '}'; break;
          case SDLK_SEMICOLON: text->textBuffer[text->textIndex] = ':'; break;
          case SDLK_COMMA: text->textBuffer[text->textIndex] = '<'; break;
          case SDLK_PERIOD: text->textBuffer[text->textIndex] = '>'; break;
          case SDLK_SLASH: text->textBuffer[text->textIndex] = '?'; break;
          case SDLK_QUOTE: text->textBuffer[text->textIndex] = '"'; break;
          default: break;
        }
      }
    } else {
      // Unmodified key entry.  We'll treat them as ascii in the printable range.
      text->textBuffer[text->textIndex] = key;
    }

    // Advance the terminating null and increase the buffer size.
    text->textBuffer[text->textIndex + 1] = 0x00;
    text->textIndex++;
    if (text->textIndex >= (sizeof(text->textBuffer) - 2)) text->textIndex--;
  }

  return 0;
}

// Executes a user command - toggles flags, adjusts parameters, etc.
int HandleCommand(command_e command) {
  modes_t *mode = &patternSets[currentSet].mode;
  textBuffer_t *text = &patternSets[currentSet].text;
  parms_t *parms = &patternSets[currentSet].parm;

  switch(command) {
    case COM_CELL: mode->cellAutoFun = !mode->cellAutoFun; break;
    case COM_TEXT_FLIP: mode->fontFlip = !mode->fontFlip; break;
    case COM_TEXT_REVERSE: mode->fontDirection = !mode->fontDirection; break;
    case COM_TEXT: mode->textSeed = !mode->textSeed; break;
    case COM_BOUNCE: mode->bouncer = !mode->bouncer; break;
    case COM_FADE: mode->fadeout = !mode->fadeout; break;
    case COM_DIFFUSE: mode->diffuse = !mode->diffuse; break;
    case COM_ROLL: mode->rollOver = !mode->rollOver; break;
    case COM_SCROLL: mode->scroller = !mode->scroller; break;
    case COM_HBAR: mode->horizontalBars = YES; break;
    case COM_VBAR: mode->verticalBars = YES; break;
    case COM_FGALL: mode->colorAll = YES; break;
    case COM_BGALL: mode->clearAll = YES; break;
    case COM_RDOT: mode->randomDots = !mode->randomDots; break;
    case COM_FGCYCLE: mode->cycleForeground = !mode->cycleForeground; break;
    case COM_BGCYCLE: mode->cycleBackground = !mode->cycleBackground; break;
    case COM_CYCLESET: cyclePatternSets = !cyclePatternSets; break;
    case COM_MODEOFF:
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
      mode->randomDots = NO;
      mode->rollOver = NO;
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
    case COM_LOADSET0: currentSet = 0; break;
    case COM_LOADSET1: currentSet = 1; break;
    case COM_LOADSET2: currentSet = 2; break;
    case COM_LOADSET3: currentSet = 3; break;
    case COM_LOADSET4: currentSet = 4; break;
    case COM_LOADSET5: currentSet = 5; break;
    case COM_LOADSET6: currentSet = 6; break;
    case COM_LOADSET7: currentSet = 7; break;
    case COM_LOADSET8: currentSet = 8; break;
    case COM_LOADSET9: currentSet = 9; break;
    case COM_FADEMODE: mode->fadeMode = !mode->fadeMode; break;
    case COM_NORED: mode->clearRed = !mode->clearRed; break;
    case COM_NOGREEN: mode->clearGreen = !mode->clearGreen; break;
    case COM_NOBLUE: mode->clearBlue = !mode->clearBlue; break;
    case COM_TEXTRESET: text->pixelIndex = -1; break;
    case COM_POSTROTATE:
      mode->postRotateZoom = !mode->postRotateZoom;
      parms->postRotationAngle = 0;
      break;
    case COM_PREROTATE: mode->preRotateZoom = !mode->preRotateZoom; break;
    case COM_AA: mode->alias = !mode->alias; break;
    case COM_MULTIPLY: mode->multiply = !mode->multiply; break;
    case COM_SIDEBAR: mode->sidebar = !mode->sidebar; break;
    case COM_ORIENTATION:
      tensor_landscape_p = !tensor_landscape_p;
      SetDims();
      DrawPreviewBorder(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y);
      //~ DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);
      break;
    case COM_TEXT_MODE_UP: mode->textStagger = (mode->textStagger + 1) % TS_COUNT; break;
    case COM_TEXT_MODE_DOWN:
      mode->textStagger--;
      if (mode->textStagger < 0) mode->textStagger = TS_COUNT - 1;
      break;
    case COM_RP_UP: mode->shiftRed = (mode->shiftRed + 1) % SM_COUNT; break;
    case COM_RP_DOWN:
      mode->shiftRed--;
      if (mode->shiftRed < 0) mode->shiftRed = SM_COUNT - 1;
      break;
    case COM_GP_UP: mode->shiftGreen = (mode->shiftGreen + 1) % SM_COUNT; break;
    case COM_GP_DOWN:
      mode->shiftGreen--;
      if (mode->shiftGreen < 0) mode->shiftGreen = SM_COUNT - 1;
      break;
    case COM_BP_UP: mode->shiftBlue = (mode->shiftBlue + 1) % SM_COUNT; break;
    case COM_BP_DOWN:
      mode->shiftBlue--;
      if (mode->shiftBlue < 0) mode->shiftBlue = SM_COUNT - 1;
      break;
    case COM_FG_DEC:
      parms->foreground--;
      if (parms->foreground < CE_RED) parms->foreground = CE_COUNT - 1;
      parms->fg = namedPalette[parms->foreground].color;
      break;
    case COM_FG_INC:
      parms->foreground = (parms->foreground + 1) % CE_COUNT;
      parms->fg = namedPalette[parms->foreground].color;
      break;
    case COM_BG_DEC:
      parms->background--;
      if (parms->background < 0) parms->background = CE_COUNT - 1;
      parms->bg = namedPalette[parms->background].color;
      break;
    case COM_BG_INC:
      parms->background = (parms->background + 1) % CE_COUNT;
      parms->bg = namedPalette[parms->background].color;
      break;
    case COM_COPYSET0: memcpy(&patternSets[0], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET1: memcpy(&patternSets[1], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET2: memcpy(&patternSets[2], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET3: memcpy(&patternSets[3], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET4: memcpy(&patternSets[4], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET5: memcpy(&patternSets[5], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET6: memcpy(&patternSets[6], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET7: memcpy(&patternSets[7], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET8: memcpy(&patternSets[8], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_COPYSET9: memcpy(&patternSets[9], &patternSets[currentSet], sizeof(patternSet_t)); break;
    case COM_TEXTO_DEC:
      parms->textOffset--;
      if (parms->textOffset < 0) {
        if ((parms->scrollDirection == DIR_LEFT) || (parms->scrollDirection == DIR_RIGHT)) {
          parms->textOffset = tensorHeight - 1;
        } else {
          parms->textOffset = tensorWidth - 1;
        }
      }
      break;
    case COM_TEXTO_INC:
      parms->textOffset++;
      if ((parms->scrollDirection == DIR_LEFT) || (parms->scrollDirection == DIR_RIGHT)) {
        parms->textOffset %= tensorHeight;
      } else {
        parms->textOffset %= tensorWidth;
      }
      break;
    case COM_DELAY_DEC:
      parms->delay--;
      if (parms->delay < 1) parms->delay = 1;  // Can't be 0!
      break;
    case COM_DELAY_RST:
      parms->delay = INITIAL_DELAY;
      break;
    case COM_DELAY_INC:
      parms->delay++;
      break;
    case COM_CP_UP: mode->shiftCyan = (mode->shiftCyan + 1) % SM_COUNT; break;
    case COM_CP_DOWN:
      mode->shiftCyan--;
      if (mode->shiftCyan < 0) mode->shiftCyan = SM_COUNT - 1;
      break;
    case COM_YP_UP: mode->shiftYellow = (mode->shiftYellow + 1) % SM_COUNT; break;
    case COM_YP_DOWN:
      mode->shiftYellow--;
      if (mode->shiftYellow < 0) mode->shiftYellow = SM_COUNT - 1;
      break;
    case COM_MP_UP: mode->shiftMagenta = (mode->shiftMagenta + 1) % SM_COUNT; break;
    case COM_MP_DOWN:
      mode->shiftMagenta--;
      if (mode->shiftMagenta < 0) mode->shiftMagenta = SM_COUNT - 1;
      break;
    case COM_IMAGE: mode->postImage = !mode->postImage; break;
    case COM_FADE_DEC: parms->fadeAllIncr--; break;
    case COM_FADE_INC: parms->fadeAllIncr++; break;
    case COM_FADE_RST: parms->fadeAllIncr = INITIAL_FADEOUT_DEC; break;
    case COM_DIFFUSE_DEC: parms->diffusionCoef -= parms->floatIncr; break;
    case COM_DIFFUSE_RST: parms->diffusionCoef = INITIAL_DIFF_COEF; break;
    case COM_DIFFUSE_INC: parms->diffusionCoef += parms->floatIncr; break;
    case COM_EXPAND_DEC: parms->expand -= parms->floatIncr; break;
    case COM_EXPAND_RST: parms->expand = INITIAL_EXPAND; break;
    case COM_EXPAND_INC: parms->expand += parms->floatIncr; break;
    case COM_RANDOM_DEC:
      parms->dotRandomness -= max(1, parms->dotRandomness / 50);
      if (parms->dotRandomness < 1) parms->dotRandomness = 1;
      break;
    case COM_RANDOM_RST: parms->dotRandomness = INITIAL_RAND_MOD; break;
    case COM_RANDOM_INC:
      parms->dotRandomness += max(1, parms->dotRandomness / 50);
      if (parms->dotRandomness > 20000) parms->dotRandomness = 20000;
      break;
    case COM_POSTROT_DEC: parms->postRotationAngle -= parms->floatIncr; break;
    case COM_POSTROT_RST: parms->postRotationAngle = INITIAL_POSTROT_ANGLE; break;
    case COM_POSTROT_INC: parms->postRotationAngle += parms->floatIncr; break;
    case COM_LOAD0: memcpy(patternSets[currentSet].fb, patternSets[0].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD1: memcpy(patternSets[currentSet].fb, patternSets[1].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD2: memcpy(patternSets[currentSet].fb, patternSets[2].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD3: memcpy(patternSets[currentSet].fb, patternSets[3].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD4: memcpy(patternSets[currentSet].fb, patternSets[4].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD5: memcpy(patternSets[currentSet].fb, patternSets[5].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD6: memcpy(patternSets[currentSet].fb, patternSets[6].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD7: memcpy(patternSets[currentSet].fb, patternSets[7].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD8: memcpy(patternSets[currentSet].fb, patternSets[8].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_LOAD9: memcpy(patternSets[currentSet].fb, patternSets[9].fb, sizeof(unsigned char) * TENSOR_BYTES); break;
    case COM_RAINDBOW_DEC: parms->rainbowInc--; break;
    case COM_RAINDBOW_RST: parms->rainbowInc = INITIAL_RAINBOW_INC; break;
    case COM_RAINDBOW_INC: parms->rainbowInc++; break;
    case COM_CYCLE_MODE: parms->colorCycleMode = (parms->colorCycleMode + 1) % CM_COUNT; break;
    case COM_CYCLE_MODE_DOWN:
      parms->colorCycleMode--;
      if (parms->colorCycleMode < 0) parms->colorCycleMode = CM_COUNT - 1;
      break;
    case COM_PREROT_DEC: parms->preRotationAngle-= parms->floatIncr; break;
    case COM_PREROT_RST: parms->preRotationAngle = INITIAL_PREROT_ANGLE; break;
    case COM_PREROT_INC: parms->preRotationAngle+= parms->floatIncr; break;
    case COM_SCROLL_UPC:
      parms->scrollDirection = DIR_UP;
      mode->scroller = YES;
      break;
    case COM_SCROLL_DOWNC:
      parms->scrollDirection = DIR_DOWN;
      mode->scroller = YES;
      break;
    case COM_SCROLL_LEFTC:
      parms->scrollDirection = DIR_LEFT;
      mode->scroller = YES;
      break;
    case COM_SCROLL_RIGHTC:
      parms->scrollDirection = DIR_RIGHT;
      mode->scroller = YES;
      break;
    case COM_SCROLL_CYCLE_UP:
      parms->scrollDirection = (parms->scrollDirection + 1) % DIR_COUNT;
      mode->scroller = YES;
      break;
    case COM_SCROLL_CYCLE_DOWN:
      parms->scrollDirection--;
      if (parms->scrollDirection < 0) parms->scrollDirection = DIR_COUNT - 1;
      mode->scroller = YES;
      break;
    case COM_MULT_DEC: parms->colorMultiplier -= parms->floatIncr; break;
    case COM_MULT_RST: parms->colorMultiplier = INITIAL_COLOR_MULTIPLIER; break;
    case COM_MULT_INC: parms->colorMultiplier += parms->floatIncr; break;
    case COM_POSTSPEED_DEC: parms->postRotationIncr -= parms->floatIncr; break;
    case COM_POSTSPEED_RST: parms->postRotationIncr = INITIAL_POSTROT_INC; break;
    case COM_POSTSPEED_INC: parms->postRotationIncr += parms->floatIncr; break;
    case COM_STEP_INC: parms->floatIncr *= 10; break;
    case COM_STEP_RST: parms->floatIncr = INITIAL_FLOAT_INC; break;
    case COM_STEP_DEC: parms->floatIncr /= 10; break;
    case COM_SCROLL_UP: parms->scrollDirection = DIR_UP; break;
    case COM_SCROLL_DOWN: parms->scrollDirection = DIR_DOWN; break;
    case COM_SCROLL_LEFT: parms->scrollDirection = DIR_LEFT; break;
    case COM_SCROLL_RIGHT: parms->scrollDirection = DIR_RIGHT; break;
    case COM_EXIT: return 1;
    case COM_BACKSPACE:
      text->textIndex--;
      if (text->textIndex < 0) text->textIndex = 0;
      text->textBuffer[text->textIndex] = 0x00;
      break;
    case COM_RETURN:
      text->textBuffer[text->textIndex] = '\n';
      text->textBuffer[text->textIndex + 1] = 0x00;
      text->textIndex++;
      if (text->textIndex >= (sizeof(text->textBuffer) - 2)) {
        text->textIndex--;
      }
      break;
    case COM_DELETE:
      text->textIndex = 0;
      text->textBuffer[0] = 0x00;
      break;

    //~ // <alt> <shift>
    //~ } else if ((key_event->key.keysym.mod & KMOD_ALT) && (key_event->key.keysym.mod & KMOD_SHIFT)) {
    //~ SavePatternSet(&key_event->key, &patternSets[0], currentSet);

    //~ // <ctrl> <shift>
    //~ } else if ((key_event->key.keysym.mod & KMOD_CTRL) && (key_event->key.keysym.mod & KMOD_SHIFT)) {
    //~ LoadPatternSet(&key_event->key, &patternSets[0], NULL);

    case COM_INVALID: case COM_NONE: case COM_COUNT:
      break;
  }
  return 0;
}

// Weighted Averager.
void Diffuse(float diffusionCoeff, unsigned char *buffer) {
  int x,y,i,j,k,l;
  color_t colorTemp, finalColor[TENSOR_WIDTH * TENSOR_HEIGHT];
  float weightSumR, weightSumG, weightSumB;
  float divisor;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      divisor = 0;
      weightSumR = 0;
      weightSumG = 0;
      weightSumB = 0;

      for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
          k = x + i;
          l = y + j;
          if ((k >= 0) && (k < tensorWidth) && (l >= 0) && (l < tensorHeight)) {
            colorTemp = GetPixel(k, l, buffer);
            if ((i == 0) && (j == 0)) {
              weightSumR += colorTemp.r;
              weightSumG += colorTemp.g;
              weightSumB += colorTemp.b;
              divisor = divisor + 1;
            } else {
              weightSumR += (diffusionCoeff * colorTemp.r);
              weightSumG += (diffusionCoeff * colorTemp.g);
              weightSumB += (diffusionCoeff * colorTemp.b);
              divisor += diffusionCoeff;
            }
          }
        }
      }

      finalColor[(y * tensorWidth) + x].r = weightSumR / divisor;
      finalColor[(y * tensorWidth) + x].g = weightSumG / divisor;
      finalColor[(y * tensorWidth) + x].b = weightSumB / divisor;
    }
  }

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      SetPixel(x,y,finalColor[(y * tensorWidth) + x], buffer);
    }
  }
}

// Color all the pixels a certain color.
void ColorAll(color_t color, unsigned char *fb) {
  int x,y;
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
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


// Lighten (or darker) all the pixels by a certain value.
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer) {
  int x, y, r, g, b;
  color_t colorTemp;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Grab the pixel color.
      colorTemp = GetPixel(x, y, buffer);

      if (fadeMode == FM_MODULAR) {
        // Modular color fade is easy regardless of inc sign.
        // Unsigned chars rollover in either direction.
        colorTemp.r += inc;
        colorTemp.g += inc;
        colorTemp.b += inc;
      } else {
        // For the limiter, we'll do the math with ints.
        r = colorTemp.r;
        g = colorTemp.g;
        b = colorTemp.b;
        r += inc;
        g += inc;
        b += inc;
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
  if ((x >= tensorWidth) || (y >= tensorHeight)) {
    fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
  }
  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Set a single pixel a particular color by color plane.
// Rediculous.
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer) {
  if ((x >= tensorWidth) || (y >= tensorHeight)) {
    fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
  }
  switch(plane) {
    case PLANE_ALL:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_RED:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      break;

    case PLANE_GREEN:
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_BLUE:
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    // More complicated.
    case PLANE_CYAN:
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_YELLOW:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_MAGENTA:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
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

  if ((x >= tensorWidth) || (y >= tensorHeight)) {
    fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
  }

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

  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Get the color of a particular pixel.
color_t GetPixel(int x, int y, unsigned char *buffer) {
  color_t colorTemp;
  colorTemp.r = buffer[(y * tensorWidth * 3) + (x * 3) + 0];
  colorTemp.g = buffer[(y * tensorWidth * 3) + (x * 3) + 1];
  colorTemp.b = buffer[(y * tensorWidth * 3) + (x * 3) + 2];
  colorTemp.a = 255; // We don't return an alpha for a written pixel?
  return colorTemp;
}





// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
void UpdateDisplays(float intensity_limit, patternSet_t *patternSet) {
  modes_t *mode = &patternSet->mode;
  parms_t *parms = &patternSet->parm;
  unsigned char *buffer = patternSet->fb;
  unsigned char fba[TENSOR_BYTES];
  int i;

  // Output rotate (doesn't effect array values, but does effect the final image).
  if (mode->postRotateZoom) {
    parms->postRotationAngle = parms->postRotationAngle + parms->postRotationIncr;
    Rotate(parms->postRotationAngle, parms->expand, mode->alias, &fba[0], &buffer[0]);

    UpdatePreview(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y, fba);
    //~ UpdatePreview(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y, fba);

    // Output diminish - no reason to do this to the preview (or is there?)
    for (i = 0; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
    }

  } else {
    UpdatePreview(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y, buffer);
    //~ UpdatePreview(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y, buffer);
    // Output diminish
    for (i = 0; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) buffer[i] * intensity_limit);
    }
  }

#ifdef USE_TENSOR
  UpdateTensor(fba);
#endif

  // I can't remember if this is necessary - it would be frame limiting, huh?
  // I think it has something to do with the UDP packets, or Tensor not liking
  // to be flooded.
  usleep(50000);

  return;
}


// Rotate
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src) {
  SDL_Surface *rotatedSurface;
  SDL_Rect offset;

  FBToSurface(scratchSurface, fb_src);

  // Rotate / scale it.
  rotatedSurface = rotozoomSurface (scratchSurface, angle, expansion, aliasmode ? 1 : 0);
  if (!rotatedSurface) {
    fprintf(stderr, "Error rotating surface: %s\n", SDL_GetError());
    return;
  }

  // Recenter and copy it back to the scratchSurface
  offset.x = 0 - (rotatedSurface->w - scratchSurface->w) / 2;
  offset.y = 0 - (rotatedSurface->h - scratchSurface->h) / 2;
  SDL_FillRect(scratchSurface, NULL, 0);
  SDL_BlitSurface(rotatedSurface, NULL, scratchSurface, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, scratchSurface);
  SDL_FreeSurface(rotatedSurface);
}


// Frame buffer to SDL surface
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB) {
  color_t pixel;
  unsigned char *surfacePixels;
  int x, y;

  surfacePixels = surface->pixels;
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

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

  for(x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      pixel = surface->pixels + y * surface->pitch + x * (surface->pitch / surface->w);
      SDL_GetRGB(*pixel, surface->format, &r, &g, &b);
      FB[((y * tensorWidth * 3) + (x * 3)) + 0] = r;
      FB[((y * tensorWidth * 3) + (x * 3)) + 1] = g;
      FB[((y * tensorWidth * 3) + (x * 3)) + 2] = b;
    }
  }

  return FB;
}


// Send frame buffer to preview area.  The preview area is a square
// encompassing the actual preview, so we should center the buffer within it.
void UpdatePreview(int xOffset, int yOffset, unsigned char *buffer) {
  color_t pixel;
  int x, y;
  int maxDim;

  // Get the outer border dimensions.
  x = (tensorWidth * PREVIEW_PIXEL_SIZE);
  y = (tensorHeight * PREVIEW_PIXEL_SIZE);

  // Get the preview area square dimension.
  maxDim = max(x, y);

  // Adjust our offsets to fit inside it.
  xOffset = xOffset + PREVIEW_BORDER_THICKNESS + (maxDim - x) / 2;
  yOffset = yOffset + PREVIEW_BORDER_THICKNESS + (maxDim - y) / 2;

  // Draw out the pixels.
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Get pixel color from the buffer.
      pixel = GetPixel(x, y, buffer);

      // Draw the output pixel as a square of dimension PREVIEW_PIXEL_SIZE - 1.
      // This leaves us with a border around our pixels.
      DrawBox(xOffset + (x * PREVIEW_PIXEL_SIZE), yOffset + (y * PREVIEW_PIXEL_SIZE),
              PREVIEW_PIXEL_SIZE - 1, PREVIEW_PIXEL_SIZE - 1, pixel);
    }
  }
}


// Scroller buffer manipulation
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane) {
  int x, y, i;
  color_t rollSave[TENSOR_WIDTH + TENSOR_HEIGHT];  // Size = Why?

  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case DIR_UP:
        for(i = 0; i < tensorWidth; i++) {
          rollSave[i] = GetPixel(i, 0, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < tensorWidth; i++) {
          rollSave[i] = GetPixel(i, tensorHeight - 1, fb);
        }
        break;

      case DIR_RIGHT:
        for (i = 0; i < tensorHeight; i++) {
          rollSave[i] = GetPixel(tensorWidth - 1, i, fb);
        }
      break;

      case DIR_LEFT:
        for (i = 0; i < tensorHeight; i++) {
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
      for (y = 0; y < (tensorHeight - 1); y++) {
        for (x = 0; x < tensorWidth; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y+1, fb), plane, fb);
        }
      }
      break;

    case DIR_DOWN:
      for (y = (tensorHeight - 1); y > 0; y--) {
        for (x = 0; x < tensorWidth; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y - 1, fb), plane, fb);
        }
      }
      break;

    case DIR_LEFT:
      for (y = 0; y < tensorHeight; y++) {
        for (x = 0; x < (tensorWidth - 1); x++) {
          SetPixelByPlane(x, y, GetPixel(x + 1, y, fb),plane, fb);
        }
      }
      break;

    case DIR_RIGHT:
    default:
      for (y = 0; y < tensorHeight; y++) {
        for (x = (tensorWidth - 1); x > 0; x--) {
          SetPixelByPlane(x, y, GetPixel(x - 1, y, fb),plane, fb);
        }
      }
      break;
  }

  // rollover mode?
  if (rollovermode == YES) {
    switch(direction) {
      case DIR_UP:
        for(i = 0; i < tensorWidth; i++) {
          SetPixelByPlane(i, tensorHeight - 1, rollSave[i], plane, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < tensorWidth; i++) {
          SetPixelByPlane(i, 0, rollSave[i],plane, fb);
        }
        break;

      case DIR_RIGHT:
        for (i = 0; i < tensorHeight; i++) {
          SetPixelByPlane(0, i, rollSave[i],plane, fb);
        }
      break;

      case DIR_LEFT:
        for (i = 0; i < tensorHeight; i++) {
          SetPixelByPlane(tensorWidth - 1, i, rollSave[i],plane, fb);
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
void WriteSlice(patternSet_t *patternSet) {
  const unsigned char charColMasks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  int sliceColOrRow;  // The seed target column / row (depending on direction).
  int sliceIndex;     // A pixel of the target slice we are working on.
  char character;     // The current character being output.
  int pixelBufferSize;  // Number of pixel columns in the text buffer.
  int fontPixelIndex;  // Index into the font to a selected pixel.
  int bufferIndex;  // Index into the text buffer of the current character.
  int charCol;  // Index into the font pixels column of the current character.
  int i, j;
  int useRand;  // The random number used during staggering (if at all).
  unsigned char horizontal;
  unsigned char pixelOn;
  unsigned char textDirection;

  // Initialize textStaggerFlag, if necessary.  This is used for 9 row stagger,
  // which is meant for an 8 pixel font height on a 9 pixel high display.
  if (patternSet->text.textStaggerFlag[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      patternSet->text.textStaggerFlag[i] = rand() % 2;
    }
  }


  // Figure out which row or column to place the seed slice into depending on direction.
  horizontal = YES;
  switch (patternSet->parm.scrollDirection) {
    case DIR_LEFT:
      sliceColOrRow = tensorWidth - 1;
      break;

    case DIR_UP:
      horizontal = NO;
      sliceColOrRow = tensorHeight - 1;
      break;

    case DIR_DOWN:
      horizontal = NO;
      // Fallthrough...
    case DIR_RIGHT:
    default:
      sliceColOrRow = 0;
      break;
  }

  // The pixel buffer correspends to the text buffer, but further subdivides
  // each text array index into a part for every pixel of the font width.
  pixelBufferSize = strlen(patternSet->text.textBuffer) * FONT_WIDTH;

  // Trivially, no text in the buffer.
  if (pixelBufferSize == 0) {
    // Just exit.  This actually causes the background to stop scrolling, which
    // might not be desireable.
    return;
  }

  // Prior to writing out a line, we increment the pixel buffer index to
  // point to the next part of the line to be written.  This depends on
  // scroll direction, text direction.
  textDirection = patternSet->parm.scrollDirection;
  if (patternSet->mode.fontDirection == BACKWARDS) {
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
  if (patternSet->text.prevScrollDir != patternSet->parm.scrollDirection) {

    // Looks like we changed direction.
    patternSet->text.prevScrollDir = patternSet->parm.scrollDirection;

    // tensorWidth is used here to preserve continuity of the text, even
    // across text buffer wrap-around, when the direction changes (but only
    // really between UP - DOWN or RIGHT - LEFT changes).
    switch(textDirection) {
      case DIR_LEFT:
        patternSet->text.pixelIndex = (patternSet->text.pixelIndex + tensorWidth) % pixelBufferSize;
        break;
      case DIR_RIGHT:
        patternSet->text.pixelIndex = patternSet->text.pixelIndex - tensorWidth;
        if (patternSet->text.pixelIndex < 0) {
          patternSet->text.pixelIndex = pixelBufferSize + patternSet->text.pixelIndex;
        }
        break;
      case DIR_DOWN:
        patternSet->text.pixelIndex = (patternSet->text.pixelIndex + tensorHeight) % pixelBufferSize;
        break;
      case DIR_UP:
        patternSet->text.pixelIndex = patternSet->text.pixelIndex - tensorHeight;
        if (patternSet->text.pixelIndex < 0) {
          patternSet->text.pixelIndex = pixelBufferSize + patternSet->text.pixelIndex;
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
        patternSet->text.pixelIndex = (patternSet->text.pixelIndex + 1) % pixelBufferSize;
        break;
      case DIR_RIGHT:
      case DIR_UP:
        patternSet->text.pixelIndex--;
        if ((patternSet->text.pixelIndex < 0) || (patternSet->text.pixelIndex >= pixelBufferSize)) {
          patternSet->text.pixelIndex = pixelBufferSize - 1;
        }
        break;
      default:
        break;
    }
  }

  // Now using the pixel index, we find out where in the text buffer we are.
  // (Integer division.)
  bufferIndex = patternSet->text.pixelIndex / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (patternSet->text.pixelIndex % FONT_WIDTH);

  // What the character is.  What it is, man.
  character = patternSet->text.textBuffer[bufferIndex];

  // First we draw the seed's background according to our textStagger mode.
  useRand = 0;
  switch (patternSet->mode.textStagger) {
    case TS_9ROWSTAGGER:
      // Stagger.  For landscape - 8 pixel font on 9 pixels high display.
      // Stagger the letter and fill in the pixel that isn't covered by the
      // letter with our background.
      sliceIndex = patternSet->parm.textOffset;
      if (!patternSet->text.textStaggerFlag[bufferIndex]) sliceIndex += 8;
      if (horizontal) {
        sliceIndex = sliceIndex % tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, patternSet->parm.bg, patternSet->fb);
      } else {
        sliceIndex = sliceIndex % tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, patternSet->parm.bg, patternSet->fb);
      }
      useRand = patternSet->text.textStaggerFlag[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = patternSet->parm.textOffset - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = tensorHeight - 1;
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, patternSet->parm.bg, patternSet->fb);
      } else {
        if (sliceIndex < 0) sliceIndex = tensorWidth - 1;
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, patternSet->parm.bg, patternSet->fb);
      }
      sliceIndex = patternSet->parm.textOffset + 8;
      if (horizontal) {
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, patternSet->parm.bg, patternSet->fb);
      } else {
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, patternSet->parm.bg, patternSet->fb);
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < tensorHeight; i++) {
          SetPixelA(sliceColOrRow, i, patternSet->parm.bg, patternSet->fb);
        }
      } else {
        for (i = 0; i < tensorWidth; i++) {
          SetPixelA(i, sliceColOrRow, patternSet->parm.bg, patternSet->fb);
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
    if (patternSet->mode.fontFlip) {
      j = (FONT_HEIGHT - 1) - i;
    } else {
      j = i;
    }
    fontPixelIndex = (j * FONT_CHARACTER_COUNT) + character;
    sliceIndex = i + useRand + patternSet->parm.textOffset;
    pixelOn = myfont[fontPixelIndex] & charColMasks[charCol];
    if (horizontal) {
      sliceIndex = sliceIndex % tensorHeight;
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? patternSet->parm.fg : patternSet->parm.bg, patternSet->fb);
    } else {
      sliceIndex = sliceIndex % tensorWidth;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? patternSet->parm.fg : patternSet->parm.bg, patternSet->fb);
    }
  }

  return;
}


void HorizontalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j += 2) {
      SetPixel(i, j, color, buffer);
    }
  }
}


void VerticalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i+=2) {
    for (j = 0; j < tensorHeight; j++) {
      SetPixel(i,j, color, buffer);
    }
  }
}


void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer) {
  int x,y;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      if (!(rand() % rFreq)) {
        SetPixel(x, y, color, buffer);
      }
    }
  }
}



void ColorCycle(parms_t *parms, int fb_mode) {
  color_t colorTemp = cBlack;
  int inpos, inposo;
  colorCycleModes_e cycleMode = parms->colorCycleMode;
  int *cycleSaver;
  int rainbowInc = parms->rainbowInc;

  // Position in the cycle must be saved in the patternSet to ensure reload is
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
void InitDisplayTexts(void) {
  int i;
  // DrawBox(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, cDkGray);
  for (i = 0; i < DISPLAY_COMMAND_SIZE; i++) {
    WriteCommand(i, mouseCommands, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
  }

  for (i = 0; i < HEAD_TEXT_SIZE; i++) {
    WriteLine(headerText[i].text, headerText[i].line, headerText[i].col, DISPLAY_COLOR_TITLES, DISPLAY_COLOR_TITLESBG);
  }

  for (i = 0; i < OTHER_COMMAND_SIZE; i++) {
    WriteCommand(i, otherCommands, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
  }

  for (i = 0; i < OTHER_TEXT_SIZE; i++) {
    WriteLine(otherText[i].text, otherText[i].line, otherText[i].col, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
  }
}

void WriteCommand(int index, const command_t *comList, color_t fg, color_t bg) {
  char text[100] = "";
  char *ind = text;
  char * const end = text + sizeof(text);
  int i;

  if (mouseCommands[index].line == -1) {
    return;
  }

  // Handle the modifier keys - worked well when we had only one modifier key
  // possible for each command, but now... Strategy: Show the modifiers from
  // the first command in the list that doesn't have KMOD_NONE, and assume it
  // will be the same for all three.
  for (i = 0; i < MOUSE_COUNT; i++) {
    if (comList[index].commands[i].mod != KMOD_NONE) {
      ind += snprintf(ind, end - ind, "%s%s%s",
        comList[index].commands[i].mod & KMOD_CTRL ? "<ctrl> " : "",
        comList[index].commands[i].mod & KMOD_SHIFT ? "<shft> " : "",
        comList[index].commands[i].mod & KMOD_ALT ? "<alt> " : "");
      break;  // Found the first, skip the rest.
    }
  }

  // Handle the key selection
  ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(comList[index].commands[MOUSE_WHEEL_UP].key),
    strlen(SDL_GetKeyName(comList[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
  ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(comList[index].commands[MOUSE_CLICK].key),
    strlen(SDL_GetKeyName(comList[index].commands[MOUSE_CLICK].key)) ? " " : "");
  ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(comList[index].commands[MOUSE_WHEEL_DOWN].key),
    strlen(SDL_GetKeyName(comList[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");

  if (strlen(text)) ind += snprintf(ind, end - ind, "- ");

  // Add the text from the mouseCommands structure.
  ind += snprintf(ind, end - ind, "%s", comList[index].text);

  // Write it.
  WriteLine(text, comList[index].line, comList[index].col, fg, bg);
}


// Update the values of the text on the display window.
void UpdateInfoDisplay(patternSet_t *patternSet, int setIndex) {

  char text[102], mybuff[102];
  int length;
  parms_t *parms = &patternSet->parm;
  modes_t *mode = &patternSet->mode;
  textBuffer_t *scrollText = &patternSet->text;

  // First column
  WriteInt(previewFPS, 21, 1, 3);
  WriteInt(guiFPS, 22, 1, 3);
  WriteBool(mode->cellAutoFun, 23, 1);
  WriteBool(mode->bouncer, 24, 1);
  WriteBool(mode->fadeout, 25, 1);
  WriteBool(mode->diffuse, 26, 1);
  WriteBool(mode->textSeed, 27, 1);
  WriteBool(mode->rollOver, 28, 1);
  WriteBool(mode->scroller, 29, 1);
  WriteBool(mode->randomDots, 34, 1);
  WriteBool(mode->cycleForeground, 35, 1);
  WriteBool(mode->cycleBackground, 36, 1);
  WriteBool(cyclePatternSets, 37, 1);
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
  WriteFloat(parms->floatIncr, 2, 3, 14, 6);
  WriteFloat(parms->diffusionCoef, 4, 3, 14, 6);
  WriteFloat(parms->expand, 5, 3, 14, 6);
  WriteInt(parms->fadeAllIncr, 6, 3, 14);
  WriteFloat(parms->preRotationAngle, 7, 3, 14, 6);
  WriteInt(parms->rainbowInc, 8, 3, 14);
  WriteFloat(parms->postRotationAngle, 9, 3, 14, 6);
  WriteFloat(parms->colorMultiplier, 10, 3, 14, 6);
  WriteFloat(parms->postRotationIncr, 11, 3, 14, 6);
  WriteFloat((1.0 / parms->dotRandomness), 12, 3, 14, 6);
  WriteString(colorCycleText[parms->colorCycleMode], 13, 3, 14);
  WriteString(dirText[parms->scrollDirection], 14, 3, 14);
  WriteInt(parms->delay, 15, 3, 14);
  WriteString(namedPalette[parms->foreground].name, 20, 3, 14);
  WriteString(namedPalette[parms->background].name, 21, 3, 14);
  WriteInt(setIndex, 26, 3, 14);
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
  WriteLine(text, 52, 0, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

void WriteBool(int value, int row, int col) {
  if (value) {
    WriteLine("YES", row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  } else {
    WriteLine(" NO", row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  }
}

void WriteInt(int value, int row, int col, int width) {
  char text[100];
  snprintf(text, sizeof(text), "%*i", width, value);
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

void WriteFloat(float value, int row, int col, int width, int precision) {
  char text[100];
  snprintf(text, sizeof(text), "%*.*f", width, precision, value);
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

void WriteString(const char *text, int line, int col, int width) {
  char temp[200];
  snprintf(temp, sizeof(temp), "%*.*s", width, width, text);
  WriteLine(temp, line, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

// Writes a line of text to the selected line and column of the
// output window with the selected color.
void WriteLine(char * thisText, int line, int col, color_t color, color_t bgColor) {

  // Vars
  const SDL_Color fontColor = {color.r, color.g, color.b};
  const SDL_Color fontbgColor = {bgColor.r, bgColor.g, bgColor.b};
  SDL_Surface *textSurface;
  SDL_Texture *textTexture;
  SDL_Rect rect;

  // Set the position of the output text.
  rect.x = colToPixel[col];
  rect.y = line * (DISPLAY_TEXT_HEIGHT);

  // Render the text to a surface.
  textSurface = TTF_RenderText_Shaded(screenFont, thisText, fontColor, fontbgColor);
  //~ textSurface = TTF_RenderText_Blended(screenFont, thisText, fontColor);//, bgColor);
  if (textSurface) {
    // Set the width and height of the output text.
    rect.w = textSurface->w;
    // rect.h = textSurface->h;
    rect.h = DISPLAY_TEXT_HEIGHT;

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

void UpdateGUI(void) {
  SDL_RenderPresent(mwRenderer);
}

void ClearWindow(void) {
  SDL_SetRenderDrawColor(mwRenderer, 0, 0, 0, 255);
  SDL_RenderClear(mwRenderer);
}

// Set some dimensions according to orientation
void SetDims(void) {
   if (tensor_landscape_p) {
    tensorWidth = TENSOR_HEIGHT;
    tensorHeight = TENSOR_WIDTH;
  } else {
    tensorWidth = TENSOR_WIDTH;
    tensorHeight = TENSOR_HEIGHT;
  }

  if (scratchSurface) SDL_FreeSurface(scratchSurface);

  scratchSurface = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight,
    32, 0, 0, 0, 0);
  if (!scratchSurface) {
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
  offset.x = 0 - (rotatedImage->w - scratchSurface->w) / 2;
  offset.y = 0 - (rotatedImage->h - scratchSurface->h) / 2;
  SDL_BlitSurface(rotatedImage, NULL, scratchSurface, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, scratchSurface);
  SDL_FreeSurface(rotatedImage);
}

// Draw the borders around the preview output.  We can switch between portrait
// and landscape, so the preview space is a square box that would accomodate
// either orientation.
void DrawPreviewBorder(int x, int y) {

  // Vars
  int w, h;
  int maxDim;
  int i;

  // Get the outer border dimensions.
  w = (tensorWidth * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_THICKNESS * 2);
  h = (tensorHeight * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER_THICKNESS * 2);

  // Preview space dimension.
  maxDim = max(w, h);

  // Erase the old preview.
  DrawBox(x, y, maxDim, maxDim, cBlack);
  DrawRectangle(x, y, maxDim, maxDim, cWhite);

  // Ajust x and y to center the preview.
  x = x + (maxDim - w) / 2;
  y = y + (maxDim - h) / 2;

  // Draw the new outer border.
  DrawRectangle(x, y, w, h, cWhite);

  // Get the inner border dimensions.
  w = (tensorWidth * PREVIEW_PIXEL_SIZE) + 1;
  h = (tensorHeight * PREVIEW_PIXEL_SIZE) + 1;

  if (tensor_landscape_p) {
    // Landscape - Draw horizontal panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(x + PREVIEW_BORDER_THICKNESS - 1, y + i * (h / 3) + PREVIEW_BORDER_THICKNESS - 1, w, (h / 3) + 1, cGray);
    }
  } else {
    // Portrait - Draw vertical panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(x + i * (w / 3) + PREVIEW_BORDER_THICKNESS - 1, y + PREVIEW_BORDER_THICKNESS - 1, (w / 3) + 1, h, cGray);
    }
  }
}

// Or not.
void CellFun(patternSet_t *patternSet) {
  int x, y;
  color_t pixelColor, oldColor;

  patternSet->parm.cellAutoCount++;

  for(x = 0 ; x < tensorWidth ; x++) {
    for(y = 0 ; y < tensorHeight ; y++) {
      oldColor = GetPixel(x, y, patternSet->fb);
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2);
      pixelColor.g = oldColor.g + pixelColor.r;
      pixelColor.b = patternSet->parm.cellAutoCount;
      SetPixel(x, y, pixelColor,  patternSet->fb);
    }
  }
}

void SavePatternSet(SDL_KeyboardEvent *key, patternSet_t *patternSet, int setIndex) {
  int thiskey = key->keysym.sym;
  FILE *fp;
  char filename[6];
  size_t count;

  if (((thiskey >= SDLK_0) && (thiskey <= SDLK_9)) ||
      ((thiskey >= SDLK_a) && (thiskey <= SDLK_z))) {

    snprintf(filename, sizeof(filename), "%s.now", SDL_GetKeyName(key->keysym.sym));
    printf( "Saving pattern set to file \"%s\"\n", filename);

    fp = fopen(filename, "wb");
    if (fp == NULL) {
      perror("Failed to open file.");
      return;
    }

    count = fwrite(patternSet, sizeof(patternSet_t), PATTERN_SET_COUNT, fp);
    printf("Wrote %i records of %lu bytes each for a total of %lu bytes.\n", (int) count, sizeof(patternSet_t), count * sizeof(patternSet_t));
    printf("fclose(fp) %s.\n", fclose(fp) == 0 ? "succeeded" : "failed");
  }
}

void LoadPatternSet(SDL_KeyboardEvent *key, patternSet_t *patternSet, char *fn) {
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

    printf( "Loading pattern set from file \"%s\"\n", filename);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
      perror("Failed to open file!");
      return;
    }

    count = fread(patternSet, sizeof(patternSet_t), PATTERN_SET_COUNT, fp);
    printf("Read %i records of %lu bytes each for a total of %lu bytes.\n", (int) count, sizeof(patternSet_t), count * sizeof(patternSet_t));
    printf("fclose(fp) %s.\n", fclose(fp) == 0 ? "succeeded" : "failed");
  }
}


void DrawSideBar(patternSet_t *patternSet) {
  int i;

  switch (patternSet->parm.scrollDirection) {
    case DIR_LEFT:
      for (i = 0; i < tensorHeight; i++) {
              SetPixel(tensorWidth - 1, i, patternSet->parm.fg, patternSet->fb);
            }
            break;

    case DIR_RIGHT:
      for (i = 0; i < tensorHeight; i++) {
              SetPixel(0, i, patternSet->parm.fg, patternSet->fb);
            }
            break;

    case DIR_UP:
      for (i = 0; i < tensorWidth; i++) {
              SetPixel(i, tensorHeight - 1, patternSet->parm.fg, patternSet->fb);
            }
            break;

    case DIR_DOWN:
      for (i = 0; i < tensorWidth; i++) {
              SetPixel(i, 0, patternSet->parm.fg, patternSet->fb);
            }
            break;


    default:
      break;
  }
}

void ClearRed(patternSet_t *currentMoment) {
  int i;
  for(i = 0; i < TENSOR_BYTES; i++) {
    currentMoment->fb[(i * 3) + 0] = 0;
  }
}

void ClearGreen(patternSet_t *currentMoment) {
  int i;
  for(i = 0; i < TENSOR_BYTES; i++) {
    currentMoment->fb[(i * 3) + 1] = 0;
  }
}

void ClearBlue(patternSet_t *currentMoment) {
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

unsigned char SameRectangle(SDL_Rect a, SDL_Rect b) {
  if ((a.x == b.x) && (a.y == b.y) && (a.w == b.w) && (a.h == b.h)) {
    return YES;
  } else {
    return NO;
  }
}
