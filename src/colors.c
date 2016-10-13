// colors.c
// Joshua Krueger
// 2016_10_12
// Contains useful definitions for using colors.

#include "colors.h"

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
const color_t cLtBlue = CD_LTBLUE;
const color_t cDkYellow = CD_DKYELLOW;
const color_t cHdkGreen = CD_HALFDK_GREEN;

// Named color palettes
const namedPalette_t paletteRGB = { (color_e[]) { CE_RED, CE_GREEN, CE_BLUE }, 3 };
const namedPalette_t paletteCMY = { (color_e[]) { CE_CYAN, CE_MAGENTA, CE_YELLOW }, 3};
const namedPalette_t paletteRWB = { (color_e[]) { CE_RED, CE_WHITE, CE_BLUE }, 3};
const namedPalette_t paletteSec = { (color_e[]) { CE_RED, CE_YELLOW, CE_GREEN,
  CE_CYAN, CE_BLUE, CE_MAGENTA }, 6 };
const namedPalette_t paletteTer = { (color_e[]) { CE_RED, CE_ORANGE, CE_YELLOW,
  CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN, CE_AZURE, CE_BLUE, CE_VIOLET,
  CE_MAGENTA, CE_ROSE }, 12 };
const namedPalette_t paletteGry = { (color_e[]) { CE_WHITE, CE_LTGRAY, CE_GRAY,
  CE_DKGRAY, CE_BLACK}, 5 };
const namedPalette_t paletteSymGry = { (color_e[]) { CE_WHITE, CE_LTGRAY, CE_GRAY,
  CE_DKGRAY, CE_BLACK, CE_DKGRAY, CE_GRAY, CE_LTGRAY}, 8 };
  
