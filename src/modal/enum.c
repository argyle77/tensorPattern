// ENUM

 case IM_ENUM:
switch(event.type) {
  case SDL_KEYDOWN:
    // Put some stuff here.
    if (HandleEnumSelect(event.key.keysym.sym,
      displaySet ? altSet : liveSet, thisHover, &enumHover))
      inputMode = IM_NORMAL;
    break;
  case SDL_MOUSEMOTION:
    // Check if its hovering over a command.
    enumHover = OverBox(displaySet ? altSet : liveSet, mouse, thisHover, &targets, &lastHover);
    break;  // End case SDL_MOUSEMOTION.
  case SDL_MOUSEBUTTONDOWN:
    // Correct the error made by the common code.
    if (event.button.button == SDL_BUTTON_LEFT) leftMouseDownOn = enumHover;
    break;
  case SDL_MOUSEBUTTONUP:
   // Mouse button unpushed.  Consider this a click.  If we're over
    // the same item we down clicked on, the user has chosen.
    if (event.button.button == SDL_BUTTON_LEFT) {
      if ((enumHover != INVALID) && (enumHover == leftMouseDownOn)) {
        element = displayCommand[thisHover].dataSource;
        if (element < PE_INVALID) element = GetSelectElement(displaySet ? altSet : liveSet, element);
        SENUM(displaySet ? altSet : liveSet, element) = enumHover;
        Message("Item %s set to %s",
          displayCommand[thisHover].text,
          enumerations[patternElements[element].etype].texts[enumHover]);
        inputMode = IM_NORMAL;
      }
    } else if (event.button.button == SDL_BUTTON_RIGHT) {
      Message("Item selection cancelled.");
      inputMode = IM_NORMAL;
    }
    break;  // End SDL_MOUSEBUTTONUP case.
  default:
    if (event.type == User_Timer && event.user.code == TI_GUI) {
      // Handle drawing the input selection boxes.
      DrawEnumSelectBox(displaySet ? altSet : liveSet, thisHover, enumHover, &targets);
    }
    break;
} // End event processing switch for ENUM selection
break; // End case IM_ENUM
