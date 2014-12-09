// Support for drawing things to the Tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2014_12_04

#ifndef DRAW_H_
  #define DRAW_H_

  #include "useful.h"

  #define WINDOW_WIDTH 1024
  #define WINDOW_HEIGHT 768

  // Preview window definitions
  #define PREVIEW_PIXEL_SIZE 10
  #define PREVIEW_BORDER 10
  #define PREVIEW_LIVE_POS_X 0
  #define PREVIEW_LIVE_POS_Y 0
  #define PREVIEW_ALT_POS_X (WINDOW_WIDTH - (TENSOR_WIDTH * PREVIEW_PIXEL_SIZE) - (PREVIEW_BORDER * 2))
  #define PREVIEW_ALT_POS_Y 0

  // Layout
  #define ROW_PA 0
  #define COL_PA 2
  #define ROW_A 45
  #define COL_A 4
  #define ROW_P 35
  #define COL_P 2
  #define ROW_D 15
  #define COL_D 2
  #define ROW_O 23
  #define COL_O 0
  #define ROW_MI (ROW_O + 8)
  #define COL_MI COL_O
  #define ROW_CB (ROW_MI + 4)
  #define COL_CB COL_MI
  #define ROW_R (ROW_CB + 3)
  #define COL_R COL_CB
  #define ROW_F 18
  #define COL_F 2
  #define ROW_M 22
  #define COL_M 2
  #define ROW_MIR 25
  #define COL_MIR 2
  #define ROW_C 28
  #define COL_C 2
  #define ROW_S 39
  #define COL_S 2
  #define ROW_PR 23
  #define COL_PR 4
  #define ROW_PE (ROW_PR + 7)
  #define COL_PE COL_PR
  #define ROW_I (ROW_PE + 7)
  #define COL_I COL_PE
  #define ROW_T 41
  #define COL_T 0

  #define PARAMETER_WIDTH 10

  // 32 bit per pixel color type - Named or array access through union.
  typedef union color_t {
    struct {
      unsigned char r, g, b, a;
    };
    unsigned char rgba[4];
  } color_t;

  // Colors
  // Named color  definitions
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
  #define CD_HALF_RED        {.r = 127, .g = 0,   .b = 0,   .a = 255}
  #define CD_HALF_ORANGE     {.r = 127, .g = 63,  .b = 0,   .a = 255}
  #define CD_HALF_YELLOW     {.r = 127, .g = 127, .b = 0,   .a = 255}
  #define CD_HALF_CHARTREUSE {.r = 63,  .g = 127, .b = 0,   .a = 255}
  #define CD_HALF_GREEN      {.r = 0,   .g = 127, .b = 0,   .a = 255}
  #define CD_HALF_AQUA       {.r = 0,   .g = 127, .b = 63,  .a = 255}
  #define CD_HALF_CYAN       {.r = 0,   .g = 127, .b = 127, .a = 255}
  #define CD_HALF_AZURE      {.r = 0  , .g = 63,  .b = 127, .a = 255}
  #define CD_HALF_BLUE       {.r = 0,   .g = 0,   .b = 127, .a = 255}
  #define CD_HALF_VIOLET     {.r = 63,  .g = 0,   .b = 127, .a = 255}
  #define CD_HALF_MAGENTA    {.r = 127, .g = 0,   .b = 127, .a = 255}
  #define CD_HALF_ROSE       {.r = 127, .g = 0,   .b = 63,  .a = 255}
  #define CD_LTRED        {.r = 255, .g = 127,   .b = 127,   .a = 255}
  #define CD_LTORANGE     {.r = 255, .g = 191, .b = 127,   .a = 255}
  #define CD_LTYELLOW     {.r = 255, .g = 255, .b = 127,   .a = 255}
  #define CD_LTCHARTREUSE {.r = 191, .g = 255, .b = 127,   .a = 255}
  #define CD_LTGREEN      {.r = 127,   .g = 255, .b = 127,   .a = 255}
  #define CD_LTAQUA       {.r = 127,   .g = 255, .b = 191, .a = 255}
  #define CD_LTCYAN       {.r = 127,   .g = 255, .b = 255, .a = 255}
  #define CD_LTAZURE      {.r = 127  , .g = 191, .b = 255, .a = 255}
  #define CD_LTBLUE       {.r = 127,   .g = 127,   .b = 255, .a = 255}
  #define CD_LTVIOLET     {.r = 191, .g = 127,   .b = 255, .a = 255}
  #define CD_LTMAGENTA    {.r = 255, .g = 127,   .b = 255, .a = 255}
  #define CD_LTROSE       {.r = 255, .g = 127,   .b = 191, .a = 255}
  #define CD_WHITE      {.r = 255, .g = 255, .b = 255, .a = 255}
  #define CD_LTGRAY     {.r = 191, .g = 191, .b = 191, .a = 255}
  #define CD_GRAY       {.r = 127, .g = 127, .b = 127, .a = 255}
  #define CD_DKGRAY     {.r = 63,  .g = 63,  .b = 63,  .a = 255}
  #define CD_BLACK      {.r = 0,   .g = 0,   .b = 0,   .a = 255}
  #define CD_DKYELLOW   {.r = 192, .g = 192, .b = 0,   .a = 255}
  //~ #define CD_LTBLUE     {.r = 127,   .g = 192,   .b = 255, .a = 255}

  extern const color_t cRed, cOrange, cYellow, cChartreuse, cGreen, cAqua, cCyan,
    cAzure, cBlue, cViolet, cMagenta, cRose, cWhite, cLtGray, cGray, cDkGray,
    cBlack, cLtBlue, cDkYellow;

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

  // The important color names, enumerated.
  // If you change color_e, change namedColors and colorsText too.
  typedef enum color_e {
    CE_INVALID = -1,
    CE_RED = 0, CE_ORANGE, CE_YELLOW, CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN,
    CE_AZURE, CE_BLUE, CE_VIOLET, CE_MAGENTA, CE_ROSE, CE_WHITE, CE_LTGRAY,
    CE_GRAY, CE_DKGRAY, CE_BLACK, CE_HALF_RED, CE_HALF_ORANGE, CE_HALF_YELLOW,
    CE_HALF_CHARTREUSE, CE_HALF_GREEN, CE_HALF_AQUA, CE_HALF_CYAN,
    CE_HALF_AZURE, CE_HALF_BLUE, CE_HALF_VIOLET, CE_HALF_MAGENTA, CE_HALF_ROSE,
    CE_LTRED, CE_LTORANGE, CE_LTYELLOW, CE_LTCHARTREUSE, CE_LTGREEN, CE_LTAQUA,
    CE_LTCYAN, CE_LTAZURE, CE_LTBLUE, CE_LTVIOLET, CE_LTMAGENTA, CE_LTROSE,
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
  extern const namedPalette_t paletteRGB;
  extern const namedPalette_t paletteCMY;
  extern const namedPalette_t paletteSec;
  extern const namedPalette_t paletteTer;
  extern const namedPalette_t paletteGry;

  // Points
  typedef struct point_t {
    int x, y;
  } point_t;

  // Rectangular box.
  typedef struct box_t {
    int x, y;
    int w, h;
  } box_t;

  //
  typedef struct displayText_t {
    SDL_Texture * texture;
    box_t targetBox;
  } displayText_t;

  // Function prototypes
  bool_t InitGui(void);
  void QuitGui(void);
  void WindowTitle(const char * text);

  // Simple drawing functions.
  void ClearWindow(void);
  void DrawRectangle(int x, int y, int w, int h, color_t color);
  void DrawSRectangle(box_t rect, color_t color);
  void DrawBox(int x, int y, int w, int h, color_t color);
  void DrawSBox(box_t rect, color_t color);
  void DrawOutlineBox(box_t rect, color_t fg, color_t bg);
  void DrawDisplayTexture(displayText_t dTexture);
  void CenterText(box_t box, char * text, color_t fg, color_t bg);
  void CenterSurface(box_t box, SDL_Surface *s);
  void UpdateGUI(void);

  // Tests
  void DrawDisplayTexts(void);
  void DrawPreviewBorder(int x, int y, int tw, int th, bool_t active);
  bool_t IsSameBox(box_t a, box_t b);
  bool_t IsInsideBox(point_t point, box_t box);

  // Line, text and data display
  SDL_Texture * CreateText(char * thisText, color_t fg, color_t bg);
  void WriteCommand(int index, color_t fg, color_t bg);
  void WriteCommandToTexture(displayText_t *target, int index, color_t fg, color_t bg);
  void WriteBool(bool_t value, int row, int col, int width);
  void WriteBoolToTexture(displayText_t *target, bool_t value, int row, int col, int width);
  void WriteInt(int value, int row, int col, int width);
  void WriteIntToTexture(displayText_t * target, int value, int row, int col, int width);
  void WriteFloat(float value, int row, int col, int width, int precision);
  void WriteFloatToTexture(displayText_t *target, float value, int row, int col, int width, int precision);
  void WriteString(const char *text, int line, int col, int width);
  void WriteStringToTexture(displayText_t *target, const char *text, int line, int col, int width);
  void WriteLine(char * thisText, int line, int col, color_t fg, color_t bg);
  void WriteLineToTexture(displayText_t *target, char * thisText, int line, int col, color_t fg, color_t bg);


  int GetPixelofColumn(int col);
  box_t GetCommandBox(int command);

  // Widgety things
  void DrawConfirmationBox(box_t *yesBox, box_t *noBox, bool_t selected, char *label);
  void DrawTextEntryBox(int item, char * text);
  void DrawEnumSelectBox(int item, int selected, box_t ** targets);

#endif /* ifndef DRAW_H_ */
