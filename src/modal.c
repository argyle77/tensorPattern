// 
#include "modal.h"
#include <SDL.h>

#include "useful.h"
#include "timers.h"

void InitGuiState(guiState_t *state) {
  state->rightMouseDownOn = INVALID;
  state->leftMouseDown = NO;
  state->rightMouseDown = NO;
  state->mouse = SetPoint(0, 0);
  state->confirmed = NO;
  state->inputMode = IM_NORMAL;
  state->oldInputMode = IM_INVALID;
  state->thisHover = INVALID;
  state->lastHover = INVALID;
  SetBox(&(state->box2), 0, 0, 0, 0);
  SetBox(&(state->boxOld2), 0, 0, 0, 0);
  SetBox(&(state->boxYes), 0, 0, 0, 0);
  SetBox(&(state->boxNo), 0, 0, 0, 0);
  state->overBorder = OP_NO;
  state->overBorderDown = OP_NO;
  state->liveHighlight = HL_SELECTED;
  state->altHighlight = HL_NO;
  state->wheelColorF = 0;
  state->wheelColorB = 0;
  state->targets = NULL;
  state->textEntry[0] = '\0';
  state->enumHover = INVALID;
}

// COMMON - Everybody wants these things done when an event happens.
void CommonActions(guiState_t *s, guiActionFlags_t *o) {
  switch (s->event.type) {

  case SDL_MOUSEBUTTONUP:
    // Button up.  Mark the buttons as being up.
    if (s->event.button.button == SDL_BUTTON_LEFT) {
      s->leftMouseDown = NO;
      s->leftWasLastUp = YES;
    } else if (s->event.button.button == SDL_BUTTON_RIGHT) {
      s->rightMouseDown = NO;
      s->leftWasLastUp = NO;
    }
    break;
  case SDL_MOUSEMOTION:
    // Mouse moved. See where it is.
    s->mouse.x = s->event.motion.x;
    s->mouse.y = s->event.motion.y;
    break;
  case SDL_MOUSEBUTTONDOWN:
    // Mouse button pushed.  Make a note of the item it was pushed over.
    if (s->event.button.button == SDL_BUTTON_LEFT) {
      s->leftMouseDownOn = s->thisHover;
      s->leftMouseDown = YES;
    } else if (s->event.button.button == SDL_BUTTON_RIGHT) {
      s->rightMouseDownOn = s->thisHover;
      s->rightMouseDown = YES;
    }
    break;
  case SDL_QUIT:
    // Window closed or <ctrl> c pushed in terminal.  Exit program.
    o->exitProgram = YES;
    break;
  case SDL_WINDOWEVENT:
    // We care about the window events that destroy our display.  Those
    // are probably resize and expose events.  We'll redraw the whole
    // display if they occur.
    if ((s->event.window.event == SDL_WINDOWEVENT_EXPOSED) ||
        (s->event.window.event == SDL_WINDOWEVENT_RESIZED)) {
      o->refreshAll = YES;
    } else {
      //~ fprintf(stderr, "Unhandled SDL window event: %i\n", event.window.event);
    }
    break;
  default:
    // There are two registered user events to check for here. They can't
    // be tested in the case statement because they are not compile-time
    // integer constants.
  
    // The first is the user timer.
    if (s->event.type == User_Timer) {
      // There are several timers to handle.
      switch(s->event.user.code) {
        case TI_FPS: ResetFPSCounters(); break;
        case TI_LIVE: o->drawNewFrameA = YES; break;  // Set a new live frame to draw.
        case TI_ALT:  o->drawNewFrameB = YES; break;  // Set a new alt frame to draw.
        case TI_GUI: o->updateInfoDisplay = YES; break;
        // Unknown timer?              
        default: 
          //~ Error("Unhandled timer expiration event.");
          break;
      }
      
    // The second is the confirmation box event, which can get pushed for
    // prompts to overwrite during file saves.
    } else if (s->event.type == s->User_ConfirmationBox) {
      s->inputMode = IM_CONFIRM;
    } else {
      //~ fprintf(stderr, "Unhandled SDL event: %i\n", event.type);
    }
    break;
 }
}
