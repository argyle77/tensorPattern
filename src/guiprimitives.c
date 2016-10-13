// Simple drawing functions for tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2016_10_12

// Includes
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "guiprimitives.h"
#include "gui.h"
#include "layout.h"

// Outline
void DrawRectangle(int x, int y, int w, int h, color_t color) {
  rectangleRGBA(mwRenderer, x, y, x + w, y + h,
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

// Outline, but with easier passing var.
void DrawSRectangle(SDL_Rect rect, color_t color) {
  DrawRectangle(rect.x, rect.y, rect.w, rect.h, color);
}

// Filled
void DrawBox(int x, int y, int w, int h, color_t color) {
  boxRGBA(mwRenderer, x, y, x + w, y + h,
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

// Filled, but with easier passing var.
void DrawSBox(SDL_Rect rect, color_t color) {
  DrawBox(rect.x, rect.y, rect.w, rect.h, color);
}

void DrawOutlineBox(SDL_Rect rect, color_t fg, color_t bg) {
  DrawSRectangle(rect, fg);
  DrawBox(rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2, bg);
}

void DrawXBox(SDL_Rect rect, color_t fg, color_t bg) {
  DrawOutlineBox(rect, fg, bg);
  //~ DrawSBox(rect, bg);
  lineRGBA(mwRenderer, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, fg.r, fg.g, fg.b, fg.a);
  lineRGBA(mwRenderer, rect.x + rect.w - 1, rect.y, rect.x, rect.y + rect.h - 1, fg.r, fg.g, fg.b, fg.a);
}

void DrawClearWindow(void) {
  //~ SDL_SetRenderDrawColor(mwRenderer, 127, 0, 0, 255);
  SDL_SetRenderDrawColor(mwRenderer, 0, 0, 0, 255);
  SDL_RenderClear(mwRenderer);
  //~ DrawBox(0, 0, GetWidth(), WINDOW_HEIGHT, cBlue);
}


//////////////////////////////////////
// Texts
//////////////////////////////////////

// Creates a text texture
SDL_Texture * CreateTextureText(char * thisText, color_t fg) {
  const SDL_Color fgc = {fg.r, fg.g, fg.b};
  SDL_Surface *textSurface;
  SDL_Texture *textTexture;

  if (strlen(thisText) == 0) return NULL; // Nothing to write.

  // Render the text to a surface.
  textSurface = TTF_RenderText_Blended(screenFont, thisText, fgc);
  if (!textSurface) {
    fprintf(stderr, "Failed to create surface for \"%s\": %s", thisText, TTF_GetError());
    return NULL;
  }

  // Create a texture in video memory from the text surface.
  textTexture = SDL_CreateTextureFromSurface(mwRenderer, textSurface);
  if (!textTexture) {
    fprintf(stderr, "Failed to create texture for \"%s\": %s", thisText, SDL_GetError());
    SDL_FreeSurface(textSurface);  // Free RAM.
    return NULL;
  }

  SDL_FreeSurface(textSurface);  // Free the surface RAM.
  return(textTexture);  // Return the text as a texture.
}

// Writes a line of text to the selected line and column of the
// output window with the selected color.
void CreateTextureLine(displayText_t *target, const char * thisText, int line, int col, color_t fg, color_t bg) {

  // Vars
  SDL_Surface *textSurface;
  SDL_Color fgc = {fg.r, fg.g, fg.b};
  SDL_Color bgc = {bg.r, bg.g, bg.b};

  target->texture = NULL;
  if (strlen(thisText) == 0) return; // Nothing to write.

  // Render the text surface in RAM.
  textSurface = TTF_RenderText_Shaded(screenFont, thisText, fgc, bgc);
  //~ textSurface = TTF_RenderText_Blended(screenFont, thisText, fgc); // EMERGENCY
  if (!textSurface) {
    fprintf(stderr, "Failed to create surface for \"%s\": %s", thisText, TTF_GetError());
    return;
  }

  // Return the texture and target area
  target->texture = SDL_CreateTextureFromSurface(mwRenderer, textSurface);
  if (!target->texture) {
    fprintf(stderr, "Failed to create texture for \"%s\": %s", thisText, SDL_GetError());
    return;
  }

  // Set the position and size of the output text.
  target->targetBox.x = ColToPixel(col);
  target->targetBox.y = line * (DISPLAY_TEXT_HEIGHT);
  target->targetBox.w = textSurface->w;
  target->targetBox.h = DISPLAY_TEXT_HEIGHT;

  // Free the original drawing surface.
  SDL_FreeSurface(textSurface);
}

void CreateTextureBoolean(displayText_t *target, bool_t value, int row, int col, int width) {
  char * text;
  text = malloc((width + 1) * sizeof(char));
  if (!text) return;
  snprintf(text, width + 1, "%*.*s", width, width, value ? "YES" : "NO");
  CreateTextureLine(target, text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
}

void CreateTextureInt(displayText_t * target, int value, int row, int col, int width) {
  char * text;
  text = malloc((width + 1) * sizeof(char));
  if (!text) return;
  snprintf(text, width + 1, "%*i", width, value);
  CreateTextureLine(target, text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
}

void CreateTextureFloat(displayText_t *target, float value, int row, int col, int width, int precision) {
  char *text, *text2;
  text = malloc((width * 10) * sizeof(char));
  if (!text) return;
  text2 = malloc((width + 1) * sizeof(char));
  if (!text2) { free(text); return; }
  snprintf(text, width * 10, "%*.*f", width, precision, value);
  snprintf(text2, width + 1, "%.*s", width, text); // Hard limit the width
  CreateTextureLine(target, text2, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
  free(text2);
}

void CreateTextureString(displayText_t *target, const char *text, int line, int col, int width) {
  char * tempText;
  tempText = malloc((width + 1) * sizeof(char));
  if (!tempText) return;
  snprintf(tempText, width + 1, "%*.*s", width, width, text);
  CreateTextureLine(target, tempText, line, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(tempText);
}


//////////////////////////////////////
// Tests
//////////////////////////////////////

// Test two boxes to see if they are the same.
bool_t IsSameBox(SDL_Rect a, SDL_Rect b) {
  if ((a.x == b.x) && (a.y == b.y) && (a.w == b.w) && (a.h == b.h)) {
    return YES;
  } else {
    return NO;
  }
}

// Test a point to see if its inside a box.
bool_t IsInsideBox(point_t point, SDL_Rect box){
  if ((point.x >= box.x) &&
      (point.y >= box.y) &&
      (point.x <= box.x + box.w) &&
      (point.y <= box.y + box.h))
    return YES;
  else return NO;
}


//////////////////////////////////////
// Utilities
//////////////////////////////////////

void DrawDisplayTexture(displayText_t dTexture) {
  SDL_RenderCopy(mwRenderer, dTexture.texture, NULL, &(dTexture.targetBox));
}

void DrawTexture(SDL_Texture *texture, SDL_Rect target) {
  SDL_RenderCopy(mwRenderer, texture, NULL, &(target));
}

// Centers some text inside a box.
void DrawCenteredText(SDL_Rect box, char * text, color_t fg, color_t bg) {
  SDL_Surface *textS = NULL;
  SDL_Texture *texture = NULL;
  SDL_Rect rect;
  SDL_Color fontColor = {fg.r, fg.g, fg.b};
  SDL_Color fontBGColor = {bg.r, bg.g, bg.b};


  textS = TTF_RenderText_Shaded(screenFont, text, fontColor, fontBGColor);
  if (!textS) {
    fprintf(stderr, "SDL error rendering text \"%s\": %s\n", text, SDL_GetError());
    exit(EXIT_FAILURE);
  }

  texture = SDL_CreateTextureFromSurface(mwRenderer, textS);
  if (!texture) {
    fprintf(stderr, "Unable to create text render texture: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  rect.w = textS->w;
  rect.h = textS->h;
  rect.x = box.x + (box.w / 2) - (rect.w / 2);
  rect.y = box.y + (box.h / 2) - (rect.h / 2);
  SDL_FreeSurface(textS);
  SDL_RenderCopy(mwRenderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

// Centers a surface inside a box.
void DrawCenteredSurface(SDL_Rect box, SDL_Surface *s) {
  SDL_Texture *texture = NULL;
  SDL_Rect rect;

  if (!s) return;

  texture = SDL_CreateTextureFromSurface(mwRenderer, s);
  if (!texture) {
    fprintf(stderr, "Unable to create render texture: %s\n", SDL_GetError());
    return;
  }
  rect.w = s->w;
  rect.h = s->h;
  rect.x = box.x + (box.w / 2) - (rect.w / 2);
  rect.y = box.y + (box.h / 2) - (rect.h / 2);
  SDL_RenderCopy(mwRenderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

// Centers a texture inside a box.  Provides a border if the texture is too
// large.
void DrawCenteredTexture(SDL_Rect box, SDL_Texture *t) {
  SDL_Rect rect;

  if (!t) return;
  SDL_QueryTexture(t, NULL, NULL, &(rect.w), &(rect.h));
  rect.w = min(rect.w, box.w - TEXTURE_BORDER);
  rect.h = min(rect.h, box.h - TEXTURE_BORDER);
  rect.x = box.x + (box.w / 2) - (rect.w / 2);
  rect.y = box.y + (box.h / 2) - (rect.h / 2);
  SDL_RenderCopy(mwRenderer, t, NULL, &rect);
}

void SetBox(SDL_Rect *box, int x, int y, int w, int h) {
  box->x = x;
  box->y = y;
  box->w = w;
  box->h = h;
}

void SetPoint(point_t *point, int x, int y) {
  point->x = x;
  point->y = y;
}
