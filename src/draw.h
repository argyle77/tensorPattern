// Support for drawing things to the Tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2014_12_04

#ifndef DRAW_H_
  #define DRAW_H_

  #include "useful.h"

  //~ #define WINDOW_WIDTH 1024
  //~ #define WINDOW_HEIGHT 768
  #define WINDOW_WIDTH 1126
  #define WINDOW_HEIGHT 900
  #define DISPLAY_TEXT_HEIGHT 14
  #define ROW_COUNT (WINDOW_HEIGHT / DISPLAY_TEXT_HEIGHT)
  #define CHAR_W 8

  // Preview window definitions
  #define PREVIEW_PIXEL_SIZE 12
  #define PREVIEW_BORDER 12
  #define PREVIEW_LIVE_POS_X 1
  #define PREVIEW_LIVE_POS_Y 1
  #define PREVIEW_ALT_POS_X (WINDOW_WIDTH - (TENSOR_WIDTH * PREVIEW_PIXEL_SIZE) - (PREVIEW_BORDER * 2) - 1)
  #define PREVIEW_ALT_POS_Y 1

  // Layout
  #define ROW_O 27
  #define COL_O 0
  #define ROW_MI (ROW_O + 8)
  #define COL_MI COL_O
  #define ROW_CB (ROW_MI + 5)
  #define COL_CB COL_MI
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

  #define PARAMETER_WIDTH 10

  // 32 bit per pixel color type - Named or array access through union.
  typedef union color_t {
    struct {
      unsigned char r, g, b, a;
    };
    unsigned char rgba[4];
  } color_t;

  // Colors
  // Named color definitions - shoulda done this algorithmically.
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
  #define CD_LTGRAY     {.r = 191, .g = 191, .b = 191, .a = 255}
  #define CD_GRAY       {.r = 127, .g = 127, .b = 127, .a = 255}
  #define CD_DKGRAY     {.r = 63,  .g = 63,  .b = 63,  .a = 255}
  #define CD_BLACK      {.r = 0,   .g = 0,   .b = 0,   .a = 255}
  //~ #define CD_DKYELLOW   {.r = 192, .g = 192, .b = 0,   .a = 255}
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

    CE_DKRED, CE_DKORANGE, CE_DKYELLOW, CE_DKCHARTREUSE, CE_DKGREEN, CE_DKAQUA, CE_DKCYAN,
    CE_DKAZURE, CE_DKBLUE, CE_DKVIOLET, CE_DKMAGENTA, CE_DKROSE, CE_BLACK,

    CE_MDRED, CE_MDORANGE, CE_MDYELLOW, CE_MDCHARTREUSE, CE_MDGREEN, CE_MDAQUA, CE_MDCYAN,
    CE_MDAZURE, CE_MDBLUE, CE_MDVIOLET, CE_MDMAGENTA, CE_MDROSE, CE_DKGRAY,

    CE_RED, CE_ORANGE, CE_YELLOW, CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN,
    CE_AZURE, CE_BLUE, CE_VIOLET, CE_MAGENTA, CE_ROSE, CE_GRAY,

    CE_MLRED, CE_MLORANGE, CE_MLYELLOW, CE_MLCHARTREUSE, CE_MLGREEN, CE_MLAQUA, CE_MLCYAN,
    CE_MLAZURE, CE_MLBLUE, CE_MLVIOLET, CE_MLMAGENTA, CE_MLROSE, CE_LTGRAY,

    CE_LTRED, CE_LTORANGE, CE_LTYELLOW, CE_LTCHARTREUSE, CE_LTGREEN, CE_LTAQUA,
    CE_LTCYAN, CE_LTAZURE, CE_LTBLUE, CE_LTVIOLET, CE_LTMAGENTA, CE_LTROSE, CE_WHITE,

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
  extern const namedPalette_t paletteSymGry;

  extern const int colorTitlesCount;

  // Points
  typedef struct point_t {
    int x, y;
  } point_t;

  // A video texture and its target location / size.
  typedef struct displayText_t {
    SDL_Texture * texture;
    SDL_Rect targetBox;
  } displayText_t;

  // Function prototypes
  bool_t InitGui(void);
  void QuitGui(void);
  void UpdateGUI(void);
  void WindowTitle(const char * text);

  // Simple drawing functions.
  void DrawClearWindow(void);
  void DrawRectangle(int x, int y, int w, int h, color_t color);
  void DrawSRectangle(SDL_Rect rect, color_t color);
  void DrawBox(int x, int y, int w, int h, color_t color);
  void DrawSBox(SDL_Rect rect, color_t color);
  void DrawOutlineBox(SDL_Rect rect, color_t fg, color_t bg);
  void DrawXBox(SDL_Rect rect, color_t fg, color_t bg);
  void DrawTexture(SDL_Texture *texture, SDL_Rect target);
  void DrawDisplayTexture(displayText_t dTexture);
  void DrawCenteredText(SDL_Rect box, char * text, color_t fg, color_t bg);
  void DrawCenteredSurface(SDL_Rect box, SDL_Surface *s);
  void DrawCenteredTexture(SDL_Rect box, SDL_Texture *t);


  // Tests
  bool_t IsSameBox(SDL_Rect a, SDL_Rect b);
  bool_t IsInsideBox(point_t point, SDL_Rect box);

  // Line, text and data display
  SDL_Texture * CreateTextureText(char * thisText, color_t fg);
  void CreateTextureCommand(displayText_t *target, int index, color_t fg, color_t bg);
  void CreateTextureBoolean(displayText_t *target, bool_t value, int row, int col, int width);
  void CreateTextureInt(displayText_t * target, int value, int row, int col, int width);
  void CreateTextureFloat(displayText_t *target, float value, int row, int col, int width, int precision);
  void CreateTextureString(displayText_t *target, const char *text, int line, int col, int width);
  void CreateTextureLine(displayText_t *target, char * thisText, int line, int col, color_t fg, color_t bg);


  int GetPixelofColumn(int col);
  SDL_Rect GetBoxofCommand(int command);
  SDL_Rect GetBoxofLine(int line, int colStart, int colEnd);

  // Complex drawing functions and widgety things
  void DrawDisplayTexts(int selected);
  void DrawPreviewBorder(int x, int y, int tw, int th, bool_t active);
  void DrawConfirmationBox(SDL_Rect *yesBox, SDL_Rect *noBox, bool_t selected, char *label);
  void DrawTextEntryBox(int item, char * text);
  void DrawEnumSelectBox(int set, int item, int selected, SDL_Rect ** targets);
  void DrawColorSelectBox(int item, int selected, SDL_Rect ** targets);
#endif /* ifndef DRAW_H_ */
