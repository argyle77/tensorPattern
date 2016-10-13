// Simple drawing functions for tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2016_10_12

#ifndef GUIPRIMITIVES_H_
  #define GUIPRIMITIVES_H_
  
  // Includes
  #include <SDL.h>
  #include "useful.h"
  #include "colors.h"
  #include "draw.h"
  
  // Typedefs
  
  // A video texture and its target location / size.
  typedef struct displayText_t {
    SDL_Texture * texture;
    SDL_Rect targetBox;
  } displayText_t;
  

  // Prototypes
  // Drawing functions
  void DrawRectangle(int x, int y, int w, int h, color_t color);
  void DrawSRectangle(SDL_Rect rect, color_t color);
  void DrawBox(int x, int y, int w, int h, color_t color);
  void DrawSBox(SDL_Rect rect, color_t color);
  void DrawOutlineBox(SDL_Rect rect, color_t fg, color_t bg);
  void DrawXBox(SDL_Rect rect, color_t fg, color_t bg);
  void DrawClearWindow(void);
  
  // Texts
  SDL_Texture * CreateTextureText(char * thisText, color_t fg);
  void CreateTextureBoolean(displayText_t *target, bool_t value, int row, int col, int width);
  void CreateTextureInt(displayText_t * target, int value, int row, int col, int width);
  void CreateTextureFloat(displayText_t *target, float value, int row, int col, int width, int precision);
  void CreateTextureString(displayText_t *target, const char *text, int line, int col, int width);
  
  // Tests
  bool_t IsSameBox(SDL_Rect a, SDL_Rect b);
  bool_t IsInsideBox(point_t point, SDL_Rect box);
  
  // Utilities
  void SetBox(SDL_Rect *box, int x, int y, int w, int h);
  void SetPoint(point_t *point, int x, int y);
  void DrawTexture(SDL_Texture *texture, SDL_Rect target);
  void DrawCenteredTexture(SDL_Rect box, SDL_Texture *t);
  void DrawDisplayTexture(displayText_t dTexture);
  void DrawCenteredSurface(SDL_Rect box, SDL_Surface *s);
  void DrawCenteredText(SDL_Rect box, char * text, color_t fg, color_t bg);

#endif /* GUIPRIMITIVES_H_ */

