// FLOAT

// User input an integer value.
case IM_FLOAT:
switch(event.type) {
  case SDL_KEYDOWN:
    switch(HandleTextEntry(event.key.keysym.sym, event.key.keysym.mod, textEntry, sizeof(textEntry))) {
      case TE_ACCEPTED:
        if (strlen(textEntry) != 0) {
          errno = 0;
          valuef = strtod(textEntry, &testptr);
          if ((errno == 0) && (testptr - strlen(textEntry) == textEntry)) {
            if (valuef != valuef) {
              Message("That's not a number.");
            } else {
              element = displayCommand[thisHover].dataSource;
              if (element < PE_INVALID) element = GetSelectElement(displaySet ? altSet : liveSet, element);
              SetValueFloat(displaySet ? altSet : liveSet, element,  valuef);
            }
          } else {
            Message("New value rejected.");
          }
        } else {
          Message("No value entered.");
        }
        inputMode = IM_NORMAL;
        break;
      case TE_REJECTED:
        inputMode = IM_NORMAL;
        Message("Edit cancelled.");
        break;
      case TE_NOTHING:
      default:
        break;
    }
    break;
  case SDL_MOUSEBUTTONUP:
    Message("Edit cancelled.");
    inputMode = IM_NORMAL;
    break;
  default:
    if (event.type == User_Timer && event.user.code == TI_GUI) {
      DrawTextEntryBox(thisHover, textEntry);
    }
    break;
} // End event processing switch for FLOAT entry.
break; // End case IM_FLOAT
