// Tensor pattern generator
// Structural elements, pattern data definitions, gui commands.
// Joshua Krueger
// tensor@core9.org
// 2014_12_04

#include <limits.h>
#include <SDL.h>

#include "elements.h"
#include "draw.h"
#include "drv-tensor.h"

// Fade modes
const char *fadeModeText[] = { "Limited", "Modular" };
const int fadeModeTextCount = sizeof(fadeModeText) / sizeof(const char *);

// Color plane shift modes - same order as dir_e please.
const char *shiftText[] = { "Up", "Left", "Down", "Right", "Hold" };
const int shiftTextCount = sizeof(shiftText) / sizeof(const char *);

// Scroller directions.
const char *dirText[] = { "Up", "Left", "Down", "Right" };
const int dirTextCount = sizeof(dirText) / sizeof(const char *);

// Color cycling modes
const char *colorCycleText[] = { "None", "R-G-B", "C-M-Y", "Secondary", "Tertiary", "Graystep", "Rainbow", "Random", "FG-BG Fade", "Tertiary-Blk" };
const int colorCycleTextCount = sizeof(colorCycleText) / sizeof(const char *);

// Text background modes
const char *textModeText[] = { "8 Row", "9 Row Stag", "10 Row", "Full BG", "No BG", "Black" };
const int textModeTextCount = sizeof(textModeText) / sizeof(const char *);

// Crossbar seed types
const char *crossbarText[] = { "None", "Vertical", "Horizontal", "V & H"};
const int crossBarTextCount = sizeof(crossbarText) / sizeof(const char *);

// Pattern set that the controls operate on
const char *operateText[] = { "Live", "Alternate" };
const int operateTextCount = sizeof(operateText) / sizeof(const char *);

// This palette order should line up with color_e enumeration values and colorsText.
const colorName_t namedColors[] = {
  { CE_RED,        CD_RED},
  { CE_ORANGE,     CD_ORANGE},
  { CE_YELLOW,     CD_YELLOW},
  { CE_CHARTREUSE, CD_CHARTREUSE},
  { CE_GREEN,      CD_GREEN},
  { CE_AQUA,       CD_AQUA},
  { CE_CYAN,       CD_CYAN},
  { CE_AZURE,      CD_AZURE},
  { CE_BLUE,       CD_BLUE},
  { CE_VIOLET,     CD_VIOLET},
  { CE_MAGENTA,    CD_MAGENTA},
  { CE_ROSE,       CD_ROSE},
  { CE_WHITE,      CD_WHITE},
  { CE_LTGRAY,     CD_LTGRAY},
  { CE_GRAY,       CD_GRAY},
  { CE_DKGRAY,     CD_DKGRAY},
  { CE_BLACK,      CD_BLACK},
  { CE_HALF_RED,        CD_HALF_RED},
  { CE_HALF_ORANGE,     CD_HALF_ORANGE},
  { CE_HALF_YELLOW,     CD_HALF_YELLOW},
  { CE_HALF_CHARTREUSE, CD_HALF_CHARTREUSE},
  { CE_HALF_GREEN,      CD_HALF_GREEN},
  { CE_HALF_AQUA,       CD_HALF_AQUA},
  { CE_HALF_CYAN,       CD_HALF_CYAN},
  { CE_HALF_AZURE,      CD_HALF_AZURE},
  { CE_HALF_BLUE,       CD_HALF_BLUE},
  { CE_HALF_VIOLET,     CD_HALF_VIOLET},
  { CE_HALF_MAGENTA,    CD_HALF_MAGENTA},
  { CE_HALF_ROSE,       CD_HALF_ROSE},
  { CE_LTRED,        CD_LTRED},
  { CE_LTORANGE,     CD_LTORANGE},
  { CE_LTYELLOW,     CD_LTYELLOW},
  { CE_LTCHARTREUSE, CD_LTCHARTREUSE},
  { CE_LTGREEN,      CD_LTGREEN},
  { CE_LTAQUA,       CD_LTAQUA},
  { CE_LTCYAN,       CD_LTCYAN},
  { CE_LTAZURE,      CD_LTAZURE},
  { CE_LTBLUE,       CD_LTBLUE},
  { CE_LTVIOLET,     CD_LTVIOLET},
  { CE_LTMAGENTA,    CD_LTMAGENTA},
  { CE_LTROSE,       CD_LTROSE},
};
const int namedColorsCount = sizeof(namedColors) / sizeof(colorName_t);

// This is separated from the namedColors because of the enumerations array.
const char *colorsText[] = { "red", "orange", "yellow", "chartreuse", "green",
  "aqua", "cyan", "azure", "blue", "violet", "magenta", "rose", "white",
  "lt gray", "gray", "dk gray", "black", "1/2 red", "1/2 orange", "1/2 yellow",
  "1/2 chartreuse", "1/2 green", "1/2 aqua", "1/2 cyan", "1/2 azure",
  "1/2 blue", "1/2 violet", "1/2 magenta", "1/2 rose", "lt red", "lt orange",
  "lt yellow", "lt chartreuse", "lt green", "lt aqua", "lt cyan", "lt azure",
  "lt blue", "lt violet", "lt magenta", "lt rose",
};
const int colorsTextSize = sizeof(colorsText) / sizeof(const char *);

// Enumeration array provides the connection between the enumeration type, its
// list of possible text values, and the size of the list.
const enums_t enumerations[] = {
  { E_DIRECTIONS, dirText, DIR_COUNT},
  { E_FADEMODES, fadeModeText, FM_COUNT},
  { E_SHIFTMODES, shiftText, SM_COUNT},
  { E_TEXTMODES, textModeText, TS_COUNT},
  { E_COLORCYCLES, colorCycleText, CM_COUNT},
  { E_COLORS, colorsText, CE_COUNT},
  { E_OPERATE, operateText, OO_COUNT },
  { E_CROSSBAR, crossbarText, CB_COUNT },
};
const int enumerationsCount = sizeof(enumerations) / sizeof(enums_t);

// The pattern elements used by the engine.  This is currently not a constant
// due to the inclusion of the dynamically allocated dataset in the type.
// ints, floats have min & maxes.  enums have etypes.  buffers and string have sizes.
const patternElement_t patternElements[] = {
//  Access enum,    "Name",        TYPE,      initial,     min,      max,      size,     etype
  { PE_CELLFUN,     "CellFun",     ET_BOOL,   {.b = NO} },
  { PE_BOUNCER,     "CycleScroll", ET_INT,    {.i = 0}, {.i = 0}, {.i = INT_MAX} },
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
  { PE_FADEINC,     "FadeIncr",    ET_INT,    {.i = INITIAL_FADE_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_DIFFUSECOEF, "DiffuseCoef", ET_FLOAT,  {.f = INITIAL_DIFF_COEF}, {.f = -1000}, {.f = 1000} },
  { PE_SCROLLDIR,   "ScrollDir",   ET_ENUM,   {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_RANDOMDOTCOEF,"DotCoef",    ET_INT,    {.i = INITIAL_RAND_MOD}, {.i = 1}, {.i = 20000} },
  { PE_FGCYCLE,     "FGCycleMode", ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_BGCYCLE,     "BGCycleMode", ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_FGRAINBOW,   "RainbowFG",   ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_BGRAINBOW,   "RainbowBG",   ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_POSTEXP,     "Expansion",   ET_FLOAT,  {.f = INITIAL_EXPAND}, {.f = -20}, {.f = 20}  },
  { PE_FLOATINC,    "FloatInc",    ET_FLOAT,  {.f = INITIAL_FLOAT_INC}, {.f = 0.00001}, {.f = 1000000000} },
  { PE_POSTRZANGLE, "PostRotAngle",ET_FLOAT,  {.f = INITIAL_POSTROT_ANGLE}, {.f = -999999999}, {.f = 1000000000 } },
  { PE_POSTRZINC,   "PostRotInc",  ET_FLOAT,  {.f = INITIAL_POSTROT_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_MULTIPLYBY,  "MultiplyBy",  ET_FLOAT,  {.f = INITIAL_MULTIPLIER}, {.f = -999999999 }, {.f = 1000000000} },
  { PE_DELAY,       "Delay",       ET_INT,    {.i = INITIAL_DELAY}, {.i = 1}, {.i = INT_MAX}},
  { PE_CYCLESAVEFG, "FGCyclePos",  ET_INT,    {.i = 0} }, // Not user
  { PE_CYCLESAVEBG, "BGCyclePos",  ET_INT,    {.i = 0} }, // Not user
  { PE_FGC,         "FGColorC",    ET_COLOR,  {.c = CD_RED} },
  { PE_BGC,         "BGColorC",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_CELLFUNCOUNT,"CellFunCt",   ET_INT,    {.i = 0} }, // Not user
  { PE_TEXTBUFFER,  "TextBuffer",  ET_STRING, {.s = INITIAL_TEXT}, .size = TEXT_BUFFER_SIZE },
  { PE_TEXTMODE,    "TextMode",    ET_ENUM,   {.e = TS_FULLBG}, .etype = E_TEXTMODES },
  { PE_FONTFLIP,    "FontFlip",    ET_BOOL,   {.b = NO} },
  { PE_FONTDIR,     "FontDir",     ET_BOOL,   {.b = FORWARDS} },
  { PE_TEXTOFFSET,  "TextOffset",  ET_INT,    {.i = TENSOR_HEIGHT / 3 - 1}, {.i = 0}, {.i = TENSOR_WIDTH} },
  { PE_TEXTINDEX,   "TextIndex",   ET_INT,    {.i = sizeof(INITIAL_TEXT) - 1} }, // Not user
  { PE_PIXELINDEX,  "PixelIndex",  ET_INT,    {.i = INVALID} }, // Not user
  { PE_SCROLLDIRLAST,"ScrollDirLast",ET_ENUM, {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_TEXTSEED,    "TextSeed",    ET_BOOL,   {.b = YES} },
  { PE_FRAMEBUFFER, "FrameBuffer", ET_BUFFER, .size = TENSOR_BYTES },
  { PE_PRERZANGLE,  "PreRotAngle", ET_FLOAT,  {.f = INITIAL_PREROT_ANGLE}, {.f = -999999999}, {.f = 1000000000} },
  { PE_PRERZALIAS,  "PreRotAlias", ET_BOOL,   {.b = NO} },
  { PE_PRERZINC,    "PreRotInc",   ET_FLOAT,  {.f = INITIAL_POSTROT_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_PRERZEXPAND, "PreRotExp",   ET_FLOAT,  {.f = INITIAL_PREEXPAND}, {.f = -20}, {.f = 20} },
  { PE_FRAMECOUNT,  "CycleFCount", ET_INT,    {.i = INITIAL_FRAMECYCLECOUNT}, {.i = 1}, {.i = INT_MAX} },
  { PE_IMAGEANGLE,  "ImageAngle",  ET_FLOAT,  {.f = INITIAL_IMAGE_ANGLE}, {.f = -999999999}, {.f = 1000000000} },
  { PE_IMAGEALIAS,  "ImageAlias",  ET_BOOL,   {.b = NO} },
  { PE_IMAGEINC,    "ImageInc",    ET_FLOAT,  {.f = INITIAL_IMAGE_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_IMAGEEXP,    "ImageExp",    ET_FLOAT,  {.f = INITIAL_IMAGE_EXPAND}, {.f = -20}, {.f = 20} },
  { PE_IMAGEXOFFSET,"ImageXOffset",ET_FLOAT,  {.f = INITIAL_IMAGE_XOFFSET}, {.f = 0}, {.f = 1} },
  { PE_IMAGEYOFFSET,"ImageYOffset",ET_FLOAT,  {.f = INITIAL_IMAGE_YOFFSET}, {.f = 0}, {.f = 1} },
  { PE_IMAGENAME,   "ImageName",   ET_STRING, {.s = INITIAL_IMAGE}, .size = 1024},
  { PE_SNAIL,       "SnailShot",   ET_BOOL,   {.b = NO} },
  { PE_SNAIL_POS,   "SnailPos",    ET_INT,    {.i = 0} }, // Not user
  { PE_FASTSNAIL,   "FastSnail",   ET_BOOL,   {.b = NO} },
  { PE_FASTSNAILP,  "FastSnailP",  ET_INT,    {.i = 0} }, // Not user
  { PE_CROSSBAR,    "Crossbar",    ET_ENUM,   {.e = CB_NONE}, .etype = E_CROSSBAR },
  { PE_CBLIKELY,    "CrossLikely", ET_INT,    {.i = 1002 }, {.i = 1}, {.i = INT_MAX} },
  { PE_MIRROR_V,    "MirrorV",     ET_BOOL,   {.b = NO } },
  { PE_MIRROR_H,    "MirrorH",     ET_BOOL,   {.b = NO } },
  { PE_SCROLLRANDEN,"ScrollRandE", ET_BOOL,   {.b = NO } },
  { PE_SCROLLRANDOM,"ScrollRand",  ET_INT,    {.i = 50 }, {.i = 1}, {.i = INT_MAX} },
  { PE_SIDEPULSE,   "SidePulse",   ET_BOOL,   {.b = NO } },
  { PE_INTENSITY,   "Intensity",   ET_FLOAT,  {.f = 1.0}, {.f = 0}, {.f = 1} },
};
#define PATTERN_ELEMENT_COUNT ( sizeof(patternElements) / sizeof(patternElement_t) )
const int patternElementCount = PATTERN_ELEMENT_COUNT;

// Clickable commands displayed on the gui.
const command_t displayCommand[] = {
  // Pattern sets
  {ROW_PA + 1, COL_PA, "Cycle through pattern sets",   PE_INVALID,  {{KMOD_CTRL, SDLK_g, COM_CYCLESET}}},
  {ROW_PA + 2, COL_PA, "Frames to play this set", PE_FRAMECOUNT, {{0, 0, COM_FCOUNT_RST}, {0, 0, COM_FCOUNT_INC}, {0, 0, COM_FCOUNT_DEC}}},
  {ROW_PA + 8, COL_PA, "Live preview set (left)",PE_INVALID, {{0, 0, COM_LIVE_INC}, {}, {0, 0, COM_LIVE_DEC}}},
  {ROW_PA + 9, COL_PA, "Alternate preview set (right)",PE_INVALID, {{0, 0, COM_ALTERNATE_INC}, {}, {0, 0, COM_ALTERNATE_DEC}}},
  {ROW_PA + 10,COL_PA, "Controls operate on set",     PE_INVALID, {{0, 0, COM_OPERATE}}},
  {ROW_PA + 11,COL_PA, "Exchange live and alternate sets", PE_INVALID, {{0, 0, COM_EXCHANGE}}},
  {ROW_PA + 12,COL_PA, "Alternate set blending amount", PE_INVALID, {{0, 0, COM_BLEND_RST}, {0, 0, COM_BLEND_INC}, {0, 0, COM_BLEND_DEC}}},
  {ROW_PA + 13,COL_PA, "Auto blend & switch sets", PE_INVALID, {{0, 0, COM_BLENDSWITCH}}},
  {ROW_PA + 14,COL_PA, "Auto blend rate", PE_INVALID, {{0, 0, COM_BLENDINC_RST},{0, 0, COM_BLENDINC_INC}, {0, 0, COM_BLENDINC_DEC}}},

  // Auxillary
  {ROW_A + 1, COL_A, "Set output intensity", PE_INTENSITY,    {{0, 0, COM_INTENSITY_RST}, {0, 0, COM_INTENSITY_UP}, {0, 0, COM_INTENSITY_DOWN}}},
  {ROW_A + 2, COL_A, "Frame delay(ms)", PE_DELAY,    {{0, 0, COM_DELAY_RST}, {0, 0, COM_DELAY_INC}, {0, 0, COM_DELAY_DEC}}},
  {ROW_A + 3, COL_A, "Float step", PE_FLOATINC, {{KMOD_ALT, SDLK_COMMA, COM_STEP_RST}, {KMOD_ALT, SDLK_m, COM_STEP_INC}, {KMOD_ALT, SDLK_PERIOD, COM_STEP_DEC}}},
  {ROW_A + 4, COL_A, "All modes off",   PE_INVALID,  {{KMOD_CTRL, SDLK_l, COM_MODEOFF}}},

  // Plane suppression
  {ROW_P + 1, COL_P, "Supress red",     PE_NORED,    {{KMOD_CTRL, SDLK_m, COM_NORED}}},
  {ROW_P + 2, COL_P, "Supress green",   PE_NOGREEN,  {{KMOD_CTRL, SDLK_COMMA, COM_NOGREEN}}},
  {ROW_P + 3, COL_P, "Supress blue",    PE_NOBLUE,   {{KMOD_CTRL, SDLK_PERIOD, COM_NOBLUE}}},

  // Diffusion
  {ROW_D + 1, COL_D, "Enable",      PE_DIFFUSE,     {{KMOD_CTRL, SDLK_r, COM_DIFFUSE}}},
  {ROW_D + 2, COL_D, "Coefficient", PE_DIFFUSECOEF, {{KMOD_ALT, SDLK_w, COM_DIFFUSE_RST}, {KMOD_ALT, SDLK_q, COM_DIFFUSE_INC}, {KMOD_ALT, SDLK_e, COM_DIFFUSE_DEC}}},

  // One shot seeds
  {ROW_O + 1, COL_O, "Horizontal bars", PE_INVALID,  {{KMOD_CTRL, SDLK_i, COM_HBAR}}},
  {ROW_O + 2, COL_O, "Vertical bars",   PE_INVALID,  {{KMOD_CTRL, SDLK_o, COM_VBAR}}},
  {ROW_O + 3, COL_O, "Foreground all",  PE_INVALID,  {{KMOD_CTRL, SDLK_p, COM_FGALL}}},
  {ROW_O + 4, COL_O, "Background all",  PE_INVALID,  {{KMOD_CTRL, SDLK_a, COM_BGALL}}},
  {ROW_O + 5, COL_O, "Image seed",      PE_INVALID,  {{0, 0, COM_IMAGEALL}}},
  {ROW_O + 6, COL_O, "Small snail seed",      PE_SNAIL,    {{0, 0, COM_SNAIL}}},
  {ROW_O + 7, COL_O, "Large snail seed", PE_FASTSNAIL,{{0, 0, COM_SNAILFAST}}},

  // Persistent Seeds
  {ROW_MI + 1, COL_MI, "Cell pattern",    PE_CELLFUN,  {{KMOD_CTRL, SDLK_q, COM_CELL}}},
  {ROW_MI + 2, COL_MI, "Sidebar seed",    PE_BARSEED,  {{KMOD_CTRL, SDLK_n, COM_SIDEBAR}}},
  {ROW_MI + 3, COL_MI, "Sidepulse seed",  PE_SIDEPULSE,{{0, 0, COM_SIDEPULSE}}},

  // Crossbar seeds
  {ROW_CB + 1, COL_CB, "Enable",  PE_CROSSBAR, {{0, 0, COM_CROSSB_RST}, {0, 0, COM_CROSSB_UP}, {0, 0, COM_CROSSB_DOWN}}},
  {ROW_CB + 2, COL_CB, "Randomness",  PE_CBLIKELY, {{0, 0, COM_CB_RST}, {0, 0, COM_CB_UP}, {0, 0, COM_CB_DOWN}}},

  // Random Dots
  {ROW_R + 1, COL_R, "Enable",     PE_RANDOMDOT,     {{KMOD_CTRL, SDLK_s, COM_RDOT}}},
  {ROW_R + 2, COL_R, "Randomness", PE_RANDOMDOTCOEF, {{KMOD_ALT, SDLK_LEFTBRACKET, COM_RANDOM_RST}, {KMOD_ALT, SDLK_p, COM_RANDOM_INC}, {KMOD_ALT, SDLK_RIGHTBRACKET, COM_RANDOM_DEC}}},

  // Fader
  {ROW_F + 1, COL_F, "Enable",           PE_FADE,     {{KMOD_CTRL, SDLK_e,  COM_FADE}}},
  {ROW_F + 2, COL_F, "Amount",     PE_FADEINC,  {{KMOD_ALT, SDLK_x, COM_FADE_RST}, {KMOD_ALT, SDLK_z, COM_FADE_INC}, {KMOD_ALT, SDLK_c, COM_FADE_DEC}}},
  {ROW_F + 3, COL_F, "Mode",      PE_FADEMODE, {{KMOD_CTRL, SDLK_h, COM_FADEMODE}}},

  // Multiplier
  {ROW_M + 1, COL_M, "Enable",      PE_MULTIPLY, {{KMOD_CTRL, SDLK_c, COM_MULTIPLY}}},
  {ROW_M + 2, COL_M, "Multiply by", PE_MULTIPLYBY, {{KMOD_ALT, SDLK_i, COM_MULT_RST}, {KMOD_ALT, SDLK_u, COM_MULT_INC}, {KMOD_ALT, SDLK_o, COM_MULT_DEC}}},

  // Mirrors
  {ROW_MIR + 1, COL_MIR, "Vertical Mirror", PE_MIRROR_V, {{0, 0, COM_MIRROR_V}}},
  {ROW_MIR + 2, COL_MIR, "Horizontal Mirror", PE_MIRROR_H, {{0, 0, COM_MIRROR_H}}},

  // Colors
  {ROW_C + 1, COL_C, "Foreground",    PE_FGE,       {{KMOD_ALT | KMOD_CTRL, SDLK_q, COM_FG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_w, COM_FG_DEC}}},
  {ROW_C + 2, COL_C, "Background",    PE_BGE,       {{KMOD_ALT | KMOD_CTRL, SDLK_a, COM_BG_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_s, COM_BG_DEC}}},
  {ROW_C + 3, COL_C, "FG cycle mode", PE_FGCYCLE,   {{0, 0, COM_FGCYCLE_RST}, {KMOD_CTRL, SDLK_d, COM_FGCYCLE_UP}, {0, 0, COM_FGCYCLE_DOWN}}},
  {ROW_C + 4, COL_C, "BG cycle mode", PE_BGCYCLE,   {{0, 0, COM_BGCYCLE_RST}, {KMOD_CTRL, SDLK_f, COM_BGCYCLE_UP}, {0, 0, COM_BGCYCLE_DOWN}}},
  {ROW_C + 5, COL_C, "FG Delta",      PE_FGRAINBOW, {{KMOD_ALT, SDLK_g, COM_FGRAINBOW_RST}, {KMOD_ALT, SDLK_f, COM_FGRAINBOW_INC}, {KMOD_ALT, SDLK_h, COM_FGRAINBOW_DEC}}},
  {ROW_C + 6, COL_C, "BG Delta",      PE_BGRAINBOW ,{{0, 0, COM_BGRAINBOW_RST}, {0, 0, COM_BGRAINBOW_INC}, {0, 0, COM_BGRAINBOW_DEC}}},

  // Scrollers
  {ROW_S + 1, COL_S, "Scroller",             PE_SCROLL,       {{KMOD_CTRL, SDLK_u, COM_SCROLL}}},
  {ROW_S + 2, COL_S, "Scroll Direction: (<a>) <arrows>", PE_SCROLLDIR, {{0, 0, COM_SCROLL_CYCLE_UP}, {}, {0, 0, COM_SCROLL_CYCLE_DOWN}}},
  {ROW_S + 3, COL_S, "Direction randomizer", PE_SCROLLRANDEN,  {{0, 0, COM_SCROLL_RAND_EN}}},
  {ROW_S + 4, COL_S, "Direction randomness", PE_SCROLLRANDOM, {{0, 0, COM_SCROLL_RAND_RST}, {0, 0, COM_SCROLL_RAND_UP}, {0, 0, COM_SCROLL_RAND_DOWN}}},
  {ROW_S + 5, COL_S, "Cycle Scroller (0 = Off)",     PE_BOUNCER,  {{0, 0, COM_BOUNCE_RST}, {0, 0, COM_BOUNCE_UP}, {0, 0, COM_BOUNCE_DOWN}}},
  {ROW_S + 6, COL_S, "Red plane",            PE_SHIFTRED,     {{0, 0, COM_RP_RST}, {KMOD_CTRL, SDLK_LEFTBRACKET, COM_RP_UP}, {0, 0, COM_RP_DOWN}}},
  {ROW_S + 7, COL_S, "Green plane",          PE_SHIFTGREEN,   {{0, 0, COM_GP_RST}, {KMOD_CTRL, SDLK_RIGHTBRACKET, COM_GP_UP}, {0, 0, COM_GP_DOWN}}},
  {ROW_S + 8, COL_S, "Blue plane",           PE_SHIFTBLUE,    {{0, 0, COM_BP_RST}, {KMOD_CTRL, SDLK_BACKSLASH, COM_BP_UP}, {0, 0, COM_BP_DOWN}}},
  {ROW_S + 9, COL_S, "Cyan plane",          PE_SHIFTCYAN,    {{0, 0, COM_CP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_LEFTBRACKET, COM_CP_UP}, {0, 0, COM_CP_DOWN}}},
  {ROW_S + 10, COL_S, "Yellow plane",        PE_SHIFTYELLOW,  {{0, 0, COM_YP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_RIGHTBRACKET, COM_YP_UP}, {0, 0, COM_YP_DOWN}}},
  {ROW_S + 11, COL_S, "Magenta plane",       PE_SHIFTMAGENTA, {{0, 0, COM_MP_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_BACKSLASH, COM_MP_UP}, {0, 0, COM_MP_DOWN}}},
  {ROW_S + 12, COL_S, "Toroidal",             PE_ROLLOVER, {{KMOD_CTRL, SDLK_y, COM_ROLL}}},

  // Post rotozoom
  {ROW_PR + 1, COL_PR, "Enable",     PE_POSTRZ,      {{KMOD_CTRL, SDLK_k, COM_POSTROTATE}}},
  {ROW_PR + 2, COL_PR, "Angle",      PE_POSTRZANGLE, {{KMOD_ALT, SDLK_b, COM_POSTROT_RST}, {KMOD_ALT, SDLK_v, COM_POSTROT_INC}, {KMOD_ALT, SDLK_n, COM_POSTROT_DEC}}},
  {ROW_PR + 3, COL_PR, "Angle Inc",  PE_POSTRZINC,   {{KMOD_ALT, SDLK_k, COM_POSTSPEED_RST}, {KMOD_ALT, SDLK_j, COM_POSTSPEED_INC}, {KMOD_ALT, SDLK_l, COM_POSTSPEED_DEC}}},
  {ROW_PR + 4, COL_PR, "Expansion",  PE_POSTEXP,      {{KMOD_ALT, SDLK_s, COM_EXPAND_RST}, {KMOD_ALT, SDLK_a, COM_EXPAND_INC}, {KMOD_ALT, SDLK_d, COM_EXPAND_DEC}}},
  {ROW_PR + 5, COL_PR, "Anti-alias", PE_ALIAS,       {{KMOD_CTRL, SDLK_x, COM_AA}}},

  // Pre rotozoom
  {ROW_PE + 1, COL_PE, "Enable",    PE_PRERZ,      {{KMOD_CTRL, SDLK_z, COM_PREROTATE}}},
  {ROW_PE + 2, COL_PE, "Angle",     PE_PRERZANGLE, {{KMOD_ALT, SDLK_t, COM_PREROT_RST}, {KMOD_ALT, SDLK_r, COM_PREROT_INC}, {KMOD_ALT, SDLK_y, COM_PREROT_DEC}}},
  {ROW_PE + 3, COL_PE, "Angle Inc", PE_PRERZINC,   {{0, 0, COM_PREROTINC_RST}, {0, 0, COM_PREROTINC_INC}, {0, 0, COM_PREROTINC_DEC}}},
  {ROW_PE + 4, COL_PE, "Expansion", PE_PRERZEXPAND,{{0, 0, COM_PREEXP_RST}, {0, 0, COM_PREEXP_INC}, {0, 0, COM_PREEXP_DEC}}},
  {ROW_PE + 5, COL_PE, "Anti-alias",PE_PRERZALIAS, {{0, 0, COM_PREALIAS}}},

  // Image stuff
  {ROW_I + 1, COL_I, "Enable",        PE_POSTIMAGE,  {{KMOD_CTRL | KMOD_ALT, SDLK_p, COM_IMAGE}}},
  {ROW_I + 2, COL_I, "Angle",         PE_IMAGEANGLE, {{0, 0, COM_IMANGLE_RST}, {0, 0, COM_IMANGLE_INC}, {0, 0, COM_IMANGLE_DEC}}},
  {ROW_I + 3, COL_I, "Angle Inc",     PE_IMAGEINC,   {{0, 0, COM_IMINC_RST},   {0, 0, COM_IMINC_INC},   {0, 0, COM_IMINC_DEC}}},
  {ROW_I + 4, COL_I, "Expansion",     PE_IMAGEEXP,   {{0, 0, COM_IMEXP_RST},   {0, 0, COM_IMEXP_INC},   {0, 0, COM_IMEXP_DEC}}},
  {ROW_I + 5, COL_I, "Center x",      PE_IMAGEXOFFSET, {{0, 0, COM_IMXOFFSET_RST}, {0, 0, COM_IMXOFFSET_INC}, {0, 0, COM_IMXOFFSET_DEC}}},
  {ROW_I + 6, COL_I, "Center y",      PE_IMAGEYOFFSET, {{0, 0, COM_IMYOFFSET_RST}, {0, 0, COM_IMYOFFSET_INC}, {0, 0, COM_IMYOFFSET_DEC}}},
  {ROW_I + 7, COL_I, "Anti-alias",    PE_IMAGEALIAS, {{0, 0, COM_IMALIAS}}},

  // Text
  {ROW_T + 1, COL_T, "Text seed",         PE_TEXTSEED,   {{KMOD_CTRL, SDLK_t, COM_TEXT}}},
  {ROW_T + 3, COL_T, "Delete last letter", PE_INVALID,    {{KMOD_NONE, SDLK_BACKSPACE, COM_BACKSPACE}}},
  {ROW_T + 4, COL_T, "Erase all text",     PE_INVALID,    {{KMOD_NONE, SDLK_DELETE, COM_DELETE}}},
  {ROW_T + 5, COL_T, "Reverse text",      PE_FONTDIR,    {{KMOD_CTRL, SDLK_QUOTE, COM_TEXT_REVERSE}}},
  {ROW_T + 6, COL_T, "Flip text.",         PE_FONTFLIP,   {{KMOD_CTRL, SDLK_SEMICOLON, COM_TEXT_FLIP}}},
  {ROW_T + 7, COL_T, "Text background",   PE_TEXTMODE,   {{KMOD_CTRL, SDLK_b, COM_TEXT_MODE_UP}, {}, {0, 0, COM_TEXT_MODE_DOWN}}},
  {ROW_T + 8, COL_T, "Edge Offset",       PE_TEXTOFFSET, {{KMOD_ALT | KMOD_CTRL, SDLK_z, COM_TEXTO_INC}, {}, {KMOD_ALT | KMOD_CTRL, SDLK_x, COM_TEXTO_DEC}}},
  {ROW_T + 9, COL_T, "Restart text",       PE_INVALID,    {{KMOD_CTRL, SDLK_j, COM_TEXTRESET}}},

  // Quit
  {51, 4, "Quit",           PE_INVALID,  {{KMOD_NONE, SDLK_ESCAPE, COM_EXIT}}},
};
const int displayCommandCount = sizeof(displayCommand) / sizeof(command_t);

// Non clickable text displayed in the gui.
const displayText_t displayText[] = {
  {ROW_T + 2, COL_T, " Add text - <Unmodified keys>"},
  {ROW_PA + 3, COL_PA, " Change to set # - <c> 0-9"},
  {ROW_PA + 4, COL_PA, " Copy set to # - <c> <a> 0-9"},
  {ROW_PA + 5, COL_PA, " Load buffer from set # - <a> 0-9"},
  {ROW_PA + 6, COL_PA, " Save to disk - <a> <s> 0-9, a-z"},
  {ROW_PA + 7, COL_PA, " Load saved set - <c> <s> 0-9, a-z"},
};
const int displayTextCount = sizeof(displayText) / sizeof(displayText_t);

// Some labels on the gui.
const displayText_t labelText[] = {
  {21, 0,          "Live Preview          FPS:"},
  {21, 4,          "Alternate Preview    FPS:"},
  {53, 4,          "                 GUI FPS:"},
  {51, 0,          " Text buffer:"},
};
const int labelCount = sizeof(labelText) / sizeof(displayText_t);

// Headers with highlighted backgrounds on the gui.
const displayText_t headerText[] = {
  {ROW_PA, COL_PA, "Pattern sets:"},
  {ROW_P, COL_P,   "Color suppression:"},
  {ROW_A, COL_A,   "Auxiliary:"},
  {ROW_MI, COL_MI, "Misc seeds:"},
  {ROW_CB, COL_CB, "Crossbar seeds:"},
  {ROW_I, COL_I,   "Image overlay:"},
  {ROW_R, COL_R,   "Random dots:"},
  {ROW_C, COL_C,   "Colors:"},
  {ROW_D, COL_D,   "Averager:"},
  {ROW_O, COL_O,   "One-shot seeds:"},
  {ROW_F, COL_F,   "Adder (fader):"},
  {ROW_M, COL_M,   "Multiplier:"},
  {ROW_T, COL_T,   "Text entry:"},
  {ROW_S, COL_S,   "Scrollers:"},
  {ROW_PR, COL_PR, "Post rotation:"},
  {ROW_PE, COL_PE, "Pre rotation:"},
  {ROW_MIR, COL_MIR, "Mirrors:"},
};
const int headerTextCount = sizeof(headerText) / sizeof(displayText_t);

// These commands correspond to key presses, but aren't displayed as mouse-over items.
const command_t otherCommands[] = {
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
const int otherCommandsCount = sizeof(otherCommands) / sizeof(command_t);

// Global elements
// patternData holds the pattern set data, to be allocated after startup.
void *patternData[PATTERN_ELEMENT_COUNT];

// Check to make sure that some of the program's data sets are built correctly.
// This is done to ensure that certain arrays can be accessed via enumeration
// values rather than indicies (i.e. enumerations must match indecies).  It
// catches some easy programmer errors.
bool_t VerifyStructuralIntegrity(void) {
  int i;
  bool_t rc = YES;

  // patternElement_e and patternElements array.
  for (i = 0; i < patternElementCount; i++) {
    if (i != patternElements[i].index) {
      fprintf(stderr, "Programmer error: patternElement_e does not match patternElements array!\n");
      fprintf(stderr, "Element %i, \"%s\" has incorrect enumeration value %i!\n", i, patternElements[i].name, patternElements[i].index);
      rc = NO;
    }
  }
  if (patternElementCount != PE_COUNT) {
    fprintf(stderr, "Programmer error: Mismatched patternElements(%i) != patternElement_e(%i)!\n", patternElementCount, PE_COUNT);
    rc = NO;
  }

  // color_e and namedColors array.
  for (i = 0; i < namedColorsCount; i++) {
    if (i != namedColors[i].index) {
      fprintf(stderr, "Programmer error: color_e does not match namedColors array!\n");
      fprintf(stderr, "Element %i, has incorrect enumeration value %i!\n", i, namedColors[i].index);
      rc = NO;
    }
  }
  if (CE_COUNT != namedColorsCount) {
    fprintf(stderr, "Programmer error: Mismatched namedColors(%i) != color_e(%i)!\n", namedColorsCount, CE_COUNT);
    rc = NO;
  }
  if (CE_COUNT != colorsTextSize) {
    fprintf(stderr, "Programmer error: Mismatched color_e(%i) != colorText(%i)!\n", CE_COUNT, colorsTextSize);
    rc = NO;
  }

  // enums_e and enumerations array.
  for (i = 0; i < enumerationsCount; i++) {
    if (i != enumerations[i].type) {
      fprintf(stderr, "Programmer error: enums_e does not match enumerations array!\n");
      fprintf(stderr, "Element %i, has incorrect enumeration value %i!\n", i, enumerations[i].type);
      rc = NO;
    }
  }
  if (E_COUNT != enumerationsCount) {
    fprintf(stderr, "Programmer error: Mismatched enums_e(%i) != enumerations(%i)!\n", E_COUNT, enumerationsCount);
    rc = NO;
  }

  // Texts - we can only do a count on these.
  if (DIR_COUNT != dirTextCount) {
    fprintf(stderr, "Programmer error: Mismatched dir_e(%i) != dirText(%i)!\n", DIR_COUNT, dirTextCount);
    rc = NO;
  }
  if (FM_COUNT != fadeModeTextCount) {
    fprintf(stderr, "Programmer error: Mismatched fadeModes_e(%i) != fadeModeText(%i)!\n", FM_COUNT, fadeModeTextCount);
    rc = NO;
  }
  if (SM_COUNT != shiftTextCount) {
    fprintf(stderr, "Programmer error: Mismatched shiftModes_e(%i) != shiftText(%i)!\n", SM_COUNT, shiftTextCount);
    rc = NO;
  }
  if (TS_COUNT != textModeTextCount) {
    fprintf(stderr, "Programmer error: Mismatched textMode_e(%i) != textModeText(%i)!\n", TS_COUNT, textModeTextCount);
    rc = NO;
  }
  if (CM_COUNT != colorCycleTextCount) {
    fprintf(stderr, "Programmer error: Mismatched colorCycleModes_e(%i) != colorCycleText(%i)!\n", CM_COUNT, colorCycleTextCount);
    rc = NO;
  }
  if (CB_COUNT != crossBarTextCount) {
    fprintf(stderr, "Programmer error: Mismatched crossBar_e(%i) != crossbarText(%i)!\n", CB_COUNT, crossBarTextCount);
    rc = NO;
  }
  if (OO_COUNT != operateTextCount) {
    fprintf(stderr, "Programmer error: Mismatched operateOn_e(%i) != operateText(%i)!\n", OO_COUNT, operateTextCount);
    rc = NO;
  }
  return rc;
}

// Allocate the data memory for the pattern sets.
bool_t AllocatePatternData(int setCount) {
  int i, j;

  // Allocate data for each element based on type.  Allocated data will be an
  // array to that multiple pattern sets can be kept in memory at a time.
  for (i = 0; i < patternElementCount; i++) {
    switch (patternElements[i].type) {
      case ET_INT: case ET_ENUM:
        // patternData[i] is a void * pointing at memory allocated for 10 ints.
        patternData[i] = malloc(sizeof(int) * setCount);
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate int %i - %s\n", i, patternElements[i].name);
          return NO;
        } else {
          for(j = 0; j < setCount; j++) {
            SINT(j, i) = patternElements[i].initial.i;
          }
        }
        break;

      case ET_FLOAT:
        patternData[i] = malloc(sizeof(float) * setCount);
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate float %i - %s\n", i, patternElements[i].name);
          return NO;
        } else {
          for(j = 0; j < setCount; j++) {
            SFLOAT(j, i) = patternElements[i].initial.f;
          }
        }
        break;

      case ET_COLOR:
        patternData[i] = malloc(sizeof(color_t) * setCount);
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate color %i - %s\n", i, patternElements[i].name);
          return NO;
        } else {
          for(j = 0; j < setCount; j++) {
            SCOLOR(j, i) = patternElements[i].initial.c;
          }
        }
        break;

      case ET_BOOL:
        patternData[i] = malloc(sizeof(unsigned char) * setCount);
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate flag %i - %s\n", i, patternElements[i].name);
          return NO;
        } else {
          for(j = 0; j < setCount; j++) {
            SBOOL(j, i) = patternElements[i].initial.b;
          }
        }
        break;

      case ET_STRING:
        if (patternElements[i].size <= 0) {
          fprintf(stderr, "Invalid size for element %i (%s) of type string - %i\n", i, patternElements[i].name, patternElements[i].size);
          return NO;
        }
        patternData[i] = malloc(sizeof(char) * patternElements[i].size * setCount);
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate string %i - %s\n", i, patternElements[i].name);
          return NO;
        } else {
          for (j = 0; j < setCount; j++) {
            strncpy(SSTRING(j, i), patternElements[i].initial.s, patternElements[i].size);
            SSTRING(j, i)[patternElements[i].size - 1] = '\0';
          }
        }
        break;

      case ET_BUFFER:
        if (patternElements[i].size <= 0) {
          fprintf(stderr, "Invalid size for element %i (%s) of type string - %i\n", i, patternElements[i].name, patternElements[i].size);
          return NO;
        }
        // Allocate and intialize to 0.
        patternData[i] = calloc(patternElements[i].size * setCount, sizeof(unsigned char));
        if (!patternData[i]) {
          fprintf(stderr, "Unable to allocate buffer %i - %s\n", i, patternElements[i].name);
          return NO;
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
  //~ for (j = 0; j < setCount; j++) {
    //~ for (i = 0; i < PSET_SIZE; i++) {
      //~ switch(patternElements[i].type) {
        //~ case ET_INT:
          //~ printf("Element %i - %i (%s) of type int is %i\n", i, patternElements[i].index, patternElements[i].name, SINT(set, i,j));
          //~ break;
        //~ case ET_FLOAT:
          //~ printf("Element %i - %i (%s) of type float is %f\n", i, patternElements[i].index, patternElements[i].name, SFLOAT(set, i, j));
          //~ break;
        //~ case ET_COLOR:
          //~ printf("Element %i - %i (%s) of type color is (%i, %i, %i, %i)\n", i, patternElements[i].index, patternElements[i].name, SCOLOR(set, i, j).r, SCOLOR(set, i, j).g, SCOLOR(set, i, j).b, SCOLOR(set, i, j).a);
          //~ break;
        //~ case ET_ENUM:
          //~ printf("Element %i - %i (%s) of type enum is %i\n", i, patternElements[i].index, patternElements[i].name, SINT(set,  i, j));
          //~ break;
        //~ case ET_BOOL:
          //~ printf("Element %i - %i (%s) of type bool is %s\n", i, patternElements[i].index, patternElements[i].name, SBOOL(j, i) ? "YES" : "NO");
          //~ break;
        //~ case ET_STRING:
          //~ printf("Element %i - %i (%s) of type string is \"%s\"\n", i, patternElements[i].index, patternElements[i].name, SSTRING(set,  i, j));
          //~ break;
        //~ case ET_BUFFER:
          //~ printf("Element %i - %i (%s) of type buffer is %i bytes.\n", i, patternElements[i].index, patternElements[i].name, patternElements[i].size);
          //~ break;
        //~ default:
          //~ fprintf(stderr, "Error - Unknown data type on element %i - %i\n", i, patternElements[i].type);
          //~ exit(EXIT_FAILURE);
          //~ break;
      //~ }
    //~ }
  //~ }
  return YES;
}

