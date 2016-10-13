// layout.h
// Manages the layout definitions for the tensor gui.
// Joshua Krueger
// 2016_10_12
//

#ifndef LAYOUT_H_
#define LAYOUT_H_

  // Includes
  #include "tensor.h"

  // Primary Defines
  #define GUI_WINDOW_WIDTH 1126
  #define GUI_WINDOW_HEIGHT 854
  #define DISPLAY_FONT_SIZE 14
  #define CHAR_W 8
  #define TEXTURE_BORDER 3
  
  // Derived Defines
  #define DISPLAY_TEXT_HEIGHT DISPLAY_FONT_SIZE
  #define ROW_COUNT (GUI_WINDOW_HEIGHT / DISPLAY_TEXT_HEIGHT)
  
  // Preview windows
  #define PREVIEW_PIXEL_SIZE (((GUI_WINDOW_WIDTH * 100) / 32) / (2 + TENSOR_WIDTH) / 10)
  //#define PREVIEW_PIXEL_SIZE 12
  #define PREVIEW_BORDER (PREVIEW_PIXEL_SIZE)
  #define PREVIEW_LIVE_POS_X 1
  #define PREVIEW_LIVE_POS_Y 1
  #define PREVIEW_ALT_POS_X (GUI_WINDOW_WIDTH - (TENSOR_WIDTH * PREVIEW_PIXEL_SIZE) - (PREVIEW_BORDER * 2) - 1)
  #define PREVIEW_ALT_POS_Y 1
  
  // Text flows
  #define ROW_O 27
  #define COL_O 0
  #define ROW_CB (ROW_O + 11)
  #define COL_CB COL_O
  #define ROW_R (ROW_CB + 3)
  #define COL_R COL_CB
  #define ROW_C (ROW_R + 3)
  #define COL_C COL_R

  #define ROW_PA 1
  #define COL_PA 2
  #define ROW_D (ROW_PA + 15)
  #define COL_D COL_PA
  #define ROW_F (ROW_D + 3)
  #define COL_F COL_D
  #define ROW_M (ROW_F + 4)
  #define COL_M COL_F
  #define ROW_P (ROW_M + 3)
  #define COL_P COL_M
  #define ROW_S (ROW_P + 4)
  #define COL_S COL_P
  #define ROW_T (ROW_S + 14)
  #define COL_T COL_S
  #define ROW_ME (ROW_T + 10)
  #define COL_ME COL_T

  #define ROW_MIR 27
  #define COL_MIR 4
  #define ROW_PR (ROW_MIR + 3)
  #define COL_PR COL_MIR
  #define ROW_PE (ROW_PR + 6)
  #define COL_PE COL_PR
  #define ROW_I (ROW_PE + 6)
  #define COL_I COL_PE
  #define ROW_A (ROW_I + 9)
  #define COL_A COL_I

  #define FPS_ROW 25
  
  // How wide to make a parameter in characters.
  #define PARAMETER_WIDTH 10
  
  // Column layout
  #define COLAWIDTH 33
  #define COLBWIDTH 39
  #define COLCWIDTH 33
  #define PARMAWIDTH 13
  #define PARMBWIDTH 12
  #define COLA2 5
  #define ACOL 1
  #define A2COL (ACOL + (COLAWIDTH - COLA2) * CHAR_W)
  #define A2COLEND (A2COL + CHAR_W * COLA2)
  #define BCOL  (ACOL + COLAWIDTH * CHAR_W)
  #define CCOL  (BCOL + PARMAWIDTH * CHAR_W)
  #define B2COL (CCOL + (COLBWIDTH - COLA2) * CHAR_W)
  #define B2COLEND (B2COL + CHAR_W * COLA2)
  #define DCOL  (CCOL + COLBWIDTH * CHAR_W)
  #define ECOL  (DCOL + PARMBWIDTH * CHAR_W)
  #define C2COL (ECOL + (COLCWIDTH - COLA2) * CHAR_W)
  #define C2COLEND (C2COL + CHAR_W * COLA2)
  #define FCOL  (ECOL + COLCWIDTH * CHAR_W)
  
  // Colors used in the gui
  // Colors for the info display texts.
  #define DISPLAY_COLOR_PARMS cAzure
  #define DISPLAY_COLOR_PARMSBG cBlack
  #define DISPLAY_COLOR_TITLES cBlack
  #define DISPLAY_COLOR_TITLESBG cGray
  #define DISPLAY_COLOR_TEXTS cWhite
  #define DISPLAY_COLOR_TEXTSBG cBlack
  #define DISPLAY_COLOR_TEXTS_HL cBlack
  #define DISPLAY_COLOR_TEXTSBG_HL cWhite
  #define DISPLAY_COLOR_TBUF cLtBlue
  #define DISPLAY_COLOR_TBUFBG cBlack
  #define DISPLAY_COLOR_INFO cGreen
  #define DISPLAY_COLOR_INFOBG cBlack
  #define DISPLAY_COLOR_LABEL cYellow
  #define DISPLAY_COLOR_LABELBG cBlack
  
  // Functions
  int ColToPixel(int col);
  int GetMaxCol(void);

#endif /* ifndef LAYOUT_H_ */
