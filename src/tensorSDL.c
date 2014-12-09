// Tensor Pattern
// For Kevin (FB) McCormick (w/o you, there may be nothing) <3
// Blau
// tensor@core9.org

// Includes
#include <stdio.h>   // File io, setvbuf
#include <stdlib.h>  // exit conditions, string conversions, malloc
#include <unistd.h>  // Don't know what I used from here?  Maybe stderr?
#include <string.h>  // memcpy, strtok
#include <time.h>    // nanosleep
#include <errno.h>   // errno for string conversions.
#include <SDL.h>
#include <SDL2_rotozoom.h>
#include <SDL_image.h>

#include "useful.h"
#include "drv-tensor.h"
#include "my_font.h"
#include "version.h"
#include "draw.h"
#include "elements.h"

// Definitions
#define PATTERN_SET_COUNT 10
#define GUIFRAMEDELAY 25
#define IP_STRING_SIZE 16

// Color plane flags - I'm unsatified with the way the cym color planes worked
// out.  I should be using a different color space or something.
#define PLANE_NONE    0x00
#define PLANE_RED     0x01
#define PLANE_GREEN   0x02
#define PLANE_BLUE    0x04
#define PLANE_CYAN    (PLANE_BLUE | PLANE_GREEN)
#define PLANE_YELLOW  (PLANE_RED | PLANE_GREEN)
#define PLANE_MAGENTA (PLANE_RED | PLANE_BLUE)
#define PLANE_ALL     (PLANE_RED | PLANE_BLUE | PLANE_GREEN)

// Gui input modes
typedef enum inputMode_e {
  IM_INVALID = -1,
  IM_NORMAL = 0, IM_CONFIRM, IM_INT, IM_FLOAT, IM_ENUM,
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
SDL_Surface *imageSeed[PATTERN_SET_COUNT];
int tensorWidth, tensorHeight;
bool_t cyclePatternSets = NO;  // cyclePatternSets mode is a global (for now).
int cycleFrameCount = INITIAL_FRAMECYCLECOUNT;    // Frame Count for cycling.
bool_t enableTensor = YES;
int currentSet = 0;
int alternateSet = 1;
operateOn_e displaySet = OO_CURRENT;
float global_intensity_limit = 1.0;
int previewFrameCountA = 0, previewFPSA = 0;
int previewFrameCountB = 0, previewFPSB = 0;
int infoFrameCount = 0, infoFPS = 0;
float alternateBlend = 0, alternateBlendRate = 0.01;
bool_t autoBlend = NO;
Uint32 FPSEventType, DRAWEventTypeA, DRAWEventTypeB, GUIEventType, CONFIRMEventType;
char statusText[100] = "";  // For confirming actions.
char keySave;
int setSave;
box_t liveBox, altBox;
point_t tensorPixelMap [TENSOR_WIDTH * TENSOR_HEIGHT];
bool_t useDefaultPixelMap = YES;
const char **ipmap1;
const char **ipmap2;
const char **ipmap3;

// Prototypes
void DrawNewFrame(int set, unsigned char primary);
void ProcessModes(int set);
Uint32 TriggerFrameDrawA(Uint32 interval, void *param);
Uint32 TriggerFrameDrawB(Uint32 interval, void *param);
Uint32 TriggerFrameCount(Uint32 interval, void *param);
Uint32 TriggerGUIUpdate(Uint32 interval, void *param);
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer);
void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane);
void WriteSlice(int set);
void CellFun(int set);
void DrawSideBar(int set);
void Diffuse(float diffusionCoeff, bool_t isToroid, unsigned char *buffer);
void HorizontalBars(color_t color, unsigned char *buffer);
void VerticalBars(color_t color, unsigned char *buffer);
void SavePatternSet(char key, int set, bool_t overWrite, bool_t backup);
void LoadPatternSet(char key, int set);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
color_t ColorCycle(int set, colorCycleModes_e cycleMode, int *cycleSaver, int cycleInc);
void ColorAll(color_t color, unsigned char *fb);
void SetDims(void);
void UpdateDisplays(int set, bool_t isPrimary, bool_t sendToTensor, float intensity_limit);
void UpdateTensor(unsigned char *buffer);
void UpdatePreview(int xOffset, int yOffset, unsigned char *buffer);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src, unsigned char exclude);
void Multiply(float multiplier, unsigned char *buffer);
void ClearRed(int set);
void ClearGreen(int set);
void ClearBlue(int set);
void DrawImage(SDL_Surface *image, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst);
bool_t HandleCommand(int set, command_e command);
bool_t HandleKey(int set, SDL_Keycode key, SDL_Keymod mod);
bool_t HandleConfirmation(SDL_Keycode key, bool_t *selected);
inputMode_e EditValue(int set, int commandToEdit);
textEntry_e HandleTextEntry(SDL_Keycode key, SDL_Keymod mod, char * textEntry, int maxTextSize);
bool_t HandleEnumSelect(SDL_Keycode key, int set, int item, int *selected);
void CopyPatternSet(int dst, int src);
void BlendAlternate(unsigned char *fba, unsigned char *fbb);
void CenterText(box_t box, char * text, color_t fg, color_t bg);
void SnailSeed(int set, int position);
void FastSnailSeed(int set, int position);
void CrossBars(int set);
point_t GetDisplayPixel(point_t mouse, box_t box);
void LoadTensorMaps(void);
void LoadPixelMap(void);
void LoadIPMap(void);
void GenerateDefaultPixelMap(void);
void VerticalMirror(unsigned char * buffer);
void HorizontalMirror(unsigned char * buffer);
void ScrollCycle(int set);
bool_t SetValueInt(int set, patternElement_e element, int value);
void SetValueFloat(int set, patternElement_e element, float value);
int OverCommand(point_t mouse, int *lastHover);
int OverBox(point_t mouse, int item, box_t ** targets, int *lastHover);
box_t GetCommandBox(int command);
void DrawSidePulse(int set);

void UpdateInfoDisplay(int set);

// Main
int main(int argc, char *argv[]) {

  // Variable declarations
  int i;
  char textEntry[100];
  SDL_Event event;
  int thisHover = INVALID;
  int lastHover = INVALID;
 box_t box2 = {0, 0, 0, 0}, boxOld2 = {0, 0, 0, 0};
  box_t boxYes, boxNo;
  point_t mouse, tpixel;
  int leftMouseDownOn = INVALID;
  bool_t leftMouseDown = NO;
  int rightMouseDownOn = INVALID;
  bool_t rightMouseDown = NO;
  bool_t refreshAll = NO;
  bool_t refreshGui = NO;
  bool_t confirmed = NO;
  bool_t drawNewFrameA = NO;
  bool_t drawNewFrameB = NO;
  bool_t exitProgram = NO;
  inputMode_e inputMode = IM_NORMAL;
  inputMode_e oldInputMode = IM_INVALID;
  int value;
  float valuef;
  char *testptr;
  int enumHover = INVALID;
  box_t *targets = NULL;
  operateOn_e displaySetOld = OO_INVALID;

  // Unbuffer the console...
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  setvbuf(stderr, (char *)NULL, _IONBF, 0);

  // Print the version #.
  fprintf(stdout, "Tensor pattern generator v%s.%s.%s%s%s\n",
    MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION,
    strlen(PRERELEASE_VERSION) != 0 ? "." : "", PRERELEASE_VERSION);

  // Commandline parameter for limiting the intensity.
  if (argc == 2) {
    global_intensity_limit = atof(argv[1]);
    if (global_intensity_limit < 0.0 || global_intensity_limit > 1.0) {
      global_intensity_limit = 1.0;
    }
  }

  // Verify the integrity of some of the data structures. Enforces consistency
  // for enumerated array access.  Errors here are programmer failures.
  if (!VerifyStructuralIntegrity()) exit(EXIT_FAILURE);

  // Allocate the pattern set memory
  if (!AllocatePatternData(PATTERN_SET_COUNT)) exit(EXIT_FAILURE);

  // Initialize the gui elements
  if (!InitGui()) {
    fprintf(stderr, "Unable to initialize the gui elements!\n");
    exit(EXIT_FAILURE);
  }

  // Set the window title
  snprintf(textEntry, sizeof(textEntry), "Tensor Control - Output: %i%%", (int) (global_intensity_limit * 100));
  WindowTitle(textEntry);

  // Initialize the image array.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    imageSeed[i] = NULL;
  }

  // Initialize Tensor.
  //tensor_landscape_p = 1;  // Landscape mode (good for single panel).
  tensor_landscape_p = 0;  // Portrait mode (normal).

  // Set the widths / heights
  SetDims();  // After set tensor_landscape_p.

  // Load the ip and pixels maps.
  LoadTensorMaps();

  // Initialize tensor communications.
  tensor_init(ipmap1, ipmap2, ipmap3);

  // Draw a border around the previews
  DrawPreviewBorder(PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y, tensorWidth, tensorHeight, YES);
  DrawPreviewBorder(PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y, tensorWidth, tensorHeight, NO);

  // Further patterData initializations
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    SINT(i, PE_TEXTOFFSET) = tensorHeight / 3 - 1;  // After SetDims()
  }

  // Attempt to load startup pattern sets from disk.  These are the 0-9.now
  // files.  Its okay if they don't exist.  Otherwise, default will be loaded
  // into those pattern sets.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    LoadPatternSet(i + '0', i);
    // Load the remaining image sets (in cases of pattern set load failure).
    if (!imageSeed[i]) {
      imageSeed[i] = IMG_Load(SSTRING(i, PE_IMAGENAME));
    }
  }

  cycleFrameCount = DINT(PE_FRAMECOUNT);

  // Bam - Show the (blank) preview.
  UpdateDisplays(currentSet, YES, YES, global_intensity_limit);

  // Add the text to the window
  DrawDisplayTexts(INVALID);

  // Initialize the user events
  FPSEventType = SDL_RegisterEvents(1);
  DRAWEventTypeA = SDL_RegisterEvents(1);
  DRAWEventTypeB = SDL_RegisterEvents(1);
  GUIEventType = SDL_RegisterEvents(1);
  CONFIRMEventType = SDL_RegisterEvents(1);

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

    // Act on queued events.
    while (SDL_PollEvent(&event)) {

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

          } else if (event.type == FPSEventType) {
            // The fps timer expired.  Set fps and reset the frame count.
            previewFPSA = previewFrameCountA;
            previewFPSB = previewFrameCountB;
            infoFPS = infoFrameCount;
            infoFrameCount = 0;
            previewFrameCountA = 0;
            previewFrameCountB = 0;

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
                    SavePatternSet(keySave, setSave, YES, NO);
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
              break;  // End case SDL_MOUSEMOTION.

            case SDL_MOUSEWHEEL:
              // The mouse wheel moved.  See if we should act on that.  We don't
              // check HandleCommand return codes because there are no mouse wheel
              // commands that exit the program.
              if (thisHover != INVALID) {

                // Wheel down.
                if (event.wheel.y < 0) {
                  // If there are no mouse wheel commands for this item, consider it a click.
                  if (displayCommand[thisHover].commands[MOUSE_WHEEL_DOWN].command == COM_NONE)
                    HandleCommand(displaySet ? alternateSet : currentSet, displayCommand[thisHover].commands[MOUSE_CLICK].command);
                  else HandleCommand(displaySet ? alternateSet : currentSet, displayCommand[thisHover].commands[MOUSE_WHEEL_DOWN].command);

                // Wheel up.
                } else {
                  // If there are no mouse wheel commands for this item, consider it a click.
                  if (displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command == COM_NONE)
                    HandleCommand(displaySet ? alternateSet : currentSet, displayCommand[thisHover].commands[MOUSE_CLICK].command);
                  else HandleCommand(displaySet ? alternateSet : currentSet, displayCommand[thisHover].commands[MOUSE_WHEEL_UP].command);
                }

              // Mouse wheel action over the live preview lets us change colors
              // without having to go to the command.
              } else if (IsInsideBox(mouse, liveBox)) {
                if (!rightMouseDown) {
                  if (event.wheel.y < 0) { // Wheel down
                    HandleCommand(currentSet, COM_FG_WHEEL_DOWN);
                  } else { // Wheel up
                    HandleCommand(currentSet, COM_FG_WHEEL_UP);
                  }
                } else {
                  // Holding down the right mouse button while wheeling changes the
                  // background color instead.
                  if (event.wheel.y < 0) { // Wheel down
                    HandleCommand(currentSet, COM_BG_WHEEL_DOWN);
                  } else { // Wheel up
                    HandleCommand(currentSet, COM_BG_WHEEL_UP);
                  }
                }
              } else if (IsInsideBox(mouse, altBox)) {
                if (!rightMouseDown) {
                  if (event.wheel.y < 0) { // Wheel down
                    HandleCommand(alternateSet, COM_FG_WHEEL_DOWN);
                  } else { // Wheel up
                    HandleCommand(alternateSet, COM_FG_WHEEL_UP);
                  }
                } else {
                  if (event.wheel.y < 0) { // Wheel down
                    HandleCommand(alternateSet, COM_BG_WHEEL_DOWN);
                  } else { // Wheel up
                    HandleCommand(alternateSet, COM_BG_WHEEL_UP);
                  }
                }
              }
              break; // End SDL_MOUSEWHEEL case

            case SDL_MOUSEBUTTONUP:
              // Mouse button unpushed.  Consider this a click.  If we're over
              // the same item we down clicked on, execute the command.
              if (event.button.button == SDL_BUTTON_LEFT) {
                if ((thisHover != INVALID) && (thisHover == leftMouseDownOn)) {
                  exitProgram = HandleCommand(displaySet ? alternateSet : currentSet, displayCommand[thisHover].commands[MOUSE_CLICK].command);
                }
              } else if (event.button.button == SDL_BUTTON_RIGHT) {
                if ((thisHover != INVALID) && (thisHover == rightMouseDownOn)) {
                  inputMode = EditValue(currentSet, thisHover);
                  if ((inputMode == IM_INT) || (inputMode == IM_FLOAT)) {
                    // Initial value to edit.
                    //~ snprintf(textEntry, sizeof(textEntry), "%i", DINT(displayCommand[thisHover].dataSource));
                    textEntry[0] = '\0';
                  } else if (inputMode == IM_ENUM) {
                    if (targets) {
                      free(targets);
                      targets = NULL;
                    }
                  }
                }
              }
              break;  // End SDL_MOUSEBUTTONUP case.

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
                      if (SetValueInt(displaySet ? alternateSet : currentSet, displayCommand[thisHover].dataSource, value)) {
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
                        SetValueFloat(displaySet ? alternateSet : currentSet, displayCommand[thisHover].dataSource, valuef);
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
              enumHover = OverBox(mouse, thisHover, &targets, &lastHover);
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
                  SENUM(displaySet ? alternateSet : currentSet, displayCommand[thisHover].dataSource) = enumHover;
                  snprintf(statusText, sizeof(statusText), "Item %s set to %s",
                    displayCommand[thisHover].text,
                    enumerations[patternElements[displayCommand[thisHover].dataSource].etype].texts[enumHover]);
                  // Special handling for the foreground and background
                  // color selectors:
                  if (displayCommand[thisHover].dataSource == PE_FGE)
                    SCOLOR(displaySet ? alternateSet : currentSet, PE_FGC) =
                      namedColors[SENUM(displaySet ? alternateSet : currentSet, PE_FGE)].color;
                  else if (displayCommand[thisHover].dataSource == PE_BGE)
                    SCOLOR(displaySet ? alternateSet : currentSet, PE_BGC) =
                      namedColors[SENUM(displaySet ? alternateSet : currentSet, PE_BGE)].color;
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
                DrawEnumSelectBox(thisHover, enumHover, &targets);
              }
              break;
          } // End event processing switch for ENUM selection
          break; // End case IM_ENUM

        // Unhandled input modes
        default:
          inputMode = IM_NORMAL;
          break;
      } // End input mode specific switch
    } // End event polling while loop.

    // Draw a new frame if the timer expired for the live set.
    if (drawNewFrameA) {
      drawNewFrameA = NO;
      previewFrameCountA++;
      DrawNewFrame(currentSet, YES);

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

    // Draw a new frame if the timer expired for the preview set.
    } else if (drawNewFrameB) {
      drawNewFrameB = NO;
      previewFrameCountB++;
      DrawNewFrame(alternateSet, NO);

    } else {
      // Large frame delays make the CPU spin because we get stuck in a tight
      // event polling loop looking for events that never occur.  Best to get
      // out of the way... Idle the processor for 1 ms.
      nanosleep((struct timespec[]) {{0,1000000}}, NULL);
    }

    // Are we trying to draw on the previews with the mouse?  We do this here
    // because we want to continue to draw whether or not there are mouse events.
    if (IsInsideBox(mouse, liveBox)) {
      // Mouse hovering over the live preview.  Which display pixel?
      tpixel = GetDisplayPixel(mouse, liveBox);
      // Mirror corrections
      if (SBOOL(currentSet, PE_MIRROR_V)) {
        if (tpixel.x > tensorWidth / 2) {
          tpixel.x = (tensorWidth - 1) - tpixel.x;
        }
      }
      if (SBOOL(currentSet, PE_MIRROR_H)) {
        if (tpixel.y > tensorHeight / 2) {
          tpixel.y = (tensorHeight - 1) - tpixel.y;
        }
      }
      // Is one of the mouse buttons down?
      if (leftMouseDown) {
        SetPixel(tpixel.x, tpixel.y, SCOLOR(currentSet, PE_FGC), SBUFFER(currentSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        SetPixel(tpixel.x, tpixel.y, SCOLOR(currentSet, PE_BGC), SBUFFER(currentSet, PE_FRAMEBUFFER));
      }
    } else if (IsInsideBox(mouse, altBox)) {
      // The alternate display.
      tpixel = GetDisplayPixel(mouse, altBox);
      // Mirror corrections
      if (SBOOL(alternateSet, PE_MIRROR_V)) {
        if (tpixel.x > tensorWidth / 2) {
          tpixel.x = (tensorWidth - 1) - tpixel.x;
        }
      }
      if (SBOOL(alternateSet, PE_MIRROR_H)) {
        if (tpixel.y > tensorHeight / 2) {
          tpixel.y = (tensorHeight - 1) - tpixel.y;
        }
      }
      if (leftMouseDown) {
        SetPixel(tpixel.x, tpixel.y, SCOLOR(alternateSet, PE_FGC), SBUFFER(alternateSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        SetPixel(tpixel.x, tpixel.y, SCOLOR(alternateSet, PE_BGC), SBUFFER(alternateSet, PE_FRAMEBUFFER));
      }
    }

    // If the input mode changed, we need to refresh the display
    if (inputMode != oldInputMode) {
      oldInputMode = inputMode;
      lastHover = INVALID;
      confirmed = NO;
      refreshAll = YES;
    }

    // Check for whole gui refresh.
    if (refreshAll) {
      refreshAll = NO;
      ClearWindow();
      DrawDisplayTexts(thisHover);
      DrawPreviewBorder(PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y, tensorWidth, tensorHeight, displaySet == OO_CURRENT);
      DrawPreviewBorder(PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y, tensorWidth, tensorHeight, displaySet == OO_ALTERNATE);
      UpdateInfoDisplay(displaySet ? alternateSet : currentSet);
      UpdateDisplays(currentSet, YES, NO, global_intensity_limit);
      UpdateDisplays(alternateSet, NO, NO, global_intensity_limit);
      refreshGui = YES;
    }

    // Check for info display refresh.
    if (refreshGui) {
      refreshGui = NO;
      UpdateGUI();
    }

    if (exitProgram) break;

  } // End FOREVER program loop

  // Backup the pattern sets in case we didnt mean to do this.
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    SavePatternSet('0' + i, i, YES, YES);
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


// The thing that happens at every frame.
void DrawNewFrame(int set, bool_t isPrimary) {

  // Cycles through the pattern sets.  Allows you to set up a bunch of pattern
  // sets and switch between them one at a time at some interval.
  if (isPrimary) {
    ProcessModes(set);

    UpdateDisplays(set, YES, YES, global_intensity_limit);
  } else {
    if (currentSet != alternateSet) {
      ProcessModes(set);
    }
    UpdateDisplays(set, NO, NO, global_intensity_limit);
  }

}

// Time to make a mess of the array.
void ProcessModes(int set) {
  int currentSet = set; // Overrides global used in D* macros.

  // Scroll direction randomizer
  if (DBOOL(PE_SCROLLRANDEN)) {
    if (!(rand() % DINT(PE_SCROLLRANDOM))) {
      DENUM(PE_SCROLLDIR) = rand() % DIR_COUNT;
    }
  }

  // Foreground color cycle.
  if (DENUM(PE_FGCYCLE)) {
    DCOLOR(PE_FGC) = ColorCycle(currentSet, DENUM(PE_FGCYCLE), &DINT(PE_CYCLESAVEFG), DINT(PE_FGRAINBOW));
  }

  // Change background color.
  if (DENUM(PE_BGCYCLE)) {
    DCOLOR(PE_BGC) = ColorCycle(currentSet, DENUM(PE_BGCYCLE), &DINT(PE_CYCLESAVEBG), DINT(PE_BGRAINBOW));
  }

  // Slap an image down on the display (every frame).
  if (DBOOL(PE_POSTIMAGE)) {
    DrawImage(imageSeed[currentSet], DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
      DFLOAT(PE_IMAGEEXP), DBOOL(PE_IMAGEALIAS), DBUFFER(PE_FRAMEBUFFER));
    DFLOAT(PE_IMAGEANGLE) += DFLOAT(PE_IMAGEINC);
  }

  // Image one-shot
  if (DBOOL(PE_IMAGEALL)) {
    DrawImage(imageSeed[currentSet], DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
      DFLOAT(PE_IMAGEEXP), DBOOL(PE_IMAGEALIAS), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_IMAGEALL) = NO;
  }

  // Seed the entire array with the foreground color.
  if (DBOOL(PE_FGCOLORALL)) {
    ColorAll(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_FGCOLORALL) = NO;
  }

  // Scroller.
  if (DBOOL(PE_SCROLL)) {
    Scroll(set, DENUM(PE_SCROLLDIR), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_ALL);
  }

  // Planar scrollers.  The enables are build into the enumeration.  Scroll()
  // will ignore them if they are SM_HOLD.
  Scroll(set, DENUM(PE_SHIFTRED), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_RED);
  Scroll(set, DENUM(PE_SHIFTGREEN), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_GREEN);
  Scroll(set, DENUM(PE_SHIFTBLUE), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_BLUE);
  Scroll(set, DENUM(PE_SHIFTCYAN), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_CYAN);
  Scroll(set, DENUM(PE_SHIFTMAGENTA), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_MAGENTA);
  Scroll(set, DENUM(PE_SHIFTYELLOW), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_YELLOW);

  // Draw a solid bar up the side we are scrolling from.
  if (DBOOL(PE_BARSEED)) {
    DrawSideBar(currentSet);
  }

  // Draw a pulse up the side we are scrolling from.
  if (DBOOL(PE_SIDEPULSE)) {
    DrawSidePulse(currentSet);
  }

  // Write a column of text on the side opposite of the scroll direction.
  if (DBOOL(PE_TEXTSEED)) {
    // Scroll provided by scroller if its on.
    WriteSlice(currentSet);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (DBOOL(PE_CELLFUN)) {
    // Give each pixel a color value.
    CellFun(currentSet);
  }

  // Bouncy bouncy (ick).
  if (DINT(PE_BOUNCER) != 0) {
    ScrollCycle(currentSet);
  }

  // Bam!  Draw some horizontal bars.
  if (DBOOL(PE_HBARS)) {
    HorizontalBars(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_HBARS) = NO;
  }

  // Bam! Vertical bars.
  if (DBOOL(PE_VBARS)) {
    VerticalBars(DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_VBARS) = NO;
  }

  // Crossbar seeds
  if (DENUM(PE_CROSSBAR) != CB_NONE) {
    CrossBars(currentSet);
  }

  // Random dots.  Most useful seed ever.
  if (DBOOL(PE_RANDOMDOT)) {
    RandomDots(DCOLOR(PE_FGC), DINT(PE_RANDOMDOTCOEF), DBUFFER(PE_FRAMEBUFFER));
  }

  // Snail seed.
  if (DBOOL(PE_SNAIL)) {
    SnailSeed(currentSet, DINT(PE_SNAIL_POS));
    DINT(PE_SNAIL_POS)++;
  }

  // Fast snail seed.
  if (DBOOL(PE_FASTSNAIL)) {
    FastSnailSeed(currentSet, DINT(PE_FASTSNAILP));
    DINT(PE_FASTSNAILP)++;
  }

  // Fader
  if (DBOOL(PE_FADE)) {
    FadeAll(DINT(PE_FADEINC), DENUM(PE_FADEMODE), DBUFFER(PE_FRAMEBUFFER));
  }

  // Averager
  if (DBOOL(PE_DIFFUSE)) {
    Diffuse(DFLOAT(PE_DIFFUSECOEF), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER));
  }

  // Clear screen.
  if (DBOOL(PE_BGCOLORALL)) {
    ColorAll(DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
    DBOOL(PE_BGCOLORALL) = NO;
  }

  // Multiplier
  if (DBOOL(PE_MULTIPLY)) {
    Multiply(DFLOAT(PE_MULTIPLYBY), DBUFFER(PE_FRAMEBUFFER));
  }

  // Rotozoomer
  if (DBOOL(PE_PRERZ)) {
    Rotate(DFLOAT(PE_PRERZANGLE), DFLOAT(PE_PRERZEXPAND), DBOOL(PE_PRERZALIAS), DBUFFER(PE_FRAMEBUFFER), DBUFFER(PE_FRAMEBUFFER), NO);
    DFLOAT(PE_PRERZANGLE) += DFLOAT(PE_PRERZINC);
  }

  // Zero the red.
  if (DBOOL(PE_NORED)) {
    ClearRed(currentSet);
  }

  // Zero the blue.
  if (DBOOL(PE_NOBLUE)) {
    ClearBlue(currentSet);
  }

  // Zero the green.
  if (DBOOL(PE_NOGREEN)) {
    ClearGreen(currentSet);
  }

  // Mirrors
  if (DBOOL(PE_MIRROR_V)) {
    VerticalMirror(DBUFFER(PE_FRAMEBUFFER));
  }
  if (DBOOL(PE_MIRROR_H)) {
    HorizontalMirror(DBUFFER(PE_FRAMEBUFFER));
  }

  // Post rotation increment.
  if (DBOOL(PE_POSTRZ)) {
    DFLOAT(PE_POSTRZANGLE) += DFLOAT(PE_POSTRZINC);
  }
}

void ScrollCycle(int set) {
  int currentSet = set; // Global override
  static int count = 0;
  static dir_e direction = DIR_UP;

  if (count > DINT(PE_BOUNCER)) count = DINT(PE_BOUNCER);
  count--;
  if (count <= 0) {
    direction = (direction + 1) % DIR_COUNT;
    count = DINT(PE_BOUNCER);
  }

  Scroll(set, direction, DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_ALL);
}

void VerticalMirror(unsigned char * buffer) {
  int x, y;
  color_t temp;

  for (x = 0; x < tensorWidth / 2; x++) {
    for (y = 0; y < tensorHeight; y++) {
      temp = GetPixel(x, y, buffer);
      SetPixel((tensorWidth - 1) - x, y, temp, buffer);
    }
  }
}

void HorizontalMirror(unsigned char * buffer) {
  int x, y;
  color_t temp;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight / 2; y++) {
      temp = GetPixel(x, y, buffer);
      SetPixel(x, (tensorHeight - 1) - y, temp, buffer);
    }
  }
}

void SnailSeed(int set, int position) {
  int currentSet = set; // Override global currentSet for D*.
  int i;
  int x = 0, y = 0;
  int tp = 0;
  int dir = 0;
  int xh,xl,yh,yl;
  xh = TENSOR_WIDTH;
  xl = 0;
  yh = TENSOR_HEIGHT;
  yl = 0;
  for (i = 0; i < TENSOR_HEIGHT * TENSOR_WIDTH; i++) {
    if (position == tp) {
      //~ printf("tp: %i, (%i, %i)\n", tp, x, y);
      SetPixel(x, y, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH) {
      DBOOL(PE_SNAIL) = NO;
    }
    switch(dir) {
      case 0:
        x++;
        if (x >= xh - 1) {
          dir = 1;
          xh--;
        }
        break;
      case 1:
        y++;
        if (y >= yh - 1) {
          dir = 2;
          yh--;
        }
        break;
      case 2:
        x--;
        if (x <= xl) {
          dir = 3;
          xl++;
        }
        break;
      case 3:
        y--;
        if (y <= yl + 1) {
          dir = 0;
          yl++;
        }
        break;
      default:
        break;
    }
  }
}

void FastSnailSeed(int set, int position) {
  int currentSet = set; // Overrides global currentSet for D*.
  int i;
  int x = -1, y = 0;
  int tp = 0;
  int dir = 0;
  int xh,xl,yh,yl;
  xh = TENSOR_WIDTH - 1;
  xl = 0;
  yh = TENSOR_HEIGHT - 1;
  yl = 1;
  for (i = 0; i < TENSOR_HEIGHT * TENSOR_WIDTH; i++) {
    if (position == tp) {
      //~ printf("tp: %i, (%i, %i)\n", tp, x, y);
      SetPixel(x, y, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      SetPixel(x, y + 1, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      SetPixel(x + 1, y + 1, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      SetPixel(x + 1, y, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH / 4 + 5) {
      DBOOL(PE_FASTSNAIL) = NO;
      break;
    }
    switch(dir) {
      case 0:
        x++;
        x++;
        if (x >= xh - 1) {
          dir = 1;
          xh--;
          xh--;
        }
        break;
      case 1:
        y++;
        y++;
        if (y >= yh - 1) {
          dir = 2;
          yh--;
          yh--;
        }
        break;
      case 2:
        x--;
        x--;
        if (x < 0) {
          x++;
        }
        if (x <= xl) {
          dir = 3;
          xl++;
          xl++;
        }
        break;
      case 3:
        y--;
        y--;
        if (y <= yl + 1) {
          dir = 0;
          yl++;
          yl++;
        }
        break;
      default:
        break;
    }
  }
}

// Crossing bars
typedef struct cbars_t {
  bool_t inProgress;
  dir_e orient;
  int x, y;
} cbars_t;
#define CBARSMAX 50  // Really prolly don't need more than TENSOR_WIDTH
void CrossBars(int set) {
  int currentSet = set;  // Override global currentSet for D*.
  static cbars_t cbars[PATTERN_SET_COUNT][CBARSMAX];
  static bool_t initial = YES;
  int i, j;

  // Set up the initial structures
  if (initial) {
    initial = NO;
    for (j = 0; j < PATTERN_SET_COUNT; j++) {
      for (i = 0 ; i < CBARSMAX; i++) {
        cbars[j][i].inProgress = NO;
      }
    }
  }

  // Find out if we add a new one.
  for (i = 0 ; i < CBARSMAX; i++) {
    if (!cbars[set][i].inProgress) {
      if (!(rand() % DINT(PE_CBLIKELY))) {
        cbars[set][i].inProgress = YES;
        // H or V?
        switch (DENUM(PE_CROSSBAR)) {
          case CB_VERT:
            cbars[set][i].orient = rand() % 2 ? DIR_UP : DIR_DOWN;
            break;
          case CB_HORZ:
            cbars[set][i].orient = rand() % 2 ? DIR_LEFT : DIR_RIGHT;
            break;
          default:
          case CB_BOTH:
            cbars[set][i].orient = rand() % 2 ? (rand() % 2 ? DIR_RIGHT : DIR_LEFT) : (rand() % 2 ? DIR_UP : DIR_DOWN);
            break;
        }

        switch (cbars[set][i].orient) {
          case DIR_UP:
            cbars[set][i].x = rand() % tensorWidth;
            cbars[set][i].y = tensorHeight - 1;
            break;
          case DIR_DOWN:
            cbars[set][i].x = rand() % tensorWidth;
            cbars[set][i].y = 0;
            break;
          case DIR_LEFT:
            cbars[set][i].y = rand() % tensorHeight;
            cbars[set][i].x = 0;
            break;
          default:
          case DIR_RIGHT:
            cbars[set][i].y = rand() % tensorHeight;
            cbars[set][i].x = tensorWidth - 1;
            break;
        }
        break;  // The for loop.
      }
    }
  }

  // Process the array.
  for (i = 0; i < CBARSMAX; i++) {
    if (cbars[set][i].inProgress) {
      SetPixel(cbars[set][i].x, cbars[set][i].y, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      switch(cbars[set][i].orient) {
        case DIR_UP:
          cbars[set][i].y--;
          if (cbars[set][i].y < 0) {
            cbars[set][i].inProgress = NO;
          }
          break;
        case DIR_DOWN:
          cbars[set][i].y++;
          if (cbars[set][i].y >= tensorHeight) {
            cbars[set][i].inProgress = NO;
          }
          break;
        case DIR_LEFT:
          cbars[set][i].x++;
          if (cbars[set][i].x >= tensorWidth) {
            cbars[set][i].inProgress = NO;
          }
          break;
        case DIR_RIGHT:
        default:
          cbars[set][i].x--;
          if (cbars[set][i].x < 0) {
            cbars[set][i].inProgress = NO;
          }
          break;
      }
    }
  }
}

// Key press processing for confirmation boxes.
bool_t HandleConfirmation(SDL_Keycode key, unsigned char *selected) {
  bool_t confirmationHandled = YES;

  switch (key) {
    case SDLK_y:
      SavePatternSet(keySave, setSave, YES, NO);
      break;

    case SDLK_n:
    case SDLK_ESCAPE:
      snprintf(statusText, sizeof(statusText), "Save action cancelled.");
      break;

    case SDLK_RETURN:
    case SDLK_SPACE:
      if (*selected == YES) {
        SavePatternSet(keySave, setSave, YES, NO);
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
  int source = displayCommand[item].dataSource;
  int boxCount = enumerations[patternElements[source].etype].size;

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
        return(HandleCommand(set, displayCommand[i].commands[j].command));
      }
    }
  }

  // Check other place for commands.
  for ( i = 0 ; i < otherCommandsCount; i++) {
    for (j = 0 ; j < MOUSE_COUNT; j++) {
      if ((otherCommands[i].commands[j].key == key) && (otherCommands[i].commands[j].mod == mod)) {
        return(HandleCommand(set, otherCommands[i].commands[j].command));
      }
    }
  }

  // Check for load or save to disk commands.
  // Save the current pattern set as <key>.now (for 0-9, a-z, only)
  if (mod == (KMOD_ALT | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      SavePatternSet(key, set, NO, NO);
      return NO;  // NO = Don't exit the program
    }
  }

  // Load a pattern set from <key>.now into the current set.
  if (mod == (KMOD_CTRL | KMOD_SHIFT)) {
    if ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) {
      LoadPatternSet(key, set);
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
bool_t HandleCommand(int set, command_e command) {
  int i;

  switch(command) {

    case COM_SIDEPULSE: SBOOL(set, PE_SIDEPULSE) = !SBOOL(set, PE_SIDEPULSE); break;
    case COM_SCROLL_RAND_EN: SBOOL(set, PE_SCROLLRANDEN) = !SBOOL(set, PE_SCROLLRANDEN); break;
    case COM_SCROLL_RAND_RST: SINT(set, PE_SCROLLRANDOM) = patternElements[PE_SCROLLRANDOM].initial.i; break;
    case COM_SCROLL_RAND_UP: SINT(set, PE_SCROLLRANDOM)++; break;
    case COM_SCROLL_RAND_DOWN:
      SINT(set, PE_SCROLLRANDOM)--;
      if (SINT(set, PE_SCROLLRANDOM) < 1) SINT(set, PE_SCROLLRANDOM) = 1;
      break;
    case COM_MIRROR_H: SBOOL(set, PE_MIRROR_H) = !SBOOL(set, PE_MIRROR_H); break;
    case COM_MIRROR_V: SBOOL(set, PE_MIRROR_V) = !SBOOL(set, PE_MIRROR_V); break;
    case COM_FG_WHEEL_UP:
      SENUM(set, PE_FGCYCLE) = CM_NONE;
      SENUM(set, PE_FGE) = (SENUM(set, PE_FGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      break;
    case COM_FG_WHEEL_DOWN:
      SENUM(set, PE_FGCYCLE) = CM_NONE;
      SENUM(set, PE_FGE)--;
      if (SENUM(set, PE_FGE) < CE_RED) SENUM(set, PE_FGE) = CE_COUNT - 1;
      SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      break;
    case COM_BG_WHEEL_UP:
      SENUM(set, PE_BGCYCLE) = CM_NONE;
      SENUM(set, PE_BGE) = (SENUM(set, PE_BGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      break;
    case COM_BG_WHEEL_DOWN:
      SENUM(set, PE_BGCYCLE) = CM_NONE;
      SENUM(set, PE_BGE)--;
      if (SENUM(set, PE_BGE) < CE_RED) SENUM(set, PE_BGE) = CE_COUNT - 1;
      SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      break;
    case COM_CROSSB_RST: SENUM(set, PE_CROSSBAR) = CB_NONE; break;
    case COM_CROSSB_UP: SENUM(set, PE_CROSSBAR) = (SENUM(set, PE_CROSSBAR) + 1) % CB_COUNT; break;
    case COM_CROSSB_DOWN:
      SENUM(set, PE_CROSSBAR)--;
      if (SENUM(set, PE_CROSSBAR) < 0) SENUM(set, PE_CROSSBAR) = CB_COUNT - 1;
      break;
    case COM_SNAILFAST:
      SBOOL(set, PE_FASTSNAIL) = !SBOOL(set, PE_FASTSNAIL);
      SINT(set, PE_FASTSNAILP) = 0;
      break;
    case COM_SNAIL:
      SBOOL(set, PE_SNAIL) = !SBOOL(set, PE_SNAIL);
      SINT(set, PE_SNAIL_POS) = 0;
      break;
    case COM_BLEND_RST: alternateBlend = 0; break;
    case COM_BLEND_INC:
      alternateBlend += SFLOAT(set, PE_FLOATINC);
      if (alternateBlend > 1.0) alternateBlend = 1.0;
      break;
    case COM_BLEND_DEC:
      alternateBlend -= SFLOAT(set, PE_FLOATINC);
      if (alternateBlend < 0.0) alternateBlend = 0.0;
      break;
    case COM_INTENSITY_RST: SFLOAT(set, PE_INTENSITY) = patternElements[PE_INTENSITY].initial.f; break;
    case COM_INTENSITY_UP:
      SFLOAT(set, PE_INTENSITY) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_INTENSITY) > patternElements[PE_INTENSITY].max.f)
        SFLOAT(set, PE_INTENSITY) = patternElements[PE_INTENSITY].max.f;
      break;
    case COM_INTENSITY_DOWN:
      SFLOAT(set, PE_INTENSITY) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_INTENSITY) < patternElements[PE_INTENSITY].min.f)
        SFLOAT(set, PE_INTENSITY) = patternElements[PE_INTENSITY].min.f;
      break;
    case COM_BLENDINC_RST: alternateBlendRate = 0.01; break;
    case COM_BLENDINC_INC:
      alternateBlendRate += SFLOAT(set, PE_FLOATINC);
      if (alternateBlendRate > 1.0) alternateBlendRate = 1.0;
      break;
    case COM_BLENDINC_DEC:
      alternateBlendRate -= SFLOAT(set, PE_FLOATINC);
      if (alternateBlendRate < 0.0) alternateBlendRate = 0.0;
      break;
    case COM_BLENDSWITCH: autoBlend = !autoBlend; break;
    case COM_EXCHANGE: i = currentSet; currentSet = alternateSet; alternateSet = i; break;
    case COM_OPERATE: displaySet = (displaySet + 1) % OO_COUNT; break;
    case COM_ALTERNATE_INC: alternateSet = (alternateSet + 1) % PATTERN_SET_COUNT; break;
    case COM_ALTERNATE_DEC: alternateSet--; if (alternateSet < 0) alternateSet = PATTERN_SET_COUNT - 1; break;
    case COM_LIVE_INC: currentSet = (currentSet + 1) % PATTERN_SET_COUNT; break;
    case COM_LIVE_DEC: currentSet--; if (currentSet < 0) currentSet = PATTERN_SET_COUNT - 1; break;
    case COM_CELL: SBOOL(set, PE_CELLFUN) = !SBOOL(set, PE_CELLFUN); break;
    case COM_TEXT_FLIP: SBOOL(set, PE_FONTFLIP) = !SBOOL(set, PE_FONTFLIP); break;
    case COM_TEXT_REVERSE: SBOOL(set, PE_FONTDIR) = !SBOOL(set, PE_FONTDIR); break;
    case COM_TEXT: SBOOL(set, PE_TEXTSEED) = !SBOOL(set, PE_TEXTSEED); break;
    case COM_BOUNCE_RST: SINT(set, PE_BOUNCER) = patternElements[PE_BOUNCER].initial.i; break;
    case COM_BOUNCE_UP: SINT(set, PE_BOUNCER)++; break;
    case COM_BOUNCE_DOWN: if (--SINT(set, PE_BOUNCER) < 0) SINT(set, PE_BOUNCER) = 0; break;
    case COM_FADE: SBOOL(set, PE_FADE) = !SBOOL(set, PE_FADE); break;
    case COM_DIFFUSE: SBOOL(set, PE_DIFFUSE) = !SBOOL(set, PE_DIFFUSE); break;
    case COM_ROLL: SBOOL(set, PE_ROLLOVER) = !SBOOL(set, PE_ROLLOVER); break;
    case COM_SCROLL: SBOOL(set, PE_SCROLL) = !SBOOL(set, PE_SCROLL); break;
    case COM_HBAR: SBOOL(set, PE_HBARS) = YES; break;
    case COM_VBAR: SBOOL(set, PE_VBARS) = YES; break;
    case COM_FGALL: SBOOL(set, PE_FGCOLORALL) = YES; break;
    case COM_BGALL: SBOOL(set, PE_BGCOLORALL) = YES; break;
    case COM_IMAGEALL: SBOOL(set, PE_IMAGEALL) = YES; break;
    case COM_RDOT: SBOOL(set, PE_RANDOMDOT) = !SBOOL(set, PE_RANDOMDOT); break;
    case COM_FGCYCLE_RST:
      SENUM(set, PE_FGCYCLE) = patternElements[PE_FGCYCLE].initial.e;
      SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      break;
    case COM_BGCYCLE_RST:
      SENUM(set, PE_BGCYCLE) = patternElements[PE_BGCYCLE].initial.e;
      SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      break;
    case COM_FGCYCLE_UP:
      SENUM(set, PE_FGCYCLE) = (SENUM(set, PE_FGCYCLE) + 1) % CM_COUNT;
      if (SENUM(set, PE_FGCYCLE) == CM_NONE) {
        SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      }
      break;
    case COM_BGCYCLE_UP:
      SENUM(set, PE_BGCYCLE) = (SENUM(set, PE_BGCYCLE) + 1) % CM_COUNT;
      if (SENUM(set, PE_BGCYCLE) == CM_NONE) {
        SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      }
      break;
    case COM_FGCYCLE_DOWN:
      SENUM(set, PE_FGCYCLE)--;
      if (SENUM(set, PE_FGCYCLE) < 0) SENUM(set, PE_FGCYCLE) = CM_COUNT - 1;
      if (SENUM(set, PE_FGCYCLE) == CM_NONE) {
        SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      }
      break;
    case COM_BGCYCLE_DOWN:
      SENUM(set, PE_BGCYCLE)--;
      if (SENUM(set, PE_BGCYCLE) < 0) SENUM(set, PE_BGCYCLE) = CM_COUNT - 1;
      if (SENUM(set, PE_BGCYCLE) == CM_NONE) {
        SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      }
      break;
    case COM_CYCLESET:
      cyclePatternSets = !cyclePatternSets;
      cycleFrameCount = SINT(set, PE_FRAMECOUNT);
      break;
    case COM_MODEOFF:
      for (i = 0; i < patternElementCount; i++) {
        if (patternElements[i].type == ET_BOOL) {
          SBOOL(set, i) = NO;
        }
      }
      SENUM(set, PE_SHIFTBLUE) = SM_HOLD;
      SENUM(set, PE_SHIFTCYAN) = SM_HOLD;
      SENUM(set, PE_SHIFTGREEN) = SM_HOLD;
      SENUM(set, PE_SHIFTMAGENTA) = SM_HOLD;
      SENUM(set, PE_SHIFTYELLOW) = SM_HOLD;
      SENUM(set, PE_SHIFTRED) = SM_HOLD;
      SENUM(set, PE_CROSSBAR) = CB_NONE;
      cyclePatternSets = NO;
      break;
    // For load, we want to replace the displayed set.
    case COM_LOADSET0: if (displaySet == OO_ALTERNATE) alternateSet = 0; else currentSet = 0; break;
    case COM_LOADSET1: if (displaySet == OO_ALTERNATE) alternateSet = 1; else currentSet = 1; break;
    case COM_LOADSET2: if (displaySet == OO_ALTERNATE) alternateSet = 2; else currentSet = 2; break;
    case COM_LOADSET3: if (displaySet == OO_ALTERNATE) alternateSet = 3; else currentSet = 3; break;
    case COM_LOADSET4: if (displaySet == OO_ALTERNATE) alternateSet = 4; else currentSet = 4; break;
    case COM_LOADSET5: if (displaySet == OO_ALTERNATE) alternateSet = 5; else currentSet = 5; break;
    case COM_LOADSET6: if (displaySet == OO_ALTERNATE) alternateSet = 6; else currentSet = 6; break;
    case COM_LOADSET7: if (displaySet == OO_ALTERNATE) alternateSet = 7; else currentSet = 7; break;
    case COM_LOADSET8: if (displaySet == OO_ALTERNATE) alternateSet = 8; else currentSet = 8; break;
    case COM_LOADSET9: if (displaySet == OO_ALTERNATE) alternateSet = 9; else currentSet = 9; break;
    case COM_FADEMODE:
      SENUM(set, PE_FADEMODE) = (SENUM(set, PE_FADEMODE) + 1) % FM_COUNT;
      break;
    case COM_NORED: SBOOL(set, PE_NORED) = !SBOOL(set, PE_NORED); break;
    case COM_NOGREEN: SBOOL(set, PE_NOGREEN) = !SBOOL(set, PE_NOGREEN); break;
    case COM_NOBLUE: SBOOL(set, PE_NOBLUE) = !SBOOL(set, PE_NOBLUE); break;
    case COM_TEXTRESET: SINT(set, PE_PIXELINDEX) = INVALID; break;
    case COM_POSTROTATE:
      SBOOL(set, PE_POSTRZ) = !SBOOL(set, PE_POSTRZ);
      SFLOAT(set, PE_POSTRZANGLE) = 0;
      break;
    case COM_PREROTATE: SBOOL(set, PE_PRERZ) = !SBOOL(set, PE_PRERZ); break;
    case COM_AA: SBOOL(set, PE_ALIAS) = !SBOOL(set, PE_ALIAS); break;
    case COM_MULTIPLY: SBOOL(set, PE_MULTIPLY) = !SBOOL(set, PE_MULTIPLY); break;
    case COM_SIDEBAR: SBOOL(set, PE_BARSEED) = !SBOOL(set, PE_BARSEED); break;
    case COM_ORIENTATION:
      tensor_landscape_p = !tensor_landscape_p;
      SetDims();
      DrawPreviewBorder(PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y,  tensorWidth, tensorHeight,displaySet == OO_CURRENT);
      DrawPreviewBorder(PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y, tensorWidth, tensorHeight, displaySet == OO_ALTERNATE);
      break;
    case COM_TEXT_MODE_UP: SENUM(set, PE_TEXTMODE) = (SENUM(set, PE_TEXTMODE) + 1) % TS_COUNT; break;
    case COM_TEXT_MODE_DOWN:
      SENUM(set, PE_TEXTMODE)--;
      if (SENUM(set, PE_TEXTMODE) < 0) SENUM(set, PE_TEXTMODE) = TS_COUNT - 1;
      break;
    case COM_RP_UP: SENUM(set, PE_SHIFTRED) = (SENUM(set, PE_SHIFTRED) + 1) % SM_COUNT; break;
    case COM_RP_RST: SENUM(set, PE_SHIFTRED) = patternElements[PE_SHIFTRED].initial.e; break;
    case COM_RP_DOWN:
      SENUM(set, PE_SHIFTRED)--;
      if (SENUM(set, PE_SHIFTRED) < 0) SENUM(set, PE_SHIFTRED) = SM_COUNT - 1;
      break;
    case COM_GP_UP: SENUM(set, PE_SHIFTGREEN) = (SENUM(set, PE_SHIFTGREEN) + 1) % SM_COUNT; break;
    case COM_GP_RST: SENUM(set, PE_SHIFTGREEN) = patternElements[PE_SHIFTGREEN].initial.e; break;
    case COM_GP_DOWN:
      SENUM(set, PE_SHIFTGREEN)--;
      if (SENUM(set, PE_SHIFTGREEN) < 0) SENUM(set, PE_SHIFTGREEN) = SM_COUNT - 1;
      break;
    case COM_BP_UP: SENUM(set, PE_SHIFTBLUE) = (SENUM(set, PE_SHIFTBLUE) + 1) % SM_COUNT; break;
    case COM_BP_RST: SENUM(set, PE_SHIFTBLUE) = patternElements[PE_SHIFTBLUE].initial.e; break;
    case COM_BP_DOWN:
      SENUM(set, PE_SHIFTBLUE)--;
      if (SENUM(set, PE_SHIFTBLUE) < 0) SENUM(set, PE_SHIFTBLUE) = SM_COUNT - 1;
      break;
    case COM_FG_DEC:
      SENUM(set, PE_FGE)--;
      if (SENUM(set, PE_FGE) < CE_RED) SENUM(set, PE_FGE) = CE_COUNT - 1;
      SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      break;
    case COM_FG_INC:
      SENUM(set, PE_FGE) = (SENUM(set, PE_FGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_FGC) = namedColors[SENUM(set, PE_FGE)].color;
      break;
    case COM_BG_DEC:
      SENUM(set, PE_BGE)--;
      if (SENUM(set, PE_BGE) < 0) SENUM(set, PE_BGE) = CE_COUNT - 1;
      SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      break;
    case COM_BG_INC:
      SENUM(set, PE_BGE) = (SENUM(set, PE_BGE) + 1) % CE_COUNT;
      SCOLOR(set, PE_BGC) = namedColors[SENUM(set, PE_BGE)].color;
      break;
    // We want to copy the display set to the selected set.  This accomplishes that.
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
    case COM_TEXTO_DEC:
      SINT(set, PE_TEXTOFFSET)--;
      if (SINT(set, PE_TEXTOFFSET) < 0) {
        if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
          SINT(set, PE_TEXTOFFSET) = tensorHeight - 1;
        } else {
          SINT(set, PE_TEXTOFFSET) = tensorWidth - 1;
        }
      }
      break;
    case COM_TEXTO_INC:
      SINT(set, PE_TEXTOFFSET)++;
      if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
        SINT(set, PE_TEXTOFFSET) %= tensorHeight;
      } else {
        SINT(set, PE_TEXTOFFSET) %= tensorWidth;
      }
      break;
    case COM_DELAY_DEC:
      SINT(set, PE_DELAY)--;
      if (SINT(set, PE_DELAY) < 1) SINT(set, PE_DELAY) = 1;  // Can't be 0!
      break;
    case COM_DELAY_RST:
      SINT(set, PE_DELAY) = INITIAL_DELAY;
      break;
    case COM_DELAY_INC:
      SINT(set, PE_DELAY)++;
      break;
    case COM_CP_UP: SENUM(set, PE_SHIFTCYAN) = (SENUM(set, PE_SHIFTCYAN) + 1) % SM_COUNT; break;
    case COM_CP_RST: SENUM(set, PE_SHIFTCYAN) = patternElements[PE_SHIFTCYAN].initial.e; break;
    case COM_CP_DOWN:
      SENUM(set, PE_SHIFTCYAN)--;
      if (SENUM(set, PE_SHIFTCYAN) < 0) SENUM(set, PE_SHIFTCYAN) = SM_COUNT - 1;
      break;
    case COM_YP_UP: SENUM(set, PE_SHIFTYELLOW) = (SENUM(set, PE_SHIFTYELLOW) + 1) % SM_COUNT; break;
    case COM_YP_RST: SENUM(set, PE_SHIFTYELLOW) = patternElements[PE_SHIFTYELLOW].initial.e; break;
    case COM_YP_DOWN:
      SENUM(set, PE_SHIFTYELLOW)--;
      if (SENUM(set, PE_SHIFTYELLOW) < 0) SENUM(set, PE_SHIFTYELLOW) = SM_COUNT - 1;
      break;
    case COM_MP_UP: SENUM(set, PE_SHIFTMAGENTA) = (SENUM(set, PE_SHIFTMAGENTA) + 1) % SM_COUNT; break;
    case COM_MP_RST: SENUM(set, PE_SHIFTMAGENTA) = patternElements[PE_SHIFTMAGENTA].initial.e; break;
    case COM_MP_DOWN:
      SENUM(set, PE_SHIFTMAGENTA)--;
      if (SENUM(set, PE_SHIFTMAGENTA) < 0) SENUM(set, PE_SHIFTMAGENTA) = SM_COUNT - 1;
      break;
    case COM_IMAGE: SBOOL(set, PE_POSTIMAGE) = !SBOOL(set, PE_POSTIMAGE); break;
    case COM_FADE_DEC: SINT(set, PE_FADEINC)--; break;
    case COM_FADE_INC: SINT(set, PE_FADEINC)++; break;
    case COM_FADE_RST: SINT(set, PE_FADEINC) = INITIAL_FADE_INC; break;
    case COM_DIFFUSE_DEC: SFLOAT(set, PE_DIFFUSECOEF) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_DIFFUSE_RST: SFLOAT(set, PE_DIFFUSECOEF) = INITIAL_DIFF_COEF; break;
    case COM_DIFFUSE_INC: SFLOAT(set, PE_DIFFUSECOEF) += SFLOAT(set, PE_FLOATINC); break;
    case COM_EXPAND_DEC: SFLOAT(set, PE_POSTEXP) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_EXPAND_RST: SFLOAT(set, PE_POSTEXP) = INITIAL_EXPAND; break;
    case COM_EXPAND_INC: SFLOAT(set, PE_POSTEXP) += SFLOAT(set, PE_FLOATINC); break;
    case COM_RANDOM_DEC:
      SINT(set, PE_RANDOMDOTCOEF) -= max(1, SINT(set, PE_RANDOMDOTCOEF) / 50);
      if (SINT(set, PE_RANDOMDOTCOEF) < 1) SINT(set, PE_RANDOMDOTCOEF) = 1;
      break;
    case COM_RANDOM_RST: SINT(set, PE_RANDOMDOTCOEF) = INITIAL_RAND_MOD; break;
    case COM_RANDOM_INC:
      SINT(set, PE_RANDOMDOTCOEF) += max(1, SINT(set, PE_RANDOMDOTCOEF) / 50);
      if (SINT(set, PE_RANDOMDOTCOEF) > 20000) SINT(set, PE_RANDOMDOTCOEF) = 20000;
      break;
    case COM_CB_DOWN:
      SINT(set, PE_CBLIKELY) -= max(1, SINT(set, PE_CBLIKELY) / 50);
      if (SINT(set, PE_CBLIKELY) < 1) SINT(set, PE_CBLIKELY) = 1;
      break;
    case COM_CB_RST: SINT(set, PE_CBLIKELY) = patternElements[PE_CBLIKELY].initial.i; break;
    case COM_CB_UP:
      SINT(set, PE_CBLIKELY) += max(1, SINT(set, PE_CBLIKELY) / 50);
      if (SINT(set, PE_CBLIKELY) > 20000) SINT(set, PE_CBLIKELY) = 20000;
      break;
    case COM_POSTROT_DEC: SFLOAT(set, PE_POSTRZANGLE) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTROT_RST: SFLOAT(set, PE_POSTRZANGLE) = INITIAL_POSTROT_ANGLE; break;
    case COM_POSTROT_INC: SFLOAT(set, PE_POSTRZANGLE) += SFLOAT(set, PE_FLOATINC); break;
    case COM_LOAD0: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(0, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD1: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(1, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD2: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(2, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD3: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(3, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD4: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(4, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD5: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(5, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD6: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(6, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD7: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(7, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD8: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(8, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_LOAD9: memcpy(SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(9, PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char)); break;
    case COM_FGRAINBOW_DEC: SINT(set, PE_FGRAINBOW)--; break;
    case COM_FGRAINBOW_RST: SINT(set, PE_FGRAINBOW) = patternElements[PE_FGRAINBOW].initial.i; break;
    case COM_FGRAINBOW_INC: SINT(set, PE_FGRAINBOW)++; break;
    case COM_BGRAINBOW_DEC: SINT(set, PE_BGRAINBOW)--; break;
    case COM_BGRAINBOW_RST: SINT(set, PE_BGRAINBOW) = patternElements[PE_BGRAINBOW].initial.i; break;
    case COM_BGRAINBOW_INC: SINT(set, PE_BGRAINBOW)++; break;
    case COM_CYCLE_MODE: SENUM(set, PE_FGCYCLE) = (SENUM(set, PE_FGCYCLE) + 1) % CM_COUNT; break;
    case COM_CYCLE_MODE_DOWN:
      SENUM(set, PE_FGCYCLE)--;
      if (SENUM(set, PE_FGCYCLE) < 0) SENUM(set, PE_FGCYCLE) = CM_COUNT - 1;
      break;
    case COM_PREROT_DEC: SFLOAT(set, PE_PRERZANGLE)-= SFLOAT(set, PE_FLOATINC); break;
    case COM_PREROT_RST: SFLOAT(set, PE_PRERZANGLE) = INITIAL_PREROT_ANGLE; break;
    case COM_PREROT_INC: SFLOAT(set, PE_PRERZANGLE)+= SFLOAT(set, PE_FLOATINC); break;
    case COM_SCROLL_UPC:
      SENUM(set, PE_SCROLLDIR) = DIR_UP;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_DOWNC:
      SENUM(set, PE_SCROLLDIR) = DIR_DOWN;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_LEFTC:
      SENUM(set, PE_SCROLLDIR) = DIR_LEFT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_RIGHTC:
      SENUM(set, PE_SCROLLDIR) = DIR_RIGHT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_CYCLE_UP:
      SENUM(set, PE_SCROLLDIR) = (SENUM(set, PE_SCROLLDIR) + 1) % DIR_COUNT;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_SCROLL_CYCLE_DOWN:
      SENUM(set, PE_SCROLLDIR)--;
      if (SENUM(set, PE_SCROLLDIR) < 0) SENUM(set, PE_SCROLLDIR) = DIR_COUNT - 1;
      SBOOL(set, PE_SCROLL) = YES;
      break;
    case COM_MULT_DEC: SFLOAT(set, PE_MULTIPLYBY) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_MULT_RST: SFLOAT(set, PE_MULTIPLYBY) = INITIAL_MULTIPLIER; break;
    case COM_MULT_INC: SFLOAT(set, PE_MULTIPLYBY) += SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTSPEED_DEC: SFLOAT(set, PE_POSTRZINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_POSTSPEED_RST: SFLOAT(set, PE_POSTRZINC) = INITIAL_POSTROT_INC; break;
    case COM_POSTSPEED_INC: SFLOAT(set, PE_POSTRZINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_STEP_INC: SFLOAT(set, PE_FLOATINC) *= 10; break;
    case COM_STEP_RST: SFLOAT(set, PE_FLOATINC) = INITIAL_FLOAT_INC; break;
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
    case COM_RETURN:
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX)] = '\n';
      SSTRING(set, PE_TEXTBUFFER)[SINT(set, PE_TEXTINDEX) + 1] = 0x00;
      SINT(set, PE_TEXTINDEX)++;
      if (SINT(set, PE_TEXTINDEX) >= (patternElements[PE_TEXTBUFFER].size - 2)) {
        SINT(set, PE_TEXTINDEX)--;
      }
      break;
    case COM_DELETE:
      SINT(set, PE_TEXTINDEX) = 0;
      SSTRING(set, PE_TEXTBUFFER)[0] = 0x00;
      break;
    case COM_INVALID: case COM_NONE: case COM_COUNT:
      break;
    case COM_PREROTINC_RST: SFLOAT(set, PE_PRERZINC) = patternElements[PE_PRERZINC].initial.f; break;
    case COM_PREROTINC_INC: SFLOAT(set, PE_PRERZINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_PREROTINC_DEC: SFLOAT(set, PE_PRERZINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_PREALIAS: SBOOL(set, PE_PRERZALIAS) = !SBOOL(set, PE_PRERZALIAS); break;
    case COM_PREEXP_RST: SFLOAT(set, PE_PRERZEXPAND) = patternElements[PE_PRERZEXPAND].initial.f; break;
    case COM_PREEXP_INC: SFLOAT(set, PE_PRERZEXPAND) += SFLOAT(set, PE_FLOATINC); break;
    case COM_PREEXP_DEC: SFLOAT(set, PE_PRERZEXPAND) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMINC_RST: SFLOAT(set, PE_IMAGEINC) = patternElements[PE_IMAGEINC].initial.f; break;
    case COM_IMINC_INC: SFLOAT(set, PE_IMAGEINC) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMINC_DEC: SFLOAT(set, PE_IMAGEINC) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMALIAS: SBOOL(set, PE_IMAGEALIAS) = !SBOOL(set, PE_IMAGEALIAS); break;
    case COM_IMEXP_RST: SFLOAT(set, PE_IMAGEEXP) = patternElements[PE_IMAGEEXP].initial.f; break;
    case COM_IMEXP_INC: SFLOAT(set, PE_IMAGEEXP) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMEXP_DEC: SFLOAT(set, PE_IMAGEEXP) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMANGLE_RST: SFLOAT(set, PE_IMAGEANGLE) = patternElements[PE_IMAGEANGLE].initial.f; break;
    case COM_IMANGLE_INC: SFLOAT(set, PE_IMAGEANGLE) += SFLOAT(set, PE_FLOATINC); break;
    case COM_IMANGLE_DEC: SFLOAT(set, PE_IMAGEANGLE) -= SFLOAT(set, PE_FLOATINC); break;
    case COM_IMXOFFSET_RST: SFLOAT(set, PE_IMAGEXOFFSET) = patternElements[PE_IMAGEXOFFSET].initial.f; break;
    case COM_IMXOFFSET_INC:
      SFLOAT(set, PE_IMAGEXOFFSET) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEXOFFSET) > 1.0) SFLOAT(set, PE_IMAGEXOFFSET) = 1.0;
      break;
    case COM_IMXOFFSET_DEC:
      SFLOAT(set, PE_IMAGEXOFFSET) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEXOFFSET) < 0.0) SFLOAT(set, PE_IMAGEXOFFSET) = 0.0;
      break;
    case COM_IMYOFFSET_RST: SFLOAT(set, PE_IMAGEYOFFSET) = patternElements[PE_IMAGEYOFFSET].initial.f; break;
    case COM_IMYOFFSET_INC:
      SFLOAT(set, PE_IMAGEYOFFSET) += SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEYOFFSET) > 1.0) SFLOAT(set, PE_IMAGEYOFFSET) = 1.0;
      break;
    case COM_IMYOFFSET_DEC:
      SFLOAT(set, PE_IMAGEYOFFSET) -= SFLOAT(set, PE_FLOATINC);
      if (SFLOAT(set, PE_IMAGEYOFFSET) < 0.0) SFLOAT(set, PE_IMAGEYOFFSET) = 0.0;
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
  }
  return NO;  // NO, don't exit the program.
}

// Weighted Averager.  Boolean isToroid causes the edges to wrap.
// Average each pixel with those around it (above, below, left, right) into a
// new buffer and then copy it back to the original.
void Diffuse(float diffusionCoeff, bool_t isToroid, unsigned char *buffer) {
  int x, y, i, j, xx, yy;
  color_t colorTemp, finalColor[TENSOR_WIDTH * TENSOR_HEIGHT];
  float weightSumR, weightSumG, weightSumB;
  float divisor;

  // Go through each pixel
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Initial values
      divisor = 0;
      weightSumR = weightSumG = weightSumB = 0;

      // Find the pixel environment
      for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
          xx = x + i;
          yy = y + j;

          if (isToroid) {
            // Wrap around the edges.
            if (xx < 0) xx = tensorWidth - 1;
            if (xx >= tensorWidth) xx = 0;
            if (yy < 0) yy = tensorHeight - 1;
            if (yy >= tensorHeight) yy = 0;
          }

          if ((xx >= 0) && (xx < tensorWidth) && (yy >= 0) && (yy < tensorHeight)) {
            colorTemp = GetPixel(xx, yy, buffer);

            // Pixel 0 (us) counts as 1.
            if ((i == 0) && (j == 0)) {
              weightSumR += colorTemp.r;
              weightSumG += colorTemp.g;
              weightSumB += colorTemp.b;
              divisor = divisor + 1.0;

            // The others count with weight.
            } else if ((i == 0) || (j == 0)) {
              weightSumR += (diffusionCoeff * colorTemp.r);
              weightSumG += (diffusionCoeff * colorTemp.g);
              weightSumB += (diffusionCoeff * colorTemp.b);
              divisor += diffusionCoeff;
            }
          }
        }
      }

      finalColor[(y * tensorWidth) + x].r = weightSumR / divisor;
      finalColor[(y * tensorWidth) + x].g = weightSumG / divisor;
      finalColor[(y * tensorWidth) + x].b = weightSumB / divisor;
    }
  }

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      SetPixel(x,y,finalColor[(y * tensorWidth) + x], buffer);
    }
  }
}

// Color all the pixels a certain color.
void ColorAll(color_t color, unsigned char *fb) {
  int x,y;
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      SetPixel(x, y, color, fb);
    }
  }
}

// Multiply all pixels by a value.
void Multiply(float multiplier, unsigned char *buffer) {
  int i;

  for (i = 0; i < TENSOR_BYTES; i++) {
    buffer[i] = (unsigned char)((float) buffer[i] * multiplier);
  }
}


// Lighten (or darker) all the pixels by a certain value.
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer) {
  int x, y, r, g, b;
  color_t colorTemp;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Grab the pixel color.
      colorTemp = GetPixel(x, y, buffer);

      if (fadeMode == FM_MODULAR) {
        // Modular color fade is easy regardless of inc sign.
        // Unsigned chars rollover in either direction.
        colorTemp.r += inc;
        colorTemp.g += inc;
        colorTemp.b += inc;
      } else {
        // For the limiter, we'll do the math with ints.
        r = colorTemp.r;
        g = colorTemp.g;
        b = colorTemp.b;
        r += inc;
        g += inc;
        b += inc;
        if ( r < 0 ) r = 0;
        if ( g < 0 ) g = 0;
        if ( b < 0 ) b = 0;
        if ( r > 255 ) r = 255;
        if ( g > 255 ) g = 255;
        if ( b > 255 ) b = 255;
        colorTemp.r = r;
        colorTemp.g = g;
        colorTemp.b = b;
      }

      SetPixel(x, y, colorTemp, buffer);
    }
  }
}




// Set a single pixel a particular color.
void SetPixel(int x, int y, color_t color, unsigned char *buffer) {
  if ((x >= tensorWidth) || (y >= tensorHeight) ||
      (x < 0) || (y < 0)) {
    //~ fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
    return;
  }
  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Set a single pixel a particular color by color plane.
// Rediculous.
void SetPixelByPlane(int x, int y, color_t color, unsigned char plane, unsigned char *buffer) {
  if ((x >= tensorWidth) || (y >= tensorHeight)) {
    fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
  }
  switch(plane) {
    case PLANE_ALL:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_RED:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      break;

    case PLANE_GREEN:
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_BLUE:
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    // More complicated.
    case PLANE_CYAN:
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;

    case PLANE_YELLOW:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
      break;

    case PLANE_MAGENTA:
      buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
      buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
      break;
    default:
      break;

  }
}

// Set a single pixel a particular color with alpha blending.
// Is this ever used?  Does it work?
void SetPixelA(int x, int y, color_t color, unsigned char *buffer) {
  color_t colorTemp;
  float a, r1, g1, b1, r2, g2, b2;

  if ((x >= tensorWidth) || (y >= tensorHeight)) {
    fprintf(stderr, "Attempt to set pixel outside of frame buffer! %i, %i\n", x, y);
  }

  // Input colors.
  r1 = color.r;
  g1 = color.g;
  b1 = color.b;

  // Get the current color.
  colorTemp = GetPixel(x, y, buffer);
  r2 = colorTemp.r;
  g2 = colorTemp.g;
  b2 = colorTemp.b;

  // Normalize the alpha value. Normal = (in - min) / (max - min)
  a = color.a;
  a = a / 255;

  // Calculate the blended outputs = value = (1 - a) Value0 + a Value1
  color.r = (unsigned char) (a * r1) + ((1 - a) * r2);
  color.g = (unsigned char) (a * g1) + ((1 - a) * g2);
  color.b = (unsigned char) (a * b1) + ((1 - a) * b2);

  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Get the color of a particular pixel.
color_t GetPixel(int x, int y, unsigned char *buffer) {
  color_t colorTemp = cBlack;
  if (x >= tensorWidth || x < 0 || y >= tensorHeight || y < 0) return colorTemp;
  colorTemp.r = buffer[(y * tensorWidth * 3) + (x * 3) + 0];
  colorTemp.g = buffer[(y * tensorWidth * 3) + (x * 3) + 1];
  colorTemp.b = buffer[(y * tensorWidth * 3) + (x * 3) + 2];
  colorTemp.a = 255; // We don't return an alpha for a written pixel?
  return colorTemp;
}

// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
void UpdateDisplays(int set, bool_t isPrimary, bool_t sendToTensor, float intensity_limit) {
  unsigned char fba[TENSOR_BYTES];
  static unsigned char fbb[TENSOR_BYTES];
  unsigned char *buffer;
  int i;
  int currentSet = set; // Override global currentSet for D*.

  // The preview...
  buffer = DBUFFER(PE_FRAMEBUFFER);

  // Post rotation
  if (DBOOL(PE_POSTRZ)) {

    // Apply the post rotation, so that it doesn't affect the feedback buffer.
    Rotate(DFLOAT(PE_POSTRZANGLE), DFLOAT(PE_POSTEXP), DBOOL(PE_ALIAS), fba, buffer, YES);

  } else {

    // Just copy the buffer, because we aren't rotating it.
    memcpy(fba, DBUFFER(PE_FRAMEBUFFER), patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  // Set Output intensity
  for (i = 0; i < TENSOR_BYTES; i++) {
    fba[i] = (unsigned char)((float) fba[i] * DFLOAT(PE_INTENSITY));
  }

  if (set == alternateSet) {
    // For alternate blending with the post.
    memcpy(fbb, fba, patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  // Send to the preview and to the wall
  if (isPrimary) {

    // Alternate blending
    BlendAlternate(fba, fbb);

    UpdatePreview(PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y, fba);

    // Tensor global output intensity limit.
    for (i = 0 ; i < TENSOR_BYTES; i++) {
      fba[i] = (unsigned char)((float) fba[i] * intensity_limit);
    }
    if (enableTensor && sendToTensor) {
      UpdateTensor(fba);
    }

  } else {
    UpdatePreview(PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y, fba);
  }

  return;
}

void BlendAlternate(unsigned char *fba, unsigned char *fbb) {
  int i, j;
  color_t temp;

  // Too small to see...
  //~ if (alternateBlend < 0.001) return;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j++) {
      temp = GetPixel(i,j,fbb);
      temp.a = alternateBlend * 255;
      SetPixelA(i, j, temp, fba);
    }
  }

  // Autoblending
  if (autoBlend) {
    alternateBlend += alternateBlendRate;
    if (alternateBlend > 1.0) {
      alternateBlend = 0.0;
      autoBlend = NO;
      i = currentSet;
      currentSet = alternateSet;
      alternateSet = i;
    }
  }
}

// Rotate
void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src, unsigned char exclude) {
  SDL_Surface *rotatedSurface = NULL;
  SDL_Surface *s1 = NULL;
  SDL_Surface *s2 = NULL;
  SDL_Rect offset;

  // Create a surface for the current wall image.
  s1 = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight, 32, 0, 0, 0, 0);
  if (!s1) {
    fprintf(stderr, "Unable to create rotation surface: %s\n", SDL_GetError());
    return;
  }

  // Grab the image to the surface.
  FBToSurface(s1, fb_src);

  // To prevent edge cut-off when rotating, we'll do the rotation on a larger
  // surface, then we'll blit it back over the original surface, keepind the
  // rotation and black blending the outer edges by a small amount.
  s2 = SDL_CreateRGBSurface(0, tensorWidth * 4, tensorHeight * 4, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
  if (!s2) {
    fprintf(stderr, "Unable to create 2nd rotation surface: %s\n", SDL_GetError());
    SDL_FreeSurface(s1);
    return;
  }

  // Add the outer blending
  if (exclude) {
    SDL_FillRect(s2, NULL, SDL_MapRGBA(s2->format, 0, 0, 0, 255));
  } else {
    SDL_FillRect(s2, NULL, SDL_MapRGBA(s2->format, 0, 0, 0, 5));
  }

  // Copy the small surface to the large one.
  offset.x = (s2->w - s1->w) / 2;
  offset.y = (s2->h - s1->h) / 2;
  SDL_SetSurfaceBlendMode(s1, SDL_BLENDMODE_NONE);
  SDL_BlitSurface(s1, NULL, s2, &offset);

  // Rotate / scale it.
  rotatedSurface = rotozoomSurface (s2, angle, expansion, aliasmode ? 1 : 0);
  if (!rotatedSurface) {
    fprintf(stderr, "Error rotating surface: %s\n", SDL_GetError());
    SDL_FreeSurface(s2);
    SDL_FreeSurface(s1);
    return;
  }
  SDL_FreeSurface(s2);

  // Create a new surface of the wall size.
  s2 = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight, 32, 0,0,0,0);
  if (!s2) {
    fprintf(stderr, "Error creating rotation surface 3: %s\n", SDL_GetError());
    SDL_FreeSurface(s1);
    SDL_FreeSurface(rotatedSurface);
    return;
  }

  // Copy the original surface back in.
  offset.x = (s2->w - s1->w) / 2;
  offset.y = (s2->h - s1->h) / 2;
  SDL_SetSurfaceBlendMode(s1, SDL_BLENDMODE_NONE);
  SDL_BlitSurface(s1, NULL, s2, NULL);

  // Copy the rotated surface over with blending around the edges.
  offset.x = 0 - (rotatedSurface->w - s2->w) / 2;
  offset.y = 0 - (rotatedSurface->h - s2->h) / 2;
  SDL_SetSurfaceBlendMode(rotatedSurface, SDL_BLENDMODE_BLEND);
  SDL_BlitSurface(rotatedSurface, NULL, s2, &offset);

  // Copy the result back to the frame buffer.
  SurfaceToFB(fb_dst, s2);

  // Free the memories
  SDL_FreeSurface(rotatedSurface);
  SDL_FreeSurface(s2);
  SDL_FreeSurface(s1);
}


// Frame buffer to SDL surface
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB) {
  color_t pixel;
  unsigned char *surfacePixels;
  int x, y;

  surfacePixels = surface->pixels;
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Get pixel color.
      pixel = GetPixel(x, y, FB);
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 0] = pixel.b;
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 1] = pixel.g;
      surfacePixels[y * surface->pitch + x * (surface->pitch / surface->w) + 2] = pixel.r;
    }
  }
  return surface;
}

// SDL Surface to frame buffer
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface) {
  Uint32 *pixel;
  Uint8 r,g,b;
  int x,y;

  for(x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      pixel = surface->pixels + y * surface->pitch + x * (surface->pitch / surface->w);
      SDL_GetRGB(*pixel, surface->format, &r, &g, &b);
      FB[((y * tensorWidth * 3) + (x * 3)) + 0] = r;
      FB[((y * tensorWidth * 3) + (x * 3)) + 1] = g;
      FB[((y * tensorWidth * 3) + (x * 3)) + 2] = b;
    }
  }

  return FB;
}


// Send frame buffer to preview area.  The preview area is a square
// encompassing the actual preview, so we should center the buffer within it.
void UpdatePreview(int xOffset, int yOffset, unsigned char *buffer) {
  color_t pixel;
  int x, y;
  int maxDim;

  // Get the outer border dimensions.
  x = (tensorWidth * PREVIEW_PIXEL_SIZE);
  y = (tensorHeight * PREVIEW_PIXEL_SIZE);

  // Get the preview area square dimension.
  maxDim = max(x, y);

  // Adjust our offsets to fit inside it.
  xOffset = xOffset + PREVIEW_BORDER + (maxDim - x) / 2;
  yOffset = yOffset + PREVIEW_BORDER + (maxDim - y) / 2;

  // Draw out the pixels.
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {

      // Get pixel color from the buffer.
      pixel = GetPixel(x, y, buffer);

      // Draw the output pixel as a square of dimension PREVIEW_PIXEL_SIZE - 1.
      // This leaves us with a border around our pixels.
      DrawBox(xOffset + (x * PREVIEW_PIXEL_SIZE), yOffset + (y * PREVIEW_PIXEL_SIZE),
              PREVIEW_PIXEL_SIZE - 1, PREVIEW_PIXEL_SIZE - 1, pixel);
    }
  }
}


// Scroller buffer manipulation
void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane) {
  int currentSet = set;  // Override
  int x, y, i;
  color_t rollSave[TENSOR_WIDTH];  // TENSOR_WIDTH is larger than TENSOR_HEIGHT

  // If the topology of the surface is toroidal, we save aside the edge so that
  // it can be wrapped.
  if (toroidal) {
    switch(direction) {
      case DIR_UP:
        for(i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, 0, fb);
        break;

      case DIR_DOWN:
        if (DBOOL(PE_MIRROR_H)) {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, (tensorHeight - 1)/2, fb);
        } else {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, tensorHeight - 1, fb);
        }
        break;

      case DIR_RIGHT:
        if (DBOOL(PE_MIRROR_V)) {
          for (i = 0; i < tensorHeight; i++) rollSave[i] = GetPixel((tensorWidth - 1) / 2, i, fb);
        } else {
          for (i = 0; i < tensorHeight; i++) rollSave[i] = GetPixel(tensorWidth - 1, i, fb);
        }
        break;

      case DIR_LEFT:
        for (i = 0; i < tensorHeight; i++) rollSave[i] = GetPixel(0, i, fb);
        break;

      default:
        break;
    }
  }

  // Make the shift.
  switch(direction) {
    case DIR_UP:
      if (DBOOL(PE_MIRROR_H)) {
        for (y = 0; y < (tensorHeight - 1) / 2; y++) {
          for (x = 0; x < tensorWidth; x++) {
            SetPixelByPlane(x, y, GetPixel(x, y+1, fb), plane, fb);
          }
        }
      } else {
        for (y = 0; y < (tensorHeight - 1); y++) {
          for (x = 0; x < tensorWidth; x++) {
            SetPixelByPlane(x, y, GetPixel(x, y+1, fb), plane, fb);
          }
        }
      }
      break;

    case DIR_DOWN:
      for (y = (tensorHeight - 1); y > 0; y--) {
        for (x = 0; x < tensorWidth; x++) {
          SetPixelByPlane(x, y, GetPixel(x, y - 1, fb), plane, fb);
        }
      }
      break;

    case DIR_LEFT:
      if (DBOOL(PE_MIRROR_V)) {
        for (y = 0; y < tensorHeight; y++) {
          for (x = 0; x < (tensorWidth - 1) / 2; x++) {
            SetPixelByPlane(x, y, GetPixel(x + 1, y, fb),plane, fb);
          }
        }
      } else {
        for (y = 0; y < tensorHeight; y++) {
          for (x = 0; x < (tensorWidth - 1); x++) {
            SetPixelByPlane(x, y, GetPixel(x + 1, y, fb),plane, fb);
          }
        }
      }
      break;

    case DIR_RIGHT:
      for (y = 0; y < tensorHeight; y++) {
        if (DBOOL(PE_MIRROR_V)) {
          for (x = (tensorWidth - 1) / 2; x > 0; x--) {
            SetPixelByPlane(x, y, GetPixel(x - 1, y, fb),plane, fb);
          }
        } else {
          for (x = (tensorWidth - 1); x > 0; x--) {
            SetPixelByPlane(x, y, GetPixel(x - 1, y, fb),plane, fb);
          }
        }
      }
      break;

    default:
      break;
  }

  // Paste the shifted out row onto the other side.
  if (toroidal) {
    switch(direction) {
      case DIR_UP:
        if (DBOOL(PE_MIRROR_H)) {
          for(i = 0; i < tensorWidth; i++)
            SetPixelByPlane(i, (tensorHeight - 1)/2, rollSave[i], plane, fb);
        } else {
          for(i = 0; i < tensorWidth; i++)
            SetPixelByPlane(i, tensorHeight - 1, rollSave[i], plane, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < tensorWidth; i++)
          SetPixelByPlane(i, 0, rollSave[i],plane, fb);
        break;

      case DIR_RIGHT:
        for (i = 0; i < tensorHeight; i++)
          SetPixelByPlane(0, i, rollSave[i],plane, fb);
        break;

      case DIR_LEFT:
        if (DBOOL(PE_MIRROR_V)) {
          for (i = 0; i < tensorHeight; i++)
            SetPixelByPlane((tensorWidth - 1)/2, i, rollSave[i],plane, fb);
        } else {
          for (i = 0; i < tensorHeight; i++)
            SetPixelByPlane(tensorWidth - 1, i, rollSave[i],plane, fb);
        }
        break;

      default:
        break;
    }
  }
}


void UpdateTensor(unsigned char *buffer) {
  unsigned char sendBuf [TENSOR_BYTES];
  int x, y;
  color_t temp;

  // Apply the pixel map
  for (y = 0; y < tensorHeight; y++) {
    for (x = 0; x < tensorWidth; x++) {
      temp = GetPixel(x, y, buffer);
      SetPixel(tensorPixelMap[x + (y * tensorWidth)].x, tensorPixelMap[x + (y * tensorWidth)].y, temp, sendBuf);
    }
  }

  tensor_send(sendBuf);
}


// Write a slice of text.
void WriteSlice(int set) {
  int currentSet = set;
  const unsigned char charColMasks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  int sliceColOrRow;  // The seed target column / row (depending on direction).
  int sliceIndex;     // A pixel of the target slice we are working on.
  char character;     // The current character being output.
  int pixelBufferSize;  // Number of pixel columns in the text buffer.
  int fontPixelIndex;  // Index into the font to a selected pixel.
  int bufferIndex;  // Index into the text buffer of the current character.
  int charCol;  // Index into the font pixels column of the current character.
  int i, j;
  int useRand;  // The random number used during staggering (if at all).
  unsigned char horizontal;
  unsigned char pixelOn;
  unsigned char textDirection;
  static unsigned char textStaggerFlag[TEXT_BUFFER_SIZE] = "a";
  color_t bgColor;

  // Initialize textStaggerFlag, if necessary.  This is used for 9 row stagger,
  // which is meant for an 8 pixel font height on a 9 pixel high display.
  if (textStaggerFlag[0] == 'a') {
    for (i = 0; i < TEXT_BUFFER_SIZE; i++) {
      textStaggerFlag[i] = rand() % 2;
    }
  }


  // Figure out which row or column to place the seed slice into depending on direction.
  horizontal = YES;
  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      sliceColOrRow = tensorWidth - 1;
      if (DBOOL(PE_MIRROR_V)) sliceColOrRow = (tensorWidth - 1) / 2;
      break;

    case DIR_UP:
      horizontal = NO;
      sliceColOrRow = tensorHeight - 1;
      if (DBOOL(PE_MIRROR_H)) sliceColOrRow = (tensorHeight - 1) / 2;
      break;

    case DIR_DOWN:
      horizontal = NO;
      // Fallthrough...
    case DIR_RIGHT:
    default:
      sliceColOrRow = 0;
      break;
  }

  // The pixel buffer correspends to the text buffer, but further subdivides
  // each text array index into a part for every pixel of the font width.
  pixelBufferSize = strlen(DSTRING(PE_TEXTBUFFER)) * FONT_WIDTH;

  // Trivially, no text in the buffer.
  if (pixelBufferSize == 0) {
    // Just exit.  This actually causes the background to stop scrolling, which
    // might not be desireable.
    return;
  }

  // Prior to writing out a line, we increment the pixel buffer index to
  // point to the next part of the line to be written.  This depends on
  // scroll direction, text direction.
  textDirection = DENUM(PE_SCROLLDIR);
  if (DBOOL(PE_FONTDIR) == BACKWARDS) {
    switch (textDirection) {
      case DIR_LEFT:
        textDirection = DIR_RIGHT;
        break;
      case DIR_RIGHT:
        textDirection = DIR_LEFT;
        break;
      case DIR_DOWN:
        textDirection = DIR_UP;
        break;
      case DIR_UP:
        textDirection = DIR_DOWN;
        break;
      default:
        break;
    }
  }
  if (DENUM(PE_SCROLLDIRLAST) != DENUM(PE_SCROLLDIR)) {

    // Looks like we changed direction.
    DENUM(PE_SCROLLDIRLAST) = DENUM(PE_SCROLLDIR);

    // tensorWidth is used here to preserve continuity of the text, even
    // across text buffer wrap-around, when the direction changes (but only
    // really between UP - DOWN or RIGHT - LEFT changes).
    switch(textDirection) {
      case DIR_LEFT:
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + tensorWidth) % pixelBufferSize;
        break;
      case DIR_RIGHT:
        DINT(PE_PIXELINDEX) = DINT(PE_PIXELINDEX) - tensorWidth;
        if (DINT(PE_PIXELINDEX) < 0) {
          DINT(PE_PIXELINDEX) = pixelBufferSize + DINT(PE_PIXELINDEX);
        }
        break;
      case DIR_DOWN:
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + tensorHeight) % pixelBufferSize;
        break;
      case DIR_UP:
        DINT(PE_PIXELINDEX) = DINT(PE_PIXELINDEX) - tensorHeight;
        if (DINT(PE_PIXELINDEX) < 0) {
          DINT(PE_PIXELINDEX) = pixelBufferSize + DINT(PE_PIXELINDEX);
        }
        break;
      default:
        break;
    }
  } else {
    // No change in direction, so we increment without worrying about continuity
    // preservation.
    switch(textDirection) {
      case DIR_LEFT:
      case DIR_DOWN:
        DINT(PE_PIXELINDEX) = (DINT(PE_PIXELINDEX) + 1) % pixelBufferSize;
        break;
      case DIR_RIGHT:
      case DIR_UP:
        DINT(PE_PIXELINDEX)--;
        if ((DINT(PE_PIXELINDEX) < 0) || (DINT(PE_PIXELINDEX) >= pixelBufferSize)) {
          DINT(PE_PIXELINDEX) = pixelBufferSize - 1;
        }
        break;
      default:
        break;
    }
  }

  // Now using the pixel index, we find out where in the text buffer we are.
  // (Integer division.)
  bufferIndex = DINT(PE_PIXELINDEX) / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (DINT(PE_PIXELINDEX) % FONT_WIDTH);

  // What the character is.  What it is, man.
  character = DSTRING(PE_TEXTBUFFER)[bufferIndex];

  // First we draw the seed's background according to our textStagger mode.
  useRand = 0;
  switch (DSENUM(PE_TEXTMODE, textMode_e)) {
    case TS_9ROWSTAGGER:
      // Stagger.  For landscape - 8 pixel font on 9 pixels high display.
      // Stagger the letter and fill in the pixel that isn't covered by the
      // letter with our background.
      sliceIndex = DINT(PE_TEXTOFFSET);
      if (!textStaggerFlag[bufferIndex]) sliceIndex += 8;
      if (horizontal) {
        sliceIndex = sliceIndex % tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex = sliceIndex % tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      useRand = textStaggerFlag[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = DINT(PE_TEXTOFFSET) - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = tensorHeight - 1;
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        if (sliceIndex < 0) sliceIndex = tensorWidth - 1;
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      sliceIndex = DINT(PE_TEXTOFFSET) + 8;
      if (horizontal) {
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < tensorHeight; i++) {
          SetPixelA(sliceColOrRow, i, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
        }
      } else {
        for (i = 0; i < tensorWidth; i++) {
          SetPixelA(i, sliceColOrRow, DCOLOR(PE_BGC), DBUFFER(PE_FRAMEBUFFER));
        }
      }
      break;

    default:
      // textStagger == TS_8ROW, which means no border, no stagger, no extra
      // background to draw, so do nothing.
      // textStagger == TS_NOBG, also no extra background to draw.
      break;
  }

  // Will the text itself have a background?  If not, set the bg alpha channel.
  bgColor = DCOLOR(PE_BGC);
  if (DENUM(PE_TEXTMODE) == TS_NOBG) bgColor.a = 0;
  if (DENUM(PE_TEXTMODE) == TS_BLACK) bgColor = cBlack;

  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    if (DBOOL(PE_FONTFLIP)) {
      j = (FONT_HEIGHT - 1) - i;
    } else {
      j = i;
    }
    fontPixelIndex = (j * FONT_CHARACTER_COUNT) + character;
    sliceIndex = i + useRand + DINT(PE_TEXTOFFSET);
    pixelOn = myfont[fontPixelIndex] & charColMasks[charCol];
    if (horizontal) {
      sliceIndex = sliceIndex % tensorHeight;
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? DCOLOR(PE_FGC) : bgColor, DBUFFER(PE_FRAMEBUFFER));
    } else {
      sliceIndex = sliceIndex % tensorWidth;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? DCOLOR(PE_FGC) : bgColor, DBUFFER(PE_FRAMEBUFFER));
    }
  }

  return;
}


void HorizontalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j += 2) {
      SetPixel(i, j, color, buffer);
    }
  }
}


void VerticalBars(color_t color, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i+=2) {
    for (j = 0; j < tensorHeight; j++) {
      SetPixel(i,j, color, buffer);
    }
  }
}


void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer) {
  int x,y;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      if (!(rand() % rFreq)) {
        SetPixel(x, y, color, buffer);
      }
    }
  }
}



color_t ColorCycle(int set, colorCycleModes_e cycleMode, int *cycleSaver, int cycleInc) {
  color_t colorTemp = cBlack;
  int inpos, inposo, inposn;
  int currentSet = set;

  // If the mode changed, we should make sure our position in the cycle limits
  // to the mode.
  switch(cycleMode) {
    case CM_RGB:
      *cycleSaver = (*cycleSaver + 1) % paletteRGB.size;
      colorTemp = namedColors[paletteRGB.palette[*cycleSaver]].color;
      break;

    case CM_CMY:
      *cycleSaver = (*cycleSaver + 1) % paletteCMY.size;
      colorTemp = namedColors[paletteCMY.palette[*cycleSaver]].color;
      break;

    case CM_SECONDARY:
      *cycleSaver = (*cycleSaver + 1) % paletteSec.size;
      colorTemp = namedColors[paletteSec.palette[*cycleSaver]].color;
      break;

    case CM_TERTIARY:
      *cycleSaver = (*cycleSaver + 1) % paletteTer.size;
      colorTemp = namedColors[paletteTer.palette[*cycleSaver]].color;
      break;

    case CM_GRAY:
      *cycleSaver = (*cycleSaver + 1) % paletteGry.size;
      colorTemp = namedColors[paletteGry.palette[*cycleSaver]].color;
      break;

    case CM_RAINBOW:
      *cycleSaver = (*cycleSaver + cycleInc) % (256 * 6);
      inposo = *cycleSaver % 256;
      inpos = *cycleSaver / 256;
      switch(inpos) {
        case 0: // R -> Y
          colorTemp.r = 255;
          colorTemp.g = inposo;
          colorTemp.b = 0;
          break;

        case 1: // Y -> G
          colorTemp.r = 255 - inposo;
          colorTemp.g = 255;
          colorTemp.b = 0;
          break;

        case 2: // G -> C
          colorTemp.r = 0;
          colorTemp.g = 255;
          colorTemp.b = inposo;
          break;

        case 3: // C -> B
          colorTemp.r = 0;
          colorTemp.g = 255 - inposo;
          colorTemp.b = 255;
          break;

        case 4: // B -> M
          colorTemp.r = inposo;
          colorTemp.g = 0;
          colorTemp.b = 255;
          break;

        case 5: // M -> R
          colorTemp.r = 255;
          colorTemp.g = 0;
          colorTemp.b = 255 - inposo;
          break;

        default:
          break;
      }
      break;

    case CM_RANDOM:
      colorTemp.r = rand() % 256;
      colorTemp.g = rand() % 256;
      colorTemp.b = rand() % 256;
      break;

    case CM_FGBGFADE:
      *cycleSaver = (*cycleSaver + cycleInc) % (256 * 2);
      inpos = *cycleSaver / 256;
      inposo = *cycleSaver % 256;
      switch(inpos) {
        case 0: // FG - BG
          colorTemp.r = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_FGE)].color.r + ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.r);
          colorTemp.g = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_FGE)].color.g + ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.g);
          colorTemp.b = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_FGE)].color.b + ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.b);
          break;

        case 1: // FG - BG
          colorTemp.r = ((inposo / 255.0) * namedColors[DENUM(PE_FGE)].color.r) + ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.r;
          colorTemp.g = ((inposo / 255.0) * namedColors[DENUM(PE_FGE)].color.g) + ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.g;
          colorTemp.b = ((inposo / 255.0) * namedColors[DENUM(PE_FGE)].color.b) + ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.b;

        default:
          break;
      }
      break;

    case CM_TERTTOBLACK:
      *cycleSaver = (*cycleSaver + cycleInc) % (256 * 24);
      if (*cycleSaver < 0) {
        *cycleSaver += (256 * 24);
        if (*cycleSaver < 0) {
          *cycleSaver = 256 * 24;  // Extra sanity.
        }
      }
      inpos = (*cycleSaver / 256) % 2;
      inposn = *cycleSaver / 256 / 2;
      inposo = *cycleSaver % 256;
      //~ printf("1: %i, 2: %i, 3: %i, cyc: %i\n", inpos, inposn, inposo, *cycleSaver);
      switch(inpos) {
        case 0: // blk - FG
          colorTemp.r = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.r + ((inposo / 255.0) * namedColors[paletteTer.palette[inposn]].color.r);
          colorTemp.g = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.g + ((inposo / 255.0) * namedColors[paletteTer.palette[inposn]].color.g);
          colorTemp.b = ((255.0 - inposo) / 255) * namedColors[DENUM(PE_BGE)].color.b + ((inposo / 255.0) * namedColors[paletteTer.palette[inposn]].color.b);
          break;

        case 1: // FG - blk
          colorTemp.r = ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.r) + ((255.0 - inposo) / 255) * namedColors[paletteTer.palette[inposn]].color.r;
          colorTemp.g = ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.g) + ((255.0 - inposo) / 255) * namedColors[paletteTer.palette[inposn]].color.g;
          colorTemp.b = ((inposo / 255.0) * namedColors[DENUM(PE_BGE)].color.b) + ((255.0 - inposo) / 255) * namedColors[paletteTer.palette[inposn]].color.b;

        default:
          break;
      }
      break;

    case CM_INVALID: case CM_NONE: case CM_COUNT:
    //~ default:
      break;
  }

  // We used to return it...
  return colorTemp;
}

// Set some dimensions according to orientation
void SetDims(void) {
  int x,y;
  int maxdim;
  int xoffset, yoffset;

  // Set the width and height according to portrait mode.
  if (tensor_landscape_p) {
    tensorWidth = TENSOR_HEIGHT;
    tensorHeight = TENSOR_WIDTH;
  } else {
    tensorWidth = TENSOR_WIDTH;
    tensorHeight = TENSOR_HEIGHT;
  }

  // Define the preview boxes for mouse detection
  x = (tensorWidth * PREVIEW_PIXEL_SIZE);
  y = (tensorHeight * PREVIEW_PIXEL_SIZE);
  maxdim = max(x, y);
  xoffset = PREVIEW_LIVE_POS_X + PREVIEW_BORDER + (maxdim - x) / 2;
  yoffset = PREVIEW_LIVE_POS_Y + PREVIEW_BORDER + (maxdim - y) / 2;

  liveBox.x = xoffset;
  liveBox.y = yoffset;
  liveBox.w = x - 1;
  liveBox.h = y - 1;

  // Alternate preview
  xoffset = PREVIEW_ALT_POS_X + PREVIEW_BORDER + (maxdim - x) / 2;
  yoffset = PREVIEW_ALT_POS_Y + PREVIEW_BORDER + (maxdim - y) / 2;

  altBox.x = xoffset;
  altBox.y = yoffset;
  altBox.w = x - 1;
  altBox.h = y - 1;

  // Regenerate the default pixel map?
  GenerateDefaultPixelMap();
}



// Draw an image to the output with rotation and expansion.
// Rotation is acheived using SDL_gfx primitive rotozoom.
// Angle is given in degrees.  Offsets specify the center (0..1).
void DrawImage(SDL_Surface *image, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst) {
  SDL_Surface *s1 = NULL;
  SDL_Surface *s2 = NULL;
  SDL_Rect offset;
  int tempx, tempy;
  int cx, cy;

  // If not, don't.
  if (!image) return;

  // Recenter the image around the chosen center.  xoffset and yoffset are the
  // normalized (0-1) choice of center.  We'll expand the surface of the image
  // to make our chosen center the real center without cutting off any edges.

  // Calculate new image size
  tempx = (fabs((xoffset - 0.5) * 2) + 1) * image->w;
  tempy = (fabs((yoffset - 0.5) * 2) + 1) * image->h;

  // Calculate the new center.
  cx = xoffset * image->w;
  cy = yoffset * image->h;

  // Make the surface
  s1 =  SDL_CreateRGBSurface(0, tempx, tempy, 32, 0, 0, 0, 0);
  if (!s1) {
    fprintf(stderr, "Error getting scratch image for recentering: %s\n", SDL_GetError());
    return;
  }

  // Get the offsets to place the image within the new surface.
  if (cx < (image->w / 2)) {
    offset.x = tempx - image->w;
  } else {
    offset.x = 0;
  }
  if (cy < (image->h / 2)) {
    offset.y = tempy - image->h;
  } else {
    offset.y = 0;
  }

  // Copy the image onto the new surface with our chosen pixel centered.
  SDL_BlitSurface(image, NULL, s1, &offset);

  // Rotate and zoom the image.
  s2 = rotozoomSurface (s1, angle, expansion, aliasmode ? 1 : 0);
  if (!s2) {
    fprintf(stderr, "Error rotating image: %s\n", SDL_GetError());
    return;
  }
  SDL_FreeSurface(s1);

  // Create a tensor sized surface to copy the image to.
  s1 = SDL_CreateRGBSurface(0, tensorWidth, tensorHeight, 32, 0, 0, 0, 0);
  if (!s1) {
    fprintf(stderr, "Error creating tensor scratch surface: %s\n", SDL_GetError());
    SDL_FreeSurface(s2);
    return;
  }

  // Get the offsets between the image center and the surface center.
  offset.x = 0 - (s2->w - s1->w) / 2;
  offset.y = 0 - (s2->h - s1->h) / 2;

  // Copy / clip the rotated image to the tensor surface.
  SDL_BlitSurface(s2, NULL, s1, &offset);
  SDL_FreeSurface(s2);

  SurfaceToFB(fb_dst, s1);
  SDL_FreeSurface(s1);
}



// Or not.
void CellFun(int set) {
  int currentSet = set; // Override global for D*
  int x, y;
  color_t pixelColor, oldColor;

  DINT(PE_CELLFUNCOUNT)++;

  for(x = 0 ; x < tensorWidth ; x++) {
    for(y = 0 ; y < tensorHeight ; y++) {
      oldColor = GetPixel(x, y, DBUFFER(PE_FRAMEBUFFER));
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2);
      pixelColor.g = oldColor.g + pixelColor.r;
      pixelColor.b = DINT(PE_CELLFUNCOUNT);
      SetPixel(x, y, pixelColor,  DBUFFER(PE_FRAMEBUFFER));
    }
  }
}

// Saves a pattern set to a file.
// Return value indicates if the action requires confirmation.
void SavePatternSet(char key, int set, bool_t overWrite, bool_t backup) {
  char filename[20] = "";
  FILE *fp;
  int i,j;

  // Filename
  if (backup) {
    snprintf(filename, sizeof(filename), "%c.now.bak", key);
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
      case ET_INT: case ET_ENUM:
        fprintf(fp, "%i", SINT(set, i));
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
void LoadPatternSet(char key, int set) {
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
          case ET_INT: case ET_ENUM:
            DINT(parameterIndex) = atoi(value);
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
  SDL_FreeSurface(imageSeed[set]);
  imageSeed[set] = IMG_Load(DSTRING(PE_IMAGENAME));
  if (!imageSeed[set]) {
    fprintf(stderr, "Unable to load image: \"%s\"\n", DSTRING(PE_IMAGENAME));
  }
  snprintf(statusText, sizeof(statusText), "Loaded filename \"%s\" into set %i", filename, currentSet);
}


void DrawSideBar(int set) {
  int currentSet = set; // Override global for D*
  int i;

  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      for (i = 0; i < tensorHeight; i++) {
        if (DBOOL(PE_MIRROR_V)) {
          SetPixel((tensorWidth - 1) / 2, i, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        } else {
          SetPixel(tensorWidth - 1, i, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_RIGHT:
      for (i = 0; i < tensorHeight; i++) {
        SetPixel(0, i, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      }
      break;

    case DIR_UP:
      for (i = 0; i < tensorWidth; i++) {
        if (DBOOL(PE_MIRROR_H)) {
          SetPixel(i, (tensorHeight - 1) / 2, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        } else {
          SetPixel(i, tensorHeight - 1, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_DOWN:
      for (i = 0; i < tensorWidth; i++) {
        SetPixel(i, 0, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      }
      break;
    default:
      break;
  }
}

void DrawSidePulse(int set) {
  int currentSet = set; // Override global for D*
  static int pos[PATTERN_SET_COUNT];
  static int dir[PATTERN_SET_COUNT];
  static int initial = YES;
  int i;

  if (initial) {
    initial = NO;
    for (i = 0 ; i < PATTERN_SET_COUNT ; i++) {
      pos[i] = 0;
      dir[i] = 1;
    }
  }

  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      if (DBOOL(PE_MIRROR_V)) {
        SetPixel((tensorWidth - 1) / 2, pos[set], DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DBOOL(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
      } else {
        SetPixel(tensorWidth - 1, pos[set], DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DBOOL(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorHeight - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_RIGHT:
        SetPixel(0, pos[set], DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DBOOL(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
        if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_UP:
      if (DBOOL(PE_MIRROR_H)) {
        SetPixel(pos[set], (tensorHeight - 1) / 2, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DBOOL(PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      } else {
        SetPixel(pos[set], tensorHeight - 1, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DBOOL(PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_DOWN:
      SetPixel(pos[set], 0, DCOLOR(PE_FGC), DBUFFER(PE_FRAMEBUFFER));
      pos[set] += dir[set];
      if (DBOOL(PE_MIRROR_V)) {
        if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
      } else {
        if (pos[set] >= tensorWidth - 1) dir[set] = -1;
      }
      if (pos[set] <= 0) dir[set] = 1;
    default:
      break;
  }
}

void ClearRed(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 0] = 0;
  }
}

void ClearGreen(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 1] = 0;
  }
}

void ClearBlue(int set) {
  int currentSet = set; // Override global for D*
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    DBUFFER(PE_FRAMEBUFFER)[(i * 3) + 2] = 0;
  }
}





// Copy a pattern set to another.  Gotta be done element by element because
// pattern sets aren't internally contiguous.
void CopyPatternSet(int dst, int src) {
  int i;
  if (src == dst) return;
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
}

// Figure out which pixel of the display the mouse is over.
point_t GetDisplayPixel(point_t mouse, box_t box) {
  point_t pixel;
  pixel.x = (mouse.x - box.x) / PREVIEW_PIXEL_SIZE;
  pixel.y = (mouse.y - box.y) / PREVIEW_PIXEL_SIZE;
  return(pixel);
}

// Allows for remapping of pixel output and ip addresses without recompiling.
void LoadTensorMaps(void) {
  LoadPixelMap();
  LoadIPMap();
}

// For remapping individual pixels sent to tensor without recompiling.
void LoadPixelMap(void) {

  // Vars
  int x, y, xx, yy;
  FILE *fp = NULL;
  char filename[] = "tensorPixel.map";
  unsigned char validData = YES;
  int inchar;
  unsigned char commentLine = NO;
  char chunk[5];
  int digit = 0;
  int value;
  int count = 0;
  point_t tempMap[TENSOR_WIDTH * TENSOR_HEIGHT];

  // Populate the default pixel map.
  GenerateDefaultPixelMap();

  // Print the default pixel map.
  //for (y = 0; y < tensorHeight; y++) {
  //  for (x = 0; x < tensorWidth; x++) {
  //    printf("%i %i, ", tensorPixelMap[x + (y * tensorWidth)].x, tensorPixelMap[x + (y * tensorWidth)].y);
  //  }
  //  printf("\n");
  //}

  // Test for existence of a pixel map on disk.
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Pixel map file \"%s\" not found. Using internal default pixel map.\n", filename);
    return;
  }

  // Read in the pixel map character by character. Pixel map is formatted as a
  // series of x y coordinates separated by spaces, commas, and/or newlines. The
  // map may be preceeded or interspersed by any number of comment lines not
  // beginning with a digit (0-9). Any character other than a digit, a comma, a
  // space, or a newline in the map itself will cause the rest of the line to be
  // ignored. There must be exactly TENSOR_WIDTH * TENSORHEIGHT coordinate sets
  // to read in, otherwise the default map will be used. Remapped pixels are
  // read as a list of x, then y coordinates, read from left to right, then top
  // to bottom of the preview.
  FOREVER {

    // Read in a character.
    inchar = fgetc(fp);

    // If this line was a comment and we've hit the newline, go on to process
    // the next line.
    if (commentLine && inchar == '\n') {
      commentLine = NO;
      continue;  // Restart FOREVER loop.
    }

    // If this line is a comment line, ignore the input.
    if (commentLine) continue;  // Restart FOREVER loop.

    // If not a comment line, check for a digit.
    if (inchar >= '0' && inchar <= '9') {

      // Get the next digit and place it into the chunk string.
      chunk[digit] = inchar;
      chunk[digit + 1] = '\0';  // Keep it terminated.

      // Increase the digit number and check for nonsensically large numbers.
      digit++;
      if (digit >= 4) {
        fprintf(stderr, "Too many digits found in a coordinate in pixel map file \"%s\"!", filename);
        validData = NO;
        break;  // End FOREVER loop.
      }

    // Check for space, comma, newline, and EOF - these end the reading of a
    // coordinate.
    } else if (inchar == ' ' || inchar == ',' || inchar == '\n' || inchar == EOF) {

      // If we have digits in our chunk, then we have a coordinate to add to the
      // list.
      if (digit != 0) {

        // Convert the chunk to a number.
        value = atoi(chunk);

        // Check the number's range.
        if (value < 0 || value >= max(tensorHeight, tensorWidth)) {
          fprintf(stderr, "Coordinate too large (%i >= %i) in pixel map file \"%s\"!", value, max(tensorHeight, tensorWidth), filename);
          validData = NO;
          break;
        }

        // Even coordinates = x, odd coordinates = y.
        if (count % 2 == 0) {
          tempMap[count / 2].x = value;
        } else {
          tempMap[count / 2].y = value;
        }

        // Reset digit count for the next chunk.
        digit = 0;
        chunk[0] = '\0';

        // Increase the coordinate count and see if we gathered too many.
        count++;
        if (count > TENSOR_HEIGHT * TENSOR_WIDTH * 2) {
          fprintf(stderr, "Too many coordinates (%i) found in pixel map file \"%s\"!", count, filename);
          validData = NO;
          break;  // End FOREVER loop.
        }
      }

    // All other characters result in the rest of the line being ignored.
    } else {
      commentLine = YES;
      continue;  // Restart FOREVER loop.
    }

    // End of file breaks the loop.
    if (inchar == EOF) break;
  }

  // Close the file.
  fclose(fp);

  // Is the data valid so far?
  if (!validData) {
    fprintf(stderr, " Using internal default map.\n");
    return;
  }

  // Print out the pixel map that was read in.
  //for (y = 0; y < tensorHeight; y++) {
  //  for (x = 0; x < tensorWidth; x++) {
  //    printf("%i %i, ", tempMap[x + (y * tensorWidth)].x, tempMap[x + (y * tensorWidth)].y);
  //  }
  //  printf("\n");
  //}

  // Did we get the right number of coordinates?
  if (count != TENSOR_HEIGHT * TENSOR_WIDTH * 2) {
    fprintf(stderr, "Wrong number of coordinates found in pixel map file \"%s\": %i != %i. Using internal default map.\n",
      filename, count, TENSOR_HEIGHT * TENSOR_WIDTH * 2);
    return;
  }

  // Check for map uniqueness.  Any preview pixel sent to more than one tensor
  // pixel can only result in duplication on the display.
  for (y = 0; y < tensorHeight; y++) {
    for (x = 0; x < tensorWidth; x++) {

      // Check each pixel against every other pixel.
      for (yy = 0; yy < tensorHeight; yy++) {
        for (xx = 0; xx < tensorWidth; xx++) {

          // Skip the self.
          if (x != xx || y != yy) {
            if (tempMap[x + (y * tensorWidth)].x == tempMap[xx + (yy * tensorWidth)].x &&
                tempMap[x + (y * tensorWidth)].y == tempMap[xx + (yy * tensorWidth)].y) {
              fprintf(stderr, "Coordinate %i identical to coordinate %i.\n",
                x + (y * tensorWidth), xx + (yy * tensorWidth));
              validData = NO;
            }
          }
        }
      }
    }
  }

  // Unique?
  if (!validData) {
    fprintf(stderr, "Map \"%s\" is not unique.  Using internal default map.\n", filename);
    return;
  }

  // Consider the new map validish and copy it into program memory.
  for (y = 0; y < tensorHeight; y++) {
    for (x = 0; x < tensorWidth; x++) {
      tensorPixelMap[x + (y * tensorWidth)].x = tempMap[x + (y * tensorWidth)].x;
      tensorPixelMap[x + (y * tensorWidth)].y = tempMap[x + (y * tensorWidth)].y;
    }
  }

  // Haven't really thought through the whole portrait vs landscape thing, so
  // I don't know if the map is really valid.
  useDefaultPixelMap = NO;
  fprintf(stdout, "Found and using valid(ish) pixel map from file \"%s\".\n", filename);
}

void GenerateDefaultPixelMap(void) {
  int x, y;
  // If we loaded a pixel map from a file, we won't be doing this.
  if (!useDefaultPixelMap) return;

  // Populate the pixel map with default values.
  for (y = 0; y < tensorHeight; y++) {
    for (x = 0; x < tensorWidth; x++) {
      tensorPixelMap[x + (y * tensorWidth)].x = x;
      tensorPixelMap[x + (y * tensorWidth)].y = y;
    }
  }
}

// For remapping the IP addresses used by the UDP broadcast without recompiling.
// Function is only called once.  I didn't free some memory I malloced in the
// error handling bits, but who cares?
void LoadIPMap(void) {
  char filename[] = "tensorIP.map";
  FILE *fp = NULL;
  int inchar;
  int i;
  unsigned char commentLine = NO;
  int digit = 0;
  int count = 0;
  int value = 0;
  const char **tempMap1 = NULL;
  const char **tempMap2 = NULL;
  const char **tempMap3 = NULL;
  char *ipString = NULL;
  char chunk[5];
  char *ind;
  char *end;
  char *start;
  unsigned char validData = YES;

  // Populate the ip map with the default values (from drv-tensor.h).
  ipmap1 = Tensor_Section1_def;
  ipmap2 = Tensor_Section2_def;
  ipmap3 = Tensor_Section3_def;

  // Open the file.
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "IP address map file \"%s\" not found. Using internal defaults.\n", filename);
    return;
  }

  // Allocate the space necessary to hold all of the ip strings.
  ipString = malloc(IP_STRING_SIZE * TENSOR_PANEL_COUNT * TENSOR_ADDR_PER_PANEL * sizeof(char));
  if (!ipString) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  ind = ipString;
  end = ipString + (IP_STRING_SIZE * TENSOR_PANEL_COUNT * TENSOR_ADDR_PER_PANEL * sizeof(char));
  start = ipString;

  // Allocate the space necessary to hold the ip map.
  tempMap1 = malloc(TENSOR_ADDR_PER_PANEL * sizeof(char *));
  tempMap2 = malloc(TENSOR_ADDR_PER_PANEL * sizeof(char *));
  tempMap3 = malloc(TENSOR_ADDR_PER_PANEL * sizeof(char *));
  if (!tempMap1 || !tempMap2 || !tempMap3) {
    fprintf(stderr, "Unable to allocate memory to hold ip map from file!\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  // Process the file character by character.  Expected file format is a list
  // of 18 dotted quad decimal ip4 addresses in order.  Spaces or commas and
  // newlines will be accepted in place of dots, or in between addresses.  Any
  // line with a character other than a digit, dot, comma, or space will be
  // ignored from that point until newline as a comment.  Processing is
  // terminated upon received the expected number of values.
  FOREVER {

    // Get the next character.
    inchar = fgetc(fp);

    // If this line was a comment and we got to the newline, process the next
    // line as if it might not be a comment.
    if (commentLine && inchar == '\n') {
      commentLine = NO;
      continue;
    }

    // If this line is a comment line, ignore the rest of it.
    if (commentLine) continue;

    // If we got a digit, add it to the current chunk of digits.
    if (inchar >= '0' && inchar <= '9') {
      chunk[digit] = inchar;
      chunk[digit + 1] = '\0';  // Keep the string terminated.
      digit++;

      // If we got too many digits, throw out the rest of the file.
      if (digit >= 4) {
        fprintf(stderr, "Too many digits found in ip address from map file \"%s\"! Using internal default map.\n", filename);
        validData = NO;
        break;
      }

    // If we got a space, a dot, a comma, a newline, or an EOF, process the last
    // chunk and add it to our address.
    } else if (inchar == ' ' || inchar == '.' || inchar == '\n' || inchar == ',' || inchar == EOF) {

      // Some digits accumulated.  Evaluate.
      if (digit != 0) {

        value = atoi(chunk);
        if (value < 0 || value > 255) {
          fprintf(stderr, "IP out of range (%i) in ip map file \"%s\"! Using internal default map.\n", value, filename);
          validData = NO;
          break;
        }

        // Append it to the ip string.
        ind += snprintf(ind, end - ind, "%i", value);
        if (count % 4 < 3) {
          // Put dots after the first 3 values.
          ind += snprintf(ind, end - ind, ".");

        // If we got the fourth value, place it in the map.
        } else {

          // After the fourth, terminate the string.
          *ind = '\0';
          ind++;

          // Put it in the correct section of the map.
          switch(count / (TENSOR_ADDR_PER_PANEL * 4)) {
            case 0:
              tempMap1[(count / 4) % TENSOR_ADDR_PER_PANEL] = start;
              break;
            case 1:
              tempMap2[(count / 4) % TENSOR_ADDR_PER_PANEL] = start;
              break;
            case 2:
              tempMap3[(count / 4) % TENSOR_ADDR_PER_PANEL] = start;
              break;
            default:  // No need to point out this error - it doesn't occur.
              break;
          }

          // Put the start pointer on the next ipstring.
          start = ind;
        }

        // Set up for the next chunk.
        digit = 0;
        count++;

        // If we got enough chunks, stop processing.
        if (count >= 4 * TENSOR_ADDR_PER_PANEL * TENSOR_PANEL_COUNT) break;
      }

    // Any other character makes this a comment line.
    } else {
      commentLine = YES;
      continue;
    }

    if (inchar == EOF) break;  // End of file ends processing of the file.
  }

  // Close the file.
  fclose(fp);

  // Was there an error?
  if (!validData) {
    return;
  }

  // Did we get enough values?
  if (count != 4 * TENSOR_ADDR_PER_PANEL * TENSOR_PANEL_COUNT) {
    fprintf(stderr, "Expected %i full ip addresses, got %i (%i chunks != %i) in ip map file \"%s\". Using internal default map.\n",
      TENSOR_ADDR_PER_PANEL * TENSOR_PANEL_COUNT, count / 4, count,
      TENSOR_ADDR_PER_PANEL * TENSOR_PANEL_COUNT * 4, filename);
    return;
  }

  // TODO - More validation?

  // Print out the ip addresses we just read.
  fprintf(stdout, "Using ip address map \"%s\"\n", filename);
  fprintf(stdout, "%-16s   %-16s   %-16s\n", "Panel 1", "Panel 2", "Panel 3");
  for (i = 0; i < TENSOR_ADDR_PER_PANEL; i++) {
    fprintf(stdout, "%-16s   %-16s   %-16s\n", tempMap1[i], tempMap2[i], tempMap3[i]);
  }

  // Reassign the ip addresses.
  ipmap1 = tempMap1;
  ipmap2 = tempMap2;
  ipmap3 = tempMap3;
}

// Edit a value on the display by type.
inputMode_e EditValue(int set, int commandToEdit) {
  //~ int currentSet = set; // Global override.

  //~ printf("Edit value\n");
  if (displayCommand[commandToEdit].dataSource == PE_INVALID) return IM_NORMAL;
  switch(patternElements[displayCommand[commandToEdit].dataSource].type) {
    case ET_INT:
      snprintf(statusText, sizeof(statusText), "Editing \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_INT;
    case ET_FLOAT:
      snprintf(statusText, sizeof(statusText), "Editing \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_FLOAT;
    case ET_ENUM:
      snprintf(statusText, sizeof(statusText), "Select \"%s\". ESC to cancel.", displayCommand[commandToEdit].text);
      return IM_ENUM;
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
  box_t box, boxOld, drawBox;

  boxOld = GetCommandBox(*lastHover);

  for (i = 0; i < displayCommandCount; i++) {
    // box is the rectangle encompassing the command text.  We could
    // precompute these if timing were important.
    box = GetCommandBox(i);

    // Is it in the rectangle of command i?
    if (IsInsideBox(mouse, box)) {

      // Is it hovering over a command is wasn't hovering over before?
      if ((!IsSameBox(box, boxOld)) || (*lastHover == INVALID)) {

        // Yeah, so draw the new highlight.
        drawBox = box;
        drawBox.w = GetPixelofColumn(displayCommand[i].col + 1) - drawBox.x;
        DrawSBox(drawBox, DISPLAY_COLOR_TEXTSBG_HL);
        WriteCommand(i, DISPLAY_COLOR_TEXTS_HL, DISPLAY_COLOR_TEXTSBG_HL);

        // And if it came off a different command, remove that highlight.
        if (*lastHover != INVALID) {
          drawBox = boxOld;
          drawBox.w = GetPixelofColumn(displayCommand[*lastHover].col + 1) - drawBox.x;
          DrawSBox(drawBox, DISPLAY_COLOR_TEXTSBG);
          WriteCommand(*lastHover, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
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
    boxOld.w = GetPixelofColumn(displayCommand[*lastHover].col + 1) - boxOld.x;
    DrawSBox(boxOld, DISPLAY_COLOR_TEXTSBG);
    WriteCommand(*lastHover, DISPLAY_COLOR_TEXTS, DISPLAY_COLOR_TEXTSBG);
    *lastHover = INVALID;
  }

  return INVALID;
}

int OverBox(point_t mouse, int item, box_t ** targets, int *lastHover) {
  int i;
  box_t box, boxOld;
  const box_t invalidBox = {0, 0, 0, 0};
  int boxCount = enumerations[patternElements[displayCommand[item].dataSource].etype].size;
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
        snprintf(text, sizeof(text), "%s", enumerations[patternElements[displayCommand[item].dataSource].etype].texts[i]);
        CenterText(box, text, fgh, bgh);

        // And if it came off a different command, remove that highlight.
        if (*lastHover != INVALID) {
          DrawOutlineBox(boxOld, fg, bg);
          snprintf(text, sizeof(text), "%s", enumerations[patternElements[displayCommand[item].dataSource].etype].texts[*lastHover]);
          CenterText(boxOld, text, fg, bg);
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
    snprintf(text, sizeof(text), "%s", enumerations[patternElements[displayCommand[item].dataSource].etype].texts[*lastHover]);
    CenterText(boxOld, text, fg, bg);
    *lastHover = INVALID;
  }

  return INVALID;
}

// Update the values of the text on the display window.
// TODO: Shorten this function.  Lots of repetition.
#define STATUS_BAR_LENGTH 75
#define BUFFER_BAR_LENGTH 100
void UpdateInfoDisplay(int set) {
  int length;
  int i;
  static infoCache_t *infoCache = NULL;
  static int infoCount = 0;
  int thisInfo = 0;
  displayText_t tempTarget;
  bool_t initial = NO;
  static char oldStatus[sizeof(statusText)] = { '\0' };
  char statusTemp[STATUS_BAR_LENGTH + 10];
  char bufferTemp1[BUFFER_BAR_LENGTH + 10];
  char bufferTemp2[BUFFER_BAR_LENGTH + 10];
  static char *oldBuffer = NULL;
  static int oldBufferSize = 0;

  // Initialize the texture cache.  All this crap actually lowers my CPU usage
  // from 25% to 12%.  Half of CPU time was dedicated to generating text.
  if (!infoCount) {
    for (i = 0; i < displayCommandCount; i++) {
      if (displayCommand[i].dataSource != PE_INVALID) {
        switch(patternElements[displayCommand[i].dataSource].type) {
          case ET_BOOL:
            WriteBoolToTexture(&tempTarget, SBOOL(set, displayCommand[i].dataSource),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.b = SBOOL(set, displayCommand[i].dataSource);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_FLOAT:
            WriteFloatToTexture(&tempTarget, SFLOAT(set, displayCommand[i].dataSource),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.f = SFLOAT(set, displayCommand[i].dataSource);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_INT:
            WriteIntToTexture(&tempTarget, SINT(set, displayCommand[i].dataSource),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.i = SINT(set, displayCommand[i].dataSource);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_ENUM:
            WriteStringToTexture(&tempTarget,
              enumerations[patternElements[displayCommand[i].dataSource].etype].texts[SENUM(set, displayCommand[i].dataSource)],
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.e = SENUM(set, displayCommand[i].dataSource);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          default:
            // Lazy
            fprintf(stderr, "No data display driver for this type of information (%i).\n", patternElements[displayCommand[i].dataSource].type);
        }
      }
    }
  }

  // Draw the textures representing the values, or get new textures if the
  // values have changed.
  for (i = 0; i < displayCommandCount; i++) {
    if (displayCommand[i].dataSource != PE_INVALID) {
      switch(patternElements[displayCommand[i].dataSource].type) {
        case ET_BOOL:
          if (infoCache[thisInfo].cacheValue.b != SBOOL(set, displayCommand[i].dataSource)) {
            infoCache[thisInfo].cacheValue.b = SBOOL(set, displayCommand[i].dataSource);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            WriteBoolToTexture(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.b, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_FLOAT:
          if (infoCache[thisInfo].cacheValue.f != SFLOAT(set, displayCommand[i].dataSource)) {
            infoCache[thisInfo].cacheValue.f = SFLOAT(set, displayCommand[i].dataSource);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            WriteFloatToTexture(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.f, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_INT:
          if (infoCache[thisInfo].cacheValue.i != SINT(set, displayCommand[i].dataSource)) {
            infoCache[thisInfo].cacheValue.i = SINT(set, displayCommand[i].dataSource);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            WriteIntToTexture(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.i, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_ENUM:
          if (infoCache[thisInfo].cacheValue.e != SENUM(set, displayCommand[i].dataSource)) {
            infoCache[thisInfo].cacheValue.e = SENUM(set, displayCommand[i].dataSource);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            WriteStringToTexture(&(infoCache[thisInfo].infoText),
              enumerations[patternElements[displayCommand[i].dataSource].etype].texts[SENUM(set, displayCommand[i].dataSource)],
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        default:
          // Lazy
          fprintf(stderr, "No data display driver for this type of information (%i).\n", patternElements[displayCommand[i].dataSource].type);
      }
    }
  }

  // The ones that aren't automatic:

  // If we got here and thisInfo is equal to infoCount, we haven't initialized
  // the non-automatic values yet.
  if (thisInfo == infoCount) {
    initial = YES;
  }

  // Live Preview FPS
  if (initial || infoCache[thisInfo].cacheValue.i != previewFPSA) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = previewFPSA;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), previewFPSA, 21, 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Alternate Preview FPS
  if (initial || infoCache[thisInfo].cacheValue.i != previewFPSB) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = previewFPSB;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), previewFPSB, 21, 5, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Gui FPS
  if (initial || infoCache[thisInfo].cacheValue.i != infoFPS) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = infoFPS;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), infoFPS, 53, 5, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Pattern set cycling
  if (cyclePatternSets) {
    if (initial || infoCache[thisInfo].cacheValue.i != cycleFrameCount) {
      if (initial) {
        infoCount++;
        infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
      } else {
        SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
      }
      infoCache[thisInfo].cacheValue.i = cycleFrameCount;
      WriteIntToTexture(&(infoCache[thisInfo].infoText), cycleFrameCount, ROW_PA + 1, COL_PA + 1,  PARAMETER_WIDTH);
    }
  } else {
    if (initial || infoCache[thisInfo].cacheValue.b != cyclePatternSets) {
      if (initial) {
        infoCount++;
        infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
      } else {
        SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
      }
      infoCache[thisInfo].cacheValue.b = cyclePatternSets;
      WriteBoolToTexture(&(infoCache[thisInfo].infoText), cyclePatternSets, ROW_PA + 1, COL_PA + 1,  PARAMETER_WIDTH);
    }
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Alternate Set
  if (initial || infoCache[thisInfo].cacheValue.i != alternateSet) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = alternateSet;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), alternateSet, ROW_PA + 9, COL_PA + 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Current Set
  if (initial || infoCache[thisInfo].cacheValue.i != currentSet) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = currentSet;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), currentSet, ROW_PA + 8, COL_PA + 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;


  // Display set
  if (initial || infoCache[thisInfo].cacheValue.i != displaySet) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = displaySet;
    WriteStringToTexture(&(infoCache[thisInfo].infoText), operateText[displaySet],  ROW_PA + 10, COL_PA + 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Alternate blend amount
  if (initial || infoCache[thisInfo].cacheValue.f != alternateBlend) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.f = alternateBlend;
    WriteFloatToTexture(&(infoCache[thisInfo].infoText), alternateBlend, ROW_PA + 12, COL_PA + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Autoblend
  if (initial || infoCache[thisInfo].cacheValue.b != autoBlend) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.b = autoBlend;
    WriteBoolToTexture(&(infoCache[thisInfo].infoText), autoBlend, ROW_PA + 13, COL_PA + 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Alternate blend rate
  if (initial || infoCache[thisInfo].cacheValue.f != alternateBlendRate) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.f = alternateBlendRate;
    WriteFloatToTexture(&(infoCache[thisInfo].infoText), alternateBlendRate, ROW_PA + 14, COL_PA + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Show the status bar
  if (initial || (strcmp(oldStatus, statusText) != 0)) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    strncpy(oldStatus, statusText, sizeof(statusText));
    snprintf(statusTemp, sizeof(statusTemp) - 1, "%-*.*s", STATUS_BAR_LENGTH, STATUS_BAR_LENGTH, statusText);
    WriteLineToTexture(&(infoCache[thisInfo].infoText), statusTemp, 53, 1, cBlack, cRed);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Show the buffer character count.
  length = strlen(SSTRING(set, PE_TEXTBUFFER));
  if (initial || infoCache[thisInfo].cacheValue.i != length) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.i = length;
    WriteIntToTexture(&(infoCache[thisInfo].infoText), length, 51, 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Show the last 100 bytes of the text buffer.
  if (initial || (strcmp(oldBuffer, SSTRING(set, PE_TEXTBUFFER)) != 0)) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    if (oldBufferSize < length) {
      oldBuffer = realloc(oldBuffer, length + 1);
      oldBufferSize = length;
    }
    strncpy(oldBuffer, SSTRING(set, PE_TEXTBUFFER), length + 1);
    strncpy(bufferTemp1, SSTRING(set, PE_TEXTBUFFER) + (length > BUFFER_BAR_LENGTH ? length - BUFFER_BAR_LENGTH : 0), BUFFER_BAR_LENGTH + 1);
    snprintf(bufferTemp2, sizeof(bufferTemp2), " %-*s", BUFFER_BAR_LENGTH + 5, bufferTemp1);
    WriteLineToTexture(&(infoCache[thisInfo].infoText), bufferTemp2, 52, 0, DISPLAY_COLOR_TBUF, DISPLAY_COLOR_TBUFBG);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;
}

