// Tensor pattern generator
// Structural elements, pattern data definitions, gui commands.
// Joshua Krueger
// tensor@core9.org
// 2014_12_04

#include <limits.h>
#include <SDL.h>

#include "elements.h"
//~ #include "draw.h"
#include "drv-tensor.h"
#include "tensor.h"
#include "layout.h"

// Fade modes
const char *fadeModeText[] = { "Limited", "Modular", "NonZero Limited", "NonZero Modular" };
const int fadeModeTextCount = sizeof(fadeModeText) / sizeof(const char *);

// One shot modes - The pattern elements using this were once booleans, so the
// exact spelling of "No" and "Yes" below are important for maintaining backward
// compatibility with old *.now files.
const char *oneShotText[] = { "No", "Yes", "OneShot" };
const int oneShotTextCount = sizeof(oneShotText) / sizeof(const char *);

// Color plane shift modes - same order as dir_e please.
const char *shiftText[] = { "Up", "Left", "Down", "Right", "Hold"};
const int shiftTextCount = sizeof(shiftText) / sizeof(const char *);

// Scroller directions.
const char *dirText[] = { "Up", "Left", "Down", "Right" };
const int dirTextCount = sizeof(dirText) / sizeof(const char *);

// Color cycling modes
const char *colorCycleText[] = { "None", "A-B Fade", "R-G-B", "R-G-B SM", "C-M-Y",
  "C-M-Y SM", "R-W-B", "R-W-B SM", "Gray Ramp", "Gry Rmp SM", "Gray Sym", "Gry Sym SM",
  "Secondary", "Tertiary", "Rainbow", "Random", "Tertiary-A"};
const int colorCycleTextCount = sizeof(colorCycleText) / sizeof(const char *);

// Text background modes
const char *textModeText[] = { "8 Row", "9 Row Stagger", "10 Row", "Full BG", "No BG", "Black" };
const int textModeTextCount = sizeof(textModeText) / sizeof(const char *);

// Crossbar seed types
const char *crossbarText[] = { "None", "Vertical", "Horizontal", "V & H"};
const int crossBarTextCount = sizeof(crossbarText) / sizeof(const char *);

// Pattern set that the controls operate on
const char *operateText[] = { "Live", "Alternate" };
const int operateTextCount = sizeof(operateText) / sizeof(const char *);

// List of seeds whose color can be altered.
const char *alterText[] = { "H Bars", "V Bars", "ColorAll A", "ColorAll B",
  "Small Snail", "Large Snail", "Cell Fun", "Sidebar", "Sidepulse", "Light pen",
  "Cross bars", "Random Dots", "Text"};
const int alterTextCount = sizeof(alterText) / sizeof(const char *);

// This palette order should line up with color_e enumeration values and colorsText.
const colorName_t namedColors[] = {
  { CE_SDKRED,       CD_SDKRED},
  { CE_DKRED,        CD_DKRED},
  { CE_MDRED,        CD_MDRED},
  { CE_RED,          CD_RED},
  { CE_MLRED,        CD_MLRED},
  { CE_LTRED,        CD_LTRED},

  { CE_SDKORANGE,    CD_SDKORANGE},
  { CE_DKORANGE,     CD_DKORANGE},
  { CE_MDORANGE,     CD_MDORANGE},
  { CE_ORANGE,       CD_ORANGE},
  { CE_MLORANGE,     CD_MLORANGE},
  { CE_LTORANGE,     CD_LTORANGE},

  { CE_SDKYELLOW,    CD_SDKYELLOW},
  { CE_DKYELLOW,     CD_DKYELLOW},
  { CE_MDYELLOW,     CD_MDYELLOW},
  { CE_YELLOW,       CD_YELLOW},
  { CE_MLYELLOW,     CD_MLYELLOW},
  { CE_LTYELLOW,     CD_LTYELLOW},

  { CE_SDKCHARTREUSE,CD_SDKCHARTREUSE},
  { CE_DKCHARTREUSE, CD_DKCHARTREUSE},
  { CE_MDCHARTREUSE, CD_MDCHARTREUSE},
  { CE_CHARTREUSE,   CD_CHARTREUSE},
  { CE_MLCHARTREUSE, CD_MLCHARTREUSE},
  { CE_LTCHARTREUSE, CD_LTCHARTREUSE},

  { CE_SDKGREEN,     CD_SDKGREEN},
  { CE_DKGREEN,      CD_DKGREEN},
  { CE_MDGREEN,      CD_MDGREEN},
  { CE_GREEN,        CD_GREEN},
  { CE_MLGREEN,      CD_MLGREEN},
  { CE_LTGREEN,      CD_LTGREEN},

  { CE_SDKAQUA,      CD_SDKAQUA},
  { CE_DKAQUA,       CD_DKAQUA},
  { CE_MDAQUA,       CD_MDAQUA},
  { CE_AQUA,         CD_AQUA},
  { CE_MLAQUA,       CD_MLAQUA},
  { CE_LTAQUA,       CD_LTAQUA},

  { CE_SDKCYAN,      CD_SDKCYAN},
  { CE_DKCYAN,       CD_DKCYAN},
  { CE_MDCYAN,       CD_MDCYAN},
  { CE_CYAN,         CD_CYAN},
  { CE_MLCYAN,       CD_MLCYAN},
  { CE_LTCYAN,       CD_LTCYAN},

  { CE_SDKAZURE,     CD_SDKAZURE},
  { CE_DKAZURE,      CD_DKAZURE},
  { CE_MDAZURE,      CD_MDAZURE},
  { CE_AZURE,        CD_AZURE},
  { CE_MLAZURE,      CD_MLAZURE},
  { CE_LTAZURE,      CD_LTAZURE},

  { CE_SDKBLUE,      CD_SDKBLUE},
  { CE_DKBLUE,       CD_DKBLUE},
  { CE_MDBLUE,       CD_MDBLUE},
  { CE_BLUE,         CD_BLUE},
  { CE_MLBLUE,       CD_MLBLUE},
  { CE_LTBLUE,       CD_LTBLUE},

  { CE_SDKVIOLET,    CD_SDKVIOLET},
  { CE_DKVIOLET,     CD_DKVIOLET},
  { CE_MDVIOLET,     CD_MDVIOLET},
  { CE_VIOLET,       CD_VIOLET},
  { CE_MLVIOLET,     CD_MLVIOLET},
  { CE_LTVIOLET,     CD_LTVIOLET},

  { CE_SDKMAGENTA,   CD_SDKMAGENTA},
  { CE_DKMAGENTA,    CD_DKMAGENTA},
  { CE_MDMAGENTA,    CD_MDMAGENTA},
  { CE_MAGENTA,      CD_MAGENTA},
  { CE_MLMAGENTA,    CD_MLMAGENTA},
  { CE_LTMAGENTA,    CD_LTMAGENTA},

  { CE_SDKROSE,      CD_SDKROSE},
  { CE_DKROSE,       CD_DKROSE},
  { CE_MDROSE,       CD_MDROSE},
  { CE_ROSE,         CD_ROSE},
  { CE_MLROSE,       CD_MLROSE},
  { CE_LTROSE,       CD_LTROSE},

  { CE_BLACK,        CD_BLACK},
  { CE_SDKGRAY,      CD_SDKGRAY},
  { CE_DKGRAY,       CD_DKGRAY},
  { CE_GRAY,         CD_GRAY},
  { CE_LTGRAY,       CD_LTGRAY},
  { CE_WHITE,        CD_WHITE},
};
const int namedColorsCount = sizeof(namedColors) / sizeof(colorName_t);


// Old and not really necessary anymore now that we have a graphical color selector:
//~ // This is separated from the namedColors because of the enumerations array.
//~ const char *colorsText[] = { "red", "orange", "yellow", "chartreuse", "green",
  //~ "aqua", "cyan", "azure", "blue", "violet", "magenta", "rose", "white",
  //~ "lt gray", "gray", "dk gray", "black", "1/2 red", "1/2 orange", "1/2 yellow",
  //~ "1/2 chartreuse", "1/2 green", "1/2 aqua", "1/2 cyan", "1/2 azure",
  //~ "1/2 blue", "1/2 violet", "1/2 magenta", "1/2 rose", "lt red", "lt orange",
  //~ "lt yellow", "lt chartreuse", "lt green", "lt aqua", "lt cyan", "lt azure",
  //~ "lt blue", "lt violet", "lt magenta", "lt rose",
//~ };
//~ const int colorsTextSize = sizeof(colorsText) / sizeof(const char *);

// Enumeration array provides the connection between the enumeration type, its
// list of possible text values, and the size of the list.
const enums_t enumerations[] = {
  { E_DIRECTIONS, dirText, DIR_COUNT},
  { E_FADEMODES, fadeModeText, FM_COUNT},
  { E_SHIFTMODES, shiftText, SM_COUNT},
  { E_TEXTMODES, textModeText, TS_COUNT},
  { E_COLORCYCLES, colorCycleText, CM_COUNT},
  //~ { E_COLORS, colorsText, CE_COUNT},
  { E_OPERATE, operateText, OO_COUNT },
  { E_CROSSBAR, crossbarText, CB_COUNT },
  { E_ALTERPALETTES, alterText, A_COUNT },
  { E_ONESHOT, oneShotText, OS_COUNT },
};
const int enumerationsCount = sizeof(enumerations) / sizeof(enums_t);

// Palette alteration provides the connection between whose palette we are
// altering and which pattern elements are involved.  PE_INVALID disables a
// specific palette item for a seed that doesn't use it.
// Order here is important. seedPalette array index must correspond to
// alterPalettes_e enumeration.  This is enforced by VerifyStructuralIntegrity.
// The rest of the items are patternElement_e (patternElement) items.
// The order or items from paletteConnect_t:
// fgColorA, fgColorB, fgCycleMode, fgCycleRate, fgCyclePos, fgAlpha,
// bgColorA, bgColorB, bgCycleMode, bgCycleRate, bgCyclePos, bgAlpha
// Most things that have color also have alpha, 2nd color, and cycle mode / rate.
// Not all seeds have a background color, which PE_INVALIDs below disable.
const paletteConnect_t seedPalette[] = {
  {A_HBARS, PE_HBARS_COLA, PE_HBARS_COLB, PE_HBARS_CM, PE_HBARS_CMR, PE_HBARS_CMPOS, PE_HBARS_ALPHA,
            PE_HBARS_BGCOLA, PE_HBARS_BGCOLB, PE_HBARS_BGCM, PE_HBARS_BGCMR, PE_HBARS_BGCMPOS, PE_HBARS_BGALPHA},
  {A_VBARS, PE_VBARS_COLA, PE_VBARS_COLB, PE_VBARS_CM, PE_VBARS_CMR, PE_VBARS_CMPOS, PE_VBARS_ALPHA,
            PE_VBARS_BGCOLA, PE_VBARS_BGCOLB, PE_VBARS_BGCM, PE_VBARS_BGCMR, PE_VBARS_BGCMPOS, PE_VBARS_BGALPHA},
  {A_COLORALLA, PE_CAA_COLA, PE_CAA_COLB, PE_CAA_CM, PE_CAA_CMR, PE_CAA_CMPOS, PE_CAA_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_COLORALLB, PE_CAB_COLA, PE_CAB_COLB, PE_CAB_CM, PE_CAB_CMR, PE_CAB_CMPOS, PE_CAB_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_SMALLSNAIL, PE_SNAIL_COLA, PE_SNAIL_COLB, PE_SNAIL_CM, PE_SNAIL_CMR, PE_SNAIL_CMPOS, PE_SNAIL_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_LARGESNAIL, PE_FSNAIL_COLA, PE_FSNAIL_COLB, PE_FSNAIL_CM, PE_FSNAIL_CMR, PE_FSNAIL_CMPOS, PE_FSNAIL_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_CELL, PE_CF_COLA, PE_CF_COLB, PE_CF_CM, PE_CF_CMR, PE_CF_CMPOS, PE_CF_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_SIDEBAR, PE_BS_COLA, PE_BS_COLB, PE_BS_CM, PE_BS_CMR, PE_BS_CMPOS, PE_BS_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_SIDEPULSE, PE_SP_COLA, PE_SP_COLB, PE_SP_CM, PE_SP_CMR, PE_SP_CMPOS, PE_SP_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_LIGHTPEN, PE_LP_COLA, PE_LP_COLB, PE_LP_CM, PE_LP_CMR, PE_LP_CMPOS, PE_LP_ALPHA,
            PE_LP_BGCOLA, PE_LP_BGCOLB, PE_LP_BGCM, PE_LP_BGCMR, PE_LP_BGCMPOS, PE_LP_BGALPHA},
  {A_CROSSBARS, PE_CB_COLA, PE_CB_COLB, PE_CB_CM, PE_CB_CMR, PE_CB_CMPOS, PE_CB_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_RDOT, PE_RDOT_COLA, PE_RDOT_COLB, PE_RDOT_CM, PE_RDOT_CMR, PE_RDOT_CMPOS, PE_RDOT_ALPHA,
            PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID, PE_INVALID},
  {A_TEXT, PE_TS_COLA, PE_TS_COLB, PE_TS_CM, PE_TS_CMR, PE_TS_CMPOS, PE_TS_ALPHA,
            PE_TS_BGCOLA, PE_TS_BGCOLB, PE_TS_BGCM, PE_TS_BGCMR, PE_TS_BGCMPOS, PE_TS_BGALPHA},
};
const int seedPaletteCount = sizeof(seedPalette) / sizeof(paletteConnect_t);

// The pattern elements used by the engine.  This is currently not a constant
// due to the inclusion of the dynamically allocated dataset in the type.
// ints, floats have min & maxes.  enums have etypes.  buffers and string have sizes.
const patternElement_t patternElements[] = {
//  Access enum,      "Name",        TYPE,      initial,     min,      max,      size,     etype
  { PE_CELLFUN,       "CellFun",       ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_CF_COLA,       "CFColorA",      ET_COLOR,  {.c = CD_AQUA} },
  { PE_CF_COLB,       "CFColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_CF_CM,         "CFCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_CF_CMR,        "CFCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_CF_CMPOS,      "CFCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_CF_ALPHA,      "CFAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_BOUNCER,       "CycleScroll",   ET_INT,    {.i = 0}, {.i = 0}, {.i = INT_MAX} },
  { PE_FADE,          "Fader",         ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_DIFFUSE,       "Diffuse",       ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_ROLLOVER,      "RollOver",      ET_BOOL,   {.b = NO} },
  { PE_SCROLL,        "Scroll",        ET_ENUM,   {.e = OS_YES}, .etype = E_ONESHOT },
  { PE_HBARS,         "Hbars",         ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_HBARS_COLA,    "HbarsColorA",   ET_COLOR,  {.c = CD_RED} },
  { PE_HBARS_COLB,    "HbarsColorB",   ET_COLOR,  {.c = CD_BLACK} },
  { PE_HBARS_CM,      "HbarsCMode",    ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_HBARS_CMR,     "HbarsCMRate",   ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_HBARS_CMPOS,   "HbarsCMPos",    ET_INT,    {.i = 0} },  // Not user
  { PE_HBARS_ALPHA,   "HbarsAlpha",    ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_HBARS_BGCOLA,  "HbarsBGColorA", ET_COLOR,  {.c = CD_BLUE} },
  { PE_HBARS_BGCOLB,  "HbarsBGColorB", ET_COLOR,  {.c = CD_BLACK} },
  { PE_HBARS_BGCM,    "HbarsBGCMode",  ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_HBARS_BGCMR,   "HbarsBGCMRate", ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_HBARS_BGCMPOS, "HbarsBGCMPos",  ET_INT,    {.i = 0} },  // Not user
  { PE_HBARS_BGALPHA, "HbarsBGAlpha",  ET_FLOAT,  {.f = 0}, {.f = 0}, {.f = 1} },
  { PE_VBARS,         "Vbars",         ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_VBARS_COLA,    "VbarsColorA",   ET_COLOR,  {.c = CD_ORANGE} },
  { PE_VBARS_COLB,    "VbarsColorB",   ET_COLOR,  {.c = CD_BLACK} },
  { PE_VBARS_CM,      "VbarsCMode",    ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_VBARS_CMR,     "VbarsCMRate",   ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_VBARS_CMPOS,   "VbarsCMPos",    ET_INT,    {.i = 0} },  // Not user
  { PE_VBARS_ALPHA,   "VbarsAlpha",    ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_VBARS_BGCOLA,  "VbarsBGColorA", ET_COLOR,  {.c = CD_AQUA} },
  { PE_VBARS_BGCOLB,  "VbarsBGColorB", ET_COLOR,  {.c = CD_BLACK} },
  { PE_VBARS_BGCM,    "VbarsBGCMode",  ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_VBARS_BGCMR,   "VbarsBGCMRate", ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_VBARS_BGCMPOS, "VbarsBGCMPos",  ET_INT,    {.i = 0} },  // Not user
  { PE_VBARS_BGALPHA, "VbarsBGAlpha",  ET_FLOAT,  {.f = 0}, {.f = 0}, {.f = 1} },
  { PE_CAA,           "ColorAllA",     ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_CAA_COLA,      "CAAColorA",     ET_COLOR,  {.c = CD_YELLOW} },
  { PE_CAA_COLB,      "CAAColorB",     ET_COLOR,  {.c = CD_BLACK} },
  { PE_CAA_CM,        "CAACMode",      ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_CAA_CMR,       "CAACMRate",     ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_CAA_CMPOS,     "CAACMPos",      ET_INT,    {.i = 0} },  // Not user
  { PE_CAA_ALPHA,     "CAAAlpha",      ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_CAB,           "ColorAllB",     ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_CAB_COLA,      "CABColorA",     ET_COLOR,  {.c = CD_BLACK} },
  { PE_CAB_COLB,      "CABColorB",     ET_COLOR,  {.c = CD_RED} },
  { PE_CAB_CM,        "CABCMode",      ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_CAB_CMR,       "CABCMRate",     ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_CAB_CMPOS,     "CABCMPos",      ET_INT,    {.i = 0} },  // Not user
  { PE_CAB_ALPHA,     "CABAlpha",      ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_RDOT,          "RandomDots",    ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_RDOT_COLA,     "RDOTColorA",    ET_COLOR,  {.c = CD_MAGENTA} },
  { PE_RDOT_COLB,     "RDOTColorB",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_RDOT_CM,       "RDOTCMode",     ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_RDOT_CMR,      "RDOTCMRate",    ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_RDOT_CMPOS,    "RDOTCMPos",     ET_INT,    {.i = 0} },  // Not user
  { PE_RDOT_ALPHA,    "RDOTAlpha",     ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_FADEMODE,      "FadeMode",      ET_ENUM,   {.e = FM_LIMIT}, .etype = E_FADEMODES },
  { PE_POSTRZ,        "PostRotZoom",   ET_BOOL,   {.b = NO} },
  { PE_PRERZ,         "PreRotZoom",    ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_ALIAS,         "AntiAlias",     ET_BOOL,   {.b = NO} },
  { PE_MULTIPLY,      "Multiply",      ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_BARSEED,       "SideBar",       ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_BS_COLA,       "SBColorA",      ET_COLOR,  {.c = CD_CYAN} },
  { PE_BS_COLB,       "SBColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_BS_CM,         "SBCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_BS_CMR,        "SBCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_BS_CMPOS,      "SBCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_BS_ALPHA,      "SBAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_NORED,         "NoRed",         ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_NOGREEN,       "NoGreen",       ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_NOBLUE,        "NoBlue",        ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_SHIFTRED,      "ShiftRed",      ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTGREEN,    "ShiftGreen",    ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTBLUE,     "ShiftBlue",     ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTCYAN,     "ShiftCyan",     ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTYELLOW,   "ShiftYellow",   ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_SHIFTMAGENTA,  "ShiftMagenta",  ET_ENUM,   {.e = SM_HOLD}, .etype = E_SHIFTMODES },
  { PE_POSTIMAGE,     "PostImage",     ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_FADEINC,       "FadeIncr",      ET_INT,    {.i = INITIAL_FADE_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_DIFFUSECOEF,   "DiffuseCoef",   ET_FLOAT,  {.f = INITIAL_DIFF_COEF}, {.f = -1000}, {.f = 1000} },
  { PE_SCROLLDIR,     "ScrollDir",     ET_ENUM,   {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_RDOTCOEF,      "DotCoef",       ET_INT,    {.i = INITIAL_RAND_MOD}, {.i = 1}, {.i = 20000} },
  { PE_POSTEXP,       "Expansion",     ET_FLOAT,  {.f = INITIAL_EXPAND}, {.f = -20}, {.f = 20}  },
  { PE_FLOATINC,      "FloatInc",      ET_FLOAT,  {.f = INITIAL_FLOAT_INC}, {.f = 0.00001}, {.f = 1000000000} },
  { PE_POSTRZANGLE,   "PostRotAngle",  ET_FLOAT,  {.f = INITIAL_POSTROT_ANGLE}, {.f = -999999999}, {.f = 1000000000 } },
  { PE_POSTRZINC,     "PostRotInc",    ET_FLOAT,  {.f = INITIAL_POSTROT_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_MULTIPLYBY,    "MultiplyBy",    ET_FLOAT,  {.f = INITIAL_MULTIPLIER}, {.f = -999999999 }, {.f = 1000000000} },
  { PE_DELAY,         "Delay",         ET_INT,    {.i = INITIAL_DELAY}, {.i = 1}, {.i = INT_MAX}},
  { PE_CYCLESAVEFG,   "FGCyclePos",    ET_INT,    {.i = 0} }, // Not user
  { PE_CYCLESAVEBG,   "BGCyclePos",    ET_INT,    {.i = 0} }, // Not user
  { PE_CELLFUNCOUNT,  "CellFunCt",     ET_INT,    {.i = 0} }, // Not user
  { PE_TEXTBUFFER,    "TextBuffer",    ET_STRING, {.s = INITIAL_TEXT}, .size = TEXT_BUFFER_SIZE },
  { PE_TEXTMODE,      "TextMode",      ET_ENUM,   {.e = TS_FULLBG}, .etype = E_TEXTMODES },
  { PE_FONTFLIP,      "FontFlip",      ET_BOOL,   {.b = NO} },
  { PE_FONTDIR,       "FontDir",       ET_BOOL,   {.b = FORWARDS} },
  { PE_TEXTOFFSET,    "TextOffset",    ET_INT,    {.i = TENSOR_HEIGHT / 3 - 1}, {.i = 0}, {.i = TENSOR_WIDTH} },
  { PE_TEXTINDEX,     "TextIndex",     ET_INT,    {.i = sizeof(INITIAL_TEXT) - 1} }, // Not user
  { PE_PIXELINDEX,    "PixelIndex",    ET_INT,    {.i = INVALID} }, // Not user
  { PE_SCROLLDIRLAST, "ScrollDirLast", ET_ENUM,   {.e = INITIAL_DIR}, .etype = E_DIRECTIONS },
  { PE_TEXTSEED,      "TextSeed",      ET_BOOL,   {.b = YES} },
  { PE_TS_COLA,       "TSColorA",      ET_COLOR,  {.c = CD_ROSE} },
  { PE_TS_COLB,       "TSColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_TS_CM,         "TSCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_TS_CMR,        "TSCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_TS_CMPOS,      "TSCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_TS_ALPHA,      "TSAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_TS_BGCOLA,     "TSBGColorA",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_TS_BGCOLB,     "TSBGColorB",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_TS_BGCM,       "TSBGCMode",     ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_TS_BGCMR,      "TSBGCMRate",    ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_TS_BGCMPOS,    "TSBGCMPos",     ET_INT,    {.i = 0} },  // Not user
  { PE_TS_BGALPHA,    "TSBGAlpha",     ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_FRAMEBUFFER,   "FrameBuffer",   ET_BUFFER, .size = TENSOR_BYTES },
  { PE_PRERZANGLE,    "PreRotAngle",   ET_FLOAT,  {.f = INITIAL_PREROT_ANGLE}, {.f = -999999999}, {.f = 1000000000} },
  { PE_PRERZALIAS,    "PreRotAlias",   ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_PRERZINC,      "PreRotInc",     ET_FLOAT,  {.f = INITIAL_POSTROT_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_PRERZEXPAND,   "PreRotExp",     ET_FLOAT,  {.f = INITIAL_PREEXPAND}, {.f = -20}, {.f = 20} },
  { PE_FRAMECOUNT,    "CycleFCount",   ET_INT,    {.i = INITIAL_FRAMECYCLECOUNT}, {.i = 1}, {.i = INT_MAX} },
  { PE_IMAGEANGLE,    "ImageAngle",    ET_FLOAT,  {.f = INITIAL_IMAGE_ANGLE}, {.f = -999999999}, {.f = 1000000000} },
  { PE_IMAGEALIAS,    "ImageAlias",    ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_IMAGEINC,      "ImageInc",      ET_FLOAT,  {.f = INITIAL_IMAGE_INC}, {.f = -999999999}, {.f = 1000000000} },
  { PE_IMAGEEXP,      "ImageExp",      ET_FLOAT,  {.f = INITIAL_IMAGE_EXPAND}, {.f = -20}, {.f = 20} },
  { PE_IMAGEXOFFSET,  "ImageXOffset",  ET_FLOAT,  {.f = INITIAL_IMAGE_XOFFSET}, {.f = 0}, {.f = 1} },
  { PE_IMAGEYOFFSET,  "ImageYOffset",  ET_FLOAT,  {.f = INITIAL_IMAGE_YOFFSET}, {.f = 0}, {.f = 1} },
  { PE_IMAGENAME,     "ImageName",     ET_STRING, {.s = INITIAL_IMAGE}, .size = 1024},
  { PE_SNAIL,         "SnailShot",     ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_SNAIL_COLA,    "SnailColorA",   ET_COLOR,  {.c = CD_CHARTREUSE} },
  { PE_SNAIL_COLB,    "SnailColorB",   ET_COLOR,  {.c = CD_BLACK} },
  { PE_SNAIL_CM,      "SnailCMode",    ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_SNAIL_CMR,     "SnailCMRate",   ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_SNAIL_CMPOS,   "SnailCMPos",    ET_INT,    {.i = 0} },  // Not user
  { PE_SNAIL_ALPHA,   "SnailAlpha",    ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_SNAIL_POS,     "SnailPos",      ET_INT,    {.i = 0} }, // Not user
  { PE_FSNAIL,        "FastSnail",     ET_ENUM,   {.e = OS_NO}, .etype = E_ONESHOT },
  { PE_FSNAIL_COLA,   "FSNAILColorA",  ET_COLOR,  {.c = CD_GREEN} },
  { PE_FSNAIL_COLB,   "FSNAILColorB",  ET_COLOR,  {.c = CD_BLACK} },
  { PE_FSNAIL_CM,     "FSNAILCMode",   ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_FSNAIL_CMR,    "FSNAILCMRate",  ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_FSNAIL_CMPOS,  "FSNAILCMPos",   ET_INT,    {.i = 0} },  // Not user
  { PE_FSNAIL_ALPHA,  "FSNAILAlpha",   ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_FSNAILP,       "FastSnailP",    ET_INT,    {.i = 0} }, // Not user
  { PE_CROSSBAR,      "Crossbar",      ET_ENUM,   {.e = CB_NONE}, .etype = E_CROSSBAR },
  { PE_CB_COLA,       "CBColorA",      ET_COLOR,  {.c = CD_VIOLET} },
  { PE_CB_COLB,       "CBColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_CB_CM,         "CBCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_CB_CMR,        "CBCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_CB_CMPOS,      "CBCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_CB_ALPHA,      "CBAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_CBLIKELY,      "CrossLikely",   ET_INT,    {.i = 1002 }, {.i = 1}, {.i = 20000} },
  { PE_MIRROR_V,      "MirrorV",       ET_ENUM,   {.e = OS_NO }, .etype = E_ONESHOT },
  { PE_MIRROR_H,      "MirrorH",       ET_ENUM,   {.e = OS_NO }, .etype = E_ONESHOT },
  { PE_SCROLLRANDEN,  "ScrollRandE",   ET_BOOL,   {.b = NO } },
  { PE_SCROLLRANDOM,  "ScrollRand",    ET_INT,    {.i = 50 }, {.i = 1}, {.i = INT_MAX} },
  { PE_SIDEPULSE,     "SidePulse",     ET_ENUM,   {.e = OS_NO }, .etype = E_ONESHOT },
  { PE_SP_COLA,       "SPColorA",      ET_COLOR,  {.c = CD_AZURE} },
  { PE_SP_COLB,       "SPColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_SP_CM,         "SPCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_SP_CMR,        "SPCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_SP_CMPOS,      "SPCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_SP_ALPHA,      "SPAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_INTENSITY,     "Intensity",     ET_FLOAT,  {.f = 1.0}, {.f = 0}, {.f = 1} },
  { PE_SCROLLALPHA,   "ScrollAlpha",   ET_FLOAT,  {.f = 1.0}, {.f = 0}, {.f = 1.0} },
  { PE_IMAGEALPHA,    "ImageAlpha",    ET_FLOAT,  {.f = 1.0}, {.f = 0}, {.f = 1.0} },
  { PE_PALETTEALTER,  "PaletteSelct",  ET_ENUM,   {.e = A_HBARS}, .etype = E_ALTERPALETTES },
  { PE_LP_COLA,       "LPColorA",      ET_COLOR,  {.c = CD_BLUE} },
  { PE_LP_COLB,       "LPColorB",      ET_COLOR,  {.c = CD_BLACK} },
  { PE_LP_CM,         "LPCMode",       ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_LP_CMR,        "LPCMRate",      ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_LP_CMPOS,      "LPCMPos",       ET_INT,    {.i = 0} },  // Not user
  { PE_LP_ALPHA,      "LPAlpha",       ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_LP_BGCOLA,     "LPBGColorA",    ET_COLOR,  {.c = CD_RED} },
  { PE_LP_BGCOLB,     "LPBGColorB",    ET_COLOR,  {.c = CD_BLACK} },
  { PE_LP_BGCM,       "LPBGCMode",     ET_ENUM,   {.e = CM_NONE}, .etype = E_COLORCYCLES },
  { PE_LP_BGCMR,      "LPBGCMRate",    ET_INT,    {.i = INITIAL_RAINBOW_INC}, {.i = INT_MIN}, {.i = INT_MAX} },
  { PE_LP_BGCMPOS,    "LPBGCMPos",     ET_INT,    {.i = 0} },  // Not user
  { PE_LP_BGALPHA,    "LPBGAlpha",     ET_FLOAT,  {.f = 1}, {.f = 0}, {.f = 1} },
  { PE_PAUSE,         "Pause",         ET_BOOL,   {.b = NO } },
};
#define PATTERN_ELEMENT_COUNT ( sizeof(patternElements) / sizeof(patternElement_t) )
const int patternElementCount = PATTERN_ELEMENT_COUNT;

// Clickable commands displayed on the gui.
const command_t displayCommand[] = {
  // Misc seeds
  {ROW_O + 1, COL_O, "Horizontal bars",  PE_HBARS,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_i, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_HBARS},
  {ROW_O + 2, COL_O, "Vertical bars",    PE_VBARS,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_o, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_VBARS},
  {ROW_O + 3, COL_O, "Color all A",      PE_CAA,      {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_p, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_COLORALLA},
  {ROW_O + 4, COL_O, "Color all B",      PE_CAB,      {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_a, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_COLORALLB},
  {ROW_O + 5, COL_O, "Small snail seed", PE_SNAIL,    {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_SMALLSNAIL},
  {ROW_O + 6, COL_O, "Large snail seed", PE_FSNAIL,   {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_LARGESNAIL},
  {ROW_O + 7, COL_O, "Cell pattern",    PE_CELLFUN,  {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_q, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_CELL},
  {ROW_O + 8, COL_O, "Sidebar seed",    PE_BARSEED,  {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_n, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_SIDEBAR},
  {ROW_O + 9, COL_O, "Sidepulse seed",  PE_SIDEPULSE,{{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_SIDEPULSE},
  {ROW_O + 10, COL_O, "Light pen",       PE_INVALID,  {}, A_LIGHTPEN},

  // Crossbar seeds
  {ROW_CB + 1, COL_CB, "Enable",      PE_CROSSBAR, {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_CROSSBARS},
  {ROW_CB + 2, COL_CB, "Randomness",  PE_CBLIKELY, {{}, {0, 0, COM_INT_RST}, {0, 0, COM_LINT_INC}, {0, 0, COM_LINT_DEC}}, A_INVALID},

  // Random Dots
  {ROW_R + 1, COL_R, "Enable",     PE_RDOT,     {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_s, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_RDOT},
  {ROW_R + 2, COL_R, "Randomness", PE_RDOTCOEF, {{}, {KMOD_ALT, SDLK_LEFTBRACKET, COM_INT_RST}, {KMOD_ALT, SDLK_p, COM_LINT_INC}, {KMOD_ALT, SDLK_RIGHTBRACKET, COM_LINT_DEC}}, A_INVALID},

    // Text
  {ROW_T + 1, COL_T, "Text seed",          PE_TEXTSEED,   {{KMOD_CTRL, SDLK_t, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_TEXT},
  {ROW_T + 3, COL_T, "Delete last letter", PE_INVALID,    {{KMOD_NONE, SDLK_BACKSPACE, COM_BACKSPACE}}, A_INVALID},
  {ROW_T + 4, COL_T, "Erase all text",     PE_INVALID,    {{KMOD_NONE, SDLK_DELETE, COM_DELETE}}, A_INVALID},
  {ROW_T + 5, COL_T, "Reverse text",       PE_FONTDIR,    {{KMOD_CTRL, SDLK_QUOTE, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_T + 6, COL_T, "Flip text.",         PE_FONTFLIP,   {{KMOD_CTRL, SDLK_SEMICOLON, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_T + 7, COL_T, "Background mode",    PE_TEXTMODE,   {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_b, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_T + 8, COL_T, "Edge Offset",        PE_TEXTOFFSET, {{}, {0, 0, COM_TEXTO_RST}, {KMOD_ALT | KMOD_CTRL, SDLK_z, COM_TEXTO_INC}, {KMOD_ALT | KMOD_CTRL, SDLK_x, COM_TEXTO_DEC}}, A_INVALID},
  {ROW_T + 9, COL_T, "Restart text",       PE_INVALID,    {{KMOD_CTRL, SDLK_j, COM_TEXTRESET}, {0, 0, COM_TEXTRESET}}, A_INVALID},

  // Pattern sets
  {ROW_PA + 1, COL_PA, "Cycle through pattern sets",       PE_INVALID,    {{KMOD_CTRL, SDLK_g, COM_CYCLESET}, {0, 0, COM_CYCLESET}}, A_INVALID},
  {ROW_PA + 2, COL_PA, "Frames to play this set",          PE_FRAMECOUNT, {{}, {0, 0, COM_FCOUNT_RST}, {0, 0, COM_FCOUNT_INC}, {0, 0, COM_FCOUNT_DEC}}, A_INVALID},
  {ROW_PA + 8, COL_PA, "Live preview set (left)",          PE_INVALID,    {{0, 0, COM_LIVE_INC}, {0, 0, COM_LIVE_DEC}, {0, 0, COM_LIVE_INC}, {0, 0, COM_LIVE_DEC}}, A_INVALID},
  {ROW_PA + 9, COL_PA, "Alternate preview set (right)",    PE_INVALID,    {{0, 0, COM_ALTERNATE_INC}, {0, 0, COM_ALTERNATE_DEC}, {0, 0, COM_ALTERNATE_INC}, {0, 0, COM_ALTERNATE_DEC}}, A_INVALID},
  {ROW_PA + 10,COL_PA, "Controls operate on set",          PE_INVALID,    {{0, 0, COM_OPERATE}, {0, 0, COM_OPERATE_RST}}, A_INVALID},
  {ROW_PA + 11,COL_PA, "Exchange live and alternate sets", PE_INVALID,    {{0, 0, COM_EXCHANGE}, {0, 0, COM_EXCHANGE}}, A_INVALID},
  {ROW_PA + 12,COL_PA, "Alternate set blending amount",    PE_INVALID,    {{}, {0, 0, COM_BLEND_RST}, {0, 0, COM_BLEND_INC}, {0, 0, COM_BLEND_DEC}}, A_INVALID},
  {ROW_PA + 13,COL_PA, "Auto blend & switch sets",         PE_INVALID,    {{0, 0, COM_BLENDSWITCH}, {0, 0, COM_BLENDSWITCH_RST}}, A_INVALID},
  {ROW_PA + 14,COL_PA, "Auto blend rate",                  PE_INVALID,    {{}, {0, 0, COM_BLENDINC_RST}, {0, 0, COM_BLENDINC_INC}, {0, 0, COM_BLENDINC_DEC}}, A_INVALID},

  // Averager / Diffusion
  {ROW_D + 1, COL_D, "Enable",      PE_DIFFUSE,     {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_r, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_D + 2, COL_D, "Coefficient", PE_DIFFUSECOEF, {{}, {KMOD_ALT, SDLK_w, COM_RST_FLOAT}, {KMOD_ALT, SDLK_q, COM_INC_FLOAT}, {KMOD_ALT, SDLK_e, COM_DEC_FLOAT}}, A_INVALID},

  // Fader
  {ROW_F + 1, COL_F, "Enable", PE_FADE,     {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_e, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_F + 2, COL_F, "Amount", PE_FADEINC,  {{}, {KMOD_ALT,  SDLK_x, COM_INT_RST}, {KMOD_ALT, SDLK_z, COM_INT_INC}, {KMOD_ALT, SDLK_c, COM_INT_DEC}}, A_INVALID},
  {ROW_F + 3, COL_F, "Mode",   PE_FADEMODE, {{}, {KMOD_CTRL, SDLK_h, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},

  // Multiplier
  {ROW_M + 1, COL_M, "Enable",      PE_MULTIPLY,   {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_c, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_M + 2, COL_M, "Multiply by", PE_MULTIPLYBY, {{}, {KMOD_ALT, SDLK_i, COM_RST_FLOAT}, {KMOD_ALT, SDLK_u, COM_INC_FLOAT}, {KMOD_ALT, SDLK_o, COM_DEC_FLOAT}}, A_INVALID},

  // Colors
  {ROW_C + 1, COL_C, "Alter palette of", PE_PALETTEALTER, {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_C + 2, COL_C, "FG Color A",       PE_FGCOLORA,     {}, A_INVALID},
  {ROW_C + 3, COL_C, "FG Color B",       PE_FGCOLORB,     {}, A_INVALID},
  {ROW_C + 4, COL_C, "FG Cycle mode",    PE_FGCYCLE,      {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_C + 5, COL_C, "FG Cycle rate",    PE_FGCYCLERATE,  {{}, {0, 0, COM_INT_RST}, {0, 0, COM_INT_INC}, {0, 0, COM_INT_DEC}}, A_INVALID},
  {ROW_C + 6, COL_C, "FG Opacity",       PE_FGALPHA,      {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_C + 7, COL_C, "BG Color A",       PE_BGCOLORA,     {}, A_INVALID},
  {ROW_C + 8, COL_C, "BG Color B",       PE_BGCOLORB,     {}, A_INVALID},
  {ROW_C + 9, COL_C, "BG Cycle mode",    PE_BGCYCLE,      {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_C + 10, COL_C, "BG Cycle rate",   PE_BGCYCLERATE,  {{}, {0, 0, COM_INT_RST}, {0, 0, COM_INT_INC}, {0, 0, COM_INT_DEC}}, A_INVALID},
  {ROW_C + 11, COL_C, "BG Opacity",      PE_BGALPHA,      {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},

  // Plane suppression
  {ROW_P + 1, COL_P, "Supress red",     PE_NORED,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_m, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_P + 2, COL_P, "Supress green",   PE_NOGREEN,  {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_COMMA, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_P + 3, COL_P, "Supress blue",    PE_NOBLUE,   {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_PERIOD, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},

  // Scrollers
  {ROW_S + 1, COL_S, "Scroller",       PE_SCROLL,       {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_u, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 2, COL_S, "Direction (<a>, <c>) <arrows>", PE_SCROLLDIR, {{}, {}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 3, COL_S, "Direction randomizer", PE_SCROLLRANDEN,  {{0, 0, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_S + 4, COL_S, "Direction randomness", PE_SCROLLRANDOM, {{}, {0, 0, COM_INT_RST}, {0, 0, COM_INT_INC}, {0, 0, COM_INT_DEC}}, A_INVALID},
  {ROW_S + 5, COL_S, "Cycle Scroller (0 = Off)",     PE_BOUNCER,  {{}, {0, 0, COM_INT_RST}, {0, 0, COM_INT_INC}, {0, 0, COM_INT_DEC}}, A_INVALID},
  {ROW_S + 6, COL_S, "Red plane",      PE_SHIFTRED,     {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_LEFTBRACKET, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 7, COL_S, "Green plane",    PE_SHIFTGREEN,   {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_RIGHTBRACKET, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 8, COL_S, "Blue plane",     PE_SHIFTBLUE,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_BACKSLASH, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 9, COL_S, "Cyan plane",     PE_SHIFTCYAN,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_LEFTBRACKET, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 10, COL_S, "Yellow plane",  PE_SHIFTYELLOW,  {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_RIGHTBRACKET, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 11, COL_S, "Magenta plane", PE_SHIFTMAGENTA, {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_BACKSLASH, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_S + 12, COL_S, "Toroidal",      PE_ROLLOVER, {{KMOD_CTRL, SDLK_y, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_S + 13, COL_S, "Scroll opacity", PE_SCROLLALPHA, {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},

  // Mirrors
  {ROW_MIR + 1, COL_MIR, "Vertical Mirror",   PE_MIRROR_V, {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_MIR + 2, COL_MIR, "Horizontal Mirror", PE_MIRROR_H, {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},

  // Post rotozoom
  {ROW_PR + 1, COL_PR, "Enable",     PE_POSTRZ,      {{KMOD_CTRL, SDLK_k, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_PR + 2, COL_PR, "Angle",      PE_POSTRZANGLE, {{}, {KMOD_ALT, SDLK_b, COM_RST_FLOAT}, {KMOD_ALT, SDLK_v, COM_INC_FLOAT}, {KMOD_ALT, SDLK_n, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PR + 3, COL_PR, "Angle Inc",  PE_POSTRZINC,   {{}, {KMOD_ALT, SDLK_k, COM_RST_FLOAT}, {KMOD_ALT, SDLK_j, COM_INC_FLOAT}, {KMOD_ALT, SDLK_l, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PR + 4, COL_PR, "Expansion",  PE_POSTEXP,     {{}, {KMOD_ALT, SDLK_s, COM_RST_FLOAT}, {KMOD_ALT, SDLK_a, COM_INC_FLOAT}, {KMOD_ALT, SDLK_d, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PR + 5, COL_PR, "Anti-alias", PE_ALIAS,       {{KMOD_CTRL, SDLK_x, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},

  // Pre rotozoom
  {ROW_PE + 1, COL_PE, "Enable",    PE_PRERZ,      {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL, SDLK_z, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_PE + 2, COL_PE, "Angle",     PE_PRERZANGLE, {{}, {KMOD_ALT, SDLK_t, COM_RST_FLOAT}, {KMOD_ALT, SDLK_r, COM_INC_FLOAT}, {KMOD_ALT, SDLK_y, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PE + 3, COL_PE, "Angle Inc", PE_PRERZINC,   {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PE + 4, COL_PE, "Expansion", PE_PRERZEXPAND,{{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_PE + 5, COL_PE, "Anti-alias",PE_PRERZALIAS, {{}, {0, 0, COM_ENUM_RST}, {0, 0, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},

  // Image stuff
  {ROW_I + 1, COL_I, "Enable",        PE_POSTIMAGE,    {{}, {0, 0, COM_ENUM_RST}, {KMOD_CTRL | KMOD_ALT, SDLK_p, COM_ENUM_INC}, {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_I + 2, COL_I, "Angle",         PE_IMAGEANGLE,   {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_I + 3, COL_I, "Angle Inc",     PE_IMAGEINC,     {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_I + 4, COL_I, "Expansion",     PE_IMAGEEXP,     {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_I + 5, COL_I, "Center x",      PE_IMAGEXOFFSET, {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_I + 6, COL_I, "Center y",      PE_IMAGEYOFFSET, {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_I + 7, COL_I, "Anti-alias",    PE_IMAGEALIAS,   {{}, {0, 0, COM_ENUM_RST},  {0, 0, COM_ENUM_INC},  {0, 0, COM_ENUM_DEC}}, A_INVALID},
  {ROW_I + 8, COL_I, "Image opacity",   PE_IMAGEALPHA,   {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0,0,COM_DEC_FLOAT}}, A_INVALID},

  // Auxillary
  {ROW_A + 1, COL_A, "Tensor Broadcast",     PE_INVALID,   {{0, 0, COM_BROAD_FLIP}, {0, 0, COM_BROAD_RST}}, A_INVALID},
  {ROW_A + 2, COL_A, "Pause",                PE_PAUSE,     {{0, 0, COM_BOOL_FLIP}, {0, 0, COM_BOOL_RST}}, A_INVALID},
  {ROW_A + 3, COL_A, "Set output intensity", PE_INTENSITY, {{}, {0, 0, COM_RST_FLOAT}, {0, 0, COM_INC_FLOAT}, {0, 0, COM_DEC_FLOAT}}, A_INVALID},
  {ROW_A + 4, COL_A, "Frame delay(ms)",      PE_DELAY,     {{}, {0, 0, COM_INT_RST}, {0, 0, COM_INT_INC}, {0, 0, COM_INT_DEC}}, A_INVALID},
  {ROW_A + 5, COL_A, "Float step",           PE_FLOATINC,  {{}, {KMOD_ALT, SDLK_COMMA, COM_RST_FLOAT}, {KMOD_ALT, SDLK_m, COM_STEP_INC}, {KMOD_ALT, SDLK_PERIOD, COM_STEP_DEC}}, A_INVALID},
  {ROW_A + 6, COL_A, "All modes off",        PE_INVALID,   {{KMOD_CTRL, SDLK_l, COM_MODEOFF}, {0, 0, COM_MODEOFF}}, A_INVALID},

  // Quit
  {ROW_COUNT - 2, 4, "Quit",           PE_INVALID,  {{KMOD_NONE, SDLK_ESCAPE, COM_EXIT}, {KMOD_NONE, SDLK_ESCAPE, COM_EXIT}}, A_INVALID},
};
const int displayCommandCount = sizeof(displayCommand) / sizeof(command_t);

// Non clickable text displayed in the gui.
const guiText_t displayText[] = {
  {ROW_T + 2, COL_T, " Add text - <Unmodified keys>"},
  {ROW_PA + 3, COL_PA, " Change to set # - <c> 0-9"},
  {ROW_PA + 4, COL_PA, " Copy set to # - <c> <a> 0-9"},
  {ROW_PA + 5, COL_PA, " Load buffer from set # - <a> 0-9"},
  {ROW_PA + 6, COL_PA, " Save to disk - <a> <s> 0-9, a-z"},
  {ROW_PA + 7, COL_PA, " Load saved set - <c> <s> 0-9, a-z"},
  {ROW_ME + 1, COL_ME, "Left button: Edit / toggle item"},
  {ROW_ME + 2, COL_ME, "Right button: Reset item / cancel selection"},
  {ROW_ME + 3, COL_ME, "Mouse wheel: Increment / decrement item"},
  {ROW_ME + 4, COL_ME, "Left click color swatch to edit item palette"},
};
const int displayTextCount = sizeof(displayText) / sizeof(guiText_t);

// Some labels on the gui.
const guiText_t labelText[] = {
  {FPS_ROW, 0,       "Live Preview          FPS:"},
  {FPS_ROW, 4,       "Alternate Preview    FPS:"},
  {ROW_COUNT - 1, 4, "                 GUI FPS:"},
  {ROW_COUNT - 3, 0, " Text buffer:"},
};
const int labelCount = sizeof(labelText) / sizeof(guiText_t);

// Headers with highlighted backgrounds on the gui.
const guiText_t headerText[] = {
  {ROW_PA,  COL_PA, "Pattern sets:"},
  {ROW_P,   COL_P,  "Color suppression:"},
  {ROW_A,   COL_A,  "Auxiliary:"},
  {ROW_CB,  COL_CB, "Crossbar seeds:"},
  {ROW_I,   COL_I,  "Image overlay:"},
  {ROW_R,   COL_R,  "Random dot seeds:"},
  {ROW_C,   COL_C,  "Item palette editor:"},
  {ROW_D,   COL_D,  "Averager:"},
  {ROW_O,   COL_O,  "Misc seeds:"},
  {ROW_F,   COL_F,  "Adder (fader):"},
  {ROW_M,   COL_M,  "Multiplier:"},
  {ROW_T,   COL_T,  "Text entry:"},
  {ROW_S,   COL_S,  "Scrollers:"},
  {ROW_PR,  COL_PR, "Post rotation:"},
  {ROW_PE,  COL_PE, "Pre rotation:"},
  {ROW_MIR, COL_MIR,"Mirrors:"},
  {ROW_ME,  COL_ME, "Mouse controls:"},
};
const int headerTextCount = sizeof(headerText) / sizeof(guiText_t);

// These commands correspond to key presses, but aren't displayed as mouse-over items.
const command_t otherCommands[] = {
  {-1, 0, "Orientation", PE_INVALID, {{KMOD_CTRL, SDLK_v, COM_ORIENTATION}}},
  {-1, 2, "Scroll Up", PE_INVALID, {{KMOD_NONE, SDLK_UP, COM_SCROLL_UP}}},
  {-1, 2, "Scroll Down", PE_INVALID, {{KMOD_NONE, SDLK_DOWN, COM_SCROLL_DOWN}}},
  {-1, 2, "Scroll Left", PE_INVALID, {{KMOD_NONE, SDLK_LEFT, COM_SCROLL_LEFT}}},
  {-1, 2, "Scroll Right", PE_INVALID, {{KMOD_NONE, SDLK_RIGHT, COM_SCROLL_RIGHT}}},
  {-1, 2, "Scroll Up Once", PE_INVALID, {{KMOD_CTRL, SDLK_UP, COM_SCROLL_UPC}}},
  {-1, 2, "Scroll Down Once", PE_INVALID, {{KMOD_CTRL, SDLK_DOWN, COM_SCROLL_DOWNC}}},
  {-1, 2, "Scroll Left Once", PE_INVALID, {{KMOD_CTRL, SDLK_LEFT, COM_SCROLL_LEFTC}}},
  {-1, 2, "Scroll Right Once", PE_INVALID, {{KMOD_CTRL, SDLK_RIGHT, COM_SCROLL_RIGHTC}}},
  {-1, 2, "Activate Scroll Up", PE_INVALID, {{KMOD_ALT, SDLK_UP, COM_SCROLL_UPA}}},
  {-1, 2, "Activate Scroll Down", PE_INVALID, {{KMOD_ALT, SDLK_DOWN, COM_SCROLL_DOWNA}}},
  {-1, 2, "Activate Scroll Left", PE_INVALID, {{KMOD_ALT, SDLK_LEFT, COM_SCROLL_LEFTA}}},
  {-1, 2, "Activate Scroll Right", PE_INVALID, {{KMOD_ALT, SDLK_RIGHT, COM_SCROLL_RIGHTA}}},
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

// Verify item sizes
void CheckCount(bool_t *rc, int a, int b, const char *at, const char *bt) {
  if (a != b) {
    fprintf(stderr, "Programmer error: Mismatched %s(%i) != %s(%i)!\n", at, a, bt, b);
    *rc = NO;
  }
}

// Print and set an error.
// a = array index, b = stored enum, at = array name, bt = enum name,
// aName = optional array element name.
void Errps(bool_t *rc, int a, int b, const char * at, const char *bt, const char *aName) {
  if (a != b) {
    fprintf(stderr, "Programmer error: %s does not match %s array!\n", bt, at);
    if (!aName) {
      fprintf(stderr, "Element %i, has incorrect enumeration value %i!\n", a, b);
    } else {
      fprintf(stderr, "Element %i, \"%s\" has incorrect enumeration value %i!\n", a, aName, b);
    }
    *rc = NO;
  }
}

// Check to make sure that some of the program's data sets are built correctly.
// This is done to ensure that certain arrays can be accessed via enumeration
// values rather than indicies (i.e. enumerations must match indecies).  It
// catches some programming errors that can result in seg faults.
bool_t VerifyStructuralIntegrity(void) {
  int i;
  bool_t rc = YES;

  for (i = 0; i < patternElementCount; i++)
    Errps(&rc, i, patternElements[i].index, "patternElements", "patternElement_e", patternElements[i].name);
  for (i = 0; i < namedColorsCount; i++)
    Errps(&rc, i, namedColors[i].index, "namedColors", "color_e", NULL);
  for (i = 0; i < seedPaletteCount; i++)
    Errps(&rc, i, seedPalette[i].seed, "seedPalette", "alterPalettes_e", NULL);
  for (i = 0; i < enumerationsCount; i++)
    Errps(&rc, i, enumerations[i].type, "enumerations", "enums_e", NULL);

  CheckCount(&rc, PE_COUNT, patternElementCount, "patternElement_e", "patternElements");
  CheckCount(&rc, CE_COUNT, namedColorsCount, "color_e", "namedColors");
  // CheckCount(&rc, CE_COUNT, colorTextSize, "color_e", "colorText");
  CheckCount(&rc, A_COUNT, seedPaletteCount, "alterPalettes_e", "seedPalette");
  CheckCount(&rc, A_COUNT, alterTextCount, "alterPalettes_e", "alterText");
  CheckCount(&rc, E_COUNT, enumerationsCount, "enums_e", "enumerations");
  CheckCount(&rc, DIR_COUNT, dirTextCount, "dir_e", "dirText");
  CheckCount(&rc, FM_COUNT, fadeModeTextCount, "fadeModes_e", "fadeModeText");
  CheckCount(&rc, OS_COUNT, oneShotTextCount, "oneShots_e", "oneShotText");
  CheckCount(&rc, SM_COUNT, shiftTextCount, "shiftModes_e", "shiftText");
  CheckCount(&rc, TS_COUNT, textModeTextCount, "textMode_e", "textModeText");
  CheckCount(&rc, CM_COUNT, colorCycleTextCount, "colorCycleModes_e", "colorCycleText");
  CheckCount(&rc, CB_COUNT, crossBarTextCount, "crossBar_e", "crossbarText");
  CheckCount(&rc, OO_COUNT, operateTextCount, "operateOn_e", "operateText");

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

// Map between seedPalette and the pattern set elements.
patternElement_e GetSelectElement(int set, patternElement_e selector) {
  switch(selector) {
    case PE_FGCOLORA: return seedPalette[SENUM(set, PE_PALETTEALTER)].fgColorA;
    case PE_FGCOLORB: return seedPalette[SENUM(set, PE_PALETTEALTER)].fgColorB;
    case PE_FGCYCLE:  return seedPalette[SENUM(set, PE_PALETTEALTER)].fgCycleMode;
    case PE_FGCYCLERATE: return seedPalette[SENUM(set, PE_PALETTEALTER)].fgCycleRate;
    case PE_FGALPHA: return seedPalette[SENUM(set, PE_PALETTEALTER)].fgAlpha;
    case PE_BGCOLORA: return seedPalette[SENUM(set, PE_PALETTEALTER)].bgColorA;
    case PE_BGCOLORB: return seedPalette[SENUM(set, PE_PALETTEALTER)].bgColorB;
    case PE_BGCYCLE:  return seedPalette[SENUM(set, PE_PALETTEALTER)].bgCycleMode;
    case PE_BGCYCLERATE: return seedPalette[SENUM(set, PE_PALETTEALTER)].bgCycleRate;
    case PE_BGALPHA: return seedPalette[SENUM(set, PE_PALETTEALTER)].bgAlpha;
    default: return PE_INVALID;
  }
}

