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
#include "gui.h"
#include "colors.h"
#include "layout.h"
#include "machine.h"

#define CHAR_H 19
#define ENUM_BORDER 4



// Return the rectangular region surrounding a displayed command.
SDL_Rect GetBoxofCommand(int command) {
  SDL_Rect box = {0, 0, 0, 0};

  if (command < 0 || command >= displayCommandCount) {
    return box;
  }

  box.x = ColToPixel(displayCommand[command].col);
  box.y = displayCommand[command].line * DISPLAY_TEXT_HEIGHT;
  box.w = ColToPixel(displayCommand[command].col + 2) - box.x;
  box.h = (displayCommand[command].line + 1) * DISPLAY_TEXT_HEIGHT - box.y;
  return box;
}

SDL_Rect GetBoxofLine(int line, int colStart, int colEnd) {
  SDL_Rect box = {0, 0, 0, 0};
  if (line < 0 || line >= ROW_COUNT) return box;
  if (colStart < 0 || colStart >= GetMaxCol()) return box;
  if (colEnd < colStart) colEnd = colStart;
  box.x = ColToPixel(colStart);
  box.y = line * DISPLAY_TEXT_HEIGHT;
  box.w = ColToPixel(colEnd + 1) - box.x;
  box.h = DISPLAY_TEXT_HEIGHT;
  return box;
}



#define CONFBOX_BORDER_WIDTH 10
// Confirmation box for dangerous actions.
void DrawConfirmationBox(SDL_Rect *yesBox, SDL_Rect *noBox, bool_t selected, char *label) {
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
  boxX = (GetWindowWidth() / 2) - (boxWidth / 2);
  boxY = (GetWindowHeight() / 2) - (boxHeight / 2);
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


// Display box for entering keyboard text into a parameter.
void DrawTextEntryBox(int item, char * text) {

  // Vars
  SDL_Rect box;
  SDL_Rect dstRect;
  SDL_Color fontColor = {DISPLAY_COLOR_PARMS.r, DISPLAY_COLOR_PARMS.g, DISPLAY_COLOR_PARMS.b};
  SDL_Color fontBGColor = {DISPLAY_COLOR_PARMSBG.r, DISPLAY_COLOR_PARMSBG.g, DISPLAY_COLOR_PARMSBG.b};
  SDL_Surface *textS = NULL;
  SDL_Texture *texture = NULL;

  // Get the dimensions.
  box.x = ColToPixel(displayCommand[item].col + 1);
  box.y = displayCommand[item].line * DISPLAY_TEXT_HEIGHT;
  //~ box.w = (ColToPixel(displayCommand[item].col + 2) - box.x);
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
void DrawEnumSelectBox(int set, int item, int selected, SDL_Rect ** targets) {

  // Vars
  color_t fg = DISPLAY_COLOR_PARMS;
  color_t bgh = DISPLAY_COLOR_TEXTSBG_HL;
  point_t boxOrigin;  // All boxes are relative to this.
  int i, h, boxCount;
  int boxH, boxW;
  SDL_Rect wholeBox, labelBox;
  char text[100];
  static SDL_Texture **textures = NULL;
  static int textureCount = 0;
  static SDL_Texture *label = NULL;
  patternElement_e element = displayCommand[item].dataSource;
  if (element < PE_INVALID) element = GetSelectElement(set, element);

  // Get the number of choices in the selector.
  boxCount = enumerations[patternElements[element].etype].size;

  // Calculate the entire box width and position.
  h = CHAR_H + (ENUM_BORDER * 2);
  boxW = max(strlen(displayCommand[item].text) * CHAR_W + (ENUM_BORDER * 2), 2 * (CHAR_W * PARAMETER_WIDTH + (ENUM_BORDER * 2)));
  boxH = h * ((boxCount / 2) + (boxCount % 2) + 1);
  boxOrigin.x = (GetWindowWidth() / 2) - boxW / 2;
  boxOrigin.y = (GetWindowHeight() / 2) - boxH / 2;
  wholeBox.x = boxOrigin.x;
  wholeBox.y = boxOrigin.y;
  wholeBox.w = boxW;
  wholeBox.h = boxH;

  // Draw the outline of the whole box.
  DrawOutlineBox(wholeBox, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);

  // Draw the label decoration.
  labelBox.x = boxOrigin.x;
  labelBox.y = boxOrigin.y;
  labelBox.w = boxW;
  labelBox.h = h;
  DrawSBox(labelBox, DISPLAY_COLOR_PARMS);

  // Initialize the boxes if we haven't.
  if (! *targets) {

    // Allocate the space for the target boxes.
    (*targets) = malloc(sizeof(**targets) * boxCount);
    if (! (*targets)) {
      fprintf(stderr, "Couldn't allocate some important memory.  Fuck it.\n");
      exit(EXIT_FAILURE);
    }

    // Allocate the space for the texture targets.
    if (textureCount) {
      // First free the textures.
      for (i = 0 ; i < textureCount; i++) {
        SDL_DestroyTexture(textures[i]);
      }
      SDL_DestroyTexture(label);
      // Now free the texture pointers.
      free(textures);
    }

    // Allocate the texture pointers.
    textureCount = boxCount;
    textures = malloc(boxCount * sizeof(SDL_Texture *));
    if (!textures) {
      fprintf(stderr, "Couldn't allocate the text texture memory!\n");
      exit(EXIT_FAILURE);
    }

    // Next, generate the texts
    for(i = 0 ; i < boxCount; i++) {
      snprintf(text, sizeof(text), "%s", enumerations[patternElements[element].etype].texts[i]);
      textures[i] = CreateTextureText(text, DISPLAY_COLOR_PARMS);
    }
    snprintf(text, sizeof(text), "%s", displayCommand[item].text);
    label = CreateTextureText(text, DISPLAY_COLOR_PARMSBG);

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
    if (i == selected) {
      DrawOutlineBox((*targets)[i], fg, bgh);
      DrawCenteredTexture((*targets)[i], textures[i]);
    } else {
      DrawCenteredTexture((*targets)[i], textures[i]);
    }
  }
  DrawCenteredTexture(labelBox, label);
}

// Color headings
const char *colorTitles[] = {"Red", "Orange", "Yellow", "Chartreuse", "Green", "Aqua",
  "Cyan", "Azure", "Blue", "Violet", "Magenta", "Rose", "Gray scale"};
const int colorTitlesCount = sizeof(colorTitles) / sizeof(const char *);

void DrawColorSelectBox(int item, int selected, SDL_Rect ** targets) {
  char text[100];
  int i, w;
  static SDL_Texture **textures = NULL;
  static int h = 0;
  int totalWidth, boxWidth;
  point_t boxOrigin;  // All boxes are relative to this.
  SDL_Rect box;

 // Calculate the box positions.
  totalWidth = 3 * GetWindowWidth() / 4;
  boxWidth = totalWidth / colorTitlesCount;
  boxOrigin.x = GetWindowWidth() / 2 - totalWidth / 2;
  boxOrigin.y = GetWindowHeight() / 2;

  // Initialize the boxes if we haven't.
  if (! *targets) {

    // Allocate the space for the target boxes.
    (*targets) = malloc(sizeof(**targets) * CE_COUNT);
    if (! (*targets)) {
      fprintf(stderr, "Couldn't allocate some important memory.  Fuck it.\n");
      exit(EXIT_FAILURE);
    }

    // Allocate the texture pointers, if not already done.
    if (!textures) {

      textures = malloc(colorTitlesCount * sizeof(SDL_Texture *));
      if (!textures) {
        fprintf(stderr, "Couldn't allocate the text texture memory!\n");
        exit(EXIT_FAILURE);
      }

      // Next, generate the texts
      for(i = 0 ; i < colorTitlesCount; i++) {
        snprintf(text, sizeof(text), "%s", colorTitles[i]);
        textures[i] = CreateTextureText(text, DISPLAY_COLOR_PARMS);
        if (!textures[i]) exit(EXIT_FAILURE);
        SDL_QueryTexture(textures[i], NULL, NULL, &w, &h);
      }
    }

    // Layout the target boxes.  Top to bottom, Left to right.
    for (i = 0; i < CE_COUNT; i++) {
      (*targets)[i].x = boxOrigin.x + ((boxWidth + 1) * (i / (CE_COUNT / colorTitlesCount)) + 1);
      (*targets)[i].y = boxOrigin.y + (h * 2 * ((i % (CE_COUNT / colorTitlesCount)) + 1));
      (*targets)[i].w = boxWidth + 1;
      (*targets)[i].h = h * 2;
    }
  }

  // Draw the titles.
  for (i = 0; i < colorTitlesCount; i++) {
    box.x = boxOrigin.x + (boxWidth + 1) * i;
    box.y = boxOrigin.y;
    box.w = boxWidth + 1;
    box.h = h * 2;
    DrawOutlineBox(box, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
    DrawCenteredTexture(box, textures[i]);
  }

  // Draw the boxes.
  for (i = 0; i < CE_COUNT; i++) {
    box = (*targets)[i];
    if (i == selected) {
      DrawSBox(box, DISPLAY_COLOR_PARMS);
      DrawSRectangle(box, cBlack);
      box.x += 4;
      box.y += 4;
      box.w -= 8;
      box.h -= 8;
      DrawSBox(box, namedColors[i].color);
      DrawSRectangle(box, cBlack);
    } else {
      DrawSBox(box, namedColors[i].color);
    }
  }

  // Draw the outline box
  DrawRectangle(boxOrigin.x, boxOrigin.y, totalWidth + 1, h * 2 * ((CE_COUNT / colorTitlesCount) + 1), DISPLAY_COLOR_PARMS);
  DrawRectangle(boxOrigin.x - 1, boxOrigin.y - 1, totalWidth + 3, h * 2 * ((CE_COUNT / colorTitlesCount) + 1) + 2, DISPLAY_COLOR_PARMS);
}

// Write the static menu information to the display.
void DrawDisplayTexts(int selected) {

  // Vars
  int i;
  char text[100];
  static displayText_t *displayTexts = NULL;
  static int textCount = 0;
  displayText_t textTemp;
  //~ static int headerStart = 0;

  // If we haven't done so yet, we'll render all of the display texts to be
  // cached in video RAM.
  if (!textCount) {

    // Render the commands
    for (i = 0; i < displayCommandCount; i++) {
      CreateTextureCommand(&textTemp, i, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the headers.
    //~ headerStart = textCount;
    for (i = 0; i < headerTextCount; i++) {
      // We precalculate the width purely to allow the background of the header
      // to span the whole box.  This way we don't have to draw the background
      // separately.  It is rendered as part of the text.
      textTemp.targetBox.x = ColToPixel(headerText[i].col);
      textTemp.targetBox.w = ColToPixel(headerText[i].col + 1) - textTemp.targetBox.x + ((CHAR_W * PARAMETER_WIDTH) - CHAR_W);
      snprintf(text, sizeof(text), " %-*s", textTemp.targetBox.w / CHAR_W, headerText[i].text);
      CreateTextureLine(&textTemp, text, headerText[i].line, headerText[i].col, DISPLAY_COLOR_TITLES, DISPLAY_COLOR_TITLESBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the labels
    for (i = 0; i < labelCount; i++) {
      snprintf(text, sizeof(text), " %s", labelText[i].text);
      CreateTextureLine(&textTemp, text, labelText[i].line, labelText[i].col, DISPLAY_COLOR_LABEL, DISPLAY_COLOR_LABELBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }

    // Render the texts.
    for (i = 0; i < displayTextCount; i++) {
      snprintf(text, sizeof(text), " %s", displayText[i].text);
      CreateTextureLine(&textTemp, text, displayText[i].line, displayText[i].col, DISPLAY_COLOR_INFO, DISPLAY_COLOR_INFOBG);
      if (textTemp.texture != NULL) {
        textCount++;
        displayTexts = realloc(displayTexts, textCount * sizeof(displayText_t));
        displayTexts[textCount - 1] = textTemp;
      }
    }
  }

  // Draw the textures to the rendering surface.
  for (i = 0; i < textCount; i++) {
    //~ if (i == selected) {
      //~ DrawSBox(displayTexts[i].targetBox, DISPLAY_COLOR_TEXTSBG_HL);
    //~ } else if (i >=df headerStart && i < headerStart + headerTextCount) {
      //~ DrawSBox(displayTexts[i].targetBox, DISPLAY_COLOR_TITLESBG);
    //~ }
    SDL_RenderCopy(mwRenderer, displayTexts[i].texture, NULL, &displayTexts[i].targetBox);
  }
}

void CreateTextureCommand(displayText_t *target, int index, color_t fg, color_t bg) {
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
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_EDIT].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_EDIT].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_RST].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_RST].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s", SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  } else {
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_EDIT].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_EDIT].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_EDIT].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_UP].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_RST].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_RST].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_RST].key)) ? " " : "");
    ind += snprintf(ind, end - ind, "%s%s%s",
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " - " : "",
      SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key),
      strlen(SDL_GetKeyName(displayCommand[index].commands[MOUSE_WHEEL_DOWN].key)) ? " " : "");
  }

  // Write it.
  CreateTextureLine(target, text, displayCommand[index].line, displayCommand[index].col, fg, bg);
}

// Draw the borders around the preview output.  We can switch between portrait
// and landscape, so the preview space is a square box that would accomodate
// either orientation.
void DrawPreviewBorder(point_t location, highLight_e highLight) {

  // Vars
  int w, h;
  int tw, th;
  int maxDim;
  int i;
  color_t color; //hl = {{0, 127, 0, 255}};
  
  // Get the current widths and heights to construct the preview border from.
  tw = GetTensorWidth();
  th = GetTensorHeight();

  // Get the outer border dimensions.
  w = (tw * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);
  h = (th * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);

  // Preview space dimension.
  maxDim = max(w, h);

  // Erase the old preview.
  DrawBox(location.x, location.y, maxDim, maxDim, cBlack);
  DrawRectangle(location.x, location.y, maxDim, maxDim, cWhite);
  
  //fprintf(stdout, "highlight: %i\n", highLight);
  
  switch (highLight) {
    case HL_SELECTED:
      color = cHdkGreen;
      break;
    case HL_HOVER:
      color = cYellow;
      break;
    case HL_NO:
    default:
      color = cBlack;
      break;
  }
      
  DrawBox(location.x + 1, location.y + 1, maxDim - 2, maxDim - 2, color);

//  printf("Preview box: %i, %i, %i, %i\n", x, y, maxDim, maxDim);
//  printf("Preview pixel size: %i\n", PREVIEW_PIXEL_SIZE);
  // Ajust x and y to center the preview.
  location.x = location.x + (maxDim - w) / 2;
  location.y = location.y + (maxDim - h) / 2;

  // Draw the new outer border.
  DrawRectangle(location.x, location.y, w, h, cWhite);
  DrawBox(location.x + 1, location.y + 1, w - 2, h - 2, cBlack);

  // Get the inner border dimensions.
  w = (tw * PREVIEW_PIXEL_SIZE) + 1;
  h = (th * PREVIEW_PIXEL_SIZE) + 1;

  if (tw < th) {
    // Portrait - Draw horizontal panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(location.x + PREVIEW_BORDER - 1, location.y + i * (h / 3) + PREVIEW_BORDER - 1, w, (h / 3) + 1, cGray);
    }
  } else {
    // Landscape - Draw vertical panel indicators.
    for (i = 0; i < 3; i++) {
      DrawRectangle(location.x + i * (w / 3) + PREVIEW_BORDER - 1, location.y + PREVIEW_BORDER - 1, (w / 3) + 1, h, cGray);
    }
  }
}

// Check if the mouse coordinates fall within a preview border.
overPreview_e OverPreviewBorder(point_t mouse) {
  // Vars
  int w, h;
  int maxDim;
  SDL_Rect lbox, abox, sbox;
  
  point_t live, alt;
  live = GetLiveLoc();
  alt = GetAltLoc();
  int tw, th;
  tw = GetTensorWidth();
  th = GetTensorHeight();

  // Calculate width and height of the outer box.
  w = (tw * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);
  h = (th * PREVIEW_PIXEL_SIZE) + (PREVIEW_BORDER * 2);
  maxDim = max(w, h);
  
  // The outer boxes
  SetBox(&lbox, live.x, live.y, maxDim, maxDim);
  SetBox(&abox, alt.x, alt.y, maxDim, maxDim);

  // Inner box is centered on the outer box, so adjust the starting points.
  live.x = live.x + (maxDim - w) / 2;
  live.y = live.y + (maxDim - h) / 2;
  alt.x = alt.x + (maxDim - w) / 2;
  alt.y = alt.y + (maxDim - h) / 2;

  // Calculate the inner box size.
  w = (tw * PREVIEW_PIXEL_SIZE) + (2 * PREVIEW_BORDER);
  h = (th * PREVIEW_PIXEL_SIZE) + (2 * PREVIEW_BORDER);
  
  // If we are in the live outer box...
  if (IsInsideBox(mouse, lbox)) {
    
    // Then the inner box is this:
    SetBox(&sbox, live.x, live.y, w, h);
    //DrawBox(sbox.x, sbox.y, sbox.w, sbox.h, cRed);
    
    // If we are outside the inner box, we good.
    if (!IsInsideBox(mouse, sbox)) {
      return OP_LIVE;
    }
    
  // Or if we are in the alternate outer box...
  } else if (IsInsideBox(mouse, abox)) {
    
    // Then the inner box is this:
    SetBox(&sbox, alt.x, alt.y, w, h);
    //DrawBox(sbox.x, sbox.y, sbox.w, sbox.h, cBlue);
    
    // If we are outside the inner box, good.
    if (!IsInsideBox(mouse, sbox)) {
      return OP_ALT;
    }  
  }
  
  return OP_NO;
}
