// Modal.h
// Joshua Krueger
// 2016_10_16

#ifndef MODAL_H_
  #define MODAL_H_
  
  // Includes
  #include <SDL.h>
  #include "useful.h"
  #include "drawSupport.h"
  #include "guidraw.h"
  
  #define TEXT_ENTRY_SIZE 100
  
  // Gui input modes
  typedef enum inputMode_e {
    IM_INVALID = -1,
    IM_NORMAL = 0, IM_CONFIRM, IM_INT, IM_FLOAT, IM_ENUM, IM_COLOR,
    IM_COUNT // Last
  } inputMode_e;
  
  typedef struct guiState_t {
    SDL_Event event;
    bool_t leftWasLastUp;
    int leftMouseDownOn; // = INVALID;
    int rightMouseDownOn; // = INVALID;
    bool_t leftMouseDown; // = NO;
    bool_t rightMouseDown; // = NO;
    point_t mouse; // = {0, 0};
    bool_t confirmed; // = NO;
    inputMode_e inputMode; // = IM_NORMAL;
    inputMode_e oldInputMode; // = IM_INVALID;
    int thisHover; // = INVALID;
    int lastHover; // = INVALID;
    SDL_Rect box2, boxOld2; // = {0, 0, 0, 0}, boxOld2 = {0, 0, 0, 0};
    SDL_Rect boxYes, boxNo;
    Uint32 User_ConfirmationBox;
    overPreview_e overBorder, overBorderDown;
    highLight_e liveHighlight, altHighlight;
    color_e wheelColorF, wheelColorB;
    SDL_Rect *targets;
    char textEntry[TEXT_ENTRY_SIZE];    
    int enumHover;
    
  } guiState_t;
  
  typedef struct guiActionFlags_t {
    bool_t refreshAll;
    bool_t updateInfoDisplay;
    bool_t refreshGui;
    bool_t refreshPreviewBorders;
    bool_t drawNewFrameA;
    bool_t drawNewFrameB;
    bool_t exitProgram;
  } guiActionFlags_t;

  void InitGuiState(guiState_t *state);
  void CommonActions(guiState_t *s, guiActionFlags_t *o);
  
#endif /* #ifndef MODAL_H_ */
