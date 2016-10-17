// Integer Entry

  // User input an integer value.
case IM_INT:
switch(event.type) {
  case SDL_KEYDOWN:
    switch(HandleTextEntry(event.key.keysym.sym, event.key.keysym.mod, textEntry, sizeof(textEntry))) {
      case TE_ACCEPTED:
        if (strlen(textEntry) != 0) {
          errno = 0;
          value = strtol(textEntry, &testptr, 0);
          if ((errno == 0) && (testptr - strlen(textEntry) == textEntry)) {
            element = displayCommand[thisHover].dataSource;
            if (element < PE_INVALID) element = GetSelectElement(displaySet ? altSet : liveSet, element);
            if (SetValueInt(displaySet ? altSet : liveSet, element, value)) {
              Message("New value accepted.");
            } else {
              Message("New value rejected.");
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
}
break; // End case IM_INT
