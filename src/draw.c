// Support for drawing things to the Tensor gui.
// Makes use of SDL2.
// Joshua Krueger
// 2014_12_04

// Includes
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include "useful.h"
#include "draw.h"
#include "elements.h"

// Defines
#define DISPLAY_FONT_SIZE 14
#define FONT_FILE "font.ttf"
#define DISPLAY_TEXT_HEIGHT 14

// Layout
#define CHAR_W 8
#define CHAR_H 19
#define ACOL 0
#define BCOL (ACOL + 27 * CHAR_W)
#define CCOL (BCOL + 12 * CHAR_W)
#define DCOL (CCOL + 39 * CHAR_W)
#define ECOL (DCOL + 13 * CHAR_W)
#define FCOL (ECOL + 26 * CHAR_W)
#define ENUM_BORDER 4

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

// Named color palettes
const namedPalette_t paletteRGB = { (color_e[]) { CE_RED, CE_GREEN, CE_BLUE }, 3 };
const namedPalette_t paletteCMY = { (color_e[]) {CE_CYAN, CE_MAGENTA, CE_YELLOW }, 3};
const namedPalette_t paletteSec = { (color_e[]) { CE_RED, CE_YELLOW, CE_GREEN,
  CE_CYAN, CE_BLUE, CE_MAGENTA }, 6 };
const namedPalette_t paletteTer = { (color_e[]) { CE_RED, CE_ORANGE, CE_YELLOW,
  CE_CHARTREUSE, CE_GREEN, CE_AQUA, CE_CYAN, CE_AZURE, CE_BLUE, CE_VIOLET,
  CE_MAGENTA, CE_ROSE }, 12 };
const namedPalette_t paletteGry = { (color_e[]) { CE_WHITE, CE_LTGRAY, CE_GRAY,
  CE_DKGRAY }, 4 };

// Global elements
TTF_Font *screenFont;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;
const int colToPixel[] = {ACOL, BCOL, CCOL, DCOL, ECOL, FCOL, WINDOW_WIDTH};
#define MAX_COL (sizeof(colToPixel) / sizeof(const int))

// Initialization
bool_t InitGui(void) {

  // Initialize SDL.
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    return FALSE;
  }

  // Register the SDL cleanup function.
  atexit(SDL_Quit);

  // Initialize the window and renderer.
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &mainWindow, &mwRenderer);
  if ((!mainWindow) || (!mwRenderer)) {
    fprintf(stderr, "Unable to create main window or renderer: %s\n", SDL_GetError());
    return FALSE;
  }

  // Window scaling hints:
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(mwRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);

  // Load the screen font.  Should be fixed width.
  if (TTF_Init() < 0) {
    fprintf(stderr, "Couldn't initialize SDL TTF: %s\n", TTF_GetError());
    return FALSE;
  }

  screenFont = TTF_OpenFont(FONT_FILE, DISPLAY_FONT_SIZE);
  if(!screenFont) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return FALSE;
  }

  return TRUE;
}

void QuitGui(void) {
  // Close up the TTF stuff.
  TTF_CloseFont(screenFont);
  screenFont = NULL; // to be safe...?
  TTF_Quit();
}

// Window title
void WindowTitle(const char * text) {
  SDL_SetWindowTitle(mainWindow, text);
}

// Outline
void DrawRectangle(int x, int y, int w, int h, color_t color) {
  rectangleRGBA(mwRenderer, x, y, x + w, y + h,
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

// Outline, but with easier passing var.
void DrawSRectangle(box_t rect, color_t color) {
  DrawRectangle(rect.x, rect.y, rect.w, rect.h, color);
}

// Filled
void DrawBox(int x, int y, int w, int h, color_t color) {
  boxRGBA(mwRenderer, x, y, x + w, y + h,
    (Uint8) color.r, (Uint8) color.g, (Uint8) color.b, (Uint8) color.a);
}

// Filled, but with easier passing var.
void DrawSBox(box_t rect, color_t color) {
  DrawBox(rect.x, rect.y, rect.w, rect.h, color);
}

void DrawOutlineBox(box_t rect, color_t fg, color_t bg) {
  DrawSRectangle(rect, fg);
  DrawBox(rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2, bg);
}

// Writes a line of text to the selected line and column of the
// output window with the selected color.
void WriteLine(char * thisText, int line, int col, color_t fg, color_t bg) {

  // Vars
  SDL_Texture *textTexture;
  SDL_Rect rect;

  if (strlen(thisText) == 0) return; // Nothing to write.

  // Create the text texture in video ram.
  textTexture = CreateText(thisText, fg, bg);
  if (!textTexture) return;

  // Set the position of the output text.
  rect.x = colToPixel[col];
  rect.y = line * (DISPLAY_TEXT_HEIGHT);

  // Set the width and height of the output text.
  SDL_QueryTexture(textTexture, NULL, NULL, &(rect.w), NULL);
  // Beh.  Text rendered at 19 pixel high, but gets squished to 14 here.
  rect.h = DISPLAY_TEXT_HEIGHT;

  // Copy it to the rendering context.
  SDL_RenderCopy(mwRenderer, textTexture, NULL, &rect);
  SDL_DestroyTexture(textTexture);  // Free video memory
}

// Writes a line of text to the selected line and column of the
// output window with the selected color.
void WriteLineToTexture(displayText_t *target, char * thisText, int line, int col, color_t fg, color_t bg) {

  // Vars
  SDL_Surface *textSurface;
  SDL_Color fgc = {fg.r, fg.g, fg.b};
  SDL_Color bgc = {bg.r, bg.g, bg.b};

  target->texture = NULL;
  if (strlen(thisText) == 0) return; // Nothing to write.

  // Render the text surface in RAM.
  textSurface = TTF_RenderText_Shaded(screenFont, thisText, fgc, bgc);
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
  target->targetBox.x = colToPixel[col];
  target->targetBox.y = line * (DISPLAY_TEXT_HEIGHT);
  target->targetBox.w = textSurface->w;
  target->targetBox.h = DISPLAY_TEXT_HEIGHT;

  // Free the original drawing surface.
  SDL_FreeSurface(textSurface);
}

// Creates a text texture
SDL_Texture * CreateText(char * thisText, color_t fg, color_t bg) {
  const SDL_Color fgc = {fg.r, fg.g, fg.b};
  const SDL_Color bgc = {bg.r, bg.g, bg.b, 0};
  SDL_Surface *textSurface;
  SDL_Texture *textTexture;

  if (strlen(thisText) == 0) return NULL; // Nothing to write.

  // Render the text to a surface.
  textSurface = TTF_RenderText_Shaded(screenFont, thisText, fgc, bgc);
  //~ textSurface = TTF_RenderText_Blended(screenFont, thisText, fontColor);
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

void UpdateGUI(void) {
  SDL_RenderPresent(mwRenderer);
}

void ClearWindow(void) {
  SDL_SetRenderDrawColor(mwRenderer, 0, 0, 0, 255);
  SDL_RenderClear(mwRenderer);
}

int GetPixelofColumn(int col) {
  if (col >= MAX_COL) {
    fprintf(stderr, "Selected column %i exceeds maximum %i!\n", col, (int) MAX_COL - 1);
    return colToPixel[MAX_COL - 1];
  } else if (col < 0) {
    fprintf(stderr, "Selected column %i less than 0?\n", col);
    return colToPixel[0];
  }

  return colToPixel[col];
}


// Return the rectangular region surrounding a displayed command.
box_t GetCommandBox(int command) {
  box_t box = {0, 0, 0, 0};

  if (command < 0 || command >= displayCommandCount) {
    return box;
  }

  box.x = GetPixelofColumn(displayCommand[command].col);
  box.y = displayCommand[command].line * DISPLAY_TEXT_HEIGHT;
  box.w = GetPixelofColumn(displayCommand[command].col + 2) - box.x;
  box.h = (displayCommand[command].line + 1) * DISPLAY_TEXT_HEIGHT - box.y;
  return box;
}

void WriteBool(bool_t value, int row, int col, int width) {
  char text[100];
  if (value) {
    snprintf(text, sizeof(text), "%*.*s", width, width, "YES");
  } else {
    snprintf(text, sizeof(text), "%*.*s", width, width, "NO");
  }
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  //~ WriteLine(text, row, col, DISPLAY_COLOR_PARMS, cOrange);
}

void WriteBoolToTexture(displayText_t *target, bool_t value, int row, int col, int width) {
  char * text;
  text = malloc((width + 1) * sizeof(char));
  if (!text) return;
  snprintf(text, width + 1, "%*.*s", width, width, value ? "YES" : "NO");
  WriteLineToTexture(target, text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
}

void WriteInt(int value, int row, int col, int width) {
  char text[100];
  snprintf(text, sizeof(text), "%*i", width, value);
  WriteLine(text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  //~ WriteLine(text, row, col, DISPLAY_COLOR_PARMS, cYellow);
}

void WriteIntToTexture(displayText_t * target, int value, int row, int col, int width) {
  char * text;
  text = malloc((width + 1) * sizeof(char));
  if (!text) return;
  snprintf(text, width + 1, "%*i", width, value);
  WriteLineToTexture(target, text, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
}

void WriteFloat(float value, int row, int col, int width, int precision) {
  char text[100];
  char text2[100];
  snprintf(text, sizeof(text), "%*.*f", width, precision, value);
  snprintf(text2, sizeof(text2), "%.*s", width, text); // Hard limit the width
  WriteLine(text2, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  //~ WriteLine(text2, row, col, DISPLAY_COLOR_PARMS, cAqua);
}

void WriteFloatToTexture(displayText_t *target, float value, int row, int col, int width, int precision) {
  char *text, *text2;
  text = malloc((width * 10) * sizeof(char));
  if (!text) return;
  text2 = malloc((width + 1) * sizeof(char));
  if (!text2) { free(text); return; }
  snprintf(text, width * 10, "%*.*f", width, precision, value);
  snprintf(text2, width + 1, "%.*s", width, text); // Hard limit the width
  WriteLineToTexture(target, text2, row, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(text);
  free(text2);
}

void WriteStringToTexture(displayText_t *target, const char *text, int line, int col, int width) {
  char * tempText;
  tempText = malloc((width + 1) * sizeof(char));
  if (!tempText) return;
  snprintf(tempText, width + 1, "%*.*s", width, width, text);
  WriteLineToTexture(target, tempText, line, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  free(tempText);
}

void WriteString(const char *text, int line, int col, int width) {
  char temp[200];
  snprintf(temp, sizeof(temp), "%*.*s", width, width, text);
  WriteLine(temp, line, col, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
  //~ WriteLine(temp, line, col, DISPLAY_COLOR_PARMS, cRose);
}

#define CONFBOX_BORDER_WIDTH 10
// Confirmation box for dangerous actions.
void DrawConfirmationBox(box_t *yesBox, box_t *noBox, bool_t selected, char *label) {
  SDL_Surface *t1 = NULL, *t2 = NULL, *t3 = NULL;
  SDL_Texture *texture = NULL;
  SDL_Color fg = {DISPLAY_COLOR_TEXTS.r, DISPLAY_COLOR_TEXTS.g, DISPLAY_COLOR_TEXTS.b};
  SDL_Color bg = {DISPLAY_COLOR_TEXTSBG.r, DISPLAY_COLOR_TEXTSBG.g, DISPLAY_COLOR_TEXTSBG.b};
  SDL_Color fghl = {DISPLAY_COLOR_TEXTS_HL.r, DISPLAY_COLOR_TEXTS_HL.g, DISPLAY_COLOR_TEXTS_HL.b};
  SDL_Color bghl = {DISPLAY_COLOR_TEXTSBG_HL.r, DISPLAY_COLOR_TEXTSBG_HL.g, DISPLAY_COLOR_TEXTSBG_HL.b};
  SDL_Rect rect;
  int tHeight, boxHeight, boxWidth, boxX, boxY;

  // Create the texts.
  t1 = TTF_RenderText_Shaded(screenFont, label, fg, bg);
  if (!t1) {
    fprintf(stderr, "SDL error rendering text \"%s\": %s\n", label, SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (selected == YES) {
    t2 = TTF_RenderText_Shaded(screenFont, "Yes", fghl, bghl);
  } else {
    t2 = TTF_RenderText_Shaded(screenFont, "Yes", fg, bg);
  }
  if (!t2) {
    fprintf(stderr, "SDL error rendering text \"%s\": %s\n", "Yes", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (selected == NO) {
    t3 = TTF_RenderText_Shaded(screenFont, "No", fghl, bghl);
  } else {
    t3 = TTF_RenderText_Shaded(screenFont, "No", fg, bg);
  }
  if (!t3) {
    fprintf(stderr, "SDL error rendering text \"%s\": %s\n", "No", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Calculations of the box's positions and sizes.
  tHeight = max(max(t1->h, t2->h), t3->h);
  boxHeight = 3 * tHeight + 4 * CONFBOX_BORDER_WIDTH;
  boxWidth = max(4 * CONFBOX_BORDER_WIDTH + max(t2->w, t3->w) * 2, 2 * CONFBOX_BORDER_WIDTH + t1->w);
  boxX = (WINDOW_WIDTH / 2) - (boxWidth / 2);
  boxY = (WINDOW_HEIGHT / 2) - (boxHeight / 2);
  //~ printf("tHeight: %i, %i, %i, tWidth: %i, %i, %i\n", t1->h, t2->h, t3->h, t1->w, t2->w, t3->w);

  // Get the yes and no box areas to pass back to the caller.
  yesBox->x = boxX;
  yesBox->y = boxY + boxHeight / 2 - 1;
  yesBox->w = boxWidth / 2 + 1;
  yesBox->h = boxHeight / 2 + 1;
  noBox->x = boxX + boxWidth / 2;
  noBox->y = boxY + boxHeight / 2 - 1;
  noBox->w = boxWidth / 2 + 1;
  noBox->h = boxHeight / 2 + 1;

  // Draw the underlying box areas.
  DrawBox(boxX, boxY, boxWidth + 1, boxHeight + 1, DISPLAY_COLOR_TEXTSBG);
  DrawRectangle(boxX, boxY, boxWidth + 1, boxHeight / 2 + 1, DISPLAY_COLOR_TEXTS);
  if (selected == YES) {
    DrawSBox(*yesBox, DISPLAY_COLOR_TEXTSBG_HL);
  } else if (selected == NO) {
    DrawSBox(*noBox, DISPLAY_COLOR_TEXTSBG_HL);
  }
  DrawSRectangle(*yesBox, DISPLAY_COLOR_TEXTS);
  DrawSRectangle(*noBox, DISPLAY_COLOR_TEXTS);

  // Draw the texts.
  texture = SDL_CreateTextureFromSurface(mwRenderer, t1);
  if (!texture) {
    fprintf(stderr, "Unable to create text render texture: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  rect.x = boxX + CONFBOX_BORDER_WIDTH;
  rect.y = boxY + boxHeight / 4 - tHeight / 2;
  rect.w = t1->w;
  rect.h = t1->h;
  SDL_FreeSurface(t1);
  SDL_RenderCopy(mwRenderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
  //~ DrawSBox(mwRenderer, rect, cRed);

  texture = SDL_CreateTextureFromSurface(mwRenderer, t2);
  if (!texture) {
    fprintf(stderr, "Unable to create text render texture: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  rect.x = boxX + (boxWidth / 4 - t2->w / 2);
  rect.y = boxY + (3 * boxHeight / 4) - tHeight / 2;
  rect.w = t2->w;
  rect.h = t2->h;
  SDL_FreeSurface(t2);
  SDL_RenderCopy(mwRenderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);

  texture = SDL_CreateTextureFromSurface(mwRenderer, t3);
  if (!texture) {
    fprintf(stderr, "Unable to create text render texture: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  rect.x = boxX + (3 * boxWidth / 4 - t3->w / 2);
  rect.y = boxY + (3 * boxHeight / 4) - tHeight / 2;
  rect.w = t3->w;
  rect.h = t3->h;
  SDL_FreeSurface(t3);
  SDL_RenderCopy(mwRenderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

// Centers some text inside a box.
void CenterText(box_t box, char * text, color_t fg, color_t bg) {
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
void CenterSurface(box_t box, SDL_Surface *s) {
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

// Display box for entering keyboard text into a parameter.
void DrawTextEntryBox(int item, char * text) {

  // Vars
  box_t box;
  SDL_Rect dstRect;
  SDL_Color fontColor = {DISPLAY_COLOR_PARMS.r, DISPLAY_COLOR_PARMS.g, DISPLAY_COLOR_PARMS.b};
  SDL_Color fontBGColor = {DISPLAY_COLOR_PARMSBG.r, DISPLAY_COLOR_PARMSBG.g, DISPLAY_COLOR_PARMSBG.b};
  SDL_Surface *textS = NULL;
  SDL_Texture *texture = NULL;

  // Get the dimensions.
  box.x = GetPixelofColumn(displayCommand[item].col + 1);
  box.y = displayCommand[item].line * DISPLAY_TEXT_HEIGHT;
  //~ box.w = (GetPixelofColumn(displayCommand[item].col + 2) - box.x);
  box.w = (PARAMETER_WIDTH * CHAR_W);
  box.h = ((displayCommand[item].line + 1) * DISPLAY_TEXT_HEIGHT - box.y + 1);

  // Draw the outline box.
  DrawSRectangle(box, DISPLAY_COLOR_PARMS);
  DrawBox(box.x + 1, box.y + 1, box.w - 2, box.h - 2, cBlack);

  // Write the text.  If there is no text, skip this part.
  if (strlen(text) == 0) return;

  // Render the text surface from the font.
  textS = TTF_RenderText_Shaded(screenFont, text, fontColor, fontBGColor);
  if (!textS) {
    fprintf(stderr, "SDL error rendering text \"%s\": %s\n", text, SDL_GetError());
    return;
  }

  // Copy the RAM text surface into a video memory texture.
  texture = SDL_CreateTextureFromSurface(mwRenderer, textS);
  if (!texture) {
    fprintf(stderr, "Unable to create text render texture: %s\n", SDL_GetError());
    SDL_FreeSurface(textS);
    return;
  }

  // Calculate the position and size of the text to render to the output.
  dstRect.h = box.h - 3;
  dstRect.w = box.w - 3;
  dstRect.x = (box.x + dstRect.w) - min(textS->w, dstRect.w - 1);
  dstRect.y = box.y + 1;
  dstRect.w = min(dstRect.w, textS->w);

  // Render the texture to the output window.
  SDL_RenderCopy(mwRenderer, texture, NULL, &dstRect);

  // Cleanup
  SDL_FreeSurface(textS);
  SDL_DestroyTexture(texture);
}


// Enumeration Selection Boxes
void DrawEnumSelectBox(int item, int selected, box_t ** targets) {

  // Vars
  color_t fg = DISPLAY_COLOR_PARMS;
  color_t bg = DISPLAY_COLOR_PARMSBG;
  color_t fgh = DISPLAY_COLOR_TEXTS_HL;
  color_t bgh = DISPLAY_COLOR_TEXTSBG_HL;
  point_t boxOrigin;  // All boxes are relative to this.
  int i, h, boxCount;
  int boxH, boxW;
  box_t wholeBox, labelBox;
  char text[11];

  // Get the number of choices in the selector.
  boxCount = enumerations[patternElements[displayCommand[item].dataSource].etype].size;

  // Calculate the entire box width and position.
  h = CHAR_H + (ENUM_BORDER * 2);
  boxW = max(strlen(displayCommand[item].text) * CHAR_W + (ENUM_BORDER * 2), 2 * (CHAR_W * PARAMETER_WIDTH + (ENUM_BORDER * 2)));
  boxH = h * ((boxCount / 2) + (boxCount % 2) + 1);
  boxOrigin.x = (WINDOW_WIDTH / 2) - boxW / 2;
  boxOrigin.y = (WINDOW_HEIGHT / 2) - boxH / 2;
  wholeBox.x = boxOrigin.x;
  wholeBox.y = boxOrigin.y;
  wholeBox.w = boxW;
  wholeBox.h = boxH;

  // Draw the outline of the whole box.
  DrawOutlineBox(wholeBox, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);

  // Draw the label.
  labelBox.x = boxOrigin.x;
  labelBox.y = boxOrigin.y;
  labelBox.w = boxW;
  labelBox.h = h;
  DrawSRectangle(labelBox, DISPLAY_COLOR_PARMS);
  CenterText(labelBox, displayCommand[item].text, fg, bg);

  // Initialize the boxes if we haven't.
  if (! *targets) {

    // Allocate the space for the target boxes.
    (*targets) = malloc(sizeof(**targets) * boxCount);
    if (! (*targets)) {
      fprintf(stderr, "Couldn't allocate some important memory.  Fuck it.\n");
      exit(EXIT_FAILURE);
    }

    // Calculate the box positions.
    for (i = 0; i < boxCount; i++) {
      (*targets)[i].x = boxOrigin.x + (wholeBox.w / 2) * (i % 2);
      (*targets)[i].y = boxOrigin.y + (h - 1) + (h * (i / 2));
      (*targets)[i].w = wholeBox.w / 2 + ((i+1) % 2);
      (*targets)[i].h = h + 1;
    }
  }

  // Draw the boxes.
  for (i = 0; i < boxCount; i++) {
    DrawSRectangle((*targets)[i], DISPLAY_COLOR_PARMS);
    snprintf(text, sizeof(text), "%s", enumerations[patternElements[displayCommand[item].dataSource].etype].texts[i]);
    if (i == selected) {
      DrawOutlineBox((*targets)[i], fg, bgh);
      CenterText((*targets)[i], text, fgh, bgh);
    } else {
      CenterText((*targets)[i], text, fg, bg);
    }
  }
}

// Write the static menu information to the display.
void DrawDisplayTexts(void) {

  // Vars
  int i;
  char text[100];
  static displayText_t *displayTexts = NULL;
  static int textCount = 0;
  displayText_t textTemp;

  // If we haven't done so yet, we'll render all of the display texts to be
  // cached in video RAM.
  if (!textCount) {

    // Render the commands
    for (i = 0; i < displayCommandCount; i++) {
      WriteCommandToTexture(&textTemp, i, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the headers.
    for (i = 0; i < headerTextCount; i++) {
      // We precalculate the width purely to allow the background of the header
      // to span the whole box.  This way we don't have to draw the background
      // separately.  It is rendered as part of the text.
      textTemp.targetBox.x = GetPixelofColumn(headerText[i].col);
      textTemp.targetBox.w = GetPixelofColumn(headerText[i].col + 1) - textTemp.targetBox.x + ((CHAR_W * PARAMETER_WIDTH) - CHAR_W);
      snprintf(text, sizeof(text), " %-*s", textTemp.targetBox.w / CHAR_W, headerText[i].text);
      WriteLineToTexture(&textTemp, text, headerText[i].line, headerText[i].col, DISPLAY_COLOR_TITLES, DISPLAY_COLOR_TITLESBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the labels
    for (i = 0; i < labelCount; i++) {
      snprintf(text, sizeof(text), " %s", labelText[i].text);
      WriteLineToTexture(&textTemp, text, labelText[i].line, labelText[i].col, DISPLAY_COLOR_LABEL, DISPLAY_COLOR_LABELBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the texts.
    for (i = 0; i < displayTextCount; i++) {
      snprintf(text, sizeof(text), " %s", displayText[i].text);
      WriteLineToTexture(&textTemp, text, displayText[i].line, displayText[i].col, DISPLAY_COLOR_INFO, DISPLAY_COLOR_INFOBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }
  }

  // Draw the textures to the rendering surface.
  for (i = 0; i < textCount; i++) {
    SDL_RenderCopy(mwRenderer, displayTexts[i].texture, NULL, (SDL_Rect *) &displayTexts[i].targetBox);
  }
}

void WriteCommand(int index, color_t fg, color_t bg) {
  char text[100] = "";
  char *ind = text;
  char * const end = text + sizeof(text);
  int i;
  bool_t modified = NO;

  // Verify the index.
  if (index < 0 || index >= displayCommandCount) return;

  // Exclude any command with invalid line position.
  if (displayCommand[index].line == INVALID) return;

  // Add the text from the displayCommand structure.
  ind += snprintf(ind, end - ind, "  %s", displayCommand[index].text);

  // Handle the modifier keys - worked well when we had only one modifier key
  // possible for each command, but now... Strategy: Show the modifiers from
  // the first command in the list that doesn't have KMOD_NONE, and assume it
  // will be the same for all three.
  for (i = 0; i < MOUSE_COUNT; i++) {
    if (displayCommand[index].commands[i].mod != KMOD_NONE) {
      modified = YES;
      ind += snprintf(ind, end - ind, " %s%s%s",
        displayCommand[index].commands[i].mod & KMOD_CTRL ? "<c>" : "",
        displayCommand[index].commands[i].mod & KMOD_SHIFT ? "<s>" : "",
        displayCommand[index].commands[i].mod & KMOD_ALT ? "<a>" : "");
      break;  // Found the first, skip the rest.
    }
  }

  if (modified) ind += snprintf(ind, end - ind, " ");

  // Handle the key selection. TODO: Fix this mess.
  if (modified) {
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  } else {
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  }

  // Write it.
  WriteLine(text, displayCommand[index].line, displayCommand[index].col, fg, bg);
}


void WriteCommandToTexture(displayText_t *target, int index, color_t fg, color_t bg) {
  char text[100] = "";
  char *ind = text;
  char * const end = text + sizeof(text);
  int i;
  bool_t modified = NO;

  // Verify the index.
  if (index < 0 || index >= displayCommandCount) return;

  // Exclude any command with invalid line position.
  if (displayCommand[index].line == INVALID) return;

  // Add the text from the displayCommand structure.
  ind += snprintf(ind, end - ind, "  %s", displayCommand[index].text);

  // Handle the modifier keys - worked well when we had only one modifier key
  // possible for each command, but now... Strategy: Show the modifiers from
  // the first command in the list that doesn't have KMOD_NONE, and assume it
  // will be the same for all three.
  for (i = 0; i < MOUSE_COUNT; i++) {
    if (displayCommand[index].commands[i].mod != KMOD_NONE) {
      modified = YES;
      ind += snprintf(ind, end - ind, " %s%s%s",
        displayCommand[index].commands[i].mod & KMOD_CTRL ? "<c>" : "",
        displayCommand[index].commands[i].mod & KMOD_SHIFT ? "<s>" : "",
        displayCommand[index].commands[i].mod & KMOD_ALT ? "<a>" : "");
      break;  // Found the first, skip the rest.
    }
  }

  if (modified) ind += snprintf(ind, end - ind, " ");

  // Handle the key selection. TODO: Fix this mess.
  if (modified) {
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  } else {
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_CLICK].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  }

  // Write it.
  WriteLineToTexture(target, text, displayCommand[index].line, displayCommand[index].col, fg, bg);
}


// Test two boxes to see if they are the same.
bool_t IsSameBox(box_t a, box_t b) {
  if ((a.x == b.x) && (a.y == b.y) && (a.w == b.w) && (a.h == b.h)) {
    return YES;
  } else {
    return NO;
  }
}

// Test a point to see if its inside a box.
bool_t IsInsideBox(point_t point, box_t box){
  if ((point.x >= box.x) &&
      (point.y >= box.y) &&
      (point.x <= box.x + box.w) &&
      (point.y <= box.y + box.h))
    return YES;
  else return NO;
}

// Draw the borders around the preview output.  We can switch between portrait
// and landscape, so the preview space is a square box that would accomodate
// either orientation.
void DrawPreviewBorder(int x, int y, int tw, int th, bool_t active) {

  // Vars
  int w, h;
  int maxDim;
  int i;
  color_t highlight = {{0, 127, 0, 255}};

  // Get the outer border dimensions.
  w = (tw * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);
  h = (th * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);

  // Preview space dimension.
  maxDim = max(w, h);

  // Erase the old preview.
  DrawBox(x, y, maxDim, maxDim, cBlack);
  DrawRectangle(x, y, maxDim, maxDim, cWhite);
  if (active) {
    DrawBox(x + 1, y + 1, maxDim - 2, maxDim - 2, highlight);
  } else {
    DrawBox(x + 1, y + 1, maxDim - 2, maxDim - 2, cBlack);
  }

  // Ajust x and y to center the preview.
  x = x + (maxDim - w) / 2;
  y = y + (maxDim - h) / 2;

  // Draw the new outer border.
  DrawRectangle(x, y, w, h, cWhite);
  DrawBox(x + 1, y + 1, w - 2, h - 2, cBlack);

  // Get the inner border dimensions.
  w = (tw * PREVIEW_PIXEL_SIZE) + 1;
  h = (th * PREVIEW_PIXEL_SIZE) + 1;

  if (tw < th) {
    // Portrait - Draw horizontal panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(x + PREVIEW_BORDER - 1, y + i * (h / 3) + PREVIEW_BORDER - 1, w, (h / 3) + 1, cGray);
    }
  } else {
    // Landscape - Draw vertical panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(x + i * (w / 3) + PREVIEW_BORDER - 1, y + PREVIEW_BORDER - 1, (w / 3) + 1, h, cGray);
    }
  }
}

void DrawDisplayTexture(displayText_t dTexture) {
  SDL_RenderCopy(mwRenderer, dTexture.texture, NULL, (SDL_Rect *) &(dTexture.targetBox));
}

