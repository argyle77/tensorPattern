// gui.h
// Contains the prototypes for setting up and drawing to the gui.
// Johua Krueger
// 2016_10_12

#ifndef GUI_H_
  #define GUI_H_
  
  // Includes
  #include <SDL_ttf.h>
  #include "useful.h"  
  
  // Public prototypes
  bool_t InitGui(int w, int h, int fontSize);
  void SetWindowTitle(const char * text);
  void UpdateGUI(void);
  void QuitGui(void);
  void ShowMouse(void);
  void HideMouse(void);
  
  // Getters
  int GetWindowWidth(void);
  int GetWindowHeight(void);
  int GetRowCount(void);
  
  // Public data
  extern SDL_Window *mainWindow;
  extern SDL_Renderer *mwRenderer;
  extern TTF_Font *screenFont;
  
#endif /* GUI_H_ */
