// layout.c
// Manages the gui layout
// Joshua Krueger
// 2016_10_12

// Includes
#include <stdio.h>
#include "layout.h"


// Constants
const int colToPixel[] = {ACOL, BCOL, CCOL, DCOL, ECOL, FCOL, GUI_WINDOW_WIDTH,
    A2COL, A2COLEND, B2COL, B2COLEND, C2COL, C2COLEND, GUI_WINDOW_WIDTH};
#define MAX_COL (sizeof(colToPixel) / sizeof(const int))

int ColToPixel(int col) {
  if (col >= MAX_COL) {
    fprintf(stderr, "Selected column %i exceeds maximum %i!\n", col, (int) MAX_COL - 1);
    return colToPixel[MAX_COL - 1];
  } else if (col < 0) {
    fprintf(stderr, "Selected column %i less than 0?\n", col);
    return colToPixel[0];
  }

  return colToPixel[col];
}
 
int GetMaxCol(void) {
  return MAX_COL;
}
