// Tensor Pattern
// For Kevin (FB) McCormick (w/o you, there may be nothing) <3
// Blau
// tensor@core9.org

#define MAIN_C_

// Includes
#include <stdio.h> // File io
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
#include <string.h>  // memcpy
#include "version.h"
#include <errno.h>

// Defines
#define NO 0
#define YES 1
#define OFF 0
#define ON 1
//~ #define FOREGROUND 0
//~ #define BACKGROUND 1
#define FORWARDS 0
#define BACKWARDS 1
#define INVALID (-1)
#define FOREVER for(;;)
#define GUIFRAMEDELAY 25

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
#define INITIAL_MULTIPLIER 1.0
#define INITIAL_DIFF_COEF 0.002
#define INITIAL_FADE_INC -1
#define INITIAL_RAND_MOD 100
#define INITIAL_RAINBOW_INC 8
#define INITIAL_EXPAND 1
#define INITIAL_FLOAT_INC 0.1
#define INITIAL_POSTROT_INC 0
#define INITIAL_POSTROT_ANGLE 0
#define INITIAL_PREROT_ANGLE 0
#define INITIAL_DELAY 60
#define INITIAL_TEXT "Frostbyte was an engineer. "
#define INITIAL_DIR DIR_LEFT
#define INITIAL_PREEXPAND 1.0
#define INITIAL_FRAMECYCLECOUNT 1000
#define INITIAL_IMAGE_ANGLE 0.0
#define INITIAL_IMAGE_EXPAND 1.0
#define INITIAL_IMAGE_INC 0.0
#define INITIAL_IMAGE_XOFFSET 0.5
#define INITIAL_IMAGE_YOFFSET 0.5
#define INITIAL_IMAGE "Fbyte-01.jpg"

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
#define DISPLAY_COLOR_TBUF cRed
#define DISPLAY_COLOR_TBUFBG cBlack
#define DISPLAY_COLOR_INFO cGreen
#define DISPLAY_COLOR_INFOBG cBlack

// Named color palette structures and constants

// If you change color_e, change namedPalette and colorsText too.
typedef enum color_e {
  CE_INVALID = -1,
  CE_RED = 0, CE_ORANGE, CE_YELLOW, CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN,
  CE_AZURE, CE_BLUE, CE_VIOLET, CE_MAGENTA, CE_ROSE, CE_WHITE, CE_LTGRAY,
  CE_GRAY, CE_DKGRAY, CE_BLACK,
  CE_COUNT // Last.
} color_e;
// Palette typedef - the enumeration, the color constant
typedef struct palette_t {
  color_e index;
  color_t color;
} palette_t;
// This palette order should line up with color_e enumeration values, and colorsText.
const palette_t namedPalette[] = {
  { CE_RED,        CD_RED },
  { CE_ORANGE,     CD_ORANGE },
  { CE_YELLOW,     CD_YELLOW },
  { CE_CHARTREUSE, CD_CHARTREUSE },
  { CE_GREEN,      CD_GREEN },
  { CE_AQUA,       CD_AQUA },
  { CE_CYAN,       CD_CYAN },
  { CE_AZURE,      CD_AZURE },
  { CE_BLUE,       CD_BLUE },
  { CE_VIOLET,     CD_VIOLET },
  { CE_MAGENTA,    CD_MAGENTA },
  { CE_ROSE,       CD_ROSE },
  { CE_WHITE,      CD_WHITE },
  { CE_LTGRAY,     CD_LTGRAY },
  { CE_GRAY,       CD_GRAY },
  { CE_DKGRAY,     CD_DKGRAY },
  { CE_BLACK,      CD_BLACK }
};
#define NAMEDPALETTE_SIZE (sizeof(namedPalette) / sizeof(palette_t))
const char *colorsText[] = { "red", "orange", "yellow", "chartreuse", "green",
  "aqua", "cyan", "azure", "blue", "violet", "magenta", "rose", "white",
  "lt gray", "gray", "dk gray", "black" };
#define COLORSTEXT_SIZE (sizeof(colorsText) / sizeof(const char *))

// Enumerations and Strings.

// Scroller directions
typedef enum dir_e {
  DIR_INVALID = -1,
  DIR_UP = 0, DIR_LEFT, DIR_DOWN, DIR_RIGHT,
  DIR_COUNT // Last.
} dir_e;
const char *dirText[] = { "Up", "Left", "Down", "Right" };
#define DIRTEXT_SIZE (sizeof(dirText) / sizeof(const char *))

// Fade modes
typedef enum fadeModes_e{
  FM_INVALID = -1,
  FM_LIMIT = 0, FM_MODULAR,
  FM_COUNT // Last.
} fadeModes_e;
const char *fadeModeText[] = { "Limited", "Modular" };
#define FADEMODETEXT_SIZE (sizeof(fadeModeText) / sizeof(const char *))

// Color plane shift modes.
typedef enum shiftModes_e {
  SM_INVALID = -1,
  SM_HOLD = 0, SM_UP, SM_LEFT, SM_DOWN, SM_RIGHT,
  SM_COUNT // Last.
} shiftModes_e;
const char *shiftText[] = { "Hold", "Up", "Left", "Down", "Right" };
#define SHIFTTEXT_SIZE (sizeof(shiftText) / sizeof(const char *))

// Text background modes
typedef enum textMode_e {
  TS_INVALID = -1,
  TS_8ROW = 0, TS_9ROWSTAGGER, TS_10ROWBORDER, TS_FULLBG,
  TS_COUNT // Last.
} textMode_e;
const char *textModeText[] = { "8 Row", "9 Row Stag", "10 Row", "Full BG" };
#define TEXTMODETEXT_SIZE (sizeof(textModeText) / sizeof(const char *))

// Color palette cycle modes
typedef enum colorCycleModes_e{
  CM_INVALID = -1,
  CM_NONE = 0, CM_RGB, CM_CMY, CM_SECONDARY, CM_TERTIARY, CM_GRAY, CM_RAINBOW,
  CM_RANDOM, CM_FGBGFADE,
  CM_COUNT // Last.
} colorCycleModes_e;
const char *colorCycleText[] = { "None", "R-G-B", "C-M-Y", "Secondary", "Tertiary", "Graystep", "Rainbow", "Random", "FG-BG Fade" };
#define COLORCYCLETEXT_SIZE (sizeof(colorCycleText) / sizeof(const char *))

typedef enum operateOn_e {
  OO_INVALID = -1,
  OO_CURRENT = 0, OO_ALTERNATE = 1,
  OO_COUNT // Last
} operateOn_e;
const char *operateText[] = { "Live", "Alternate" };
#define OPERATETEXT_SIZE (sizeof(operateText) / sizeof(const char *))

// Connect enumeration types to their strings.
typedef enum enums_e {
  E_INVALID = -1,
  E_DIRECTIONS = 0, E_FADEMODES, E_SHIFTMODES, E_TEXTMODES, E_COLORCYCLES, E_COLORS, E_OPERATE,
  E_COUNT // Last.
} enums_e;
typedef struct enums_t {
  enums_e type;
  const char **texts;
} enums_t;
const enums_t enumerations[] = {
  { E_DIRECTIONS, dirText},
  { E_FADEMODES, fadeModeText},
  { E_SHIFTMODES, shiftText},
  { E_TEXTMODES, textModeText},
  { E_COLORCYCLES, colorCycleText},
  { E_COLORS, colorsText },
  { E_OPERATE, operateText },
};
#define ENUMERATIONS_SIZE (sizeof(enumerations) / sizeof(enums_t))

// A way to keep track of palette size.
typedef struct cyclePalette_t {
  const color_e *palette;
  const int size;
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

// Pattern element data types and access macros

// A pattern set consists of pattern elements (mode flags, parameters, text,
// and even the frame buffer) that tell the engine how to behave while
// constructing the next frame.  The data for each element starts as a void
// pointer that is allocated at runtime.  Each element is allocated as an array,
// allowing there to be multiple pattern sets, which the user can switch
// between.

// This patternElement enumeration should be maintained in the same order as
// the patternSet global pattern data set array.  The program will run a check on
// startup to make sure this occurs.
typedef enum patternElement_e {
  PE_INVALID = -1,
  // Modes
  PE_CELLFUN = 0, PE_BOUNCER, PE_FADE, PE_DIFFUSE, PE_ROLLOVER,
  PE_SCROLL, PE_HBARS, PE_VBARS, PE_FGCOLORALL, PE_BGCOLORALL, PE_IMAGEALL,
  PE_RANDOMDOT, PE_FADEMODE, PE_POSTRZ, PE_PRERZ, PE_ALIAS,
  PE_MULTIPLY, PE_BARSEED, PE_NORED, PE_NOGREEN, PE_NOBLUE, PE_SHIFTRED,
  PE_SHIFTGREEN, PE_SHIFTBLUE, PE_SHIFTCYAN, PE_SHIFTYELLOW, PE_SHIFTMAGENTA,
  PE_POSTIMAGE,
  // Parameters
  PE_FGE, PE_BGE, PE_FADEINC, PE_DIFFUSECOEF, PE_SCROLLDIR, PE_RANDOMDOTCOEF,
  PE_FGCYCLE, PE_BGCYCLE, PE_FGRAINBOW, PE_BGRAINBOW, PE_POSTEXP, PE_FLOATINC,
  PE_POSTRZANGLE, PE_POSTRZINC, PE_MULTIPLYBY, PE_DELAY,
  // Internal parameters
  PE_CYCLESAVEFG, PE_CYCLESAVEBG, PE_FGC, PE_BGC, PE_CELLFUNCOUNT, PE_BOUNCESCR,
  // Text
  PE_TEXTBUFFER, PE_TEXTMODE, PE_FONTFLIP, PE_FONTDIR, PE_TEXTOFFSET,
  PE_TEXTINDEX, PE_PIXELINDEX, PE_SCROLLDIRLAST, PE_TEXTSEED,
  // Frame buffer
  PE_FRAMEBUFFER,
  //
  PE_PRERZANGLE, PE_PRERZALIAS, PE_PRERZINC, PE_PRERZEXPAND, PE_FRAMECOUNT,
  PE_IMAGEANGLE, PE_IMAGEALIAS, PE_IMAGEINC, PE_IMAGEEXP,
  PE_IMAGEXOFFSET, PE_IMAGEYOFFSET, PE_IMAGENAME,
  PE_COUNT // LAst
} patternElement_e;

// These are the supported data types for a pattern element.
typedef enum elementType_e {
  ET_INVALID = -1,
  ET_BOOL = 0, ET_INT, ET_FLOAT, ET_COLOR, ET_ENUM, ET_STRING, ET_BUFFER,
  ET_COUNT // LAST
} elementType_e;

// This holds the pattern element initializer, which is of a type determined
// depending on the element.
typedef union default_u {
  int i;           // Integer initializer
  int e;           // Enumerated intializer
  float f;         // Float
  color_t c;       // Color type - the big one.
  char *s;         // String pointer.
  unsigned char b; // Boolean flag.
  // No buffer initializer.
} default_u;

// patternElement type - Includes descriptors of the element as well as a
// pointer (to be allocated) to the element's array of data.
typedef struct patternElement_t {
  const patternElement_e index;  // Corresponds to patterElement_e
  const char *name;              // String name of the element.  Unique, no spaces!
  const elementType_e type;      // Element type.
  const default_u initial;       // Initial value.
  const int size;                // Size of array elements (string, buffer)
  const enums_e etype;            // Enumeration type for ET_ENUM.
  void *data;                    // Pointer to the element's data.  Allocated later.
} patternElement_t;

// The pattern elements used by the engine.
patternElement_t patternSet[] = {
  { PE_CELLFUN,     "CellFun",     ET_BOOL,   {.b = NO} },
  { PE_BOUNCER,     "Bouncer",     ET_BOOL,   {.b = NO} },
  { PE_FADE,        "Fader",       ET_BOOL,   {.b = NO} },
  { PE_DIFFUSE,     "Diffuse",     ET_BOOL,   {.b = NO} },
  { PE_ROLLOVER,    "RollOver",    ET_BOOL,   {.b = NO} },
  { PE_SCROLL,      "Scroll",      ET_BOOL,   {.b = YES} },
  { PE_HBARS,       "Hbars",       ET_BOOL,   {.b = NO} },
  { PE_VBARS,       "Vbars",       ET_BOOL,   {.b = NO} },
  { PE_FGCOLORALL,  "FGColorAll",  ET_BOOL,   {.b = NO} },
  { PE_BGCOLORALL,  "BGColorAll",  ET_BOOL,   {.b = NO} },
  { PE_IMAGEALL,    "ImageAll",    ET_BOOL,   {.b = NO} },
  { PE_RANDOMDOT,   "RandomDots",  ET_BOOL,   {.b = NO} },
  { PE_FADEMODE,    "FadeMode",    ET_ENUM,   {.e = FM_LIMIT}, .etype = E_FADEMODES },
  { PE_POSTRZ,      "PostRotZoom", ET_BOOL,   {.b = NO} },
  { PE_PRERZ,       "PreRotZoom",  ET_BOOL,   {.b = NO} },
  { PE_ALIAS,       "AntiAlias",   ET_BOOL,   {.b = NO} },
  { PE_MULTIPLY,    "Multiply",    ET_BOOL,   {.b = NO} },
  { PE_BARSEED,     "SideBar",     ET_BOOL,   {.b = NO} },
  { PE_NORED,       "NoRed",       ET_BOOL,   {.b = NO} },
  { PE_NOGREEN,     "NoGreen",     ET_BOOL,   {.b = NO} },
  { PE_NOBLUE,      "NoBlue",      ET_BOOL,   {.b = NO} },
  { PE_SHIFTRED,    "ShiftRed",    ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTGREEN,  "ShiftGreen",  ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTBLUE,   "ShiftBlue",   ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTCYAN,   "ShiftCyan",   ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTYELLOW, "ShiftYellow", ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTMAGENTA,"ShiftMagenta",ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_POSTIMAGE,   "PostImage",   ET_BOOL,   {.b = NO} },
  { PE_FGE,         "FGColorE",    ET_ENUM,   {.e = CE_RED}, .etype = E_COLORS },
  { PE_BGE,         "BGColorE",    ET_ENUM,   {.e = CE_BLACK}, .etype = E_COLORS },
  { PE_FADEINC,     "FadeIncr",    ET_INT,    {.i = INITIAL_FADE_INC} },
  { PE_DIFFUSECOEF, "DiffuseCoef", ET_FLOAT,  {.f = INITIAL_DIFF_COEF} },
  { PE_SCROLLDIR,   "ScrollDir",   ET_ENUM,   {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_RANDOMDOTCOEF,"DotCoef",    ET_INT,    {.i = INITIAL_RAND_MOD} },
  { PE_FGCYCLE,     "FGCycleMode", ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_BGCYCLE,     "BGCycleMode", ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_FGRAINBOW,   "RainbowFG",   ET_INT,    {.i = INITIAL_RAINBOW_INC} },
  { PE_BGRAINBOW,   "RainbowBG",   ET_INT,    {.i = INITIAL_RAINBOW_INC} },
  { PE_POSTEXP,     "Expansion",   ET_FLOAT,  {.f = INITIAL_EXPAND} },
  { PE_FLOATINC,    "FloatInc",    ET_FLOAT,  {.f = INITIAL_FLOAT_INC} },
  { PE_POSTRZANGLE, "PostRotAngle",ET_FLOAT,  {.f = INITIAL_POSTROT_ANGLE} },
  { PE_POSTRZINC,   "PostRotInc",  ET_FLOAT,  {.f = INITIAL_POSTROT_INC} },
  { PE_MULTIPLYBY,  "MultiplyBy",  ET_FLOAT,  {.f = INITIAL_MULTIPLIER} },
  { PE_DELAY,       "Delay",       ET_INT,    {.i = INITIAL_DELAY} },
  { PE_CYCLESAVEFG, "FGCyclePos",  ET_INT,    {.i = 0} },
  { PE_CYCLESAVEBG, "BGCyclePos",  ET_INT,    {.i = 0} },
  { PE_FGC,         "FGColorC",    ET_COLOR,  {.c = CD_RED} },
  { PE_BGC,         "BGColorC",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_CELLFUNCOUNT,"CellFunCt",   ET_INT,    {.i = 0} },
  { PE_BOUNCESCR,   "BounceScr",   ET_ENUM,   {.e = DIR_UP}, .etype = E_DIRECTIONS },
  { PE_TEXTBUFFER,  "TextBuffer",  ET_STRING, {.s = INITIAL_TEXT}, TEXT_BUFFER_SIZE },
  { PE_TEXTMODE,    "TextMode",    ET_ENUM,   {.e = TS_FULLBG}, .etype = E_TEXTMODES },
  { PE_FONTFLIP,    "FontFlip",    ET_BOOL,   {.b = NO } },
  { PE_FONTDIR,     "FontDir",     ET_BOOL,   {.b = FORWARDS} },
  { PE_TEXTOFFSET,  "TextOffset",  ET_INT,    {.i = TENSOR_HEIGHT / 3 - 1} },
  { PE_TEXTINDEX,   "TextIndex",   ET_INT,    {.i = sizeof(INITIAL_TEXT) - 1} },
  { PE_PIXELINDEX,  "PixelIndex",  ET_INT,    {.i = INVALID} },
  { PE_SCROLLDIRLAST,"ScrollDirLast",ET_ENUM, {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_TEXTSEED,    "TextSeed",    ET_BOOL,   {.b = YES} },
  { PE_FRAMEBUFFER, "FrameBuffer", ET_BUFFER, .size = TENSOR_BYTES },
  { PE_PRERZANGLE,  "PreRotAngle", ET_FLOAT,  {.f = INITIAL_PREROT_ANGLE} },
  { PE_PRERZALIAS,  "PreRotAlias", ET_BOOL,   {.b = NO} },
  { PE_PRERZINC,    "PreRotInc",   ET_FLOAT,  {.f = INITIAL_POSTROT_INC} },
  { PE_PRERZEXPAND, "PreRotExp",   ET_FLOAT,  {.f = INITIAL_PREEXPAND} },
  { PE_FRAMECOUNT,  "CycleFCount", ET_INT,    {.i = INITIAL_FRAMECYCLECOUNT} },
  { PE_IMAGEANGLE,  "ImageAngle",  ET_FLOAT,  {.f = INITIAL_IMAGE_ANGLE} },
  { PE_IMAGEALIAS,  "ImageAlias",  ET_BOOL,   {.b = NO} },
  { PE_IMAGEINC,    "ImageInc",    ET_FLOAT,  {.f = INITIAL_IMAGE_INC} },
  { PE_IMAGEEXP,    "ImageExp",    ET_FLOAT,  {.f = INITIAL_IMAGE_EXPAND} },
  { PE_IMAGEXOFFSET,"ImageXOffset",ET_FLOAT,  {.f = INITIAL_IMAGE_XOFFSET} },
  { PE_IMAGEYOFFSET,"ImageYOffset",ET_FLOAT,  {.f = INITIAL_IMAGE_YOFFSET} },
  { PE_IMAGENAME,   "ImageName",   ET_STRING, {.s = INITIAL_IMAGE}, 1024},
};
#define PSET_SIZE (sizeof(patternSet) / sizeof(patternElement_t))
#define GLOBAL_PATTERN_ELEMENT_ARRAY patternSet

// Data element access macros - with patternElement array, element, and set selection.
#define ABOOL(varName, varSet, varElement) ( ((unsigned char *)varName[varElement].data)[varSet] )
#define AINT(varName, varSet, varElement) ( ((int *)varName[varElement].data)[varSet] )
#define AFLOAT(varName, varSet, varElement) ( ((float *)varName[varElement].data)[varSet] )
#define ACOLOR(varName, varSet, varElement) ( ((color_t *)varName[varElement].data)[varSet] )
#define AENUM(varName, varSet, varElement) ( ((int *)varName[varElement].data)[varSet] )
#define ASENUM(varName, varSet, varElement, varType) ( ((varType *)varName[varElement].data)[varSet] )
#define ASTRING(varName, varSet, varElement) ( &((char *)varName[varElement].data)[varSet * varName[varElement].size] )
#define ABUFFER(varName, varSet, varElement) ( &((unsigned char *)varName[varElement].data)[varSet * varName[varElement].size] )

// Data element access macros - using the global patternElement array, with element and set selection.
#define SBOOL(varSet, varElement) ( ABOOL(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SINT(varSet, varElement) ( AINT(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SFLOAT(varSet, varElement) ( AFLOAT(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SCOLOR(varSet, varElement) ( ACOLOR(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SENUM(varSet, varElement) ( AENUM(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SSENUM(varSet, varElement, varType) ( ASENUM(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement, varType) )
#define SSTRING(varSet, varElement) ( ASTRING(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
#define SBUFFER(varSet, varElement) ( ABUFFER(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )

// Data element access macros - using the global patternElement array and the currentSet, with element selection.
#define DBOOL(varElement) ( SBOOL(currentSet, varElement) )
#define DINT(varElement) ( SINT(currentSet, varElement) )
#define DFLOAT(varElement) ( SFLOAT(currentSet, varElement) )
#define DCOLOR(varElement) ( SCOLOR(currentSet, varElement) )
#define DENUM(varElement) ( SENUM(currentSet, varElement) )
#define DSENUM(varElement, varType) ( SSENUM(currentSet, varElement, varType) )
#define DSTRING(varElement) ( SSTRING(currentSet, varElement) )
#define DBUFFER(varElement) ( SBUFFER(currentSet, varElement) )

// User Commands
typedef enum command_e {
  COM_INVALID = -1, COM_NONE = 0,
  COM_CELL = 1, COM_BOUNCE, COM_FADE, COM_DIFFUSE, COM_TEXT, COM_ROLL,
  COM_SCROLL, COM_HBAR, COM_VBAR, COM_FGALL, COM_BGALL, COM_RDOT,
  COM_CYCLESET, COM_FADEMODE, COM_TEXTRESET, COM_POSTROTATE,
  COM_MODEOFF, COM_PREROTATE, COM_AA, COM_MULTIPLY, COM_SIDEBAR, COM_NORED,
  COM_NOGREEN, COM_NOBLUE, COM_IMAGE, COM_STEP_INC, COM_STEP_DEC, COM_STEP_RST,
  COM_DIFFUSE_INC, COM_DIFFUSE_DEC, COM_DIFFUSE_RST, COM_EXPAND_INC,
  COM_EXPAND_DEC, COM_EXPAND_RST, COM_FADE_INC, COM_FADE_DEC, COM_FADE_RST,
  COM_PREROT_INC, COM_PREROT_DEC, COM_PREROT_RST, COM_POSTROT_INC, COM_POSTROT_DEC,
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
  COM_COPYSET9, COM_PREROTINC_RST, COM_PREROTINC_INC, COM_PREROTINC_DEC,
  COM_PREEXP_INC, COM_PREEXP_DEC, COM_PREEXP_RST, COM_PREALIAS,
  COM_RP_RST, COM_GP_RST, COM_BP_RST, COM_MP_RST, COM_YP_RST, COM_CP_RST,
  COM_FGRAINBOW_INC, COM_FGRAINBOW_DEC, COM_FGRAINBOW_RST,
  COM_BGRAINBOW_INC, COM_BGRAINBOW_DEC, COM_BGRAINBOW_RST,
  COM_FGCYCLE_RST, COM_FGCYCLE_UP, COM_FGCYCLE_DOWN,
  COM_BGCYCLE_RST, COM_BGCYCLE_UP, COM_BGCYCLE_DOWN,
  COM_IMAGEALL,
  COM_FCOUNT_RST, COM_FCOUNT_INC, COM_FCOUNT_DEC,
  COM_IMANGLE_RST, COM_IMANGLE_INC, COM_IMANGLE_DEC,
  COM_IMINC_RST, COM_IMINC_INC, COM_IMINC_DEC,
  COM_IMEXP_RST, COM_IMEXP_INC, COM_IMEXP_DEC, COM_IMALIAS,
  COM_IMXOFFSET_RST, COM_IMXOFFSET_INC, COM_IMXOFFSET_DEC,
  COM_IMYOFFSET_RST, COM_IMYOFFSET_INC, COM_IMYOFFSET_DEC,
  COM_ALTERNATE_INC, COM_ALTERNATE_DEC,
  COM_LIVE_INC, COM_LIVE_DEC, COM_OPERATE, COM_EXCHANGE,
  COM_BLEND_RST, COM_BLEND_INC, COM_BLEND_DEC,
  COM_BLENDINC_RST, COM_BLENDINC_INC, COM_BLENDINC_DEC,
  COM_BLENDSWITCH,
  COM_COUNT // Last
} command_e;

// Associate key presses and commands
typedef struct keyCommand_t {
  SDL_Keymod mod;
  SDL_Keycode key;
  command_e command;
} keyCommand_t;

// Mouse command enumeration.
typedef enum mouseCommand_e {
  MOUSE_INVALID = -1,
  MOUSE_CLICK = 0, MOUSE_WHEEL_UP, MOUSE_WHEEL_DOWN,
  MOUSE_COUNT // Last
} mouseCommand_e;

// Structure of a command includes its position on screen (line, col)
// (line = -1 supresses display), command name to display, and which command
// enumerations and keys are associated with each mouse action.
typedef struct command_t {
  int line;
  int col;
  char *text;
  patternElement_e dat;  // Pattern set data to display after text.  PE_INVALID disables.
  keyCommand_t commands[MOUSE_COUNT];
} command_t;

const command_t displayCommand[] = {
  // Pattern sets
  #define ROW_PA 0
  #define COL_PA 2
  {ROW_PA + 1, COL_PA, "Pattern set cycle:",   PE_INVALID,  {{KMOD_CTRL, SDLK_g, COM_CYCLESET}}},
  {ROW_PA + 2, COL_PA, "Frames until cycle:", PE_FRAMECOUNT, {{0, 0, COM_FCOUNT_RST}, {0, 0, COM_FCOUNT_INC}, {0, 0, COM_FCOUNT_DEC}}},
  {ROW_PA + 8, COL_PA, "Live pattern set:",PE_INVALID, {{0, 0, COM_LIVE_INC}, {}, {0, 0, COM_LIVE_DEC}}},
  {ROW_PA + 9, COL_PA, "Alternate preview:",PE_INVALID, {{0, 0, COM_ALTERNATE_INC}, {}, {0, 0, COM_ALTERNATE_DEC}}},
  {ROW_PA + 10,COL_PA, "Control set:",     PE_INVALID, {{0, 0, COM_OPERATE}}},
  {ROW_PA + 11,COL_PA, "Exchange live and alternate sets", PE_INVALID, {{0, 0, COM_EXCHANGE}}},
  {ROW_PA + 12,COL_PA, "Alternate blend amount", PE_INVALID, {{0, 0, COM_BLEND_RST}, {0, 0, COM_BLEND_INC}, {0, 0, COM_BLEND_DEC}}},
  {ROW_PA + 13,COL_PA, "Auto blend rate", PE_INVALID, {{0, 0, COM_BLENDINC_RST},{0, 0, COM_BLENDINC_INC}, {0, 0, COM_BLENDINC_DEC}}},
  {ROW_PA + 14,COL_PA, "Auto blend & switch", PE_INVALID, {{0, 0, COM_BLENDSWITCH}}},

  // Auxillary
  #define ROW_A 46
  #define COL_A 4
  {ROW_A + 1, COL_A, "Delay(ms):", PE_DELAY,    {{KMOD_ALT | KMOD_CTRL, SDLK_i, COM_DELAY_RST}, {KMOD_ALT | KMOD_CTRL, SDLK_u, COM_DELAY_INC}, {KMOD_ALT | KMOD_CTRL, SDLK_o, COM_DELAY_DEC}}},
  {ROW_A + 2, COL_A, "Float step:", PE_FLOATINC, {{KMOD_ALT, SDLK_COMMA, COM_STEP_RST}, {KMOD_ALT, SDLK_m, COM_STEP_INC}, {KMOD_ALT, SDLK_PERIOD, COM_STEP_DEC}}},
  {ROW_A + 3, COL_A, "All modes off",   PE_INVALID,  {{KMOD_CTRL, SDLK_l, COM_MODEOFF}}},

  // Plane suppression
  #define ROW_P 35
  #define COL_P 0
  {ROW_P + 1, COL_P, "Supress red:",     PE_NORED,    {{KMOD_CTRL, SDLK_m, COM_NORED}}},
  {ROW_P + 2, COL_P, "Supress green:",   PE_NOGREEN,  {{KMOD_CTRL, SDLK_COMMA, COM_NOGREEN}}},
  {ROW_P + 3, COL_P, "Supress blue:",    PE_NOBLUE,   {{KMOD_CTRL, SDLK_PERIOD, COM_NOBLUE}}},

  // Diffusion
  #define ROW_D 20
  #define COL_D 2
  {ROW_D + 1, COL_D, "Enable:",      PE_DIFFUSE,     {{KMOD_CTRL, SDLK_r, COM_DIFFUSE}}},
  {ROW_D + 2, COL_D, "Coefficient:", PE_DIFFUSECOEF, {{KMOD_ALT, SDLK_w, COM_DIFFUSE_RST}, {KMOD_ALT, SDLK_q, COM_DIFFUSE_INC}, {KMOD_ALT, SDLK_e, COM_DIFFUSE_DEC}}},

  // Random Dots
  #define ROW_R 16
  #define COL_R 2
  {ROW_R + 1, COL_R, "Enable:",     PE_RANDOMDOT,     {{KMOD_CTRL, SDLK_s, COM_RDOT}}},
  {ROW_R + 2, COL_R, "Randomness:", PE_RANDOMDOTCOEF, {{KMOD_ALT, SDLK_LEFTBRACKET, COM_RANDOM_RST}, {KMOD_ALT, SDLK_p, COM_RANDOM_INC}, {KMOD_ALT, SDLK_RIGHTBRACKET, COM_RANDOM_DEC}}},

  // One shot seeds
  #define ROW_O 23
  #define COL_O 0
  {ROW_O + 1, COL_O, "Horizontal bars", PE_INVALID,  {{KMOD_CTRL, SDLK_i, COM_HBAR}}},
  {ROW_O + 2, COL_O, "Vertical bars",   PE_INVALID,  {{KMOD_CTRL, SDLK_o, COM_VBAR}}},
  {ROW_O + 3, COL_O, "Foreground all",  PE_INVALID,  {{KMOD_CTRL, SDLK_p, COM_FGALL}}},
  {ROW_O + 4, COL_O, "Background all",  PE_INVALID,  {{KMOD_CTRL, SDLK_a, COM_BGALL}}},
  {ROW_O + 5, COL_O, "Image seed",      PE_INVALID,  {{0, 0, COM_IMAGEALL}}},

    // Misc
  #define ROW_MI (ROW_O + 7)
  #define COL_MI COL_O
  {ROW_MI + 1, COL_MI, "Cell pattern:",    PE_CELLFUN,  {{KMOD_CTRL, SDLK_q, COM_CELL}}},
  {ROW_MI + 2, COL_MI, "Bouncer:",         PE_BOUNCER,  {{KMOD_CTRL, SDLK_w, COM_BOUNCE}}},
  {ROW_MI + 3, COL_MI, "Sidebar seed:",    PE_BARSEED,  {{KMOD_CTRL, SDLK_n, COM_SIDEBAR}}},

  // Fader
  #define ROW_F 24
  #define COL_F 2
  {ROW_F + 1, COL_F, "Enable:",           PE_FADE,     {{KMOD_CTRL, SDLK_e,  COM_FADE}}},
  {ROW_F + 2, COL_F, "Amount:",     PE_FADEINC,  {{KMOD_ALT, SDLK_x, COM_FADE_RST}, {KMOD_ALT, SDLK_z, COM_FADE_INC}, {KMOD_ALT, SDLK_c, COM_FADE_DEC}}},
  {ROW_F + 3, COL_F, "Mode:",      PE_FADEMODE, {{KMOD_CTRL, SDLK_h, COM_FADEMODE}}},

  // Multiplier
  #define ROW_M 29
  #define COL_M 2
  {ROW_M + 1, COL_M, "Enable:",      PE_MULTIPLY, {{KMOD_CTRL, SDLK_c, COM_MULTIPLY}}},
  {ROW_M + 2, COL_M, "Multiply by:", PE_MULTIPLYBY, {{KMOD_ALT, SDLK_i, COM_MULT_RST}, {KMOD_ALT, SDLK_u, COM_MULT_INC}, {KMOD_ALT, SDLK_o, COM_MULT_DEC}}},

  // Colors
  #define ROW_C 33
  #define COL_C 2
  {ROW_C + 1, COL_C, "Foreground:",    PE_FGE,       {{KMOD_ALT | KMOD_CTRL, SDLK_q, COM_FG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_w, COM_FG_DEC}}},
  {ROW_C + 2, COL_C, "Background:",    PE_BGE,       {{KMOD_ALT | KMOD_CTRL, SDLK_a, COM_BG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_s, COM_BG_DEC}}},
  {ROW_C + 3, COL_C, "FG cycle mode:", PE_FGCYCLE,   {{0, 0, COM_FGCYCLE_RST}, {KMOD_CTRL, SDLK_d, COM_FGCYCLE_UP}, {0, 0, COM_FGCYCLE_DOWN}}},
  {ROW_C + 4, COL_C, "BG cycle mode:", PE_BGCYCLE,   {{0, 0, COM_BGCYCLE_RST}, {KMOD_CTRL, SDLK_f, COM_BGCYCLE_UP}, {0, 0, COM_BGCYCLE_DOWN}}},
  {ROW_C + 5, COL_C, "FG Delta:",      PE_FGRAINBOW, {{KMOD_ALT, SDLK_g, COM_FGRAINBOW_RST}, {KMOD_ALT, SDLK_f, COM_FGRAINBOW_INC}, {KMOD_ALT, SDLK_h, COM_FGRAINBOW_DEC}}},
  {ROW_C + 6, COL_C, "BG Delta:",      PE_BGRAINBOW ,{{0, 0, COM_BGRAINBOW_RST}, {0, 0, COM_BGRAINBOW_INC}, {0, 0, COM_BGRAINBOW_DEC}}},

  // Scrollers
  #define ROW_S 41
  #define COL_S 2
  {ROW_S + 1, COL_S, "Scroller:",        PE_SCROLL,       {{KMOD_CTRL, SDLK_u, COM_SCROLL}}},
  {ROW_S + 2, COL_S, "(<a>) <arrows> - Scroll dir:", PE_SCROLLDIR, {{0, 0, COM_SCROLL_CYCLE_UP}, {}, {0, 0, COM_SCROLL_CYCLE_DOWN}}},
  {ROW_S + 3, COL_S, "Scroll rollover:", PE_ROLLOVER, {{KMOD_CTRL, SDLK_y, COM_ROLL}}},
  {ROW_S + 4, COL_S, "Red plane:",       PE_SHIFTRED,     {{0, 0, COM_RP_RST}, {KMOD_CTRL, SDLK_LEFTBRACKET, COM_RP_UP}, {0, 0, COM_RP_DOWN}}},
  {ROW_S + 5, COL_S, "Green plane:",     PE_SHIFTGREEN,   {{0, 0, COM_GP_RST}, {KMOD_CTRL, SDLK_RIGHTBRACKET, COM_GP_UP}, {0, 0, COM_GP_DOWN}}},
  {ROW_S + 6, COL_S, "Blue plane:",      PE_SHIFTBLUE,    {{0, 0, COM_BP_RST}, {KMOD_CTRL, SDLK_BACKSLASH, COM_BP_UP}, {0, 0, COM_BP_DOWN}}},
  {ROW_S + 7, COL_S, "Cyan plane:",      PE_SHIFTCYAN,    {{0, 0, COM_CP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_LEFTBRACKET, COM_CP_UP}, {0, 0, COM_CP_DOWN}}},
  {ROW_S + 8, COL_S, "Yellow plane:",    PE_SHIFTYELLOW,  {{0, 0, COM_YP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_RIGHTBRACKET, COM_YP_UP}, {0, 0, COM_YP_DOWN}}},
  {ROW_S + 9, COL_S, "Magenta plane:",   PE_SHIFTMAGENTA, {{0, 0, COM_MP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_BACKSLASH, COM_MP_UP}, {0, 0, COM_MP_DOWN}}},

  // Post rotozoom
  #define ROW_PR (23)
  #define COL_PR (4)
  {ROW_PR + 1, COL_PR, "Enable:",     PE_POSTRZ,      {{KMOD_CTRL, SDLK_k, COM_POSTROTATE}}},
  {ROW_PR + 2, COL_PR, "Angle:",      PE_POSTRZANGLE, {{KMOD_ALT, SDLK_b, COM_POSTROT_RST}, {KMOD_ALT, SDLK_v, COM_POSTROT_INC}, {KMOD_ALT, SDLK_n, COM_POSTROT_DEC}}},
  {ROW_PR + 3, COL_PR, "Angle Inc:",  PE_POSTRZINC,   {{KMOD_ALT, SDLK_k, COM_POSTSPEED_RST}, {KMOD_ALT, SDLK_j, COM_POSTSPEED_INC}, {KMOD_ALT, SDLK_l, COM_POSTSPEED_DEC}}},
  {ROW_PR + 4, COL_PR, "Expansion:",  PE_POSTEXP,      {{KMOD_ALT, SDLK_s, COM_EXPAND_RST}, {KMOD_ALT, SDLK_a, COM_EXPAND_INC}, {KMOD_ALT, SDLK_d, COM_EXPAND_DEC}}},
  {ROW_PR + 5, COL_PR, "Anti-alias:", PE_ALIAS,       {{KMOD_CTRL, SDLK_x, COM_AA}}},

  // Pre rotozoom
  #define ROW_PE (ROW_PR + 7)
  #define COL_PE (COL_PR)
  {ROW_PE + 1, COL_PE, "Enable:",    PE_PRERZ,      {{KMOD_CTRL, SDLK_z, COM_PREROTATE}}},
  {ROW_PE + 2, COL_PE, "Angle:",     PE_PRERZANGLE, {{KMOD_ALT, SDLK_t, COM_PREROT_RST}, {KMOD_ALT, SDLK_r, COM_PREROT_INC}, {KMOD_ALT, SDLK_y, COM_PREROT_DEC}}},
  {ROW_PE + 3, COL_PE, "Angle Inc:", PE_PRERZINC,   {{0, 0, COM_PREROTINC_RST}, {0, 0, COM_PREROTINC_INC}, {0, 0, COM_PREROTINC_DEC}}},
  {ROW_PE + 4, COL_PE, "Expansion:", PE_PRERZEXPAND,{{0, 0, COM_PREEXP_RST}, {0, 0, COM_PREEXP_INC}, {0, 0, COM_PREEXP_DEC}}},
  {ROW_PE + 5, COL_PE, "Anti-alias:",PE_PRERZALIAS, {{0, 0, COM_PREALIAS}}},

  // Image stuff
  #define ROW_I (ROW_PE + 7)
  #define COL_I COL_PE
  {ROW_I + 1, COL_I, "Image overlay:", PE_POSTIMAGE,  {{KMOD_CTRL | KMOD_ALT, SDLK_p, COM_IMAGE}}},
  {ROW_I + 2, COL_I, "Angle:",         PE_IMAGEANGLE, {{0, 0, COM_IMANGLE_RST}, {0, 0, COM_IMANGLE_INC}, {0, 0, COM_IMANGLE_DEC}}},
  {ROW_I + 3, COL_I, "Angle Inc:",     PE_IMAGEINC,   {{0, 0, COM_IMINC_RST},   {0, 0, COM_IMINC_INC},   {0, 0, COM_IMINC_DEC}}},
  {ROW_I + 4, COL_I, "Expansion:",     PE_IMAGEEXP,   {{0, 0, COM_IMEXP_RST},   {0, 0, COM_IMEXP_INC},   {0, 0, COM_IMEXP_DEC}}},
  {ROW_I + 5, COL_I, "Xoffset:",       PE_IMAGEXOFFSET, {{0, 0, COM_IMXOFFSET_RST}, {0, 0, COM_IMXOFFSET_INC}, {0, 0, COM_IMXOFFSET_DEC}}},
  {ROW_I + 6, COL_I, "Yoffset:",       PE_IMAGEYOFFSET, {{0, 0, COM_IMYOFFSET_RST}, {0, 0, COM_IMYOFFSET_INC}, {0, 0, COM_IMYOFFSET_DEC}}},
  {ROW_I + 7, COL_I, "Anti-alias:",    PE_IMAGEALIAS, {{0, 0, COM_IMALIAS}}},

  // Text
  #define ROW_T (40)
  #define COL_T (0)
  {ROW_T + 1, COL_T, "Text seed:",         PE_TEXTSEED,   {{KMOD_CTRL, SDLK_t, COM_TEXT}}},
  {ROW_T + 3, COL_T, "Delete last letter", PE_INVALID,    {{KMOD_NONE, SDLK_BACKSPACE, COM_BACKSPACE}}},
  {ROW_T + 4, COL_T, "Erase all text",     PE_INVALID,    {{KMOD_NONE, SDLK_DELETE, COM_DELETE}}},
  {ROW_T + 5, COL_T, "Reverse text:",      PE_FONTDIR,    {{KMOD_CTRL, SDLK_QUOTE, COM_TEXT_REVERSE}}},
  {ROW_T + 6, COL_T, "Flip text.",         PE_FONTFLIP,   {{KMOD_CTRL, SDLK_SEMICOLON, COM_TEXT_FLIP}}},
  {ROW_T + 7, COL_T, "Text mode:",         PE_TEXTMODE,   {{KMOD_CTRL, SDLK_b, COM_TEXT_MODE_UP}, {}, {0, 0, COM_TEXT_MODE_DOWN}}},
  {ROW_T + 8, COL_T, "Edge Offset:",       PE_TEXTOFFSET, {{KMOD_ALT | KMOD_CTRL, SDLK_z, COM_TEXTO_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_x, COM_TEXTO_DEC}}},
  {ROW_T + 9, COL_T, "Restart text",       PE_INVALID,    {{KMOD_CTRL, SDLK_j, COM_TEXTRESET}}},

  // Quit
  {53, 4, "Quit",           PE_INVALID,  {{KMOD_NONE, SDLK_ESCAPE, COM_EXIT}}},
};
#define DISPLAYCOMMAND_SIZE (sizeof(displayCommand) / sizeof(command_t))

// These commands correspond to key presses, but aren't displayed as mouse-over items.
const command_t otherCommand[] = {
  {-1, 0, "Orientation", PE_INVALID, {{KMOD_CTRL, SDLK_v, COM_ORIENTATION}}},
  {-1, 2, "Scroll Up", PE_INVALID, {{KMOD_NONE, SDLK_UP, COM_SCROLL_UP}}},
  {-1, 2, "Scroll Down", PE_INVALID, {{KMOD_NONE, SDLK_DOWN, COM_SCROLL_DOWN}}},
  {-1, 2, "Scroll Left", PE_INVALID, {{KMOD_NONE, SDLK_LEFT, COM_SCROLL_LEFT}}},
  {-1, 2, "Scroll Right", PE_INVALID, {{KMOD_NONE, SDLK_RIGHT, COM_SCROLL_RIGHT}}},
  {-1, 2, "Activate & Scroll Up", PE_INVALID, {{KMOD_ALT, SDLK_UP, COM_SCROLL_UPC}}},
  {-1, 2, "Activate & Scroll Down", PE_INVALID, {{KMOD_ALT, SDLK_DOWN, COM_SCROLL_DOWNC}}},
  {-1, 2, "Activate & Scroll Left", PE_INVALID, {{KMOD_ALT, SDLK_LEFT, COM_SCROLL_LEFTC}}},
  {-1, 2, "Activate & Scroll Right", PE_INVALID, {{KMOD_ALT, SDLK_RIGHT, COM_SCROLL_RIGHTC}}},
  {-1, 0, "Load image buffer 0", PE_INVALID, {{KMOD_ALT,  SDLK_0, COM_LOAD0 }}},
  {-1, 0, "Load image buffer 1", PE_INVALID, {{KMOD_ALT,  SDLK_1, COM_LOAD1 }}},
  {-1, 0, "Load image buffer 2", PE_INVALID, {{KMOD_ALT,  SDLK_2, COM_LOAD2 }}},
  {-1, 0, "Load image buffer 3", PE_INVALID, {{KMOD_ALT,  SDLK_3, COM_LOAD3 }}},
  {-1, 0, "Load image buffer 4", PE_INVALID, {{KMOD_ALT,  SDLK_4, COM_LOAD4 }}},
  {-1, 0, "Load image buffer 5", PE_INVALID, {{KMOD_ALT,  SDLK_5, COM_LOAD5 }}},
  {-1, 0, "Load image buffer 6", PE_INVALID, {{KMOD_ALT,  SDLK_6, COM_LOAD6 }}},
  {-1, 0, "Load image buffer 7", PE_INVALID, {{KMOD_ALT,  SDLK_7, COM_LOAD7 }}},
  {-1, 0, "Load image buffer 8", PE_INVALID, {{KMOD_ALT,  SDLK_8, COM_LOAD8 }}},
  {-1, 0, "Load image buffer 9", PE_INVALID, {{KMOD_ALT,  SDLK_9, COM_LOAD9 }}},
  {-1, 0, "Load pattern set 0", PE_INVALID, {{KMOD_CTRL,  SDLK_0, COM_LOADSET0 }}},
  {-1, 0, "Load pattern set 1", PE_INVALID, {{KMOD_CTRL,  SDLK_1, COM_LOADSET1 }}},
  {-1, 0, "Load pattern set 2", PE_INVALID, {{KMOD_CTRL,  SDLK_2, COM_LOADSET2 }}},
  {-1, 0, "Load pattern set 3", PE_INVALID, {{KMOD_CTRL,  SDLK_3, COM_LOADSET3 }}},
  {-1, 0, "Load pattern set 4", PE_INVALID, {{KMOD_CTRL,  SDLK_4, COM_LOADSET4 }}},
  {-1, 0, "Load pattern set 5", PE_INVALID, {{KMOD_CTRL,  SDLK_5, COM_LOADSET5 }}},
  {-1, 0, "Load pattern set 6", PE_INVALID, {{KMOD_CTRL,  SDLK_6, COM_LOADSET6 }}},
  {-1, 0, "Load pattern set 7", PE_INVALID, {{KMOD_CTRL,  SDLK_7, COM_LOADSET7 }}},
  {-1, 0, "Load pattern set 8", PE_INVALID, {{KMOD_CTRL,  SDLK_8, COM_LOADSET8 }}},
  {-1, 0, "Load pattern set 9", PE_INVALID, {{KMOD_CTRL,  SDLK_9, COM_LOADSET9 }}},
  {-1, 0, "Copy to set 0", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_0, COM_COPYSET0 }}},
  {-1, 0, "Copy to set 1", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_1, COM_COPYSET1 }}},
  {-1, 0, "Copy to set 2", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_2, COM_COPYSET2 }}},
  {-1, 0, "Copy to set 3", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_3, COM_COPYSET3 }}},
  {-1, 0, "Copy to set 4", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_4, COM_COPYSET4 }}},
  {-1, 0, "Copy to set 5", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_5, COM_COPYSET5 }}},
  {-1, 0, "Copy to set 6", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_6, COM_COPYSET6 }}},
  {-1, 0, "Copy to set 7", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_7, COM_COPYSET7 }}},
  {-1, 0, "Copy to set 8", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_8, COM_COPYSET8 }}},
  {-1, 0, "Copy to set 9", PE_INVALID, {{KMOD_CTRL | KMOD_ALT,  SDLK_9, COM_COPYSET9 }}},
};
#define OTHERCOMMAND_SIZE (sizeof(otherCommand) / sizeof(command_t))

// Display titles
typedef struct displayText_t {
  int line;
  int col;
  char *text;
} displayText_t;

displayText_t displayText[] = {
  {ROW_T + 2, COL_T, "<Unmodified keys> - Add text."},
  {ROW_PA + 3, COL_PA, "<c> 0-9 - Change to set #"},
  {ROW_PA + 4, COL_PA, "<c> <a> 0-9 - Copy set to #"},
  {ROW_PA + 5, COL_PA, "<a> 0-9 - Load buffer from set #"},
  {ROW_PA + 6, COL_PA, "<a> <s> 0-9, a-z - Save to disk."},
  {ROW_PA + 7, COL_PA, "<c> <s> 0-9, a-z - Load saved set."},
};
#define DISPLAYTEXT_SIZE (sizeof(displayText) / sizeof(displayText_t))

displayText_t headerText[] = {
  {21, 0,          "Live Preview          FPS:"},
  {21, 4,          "Alternate Preview     FPS:"},
  {51, 4,          "                 GUI  FPS:"},
  {51, 0,          "Text buffer:"},
  {ROW_PA, COL_PA, "Pattern sets:"},
  {ROW_P, COL_P,   "Plane suppression:"},
  {ROW_A, COL_A,   "Auxiliary:"},
  {ROW_MI, COL_MI, "Misc:"},
  {ROW_I, COL_I,   "Image:"},
  {ROW_R, COL_R,   "Random dots:"},
  {ROW_C, COL_C,   "Colors:"},
  {ROW_D, COL_D,   "Diffusion:"},
  {ROW_O, COL_O,   "One-shot seeds:"},
  {ROW_F, COL_F,   "Fader:"},
  {ROW_M, COL_M,   "Multiplier:"},
  {ROW_T, COL_T,   "Text entry:"},
  {ROW_S, COL_S,   "Scrollers:"},
  {ROW_PR, COL_PR, "Post rotation:"},
  {ROW_PE, COL_PE, "Pre rotation:"}
};
#define HEADERTEXT_SIZE (sizeof(headerText) / sizeof(displayText_t))

// Globals - We do love our globals.
TTF_Font *screenFont;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;
SDL_Surface *imageSeed[PATTERN_SET_COUNT];
int tensorWidth, tensorHeight;
unsigned char cyclePatternSets = NO;  // cyclePatternSets mode is a global (for now).
int cycleFrameCount = INITIAL_FRAMECYCLECOUNT;    // Frame Count for cycling.
unsigned char enableTensor = YES;
int currentSet = 0;
int alternateSet = 7;
operateOn_e displaySet = OO_CURRENT;
float global_intensity_limit = 1.0;
int previewFrameCountA = 0, previewFPSA = 0;
int previewFrameCountB = 0, previewFPSB = 0;
int guiFrameCount = 0, guiFPS = 0;
float alternateBlend = 0, alternateBlendRate = 0.01;
unsigned char autoBlend = NO;
#define CHAR_W (8)
#define ACOL (0)
#define BCOL (ACOL + 26 * CHAR_W)
#define CCOL (BCOL + 12 * CHAR_W)
#define DCOL (CCOL + 40 * CHAR_W)
#define ECOL (DCOL + 13 * CHAR_W)
#define FCOL (ECOL + 26 * CHAR_W)
const int colToPixel[] = {ACOL, BCOL, CCOL, DCOL, ECOL, FCOL, WINDOW_WIDTH};
Uint32 FPSEventType, DRAWEventTypeA, DRAWEventTypeB, GUIEventType;
unsigned char fbb[TENSOR_BYTES];

// Prototypes
void DrawNewFrame(int set, unsigned char primary);
void ProcessModes(int set);
Uint32 TriggerFrameDrawA(Uint32 interval, void *param);
Uint32 TriggerFrameDrawB(Uint32 interval, void *param);
Uint32 TriggerFrameCount(Uint32 interval, void *param);
Uint32 TriggerGUIUpdate(Uint32 interval, void *param);
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer);
void Scroll (dir_e direction, int rollovermode, unsigned char *fb, unsigned char plane);
void WriteSlice(int set);
void CellFun(int set);
void DrawSideBar(int set);
void Diffuse(float diffusionCoeff, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
void SavePatternSet(char key, int set);
void LoadPatternSet(char key, int set);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
color_t ColorCycle(int set, colorCycleModes_e cycleMode, int *cycleSaver, int cycleInc);
void ColorAll(color_t color, unsigned char *fb);
void SetDims(void);
void UpdateDisplays(int set, unsigned char sendToTensor, float intensity_limit);
void UpdateGUI(void);
void UpdateTensor(unsigned char *buffer);
void DrawPreviewBorder(int x, int y);
void UpdatePreview(int xOffset, int yOffset, unsigned char *buffer);
void InitDisplayTexts(void);
void UpdateInfoDisplay(int set);
void ClearWindow(void);
void WriteLine(char * thisText, int line, int col, color_t color, color_t bgColor);
void WriteBool(int value, int row, int col, int width);
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
void ClearRed(int set);
void ClearGreen(int set);
void ClearBlue(int set);
void DrawImage(int set, double angle, float xoffset, float yoffset, double expansion, int aliasmode, unsigned char *fb_dst);
int min(int a, int b);
int max(int a, int b);
unsigned char SameRectangle(SDL_Rect a, SDL_Rect b);
int HandleCommand(command_e command);
int HandleKey(SDL_Keycode key, SDL_Keymod mod);
void AllocatePatternData(void);
void VerifyStructuralIntegrity(void);
void CopyPatternSet(int dst, int src);
void BlendAlternate(unsigned char *fba);

// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  int i;
  unsigned char exitProgram = NO;
  char caption_temp[100];
  SDL_Event event;
  unsigned char drawNewFrameA = NO;
  unsigned char drawNewFrameB = NO;
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

  // Verify the integrity of some of the data structures.  This is to enforce
  // consistency for enumerated array access.
  VerifyStructuralIntegrity();

  // Allocate pattern set memory
  AllocatePatternData();

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

  // Initialize the image array.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    imageSeed[i] = NULL;
  }

  // Initialize tensor communications and aspect.
  tensor_init();
  //tensor_landscape_p = 1;  // Landscape mode (good for single panel).
  tensor_landscape_p = 0;  // Portrait mode (normal).

  // Set the widths / heights
  SetDims();  // After set tensor_landscape_p.

  // Draw a border around the preview
  DrawPreviewBorder(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y);
  DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);

  // Further patternSet initializations
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    SINT(i, PE_TEXTOFFSET) = tensorHeight / 3 - 1;  // After SetDims()
  }

  // Attempt to load startup pattern sets from disk.  These are 0-9.now
  // files.  Its okay if they don't exist.  Otherwise, default will be loaded
  // into those pattern sets.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    LoadPatternSet(i + '0', i);
  }
  cycleFrameCount = DINT(PE_FRAMECOUNT);

  // Bam - Show the (blank) preview.
  UpdateDisplays(currentSet, alternateSet, global_intensity_limit);

  // Add the text to the window
  InitDisplayTexts();

  // Initialize the user events
  FPSEventType = SDL_RegisterEvents(1);
  DRAWEventTypeA = SDL_RegisterEvents(1);
  DRAWEventTypeB = SDL_RegisterEvents(1);
  GUIEventType = SDL_RegisterEvents(1);

  // Init the processing timer.
  if (!SDL_AddTimer(DINT(PE_DELAY), TriggerFrameDrawA, NULL)) {
    fprintf(stderr, "Can't initialize the processing timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  if (!SDL_AddTimer(SINT(alternateSet, PE_DELAY), TriggerFrameDrawB, NULL)) {
    fprintf(stderr, "Can't initialize the 2nd processing timer! %s\n", SDL_GetError());
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
          for (i = 0; i < DISPLAYCOMMAND_SIZE; i++) {
            // box is the rectangle encompassing the command text.  We could
            // precompute these if timing were important.
            box.x = colToPixel[displayCommand[i].col];
            box.y = displayCommand[i].line * DISPLAY_TEXT_HEIGHT;
            box.w = colToPixel[displayCommand[i].col + 2] - box.x;
            box.h = (displayCommand[i].line + 1) * DISPLAY_TEXT_HEIGHT - box.y + 1;

            // Is it in the rectangle of command i?
            if ((y >= box.y) && (y < box.y + box.h)) {
              if ((x >= box.x) && (x < box.x + box.w)) {

                // Yep.
                thisHover = i;

                // Is it hovering over a command is wasn't hovering over before?
                if ((!SameRectangle(box, boxOld)) || (lastHover == INVALID)) {

                  // Yeah, so draw the new highlight.
                  DrawBox(box.x, box.y, box.w, box.h, DISPLAY_COLOR_TEXTSBG_HL);
                  WriteCommand(i, displayCommand, DISPLAY_COLOR_TEXTS_HL, DISPLAY_COLOR_TEXTSBG_HL);

                  // And if it came off a different command, remove that highlight.
                  if (lastHover != INVALID) {
                    DrawBox(boxOld.x, boxOld.y, boxOld.w, boxOld.h, DISPLAY_COLOR_TEXTSBG);
                    WriteCommand(lastHover, displayCommand, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
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
            WriteCommand(lastHover, displayCommand, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
            lastHover = INVALID;
          }
          break;

        case SDL_MOUSEWHEEL:
          // The mouse wheel moved.  See if we should act on that.
          if (thisHover != INVALID) {

            // Wheel down.
            if (event.wheel.y < 0) {

              // If there are no mouse wheel commands for this item, consider it a click.
              if (displayCommand[thisHover].commands[MOUSE_WHEEL_DOWN].command == COM_NONE) {
                exitProgram = HandleCommand(displayCommand[thisHover].commands[MOUSE_CLICK].command);
              } else {
                exitProgram = HandleCommand(displayCommand[thisHover].commands[MOUSE_WHEEL_DOWN].command);
              }

            // Wheel up.
            } else {

              // If there are no mouse wheel commands for this item, consider it a click.
              if (displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command == COM_NONE) {
                exitProgram = HandleCommand(displayCommand[thisHover].commands[MOUSE_CLICK].command);
              } else {
                exitProgram = HandleCommand(displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command);
              }
            }
          }
          break;

        case SDL_MOUSEBUTTONUP:
          // Mouse button unpushed.  Consider this a click.  If we're over
          // the same item we down clicked on, execute a command.
          if (event.button.button == SDL_BUTTON_LEFT) {
            if ((thisHover != INVALID) && (thisHover == mouseDownOn)) {
              exitProgram = HandleCommand(displayCommand[thisHover].commands[MOUSE_CLICK].command);
            }
          }
          break;

        case SDL_MOUSEBUTTONDOWN:
          // Mouse button pushed.  Make a note of the item it was pushed over.
          if (event.button.button == SDL_BUTTON_LEFT) {
            mouseDownOn = thisHover;
          }
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
            DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);
          }
          break;

        default:
          // There are two registered user events to check for here.  They can't
          // be tested in the case statement because they are not compile-time
          // integer constants.
          if (event.type == DRAWEventTypeA) {
            // The frame timer expired.  Set a new frame to draw.
            drawNewFrameA = YES;

          } else if (event.type == DRAWEventTypeB) {
            // The alternate frame timer expired.  Set a new frame to draw.
            drawNewFrameB = YES;

          } else if (event.type == FPSEventType) {
            // The fps timer expired.  Set fps and reset the frame count.
            previewFPSA = previewFrameCountA;
            previewFPSB = previewFrameCountB;
            guiFPS = guiFrameCount;
            guiFrameCount = 0;
            previewFrameCountA = 0;
            previewFrameCountB = 0;

          } else if (event.type == GUIEventType) {
            // Update the text display.
            UpdateInfoDisplay(displaySet ? alternateSet : currentSet);
            UpdateGUI();
            guiFrameCount++;

          } else {
            //~ fprintf(stderr, "Unhandled SDL event: %i\n", event.type);
          }
          break;
      }
    }

    // Draw a new frame once every n ms, where n defaults to 60.
    if ((drawNewFrameA) || (drawNewFrameB)) {
      if (drawNewFrameA) {
        drawNewFrameA = NO;
        previewFrameCountA++;
        DrawNewFrame(currentSet, YES);
        if (cyclePatternSets) {
          cycleFrameCount--;
          if (cycleFrameCount < 0) {
            currentSet = (currentSet + 1) % PATTERN_SET_COUNT;
            cycleFrameCount = DINT(PE_FRAMECOUNT);
          }
          if (cycleFrameCount > DINT(PE_FRAMECOUNT)) {
            cycleFrameCount = DINT(PE_FRAMECOUNT);
          }
        }
      }
      if (drawNewFrameB) {
        drawNewFrameB = NO;
        previewFrameCountB++;
        DrawNewFrame(alternateSet, NO);
      }
    } else {
      // Large delays make the CPU spin. Its because we;re stuck in a tight
      // loop, polling for events that never occur.  Best to get out of the way...
      // Idle the processor for 1 ms.
      nanosleep((struct timespec[]) {{0,1000000}}, NULL);
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
Uint32 TriggerFrameDrawA(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = DRAWEventTypeA;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(SINT(currentSet, PE_DELAY));
}

// Event that triggers a frame to be drawn.
Uint32 TriggerFrameDrawB(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = DRAWEventTypeB;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(SINT(alternateSet, PE_DELAY));
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

// Check to make sure that some of the program's data sets are built correctly.
// This is done to ensure that certain arrays can be accessed via enumeration
// values rather than indicies (i.e. enumerations must match indecies).
void VerifyStructuralIntegrity(void) {
  int i;

  // patternElement_e and patternSet array.
  for (i = 0; i < PSET_SIZE; i++) {
    if (i != patternSet[i].index) {
      fprintf(stderr, "Programmer error: patternElement_e does not match patternSet array!\n");
      fprintf(stderr, "Element %i, \"%s\" has incorrect enumeration value %i!\n", i, patternSet[i].name, patternSet[i].index);
      exit(EXIT_FAILURE);
    }
  }
  if (PSET_SIZE != PE_COUNT) {
    fprintf(stderr, "Programmer error: Mismatched patternSet(%i) != patternElement_e(%i)!\n", (int) PSET_SIZE, PE_COUNT);
    exit(EXIT_FAILURE);
  }

  // color_e and namedPalette array.
  for (i = 0; i < NAMEDPALETTE_SIZE; i++) {
    if (i != namedPalette[i].index) {
      fprintf(stderr, "Programmer error: color_e does not match namedPalette array!\n");
      fprintf(stderr, "Element %i, has incorrect enumeration value %i!\n", i, namedPalette[i].index);
      exit(EXIT_FAILURE);
    }
  }
  if (CE_COUNT != NAMEDPALETTE_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched namedPalette(%i) != color_e(%i)!\n", (int) NAMEDPALETTE_SIZE, CE_COUNT);
    exit(EXIT_FAILURE);
  }
  if (CE_COUNT != COLORSTEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched color_e(%i) != colorText(%i)!\n", CE_COUNT, (int) COLORSTEXT_SIZE);
    exit(EXIT_FAILURE);
  }

  // enums_e and enumerations array.
  for (i = 0; i < ENUMERATIONS_SIZE; i++) {
    if (i != enumerations[i].type) {
      fprintf(stderr, "Programmer error: enums_e does not match enumerations array!\n");
      fprintf(stderr, "Element %i, has incorrect enumeration value %i!\n", i, enumerations[i].type);
      exit(EXIT_FAILURE);
    }
  }
  if (E_COUNT != ENUMERATIONS_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched enums_e(%i) != enumerations(%i)!\n", E_COUNT, (int) ENUMERATIONS_SIZE);
    exit(EXIT_FAILURE);
  }

  // Texts - we can only do a count on these.
  if (DIR_COUNT != DIRTEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched dir_e(%i) != dirText(%i)!\n", DIR_COUNT, (int) DIRTEXT_SIZE);
    exit(EXIT_FAILURE);
  }
  if (FM_COUNT != FADEMODETEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched fadeModes_e(%i) != fadeModeText(%i)!\n", FM_COUNT, (int) FADEMODETEXT_SIZE);
    exit(EXIT_FAILURE);
  }
  if (SM_COUNT != SHIFTTEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched shiftModes_e(%i) != shiftText(%i)!\n", SM_COUNT, (int) SHIFTTEXT_SIZE);
    exit(EXIT_FAILURE);
  }
  if (TS_COUNT != TEXTMODETEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched textMode_e(%i) != textModeText(%i)!\n", TS_COUNT, (int) TEXTMODETEXT_SIZE);
    exit(EXIT_FAILURE);
  }
  if (CM_COUNT != COLORCYCLETEXT_SIZE) {
    fprintf(stderr, "Programmer error: Mismatched colorCycleModes_e(%i) != colorCycleText(%i)!\n", CM_COUNT, (int) COLORCYCLETEXT_SIZE);
    exit(EXIT_FAILURE);
  }
}

// Allocate the data memory for the pattern sets.
void AllocatePatternData(void) {
  int i, j;

  // Allocate data for each element based on type.  Allocated data will be an
  // array to that multiple pattern sets can be kept in memory at a time.
  for (i = 0; i < PSET_SIZE; i++) {
    switch (patternSet[i].type) {
      case ET_INT: case ET_ENUM:
        // patternSet[i].data is a void * pointing at memory allocated for 10 ints.
        patternSet[i].data = malloc(sizeof(int) * PATTERN_SET_COUNT);
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate int %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        } else {
          for(j = 0; j < PATTERN_SET_COUNT; j++) {
            SINT(j, i) = patternSet[i].initial.i;
          }
        }
        break;

      case ET_FLOAT:
        patternSet[i].data = malloc(sizeof(float) * PATTERN_SET_COUNT);
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate float %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        } else {
          for(j = 0; j < PATTERN_SET_COUNT; j++) {
            SFLOAT(j, i) = patternSet[i].initial.f;
          }
        }
        break;

      case ET_COLOR:
        patternSet[i].data = malloc(sizeof(color_t) * PATTERN_SET_COUNT);
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate color %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        } else {
          for(j = 0; j < PATTERN_SET_COUNT; j++) {
            SCOLOR(j, i) = patternSet[i].initial.c;
          }
        }
        break;

      case ET_BOOL:
        patternSet[i].data = malloc(sizeof(unsigned char) * PATTERN_SET_COUNT);
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate flag %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        } else {
          for(j = 0; j < PATTERN_SET_COUNT; j++) {
            SBOOL(j, i) = patternSet[i].initial.b;
          }
        }
        break;

      case ET_STRING:
        if (patternSet[i].size <= 0) {
          fprintf(stderr, "Invalid size for element %i (%s) of type string - %i\n", i, patternSet[i].name, patternSet[i].size);
          exit(EXIT_FAILURE);
        }
        patternSet[i].data = malloc(sizeof(char) * patternSet[i].size * PATTERN_SET_COUNT);
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate string %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        } else {
          for (j = 0; j < PATTERN_SET_COUNT; j++) {
            strncpy(SSTRING(j, i), patternSet[i].initial.s, patternSet[i].size);
            SSTRING(j, i)[patternSet[i].size - 1] = '\0';
          }
        }
        break;

      case ET_BUFFER:
        if (patternSet[i].size <= 0) {
          fprintf(stderr, "Invalid size for element %i (%s) of type string - %i\n", i, patternSet[i].name, patternSet[i].size);
          exit(EXIT_FAILURE);
        }
        // Allocate and intialize to 0.
        patternSet[i].data = calloc(patternSet[i].size * PATTERN_SET_COUNT, sizeof(unsigned char));
        if (!patternSet[i].data) {
          fprintf(stderr, "Unable to allocate buffer %i - %s\n", i, patternSet[i].name);
          exit(EXIT_FAILURE);
        }
        break;

      case ET_INVALID:
      case ET_COUNT:
      default:
        fprintf(stderr, "Error.  Unrecognized type!\n");
        break;
    }
  }

  // Write out the pattern data set.
  //~ for (j = 0; j < PATTERN_SET_COUNT; j++) {
    //~ for (i = 0; i < PSET_SIZE; i++) {
      //~ switch(patternSet[i].type) {
        //~ case ET_INT:
          //~ printf("Element %i - %i (%s) of type int is %i\n", i, patternSet[i].index, patternSet[i].name, SINT(set, i,j));
          //~ break;
        //~ case ET_FLOAT:
          //~ printf("Element %i - %i (%s) of type float is %f\n", i, patternSet[i].index, patternSet[i].name, SFLOAT(set, i, j));
          //~ break;
        //~ case ET_COLOR:
          //~ printf("Element %i - %i (%s) of type color is (%i, %i, %i, %i)\n", i, patternSet[i].index, patternSet[i].name, SCOLOR(set, i, j).r, SCOLOR(set, i, j).g, SCOLOR(set, i, j).b, SCOLOR(set, i, j).a);
          //~ break;
        //~ case ET_ENUM:
          //~ printf("Element %i - %i (%s) of type enum is %i\n", i, patternSet[i].index, patternSet[i].name, SINT(set,  i, j));
          //~ break;
        //~ case ET_BOOL:
          //~ printf("Element %i - %i (%s) of type bool is %s\n", i, patternSet[i].index, patternSet[i].name, SBOOL(j, i) ? "YES" : "NO");
          //~ break;
        //~ case ET_STRING:
          //~ printf("Element %i - %i (%s) of type string is \"%s\"\n", i, patternSet[i].index, patternSet[i].name, SSTRING(set,  i, j));
          //~ break;
        //~ case ET_BUFFER:
          //~ printf("Element %i - %i (%s) of type buffer is %i bytes.\n", i, patternSet[i].index, patternSet[i].name, patternSet[i].size);
          //~ break;
        //~ default:
          //~ fprintf(stderr, "Error - Unknown data type on element %i - %i\n", i, patternSet[i].type);
          //~ exit(EXIT_FAILURE);
          //~ break;
      //~ }
    //~ }
  //~ }
}
// The thing that happens at every frame.
void DrawNewFrame(int set, unsigned char primary) {

  // Cycles through the pattern sets.  Allows you to set up a bunch of pattern
  // sets and switch between them one at a time at some interval.
  if (primary) {
    ProcessModes(set);

    UpdateDisplays(set, primary, global_intensity_limit);
  } else {
    if (currentSet != alternateSet) {
      ProcessModes(set);
    }
    UpdateDisplays(set, primary, global_intensity_limit);
  }

}

// Time to make a mess of the array.
void ProcessModes(int set) {
  int currentSet = set; // Overrides global used in D* macros.

  // Foreground color cycle.
  if (DENUM(PE_FGCYCLE)) {
    DCOLOR(PE_FGC) = ColorCycle(currentSet, DENUM(PE_FGCYCLE), &DINT(PE_CYCLESAVEFG), DINT(PE_FGRAINBOW));
  }

  // Change background color.
  if (DENUM(PE_BGCYCLE)) {
    DCOLOR(PE_BGC) = ColorCycle(currentSet, DENUM(PE_BGCYCLE), &DINT(PE_CYCLESAVEBG), DINT(PE_BGRAINBOW));
  }

  // Seed the entire array with the foreground color.
  if (DBOOL(PE_FGCOLORALL)) {
    ColorAll(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_FGCOLORALL) = NO;
  }

  // Scroller.
  if (DBOOL(PE_SCROLL)) {
    Scroll(DENUM(PE_SCROLLDIR), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_ALL);
  }

  // Scroll red.
  if (DENUM(PE_SHIFTRED)) {
    Scroll(DENUM(PE_SHIFTRED) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_RED);
  }

  // Scroll green.
  if (DENUM(PE_SHIFTGREEN)) {
    Scroll(DENUM(PE_SHIFTGREEN) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_GREEN);
  }

  // Scroll blue.
  if (DENUM(PE_SHIFTBLUE)) {
    Scroll(DENUM(PE_SHIFTBLUE) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_BLUE);
  }

  // Scroll blue and green
  if (DENUM(PE_SHIFTCYAN)) {
      Scroll(DENUM(PE_SHIFTCYAN) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_CYAN);
  }

  // Scroll red and blue.
  if (DENUM(PE_SHIFTMAGENTA)) {
    Scroll(DENUM(PE_SHIFTMAGENTA) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_MAGENTA);
  }

  // Scroll green and red.
  if (DENUM(PE_SHIFTYELLOW)) {
    Scroll(DENUM(PE_SHIFTYELLOW) - 1, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_YELLOW);
  }

  // Draw a solid bar up the side we are scrolling from.
  if (DBOOL(PE_BARSEED)) {
    DrawSideBar(currentSet);
  }

  // First stab experimental image drawing.  Needs work.
  if (DBOOL(PE_POSTIMAGE)) {
    DrawImage(set, DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
      DFLOAT(PE_IMAGEEXP), DBOOL(PE_IMAGEALIAS), DBUFFER(PE_FRAMEBUFFER));
    DFLOAT(PE_IMAGEANGLE) += DFLOAT(PE_IMAGEINC);
  }

  // Image one-shot
  if (DBOOL(PE_IMAGEALL)) {
    DrawImage(set, DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
      DFLOAT(PE_IMAGEEXP), DBOOL(PE_IMAGEALIAS), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_IMAGEALL) = NO;
  }

  // Write a column of text on the side opposite of the scroll direction.
  if (DBOOL(PE_TEXTSEED)) {
    // Scroll provided by scroller if its on.
    WriteSlice(currentSet);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (DBOOL(PE_CELLFUN)) {
    // Give each pixel a color value.
    CellFun(currentSet);
  }

  // Bouncy bouncy (ick).
  if (DBOOL(PE_BOUNCER)) {
    DENUM(PE_BOUNCESCR) = (DENUM(PE_BOUNCESCR) + 1) % DIR_COUNT;
    Scroll(DENUM(PE_BOUNCESCR), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_ALL);
  }

  // Bam!  Draw some horizontal bars.
  if (DBOOL(PE_HBARS)) {
    HorizontalBars(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_HBARS) = NO;
  }

  // Bam!
  if (DBOOL(PE_VBARS)) {
    VerticalBars(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_VBARS) = NO;
  }

  // Random dots.  Most useful seed ever.
  if (DBOOL(PE_RANDOMDOT)) {
    RandomDots(DCOLOR(PE_FGC), DINT(PE_RANDOMDOTCOEF), DBUFFER(PE_FRAMEBUFFER));
  }

  // Fader
  if (DBOOL(PE_FADE)) {
    FadeAll(DINT(PE_FADEINC), DENUM(PE_FADEMODE), DBUFFER(PE_FRAMEBUFFER));
  }

  // Averager
  if (DBOOL(PE_DIFFUSE)) {
    Diffuse(DFLOAT(PE_DIFFUSECOEF), DBUFFER(PE_FRAMEBUFFER));
  }

  // Clear screen.
  if (DBOOL(PE_BGCOLORALL)) {
    ColorAll(DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_BGCOLORALL) = NO;
  }

  // Multiplier
  if (DBOOL(PE_MULTIPLY)) {
    Multiply(DFLOAT(PE_MULTIPLYBY), DBUFFER(PE_FRAMEBUFFER));
  }

  // Rotozoomer
  if (DBOOL(PE_PRERZ)) {
    Rotate(DFLOAT(PE_PRERZANGLE), DFLOAT(PE_PRERZEXPAND), DBOOL(PE_PRERZALIAS), DBUFFER(PE_FRAMEBUFFER), DBUFFER(PE_FRAMEBUFFER));
    DFLOAT(PE_PRERZANGLE) += DFLOAT(PE_PRERZINC);
  }

  // Zero the red.
  if (DBOOL(PE_NORED)) {
    ClearRed(currentSet);
  }

  // Zero the blue.
  if (DBOOL(PE_NOBLUE)) {
    ClearBlue(currentSet);
  }

  // Zero the green.
  if (DBOOL(PE_NOGREEN)) {
    ClearGreen(currentSet);
  }

  // Post rotation increment.
  if (DBOOL(PE_POSTRZ)) {
    DFLOAT(PE_POSTRZANGLE) += DFLOAT(PE_POSTRZINC);
  }
}

// Key press processing.
int HandleKey(SDL_Keycode key, SDL_Keymod mod) {
  int i, j;

  // Prolly not necessary.
  if (key == SDLK_UNKNOWN) return 0;

  // To make exact comparisons to mod, left or right equivalents of modifier
  // keys must both be set if one is set.
  if (mod & KMOD_CTRL) mod |= KMOD_CTRL;
  if (mod & KMOD_ALT) mod |= KMOD_ALT;
  if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;

  // Check to see if the key combination activates a command.
  for ( i = 0 ; i < DISPLAYCOMMAND_SIZE; i++) {
    for (j = 0; j < MOUSE_COUNT; j++) {
      if ((displayCommand[i].commands[j].key == key) && (displayCommand[i].commands[j].mod == mod)) {
        return(HandleCommand(displayCommand[i].commands[j].command));
      }
    }
  }

  // Check other place for commands.
  for ( i = 0 ; i < OTHERCOMMAND_SIZE; i++) {
    for (j = 0 ; j < MOUSE_COUNT; j++) {
      if ((otherCommand[i].commands[j].key == key) && (otherCommand[i].commands[j].mod == mod)) {
        return(HandleCommand(otherCommand[i].commands[j].command));
      }
    }
  }

  // Check for load or save to disk commands.
  // Save the current pattern set as <key>.now (for 0-9, a-z, only)
  if (mod == (KMOD_ALT | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      SavePatternSet(key, currentSet);
      return 0;
    }
  }

  // Load a pattern set from <key>.now into the current set.
  if (mod == (KMOD_CTRL | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      LoadPatternSet(key, currentSet);
      return 0;
    }
  }

  // If no command by now, then the key prolly goes in the text buffer.  First,
  // make sure we are dealing with printable characters:
  if ((key <= '~' && key >= ' ') && (!(mod & KMOD_ALT)) && (!(mod & KMOD_CTRL))) {
    if ((mod & KMOD_SHIFT) == KMOD_SHIFT) {
      // Keys with shift held down.
      if (key <= SDLK_z && key >= SDLK_a) {
        // Capitalize for a - z.
        DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = key - ('a' - 'A');
      } else {
        // Lookup the symbols for the rest of the keys.
        switch (key) {
          case SDLK_1: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '!'; break;
          case SDLK_2: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '@'; break;
          case SDLK_3: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '#'; break;
          case SDLK_4: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '$'; break;
          case SDLK_5: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '%'; break;
          case SDLK_6: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '^'; break;
          case SDLK_7: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '&'; break;
          case SDLK_8: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '*'; break;
          case SDLK_9: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '('; break;
          case SDLK_0: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = ')'; break;
          case SDLK_BACKSLASH: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '|'; break;
          case SDLK_BACKQUOTE: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '~'; break;
          case SDLK_MINUS: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '_'; break;
          case SDLK_EQUALS: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '+'; break;
          case SDLK_LEFTBRACKET: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '{'; break;
          case SDLK_RIGHTBRACKET: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '}'; break;
          case SDLK_SEMICOLON: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = ':'; break;
          case SDLK_COMMA: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '<'; break;
          case SDLK_PERIOD: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '>'; break;
          case SDLK_SLASH: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '?'; break;
          case SDLK_QUOTE: DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = '"'; break;
          default: break;
        }
      }
    } else {
      // Unmodified key entry.  We'll treat them as ascii in the printable range.
      DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX)] = key;
    }

    // Advance the terminating null and increase the buffer size.
    DSTRING(PE_TEXTBUFFER)[DINT(PE_TEXTINDEX) + 1] = 0x00;
    DINT(PE_TEXTINDEX)++;
    if (DINT(PE_TEXTINDEX) >= (patternSet[PE_TEXTBUFFER].size - 2)) DINT(PE_TEXTINDEX)--;
  }

  return 0;
}

// Executes a user command - toggles flags, adjusts parameters, etc.
int HandleCommand(command_e command) {
  int i;

  // Support for operating on selected sets.
  int set = currentSet;
  if (displaySet == OO_ALTERNATE) {
    set = alternateSet;
  }

  switch(command) {
    case COM_BLEND_RST: alternateBlend = 0; break;
    case COM_BLEND_INC:
      alternateBlend += SFLOAT(set, PE_FLOATINC);
      if (alternateBlend > 1.0) alternateBlend = 1.0;
      break;
    case COM_BLEND_DEC:
      alternateBlend -= SFLOAT(set, PE_FLOATINC);
      if (alternateBlend < 0.0) alternateBlend = 0.0;
      break;
    case COM_BLENDINC_RST: alternateBlendRate = 0.01; break;
    case COM_BLENDINC_INC:
      alternateBlendRate += SFLOAT(set, PE_FLOATINC);
      if (alternateBlendRate > 1.0) alternateBlendRate = 1.0;
      break;
    case COM_BLENDINC_DEC:
      alternateBlendRate -= SFLOAT(set, PE_FLOATINC);
      if (alternateBlendRate < 0.0) alternateBlendRate = 0.0;
      break;
    case COM_BLENDSWITCH: autoBlend = !autoBlend; break;
    case COM_EXCHANGE: i = currentSet; currentSet = alternateSet; alternateSet = i; break;
    case COM_OPERATE: displaySet = (displaySet + 1) % OO_COUNT; break;
    case COM_ALTERNATE_INC: alternateSet = (alternateSet + 1) % PATTERN_SET_COUNT; break;
    case COM_ALTERNATE_DEC: alternateSet--; if (alternateSet < 0) alternateSet = PATTERN_SET_COUNT - 1; break;
    case COM_LIVE_INC: currentSet = (currentSet + 1) % PATTERN_SET_COUNT; break;
    case COM_LIVE_DEC: currentSet--; if (currentSet < 0) currentSet = PATTERN_SET_COUNT - 1; break;
    case COM_CELL: SBOOL(set, PE_CELLFUN) = !SBOOL(set, PE_CELLFUN); break;
    case COM_TEXT_FLIP: SBOOL(set, PE_FONTFLIP) = !SBOOL(set, PE_FONTFLIP); break;
    case COM_TEXT_REVERSE: SBOOL(set, PE_FONTDIR) = !SBOOL(set, PE_FONTDIR); break;
    case COM_TEXT: SBOOL(set, PE_TEXTSEED) = !SBOOL(set, PE_TEXTSEED); break;
    case COM_BOUNCE: SBOOL(set, PE_BOUNCER) = !SBOOL(set, PE_BOUNCER); break;
    case COM_FADE: SBOOL(set, PE_FADE) = !SBOOL(set, PE_FADE); break;
    case COM_DIFFUSE: SBOOL(set, PE_DIFFUSE) = !SBOOL(set, PE_DIFFUSE); break;
    case COM_ROLL: SBOOL(set, PE_ROLLOVER) = !SBOOL(set, PE_ROLLOVER); break;
    case COM_SCROLL: SBOOL(set, PE_SCROLL) = !SBOOL(set, PE_SCROLL); break;
    case COM_HBAR: SBOOL(set, PE_HBARS) = YES; break;
    case COM_VBAR: SBOOL(set, PE_VBARS) = YES; break;
    case COM_FGALL: SBOOL(set, PE_FGCOLORALL) = YES; break;
    case COM_BGALL: SBOOL(set, PE_BGCOLORALL) = YES; break;
    case COM_IMAGEALL: SBOOL(set, PE_IMAGEALL) = YES; break;
    case COM_RDOT: SBOOL(set, PE_RANDOMDOT) = !SBOOL(set, PE_RANDOMDOT); break;
    case COM_FGCYCLE_RST: SENUM(set, PE_FGCYCLE) = patternSet[PE_FGCYCLE].initial.e; break;
    case COM_BGCYCLE_RST: SENUM(set, PE_BGCYCLE) = patternSet[PE_BGCYCLE].initial.e; break;
    case COM_FGCYCLE_UP: SENUM(set, PE_FGCYCLE) = (SENUM(set, PE_FGCYCLE) + 1) % CM_COUNT; break;
    case COM_BGCYCLE_UP: SENUM(set, PE_BGCYCLE) = (SENUM(set, PE_BGCYCLE) + 1) % CM_COUNT; break;
    case COM_FGCYCLE_DOWN:
      SENUM(set, PE_FGCYCLE)--;
      if (SENUM(set, PE_FGCYCLE) < 0) SENUM(set, PE_FGCYCLE) = CM_COUNT - 1;
      break;
    case COM_BGCYCLE_DOWN:
      SENUM(set, PE_BGCYCLE)--;
      if (SENUM(set, PE_BGCYCLE) < 0) SENUM(set, PE_BGCYCLE) = CM_COUNT - 1;
      break;
    case COM_CYCLESET:
      cyclePatternSets = !cyclePatternSets;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_MODEOFF:
      for (i = 0; i < PSET_SIZE; i++) {
        if (patternSet[i].type == ET_BOOL) {
          SBOOL(set, i) = NO;
        }
      }
      SENUM(set, PE_SHIFTBLUE) = SM_HOLD;
      SENUM(set, PE_SHIFTCYAN) = SM_HOLD;
      SENUM(set, PE_SHIFTGREEN) = SM_HOLD;
      SENUM(set, PE_SHIFTMAGENTA) = SM_HOLD;
      SENUM(set, PE_SHIFTYELLOW) = SM_HOLD;
      SENUM(set, PE_SHIFTRED) = SM_HOLD;
      cyclePatternSets = NO;
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
    case COM_FADEMODE:
      SENUM(set, PE_FADEMODE) = (SENUM(set, PE_FADEMODE) + 1) % FM_COUNT;
      break;
    case COM_NORED: SBOOL(set, PE_NORED) = !SBOOL(set, PE_NORED); break;
    case COM_NOGREEN: SBOOL(set, PE_NOGREEN) = !SBOOL(set, PE_NOGREEN); break;
    case COM_NOBLUE: SBOOL(set, PE_NOBLUE) = !SBOOL(set, PE_NOBLUE); break;
    case COM_TEXTRESET: SINT(set, PE_PIXELINDEX) = INVALID; break;
    case COM_POSTROTATE:
      SBOOL(set, PE_POSTRZ) = !SBOOL(set, PE_POSTRZ);
      SFLOAT(set, PE_POSTRZANGLE) = 0;
      break;
    case COM_PREROTATE: SBOOL(set, PE_PRERZ) = !SBOOL(set, PE_PRERZ); break;
    case COM_AA: SBOOL(set, PE_ALIAS) = !SBOOL(set, PE_ALIAS); break;
    case COM_MULTIPLY: SBOOL(set, PE_MULTIPLY) = !SBOOL(set, PE_MULTIPLY); break;
    case COM_SIDEBAR: SBOOL(set, PE_BARSEED) = !SBOOL(set, PE_BARSEED); break;
    case COM_ORIENTATION:
      tensor_landscape_p = !tensor_landscape_p;
      SetDims();
      DrawPreviewBorder(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y);
      DrawPreviewBorder(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y);
      break;
    case COM_TEXT_MODE_UP: SENUM(set, PE_TEXTMODE) = (SENUM(set, PE_TEXTMODE) + 1) % TS_COUNT; break;
    case COM_TEXT_MODE_DOWN:
      SENUM(set, PE_TEXTMODE)--;
      if (SENUM(set, PE_TEXTMODE) < 0) SENUM(set, PE_TEXTMODE) = TS_COUNT - 1;
      break;
    case COM_RP_UP: SENUM(set, PE_SHIFTRED) = (SENUM(set, PE_SHIFTRED) + 1) % SM_COUNT; break;
    case COM_RP_RST: SENUM(set, PE_SHIFTRED) = patternSet[PE_SHIFTRED].initial.e; break;
    case COM_RP_DOWN:
      SENUM(set, PE_SHIFTRED)--;
      if (SENUM(set, PE_SHIFTRED) < 0) SENUM(set, PE_SHIFTRED) = SM_COUNT - 1;
      break;
    case COM_GP_UP: SENUM(set, PE_SHIFTGREEN) = (SENUM(set, PE_SHIFTGREEN) + 1) % SM_COUNT; break;
    case COM_GP_RST: SENUM(set, PE_SHIFTGREEN) = patternSet[PE_SHIFTGREEN].initial.e; break;
    case COM_GP_DOWN:
      SENUM(set, PE_SHIFTGREEN)--;
      if (SENUM(set, PE_SHIFTGREEN) < 0) SENUM(set, PE_SHIFTGREEN) = SM_COUNT - 1;
      break;
    case COM_BP_UP: SENUM(set, PE_SHIFTBLUE) = (SENUM(set, PE_SHIFTBLUE) + 1) % SM_COUNT; break;
    case COM_BP_RST: SENUM(set, PE_SHIFTBLUE) = patternSet[PE_SHIFTBLUE].initial.e; break;
    case COM_BP_DOWN:
      SENUM(set, PE_SHIFTBLUE)--;
      if (SENUM(set, PE_SHIFTBLUE) < 0) SENUM(set, PE_SHIFTBLUE) = SM_COUNT - 1;
      break;
    case COM_FG_DEC:
      SENUM(set, PE_FGE)--;
      if (SENUM(set, PE_FGE) < CE_RED) SENUM(set, PE_FGE) = CE_COUNT - 1;
      SCOLOR(set, PE_FGC) = namedPalette[SENUM(set, PE_FGE)].color;
      break;
    case COM_FG_INC:
      SENUM(set, PE_FGE) = (SENUM(set, PE_FGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_FGC) = namedPalette[SENUM(set, PE_FGE)].color;
      break;
    case COM_BG_DEC:
      SENUM(set, PE_BGE)--;
      if (SENUM(set, PE_BGE) < 0) SENUM(set, PE_BGE) = CE_COUNT - 1;
      SCOLOR(set, PE_BGC) = namedPalette[SENUM(set, PE_BGE)].color;
      break;
    case COM_BG_INC:
      SENUM(set, PE_BGE) = (SENUM(set, PE_BGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_BGC) = namedPalette[SENUM(set, PE_BGE)].color;
      break;
    case COM_COPYSET0: CopyPatternSet(0, set); break;
    case COM_COPYSET1: CopyPatternSet(1, set); break;
    case COM_COPYSET2: CopyPatternSet(2, set); break;
    case COM_COPYSET3: CopyPatternSet(3, set); break;
    case COM_COPYSET4: CopyPatternSet(4, set); break;
    case COM_COPYSET5: CopyPatternSet(5, set); break;
    case COM_COPYSET6: CopyPatternSet(6, set); break;
    case COM_COPYSET7: CopyPatternSet(7, set); break;
    case COM_COPYSET8: CopyPatternSet(8, set); break;
    case COM_COPYSET9: CopyPatternSet(9, set); break;
    case COM_TEXTO_DEC:
      SINT(set, PE_TEXTOFFSET)--;
      if (SINT(set, PE_TEXTOFFSET) < 0) {
        if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
          SINT(set, PE_TEXTOFFSET) = tensorHeight - 1;
        } else {
          SINT(set, PE_TEXTOFFSET) = tensorWidth - 1;
        }
      }
      break;
    case COM_TEXTO_INC:
      SINT(set, PE_TEXTOFFSET)++;
      if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
        SINT(set, PE_TEXTOFFSET) %= tensorHeight;
      } else {
        SINT(set, PE_TEXTOFFSET) %= tensorWidth;
      }
      break;
    case COM_DELAY_DEC:
      SINT(set, PE_DELAY)--;
      if (SINT(set, PE_DELAY) < 1) SINT(set, PE_DELAY) = 1;  // Can't be 0!
      break;
    case COM_DELAY_RST:
      SINT(set, PE_DELAY) = INITIAL_DELAY;
      break;
    case COM_DELAY_INC:
      SINT(set, PE_DELAY)++;
      break;
    case COM_CP_UP: SENUM(set, PE_SHIFTCYAN) = (SENUM(set, PE_SHIFTCYAN) + 1) % SM_COUNT; break;
    case COM_CP_RST: SENUM(set, PE_SHIFTCYAN) = patternSet[PE_SHIFTCYAN].initial.e; break;
    case COM_CP_DOWN:
      SENUM(set, PE_SHIFTCYAN)--;
      if (SENUM(set, PE_SHIFTCYAN) < 0) SENUM(set, PE_SHIFTCYAN) = SM_COUNT - 1;
      break;
    case COM_YP_UP: SENUM(set, PE_SHIFTYELLOW) = (SENUM(set, PE_SHIFTYELLOW) + 1) % SM_COUNT; break;
    case COM_YP_RST: SENUM(set, PE_SHIFTYELLOW) = patternSet[PE_SHIFTYELLOW].initial.e; break;
    case COM_YP_DOWN:
      SENUM(set, PE_SHIFTYELLOW)--;
      if (SENUM(set, PE_SHIFTYELLOW) < 0) SENUM(set, PE_SHIFTYELLOW) = SM_COUNT - 1;
      break;
    case COM_MP_UP: SENUM(set, PE_SHIFTMAGENTA) = (SENUM(set, PE_SHIFTMAGENTA) + 1) % SM_COUNT; break;
    case COM_MP_RST: SENUM(set, PE_SHIFTMAGENTA) = patternSet[PE_SHIFTMAGENTA].initial.e; break;
    case COM_MP_DOWN:
      SENUM(set, PE_SHIFTMAGENTA)--;
      if (SENUM(set, PE_SHIFTMAGENTA) < 0) SENUM(set, PE_SHIFTMAGENTA) = SM_COUNT - 1;
      break;
    case COM_IMAGE: SBOOL(set, PE_POSTIMAGE) = !SBOOL(set, PE_POSTIMAGE); break;
    case COM_FADE_DEC: SINT(set, PE_FADEINC)--; break;
    case COM_FADE_INC: SINT(set, PE_FADEINC)++; break;
    case COM_FADE_RST: SINT(set, PE_FADEINC) = INITIAL_FADE_INC; break;
    case COM_DIFFUSE_DEC: SFLOAT(set, PE_DIFFUSECOEF) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_DIFFUSE_RST: SFLOAT(set, PE_DIFFUSECOEF) = INITIAL_DIFF_COEF; break;
    case COM_DIFFUSE_INC: SFLOAT(set, PE_DIFFUSECOEF) += SFLOAT(set, PE_FLOATINC); break;
    case COM_EXPAND_DEC: SFLOAT(set, PE_POSTEXP) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_EXPAND_RST: SFLOAT(set, PE_POSTEXP) = INITIAL_EXPAND; break;
    case COM_EXPAND_INC: SFLOAT(set, PE_POSTEXP) += SFLOAT(set, PE_FLOATINC); break;
    case COM_RANDOM_DEC:
      SINT(set, PE_RANDOMDOTCOEF) -= max(1, SINT(set, PE_RANDOMDOTCOEF) / 50);
      if (SINT(set, PE_RANDOMDOTCOEF) < 1) SINT(set, PE_RANDOMDOTCOEF) = 1;
      break;
    case COM_RANDOM_RST: SINT(set, PE_RANDOMDOTCOEF) = INITIAL_RAND_MOD; break;
    case COM_RANDOM_INC:
      SINT(set, PE_RANDOMDOTCOEF) += max(1, SINT(set, PE_RANDOMDOTCOEF) / 50);
      if (SINT(set, PE_RANDOMDOTCOEF) > 20000) SINT(set, PE_RANDOMDOTCOEF) = 20000;
      break;
    case COM_POSTROT_DEC: SFLOAT(set, PE_POSTRZANGLE) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTROT_RST: SFLOAT(set, PE_POSTRZANGLE) = INITIAL_POSTROT_ANGLE; break;
    case COM_POSTROT_INC: SFLOAT(set, PE_POSTRZANGLE) += SFLOAT(set, PE_FLOATINC); break;
    case COM_LOAD0: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(0, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD1: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(1, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD2: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(2, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD3: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(3, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD4: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(4, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD5: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(5, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD6: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(6, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD7: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(7, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD8: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(8, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD9: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(9, PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_FGRAINBOW_DEC: SINT(set, PE_FGRAINBOW)--; break;
    case COM_FGRAINBOW_RST: SINT(set, PE_FGRAINBOW) = patternSet[PE_FGRAINBOW].initial.i; break;
    case COM_FGRAINBOW_INC: SINT(set, PE_FGRAINBOW)++; break;
    case COM_BGRAINBOW_DEC: SINT(set, PE_BGRAINBOW)--; break;
    case COM_BGRAINBOW_RST: SINT(set, PE_BGRAINBOW) = patternSet[PE_BGRAINBOW].initial.i; break;
    case COM_BGRAINBOW_INC: SINT(set, PE_BGRAINBOW)++; break;
    case COM_CYCLE_MODE: SENUM(set, PE_FGCYCLE) = (SENUM(set, PE_FGCYCLE) + 1) % CM_COUNT; break;
    case COM_CYCLE_MODE_DOWN:
      SENUM(set, PE_FGCYCLE)--;
      if (SENUM(set, PE_FGCYCLE) < 0) SENUM(set, PE_FGCYCLE) = CM_COUNT - 1;
      break;
    case COM_PREROT_DEC: SFLOAT(set, PE_PRERZANGLE)-= SFLOAT(set, PE_FLOATINC); break;
    case COM_PREROT_RST: SFLOAT(set, PE_PRERZANGLE) = INITIAL_PREROT_ANGLE; break;
    case COM_PREROT_INC: SFLOAT(set, PE_PRERZANGLE)+= SFLOAT(set, PE_FLOATINC); break;
    case COM_SCROLL_UPC:
      SENUM(set, PE_SCROLLDIR) = DIR_UP;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_DOWNC:
      SENUM(set, PE_SCROLLDIR) = DIR_DOWN;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_LEFTC:
      SENUM(set, PE_SCROLLDIR) = DIR_LEFT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_RIGHTC:
      SENUM(set, PE_SCROLLDIR) = DIR_RIGHT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_CYCLE_UP:
      SENUM(set, PE_SCROLLDIR) = (SENUM(set, PE_SCROLLDIR) + 1) % DIR_COUNT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_CYCLE_DOWN:
      SENUM(set, PE_SCROLLDIR)--;
      if (SENUM(set, PE_SCROLLDIR) < 0) SENUM(set, PE_SCROLLDIR) = DIR_COUNT - 1;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_MULT_DEC: SFLOAT(set, PE_MULTIPLYBY) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_MULT_RST: SFLOAT(set, PE_MULTIPLYBY) = INITIAL_MULTIPLIER; break;
    case COM_MULT_INC: SFLOAT(set, PE_MULTIPLYBY) += SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTSPEED_DEC: SFLOAT(set, PE_POSTRZINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTSPEED_RST: SFLOAT(set, PE_POSTRZINC) = INITIAL_POSTROT_INC; break;
    case COM_POSTSPEED_INC: SFLOAT(set, PE_POSTRZINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_STEP_INC: SFLOAT(set, PE_FLOATINC) *= 10; break;
    case COM_STEP_RST: SFLOAT(set, PE_FLOATINC) = INITIAL_FLOAT_INC; break;
    case COM_STEP_DEC: SFLOAT(set, PE_FLOATINC) /= 10; break;
    case COM_SCROLL_UP: SENUM(set, PE_SCROLLDIR) = DIR_UP; break;
    case COM_SCROLL_DOWN: SENUM(set, PE_SCROLLDIR) = DIR_DOWN; break;
    case COM_SCROLL_LEFT: SENUM(set, PE_SCROLLDIR) = DIR_LEFT; break;
    case COM_SCROLL_RIGHT: SENUM(set, PE_SCROLLDIR) = DIR_RIGHT; break;
    case COM_EXIT: return 1;
    case COM_BACKSPACE:
      SINT(set, PE_TEXTINDEX)--;
      if (SINT(set, PE_TEXTINDEX) < 0) SINT(set, PE_TEXTINDEX) = 0;
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = 0x00;
      break;
    case COM_RETURN:
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '\n';
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX) + 1] = 0x00;
      SINT(set, PE_TEXTINDEX)++;
      if (SINT(set, PE_TEXTINDEX) >= (patternSet[PE_TEXTBUFFER].size - 2)) {
        SINT(set, PE_TEXTINDEX)--;
      }
      break;
    case COM_DELETE:
      SINT(set, PE_TEXTINDEX) = 0;
      SSTRING(set, PE_TEXTBUFFER)[0] = 0x00;
      break;
    case COM_INVALID: case COM_NONE: case COM_COUNT:
      break;
    case COM_PREROTINC_RST: SFLOAT(set, PE_PRERZINC) = patternSet[PE_PRERZINC].initial.f; break;
    case COM_PREROTINC_INC: SFLOAT(set, PE_PRERZINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_PREROTINC_DEC: SFLOAT(set, PE_PRERZINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_PREALIAS: SBOOL(set, PE_PRERZALIAS) = !SBOOL(set, PE_PRERZALIAS); break;
    case COM_PREEXP_RST: SFLOAT(set, PE_PRERZEXPAND) = patternSet[PE_PRERZEXPAND].initial.f; break;
    case COM_PREEXP_INC: SFLOAT(set, PE_PRERZEXPAND) += SFLOAT(set, PE_FLOATINC); break;
    case COM_PREEXP_DEC: SFLOAT(set, PE_PRERZEXPAND) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMINC_RST: SFLOAT(set, PE_IMAGEINC) = patternSet[PE_IMAGEINC].initial.f; break;
    case COM_IMINC_INC: SFLOAT(set, PE_IMAGEINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMINC_DEC: SFLOAT(set, PE_IMAGEINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMALIAS: SBOOL(set, PE_IMAGEALIAS) = !SBOOL(set, PE_IMAGEALIAS); break;
    case COM_IMEXP_RST: SFLOAT(set, PE_IMAGEEXP) = patternSet[PE_IMAGEEXP].initial.f; break;
    case COM_IMEXP_INC: SFLOAT(set, PE_IMAGEEXP) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMEXP_DEC: SFLOAT(set, PE_IMAGEEXP) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMANGLE_RST: SFLOAT(set, PE_IMAGEANGLE) = patternSet[PE_IMAGEANGLE].initial.f; break;
    case COM_IMANGLE_INC: SFLOAT(set, PE_IMAGEANGLE) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMANGLE_DEC: SFLOAT(set, PE_IMAGEANGLE) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMXOFFSET_RST: SFLOAT(set, PE_IMAGEXOFFSET) = patternSet[PE_IMAGEXOFFSET].initial.f; break;
    case COM_IMXOFFSET_INC:
      SFLOAT(set, PE_IMAGEXOFFSET) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEXOFFSET) > 1.0) SFLOAT(set, PE_IMAGEXOFFSET) = 1.0;
      break;
    case COM_IMXOFFSET_DEC:
      SFLOAT(set, PE_IMAGEXOFFSET) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEXOFFSET) < 0.0) SFLOAT(set, PE_IMAGEXOFFSET) = 0.0;
      break;
    case COM_IMYOFFSET_RST: SFLOAT(set, PE_IMAGEYOFFSET) = patternSet[PE_IMAGEYOFFSET].initial.f; break;
    case COM_IMYOFFSET_INC:
      SFLOAT(set, PE_IMAGEYOFFSET) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEYOFFSET) > 1.0) SFLOAT(set, PE_IMAGEYOFFSET) = 1.0;
      break;
    case COM_IMYOFFSET_DEC:
      SFLOAT(set, PE_IMAGEYOFFSET) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEYOFFSET) < 0.0) SFLOAT(set, PE_IMAGEYOFFSET) = 0.0;
      break;
    case COM_FCOUNT_RST:
      SINT(set, PE_FRAMECOUNT) = patternSet[PE_FRAMECOUNT].initial.i;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_FCOUNT_INC:
      SINT(set, PE_FRAMECOUNT)++;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_FCOUNT_DEC:
      SINT(set, PE_FRAMECOUNT)--;
      if (SINT(set, PE_FRAMECOUNT) < 1) SINT(set, PE_FRAMECOUNT) = 1;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
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
void UpdateDisplays(int set, unsigned char sendToTensor, float intensity_limit) {
  unsigned char fba[TENSOR_BYTES];
  unsigned char *buffer;
  int i;
  int currentSet = set; // Override global currentSet for D*.

  // The preview...
  buffer = DBUFFER(PE_FRAMEBUFFER);

  // Post rotation
  if (DBOOL(PE_POSTRZ)) {

    // Apply the post rotation, so that it doesn't affect the feedback buffer.
    Rotate(DFLOAT(PE_POSTRZANGLE), DFLOAT(PE_POSTEXP), DBOOL(PE_ALIAS), fba, buffer);

  } else {

    // Just copy the buffer, because we aren't rotating it.
    memcpy(fba, DBUFFER(PE_FRAMEBUFFER), patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  if (set == alternateSet) {
    // For alternate blending with the post.
    memcpy(fbb, fba, patternSet[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  // Send to the preview and to the wall
  if (sendToTensor) {
    // Alternate blending
    BlendAlternate(fba);

    UpdatePreview(PREVIEW_A_POSITION_X, PREVIEW_A_POSITION_Y, fba);
    for (i = 0 ; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
    }
    if (enableTensor) {
      UpdateTensor(fba);
    }

  } else {
    UpdatePreview(PREVIEW_B_POSITION_X, PREVIEW_B_POSITION_Y, fba);
  }

  return;
}

void BlendAlternate(unsigned char *fba) {
  int i, j;
  color_t temp;

  // Too small to see...
  //~ if (alternateBlend < 0.001) return;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j++) {
      temp = GetPixel(i,j,fbb);
      temp.a = alternateBlend * 255;
      SetPixelA(i, j, temp, fba);
    }
  }

  // Autoblending
  if (autoBlend) {
    alternateBlend += alternateBlendRate;
    if (alternateBlend > 1.0) {
      alternateBlend = 0.0;
      autoBlend = NO;
      i = currentSet;
      currentSet = alternateSet;
      alternateSet = i;
    }
  }
}

// Rotate
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src) {
  SDL_Surface *rotatedSurface;
  SDL_Surface *scratch;
  SDL_Rect offset;

  scratch = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight, 32, 0, 0, 0, 0);
  FBToSurface(scratch, fb_src);

  // Rotate / scale it.
  rotatedSurface = rotozoomSurface (scratch, angle, expansion, aliasmode ? 1 : 0);
  if (!rotatedSurface) {
    fprintf(stderr, "Error rotating surface: %s\n", SDL_GetError());
    return;
  }

  // Recenter and copy it back to the scratchSurface
  offset.x = 0 - (rotatedSurface->w - scratch->w) / 2;
  offset.y = 0 - (rotatedSurface->h - scratch->h) / 2;
  SDL_FillRect(scratch, NULL, 0);
  SDL_BlitSurface(rotatedSurface, NULL, scratch, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, scratch);
  SDL_FreeSurface(rotatedSurface);
  SDL_FreeSurface(scratch);
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
void WriteSlice(int set) {
  int currentSet = set;
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
  static unsigned char textStaggerFlag[TEXT_BUFFER_SIZE] = "a";

  // Initialize textStaggerFlag, if necessary.  This is used for 9 row stagger,
  // which is meant for an 8 pixel font height on a 9 pixel high display.
  if (textStaggerFlag[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      textStaggerFlag[i] = rand() % 2;
    }
  }


  // Figure out which row or column to place the seed slice into depending on direction.
  horizontal = YES;
  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
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
  pixelBufferSize = strlen(DSTRING(PE_TEXTBUFFER)) * FONT_WIDTH;

  // Trivially, no text in the buffer.
  if (pixelBufferSize == 0) {
    // Just exit.  This actually causes the background to stop scrolling, which
    // might not be desireable.
    return;
  }

  // Prior to writing out a line, we increment the pixel buffer index to
  // point to the next part of the line to be written.  This depends on
  // scroll direction, text direction.
  textDirection = DENUM(PE_SCROLLDIR);
  if (DBOOL(PE_FONTDIR) == BACKWARDS) {
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
  if (DENUM(PE_SCROLLDIRLAST) != DENUM(PE_SCROLLDIR)) {

    // Looks like we changed direction.
    DENUM(PE_SCROLLDIRLAST) = DENUM(PE_SCROLLDIR);

    // tensorWidth is used here to preserve continuity of the text, even
    // across text buffer wrap-around, when the direction changes (but only
    // really between UP - DOWN or RIGHT - LEFT changes).
    switch(textDirection) {
      case DIR_LEFT:
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + tensorWidth) % pixelBufferSize;
        break;
      case DIR_RIGHT:
        DINT(PE_PIXELINDEX) = DINT(PE_PIXELINDEX) - tensorWidth;
        if (DINT(PE_PIXELINDEX) < 0) {
          DINT(PE_PIXELINDEX) = pixelBufferSize + DINT(PE_PIXELINDEX);
        }
        break;
      case DIR_DOWN:
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + tensorHeight) % pixelBufferSize;
        break;
      case DIR_UP:
        DINT(PE_PIXELINDEX) = DINT(PE_PIXELINDEX) - tensorHeight;
        if (DINT(PE_PIXELINDEX) < 0) {
          DINT(PE_PIXELINDEX) = pixelBufferSize + DINT(PE_PIXELINDEX);
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
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + 1) % pixelBufferSize;
        break;
      case DIR_RIGHT:
      case DIR_UP:
        DINT(PE_PIXELINDEX)--;
        if ((DINT(PE_PIXELINDEX) < 0) || (DINT(PE_PIXELINDEX) >= pixelBufferSize)) {
          DINT(PE_PIXELINDEX) = pixelBufferSize - 1;
        }
        break;
      default:
        break;
    }
  }

  // Now using the pixel index, we find out where in the text buffer we are.
  // (Integer division.)
  bufferIndex = DINT(PE_PIXELINDEX) / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (DINT(PE_PIXELINDEX) % FONT_WIDTH);

  // What the character is.  What it is, man.
  character = DSTRING(PE_TEXTBUFFER)[bufferIndex];

  // First we draw the seed's background according to our textStagger mode.
  useRand = 0;
  switch (DSENUM(PE_TEXTMODE, textMode_e)) {
    case TS_9ROWSTAGGER:
      // Stagger.  For landscape - 8 pixel font on 9 pixels high display.
      // Stagger the letter and fill in the pixel that isn't covered by the
      // letter with our background.
      sliceIndex = DINT(PE_TEXTOFFSET);
      if (!textStaggerFlag[bufferIndex]) sliceIndex += 8;
      if (horizontal) {
        sliceIndex = sliceIndex % tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex = sliceIndex % tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      useRand = textStaggerFlag[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = DINT(PE_TEXTOFFSET) - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = tensorHeight - 1;
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        if (sliceIndex < 0) sliceIndex = tensorWidth - 1;
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      sliceIndex = DINT(PE_TEXTOFFSET) + 8;
      if (horizontal) {
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < tensorHeight; i++) {
          SetPixelA(sliceColOrRow, i, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
        }
      } else {
        for (i = 0; i < tensorWidth; i++) {
          SetPixelA(i, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
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
    if (DBOOL(PE_FONTFLIP)) {
      j = (FONT_HEIGHT - 1) - i;
    } else {
      j = i;
    }
    fontPixelIndex = (j * FONT_CHARACTER_COUNT) + character;
    sliceIndex = i + useRand + DINT(PE_TEXTOFFSET);
    pixelOn = myfont[fontPixelIndex] & charColMasks[charCol];
    if (horizontal) {
      sliceIndex = sliceIndex % tensorHeight;
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? DCOLOR(PE_FGC) : DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
    } else {
      sliceIndex = sliceIndex % tensorWidth;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? DCOLOR(PE_FGC) : DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
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



color_t ColorCycle(int set, colorCycleModes_e cycleMode, int *cycleSaver, int cycleInc) {
  color_t colorTemp = cBlack;
  int inpos, inposo;
  int currentSet = set;

  // If the mode changed, we should make sure our position in the cycle limits
  // to the mode.
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
      *cycleSaver = (*cycleSaver + cycleInc) % (256 * 6);
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
      colorTemp.r = rand() % 256;
      colorTemp.g = rand() % 256;
      colorTemp.b = rand() % 256;
      break;

    case CM_FGBGFADE:
      *cycleSaver = (*cycleSaver + cycleInc) % (256 * 2);
      inpos = *cycleSaver / 256;
      inposo = *cycleSaver % 256;
      switch(inpos) {
        case 0: // FG - BG
          colorTemp.r = ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_FGE)].color.r + ((inposo / 255.0) * namedPalette[DENUM(PE_BGE)].color.r);
          colorTemp.g = ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_FGE)].color.g + ((inposo / 255.0) * namedPalette[DENUM(PE_BGE)].color.g);
          colorTemp.b = ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_FGE)].color.b + ((inposo / 255.0) * namedPalette[DENUM(PE_BGE)].color.b);
          break;

        case 1: // FG - BG
          colorTemp.r = ((inposo / 255.0) * namedPalette[DENUM(PE_FGE)].color.r) + ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_BGE)].color.r;
          colorTemp.g = ((inposo / 255.0) * namedPalette[DENUM(PE_FGE)].color.g) + ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_BGE)].color.g;
          colorTemp.b = ((inposo / 255.0) * namedPalette[DENUM(PE_FGE)].color.b) + ((255.0 - inposo) / 255) * namedPalette[DENUM(PE_BGE)].color.b;

        default:
          break;
      }
      break;




    case CM_INVALID: case CM_NONE: case CM_COUNT:
    //~ default:
      break;
  }

  // We used to return it...
  return colorTemp;
}


// Write the static menu information to the display.
void InitDisplayTexts(void) {
  int i;
  // DrawBox(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, cDkGray);
  for (i = 0; i < DISPLAYCOMMAND_SIZE; i++) {
    WriteCommand(i, displayCommand, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
  }

  for (i = 0; i < HEADERTEXT_SIZE; i++) {
    WriteLine(headerText[i].text, headerText[i].line, headerText[i].col, DISPLAY_COLOR_TITLES, DISPLAY_COLOR_TITLESBG);
  }

  for (i = 0; i < OTHERCOMMAND_SIZE; i++) {
    WriteCommand(i, otherCommand, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
  }

  for (i = 0; i < DISPLAYTEXT_SIZE; i++) {
    WriteLine(displayText[i].text, displayText[i].line, displayText[i].col, DISPLAY_COLOR_INFO, DISPLAY_COLOR_INFOBG);
  }
}

void WriteCommand(int index, const command_t *comList, color_t fg, color_t bg) {
  char text[100] = "";
  char *ind = text;
  char * const end = text + sizeof(text);
  int i;

  if (displayCommand[index].line == -1) {
    return;
  }

  // Handle the modifier keys - worked well when we had only one modifier key
  // possible for each command, but now... Strategy: Show the modifiers from
  // the first command in the list that doesn't have KMOD_NONE, and assume it
  // will be the same for all three.
  for (i = 0; i < MOUSE_COUNT; i++) {
    if (comList[index].commands[i].mod != KMOD_NONE) {
      ind += snprintf(ind, end - ind, "%s%s%s",
        comList[index].commands[i].mod & KMOD_CTRL ? "<c> " : "",
        comList[index].commands[i].mod & KMOD_SHIFT ? "<s> " : "",
        comList[index].commands[i].mod & KMOD_ALT ? "<a> " : "");
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

  // Add the text from the displayCommand structure.
  ind += snprintf(ind, end - ind, "%s", comList[index].text);

  // Write it.
  WriteLine(text, comList[index].line, comList[index].col, fg, bg);
}


// Update the values of the text on the display window.
void UpdateInfoDisplay(int set) {
  char text[102], mybuff[102];
  int length;
  int i;

  for (i = 0; i < DISPLAYCOMMAND_SIZE; i++) {
    if (displayCommand[i].dat != PE_INVALID) {
      switch(patternSet[displayCommand[i].dat].type) {
        case ET_BOOL:
          WriteBool(SBOOL(set, displayCommand[i].dat), displayCommand[i].line, displayCommand[i].col + 1, 10);
          break;

        case ET_FLOAT:
          WriteFloat(SFLOAT(set, displayCommand[i].dat), displayCommand[i].line, displayCommand[i].col + 1, 10, 5);
          break;

        case ET_INT:
          WriteInt(SINT(set, displayCommand[i].dat), displayCommand[i].line, displayCommand[i].col + 1, 10);
          break;

        case ET_ENUM:
          WriteString(enumerations[patternSet[displayCommand[i].dat].etype].texts[SENUM(set, displayCommand[i].dat)],
            displayCommand[i].line, displayCommand[i].col + 1, 10);
          break;

        default:
          // Lazy
          fprintf(stderr, "No data display driver for this type of information (%i).\n", patternSet[displayCommand[i].dat].type);
      }
    }
  }

  // The ones that aren't automatic:
  WriteInt(previewFPSA, 21, 1, 10);
  WriteInt(previewFPSB, 21, 5, 10);
  WriteInt(guiFPS, 51, 5, 10);
  if (cyclePatternSets) {
    WriteInt(cycleFrameCount, ROW_PA + 1, COL_PA + 1, 10);
  } else {
    WriteBool(cyclePatternSets, ROW_PA + 1, COL_PA + 1, 10);
  }
  WriteInt(alternateSet, ROW_PA + 9, COL_PA + 1, 10);
  WriteInt(currentSet, ROW_PA + 8, COL_PA + 1, 10);
  WriteString(operateText[displaySet], ROW_PA + 10, COL_PA + 1, 10);
  WriteFloat(alternateBlend, ROW_PA + 12, COL_PA + 1, 10, 5);
  WriteFloat(alternateBlendRate, ROW_PA + 13, COL_PA + 1, 10, 5);
  WriteBool(autoBlend, ROW_PA + 14, COL_PA + 1, 10);

  // Show the last 100 bytes of the text buffer.
  length = strlen(SSTRING(set, PE_TEXTBUFFER));
  WriteInt(length, 51, 1, 10);
  strncpy(mybuff, SSTRING(set, PE_TEXTBUFFER) + (length > 100 ? length - 100 : 0), 101);
  // The extra snprintf takes care of overwriting longer lines.
  snprintf(text, sizeof(text), "%-100s", mybuff );
  WriteLine(text, 52, 0, DISPLAY_COLOR_TBUF, DISPLAY_COLOR_TBUFBG);
}

void WriteBool(int value, int row, int col, int width) {
  char text[100];
  if (value) {
    snprintf(text, sizeof(text), "%*.*s", width, width, "YES");
  } else {
    snprintf(text, sizeof(text), "%*.*s", width, width, "NO");
  }
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

void WriteInt(int value, int row, int col, int width) {
  char text[100];
  snprintf(text, sizeof(text), "%*i", width, value);
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
}

void WriteFloat(float value, int row, int col, int width, int precision) {
  char text[100];
  char text2[100];
  snprintf(text, sizeof(text), "%*.*f", width, precision, value);
  snprintf(text2, sizeof(text2), "%.*s", width, text); // Hard limit the width
  WriteLine(text2, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
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
void DrawImage(int set, double angle, float xoffset, float yoffset, double expansion, int aliasmode, unsigned char *fb_dst) {
  SDL_Surface *rotatedImage;
  SDL_Surface *scratch;
  SDL_Rect offset;

  // Don't if it isn't.
  if (!imageSeed[set]) return;

  // Rotate / scale it.
  rotatedImage = rotozoomSurface (imageSeed[set], angle, expansion, aliasmode ? 1 : 0);
  if (!rotatedImage) {
    fprintf(stderr, "Error rotating image: %s\n", SDL_GetError());
    return;
  }

  scratch = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight, 32, 0, 0, 0, 0);

  // This clips the image and recenters it on the preview window.

  offset.x = 0 - SFLOAT(set, PE_IMAGEXOFFSET) * (rotatedImage->w - scratch->w);
  offset.y = 0 - SFLOAT(set, PE_IMAGEYOFFSET) * (rotatedImage->h - scratch->h);
  SDL_BlitSurface(rotatedImage, NULL, scratch, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, scratch);
  SDL_FreeSurface(rotatedImage);
  SDL_FreeSurface(scratch);
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
  DrawBox(x + 1, y + 1, maxDim - 2, maxDim - 2, cGray);

  // Ajust x and y to center the preview.
  x = x + (maxDim - w) / 2;
  y = y + (maxDim - h) / 2;

  // Draw the new outer border.
  DrawRectangle(x, y, w, h, cWhite);
  DrawBox(x + 1, y + 1, w - 2, h - 2, cBlack);

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
void CellFun(int set) {
  int currentSet = set; // Override global for D*
  int x, y;
  color_t pixelColor, oldColor;

  DINT(PE_CELLFUNCOUNT)++;

  for(x = 0 ; x < tensorWidth ; x++) {
    for(y = 0 ; y < tensorHeight ; y++) {
      oldColor = GetPixel(x, y, DBUFFER(PE_FRAMEBUFFER));
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2);
      pixelColor.g = oldColor.g + pixelColor.r;
      pixelColor.b = DINT(PE_CELLFUNCOUNT);
      SetPixel(x, y, pixelColor,  DBUFFER(PE_FRAMEBUFFER));
    }
  }
}

// Saves a pattern set to a file.
void SavePatternSet(char key, int set) {
  char filename[8] = "";
  FILE *fp;
  int i,j;

  // Filename
  snprintf(filename, sizeof(filename), "%c.now", key);
  fprintf(stdout, "Save filename: %s\n", filename);

  // Open the file for write.
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file for output: %s", filename);
    return;
  }

  // Write the version information to the file just in case we ever use this.
  fprintf(fp, "Tensor pattern parameter set file\n");
  fprintf(fp, "Version %s.%s.%s.%s\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, PRERELEASE_VERSION);

  // Write the file according to the structure
  for (i = 0; i < PSET_SIZE; i++) {
    //~ printf("Saving element %i - %s\n", i, patternSet[i].name);
    fprintf(fp, "%s ", patternSet[i].name);
    switch(patternSet[i].type) {
      case ET_BOOL:
        fprintf(fp, "%s", SBOOL(set, i) ? "YES" : "NO");
        break;
      case ET_INT: case ET_ENUM:
        fprintf(fp, "%i", SINT(set, i));
        break;
      case ET_FLOAT:
        fprintf(fp, "%f", SFLOAT(set, i));
        break;
      case ET_COLOR:
        fprintf(fp, "%i %i %i %i", SCOLOR(set, i).r, SCOLOR(set, i).g, SCOLOR(set, i).b, SCOLOR(set, i).a);
        break;
      case ET_STRING:
        fprintf(fp, "%s", SSTRING(set, i));
        break;
      case ET_BUFFER:
        for (j = 0; j < patternSet[i].size; j++) {
          fprintf(fp, "%02x ", SBUFFER(set, i)[j]);
        }
        break;
      default:
      case ET_INVALID:
      case ET_COUNT:
        fprintf(stderr, "Error!\n");
        break;
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

// Loads a pattern set from a file.
void LoadPatternSet(char key, int set) {
  int currentSet = set; // Override global for D*.
  char filename[8] = "";
  FILE *fp;
  char *ikey = NULL;
  char *value = NULL;
  char *saveptr = NULL;
  char *tmp = NULL;
  int keySize = 0;
  int valueSize = 0;
  int keyCount = 0;
  int valueCount = 0;
  int inChar;
  int result;
  typedef enum processing_e { P_KEY, P_VALUE, P_EOL } processing_e;
  processing_e lookingFor = P_KEY;
  int i;
  int parameterIndex = INVALID;
  int z, y, x, w;
  color_t temp;
  char *token;
  unsigned char * bufferData;

  // Filename
  snprintf(filename, sizeof(filename), "%c.now", key);
  fprintf(stdout, "Load filename: %s\n", filename);

  // Open file.
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    return;
  }

  // Read file.  We'll do it character by character.  At the start of a line,
  // we read in characters until we hit a space or newline.  If its a space,
  // we'll check the read characters against known tokens.  If its recognized,
  // we'll read the rest of the line and process the input, otherwise we'll drop
  // the rest until the next newline.
  FOREVER {

    // Get a character from the file.
    inChar = fgetc(fp);

    // EOF? Done.
    if (inChar == EOF) break;

    // If we are currently looking for a key...
    if (lookingFor == P_KEY) {

      // A space denotes the end of a key.
      if (inChar == ' ') {

        // Key done, check it against the known keys.
        for (i = 0; i < PSET_SIZE; i++) {
          result = strcasecmp(ikey, patternSet[i].name);
          // If it was found, stop checking.
          if (result == 0) break;
        }

        if (result == 0) {
          // The key was found amongst our known keys.
          //~ printf("Key found (%i - %s) = \"%s\"\n", i, patternSet[i].name, ikey);
          parameterIndex = i;  // Save aside the parameter we're working on.
          keyCount = 0;  // Reset for next key processing.
          lookingFor = P_VALUE;  // Gather the parameter's value.

        } else {
          // This is an unknown key.
          //~ printf("Key unknown - \"%s\"\n", ikey);
          // Look for the end of the line, ignoring the rest of the characters.
          lookingFor = P_EOL;
          keyCount = 0;  // Reset for next key processing.
        }

      // A newline while looking for a key denotes a valueless key.  There are
      // none of these, so we'll start looking for the next key.
      } else if (inChar == '\n') {
        //~ printf("Key, no value: \"%s\"\n", ikey);
        keyCount = 0;
        lookingFor = P_KEY;

      // Any other letter is part of the key we are looking for.  Add it to the
      // key string.
      } else {

        // See if there's room.
        keyCount++;
        if (keyCount >= keySize) {
          //~ printf("Allocating key memory. Initial size: %i ", keySize);
          keySize+=100;  // Allocate memory in chunks.
          //~ printf("New size: %i\n", keySize);
          ikey = realloc(ikey, keySize * sizeof(char));
          if (!ikey) {
            fprintf(stderr, "Unable to allocate key memory!\n");
            break;
          }
        }

        // Add the character on to the end of the string and terminate it.
        ikey[keyCount - 1] = inChar;
        ikey[keyCount] = '\0';
      }

    // Looking for the end of line, but ignoring the rest of the text.
    } else if (lookingFor == P_EOL) {
      if (inChar == '\n') {
        lookingFor = P_KEY;
      }

    // Looking for the value
    } else {

      // Newline denotes end of the value string.  Process the result.
      if (inChar == '\n') {
        //~ printf("Value of %s (type %i) = \"%s\"\n", patternSet[parameterIndex].name, patternSet[parameterIndex].type, value);
        // Process the result by parameter type.
        switch(patternSet[parameterIndex].type) {
          case ET_INT: case ET_ENUM:
            DINT(parameterIndex) = atoi(value);
            break;
          case ET_BOOL:
            if (strncasecmp(value, "YES", 3) == 0) {
              DBOOL(parameterIndex) = YES;
            } else if (strncasecmp(value, "NO", 2) == 0) {
              DBOOL(parameterIndex) = NO;
            } else {
              fprintf(stderr, "Ignoring invalid value for boolean parameter %s. (\"%s\")\n", patternSet[parameterIndex].name, value);
            }
            break;
          case ET_FLOAT:
            DFLOAT(parameterIndex) = atof(value);
            break;
          case ET_STRING:
            strncpy(DSTRING(parameterIndex), value, patternSet[parameterIndex].size);
            // Just in case...
            DSTRING(parameterIndex)[patternSet[parameterIndex].size - 1] = '\0';
            break;
          case ET_COLOR:
            z = y = x = w = INVALID;
            if (sscanf(value, "%i %i %i %i", &z, &y, &x, &w) == 4) {
              if (z >= 0 && z <= 255 && y >= 0 && y <= 255 &&
                  x >= 0 && x <= 255 && w >= 0 && w <= 255) {
                temp.r = z;
                temp.g = y;
                temp.b = x;
                temp.a = w;
                DCOLOR(parameterIndex) = temp;
              } else {
                fprintf(stderr, "Ignoring inappropriate format for color of %s: \"%s\"\n", patternSet[parameterIndex].name, value);
              }
            } else {
              fprintf(stderr, "Ignoring inappropriate format for color of %s: \"%s\"\n", patternSet[parameterIndex].name, value);
            }
            break;
          case ET_BUFFER:
            // Count the input bytes, check for valid values, save aside the data.
            tmp = value;
            y = w = 0;
            token = strtok_r(tmp, " ", &saveptr);
            bufferData = malloc(patternSet[parameterIndex].size * sizeof(unsigned char));
            if (!bufferData) {
              fprintf(stderr, "Couldn't allocate space for buffer input!\n");
              break;
            }
            while (token) {
              //~ printf("Process token %s as #%i\n", token, y);
              z = INVALID;
              x = sscanf(token, "%x", &z);
              if (z >= 0 && z <= 255 && x == 1) {
                bufferData[y] = z;
                y++;
              } else {
                w = INVALID;
                break;
              }
              token = strtok_r(NULL, " ", &saveptr);
            }

            // If the data was valid, apply it to the buffer.
            if (w != INVALID && y == patternSet[parameterIndex].size) {
              //~ printf("Valid buffer input found.\n");
              for (i = 0; i < patternSet[parameterIndex].size; i++) {
                DBUFFER(parameterIndex)[i] = bufferData[i];
              }
            } else {
              fprintf(stderr, "Ignoring invalid buffer data for %s.\n", patternSet[parameterIndex].name);
            }
            free(bufferData);
            break;
          case ET_COUNT: case ET_INVALID:
            fprintf(stderr, "What?\n");
            break;
        }
        lookingFor = P_KEY;
        valueCount = 0;

      // Get the next character and add it to the value string.
      } else {

        // Check to make sure we have room to add a character.
        valueCount++;
        if (valueCount >= valueSize) {

          // Allocate more memory for the value.
          //~ printf("Allocating value memory. Initial size: %i ", valueSize);
          valueSize+=100;
          //~ printf("New size: %i\n", valueSize);
          value = realloc(value, valueSize * sizeof(char));
          if (!value) {
            fprintf(stderr, "Unable to allocate value memory!\n");
            break;
          }
        }

        // Add the character on to the end of the string and terminate it.
        value[valueCount - 1] = inChar;
        value[valueCount] = '\0';
      }
    }
  }

  free(ikey);
  free(value);
  fclose(fp);

  // Now, the new set may have specified an image to load.  Let's do that here.
  SDL_FreeSurface(imageSeed[set]);
  imageSeed[set] = IMG_Load(DSTRING(PE_IMAGENAME));
  if (!imageSeed[set]) {
    fprintf(stderr, "Unable to load image: \"%s\"\n", DSTRING(PE_IMAGENAME));
  }
}


void DrawSideBar(int set) {
  int currentSet = set; // Override global for D*
  int i;

  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      for (i = 0; i < tensorHeight; i++) {
              SetPixel(tensorWidth - 1, i, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
            }
            break;

    case DIR_RIGHT:
      for (i = 0; i < tensorHeight; i++) {
              SetPixel(0, i, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
            }
            break;

    case DIR_UP:
      for (i = 0; i < tensorWidth; i++) {
              SetPixel(i, tensorHeight - 1, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
            }
            break;

    case DIR_DOWN:
      for (i = 0; i < tensorWidth; i++) {
              SetPixel(i, 0, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
            }
            break;
    default:
      break;
  }
}

void ClearRed(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 0] = 0;
  }
}

void ClearGreen(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 1] = 0;
  }
}

void ClearBlue(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 2] = 0;
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

// Copy a pattern set to another.  Gotta be done element by element because
// pattern sets aren't internally contiguous.
void CopyPatternSet(int dst, int src) {
  int i;
  if (src == dst) return;
  for (i = 0; i < PSET_SIZE; i++) {
    switch(patternSet[i].type) {
      case ET_BOOL: SBOOL(dst, i) = SBOOL(src, i); break;
      case ET_INT: SINT(dst, i) = SINT(src, i); break;
      case ET_FLOAT: SFLOAT(dst, i) = SFLOAT(src, i); break;
      case ET_COLOR: SCOLOR(dst, i) = SCOLOR(src, i); break;
      case ET_ENUM: SENUM(dst, i) = SENUM(src, i); break;
      case ET_STRING: memcpy(SSTRING(dst, i), SSTRING(src, i), patternSet[i].size); break;
      case ET_BUFFER: memcpy(SBUFFER(dst, i), SBUFFER(src, i), patternSet[i].size); break;
      case ET_COUNT: case ET_INVALID: break;
    }
  }
}

