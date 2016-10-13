// Gui.c
// Contains the functions for handling the gui.
// Johua Krueger
// 2016_10_12

// Includes
#include <SDL.h>
#include <SDL_ttf.h>
#include "gui.h"

// Defines
#define WWIDTH_MIN 100
#define WWIDTH_MAX 10000
#define WHEIGHT_MIN 100
#define WHEIGHT_MAX 10000
#define FSIZE_MIN 1
#define FSIZE_MAX 100
#define FONT_FILE "font.ttf"

// Vars
int wWidth, wHeight, fSize;
TTF_Font *screenFont = NULL;
SDL_Window *mainWindow = NULL;
SDL_Renderer *mwRenderer = NULL;

// Initialization
// Font size here is basically height.
bool_t InitGui(int w, int h, int fontSize) {
  
  // Check the inputs
  if (w < WWIDTH_MIN ||
      w > WWIDTH_MAX ||
      h < WHEIGHT_MIN ||
      h > WHEIGHT_MAX) {
    fprintf(stderr, "Window dimensions exceed limits (%i, %i)!\n", w, h);
    return FALSE;
  }
  
  if (fontSize < FSIZE_MIN ||
      fontSize > FSIZE_MAX) {
    fprintf(stderr, "Font size outside limits (%i)!\n", fontSize);
    return FALSE;
  }    
  
  // Assign the internal variables.
  wWidth = w;
  wHeight = h;
  fSize = fontSize;

  // Initialize SDL.
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    return FALSE;
  }

  // Register the SDL cleanup function.
  atexit(SDL_Quit);

  // Initialize the window and renderer.
  SDL_CreateWindowAndRenderer(wWidth, wHeight, SDL_WINDOW_RESIZABLE, &mainWindow, &mwRenderer);
  if ((!mainWindow) || (!mwRenderer)) {
    fprintf(stderr, "Unable to create main window or renderer: %s\n", SDL_GetError());
    return FALSE;
  }

  // Window scaling hints:
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(mwRenderer, wWidth, wHeight);

  // Initialize the font handling.
  if (TTF_Init() < 0) {
    fprintf(stderr, "Couldn't initialize SDL TTF: %s\n", TTF_GetError());
    return FALSE;
  }

  // Load the screen font.  Should be fixed width.
  screenFont = TTF_OpenFont(FONT_FILE, fSize);
  if(!screenFont) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return FALSE;
  }

  return TRUE;
}


// Window title
void SetWindowTitle(const char * text) {
  SDL_SetWindowTitle(mainWindow, text);
}


// Update the rendered output.
void UpdateGUI(void) {
  SDL_RenderPresent(mwRenderer);
}


// Clean up before exiting.
void QuitGui(void) {
  // Close up the TTF stuff.
  TTF_CloseFont(screenFont);
  screenFont = NULL; // to be safe...?
  TTF_Quit();
}


int GetWindowWidth(void) {
  return wWidth;
}

int GetWindowHeight(void) {
  return wHeight;
}

int GetFontSize(void) {
  return fSize;
}

