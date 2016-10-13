// colors.h
// Joshua Krueger
// 2016_10_12
// Contains useful definitions for using colors.

#ifndef COLORS_H_
  #define COLORS_H_

  // 32 BPP RGBA Color Type
  // Named or array access provided through union.
  typedef union color_t {
    struct {
      unsigned char r, g, b, a;
    };
    unsigned char rgba[4];
  } color_t;

  // Named color definitions - shoulda done this algorithmically.
  #define CD_SDKRED        {.r = 42, .g = 0,   .b = 0,   .a = 255}
  #define CD_SDKORANGE     {.r = 42, .g = 21, .b = 0,   .a = 255}
  #define CD_SDKYELLOW     {.r = 42, .g = 42, .b = 0,   .a = 255}
  #define CD_SDKCHARTREUSE {.r = 21, .g = 42, .b = 0,   .a = 255}
  #define CD_SDKGREEN      {.r = 0,   .g = 42, .b = 0,   .a = 255}
  #define CD_SDKAQUA       {.r = 0,   .g = 42, .b = 21, .a = 255}
  #define CD_SDKCYAN       {.r = 0,   .g = 42, .b = 42, .a = 255}
  #define CD_SDKAZURE      {.r = 0  , .g = 21, .b = 42, .a = 255}
  #define CD_SDKBLUE       {.r = 0,   .g = 0,   .b = 42, .a = 255}
  #define CD_SDKVIOLET     {.r = 21, .g = 0,   .b = 42, .a = 255}
  #define CD_SDKMAGENTA    {.r = 42, .g = 0,   .b = 42, .a = 255}
  #define CD_SDKROSE       {.r = 21, .g = 0,   .b = 21, .a = 255}

  #define CD_DKRED        {.r = 85, .g = 0,   .b = 0,   .a = 255}
  #define CD_DKORANGE     {.r = 85, .g = 42, .b = 0,   .a = 255}
  #define CD_DKYELLOW     {.r = 85, .g = 85, .b = 0,   .a = 255}
  #define CD_DKCHARTREUSE {.r = 42, .g = 85, .b = 0,   .a = 255}
  #define CD_DKGREEN      {.r = 0,   .g = 85, .b = 0,   .a = 255}
  #define CD_DKAQUA       {.r = 0,   .g = 85, .b = 42, .a = 255}
  #define CD_DKCYAN       {.r = 0,   .g = 85, .b = 85, .a = 255}
  #define CD_DKAZURE      {.r = 0  , .g = 42, .b = 85, .a = 255}
  #define CD_DKBLUE       {.r = 0,   .g = 0,   .b = 85, .a = 255}
  #define CD_DKVIOLET     {.r = 42, .g = 0,   .b = 85, .a = 255}
  #define CD_DKMAGENTA    {.r = 85, .g = 0,   .b = 85, .a = 255}
  #define CD_DKROSE       {.r = 85, .g = 0,   .b = 42, .a = 255}

  #define CD_MDRED        {.r = 170, .g = 0,   .b = 0,   .a = 255}
  #define CD_MDORANGE     {.r = 170, .g = 85, .b = 0,   .a = 255}
  #define CD_MDYELLOW     {.r = 170, .g = 170, .b = 0,   .a = 255}
  #define CD_MDCHARTREUSE {.r = 85, .g = 170, .b = 0,   .a = 255}
  #define CD_MDGREEN      {.r = 0,   .g = 170, .b = 0,   .a = 255}
  #define CD_MDAQUA       {.r = 0,   .g = 170, .b = 85, .a = 255}
  #define CD_MDCYAN       {.r = 0,   .g = 170, .b = 170, .a = 255}
  #define CD_MDAZURE      {.r = 0  , .g = 85, .b = 170, .a = 255}
  #define CD_MDBLUE       {.r = 0,   .g = 0,   .b = 170, .a = 255}
  #define CD_MDVIOLET     {.r = 85, .g = 0,   .b = 170, .a = 255}
  #define CD_MDMAGENTA    {.r = 170, .g = 0,   .b = 170, .a = 255}
  #define CD_MDROSE       {.r = 170, .g = 0,   .b = 85, .a = 255}

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

  #define CD_MLRED        {.r = 255, .g = 85,   .b = 85,   .a = 255}
  #define CD_MLORANGE     {.r = 255, .g = 170, .b = 85,   .a = 255}
  #define CD_MLYELLOW     {.r = 255, .g = 255, .b = 85,   .a = 255}
  #define CD_MLCHARTREUSE {.r = 170, .g = 255, .b = 85,   .a = 255}
  #define CD_MLGREEN      {.r = 85,   .g = 255, .b = 85,   .a = 255}
  #define CD_MLAQUA       {.r = 85,   .g = 255, .b = 170, .a = 255}
  #define CD_MLCYAN       {.r = 85,   .g = 255, .b = 255, .a = 255}
  #define CD_MLAZURE      {.r = 85  , .g = 170, .b = 255, .a = 255}
  #define CD_MLBLUE       {.r = 85,   .g = 85,   .b = 255, .a = 255}
  #define CD_MLVIOLET     {.r = 170, .g = 85,   .b = 255, .a = 255}
  #define CD_MLMAGENTA    {.r = 255, .g = 85,   .b = 255, .a = 255}
  #define CD_MLROSE       {.r = 255, .g = 85,   .b = 170, .a = 255}

  #define CD_LTRED        {.r = 255, .g = 170,   .b = 170,   .a = 255}
  #define CD_LTORANGE     {.r = 255, .g = 213, .b = 170,   .a = 255}
  #define CD_LTYELLOW     {.r = 255, .g = 255, .b = 170,   .a = 255}
  #define CD_LTCHARTREUSE {.r = 213, .g = 255, .b = 170,   .a = 255}
  #define CD_LTGREEN      {.r = 170,   .g = 255, .b = 170,   .a = 255}
  #define CD_LTAQUA       {.r = 170,   .g = 255, .b = 213, .a = 255}
  #define CD_LTCYAN       {.r = 170,   .g = 255, .b = 255, .a = 255}
  #define CD_LTAZURE      {.r = 170  , .g = 213, .b = 255, .a = 255}
  #define CD_LTBLUE       {.r = 170,   .g = 170,   .b = 255, .a = 255}
  #define CD_LTVIOLET     {.r = 213, .g = 170,   .b = 255, .a = 255}
  #define CD_LTMAGENTA    {.r = 255, .g = 170,   .b = 255, .a = 255}
  #define CD_LTROSE       {.r = 255, .g = 170,   .b = 213, .a = 255}

  #define CD_HALFDK_RED        {.r = 127, .g = 0,   .b = 0,   .a = 255}
  #define CD_HALFDK_ORANGE     {.r = 127, .g = 63,  .b = 0,   .a = 255}
  #define CD_HALFDK_YELLOW     {.r = 127, .g = 127, .b = 0,   .a = 255}
  #define CD_HALFDK_CHARTREUSE {.r = 63,  .g = 127, .b = 0,   .a = 255}
  #define CD_HALFDK_GREEN      {.r = 0,   .g = 127, .b = 0,   .a = 255}
  #define CD_HALFDK_AQUA       {.r = 0,   .g = 127, .b = 63,  .a = 255}
  #define CD_HALFDK_CYAN       {.r = 0,   .g = 127, .b = 127, .a = 255}
  #define CD_HALFDK_AZURE      {.r = 0  , .g = 63,  .b = 127, .a = 255}
  #define CD_HALFDK_BLUE       {.r = 0,   .g = 0,   .b = 127, .a = 255}
  #define CD_HALFDK_VIOLET     {.r = 63,  .g = 0,   .b = 127, .a = 255}
  #define CD_HALFDK_MAGENTA    {.r = 127, .g = 0,   .b = 127, .a = 255}
  #define CD_HALFDK_ROSE       {.r = 127, .g = 0,   .b = 63,  .a = 255}

  #define CD_HALFLT_RED        {.r = 255, .g = 127,   .b = 127,   .a = 255}
  #define CD_HALFLT_ORANGE     {.r = 255, .g = 191, .b = 127,   .a = 255}
  #define CD_HALFLT_YELLOW     {.r = 255, .g = 255, .b = 127,   .a = 255}
  #define CD_HALFLT_CHARTREUSE {.r = 191, .g = 255, .b = 127,   .a = 255}
  #define CD_HALFLT_GREEN      {.r = 127,   .g = 255, .b = 127,   .a = 255}
  #define CD_HALFLT_AQUA       {.r = 127,   .g = 255, .b = 191, .a = 255}
  #define CD_HALFLT_CYAN       {.r = 127,   .g = 255, .b = 255, .a = 255}
  #define CD_HALFLT_AZURE      {.r = 127  , .g = 191, .b = 255, .a = 255}
  #define CD_HALFLT_BLUE       {.r = 127,   .g = 127,   .b = 255, .a = 255}
  #define CD_HALFLT_VIOLET     {.r = 191, .g = 127,   .b = 255, .a = 255}
  #define CD_HALFLT_MAGENTA    {.r = 255, .g = 127,   .b = 255, .a = 255}
  #define CD_HALFLT_ROSE       {.r = 255, .g = 127,   .b = 191, .a = 255}

  #define CD_WHITE      {.r = 255, .g = 255, .b = 255, .a = 255}
  #define CD_LTGRAY     {.r = 205, .g = 205, .b = 205, .a = 255}
  #define CD_GRAY       {.r = 154, .g = 154, .b = 154, .a = 255}
  #define CD_DKGRAY     {.r = 102,  .g = 102,  .b = 102,  .a = 255}
  #define CD_SDKGRAY    {.r = 51,  .g = 51,  .b = 51,  .a = 255}
  #define CD_BLACK      {.r = 0,   .g = 0,   .b = 0,   .a = 255}
  //~ #define CD_DKYELLOW   {.r = 192, .g = 192, .b = 0,   .a = 255}
  //~ #define CD_LTBLUE     {.r = 127,   .g = 192,   .b = 255, .a = 255}
  
  // Named color constants
  extern const color_t cRed, cOrange, cYellow, cChartreuse, cGreen, cAqua, cCyan,
    cAzure, cBlue, cViolet, cMagenta, cRose, cWhite, cLtGray, cGray, cDkGray,
    cBlack, cLtBlue, cDkYellow, cHdkGreen;
    
  // The important color names, enumerated.
  // If you change color_e, change namedColors and colorsText too.
  typedef enum color_e {
    CE_INVALID = -1,
    CE_SDKRED,        CE_DKRED,        CE_MDRED,        CE_RED,        CE_MLRED,        CE_LTRED,
    CE_SDKORANGE,     CE_DKORANGE,     CE_MDORANGE,     CE_ORANGE,     CE_MLORANGE,     CE_LTORANGE,
    CE_SDKYELLOW,     CE_DKYELLOW,     CE_MDYELLOW,     CE_YELLOW,     CE_MLYELLOW,     CE_LTYELLOW,
    CE_SDKCHARTREUSE, CE_DKCHARTREUSE, CE_MDCHARTREUSE, CE_CHARTREUSE, CE_MLCHARTREUSE, CE_LTCHARTREUSE,
    CE_SDKGREEN,      CE_DKGREEN,      CE_MDGREEN,      CE_GREEN,      CE_MLGREEN,      CE_LTGREEN,
    CE_SDKAQUA,       CE_DKAQUA,       CE_MDAQUA,       CE_AQUA,       CE_MLAQUA,       CE_LTAQUA,
    CE_SDKCYAN,       CE_DKCYAN,       CE_MDCYAN,       CE_CYAN,       CE_MLCYAN,       CE_LTCYAN,
    CE_SDKAZURE,      CE_DKAZURE,      CE_MDAZURE,      CE_AZURE,      CE_MLAZURE,      CE_LTAZURE,
    CE_SDKBLUE,       CE_DKBLUE,       CE_MDBLUE,       CE_BLUE,       CE_MLBLUE,       CE_LTBLUE,
    CE_SDKVIOLET,     CE_DKVIOLET,     CE_MDVIOLET,     CE_VIOLET,     CE_MLVIOLET,     CE_LTVIOLET,
    CE_SDKMAGENTA,    CE_DKMAGENTA,    CE_MDMAGENTA,    CE_MAGENTA,    CE_MLMAGENTA,    CE_LTMAGENTA,
    CE_SDKROSE,       CE_DKROSE,       CE_MDROSE,       CE_ROSE,       CE_MLROSE,       CE_LTROSE,
    CE_BLACK,         CE_SDKGRAY,      CE_DKGRAY,       CE_GRAY,       CE_LTGRAY,       CE_WHITE,
    CE_COUNT // Last.
  } color_e;

  // colorName_t typedef - used to link color name enumerations to color constants
  typedef struct colorName_t {
    const color_e index;
    const color_t color;
  } colorName_t;

  // Named palettes - A named palette is an array of color_e color names. The
  // namedPalette_t type points to a named palette and its size.
  typedef struct namedPalette_t {
    const color_e *palette;
    const int size;
  } namedPalette_t;

  // Some color palettes
  extern const namedPalette_t paletteRGB, paletteCMY, paletteRWB, paletteSec, paletteTer, paletteGry, paletteSymGry;

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
  
#endif /* COLORS_H_ */
