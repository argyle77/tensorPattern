// Color Selection

// Inputs
event
displaySet
altSet
liveSet
thisHover
mouse
leftMouseDownOn
&enumHover
&targets
&lastHover
displayCommand
namedColors
SCOLOR - patternElements

// Outputs
inputMode
enumHover
targets
lastHover
leftMouseDownOn
element
patternData

// Handle color selection dialog
case IM_COLOR:
switch(event.type) {
  case SDL_KEYDOWN:
    // Put some stuff here.
    if (HandleColorSelect(event.key.keysym.sym,
      displaySet ? altSet : liveSet, thisHover, &enumHover))
      inputMode = IM_NORMAL;
    break;
  case SDL_MOUSEMOTION:
    // Check if its hovering over a command.
    enumHover = OverColorBox(displaySet ? altSet : liveSet, mouse, thisHover, &targets, &lastHover);
    break;  // End case SDL_MOUSEMOTION.
  case SDL_MOUSEBUTTONDOWN:
    // Correct the error made by the common code.
    if (event.button.button == SDL_BUTTON_LEFT) leftMouseDownOn = enumHover;
    break;

  case SDL_MOUSEBUTTONUP:
    if (event.button.button == SDL_BUTTON_RIGHT) {
      Message("Color selection cancelled.");
      inputMode = IM_NORMAL;
    } else if (event.button.button == SDL_BUTTON_LEFT) {
      if ((enumHover != INVALID) && (enumHover == leftMouseDownOn)) {
        element = GetSelectElement(displaySet ? altSet : liveSet, displayCommand[thisHover].dataSource);
        SCOLOR(displaySet ? altSet : liveSet, element) = namedColors[enumHover].color;
        Message("Item %s set.",
          patternElements[element].name);
        inputMode = IM_NORMAL;
      }
    }
    break;
  default:
    if (event.type == User_Timer && event.user.code == TI_GUI) {
      // Handle drawing the input selection boxes.
      DrawColorSelectBox(thisHover, enumHover, &targets);
    }
    break;
}  // End event processing switch for COLOR selection
break;  // End case IM_COLOR
