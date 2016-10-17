// CONFIRM

// Inputs
event
&confirmed
mouse
&boxYes
&boxNo
boxOld2
box2
lastHover
thisHover
leftMouseDownOn
User_Timer
keySave
setSave

// Outputs
confirmed
inputMode
thisHover
lastHover
box2


// Process the event.
switch(event.type) {
  case SDL_KEYDOWN:
    if (HandleConfirmation(event.key.keysym.sym, &confirmed)) inputMode = IM_NORMAL;
    break;

  case SDL_MOUSEMOTION:
    // Check for intersection with confirmation boxes.
    thisHover = INVALID;
    if (IsInsideBox(mouse, boxYes)) {
      thisHover = YES;
      confirmed = YES;
      box2 = boxYes;
    } else if (IsInsideBox(mouse, boxNo)) {
      thisHover = NO;
      confirmed = NO;
      box2 = boxNo;
    }
    if (thisHover != INVALID) {
      // Is it hovering over a command is wasn't hovering over before?
      if ((!IsSameBox(box2, boxOld2)) || (lastHover == INVALID)) {
        boxOld2 = box2;
        lastHover = thisHover;
      }
    }
    // Not over a new command? May have to clear the old highlight anyway.
    if ((thisHover == INVALID) && (lastHover != INVALID)) {
      lastHover = INVALID;
      confirmed = NO;
    }
    break;

  case SDL_MOUSEBUTTONUP:
    // Mouse button unpushed.  Consider this a click.  If we're over
    // the same item we down clicked on, execute a command.
    if (event.button.button == SDL_BUTTON_LEFT) {
      if ((thisHover != INVALID) && (thisHover == leftMouseDownOn)) {
        if (thisHover == YES) {
          SavePatternSet(keySave, setSave, YES, "");
        } else {
          Message("Save action cancelled.");
        }
        inputMode = IM_NORMAL;
      }
    }
    break;

  default:
    // Redraw the confirmation box, but only on GUI updates.
    if (event.type == User_Timer && event.user.code == TI_GUI) {
      DrawConfirmationBox(&boxYes, &boxNo, confirmed, GetLastMessage());
    }
    break;
}  // End event processing switch
break;
