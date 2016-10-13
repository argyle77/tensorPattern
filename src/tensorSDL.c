// Tensor pattern generator.
// For Kevin "FrostByte" McCormick (w/o you, there may be nothing) <3
// Blau
// tensor@core9.org

#define main_c_

// Includes
#include <stdio.h>   // File io, setvbuf
#include <stdlib.h>  // exit conditions, string conversions, malloc
#include <unistd.h>  // Don't know what I used from here?  Maybe stderr?
#include <string.h>  // memcpy, strtok
#include <time.h>    // nanosleep
#include <errno.h>   // errno for string conversions.
#include <limits.h>  // INT_MIN
#include <SDL.h>
#include <SDL2_rotozoom.h>


#include "useful.h"
#include "badOldDefines.h"
#include "gui.h"
#include "version.h"
#include "draw.h"
#include "elements.h"
#include "layout.h"
#include "machine.h"
#include "transforms.h"
#include "my_font.h"


// Definitions
#define GUIFRAMEDELAY 25


// Gui input modes
typedef enum inputMode_e {
  IM_INVALID = -1,
  IM_NORMAL = 0, IM_CONFIRM, IM_INT, IM_FLOAT, IM_ENUM, IM_COLOR,
  IM_COUNT // Last
} inputMode_e;

// Text entry results
typedef enum textEntry_e {
  TE_INVALID = -1,
  TE_NOTHING = 0, TE_ACCEPTED, TE_REJECTED,
  TE_COUNT // Last
} textEntry_e;

// Globals - We do love our globals.  My guess is their proliferation is the
// sign of a weakly structured program.


bool_t cyclePatternSets = NO;  // cyclePatternSets mode is a global (for now).
int cycleFrameCount = INITIAL_FRAMECYCLECOUNT;    // Frame Count for cycling.


operateOn_e displaySet = OO_CURRENT;

int previewFrameCountA = 0, previewFPSA = 0;
int previewFrameCountB = 0, previewFPSB = 0;
int updateFrameCount = 0, updateFPS = 0;
int infoFrameCount = 0, infoFPS = 0;


Uint32 FPSEventType, DRAWEventTypeA, DRAWEventTypeB, GUIEventType, CONFIRMEventType, UPDATEEventType;
char statusText[100] = "";  // For confirming actions.
char keySave;
int setSave;
SDL_Rect liveBox, altBox;


int currentSet = 0;
int alternateSet = 1;



// Externs
extern int tensor_landscape_p;

// Prototypes
Uint32 TriggerFrameDrawA(Uint32 interval, void *param);
Uint32 TriggerFrameDrawB(Uint32 interval, void *param);
Uint32 TriggerFrameUpdate(Uint32 interval, void *param);
Uint32 TriggerFrameCount(Uint32 interval, void *param);
Uint32 TriggerGUIUpdate(Uint32 interval, void *param);


void SavePatternSet(char key, int set, bool_t overWrite, const char *backupName);
void LoadPatternSet(char key, int set, bool_t backup);

bool_t HandleCommand(int set, command_e command, int selection);
bool_t HandleKey(int set, SDL_Keycode key, SDL_Keymod mod);
bool_t HandleConfirmation(SDL_Keycode key, bool_t *selected);
inputMode_e EditValue(int set, int commandToEdit);
textEntry_e HandleTextEntry(SDL_Keycode key, SDL_Keymod mod, char * textEntry, int maxTextSize);
bool_t HandleEnumSelect(SDL_Keycode key, int set, int item, int *selected);
void CopyPatternSet(int dst, int src);
void CopyBuffer(int dst, int set);
void SwitchToSet(int set);

point_t GetDisplayPixel(point_t mouse, SDL_Rect box);

bool_t SetValueInt(int set, patternElement_e element, int value);
void SetValueFloat(int set, patternElement_e element, float value);
int OverCommand(point_t mouse, int *lastHover);
int OverBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover);
SDL_Rect GetBoxofCommand(int command);

int HandleColorSelection(point_t mouse, int thisHover, bool_t resetEvent, int set);
void UpdateInfoDisplay(int set);
int OverColorBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover);
bool_t HandleColorSelect(SDL_Keycode key, int set, int item, int *selected);

void DrawAndCache(int *cachePosition, elementType_e type, void *value, int row, int col, int width);
void DrawAndCacheString(int *cachePosition, const char *value, int row, int col, color_t fg, color_t bg);

// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  int i;
  char textEntry[100];
  SDL_Event event;
  int thisHover = INVALID;
  int lastHover = INVALID;
  SDL_Rect box2 = {0, 0, 0, 0}, boxOld2 = {0, 0, 0, 0};
  SDL_Rect boxYes, boxNo;
  point_t mouse;
  int leftMouseDownOn = INVALID;
  bool_t leftMouseDown = NO;
  int rightMouseDownOn = INVALID;
  bool_t rightMouseDown = NO;
  bool_t refreshAll = NO;
  bool_t refreshGui = NO;
  bool_t updateDisplays = NO;
  bool_t refreshPreviewBorders = NO;
  bool_t confirmed = NO;
  bool_t drawNewFrameA = NO;
  bool_t drawNewFrameB = NO;
  bool_t updateEvent = NO;
  bool_t exitProgram = NO;
  inputMode_e inputMode = IM_NORMAL;
  inputMode_e oldInputMode = IM_INVALID;
  int value;
  float valuef;
  char *testptr;
  int enumHover = INVALID;
  SDL_Rect *targets = NULL;
  operateOn_e displaySetOld = OO_INVALID;
  patternElement_e element;
  command_e command;
  color_e wheelColorF = 0, wheelColorB = 0;
  overPreview_e overBorder = OP_NO;
  overPreview_e overBorderDown = OP_NO;
  highLight_e liveHighlight = HL_SELECTED;
  highLight_e altHighlight = HL_NO;
  

  // Unbuffer the console...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  setvbuf(stderr, (char *)NULL, _IONBF, 0);

  // Print the version #.
  fprintf(stdout, "Tensor pattern generator v%s.%s.%s%s%s\n",
    MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION,
    strlen(PRERELEASE_VERSION) != 0 ? "." : "", PRERELEASE_VERSION);

  // Process commandline parameters.
  if (argc == 2) {
    
    // Print the help file.
    if ((strncmp(argv[1], "-h", 2) == 0) || (strncmp(argv[1], "--h", 3) == 0)) {
      fprintf(stdout, "%s -h|--help     - Print this help file.\n", argv[0]);
      fprintf(stdout, "%s [<intensity>] - Run the pattern generator at the optional output\n", argv[0]);
      fprintf(stdout, "  intensity specified by <intensity>.  Valid values for <intensity> range from\n");
      fprintf(stdout, "  %3.1f to %3.1f.  Default is %3.1f.\n", 
        GLOBAL_INTENSITY_LIMIT_MIN, GLOBAL_INTENSITY_LIMIT_MAX, GLOBAL_INTENSITY_LIMIT_DEFAULT);
      exit(EXIT_SUCCESS);
    }
    
    // Otherwise, we may have an intensity limiter.
    SetTensorLimit(atof(argv[1]));
  }

  // Verify the integrity of some of the data structures. Enforces consistency
  // for enumerated array access.  Errors here are programmer failures.
  if (!VerifyStructuralIntegrity()) exit(EXIT_FAILURE);

  // Allocate the pattern set memory
  if (!AllocatePatternData(PATTERN_SET_COUNT)) exit(EXIT_FAILURE);

  // Initialize the gui elements
  if (!InitGui(GUI_WINDOW_WIDTH, GUI_WINDOW_HEIGHT, DISPLAY_FONT_SIZE)) {
    fprintf(stderr, "Unable to initialize the gui elements!\n");
    exit(EXIT_FAILURE);
  }

  // Set the window title
  snprintf(textEntry, sizeof(textEntry), "Tensor Control - Output: %i%%", (int) (GetTensorLimit() * 100));
  SetWindowTitle(textEntry);

  
  // Initialize Tensor.
  InitTensor();

  
  // Initialize the simulator
  point_t live, alt;
  SetPoint(&live, PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y);
  SetPoint(&alt, PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y);
  InitMachine(TENSOR_BYTES, live, alt, &currentSet, &alternateSet);

  // Further patterData initializations
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    SINT(i, PE_TEXTOFFSET) = GetTensorHeight() / 3 - 1;  // After SetDims()
  }

  // Attempt to load startup pattern sets from disk.  These are the 0-9.now
  // files.  Its okay if they don't exist.  Otherwise, default will be loaded
  // into those pattern sets.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    LoadPatternSet(i + '0', i, NO);
    // Load the remaining image sets (in cases of pattern set load failure).
    //~ LoadImage(i);  // This doesn't make sense?  Unecessary.
  }

  cycleFrameCount = DINT(PE_FRAMECOUNT);


  // Bam - Show the (blank) preview.
  UpdateDisplay(YES, YES);

  // Add the text to the window
  DrawDisplayTexts(INVALID);

  // Initialize the user events
  FPSEventType = SDL_RegisterEvents(1);
  DRAWEventTypeA = SDL_RegisterEvents(1);
  DRAWEventTypeB = SDL_RegisterEvents(1);
  GUIEventType = SDL_RegisterEvents(1);
  CONFIRMEventType = SDL_RegisterEvents(1);
  UPDATEEventType = SDL_RegisterEvents(1);

  // Init the live preview timer.
  if (!SDL_AddTimer(DINT(PE_DELAY), TriggerFrameDrawA, NULL)) {
    fprintf(stderr, "Can't initialize the live preview timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Init the alternate preview timer.
  if (!SDL_AddTimer(SINT(alternateSet, PE_DELAY), TriggerFrameDrawB, NULL)) {
    fprintf(stderr, "Can't initialize the alternate preview timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  
  // Init the update timer.
  if (!SDL_AddTimer(60, TriggerFrameUpdate, NULL)) {
    fprintf(stderr, "Can't initialize the frame update timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // FPS counter.
  if (!SDL_AddTimer(1000, TriggerFrameCount, NULL)) {
    fprintf(stderr, "Can't initialize the frame count timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // GUI update timer. The gui is updated when the preview frames update, but
  // frame update speeds are adjustable. To ensure that the gui stays responsive
  // between frames, there is a timer for this purpose.  Current setting places
  // this timer at 40Hz.
  if (!SDL_AddTimer(GUIFRAMEDELAY, TriggerGUIUpdate, NULL)) {
    fprintf(stderr, "Can't initialize the gui update timer! %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Main program loop...
  FOREVER {
    
    bool_t okayToSleep = YES;

    // Act on queued events.
    while (SDL_PollEvent(&event)) {

      // TODO: Can we process <ctrl>-click and drag events?
      // TODO: Can we modularize the transformations menus so they can be
      //       scrolled or paged through?
      // Carry out actions common to all input modes  COMMON ACTIONS
      switch(event.type) {
        case SDL_MOUSEBUTTONUP:
          // Button up.  Mark the buttons as being up.
          if (event.button.button == SDL_BUTTON_LEFT) leftMouseDown = NO;
          else if (event.button.button == SDL_BUTTON_RIGHT) rightMouseDown = NO;
          break;
        case SDL_MOUSEMOTION:
          // Mouse moved. See where it is.
          mouse.x = event.motion.x;
          mouse.y = event.motion.y;
          break;
        case SDL_MOUSEBUTTONDOWN:
          // Mouse button pushed.  Make a note of the item it was pushed over.
          if (event.button.button == SDL_BUTTON_LEFT) {
            leftMouseDownOn = thisHover;
            leftMouseDown = YES;
          } else if (event.button.button == SDL_BUTTON_RIGHT) {
            rightMouseDownOn = thisHover;
            rightMouseDown = YES;
          }
          break;
        case SDL_QUIT:
          // Window closed or <ctrl> c pushed in terminal.  Exit program.
          exitProgram = YES;
          break;
        case SDL_WINDOWEVENT:
          // We care about the window events that destroy our display.  Those
          // are probably resize and expose events.  We'll redraw the whole
          // display if they occur.
          if ((event.window.event == SDL_WINDOWEVENT_EXPOSED) ||
              (event.window.event == SDL_WINDOWEVENT_RESIZED)) {
            refreshAll = YES;
          } else {
            //~ fprintf(stderr, "Unhandled SDL window event: %i\n", event.window.event);
          }
          break;
        default:
          // There are five registered user events to check for here. They can't
          // be tested in the case statement because they are not compile-time
          // integer constants.
          if (event.type == DRAWEventTypeA) {
            // The frame timer expired.  Set a new frame to draw.
            drawNewFrameA = YES;

          } else if (event.type == DRAWEventTypeB) {
            // The alternate frame timer expired.  Set a new frame to draw.
            drawNewFrameB = YES;
            
          } else if (event.type == UPDATEEventType) {
            updateEvent = YES;

          } else if (event.type == FPSEventType) {
            // The fps timer expired.  Set fps and reset the frame count.
            previewFPSA = previewFrameCountA;
            previewFPSB = previewFrameCountB;
            updateFPS = updateFrameCount;
            infoFPS = infoFrameCount;
            infoFrameCount = 0;
            previewFrameCountA = 0;
            previewFrameCountB = 0;
            updateFrameCount = 0;
            // fprintf(stdout, "Live: %i, Alt: %i, Update: %i, Gui: %i\n", previewFPSA, previewFPSB, updateFPS, infoFPS);          

          } else if (event.type == GUIEventType) {
            // Update the informational display.
            UpdateInfoDisplay(displaySet ? alternateSet : currentSet);
            infoFrameCount++;
            refreshGui = YES;
            //~ refreshAll = YES;  // EMERGENCY
            if (displaySetOld != displaySet) {
              displaySetOld = displaySet;
              refreshAll = YES;
            }

          } else if (event.type == CONFIRMEventType) {

            inputMode = IM_CONFIRM;
          } else {
            //~ fprintf(stderr, "Unhandled SDL event: %i\n", event.type);
          }
          break;
      } // End common actions event processing switch.

      // Carry out actions specific to the input mode
      switch(inputMode) {
        // Confirmation box events
        case IM_CONFIRM:

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
                    snprintf(statusText, sizeof(statusText), "Save action cancelled.");
                  }
                  inputMode = IM_NORMAL;
                }
              }
              break;

            default:
              if (event.type == GUIEventType) {
                DrawConfirmationBox(&boxYes, &boxNo, confirmed, statusText);
              }
              break;
          }  // End event processing switch
          break;

        // Normal mode events
        case IM_NORMAL:
          switch(event.type) {
            case SDL_KEYDOWN:
              exitProgram = HandleKey(displaySet ? alternateSet : currentSet, event.key.keysym.sym, event.key.keysym.mod);
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
                  HandleCommand(displaySet ? alternateSet : currentSet, command, thisHover);

                // Wheel up.
                } else {
                  command = displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command;
                  if (command == COM_NONE) {
                    command = displayCommand[thisHover].commands[MOUSE_EDIT].command;
                  }
                  HandleCommand(displaySet ? alternateSet : currentSet, command, thisHover);
                }

              // Mouse wheel action over the live preview lets us change colors
              // without having to go to the command.
              } else if (IsInsideBox(mouse, liveBox)) {
                if (!rightMouseDown) {
                  if (event.wheel.y < 0) { // Wheel down
                    wheelColorF--;
                    if (wheelColorF < 0) wheelColorF = CE_COUNT - 1;
                    SCOLOR(currentSet, PE_LP_COLA) = namedColors[wheelColorF].color;
                  } else { // Wheel up
                    wheelColorF++;
                    if (wheelColorF >= CE_COUNT) wheelColorF = 0;
                    SCOLOR(currentSet, PE_LP_COLA) = namedColors[wheelColorF].color;
                  }
                } else {
                  // Holding down the right mouse button while wheeling changes the
                  // background color instead.
                  if (event.wheel.y < 0) { // Wheel down
                    wheelColorB--;
                    if (wheelColorB < 0) wheelColorB = CE_COUNT - 1;
                    SCOLOR(currentSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
                  } else { // Wheel up
                    wheelColorB++;
                    if (wheelColorB >= CE_COUNT) wheelColorB = 0;
                    SCOLOR(currentSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
                  }
                }
              // Same for the alternate preview, except we change the alternate
              // set values.
              } else if (IsInsideBox(mouse, altBox)) {
                if (!rightMouseDown) {
                   if (event.wheel.y < 0) { // Wheel down
                    wheelColorF--;
                    if (wheelColorF < 0) wheelColorF = CE_COUNT - 1;
                    SCOLOR(alternateSet, PE_LP_COLA) = namedColors[wheelColorF].color;
                  } else { // Wheel up
                    wheelColorF++;
                    if (wheelColorF >= CE_COUNT) wheelColorF = 0;
                    SCOLOR(alternateSet, PE_LP_COLA) = namedColors[wheelColorF].color;
                  }
                } else {
                  // Holding down the right mouse button while wheeling changes the
                  // background color instead.
                  if (event.wheel.y < 0) { // Wheel down
                    wheelColorB--;
                    if (wheelColorB < 0) wheelColorB = CE_COUNT - 1;
                    SCOLOR(alternateSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
                  } else { // Wheel up
                    wheelColorB++;
                    if (wheelColorB >= CE_COUNT) wheelColorB = 0;
                    SCOLOR(alternateSet, PE_LP_BGCOLA) = namedColors[wheelColorB].color;
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
                  if (!HandleColorSelection(mouse, thisHover, YES, displaySet ? alternateSet : currentSet)) {
                    exitProgram = HandleCommand(displaySet ? alternateSet : currentSet,
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
                  if (HandleColorSelection(mouse, thisHover, NO, displaySet ? alternateSet : currentSet)) {
                    // For edit clicks on the color boxes, we set the color
                    // selector to the appropriate color source.
                    SENUM(displaySet ? alternateSet : currentSet, PE_PALETTEALTER) =
                      displayCommand[thisHover].colorSource;
                  } else {
                    // Otherwise, not on a color box, we execute the edit
                    // command (which may be nothing), and possibly change the
                    // input mode.
                    exitProgram = HandleCommand(displaySet ? alternateSet : currentSet,
                        displayCommand[thisHover].commands[MOUSE_EDIT].command,
                        thisHover);

                    // Change input mode?
                    inputMode = EditValue(currentSet, thisHover);
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
                      if (element < PE_INVALID) element = GetSelectElement(displaySet ? alternateSet : currentSet, element);
                      if (SetValueInt(displaySet ? alternateSet : currentSet, element, value)) {
                        snprintf(statusText, sizeof(statusText), "New value accepted.");
                      } else {
                        snprintf(statusText, sizeof(statusText), "New value rejected.");
                      }
                    } else {
                      snprintf(statusText, sizeof(statusText), "New value rejected.");
                    }
                  } else {
                    snprintf(statusText, sizeof(statusText), "No value entered.");
                  }
                  inputMode = IM_NORMAL;
                  break;
                case TE_REJECTED:
                  inputMode = IM_NORMAL;
                  snprintf(statusText, sizeof(statusText), "Edit cancelled.");
                  break;
                case TE_NOTHING:
                default:
                  break;
              }
              break;
            case SDL_MOUSEBUTTONUP:
              snprintf(statusText, sizeof(statusText), "Edit cancelled.");
              inputMode = IM_NORMAL;
              break;
            default:
              if (event.type == GUIEventType) {
                DrawTextEntryBox(thisHover, textEntry);
              }
              break;
          }
          break; // End case IM_INT

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
                        snprintf(statusText, sizeof(statusText), "That's not a number.");
                      } else {
                        element = displayCommand[thisHover].dataSource;
                        if (element < PE_INVALID) element = GetSelectElement(displaySet ? alternateSet : currentSet, element);
                        SetValueFloat(displaySet ? alternateSet : currentSet, element,  valuef);
                      }
                    } else {
                      snprintf(statusText, sizeof(statusText), "New value rejected.");
                    }
                  } else {
                    snprintf(statusText, sizeof(statusText), "No value entered.");
                  }
                  inputMode = IM_NORMAL;
                  break;
                case TE_REJECTED:
                  inputMode = IM_NORMAL;
                  snprintf(statusText, sizeof(statusText), "Edit cancelled.");
                  break;
                case TE_NOTHING:
                default:
                  break;
              }
              break;
            case SDL_MOUSEBUTTONUP:
              snprintf(statusText, sizeof(statusText), "Edit cancelled.");
              inputMode = IM_NORMAL;
              break;
            default:
              if (event.type == GUIEventType) {
                DrawTextEntryBox(thisHover, textEntry);
              }
              break;
          } // End event processing switch for FLOAT entry.
          break; // End case IM_FLOAT

        case IM_ENUM:
          switch(event.type) {
            case SDL_KEYDOWN:
              // Put some stuff here.
              if (HandleEnumSelect(event.key.keysym.sym,
                displaySet ? alternateSet : currentSet, thisHover, &enumHover))
                inputMode = IM_NORMAL;
              break;
            case SDL_MOUSEMOTION:
              // Check if its hovering over a command.
              enumHover = OverBox(displaySet ? alternateSet : currentSet, mouse, thisHover, &targets, &lastHover);
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
                  if (element < PE_INVALID) element = GetSelectElement(displaySet ? alternateSet : currentSet, element);
                  SENUM(displaySet ? alternateSet : currentSet, element) = enumHover;
                  snprintf(statusText, sizeof(statusText), "Item %s set to %s",
                    displayCommand[thisHover].text,
                    enumerations[patternElements[element].etype].texts[enumHover]);
                  inputMode = IM_NORMAL;
                }
              } else if (event.button.button == SDL_BUTTON_RIGHT) {
                snprintf(statusText, sizeof(statusText), "Item selection cancelled.");
                inputMode = IM_NORMAL;
              }
              break;  // End SDL_MOUSEBUTTONUP case.
            default:
              if (event.type == GUIEventType) {
                // Handle drawing the input selection boxes.
                DrawEnumSelectBox(displaySet ? alternateSet : currentSet, thisHover, enumHover, &targets);
              }
              break;
          } // End event processing switch for ENUM selection
          break; // End case IM_ENUM

        // Handle color selection dialog
        case IM_COLOR:
          switch(event.type) {
            case SDL_KEYDOWN:
              // Put some stuff here.
              if (HandleColorSelect(event.key.keysym.sym,
                displaySet ? alternateSet : currentSet, thisHover, &enumHover))
                inputMode = IM_NORMAL;
              break;
            case SDL_MOUSEMOTION:
              // Check if its hovering over a command.
              enumHover = OverColorBox(displaySet ? alternateSet : currentSet, mouse, thisHover, &targets, &lastHover);
              break;  // End case SDL_MOUSEMOTION.
            case SDL_MOUSEBUTTONDOWN:
              // Correct the error made by the common code.
              if (event.button.button == SDL_BUTTON_LEFT) leftMouseDownOn = enumHover;
              break;

            case SDL_MOUSEBUTTONUP:
              if (event.button.button == SDL_BUTTON_RIGHT) {
                snprintf(statusText, sizeof(statusText), "Color selection cancelled.");
                inputMode = IM_NORMAL;
              } else if (event.button.button == SDL_BUTTON_LEFT) {
                if ((enumHover != INVALID) && (enumHover == leftMouseDownOn)) {
                  element = GetSelectElement(displaySet ? alternateSet : currentSet, displayCommand[thisHover].dataSource);
                  SCOLOR(displaySet ? alternateSet : currentSet, element) = namedColors[enumHover].color;
                  snprintf(statusText, sizeof(statusText), "Item %s set.",
                    patternElements[element].name);
                  inputMode = IM_NORMAL;
                }
              }
              break;
            default:
              if (event.type == GUIEventType) {
                // Handle drawing the input selection boxes.
                DrawColorSelectBox(thisHover, enumHover, &targets);
              }
              break;
          }  // End event processing switch for COLOR selection
          break;  // End case IM_COLOR

        // Unhandled input modes
        default:
          inputMode = IM_NORMAL;
          break;
      } // End input mode specific switch
    } // End event polling while loop.

    // Are we trying to draw on the previews with the mouse?  We do this here
    // because we want to draw whether or not there were mouse events.
    point_t tpixel;
    if (IsInsideBox(mouse, liveBox)) {
      // Mouse hovering over the live preview.  Which display pixel?
      tpixel = GetDisplayPixel(mouse, liveBox);

      // Is one of the mouse buttons down?
      if (leftMouseDown) {
        LightPen(tpixel, currentSet, FBS_FG, SBUFFER(currentSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        LightPen(tpixel, currentSet, FBS_BG, SBUFFER(currentSet, PE_FRAMEBUFFER));
      }
    } else if (IsInsideBox(mouse, altBox)) {
      // The alternate display.
      tpixel = GetDisplayPixel(mouse, altBox);

      if (leftMouseDown) {
        LightPen(tpixel, alternateSet, FBS_FG, SBUFFER(alternateSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        LightPen(tpixel, alternateSet, FBS_BG, SBUFFER(alternateSet, PE_FRAMEBUFFER));
      }
    }
    
    // Draw a new frame if the timer expired for the live set.
    if (drawNewFrameA) {
      drawNewFrameA = NO;
      previewFrameCountA++;
      DrawNextFrame(currentSet, YES);
      
      // This is for if the broadcast clock is to match the frame clock.
      UpdateDisplay(YES, YES);
      refreshGui = YES;      

      // Deal with the pattern set cycle timer
      if (cyclePatternSets) {
        cycleFrameCount--;
        if (cycleFrameCount < 0) {
          currentSet = (currentSet + 1) % PATTERN_SET_COUNT;
          cycleFrameCount = DINT(PE_FRAMECOUNT);
        }
        if (cycleFrameCount > DINT(PE_FRAMECOUNT)) {
          cycleFrameCount = DINT(PE_FRAMECOUNT);
        }
      }    
    }
    
    // Draw a new frame if the timer expired for the preview set.
    if (drawNewFrameB) {
      drawNewFrameB = NO;
      previewFrameCountB++;
      DrawNextFrame(alternateSet, NO);
      
      // If broadcast clock matches frame clock.
      UpdateDisplay(NO, NO);
      refreshGui = YES;
    }    
    
    if (updateEvent) {
      updateEvent = NO;
      updateFrameCount++;
      //UpdateDisplay(YES, YES, global_intensity_limit);
      //UpdateDisplay(NO, NO, global_intensity_limit);
      //refreshGui = YES;      
    }

    // If the input mode changed, we need to refresh the display
    if (inputMode != oldInputMode) {
      oldInputMode = inputMode;
      lastHover = INVALID;
      confirmed = NO;
      refreshAll = YES;
      enumHover = INVALID;
      if (inputMode == IM_NORMAL) thisHover = INVALID;
    }

    // Check for whole gui refresh.
    if (refreshAll) {
      refreshAll = NO;
      DrawClearWindow();
      DrawDisplayTexts(thisHover);
      UpdateInfoDisplay(displaySet ? alternateSet : currentSet);
      refreshPreviewBorders = YES;
      refreshGui = YES;
    }

    // Check for preview border refresh
    if (refreshPreviewBorders) {
      refreshPreviewBorders = NO;
      if (liveHighlight != HL_HOVER) {
        liveHighlight = (displaySet == OO_CURRENT ? HL_SELECTED : HL_NO);
      }
      if (altHighlight != HL_HOVER) {
        altHighlight = (displaySet == OO_ALTERNATE ? HL_SELECTED : HL_NO);
      }
      DrawPreviewBorder(live, liveHighlight);
      DrawPreviewBorder(alt, altHighlight);
      
      updateDisplays = YES;
      refreshGui = YES;
    }      
    
    // Check for display update.
    if (updateDisplays) {
      updateDisplays = NO;
      UpdateDisplay(YES, NO);
      UpdateDisplay(NO, NO);
      refreshGui = YES;
    }
      
    // Check for info display refresh.
    if (refreshGui) {
      refreshGui = NO;
      okayToSleep = NO;
      UpdateGUI();
    }
    
    // Sleep the processor if we've done little.  The CPU can spin if we get 
    // stuck in a tight loop here.  Best to get out of the way for a bit.
    if (okayToSleep) {
      nanosleep((struct timespec[]) {{0,1000000}}, NULL);  // Idle the processor for 1 ms.
    }
    
    // Exit the program loop.
    if (exitProgram) break;

  } // End FOREVER program loop

  // Save the pattern sets to make them persistent across restarts.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    // Note, we used to save them as backups, but saving them directly makes
    // them persistent (since they're auto-loaded at startup).
    // SavePatternSet('0' + i, i, YES, "closebak");
    SavePatternSet('0' + i, i, YES, "");
  }

  // Clean up and exit.
  QuitGui();
  exit(EXIT_SUCCESS);  // Is it?
}

// Event that triggers a frame to be drawn.
Uint32 TriggerFrameDrawA(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = DRAWEventTypeA;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(SINT(currentSet, PE_DELAY));
}

// Event that triggers a frame to be drawn.
Uint32 TriggerFrameDrawB(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = DRAWEventTypeB;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(SINT(alternateSet, PE_DELAY));
}

// Event that triggers a frame to be updated.
Uint32 TriggerFrameUpdate(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for frame drawing and push it to the queue.
  SDL_zero(event);
  event.type = UPDATEEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the next delay sets the timer to fire again.
  return(100);
}

// Event that triggers the fps to be updated.
Uint32 TriggerFrameCount(Uint32 interval, void *param) {
  SDL_Event event;

  // Make a new event for fps measurement, and push it to the queue.
  SDL_zero(event);
  event.type = FPSEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the interval starts the timer again for the same period.
  return(interval);
}

// Gui update timer event pusher
Uint32 TriggerGUIUpdate(Uint32 interval, void *param) {
    SDL_Event event;

  // Make a new event for fps measurement, and push it to the queue.
  SDL_zero(event);
  event.type = GUIEventType;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  // Returning the interval starts the timer again for the same period.
  return(interval);
}


// Key press processing for confirmation boxes.
bool_t HandleConfirmation(SDL_Keycode key, unsigned char *selected) {
  bool_t confirmationHandled = YES;

  switch (key) {
    case SDLK_y:
      SavePatternSet(keySave, setSave, YES, "");
      break;

    case SDLK_n:
    case SDLK_ESCAPE:
      snprintf(statusText, sizeof(statusText), "Save action cancelled.");
      break;

    case SDLK_RETURN:
    case SDLK_SPACE:
      if (*selected == YES) {
        SavePatternSet(keySave, setSave, YES, "");
      } else {
        snprintf(statusText, sizeof(statusText), "Save action cancelled.");
      }
      break;

    case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
      *selected = !(*selected);
      confirmationHandled = NO;
      break;

    default:
      confirmationHandled = NO;
      break;
  }
  return confirmationHandled;
}

// Key press processing for enumeration selection boxes.
bool_t HandleEnumSelect(SDL_Keycode key, int set, int item, int *selected) {
  bool_t selectionDismissed = YES;
  int boxCount;
  int source = displayCommand[item].dataSource;
  if (source <= PE_INVALID) source = GetSelectElement(set, source);
  boxCount = enumerations[patternElements[source].etype].size;


  switch (key) {
    case SDLK_ESCAPE:
      snprintf(statusText, sizeof(statusText), "Selection cancelled.");
      break;

    case SDLK_RETURN:
    case SDLK_SPACE:
      SENUM(set, source) = *selected;
      snprintf(statusText, sizeof(statusText), "Item %s set to %s",
        displayCommand[item].text,
        enumerations[patternElements[source].etype].texts[*selected]);
      break;

    // They're laid out in a 2 by x grid.
    case SDLK_LEFT:
      (*selected)--;
      if (*selected >= boxCount) *selected = 0;
      if (*selected < 0) {
        (*selected)++;
        if (*selected < 0) (*selected) = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_UP:
      *selected -= 2;
      if (*selected >= boxCount) *selected = 0;
      if (*selected < 0) {
        *selected += 2;
        if (*selected < 0) *selected = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_DOWN:
      *selected += 2;
      if (*selected >= boxCount) {
        *selected -= 2;
        if (*selected >= boxCount) *selected = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_RIGHT:
      *selected += 1;
      if (*selected >= boxCount) {
        *selected -= 1;
        if (*selected >= boxCount) *selected = 0;
      }
      selectionDismissed = NO;
      break;

    default:
      selectionDismissed = NO;
      break;
  }
  return selectionDismissed;
}

// Key press processing.
bool_t HandleKey(int set, SDL_Keycode key, SDL_Keymod mod) {
  int i, j;

  // Prolly not necessary.
  if (key == SDLK_UNKNOWN) return NO;  // NO = Don't exit the program

  // To make exact comparisons to mod, left or right equivalents of modifier
  // keys must both be set if one is set.
  if (mod & KMOD_CTRL) mod |= KMOD_CTRL;
  if (mod & KMOD_ALT) mod |= KMOD_ALT;
  if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;

  // Check to see if the key combination activates a command.
  for ( i = 0 ; i < displayCommandCount; i++) {
    for (j = 0; j < MOUSE_COUNT; j++) {
      if ((displayCommand[i].commands[j].key == key) && (displayCommand[i].commands[j].mod == mod)) {
        return(HandleCommand(set, displayCommand[i].commands[j].command, i));
      }
    }
  }

  // Check other place for commands.
  for ( i = 0 ; i < otherCommandsCount; i++) {
    for (j = 0 ; j < MOUSE_COUNT; j++) {
      if ((otherCommands[i].commands[j].key == key) && (otherCommands[i].commands[j].mod == mod)) {
        return(HandleCommand(set, otherCommands[i].commands[j].command, i));
      }
    }
  }

  // Check for load or save to disk commands.
  // Save the current pattern set as <key>.now (for 0-9, a-z, only)
  if (mod == (KMOD_ALT | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      SavePatternSet(key, set, NO, "");
      return NO;  // NO = Don't exit the program
    }
  }

  // Load a pattern set from <key>.now into the current set.
  if (mod == (KMOD_CTRL | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      LoadPatternSet(key, set, YES);
      return NO;  // NO = Don't exit the program
    }
  }

  // If no command by now, then the key prolly goes in the text buffer.  First,
  // make sure we are dealing with printable characters:
  if ((key <= '~' && key >= ' ') && (!(mod & KMOD_ALT)) && (!(mod & KMOD_CTRL))) {
    if ((mod & KMOD_SHIFT) == KMOD_SHIFT) {
      // Keys with shift held down.
      if (key <= SDLK_z && key >= SDLK_a) {
        // Capitalize for a - z.
        SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = key - ('a' - 'A');
      } else {
        // Lookup the symbols for the rest of the keys.
        switch (key) {
          case SDLK_1: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '!'; break;
          case SDLK_2: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '@'; break;
          case SDLK_3: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '#'; break;
          case SDLK_4: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '$'; break;
          case SDLK_5: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '%'; break;
          case SDLK_6: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '^'; break;
          case SDLK_7: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '&'; break;
          case SDLK_8: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '*'; break;
          case SDLK_9: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '('; break;
          case SDLK_0: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = ')'; break;
          case SDLK_BACKSLASH: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '|'; break;
          case SDLK_BACKQUOTE: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '~'; break;
          case SDLK_MINUS: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '_'; break;
          case SDLK_EQUALS: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '+'; break;
          case SDLK_LEFTBRACKET: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '{'; break;
          case SDLK_RIGHTBRACKET: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '}'; break;
          case SDLK_SEMICOLON: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = ':'; break;
          case SDLK_COMMA: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '<'; break;
          case SDLK_PERIOD: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '>'; break;
          case SDLK_SLASH: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '?'; break;
          case SDLK_QUOTE: SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '"'; break;
          default: break;
        }
      }
    } else {
      // Unmodified key entry.  We'll treat them as ascii in the printable range.
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = key;
    }

    // Advance the terminating null and increase the buffer size.
    SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX) + 1] = 0x00;
    SINT(set, PE_TEXTINDEX)++;
    if (SINT(set, PE_TEXTINDEX) >= (patternElements[PE_TEXTBUFFER].size - 2)) SINT(set, PE_TEXTINDEX)--;
  }

  return NO;  // NO = Don't exit the program
}

// Executes a user command - toggles flags, adjusts parameters, etc.
bool_t HandleCommand(int set, command_e command, int selection) {
  int i;
  patternElement_e element;

  element = displayCommand[selection].dataSource;

  // To deal with disabled color selector elements:
  if (element < PE_INVALID) {
    element = GetSelectElement(set, element);
    if (element == PE_INVALID) {
      switch(command) {
        case COM_RST_FLOAT: case COM_INC_FLOAT: case COM_DEC_FLOAT: case COM_BOOL_FLIP:
        case COM_BOOL_RST: case COM_ENUM_RST: case COM_ENUM_INC: case COM_ENUM_DEC:
        case COM_INT_RST: case COM_INT_INC: case COM_INT_DEC: case COM_LINT_DEC:
        case COM_LINT_INC:
          return NO;
        default:
          break;
      }
    }
  }

  switch(command) {

    // First the common cases - Uncomplicated increments, decrements, and resets.
    case COM_RST_FLOAT: SFLOAT(set, element) = patternElements[element].initial.f; break;
    case COM_INC_FLOAT:
      SFLOAT(set, element) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, element) > patternElements[element].max.f)
        SFLOAT(set, element) = patternElements[element].max.f;
      break;
    case COM_DEC_FLOAT:
      SFLOAT(set, element) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, element) < patternElements[element].min.f)
        SFLOAT(set, element) = patternElements[element].min.f;
      break;
    case COM_BOOL_FLIP: SBOOL(set, element) = !SBOOL(set, element); break;
    case COM_BOOL_RST: SBOOL(set, element) = patternElements[element].initial.b; break;
    case COM_ENUM_RST: SENUM(set, element) = patternElements[element].initial.e; break;
    case COM_ENUM_INC:
      SENUM(set, element) = (SENUM(set, element) + 1) % enumerations[patternElements[element].etype].size;
      break;
    case COM_ENUM_DEC:
      SENUM(set, element) = (SENUM(set, element) - 1);
      if (SENUM(set, element) < 0)
        SENUM(set, element) = enumerations[patternElements[element].etype].size - 1;
      break;
    case COM_INT_RST: SINT(set, element) = patternElements[element].initial.i;  break;
    case COM_INT_INC:
      SINT(set, element)++;
      if (SINT(set, element) > patternElements[element].max.i)
        SINT(set, element) = patternElements[element].max.i;
      break;
    case COM_INT_DEC:
      SINT(set, element)--;
      if (SINT(set, element) < patternElements[element].min.i)
        SINT(set, element) = patternElements[element].min.i;
      break;
    // The lints are for the randomness vars.  Makes it easier to get through them.
    case COM_LINT_DEC:
      SINT(set, element) -= max(1, SINT(set, element) / 50);
      if (SINT(set, element) < patternElements[element].min.i)
        SINT(set, element) = patternElements[element].min.i;
      break;
    case COM_LINT_INC:
      SINT(set, element) += max(1, SINT(set, element) / 50);
      if (SINT(set, element) > patternElements[element].max.i)
        SINT(set, element) = patternElements[element].max.i;
      break;

    case COM_BROAD_FLIP: SetTensorEnable(!GetTensorEnable()); break;
    case COM_BROAD_RST: SetTensorEnable(YES); break;
    case COM_BLEND_RST: SetAltBlend(0.0); break;
    case COM_BLEND_INC:
      SetAltBlend(GetAltBlend() + SFLOAT(set, PE_FLOATINC));
      if (GetAltBlend() > 1.0) SetAltBlend(1.0);
      break;
    case COM_BLEND_DEC:
      SetAltBlend(GetAltBlend() - SFLOAT(set, PE_FLOATINC));
      if (GetAltBlend() < 0.0) SetAltBlend(0.0);
      break;
    case COM_BLENDINC_RST: SetAltBlendRate(0.01); break;
    case COM_BLENDINC_INC:
      SetAltBlendRate(GetAltBlendRate() + SFLOAT(set, PE_FLOATINC));
      if (GetAltBlendRate() > 1.0) SetAltBlendRate(1.0);
      break;
    case COM_BLENDINC_DEC:
      SetAltBlendRate(GetAltBlendRate() - SFLOAT(set, PE_FLOATINC));
      if (GetAltBlendRate() < 0.0) SetAltBlendRate(0.0);
      break;
    case COM_BLENDSWITCH: SetAutoBlend(!GetAutoBlend()); break;
    case COM_BLENDSWITCH_RST: SetAutoBlend(NO); break;
    case COM_EXCHANGE: i = currentSet; currentSet = alternateSet; alternateSet = i; break;
    case COM_OPERATE: displaySet = (displaySet + 1) % OO_COUNT; break;
    case COM_OPERATE_RST: displaySet = OO_CURRENT; break;
    case COM_ALTERNATE_INC: alternateSet = (alternateSet + 1) % PATTERN_SET_COUNT; break;
    case COM_ALTERNATE_DEC: alternateSet--; if (alternateSet < 0) alternateSet = PATTERN_SET_COUNT - 1; break;
    case COM_LIVE_INC: currentSet = (currentSet + 1) % PATTERN_SET_COUNT; break;
    case COM_LIVE_DEC: currentSet--; if (currentSet < 0) currentSet = PATTERN_SET_COUNT - 1; break;
    case COM_CYCLESET:
      cyclePatternSets = !cyclePatternSets;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_MODEOFF:
      for (i = 0; i < patternElementCount; i++) {
        if (patternElements[i].type == ET_BOOL) {
          SBOOL(set, i) = NO;
        }
        if (patternElements[i].type == ET_ENUM) {
          switch(i) {
            case PE_SHIFTBLUE: case PE_SHIFTCYAN: case PE_SHIFTGREEN:
            case PE_SHIFTMAGENTA: case PE_SHIFTRED: case PE_SHIFTYELLOW:
              SENUM(set, i) = SM_HOLD;
              break;
            case PE_TEXTMODE: case PE_SCROLLDIR:
              break;
            default:
              SENUM(set, i) = 0;
              break;
          }
        }
      }
      SINT(set, PE_BOUNCER) = 0;
      cyclePatternSets = NO;
      break;
    case COM_TEXTRESET: SINT(set, PE_PIXELINDEX) = INVALID; break;
    case COM_ORIENTATION:
      tensor_landscape_p = !tensor_landscape_p;
      SetDims();
      DrawPreviewBorder(GetLiveLoc(), (displaySet == OO_CURRENT ? HL_SELECTED : HL_NO));
      DrawPreviewBorder(GetAltLoc(), (displaySet == OO_ALTERNATE ? HL_SELECTED : HL_NO));
      break;
    case COM_LOADSET0: SwitchToSet(0); break;
    case COM_LOADSET1: SwitchToSet(1); break;
    case COM_LOADSET2: SwitchToSet(2); break;
    case COM_LOADSET3: SwitchToSet(3); break;
    case COM_LOADSET4: SwitchToSet(4); break;
    case COM_LOADSET5: SwitchToSet(5); break;
    case COM_LOADSET6: SwitchToSet(6); break;
    case COM_LOADSET7: SwitchToSet(7); break;
    case COM_LOADSET8: SwitchToSet(8); break;
    case COM_LOADSET9: SwitchToSet(9); break;
    case COM_COPYSET0: CopyPatternSet(0, set); break;
    case COM_COPYSET1: CopyPatternSet(1, set); break;
    case COM_COPYSET2: CopyPatternSet(2, set); break;
    case COM_COPYSET3: CopyPatternSet(3, set); break;
    case COM_COPYSET4: CopyPatternSet(4, set); break;
    case COM_COPYSET5: CopyPatternSet(5, set); break;
    case COM_COPYSET6: CopyPatternSet(6, set); break;
    case COM_COPYSET7: CopyPatternSet(7, set); break;
    case COM_COPYSET8: CopyPatternSet(8, set); break;
    case COM_COPYSET9: CopyPatternSet(9, set); break;
    case COM_LOAD0: CopyBuffer(set, 0); break;
    case COM_LOAD1: CopyBuffer(set, 1); break;
    case COM_LOAD2: CopyBuffer(set, 2); break;
    case COM_LOAD3: CopyBuffer(set, 3); break;
    case COM_LOAD4: CopyBuffer(set, 4); break;
    case COM_LOAD5: CopyBuffer(set, 5); break;
    case COM_LOAD6: CopyBuffer(set, 6); break;
    case COM_LOAD7: CopyBuffer(set, 7); break;
    case COM_LOAD8: CopyBuffer(set, 8); break;
    case COM_LOAD9: CopyBuffer(set, 9); break;
    case COM_TEXTO_DEC:
      SINT(set, PE_TEXTOFFSET)--;
      if (SINT(set, PE_TEXTOFFSET) < 0) {
        if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
          SINT(set, PE_TEXTOFFSET) = GetTensorHeight() - 1;
        } else {
          SINT(set, PE_TEXTOFFSET) = GetTensorWidth() - 1;
        }
      }
      break;
    case COM_TEXTO_INC:
      SINT(set, PE_TEXTOFFSET)++;
      if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
        SINT(set, PE_TEXTOFFSET) %= GetTensorHeight();
      } else {
        SINT(set, PE_TEXTOFFSET) %= GetTensorWidth();
      }
      break;
    case COM_TEXTO_RST:
      if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
        SINT(set, PE_TEXTOFFSET) = GetTensorHeight() / 2 - FONT_HEIGHT / 2;
      } else {
        SINT(set, PE_TEXTOFFSET) = GetTensorWidth() / 2 - FONT_HEIGHT / 2;
      }
      break;
    case COM_SCROLL_UPC:
      SENUM(set, PE_SCROLLDIR) = DIR_UP;
      SENUM(set, PE_SCROLL) = OS_ONESHOT;
      break;
    case COM_SCROLL_DOWNC:
      SENUM(set, PE_SCROLLDIR) = DIR_DOWN;
      SENUM(set, PE_SCROLL) = OS_ONESHOT;
      break;
    case COM_SCROLL_LEFTC:
      SENUM(set, PE_SCROLLDIR) = DIR_LEFT;
      SENUM(set, PE_SCROLL) = OS_ONESHOT;
      break;
    case COM_SCROLL_RIGHTC:
      SENUM(set, PE_SCROLLDIR) = DIR_RIGHT;
      SENUM(set, PE_SCROLL) = OS_ONESHOT;
      break;
    case COM_SCROLL_UPA:
      SENUM(set, PE_SCROLLDIR) = DIR_UP;
      SENUM(set, PE_SCROLL) = OS_YES;
      break;
    case COM_SCROLL_DOWNA:
      SENUM(set, PE_SCROLLDIR) = DIR_DOWN;
      SENUM(set, PE_SCROLL) = OS_YES;
      break;
    case COM_SCROLL_LEFTA:
      SENUM(set, PE_SCROLLDIR) = DIR_LEFT;
      SENUM(set, PE_SCROLL) = OS_YES;
      break;
    case COM_SCROLL_RIGHTA:
      SENUM(set, PE_SCROLLDIR) = DIR_RIGHT;
      SENUM(set, PE_SCROLL) = OS_YES;
      break;
    case COM_STEP_INC: SFLOAT(set, PE_FLOATINC) *= 10; break;
    case COM_STEP_DEC: SFLOAT(set, PE_FLOATINC) /= 10; break;
    case COM_SCROLL_UP: SENUM(set, PE_SCROLLDIR) = DIR_UP; break;
    case COM_SCROLL_DOWN: SENUM(set, PE_SCROLLDIR) = DIR_DOWN; break;
    case COM_SCROLL_LEFT: SENUM(set, PE_SCROLLDIR) = DIR_LEFT; break;
    case COM_SCROLL_RIGHT: SENUM(set, PE_SCROLLDIR) = DIR_RIGHT; break;
    case COM_EXIT: return YES;  // YES, exit the program.
    case COM_BACKSPACE:
      SINT(set, PE_TEXTINDEX)--;
      if (SINT(set, PE_TEXTINDEX) < 0) SINT(set, PE_TEXTINDEX) = 0;
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = 0x00;
      break;
    case COM_DELETE:
      SINT(set, PE_TEXTINDEX) = 0;
      SSTRING(set, PE_TEXTBUFFER)[0] = 0x00;
      break;
    case COM_FCOUNT_RST:
      SINT(set, PE_FRAMECOUNT) = patternElements[PE_FRAMECOUNT].initial.i;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_FCOUNT_INC:
      SINT(set, PE_FRAMECOUNT)++;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_FCOUNT_DEC:
      SINT(set, PE_FRAMECOUNT)--;
      if (SINT(set, PE_FRAMECOUNT) < 1) SINT(set, PE_FRAMECOUNT) = 1;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_INVALID: case COM_NONE: case COM_COUNT:
      break;
  }
  return NO;  // NO, don't exit the program.
}


// Saves a pattern set to a file.
// Return value indicates if the action requires confirmation.
void SavePatternSet(char key, int set, bool_t overWrite, const char *backupName) {
  char filename[20] = "";
  FILE *fp;
  int i,j;

  // Filename
  if (strlen(backupName) != 0) {
    snprintf(filename, sizeof(filename), "%c.now.%s", key, backupName);
    overWrite = YES;
  } else {
    snprintf(filename, sizeof(filename), "%c.now", key);
  }

  // Check file existence.
  if (!overWrite) {
    fp = fopen(filename, "r");
    if (fp) {
      fclose(fp);
      snprintf(statusText, sizeof(statusText), "Overwrite \"%s\" with set %i?", filename, set);
      setSave = set;
      keySave = key;
      SDL_Event event;
      SDL_zero(event);
      event.type = CONFIRMEventType;
      event.user.code = 0;
      event.user.data1 = 0;
      event.user.data2 = 0;
      SDL_PushEvent(&event);
      return;
    }
  }

  fprintf(stdout, "Save set %i to filename: %s\n", set, filename);

  // Open the file for write.
  fp = fopen(filename, "w");
  if (!fp) {
    snprintf(statusText, sizeof(statusText), "Failed to open file for output: %s", filename);
    fprintf(stderr, "%s\n", statusText);
    return;
  }

  // Write the version information to the file just in case we ever use this.
  fprintf(fp, "Tensor pattern parameter set file\n");
  fprintf(fp, "Version %s.%s.%s.%s\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, PRERELEASE_VERSION);

  // Write the file according to the structure
  for (i = 0; i < patternElementCount; i++) {
    //~ printf("Saving element %i - %s\n", i, patternElements[i].name);
    fprintf(fp, "%s ", patternElements[i].name);
    switch(patternElements[i].type) {
      case ET_BOOL:
        fprintf(fp, "%s", SBOOL(set, i) ? "YES" : "NO");
        break;
      case ET_INT:
        fprintf(fp, "%i", SINT(set, i));
        break;
      case ET_ENUM:
        fprintf(fp, "%s", enumerations[patternElements[i].etype].texts[SINT(set, i)]);
        break;
      case ET_FLOAT:
        fprintf(fp, "%f", SFLOAT(set, i));
        break;
      case ET_COLOR:
        fprintf(fp, "%i %i %i %i", SCOLOR(set, i).r, SCOLOR(set, i).g, SCOLOR(set, i).b, SCOLOR(set, i).a);
        break;
      case ET_STRING:
        fprintf(fp, "%s", SSTRING(set, i));
        break;
      case ET_BUFFER:
        for (j = 0; j < patternElements[i].size; j++) {
          fprintf(fp, "%02x ", SBUFFER(set, i)[j]);
        }
        break;
      default:
      case ET_INVALID:
      case ET_COUNT:
        fprintf(stderr, "Error!\n");
        break;
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  snprintf(statusText, sizeof(statusText), "Saved set %i to filename: %s", set, filename);
}

// Loads a pattern set from a file.
void LoadPatternSet(char key, int set, bool_t backup) {
  int currentSet = set; // Override global for D*.
  char filename[8] = "";
  FILE *fp;
  char *ikey = NULL;
  char *value = NULL;
  char *saveptr = NULL;
  char *tmp = NULL;
  int keySize = 0;
  int valueSize = 0;
  int keyCount = 0;
  int valueCount = 0;
  int inChar;
  int result;
  typedef enum processing_e { P_KEY, P_VALUE, P_EOL } processing_e;
  processing_e lookingFor = P_KEY;
  int i;
  int parameterIndex = INVALID;
  int z, y, x, w;
  color_t temp;
  char *token;
  unsigned char * bufferData;
  bool_t gotIt = NO;

  // First, create a backup.
  if (backup) SavePatternSet(key, set, YES, "loadbak");

  // Filename
  snprintf(filename, sizeof(filename), "%c.now", key);
  fprintf(stdout, "Load filename: %s into set %i\n", filename, currentSet);

  // Open file.
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    snprintf(statusText, sizeof(statusText), "Failed to open file \"%s\"", filename);
    return;
  }

  // Read file.  We'll do it character by character.  At the start of a line,
  // we read in characters until we hit a space or newline.  If its a space,
  // we'll check the read characters against known tokens.  If its recognized,
  // we'll read the rest of the line and process the input, otherwise we'll drop
  // the rest until the next newline.
  FOREVER {

    // Get a character from the file.
    inChar = fgetc(fp);

    // EOF? Done.
    if (inChar == EOF) break;

    // If we are currently looking for a key...
    if (lookingFor == P_KEY) {

      // A space denotes the end of a key.
      if (inChar == ' ') {

        // Key done, check it against the known keys.
        for (i = 0; i < patternElementCount; i++) {
          result = strcasecmp(ikey, patternElements[i].name);
          // If it was found, stop checking.
          if (result == 0) break;
        }

        if (result == 0) {
          // The key was found amongst our known keys.
          //~ printf("Key found (%i - %s) = \"%s\"\n", i, patternElements[i].name, ikey);
          parameterIndex = i;  // Save aside the parameter we're working on.
          keyCount = 0;  // Reset for next key processing.
          lookingFor = P_VALUE;  // Gather the parameter's value.

        } else {
          // This is an unknown key.
          //~ printf("Key unknown - \"%s\"\n", ikey);
          // Look for the end of the line, ignoring the rest of the characters.
          lookingFor = P_EOL;
          keyCount = 0;  // Reset for next key processing.
        }

      // A newline while looking for a key denotes a valueless key.  There are
      // none of these, so we'll start looking for the next key.
      } else if (inChar == '\n') {
        //~ printf("Key, no value: \"%s\"\n", ikey);
        keyCount = 0;
        lookingFor = P_KEY;

      // Any other letter is part of the key we are looking for.  Add it to the
      // key string.
      } else {

        // See if there's room.
        keyCount++;
        if (keyCount >= keySize) {
          //~ printf("Allocating key memory. Initial size: %i ", keySize);
          keySize+=100;  // Allocate memory in chunks.
          //~ printf("New size: %i\n", keySize);
          ikey = realloc(ikey, keySize * sizeof(char));
          if (!ikey) {
            fprintf(stderr, "Unable to allocate key memory!\n");
            break;
          }
        }

        // Add the character on to the end of the string and terminate it.
        ikey[keyCount - 1] = inChar;
        ikey[keyCount] = '\0';
      }

    // Looking for the end of line, but ignoring the rest of the text.
    } else if (lookingFor == P_EOL) {
      if (inChar == '\n') {
        lookingFor = P_KEY;
      }

    // Looking for the value
    } else {

      // Newline denotes end of the value string.  Process the result.
      if (inChar == '\n') {
        //~ printf("Value of %s (type %i) = \"%s\"\n", patternElements[parameterIndex].name, patternElements[parameterIndex].type, value);
        // Process the result by parameter type.
        switch(patternElements[parameterIndex].type) {
          // TODO Range checking for ET_ENUM (and others).
          case ET_INT:
            DINT(parameterIndex) = atoi(value);
            break;
          case ET_ENUM:
            // Must make a string comparison to the texts of the enumeration values.
            // I want to shave off trailing white space, and make the comparison
            // without case.
            // Strip the white space off the end.
            for (i = strlen(value) - 1; i >= 0; i--) {
              if (value[i] == 32) value[i] = '\0';
              else if (value[i] == '\t') value[i] = '\0';
              else break;  // Any other value means we are no longer looking for trailing whitespace.
            }
            // Now we set the enum value by doing comparing the string to known values.
            gotIt = NO;
            for (i = 0; i < enumerations[patternElements[parameterIndex].etype].size; i++) {
              if (strcasecmp(value, enumerations[patternElements[parameterIndex].etype].texts[i]) == 0) {
                DENUM(parameterIndex) = i;
                gotIt = YES;
                break;
              }
            }
            // If we didn't find a legit value, this may be a legacy file.  In
            // that case, let's do a string conversion.
            if (!gotIt) {
              DENUM(parameterIndex) = atoi(value);
            }
            // Correct for out of range.  Out of range would segfault the program.
            if (DENUM(parameterIndex) < 0 || DENUM(parameterIndex) >= enumerations[patternElements[parameterIndex].etype].size)
              DENUM(parameterIndex) = 0;
            break;
          case ET_BOOL:
            if (strncasecmp(value, "YES", 3) == 0) {
              DBOOL(parameterIndex) = YES;
            } else if (strncasecmp(value, "NO", 2) == 0) {
              DBOOL(parameterIndex) = NO;
            } else {
              fprintf(stderr, "Ignoring invalid value for boolean parameter %s. (\"%s\")\n", patternElements[parameterIndex].name, value);
            }
            break;
          case ET_FLOAT:
            DFLOAT(parameterIndex) = atof(value);
            break;
          case ET_STRING:
            strncpy(DSTRING(parameterIndex), value, patternElements[parameterIndex].size);
            // Just in case...
            DSTRING(parameterIndex)[patternElements[parameterIndex].size - 1] = '\0';
            break;
          case ET_COLOR:
            z = y = x = w = INVALID;
            if (sscanf(value, "%i %i %i %i", &z, &y, &x, &w) == 4) {
              if (z >= 0 && z <= 255 && y >= 0 && y <= 255 &&
                  x >= 0 && x <= 255 && w >= 0 && w <= 255) {
                temp.r = z;
                temp.g = y;
                temp.b = x;
                temp.a = w;
                DCOLOR(parameterIndex) = temp;
              } else {
                fprintf(stderr, "Ignoring inappropriate format for color of %s: \"%s\"\n", patternElements[parameterIndex].name, value);
              }
            } else {
              fprintf(stderr, "Ignoring inappropriate format for color of %s: \"%s\"\n", patternElements[parameterIndex].name, value);
            }
            break;
          case ET_BUFFER:
            // Count the input bytes, check for valid values, save aside the data.
            tmp = value;
            y = w = 0;
            token = strtok_r(tmp, " ", &saveptr);
            bufferData = malloc(patternElements[parameterIndex].size * sizeof(unsigned char));
            if (!bufferData) {
              fprintf(stderr, "Couldn't allocate space for buffer input!\n");
              break;
            }
            while (token) {
              //~ printf("Process token %s as #%i\n", token, y);
              z = INVALID;
              x = sscanf(token, "%x", &z);
              if (z >= 0 && z <= 255 && x == 1) {
                bufferData[y] = z;
                y++;
                if (y >= patternElements[parameterIndex].size) {
                  break;
                }
              } else {
                w = INVALID;
                break;
              }
              token = strtok_r(NULL, " ", &saveptr);
            }

            // If the data was valid, apply it to the buffer.
            if (w != INVALID && y == patternElements[parameterIndex].size) {
              //~ printf("Valid buffer input found.\n");
              for (i = 0; i < patternElements[parameterIndex].size; i++) {
                DBUFFER(parameterIndex)[i] = bufferData[i];
              }
            } else {
              fprintf(stderr, "Ignoring invalid buffer data for %s.\n", patternElements[parameterIndex].name);
            }
            free(bufferData);
            break;
          case ET_COUNT: case ET_INVALID:
            fprintf(stderr, "What?\n");
            break;
        }
        lookingFor = P_KEY;
        valueCount = 0;

      // Get the next character and add it to the value string.
      } else {

        // Check to make sure we have room to add a character.
        valueCount++;
        if (valueCount >= valueSize) {

          // Allocate more memory for the value.
          //~ printf("Allocating value memory. Initial size: %i ", valueSize);
          valueSize+=100;
          //~ printf("New size: %i\n", valueSize);
          value = realloc(value, valueSize * sizeof(char));
          if (!value) {
            fprintf(stderr, "Unable to allocate value memory!\n");
            break;
          }
        }

        // Add the character on to the end of the string and terminate it.
        value[valueCount - 1] = inChar;
        value[valueCount] = '\0';
      }
    }
  }

  free(ikey);
  free(value);
  fclose(fp);

  // Now, the new set may have specified an image to load.  Let's do that here.
  if (!LoadImage(set)) {
    fprintf(stderr, "Unable to load image: \"%s\"\n", DSTRING(PE_IMAGENAME));
  }
  snprintf(statusText, sizeof(statusText), "Loaded filename \"%s\" into set %i", filename, currentSet);
}


// Copy a pattern set to another.  Gotta be done element by element because
// pattern sets aren't internally contiguous.
void CopyPatternSet(int dst, int src) {
  int i;
  if (src == dst) return;

  // First create a backup of the destination.
  SavePatternSet('0' + dst, dst, YES, "copybak");

  for (i = 0; i < patternElementCount; i++) {
    switch(patternElements[i].type) {
      case ET_BOOL: SBOOL(dst, i) = SBOOL(src, i); break;
      case ET_INT: SINT(dst, i) = SINT(src, i); break;
      case ET_FLOAT: SFLOAT(dst, i) = SFLOAT(src, i); break;
      case ET_COLOR: SCOLOR(dst, i) = SCOLOR(src, i); break;
      case ET_ENUM: SENUM(dst, i) = SENUM(src, i); break;
      case ET_STRING: memcpy(SSTRING(dst, i), SSTRING(src, i), patternElements[i].size); break;
      case ET_BUFFER: memcpy(SBUFFER(dst, i), SBUFFER(src, i), patternElements[i].size); break;
      case ET_COUNT: case ET_INVALID: break;
    }
  }
  snprintf(statusText, sizeof(statusText), "Copied pattern set %i to set %i", src, dst);
}

// Figure out which pixel of the display the mouse is over.
point_t GetDisplayPixel(point_t mouse, SDL_Rect box) {
  point_t pixel;
  pixel.x = (mouse.x - box.x) / PREVIEW_PIXEL_SIZE;
  pixel.y = (mouse.y - box.y) / PREVIEW_PIXEL_SIZE;
  return(pixel);
}


// Edit a value on the display by type.
inputMode_e EditValue(int set, int commandToEdit) {
  patternElement_e element;

  element = displayCommand[commandToEdit].dataSource;
  if (element < PE_INVALID) element = GetSelectElement(set, element);
  if (element <= PE_INVALID) return IM_NORMAL;
  switch(patternElements[element].type) {
    case ET_INT:
      snprintf(statusText, sizeof(statusText), "Editing \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_INT;
    case ET_FLOAT:
      snprintf(statusText, sizeof(statusText), "Editing \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_FLOAT;
    case ET_ENUM:
      snprintf(statusText, sizeof(statusText), "Select \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_ENUM;
    case ET_COLOR:
      snprintf(statusText, sizeof(statusText), "Select color \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_COLOR;
    default:
      return IM_NORMAL;
  }
  return IM_NORMAL;
}

// Handle entry for text boxes, numbers, etc.
textEntry_e HandleTextEntry(SDL_Keycode key, SDL_Keymod mod, char * textEntry, int maxTextSize) {
  int length = strlen(textEntry);

  if (key == SDLK_UNKNOWN) return TE_NOTHING;  // Not printable.

  // Homogenize the right / left control keys.
  if (mod & KMOD_CTRL) mod |= KMOD_CTRL;
  if (mod & KMOD_ALT) mod |= KMOD_ALT;
  if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;

  if ((mod & KMOD_ALT) || (mod & KMOD_CTRL)) return TE_NOTHING;  // Not printable.
  if (key == SDLK_ESCAPE) return TE_REJECTED;  // Escape pressed, reject entry.
  if (key == SDLK_RETURN) return TE_ACCEPTED;  // Enter pressed, accept entry.

  // Backspace pressed, shorten entry.
  if (key == SDLK_BACKSPACE) {
    length--;
    if (length < 0) {
      length = 0;
    }
    textEntry[length] = '\0';
    return TE_NOTHING;
  }

  // Delete pressed, clear entry.
  if (key == SDLK_DELETE) {
    length = 0;
    textEntry[0] = '\0';
    return TE_NOTHING;
  }

  // Numeric keypad
  if (key == SDLK_KP_ENTER) return TE_ACCEPTED;  // Enter pressed, accept entry.
  switch(key) {
    case SDLK_KP_0: key = '0'; break;
    case SDLK_KP_1: key = '1'; break;
    case SDLK_KP_2: key = '2'; break;
    case SDLK_KP_3: key = '3'; break;
    case SDLK_KP_4: key = '4'; break;
    case SDLK_KP_5: key = '5'; break;
    case SDLK_KP_6: key = '6'; break;
    case SDLK_KP_7: key = '7'; break;
    case SDLK_KP_8: key = '8'; break;
    case SDLK_KP_9: key = '9'; break;
    case SDLK_KP_PLUS: key = '+'; break;
    case SDLK_KP_MINUS: key = '-'; break;
    case SDLK_KP_PERIOD: key = '.'; break;
    case SDLK_KP_MULTIPLY: key = '*'; break;
    case SDLK_KP_DIVIDE: key = '/'; break;
  }

  if ((key > '~' || key < ' ')) return TE_NOTHING;  // Not a printable character.

  // Shifted keys
  if ((mod & KMOD_SHIFT) == KMOD_SHIFT) {
    if (key <= SDLK_z && key>= SDLK_a) {
      // Capital letters
      key = key - ('a' - 'A');
    } else {
      // Symbols
      switch(key) {
        case SDLK_1: key = '!'; break;
        case SDLK_2: key = '@'; break;
        case SDLK_3: key = '#'; break;
        case SDLK_4: key = '$'; break;
        case SDLK_5: key = '%'; break;
        case SDLK_6: key = '^'; break;
        case SDLK_7: key = '&'; break;
        case SDLK_8: key = '*'; break;
        case SDLK_9: key = '('; break;
        case SDLK_0: key = ')'; break;
        case SDLK_BACKSLASH: key = '|'; break;
        case SDLK_BACKQUOTE: key = '~'; break;
        case SDLK_MINUS: key = '_'; break;
        case SDLK_EQUALS: key = '+'; break;
        case SDLK_LEFTBRACKET: key = '{'; break;
        case SDLK_RIGHTBRACKET: key = '}'; break;
        case SDLK_SEMICOLON: key = ':'; break;
        case SDLK_COMMA: key = '<'; break;
        case SDLK_PERIOD: key = '>'; break;
        case SDLK_SLASH: key = '?'; break;
        case SDLK_QUOTE: key = '"'; break;
        default: break;
      }
    }
  }

  // Sanity
  if (key < 32 || key > 126) return TE_NOTHING;

  // And we got a character, so add it.
  textEntry[length] = key;
  textEntry[length + 1] = '\0'; // Terminate it.

  // Too long?
  length++;
  if (length >= (maxTextSize - 1)) {
    textEntry[length - 1] = '\0';  // Reterminate.
  }

  return TE_NOTHING;
}


bool_t SetValueInt(int set, patternElement_e element, int value) {
  // Validate the input value
  if ((value >= patternElements[element].min.i) &&
      (value <= patternElements[element].max.i)) {
    SINT(set, element) = value;
    return YES;
  }
  return NO;
}

void SetValueFloat(int set, patternElement_e element, float value) {
  // Validate the input value
  if (value < patternElements[element].min.f) {
    SFLOAT(set, element) = patternElements[element].min.f;
    snprintf(statusText, sizeof(statusText), "Input value %f too low.  Using min instead.", value);
  } else if (value > patternElements[element].max.f) {
    SFLOAT(set, element) = patternElements[element].max.f;
    snprintf(statusText, sizeof(statusText), "Input value %f too high.  Using max instead.", value);
  } else {
    SFLOAT(set, element) = value;
    snprintf(statusText, sizeof(statusText), "Value set to %f.", value);
  }
}


int OverCommand(point_t mouse, int *lastHover) {
  int i;
  SDL_Rect box, boxOld, drawBox;
  displayText_t commandTexture;

  boxOld = GetBoxofCommand(*lastHover);

  for (i = 0; i < displayCommandCount; i++) {
    // box is the rectangle encompassing the command text.  We could
    // precompute these if timing were important.
    box = GetBoxofCommand(i);

    // Is it in the rectangle of command i?
    if (IsInsideBox(mouse, box)) {

      // Is it hovering over a command is wasn't hovering over before?
      if ((!IsSameBox(box, boxOld)) || (*lastHover == INVALID)) {

        // Yeah, so draw the new highlight.
        drawBox = box;
        //~ drawBox.w = ColToPixel(displayCommand[i].col + 1) - drawBox.x;
        DrawSBox(drawBox, DISPLAY_COLOR_TEXTSBG_HL);
        CreateTextureCommand(&commandTexture, i, DISPLAY_COLOR_TEXTS_HL, DISPLAY_COLOR_TEXTSBG_HL);
        DrawDisplayTexture(commandTexture);
        SDL_DestroyTexture(commandTexture.texture);

        // And if it came off a different command, remove that highlight.
        if (*lastHover != INVALID) {
          drawBox = boxOld;
          //~ drawBox.w = ColToPixel(displayCommand[*lastHover].col + 1) - drawBox.x;
          DrawSBox(drawBox, DISPLAY_COLOR_TEXTSBG);
          CreateTextureCommand(&commandTexture, *lastHover, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
          DrawDisplayTexture(commandTexture);
          SDL_DestroyTexture(commandTexture.texture);
        }

        // New lastHover.
        *lastHover = i;
      }  // End new hover if

      // This was our intersection.  Break the loop.  Can't be hovering over
      // multiple commands.
      return i;
    }  // End intersected if
  } // End for for hover check.

  // Not over a new command? May have to clear the old highlight anyway.
  if (*lastHover != INVALID) {
    //~ boxOld.w = ColToPixel(displayCommand[*lastHover].col + 1) - boxOld.x;
    DrawSBox(boxOld, DISPLAY_COLOR_TEXTSBG);
    CreateTextureCommand(&commandTexture, *lastHover, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
    DrawDisplayTexture(commandTexture);
    SDL_DestroyTexture(commandTexture.texture);
    *lastHover = INVALID;
  }

  return INVALID;
}

int OverBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover) {
  int i;
  SDL_Rect box, boxOld;
  const SDL_Rect invalidBox = {0, 0, 0, 0};
  int boxCount;
  patternElement_e element;
  element = displayCommand[item].dataSource;
  if (element < PE_INVALID) element = GetSelectElement(set, element);
  boxCount = enumerations[patternElements[element].etype].size;
  const color_t fg = DISPLAY_COLOR_PARMS;
  const color_t bg = DISPLAY_COLOR_PARMSBG;
  const color_t fgh = DISPLAY_COLOR_TEXTS_HL;
  const color_t bgh = DISPLAY_COLOR_TEXTSBG_HL;
  char text[PARAMETER_WIDTH];

  if (*lastHover != INVALID) boxOld = (*targets)[*lastHover];
  else boxOld = invalidBox;

  for (i = 0; i < boxCount; i++) {
    // box is the rectangle encompassing the command text.  We could
    // precompute these if timing were important.
    box = (*targets)[i];

    // Is it in the rectangle of command i?
    if (IsInsideBox(mouse, box)) {

      // Is it hovering over a command is wasn't hovering over before?
      if ((!IsSameBox(box, boxOld)) || (*lastHover == INVALID)) {

        // Yeah, so draw the new highlight.
        DrawOutlineBox(box, fgh, bgh);
        snprintf(text, sizeof(text), "%s", enumerations[patternElements[element].etype].texts[i]);
        DrawCenteredText(box, text, fgh, bgh);

        // And if it came off a different command, remove that highlight.
        if (*lastHover != INVALID) {
          DrawOutlineBox(boxOld, fg, bg);
          snprintf(text, sizeof(text), "%s", enumerations[patternElements[element].etype].texts[*lastHover]);
          DrawCenteredText(boxOld, text, fg, bg);
        }

        // New lastHover.
        *lastHover = i;
      }  // End new hover if

      // This was our intersection.  Break the loop.  Can't be hovering over
      // multiple commands.
      return i;
    }  // End intersected if
  } // End for for hover check.

  // Not over a new command? May have to clear the old highlight anyway.
  if (*lastHover != INVALID) {
    DrawOutlineBox(boxOld, fg, bg);
    snprintf(text, sizeof(text), "%s", enumerations[patternElements[element].etype].texts[*lastHover]);
    DrawCenteredText(boxOld, text, fg, bg);
    *lastHover = INVALID;
  }

  return INVALID;
}

int OverColorBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover) {
  int i;
  SDL_Rect box, boxOld;
  const SDL_Rect invalidBox = {0, 0, 0, 0};
  int boxCount;
  boxCount = CE_COUNT;

  if (*lastHover != INVALID) boxOld = (*targets)[*lastHover];
  else boxOld = invalidBox;

  for (i = 0; i < boxCount; i++) {
    // box is the rectangle encompassing the command text.  We could
    // precompute these if timing were important.
    box = (*targets)[i];

    // Is it in the rectangle of command i?
    if (IsInsideBox(mouse, box)) {

      // Is it hovering over a command is wasn't hovering over before?
      if ((!IsSameBox(box, boxOld)) || (*lastHover == INVALID)) {

        // New lastHover.
        *lastHover = i;
      }  // End new hover if

      // This was our intersection.  Break the loop.  Can't be hovering over
      // multiple commands.
      return i;
    }  // End intersected if
  } // End for for hover check.

  // Not over a new command? May have to clear the old highlight anyway.
  if (*lastHover != INVALID) {
    *lastHover = INVALID;
  }

  return INVALID;
}

// Update the values of the text and color swatches on the display window.
// TODO: Shorten this function.  Lots of repetition.
#define STATUS_BAR_LENGTH 75
#define BUFFER_BAR_LENGTH 94
void UpdateInfoDisplay(int set) {
  int cachePosition = 0;
  int sCacheP = 0;
  int length, i;
  char statusTemp[STATUS_BAR_LENGTH + 10];
  char bufferTemp1[BUFFER_BAR_LENGTH + 10];
  char bufferTemp2[BUFFER_BAR_LENGTH + 10];
  patternElement_e element, element_o;
  SDL_Rect tempBox;
  color_t color;
  char enumString[100];

  // Automatic information based on displayCommand array.
  for (i = 0; i < displayCommandCount; i++) {
    element_o = element = displayCommand[i].dataSource;
    if (element < PE_INVALID) element = GetSelectElement(set, element);
    if (element > PE_INVALID) {
      switch(patternElements[element].type) {
        case ET_INT:
          DrawAndCache(&cachePosition, patternElements[element].type,
            &SINT(set, element), displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          break;
        case ET_BOOL:
          DrawAndCache(&cachePosition, patternElements[element].type,
            &SBOOL(set, element), displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          break;
        case ET_FLOAT:
          DrawAndCache(&cachePosition, patternElements[element].type,
            &SFLOAT(set, element), displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          break;
        case ET_ENUM:
          strncpy(enumString, enumerations[patternElements[element].etype].texts[SENUM(set, element)], sizeof(enumString));
          DrawAndCache(&cachePosition, patternElements[element].type, enumString,
            displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          break;
        case ET_COLOR:
          tempBox = GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 1, 0);
          tempBox.w = PARAMETER_WIDTH * CHAR_W;
          DrawOutlineBox(tempBox, DISPLAY_COLOR_PARMS, SCOLOR(set, element));
          DrawAndCache(&cachePosition, PE_INVALID, NULL, 0, 0, 0);
          break;
        default:
          break;
      }
    } else if (element_o < PE_INVALID) {
      // Make sure there is cache reserved even though we aren't really
      // using this slot right now.
      DrawAndCache(&cachePosition, PE_INVALID, NULL, 0, 0, 0);

      // Draw the disabled element boxes
      tempBox = GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 1, 0);
      tempBox.w = PARAMETER_WIDTH * CHAR_W;
      DrawXBox(tempBox, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
    }
  }

  // The ones that aren't automatic:
  DrawAndCache(&cachePosition, ET_INT, &previewFPSA, FPS_ROW, 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_INT, &previewFPSB, FPS_ROW, 5, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_INT, &infoFPS, ROW_COUNT - 1, 5, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, cyclePatternSets ? ET_INT : ET_BOOL,
    cyclePatternSets ? (void *) &cycleFrameCount : (void *) &cyclePatternSets,
    ROW_PA + 1, COL_PA + 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_INT, &alternateSet, ROW_PA + 9, COL_PA + 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_INT, &currentSet, ROW_PA + 8, COL_PA + 1, PARAMETER_WIDTH);

  // Control set operates on (an enum type)
  strncpy(enumString, enumerations[E_OPERATE].texts[displaySet], sizeof(enumString));
  DrawAndCache(&cachePosition, ET_ENUM, enumString, ROW_PA + 10, COL_PA + 1, PARAMETER_WIDTH);

  float alternateBlend = GetAltBlend();
  bool_t autoBlend = GetAutoBlend();
  float alternateBlendRate = GetAltBlendRate();
  bool_t enableTensor = GetTensorEnable();
  DrawAndCache(&cachePosition, ET_FLOAT, &alternateBlend, ROW_PA + 12, COL_PA + 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_BOOL, &autoBlend, ROW_PA + 13, COL_PA + 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_BOOL, &enableTensor, ROW_A + 1, COL_A + 1, PARAMETER_WIDTH);
  DrawAndCache(&cachePosition, ET_FLOAT, &alternateBlendRate, ROW_PA + 14, COL_PA + 1, PARAMETER_WIDTH);

  // Show the status bar
  snprintf(statusTemp, sizeof(statusTemp) - 1, "%-*.*s", STATUS_BAR_LENGTH, STATUS_BAR_LENGTH, statusText);
  DrawAndCacheString(&sCacheP, statusTemp, ROW_COUNT - 1, 1, cBlack, cRed);

  // Show the last 100 bytes of the text buffer.
  length = strlen(SSTRING(set, PE_TEXTBUFFER));
  DrawAndCache(&cachePosition, ET_INT, &length, ROW_COUNT - 3, 1, PARAMETER_WIDTH);
  strncpy(bufferTemp1, SSTRING(set, PE_TEXTBUFFER) + (length > BUFFER_BAR_LENGTH ? length - BUFFER_BAR_LENGTH : 0), BUFFER_BAR_LENGTH + 1);
  snprintf(bufferTemp2, sizeof(bufferTemp2), " %-*s", BUFFER_BAR_LENGTH + 3, bufferTemp1);
  DrawAndCacheString(&sCacheP, bufferTemp2, ROW_COUNT - 2, 0, DISPLAY_COLOR_TBUF, DISPLAY_COLOR_TBUFBG);

  // Show the color selectors swatches.
  for (i = 0; i < displayCommandCount; i++) {
    if (displayCommand[i].colorSource != A_INVALID) {
      //~ color = fgOutput[displayCommand[i].colorSource][set];
      color = GetFGColor(displayCommand[i].colorSource, set);
      color.a = 255;
      DrawOutlineBox(GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 7, 0),
        DISPLAY_COLOR_PARMS, color);
      if (seedPalette[displayCommand[i].colorSource].bgColorA != PE_INVALID) {
        // A background if there is one.
        color = GetBGColor(displayCommand[i].colorSource, set);
        color.a = 255;
        tempBox = GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 7, 0);
        tempBox.w = tempBox.w / 2;
        tempBox.x = tempBox.x + tempBox.w;
        DrawOutlineBox(tempBox, DISPLAY_COLOR_PARMS, color);
      }
    }
  }
}

void CopyBuffer(int dst, int src) {
  if (src == dst) return;
  memcpy(SBUFFER(dst, PE_FRAMEBUFFER), SBUFFER(src, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  snprintf(statusText, sizeof(statusText), "Loaded buffer from set %i into set %i", src, dst);
}

void SwitchToSet(int set) {
  if (displaySet == OO_ALTERNATE) {
    alternateSet = set;
  } else {
    currentSet = set;
  }
  snprintf(statusText, sizeof(statusText), "Switched %s set to #%i",
    displaySet == OO_ALTERNATE ? "alternate" : "live", set);
}

// This seems excessive.
int HandleColorSelection(point_t mouse, int thisHover, bool_t resetEvent, int set) {
  int currentSet = set; // Override
  patternElement_e cSource;

  if (displayCommand[thisHover].colorSource != A_INVALID) {
    if (IsInsideBox(mouse, GetBoxofLine(displayCommand[thisHover].line, displayCommand[thisHover].col + 7, 0))) {
      if (resetEvent) {
        cSource = displayCommand[thisHover].colorSource;
        if (seedPalette[cSource].fgColorA != PE_INVALID)
          DCOLOR(seedPalette[cSource].fgColorA) = patternElements[seedPalette[cSource].fgColorA].initial.c;
        if (seedPalette[cSource].fgColorB != PE_INVALID)
          DCOLOR(seedPalette[cSource].fgColorB) = patternElements[seedPalette[cSource].fgColorB].initial.c;
        if (seedPalette[cSource].fgCycleMode != PE_INVALID)
          DENUM(seedPalette[cSource].fgCycleMode) = patternElements[seedPalette[cSource].fgCycleMode].initial.e;
        if (seedPalette[cSource].fgCycleRate != PE_INVALID)
          DINT(seedPalette[cSource].fgCycleRate) = patternElements[seedPalette[cSource].fgCycleRate].initial.i;
        if (seedPalette[cSource].fgCyclePos != PE_INVALID)
          DINT(seedPalette[cSource].fgCyclePos) = patternElements[seedPalette[cSource].fgCyclePos].initial.i;
        if (seedPalette[cSource].fgAlpha != PE_INVALID)
          DFLOAT(seedPalette[cSource].fgAlpha) = patternElements[seedPalette[cSource].fgAlpha].initial.f;
        if (seedPalette[cSource].bgColorA != PE_INVALID)
          DCOLOR(seedPalette[cSource].bgColorA) = patternElements[seedPalette[cSource].bgColorA].initial.c;
        if (seedPalette[cSource].bgColorB != PE_INVALID)
          DCOLOR(seedPalette[cSource].bgColorB) = patternElements[seedPalette[cSource].bgColorB].initial.c;
        if (seedPalette[cSource].bgCycleMode != PE_INVALID)
          DENUM(seedPalette[cSource].bgCycleMode) = patternElements[seedPalette[cSource].bgCycleMode].initial.e;
        if (seedPalette[cSource].bgCycleRate != PE_INVALID)
          DINT(seedPalette[cSource].bgCycleRate) = patternElements[seedPalette[cSource].bgCycleRate].initial.i;
        if (seedPalette[cSource].bgCyclePos != PE_INVALID)
          DINT(seedPalette[cSource].bgCyclePos) = patternElements[seedPalette[cSource].bgCyclePos].initial.i;
        if (seedPalette[cSource].bgAlpha != PE_INVALID)
          DFLOAT(seedPalette[cSource].bgAlpha) = patternElements[seedPalette[cSource].bgAlpha].initial.f;
      }
      return YES;
    }
  }
  return NO;
}

// Key press processing for enumeration selection boxes.
bool_t HandleColorSelect(SDL_Keycode key, int set, int item, int *selected) {
  bool_t selectionDismissed = YES;
  int boxCount = CE_COUNT;
  patternElement_e element;

  switch (key) {
    case SDLK_ESCAPE:
      snprintf(statusText, sizeof(statusText), "Color selection cancelled.");
      break;

    case SDLK_RETURN:
    case SDLK_SPACE:
      element = GetSelectElement(set, displayCommand[item].dataSource);
      SCOLOR(set, element) = namedColors[*selected].color;
      snprintf(statusText, sizeof(statusText), "Item %s set.", patternElements[element].name);
      break;

    // They're laid out in a grid.
    case SDLK_LEFT:
      (*selected)--;
      if (*selected >= boxCount) *selected = 0;
      if (*selected < 0) {
        (*selected)++;
        if (*selected < 0) (*selected) = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_UP:
      *selected -= colorTitlesCount;
      if (*selected >= boxCount) *selected = 0;
      if (*selected < 0) {
        *selected += colorTitlesCount;
        if (*selected < 0) *selected = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_DOWN:
      *selected += colorTitlesCount;
      if (*selected >= boxCount) {
        *selected -= colorTitlesCount;
        if (*selected >= boxCount) *selected = 0;
      }
      selectionDismissed = NO;
      break;
    case SDLK_RIGHT:
      *selected += 1;
      if (*selected >= boxCount) {
        *selected -= 1;
        if (*selected >= boxCount) *selected = 0;
      }
      selectionDismissed = NO;
      break;

    default:
      selectionDismissed = NO;
      break;
  }
  return selectionDismissed;
}

// Initialize the texture cache.  All this crap actually lowers my CPU usage
// from 25% to 12%.  Half of CPU time was dedicated to regenerating the text
// textures 40x per second.
void DrawAndCache(int *cachePosition, elementType_e type, void *value, int row, int col, int width) {
  static int cacheCount = 0;
  static infoCache_t *cache = NULL;

  // Increase the cache position
  (*cachePosition)++;

  // Make a new slot if there isn't one already.
  if (*cachePosition > cacheCount) {
    cacheCount++;
    cache = realloc(cache, cacheCount * sizeof(infoCache_t));
    cache[*cachePosition - 1].infoText.texture = NULL;
  }

  // Compare new value with cached value.
  switch(type) {
    case ET_BOOL:
      if ((cache[*cachePosition - 1].cacheValue.b != *((bool_t *)value)) ||
          (!cache[*cachePosition - 1].infoText.texture)) {
        cache[*cachePosition - 1].cacheValue.b = *((bool_t *)value);
        SDL_DestroyTexture(cache[*cachePosition - 1].infoText.texture);
        CreateTextureBoolean(&(cache[*cachePosition - 1].infoText), *((bool_t *)value), row, col, width);
      }
      break;
    case ET_INT:
      if ((cache[*cachePosition - 1].cacheValue.i != *((int *)value)) ||
          (!cache[*cachePosition - 1].infoText.texture)) {
        cache[*cachePosition - 1].cacheValue.i = *((int *)value);
        SDL_DestroyTexture(cache[*cachePosition - 1].infoText.texture);
        CreateTextureInt(&(cache[*cachePosition - 1].infoText), *((int *)value), row, col, width);
      }
      break;
    case ET_FLOAT:
      if ((cache[*cachePosition - 1].cacheValue.f != *((float *)value)) ||
          (!cache[*cachePosition - 1].infoText.texture)) {
        cache[*cachePosition - 1].cacheValue.f = *((float *)value);
        SDL_DestroyTexture(cache[*cachePosition - 1].infoText.texture);
        CreateTextureFloat(&(cache[*cachePosition - 1].infoText), *((float *)value), row, col, width, width / 2);
      }
      break;
    case ET_ENUM:
      if ((cache[*cachePosition - 1].cacheValue.e != *((int *)value)) ||
          (!cache[*cachePosition - 1].infoText.texture)) {
        cache[*cachePosition - 1].cacheValue.e = *((int *)value);
        SDL_DestroyTexture(cache[*cachePosition - 1].infoText.texture);
        CreateTextureString(&(cache[*cachePosition - 1].infoText),
          (char *)value, row, col, width);
      }
      break;
    default:
      return;
  }

  DrawDisplayTexture(cache[*cachePosition - 1].infoText);
}

void DrawAndCacheString(int *cachePosition, const char *value, int row, int col, color_t fg, color_t bg) {
  static int cacheCount = 0;
  static infoCache_t *cache = NULL;
  static char **oldValue = NULL;
  int length = strlen(value);

  // Increase the cache position
  (*cachePosition)++;

  // Make a new slot if there isn't one already.
  if (*cachePosition > cacheCount) {
    cacheCount++;
    cache = realloc(cache, cacheCount * sizeof(infoCache_t));
    cache[*cachePosition - 1].infoText.texture = NULL;
    oldValue = realloc(oldValue, cacheCount * sizeof(char *));
    oldValue[*cachePosition - 1] = NULL;
  }

  // Regenerate the texture if the string changed.
  if ((!cache[*cachePosition - 1].infoText.texture) ||
      (strcmp(oldValue[*cachePosition - 1], value) != 0)) {
    oldValue[*cachePosition - 1] = realloc(oldValue[*cachePosition - 1], (length + 1) * sizeof(char));
    strncpy(oldValue[*cachePosition - 1], value, length + 1);
    SDL_DestroyTexture(cache[*cachePosition - 1].infoText.texture);
    CreateTextureLine(&(cache[*cachePosition - 1].infoText), value,
      row, col, fg, bg);
  }

  DrawDisplayTexture(cache[*cachePosition - 1].infoText);
}
