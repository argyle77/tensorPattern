// normal.c
// Normal mode input processing.
// Joshua Krueger
// 2016_10_14


// NORMAL

 // Normal mode events
case IM_NORMAL:
switch(event.type) {
  case SDL_KEYDOWN:
    exitProgram = HandleKey(displaySet ? altSet : liveSet, event.key.keysym.sym, event.key.keysym.mod);
    break;
  case SDL_MOUSEMOTION:
    // Check if its hovering over a command.
    thisHover = OverCommand(mouse, &lastHover);
    
    // Note, we don't do preview drawing here because our pen has to
    // be capable of drawing to every frame in the animation without
    // a mouse event occurring.
    
    // Check if it is hovering over a preview boarder
    overBorder = OverPreviewBorder(mouse);
                                      
    switch(overBorder) {
      case OP_ALT:
        if (altHighlight != HL_HOVER) {
          if (displaySet != OO_ALTERNATE) {
            altHighlight = HL_HOVER;
            refreshPreviewBorders = YES;
          }
        }
        break;
      case OP_LIVE:
        if (liveHighlight != HL_HOVER) {
          if (displaySet != OO_CURRENT) {
            liveHighlight = HL_HOVER;
            refreshPreviewBorders = YES;
          }
        }
        break;
      case OP_NO:
      default:
        if ((liveHighlight == HL_HOVER) || (altHighlight == HL_HOVER)) {
          liveHighlight = altHighlight = HL_INVALID;
          refreshPreviewBorders = YES;
        }
        break;
    }
                                      
    break;  // End case SDL_MOUSEMOTION.

  case SDL_MOUSEWHEEL:
    // The mouse wheel moved.  See if we should act on that.  We don't
    // check HandleCommand return codes because there are no mouse wheel
    // commands that exit the program.  Wheel up increases a value.
    // Wheel down decreases a value.  Left click edits a value, right
    // click resets that value to default.  If there are no wheel down
    // commands, the wheel up command is used.  If there are no wheel
    // up command, the edit value command is used.
    if (thisHover != INVALID) {

      // Wheel down.
      if (event.wheel.y < 0) {
        command = displayCommand[thisHover].commands[MOUSE_WHEEL_DOWN].command;
        if (command == COM_NONE) {
          command = displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command;
        }
        if (command == COM_NONE) {
          command = displayCommand[thisHover].commands[MOUSE_EDIT].command;
        }
        HandleCommand(displaySet ? altSet : liveSet, command, thisHover);

      // Wheel up.
      } else {
        command = displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command;
        if (command == COM_NONE) {
          command = displayCommand[thisHover].commands[MOUSE_EDIT].command;
        }
        HandleCommand(displaySet ? altSet : liveSet, command, thisHover);
      }

    // Mouse wheel action over the live preview lets us change colors
    // without having to go to the command.
    } else if (IsInsideBox(mouse, GetLiveBox())) {
      if (!rightMouseDown) {
        if (event.wheel.y < 0) { // Wheel down
          wheelColorF--;
          if (wheelColorF < 0) wheelColorF = CE_COUNT - 1;
          SCOLOR(liveSet, PE_LP_COLA) = namedColors[wheelColorF].color;
        } else { // Wheel up
          wheelColorF++;
          if (wheelColorF >= CE_COUNT) wheelColorF = 0;
          SCOLOR(liveSet, PE_LP_COLA) = namedColors[wheelColorF].color;
        }
      } else {
        // Holding down the right mouse button while wheeling changes the
        // background color instead.
        if (event.wheel.y < 0) { // Wheel down
          wheelColorB--;
          if (wheelColorB < 0) wheelColorB = CE_COUNT - 1;
          SCOLOR(liveSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
        } else { // Wheel up
          wheelColorB++;
          if (wheelColorB >= CE_COUNT) wheelColorB = 0;
          SCOLOR(liveSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
        }
      }
    // Same for the alternate preview, except we change the alternate
    // set values.
    } else if (IsInsideBox(mouse, GetAltBox())) {
      if (!rightMouseDown) {
         if (event.wheel.y < 0) { // Wheel down
          wheelColorF--;
          if (wheelColorF < 0) wheelColorF = CE_COUNT - 1;
          SCOLOR(altSet, PE_LP_COLA) = namedColors[wheelColorF].color;
        } else { // Wheel up
          wheelColorF++;
          if (wheelColorF >= CE_COUNT) wheelColorF = 0;
          SCOLOR(altSet, PE_LP_COLA) = namedColors[wheelColorF].color;
        }
      } else {
        // Holding down the right mouse button while wheeling changes the
        // background color instead.
        if (event.wheel.y < 0) { // Wheel down
          wheelColorB--;
          if (wheelColorB < 0) wheelColorB = CE_COUNT - 1;
          SCOLOR(altSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
        } else { // Wheel up
          wheelColorB++;
          if (wheelColorB >= CE_COUNT) wheelColorB = 0;
          SCOLOR(altSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
        }
      }
    }
    break; // End SDL_MOUSEWHEEL case

  case SDL_MOUSEBUTTONUP:
    // Mouse button unpushed.  Consider this a click.  If we're over
    // the same item we down clicked on, execute the command.
    // Right mouse button click is a parameter reset.  Left mouse
    // button click is a parameter edit.  Right click over a color
    // resets the color.  Left click over a color sets to color
    // selector to edit this item.
    if (event.button.button == SDL_BUTTON_RIGHT) {
      if ((thisHover != INVALID) && (thisHover == rightMouseDownOn)) {
        if (!HandleColorSelection(mouse, thisHover, YES, displaySet ? altSet : liveSet)) {
          exitProgram = HandleCommand(displaySet ? altSet : liveSet,
            displayCommand[thisHover].commands[MOUSE_RST].command, thisHover);
        }
      }
    } else if (event.button.button == SDL_BUTTON_LEFT) {
      // First check if we are switching preview sets...
      overBorder = OverPreviewBorder(mouse);
      if ((overBorder != OP_NO) && (overBorder == overBorderDown)) {
        displaySet = (overBorder == OP_ALT ? OO_ALTERNATE : OO_CURRENT);
        liveHighlight = altHighlight = HL_INVALID;
        refreshPreviewBorders = YES;
        //fprintf(stdout, "Switch preview to %i\n", overBorder);
        // TODO: If displaySet changes, maybe we write to the status line.
      }
      

      if ((thisHover != INVALID) && (thisHover == leftMouseDownOn)) {
        // First check if we clicked on a color selector.
        if (HandleColorSelection(mouse, thisHover, NO, displaySet ? altSet : liveSet)) {
          // For edit clicks on the color boxes, we set the color
          // selector to the appropriate color source.
          SENUM(displaySet ? altSet : liveSet, PE_PALETTEALTER) =
            displayCommand[thisHover].colorSource;
        } else {
          // Otherwise, not on a color box, we execute the edit
          // command (which may be nothing), and possibly change the
          // input mode.
          exitProgram = HandleCommand(displaySet ? altSet : liveSet,
              displayCommand[thisHover].commands[MOUSE_EDIT].command,
              thisHover);

          // Change input mode?
          inputMode = EditValue(liveSet, thisHover);
        }
        switch(inputMode) {
          case IM_INT:
          case IM_FLOAT:
            // Initial value to edit would go here, if our editor had
            // a moveable cursor.
            //~ snprintf(textEntry, sizeof(textEntry), "%i", DINT(displayCommand[thisHover].dataSource));
            textEntry[0] = '\0';
            break;
          case IM_ENUM:
          case IM_COLOR:
            // For enum edits, we first free the target boxes so that
            // new ones can be generated.
            if (targets) {
              free(targets);
              targets = NULL;
            }
            break;
          default:
            break;
        } // End inputMode switch
      } // End left mouse down if handler
    } // End mouse button if
    break;  // End SDL_MOUSEBUTTONUP case.

  case SDL_MOUSEBUTTONDOWN:
    overBorderDown = OverPreviewBorder(mouse);
    break;  // End SDL_MOUSEBUTTONDOWN case.
    
  default:
    break;
} // End normal mode events processing switch
break;  // End IM_NORMAL case.
