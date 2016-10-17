// Support for drawing things to the Tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2014_12_04

#ifndef GUIDRAW_H_
  #define GUIDRAW_H_

  #include "useful.h"
  #include "layout.h"
  #include "guiprimitives.h"
  #include "elements.h"

  // For knowing the number of colorTitles outside of here?
  extern const int colorTitlesCount;
  
  // Return codes for detecting if mouse is over a preview window
  typedef enum overPreview_e {
    OP_NO = -1,
    OP_LIVE = 0,
    OP_ALT,
    OP_COUNT // Last
  } overPreview_e;
  
  // Return codes for over preview detection
  typedef enum highLight_e {
    HL_INVALID = -1,
    HL_NO = 0,
    HL_SELECTED,
    HL_HOVER,
    HL_COUNT // Last
  } highLight_e;

  // Function prototypes

  // Tests
  overPreview_e OverPreviewBorder(point_t mouse);

  // Line, text and data display
  void DrawAndCacheString(int *cachePosition, const char *value, int row, int col, color_t fg, color_t bg);
  void DrawAndCache(int *cachePosition, elementType_e type, void *value, int row, int col, int width);
  void CreateTextureCommand(displayText_t *target, int index, color_t fg, color_t bg);
  void CreateTextureLine(displayText_t *target, const char * thisText, int line, int col, color_t fg, color_t bg);

  SDL_Rect GetBoxofCommand(int command);
  SDL_Rect GetBoxofLine(int line, int colStart, int colEnd);

  // Complex drawing functions and widgety things
  void DrawDisplayTexts(int selected);
  void DrawPreviewBorder(point_t location, highLight_e highLight);
  void DrawConfirmationBox(SDL_Rect *yesBox, SDL_Rect *noBox, bool_t selected, char *label);
  void DrawTextEntryBox(int item, char * text);
  void DrawEnumSelectBox(int set, int item, int selected, SDL_Rect ** targets);
  void DrawColorSelectBox(int item, int selected, SDL_Rect ** targets);
  void DrawPreview(point_t xyOffset, unsigned char *buffer);
  
#endif /* ifndef GUIDRAW_H_ */
