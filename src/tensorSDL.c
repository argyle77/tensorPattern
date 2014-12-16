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
#include <limits.h>  // INT_MIN
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
SDL_Rect liveBox, altBox;
point_t tensorPixelMap [TENSOR_WIDTH * TENSOR_HEIGHT];
bool_t useDefaultPixelMap = YES;
const char **ipmap1;
const char **ipmap2;
const char **ipmap3;
color_t fgOutput[A_COUNT][PATTERN_SET_COUNT];
color_t bgOutput[A_COUNT][PATTERN_SET_COUNT];

// Prototypes
void DrawNewFrame(int set, unsigned char primary);
void ProcessModes(int set);
Uint32 TriggerFrameDrawA(Uint32 interval, void *param);
Uint32 TriggerFrameDrawB(Uint32 interval, void *param);
Uint32 TriggerFrameCount(Uint32 interval, void *param);
Uint32 TriggerGUIUpdate(Uint32 interval, void *param);
void SetPixelByPlaneA(int x, int y, color_t color, unsigned char plane, unsigned char *buffer);
void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
void SetPixel(int x, int y, color_t color, unsigned char *fb);
color_t GetPixel(int x, int y, unsigned char *buffer);
void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer);
void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane);
void WriteSlice(int set);
void CellFun(int set);
void DrawSideBar(int set);
void Diffuse(float diffusionCoeff, bool_t isToroid, unsigned char *buffer);
void HorizontalBars(color_t fg, color_t bg, unsigned char *buffer);
void VerticalBars(color_t fg, color_t bg, unsigned char *buffer);
void SavePatternSet(char key, int set, bool_t overWrite, const char *backupName);
void LoadPatternSet(char key, int set, bool_t backup);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
color_t ColorCycle(colorCycleModes_e cycleMode, int *cycleSaver, int cycleInc, color_t a, color_t b);
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
void DrawImage(SDL_Surface *image, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst, float alpha);
void CopyFbAlpha(unsigned char *dest, unsigned char *src, float alpha);
bool_t HandleCommand(int set, command_e command, int selection);
bool_t HandleKey(int set, SDL_Keycode key, SDL_Keymod mod);
bool_t HandleConfirmation(SDL_Keycode key, bool_t *selected);
inputMode_e EditValue(int set, int commandToEdit);
textEntry_e HandleTextEntry(SDL_Keycode key, SDL_Keymod mod, char * textEntry, int maxTextSize);
bool_t HandleEnumSelect(SDL_Keycode key, int set, int item, int *selected);
void CopyPatternSet(int dst, int src);
void CopyBuffer(int dst, int set);
void SwitchToSet(int set);

void BlendAlternate(unsigned char *fba, unsigned char *fbb);
void CenterText(SDL_Rect box, char * text, color_t fg, color_t bg);
void SnailSeed(int set);
void FastSnailSeed(int set);
void CrossBars(int set);
point_t GetDisplayPixel(point_t mouse, SDL_Rect box);
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
int OverBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover);
SDL_Rect GetBoxofCommand(int command);
void DrawSidePulse(int set);
int HandleColorSelection(point_t mouse, int thisHover, bool_t resetEvent, int set);
void UpdateInfoDisplay(int set);
int OverColorBox(int set, point_t mouse, int item, SDL_Rect ** targets, int *lastHover);
bool_t HandleColorSelect(SDL_Keycode key, int set, int item, int *selected);
color_t ColorCyclePalette(namedPalette_t palette, int *position, int wavelength);
color_t ColorCyclePaletteSmooth(namedPalette_t palette, int *position, int wavelength);
void ResetOneShots(int set);

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
  SDL_Rect *targets = NULL;
  operateOn_e displaySetOld = OO_INVALID;
  patternElement_e element;
  command_e command;
  color_e wheelColorF = 0, wheelColorB = 0;

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
    LoadPatternSet(i + '0', i, NO);
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
        SetPixelA(tpixel.x, tpixel.y, fgOutput[A_LIGHTPEN][currentSet], SBUFFER(currentSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        SetPixelA(tpixel.x, tpixel.y, bgOutput[A_LIGHTPEN][currentSet], SBUFFER(currentSet, PE_FRAMEBUFFER));
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
        SetPixelA(tpixel.x, tpixel.y, fgOutput[A_LIGHTPEN][alternateSet], SBUFFER(alternateSet, PE_FRAMEBUFFER));
      } else if (rightMouseDown) {
        SetPixelA(tpixel.x, tpixel.y, bgOutput[A_LIGHTPEN][alternateSet], SBUFFER(alternateSet, PE_FRAMEBUFFER));
      }
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
    SavePatternSet('0' + i, i, YES, "closebak");
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
    if (!SBOOL(set, PE_PAUSE)) ProcessModes(set);

    UpdateDisplays(set, YES, YES, global_intensity_limit);
  } else {
    if (currentSet != alternateSet) {
      if (!SBOOL(set, PE_PAUSE)) ProcessModes(set);
    }
    UpdateDisplays(set, NO, NO, global_intensity_limit);
  }

}

// Time to make a mess of the array.
void ProcessModes(int set) {
  int currentSet = set; // Overrides global used in D* macros.
  int i;

  // Scroll direction randomizer
  if (DBOOL(PE_SCROLLRANDEN)) {
    if (!(rand() % DINT(PE_SCROLLRANDOM))) {
      DENUM(PE_SCROLLDIR) = rand() % DIR_COUNT;
    }
  }

  // Colors
  for (i = 0; i < A_COUNT; i++) {
    if (seedPalette[i].fgCycleMode > PE_INVALID && DENUM(seedPalette[i].fgCycleMode)) {
      fgOutput[i][set] = ColorCycle(DENUM(seedPalette[i].fgCycleMode),
        &DINT(seedPalette[i].fgCyclePos), DINT(seedPalette[i].fgCycleRate),
        DCOLOR(seedPalette[i].fgColorA), DCOLOR(seedPalette[i].fgColorB));
    } else if (seedPalette[i].fgColorA > PE_INVALID) {
      fgOutput[i][set] = DCOLOR(seedPalette[i].fgColorA);
    }

    if (seedPalette[i].bgCycleMode > PE_INVALID && DENUM(seedPalette[i].bgCycleMode)) {
      bgOutput[i][set] = ColorCycle(DENUM(seedPalette[i].bgCycleMode),
        &DINT(seedPalette[i].bgCyclePos), DINT(seedPalette[i].bgCycleRate),
        DCOLOR(seedPalette[i].bgColorA), DCOLOR(seedPalette[i].bgColorB));
    } else if (seedPalette[i].bgColorA > PE_INVALID) {
      bgOutput[i][set] = DCOLOR(seedPalette[i].bgColorA);
    }

    // Apply the alphas.
    if (seedPalette[i].fgAlpha > PE_INVALID)
      fgOutput[i][set].a = (unsigned char) (DFLOAT(seedPalette[i].fgAlpha) * 255.0);
    if (seedPalette[i].bgAlpha > PE_INVALID)
      bgOutput[i][set].a = (unsigned char) (DFLOAT(seedPalette[i].bgAlpha) * 255.0);
  }

  // Slap an image down on the display.
  if (DENUM(PE_POSTIMAGE)) {
    DrawImage(imageSeed[currentSet], DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
      DFLOAT(PE_IMAGEEXP), DENUM(PE_IMAGEALIAS) != OS_NO, DBUFFER(PE_FRAMEBUFFER), DFLOAT(PE_IMAGEALPHA));
    DFLOAT(PE_IMAGEANGLE) += DFLOAT(PE_IMAGEINC);
  }

  // Scroller.
  if (DENUM(PE_SCROLL)) {
    Scroll(set, DENUM(PE_SCROLLDIR), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_ALL);
  }

  // Planar scrollers.  The enables are built into the enumerations.  Scroll()
  // will ignore them if they are SM_HOLD.
  Scroll(set, DENUM(PE_SHIFTRED), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_RED);
  Scroll(set, DENUM(PE_SHIFTGREEN), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_GREEN);
  Scroll(set, DENUM(PE_SHIFTBLUE), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_BLUE);
  Scroll(set, DENUM(PE_SHIFTCYAN), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_CYAN);
  Scroll(set, DENUM(PE_SHIFTMAGENTA), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_MAGENTA);
  Scroll(set, DENUM(PE_SHIFTYELLOW), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER), PLANE_YELLOW);

  // Draw a solid bar up the side we are scrolling from.
  if (DENUM(PE_BARSEED)) {
    DrawSideBar(currentSet);
  }

  // Draw a pulse up the side we are scrolling from.
  if (DENUM(PE_SIDEPULSE)) {
    DrawSidePulse(currentSet);
  }

  // Write a column of text on the side opposite of the scroll direction.
  if (DBOOL(PE_TEXTSEED)) {
    // Scroll provided by scroller if its on.
    WriteSlice(currentSet);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (DENUM(PE_CELLFUN)) {
    // Give each pixel a color value.
    CellFun(currentSet);
  }

  // Bouncy bouncy (ick).
  if (DINT(PE_BOUNCER)) {
    ScrollCycle(currentSet);
  }

  // Bam!  Draw some horizontal bars.
  if (DENUM(PE_HBARS)) {
    HorizontalBars(fgOutput[A_HBARS][set], bgOutput[A_HBARS][set], DBUFFER(PE_FRAMEBUFFER));
  }

  // Bam! Vertical bars.
  if (DENUM(PE_VBARS)) {
    VerticalBars(fgOutput[A_VBARS][set], bgOutput[A_VBARS][set], DBUFFER(PE_FRAMEBUFFER));
  }

  // Crossbar seeds
  if (DENUM(PE_CROSSBAR)) {
    CrossBars(currentSet);
  }

  // Random dots.  Most useful seed ever.
  if (DENUM(PE_RDOT)) {
    RandomDots(fgOutput[A_RDOT][set], DINT(PE_RDOTCOEF), DBUFFER(PE_FRAMEBUFFER));
  }

  // Snail seed.
  if (DENUM(PE_SNAIL)) {
    SnailSeed(currentSet);
  } else {
    DINT(PE_SNAIL_POS) = 0;
  }

  // Fast snail seed.
  if (DENUM(PE_FSNAIL)) {
    FastSnailSeed(currentSet);
  } else {
    DINT(PE_FSNAILP) = 0;
  }

  // Fader
  if (DENUM(PE_FADE)) {
    FadeAll(DINT(PE_FADEINC), DENUM(PE_FADEMODE), DBUFFER(PE_FRAMEBUFFER));
  }

  // Averager
  if (DENUM(PE_DIFFUSE)) {
    Diffuse(DFLOAT(PE_DIFFUSECOEF), DBOOL(PE_ROLLOVER), DBUFFER(PE_FRAMEBUFFER));
  }

  // Multiplier
  if (DENUM(PE_MULTIPLY)) {
    Multiply(DFLOAT(PE_MULTIPLYBY), DBUFFER(PE_FRAMEBUFFER));
  }

  // Rotozoomer
  if (DENUM(PE_PRERZ)) {
    Rotate(DFLOAT(PE_PRERZANGLE), DFLOAT(PE_PRERZEXPAND), DENUM(PE_PRERZALIAS) != OS_NO, DBUFFER(PE_FRAMEBUFFER), DBUFFER(PE_FRAMEBUFFER), NO);
    DFLOAT(PE_PRERZANGLE) += DFLOAT(PE_PRERZINC);
  }

  // Zero the red.
  if (DENUM(PE_NORED)) {
    ClearRed(currentSet);
  }

  // Zero the blue.
  if (DENUM(PE_NOBLUE)) {
    ClearBlue(currentSet);
  }

  // Zero the green.
  if (DENUM(PE_NOGREEN)) {
    ClearGreen(currentSet);
  }

  // Mirrors
  if (DENUM(PE_MIRROR_V)) {
    VerticalMirror(DBUFFER(PE_FRAMEBUFFER));
  }
  if (DENUM(PE_MIRROR_H)) {
    HorizontalMirror(DBUFFER(PE_FRAMEBUFFER));
  }

  // Post rotation increment.
  if (DBOOL(PE_POSTRZ)) {
    DFLOAT(PE_POSTRZANGLE) += DFLOAT(PE_POSTRZINC);
  }

  // Seed the entire array with the foreground color.
  if (DENUM(PE_CAA)) {
    ColorAll(fgOutput[A_COLORALLA][set], DBUFFER(PE_FRAMEBUFFER));
  }

  // Clear screen.
  if (DENUM(PE_CAB)) {
    ColorAll(fgOutput[A_COLORALLB][set], DBUFFER(PE_FRAMEBUFFER));
  }

  // Reset the one shots.
  ResetOneShots(set);
}

void ResetOneShots(int set) {
  int currentSet = set; // Override
  int i;

  for (i = 0; i < patternElementCount; i++) {
    if (patternElements[i].etype == E_ONESHOT) {
      // The switch is to handle the special cases.
      switch(i) {
        case PE_PRERZALIAS: case PE_IMAGEALIAS: case PE_SNAIL: case PE_FSNAIL:
        case PE_BARSEED: case PE_SIDEPULSE:
          break;
        case PE_PRERZ:
          if ((DENUM(i) == OS_YES) && (DENUM(PE_PRERZALIAS) == OS_ONESHOT))
            DENUM(PE_PRERZALIAS) = OS_NO;
          if (DENUM(i) == OS_ONESHOT) DENUM(i) = OS_NO;
          break;
        case PE_POSTIMAGE:
          if ((DENUM(i) == OS_YES) && (DENUM(PE_IMAGEALIAS) == OS_ONESHOT))
            DENUM(PE_IMAGEALIAS) = OS_NO;
          if (DENUM(i) == OS_ONESHOT) DENUM(i) = OS_NO;
          break;
        default:
          if (DENUM(i) == OS_ONESHOT) DENUM(i) = OS_NO;
          break;
      }
    }
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

void SnailSeed(int set) {
  int currentSet = set; // Override global currentSet for D*.
  int i;
  int x = 0, y = 0;
  int tp = 0;
  int dir = 0;
  int xh,xl,yh,yl;
  color_t color = fgOutput[A_SMALLSNAIL][set];
  xh = TENSOR_WIDTH;
  xl = 0;
  yh = TENSOR_HEIGHT;
  yl = 0;
  for (i = 0; i < TENSOR_HEIGHT * TENSOR_WIDTH; i++) {
    if (DINT(PE_SNAIL_POS) == tp) {
      SetPixelA(x, y, color, DBUFFER(PE_FRAMEBUFFER));
      DINT(PE_SNAIL_POS)++;
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH) {
      if (DENUM(PE_SNAIL) == OS_ONESHOT) DENUM(PE_SNAIL) = OS_NO;
      DINT(PE_SNAIL_POS) = 0;
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

void FastSnailSeed(int set) {
  int currentSet = set; // Overrides global currentSet for D*.
  int i;
  int x = -1, y = 0;
  int tp = 0;
  int dir = 0;
  int xh,xl,yh,yl;
  color_t color = fgOutput[A_LARGESNAIL][set];
  xh = TENSOR_WIDTH - 1;
  xl = 0;
  yh = TENSOR_HEIGHT - 1;
  yl = 1;
  for (i = 0; i < TENSOR_HEIGHT * TENSOR_WIDTH; i++) {
    if (DINT(PE_FSNAILP) == tp) {
      //~ printf("tp: %i, (%i, %i)\n", tp, x, y);
      SetPixelA(x, y, color, DBUFFER(PE_FRAMEBUFFER));
      SetPixelA(x, y + 1, color, DBUFFER(PE_FRAMEBUFFER));
      SetPixelA(x + 1, y + 1, color, DBUFFER(PE_FRAMEBUFFER));
      SetPixelA(x + 1, y, color, DBUFFER(PE_FRAMEBUFFER));
      DINT(PE_FSNAILP)++;
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH / 4 + 5) {
      if (DENUM(PE_FSNAIL) == OS_ONESHOT) DENUM(PE_FSNAIL) = OS_NO;
      DINT(PE_FSNAILP) = 0;
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
  color_t color = fgOutput[A_CROSSBARS][set];

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
      SetPixelA(cbars[set][i].x, cbars[set][i].y, color, DBUFFER(PE_FRAMEBUFFER));
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

    case COM_BROAD_FLIP: enableTensor = !enableTensor; break;
    case COM_BROAD_RST: enableTensor = YES; break;
    case COM_BLEND_RST: alternateBlend = 0; break;
    case COM_BLEND_INC:
      alternateBlend += SFLOAT(set, PE_FLOATINC);
      if (alternateBlend > 1.0) alternateBlend = 1.0;
      break;
    case COM_BLEND_DEC:
      alternateBlend -= SFLOAT(set, PE_FLOATINC);
      if (alternateBlend < 0.0) alternateBlend = 0.0;
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
    case COM_BLENDSWITCH_RST: autoBlend = NO; break;
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
      DrawPreviewBorder(PREVIEW_LIVE_POS_X, PREVIEW_LIVE_POS_Y,  tensorWidth, tensorHeight,displaySet == OO_CURRENT);
      DrawPreviewBorder(PREVIEW_ALT_POS_X, PREVIEW_ALT_POS_Y, tensorWidth, tensorHeight, displaySet == OO_ALTERNATE);
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
    case COM_TEXTO_RST:
      if ((SENUM(set, PE_SCROLLDIR) == DIR_LEFT) || (SENUM(set, PE_SCROLLDIR) == DIR_RIGHT)) {
        SINT(set, PE_TEXTOFFSET) = tensorHeight / 2 - FONT_HEIGHT / 2;
      } else {
        SINT(set, PE_TEXTOFFSET) = tensorWidth / 2 - FONT_HEIGHT / 2;
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
      SetPixelA(x, y, color, fb);
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
      } else if (fadeMode == FM_LIMIT) {
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
      } else if (fadeMode == FM_NONZEROL) {
        if (colorTemp.r != 0) {
          r = colorTemp.r;
          r+=inc;
          if ( r < 0 ) r = 0;
          if ( r > 255 ) r = 255;
          colorTemp.r = r;
        }
        if (colorTemp.g != 0) {
          g = colorTemp.g;
          g+=inc;
          if ( g < 0 ) g = 0;
          if ( g > 255 ) g = 255;
          colorTemp.g = g;
        }
        if (colorTemp.b != 0) {
          b = colorTemp.b;
          b+=inc;
          if ( b < 0 ) b = 0;
          if ( b > 255 ) b = 255;
          colorTemp.b = b;
        }
      } else if (fadeMode == FM_NONZEROM) {
        if (colorTemp.r != 0) {
          colorTemp.r += inc;
          if (colorTemp.r == 0) {
            colorTemp.r = colorTemp.r + (inc < 0 ? -1 : 1);
          }
        }
        if (colorTemp.g != 0) {
          colorTemp.g += inc;
          if (colorTemp.g == 0) {
            colorTemp.g = colorTemp.g + (inc < 0 ? -1 : 1);
          }
        }
        if (colorTemp.b != 0) {
          colorTemp.b += inc;
          if (colorTemp.b == 0) {
            colorTemp.b = colorTemp.b + (inc < 0 ? -1 : 1);
          }
        }
      }
      SetPixel(x, y, colorTemp, buffer);
    }
  }
}




// Set a single pixel a particular color.
void SetPixel(int x, int y, color_t color, unsigned char *buffer) {
  if ((x >= tensorWidth) || (y >= tensorHeight) || (x < 0) || (y < 0)) return;

  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Set a single pixel a particular color by color plane.
// Rediculous.
void SetPixelByPlaneA(int x, int y, color_t newColor, unsigned char plane, unsigned char *buffer) {
  color_t curColor;
  float a;

  if ((x >= tensorWidth) || (y >= tensorHeight) || (x < 0) || (y < 0)) return;

  // Do the alpha blending, if called for.
  if (newColor.a != 255) {

    // Get the current pixel color.
    curColor = GetPixel(x, y, buffer);

    // Normalize the alpha value. Normal = (in - min) / (max - min)
    a = (float) newColor.a / 255.0;

    // Calculate the blended outputs => value = (1 - a) Value0 + a Value1
    newColor.r = (unsigned char) ((a * newColor.r) + ((1.0 - a) * curColor.r));
    newColor.g = (unsigned char) ((a * newColor.g) + ((1.0 - a) * curColor.g));
    newColor.b = (unsigned char) ((a * newColor.b) + ((1.0 - a) * curColor.b));
  }

  if (plane & PLANE_RED)
    buffer[(y * tensorWidth * 3) + (x * 3) + 0] = newColor.r;
  if (plane & PLANE_GREEN)
    buffer[(y * tensorWidth * 3) + (x * 3) + 1] = newColor.g;
  if (plane & PLANE_BLUE)
    buffer[(y * tensorWidth * 3) + (x * 3) + 2] = newColor.b;
}

// Set a single pixel a particular color with alpha blending.
// Is this ever used?  Does it work?
void SetPixelA(int x, int y, color_t color, unsigned char *buffer) {
  color_t curColor;
  float a;

  if (x >= tensorWidth || y >= tensorHeight || x < 0 || y < 0) return;

  // Get the current color.
  curColor = GetPixel(x, y, buffer);

  // Normalize the alpha value. Normal = (in - min) / (max - min)
  a = (float) color.a / 255.0;

  // Calculate the blended outputs = value = (1 - a) Value0 + a Value1
  color.r = (unsigned char) ((a * color.r) + ((1.0 - a) * curColor.r));
  color.g = (unsigned char) ((a * color.g) + ((1.0 - a) * curColor.g));
  color.b = (unsigned char) ((a * color.b) + ((1.0 - a) * curColor.b));

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
  colorTemp.a = 255;
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
  int x, y, i, j;
  color_t rollSave[TENSOR_WIDTH];  // TENSOR_WIDTH is larger than TENSOR_HEIGHT
  unsigned char alpha;
  color_t temp;

  // Ignore SM_HOLD values that have been pre decremented.
  if ((shiftModes_e)direction == SM_HOLD) return;

  // Get the scroller alpha
  alpha = (unsigned char) (DFLOAT(PE_SCROLLALPHA) * 255.0);

  // If the topology of the surface is toroidal, we save aside the edge so that
  // it can be wrapped.
  if (toroidal) {
    switch(direction) {
      case DIR_UP:
        for(i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, 0, fb);
        break;

      case DIR_DOWN:
        if (DENUM(PE_MIRROR_H)) {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, (tensorHeight - 1)/2, fb);
        } else {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, tensorHeight - 1, fb);
        }
        break;

      case DIR_RIGHT:
        if (DENUM(PE_MIRROR_V)) {
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

    // Add in the scroller alpha
    for (j = 0 ; j < i; j++) {
      rollSave[j].a = alpha;
    }
  }

  // Make the shift.
  switch(direction) {
    case DIR_UP:
      if (DENUM(PE_MIRROR_H)) {
        for (y = 0; y < (tensorHeight - 1) / 2; y++) {
          for (x = 0; x < tensorWidth; x++) {
            temp = GetPixel(x, y + 1, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
          }
        }
      } else {
        for (y = 0; y < (tensorHeight - 1); y++) {
          for (x = 0; x < tensorWidth; x++) {
            temp = GetPixel(x, y + 1, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
          }
        }
      }
      break;

    case DIR_DOWN:
      for (y = (tensorHeight - 1); y > 0; y--) {
        for (x = 0; x < tensorWidth; x++) {
          temp = GetPixel(x, y - 1, fb);
          temp.a = alpha;
          SetPixelByPlaneA(x, y, temp, plane, fb);
        }
      }
      break;

    case DIR_LEFT:
      if (DENUM(PE_MIRROR_V)) {
        for (y = 0; y < tensorHeight; y++) {
          for (x = 0; x < (tensorWidth - 1) / 2; x++) {
            temp = GetPixel(x + 1, y, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
          }
        }
      } else {
        for (y = 0; y < tensorHeight; y++) {
          for (x = 0; x < (tensorWidth - 1); x++) {
            temp = GetPixel(x + 1, y, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
          }
        }
      }
      break;

    case DIR_RIGHT:
      for (y = 0; y < tensorHeight; y++) {
        if (DENUM(PE_MIRROR_V)) {
          for (x = (tensorWidth - 1) / 2; x > 0; x--) {
            temp = GetPixel(x - 1, y, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
          }
        } else {
          for (x = (tensorWidth - 1); x > 0; x--) {
            temp = GetPixel(x - 1, y, fb);
            temp.a = alpha;
            SetPixelByPlaneA(x, y, temp, plane, fb);
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
        if (DENUM(PE_MIRROR_H)) {
          for(i = 0; i < tensorWidth; i++)
            SetPixelByPlaneA(i, (tensorHeight - 1)/2, rollSave[i], plane, fb);
        } else {
          for(i = 0; i < tensorWidth; i++)
            SetPixelByPlaneA(i, tensorHeight - 1, rollSave[i], plane, fb);
        }
        break;

      case DIR_DOWN:
        for (i = 0; i < tensorWidth; i++)
          SetPixelByPlaneA(i, 0, rollSave[i],plane, fb);
        break;

      case DIR_RIGHT:
        for (i = 0; i < tensorHeight; i++)
          SetPixelByPlaneA(0, i, rollSave[i],plane, fb);
        break;

      case DIR_LEFT:
        if (DENUM(PE_MIRROR_V)) {
          for (i = 0; i < tensorHeight; i++)
            SetPixelByPlaneA((tensorWidth - 1)/2, i, rollSave[i],plane, fb);
        } else {
          for (i = 0; i < tensorHeight; i++)
            SetPixelByPlaneA(tensorWidth - 1, i, rollSave[i],plane, fb);
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
  color_t fgColor = fgOutput[A_TEXT][set];
  color_t bgColor = bgOutput[A_TEXT][set];

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
      if (DENUM(PE_MIRROR_V)) sliceColOrRow = (tensorWidth - 1) / 2;
      break;

    case DIR_UP:
      horizontal = NO;
      sliceColOrRow = tensorHeight - 1;
      if (DENUM(PE_MIRROR_H)) sliceColOrRow = (tensorHeight - 1) / 2;
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
      case DIR_LEFT: textDirection = DIR_RIGHT; break;
      case DIR_RIGHT: textDirection = DIR_LEFT; break;
      case DIR_DOWN: textDirection = DIR_UP; break;
      case DIR_UP: textDirection = DIR_DOWN; break;
      default: break;
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
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex = sliceIndex % tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, DBUFFER(PE_FRAMEBUFFER));
      }
      useRand = textStaggerFlag[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = DINT(PE_TEXTOFFSET) - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = tensorHeight - 1;
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, DBUFFER(PE_FRAMEBUFFER));
      } else {
        if (sliceIndex < 0) sliceIndex = tensorWidth - 1;
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, DBUFFER(PE_FRAMEBUFFER));
      }
      sliceIndex = DINT(PE_TEXTOFFSET) + 8;
      if (horizontal) {
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, DBUFFER(PE_FRAMEBUFFER));
      } else {
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, DBUFFER(PE_FRAMEBUFFER));
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < tensorHeight; i++) {
          SetPixelA(sliceColOrRow, i, bgColor, DBUFFER(PE_FRAMEBUFFER));
        }
      } else {
        for (i = 0; i < tensorWidth; i++) {
          SetPixelA(i, sliceColOrRow, bgColor, DBUFFER(PE_FRAMEBUFFER));
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
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? fgColor : bgColor, DBUFFER(PE_FRAMEBUFFER));
    } else {
      sliceIndex = sliceIndex % tensorWidth;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? fgColor : bgColor, DBUFFER(PE_FRAMEBUFFER));
    }
  }

  return;
}

// Draw bars across the display.
void HorizontalBars(color_t fg, color_t bg, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j++) {
      if (j % 2) SetPixelA(i, j, bg, buffer);
      else SetPixelA(i, j, fg, buffer);
    }
  }
}


void VerticalBars(color_t fg, color_t bg, unsigned char *buffer) {
  int i, j;
  for (i = 0; i < tensorWidth; i++) {
    for (j = 0; j < tensorHeight; j++) {
      if (i % 2) SetPixelA(i,j, bg, buffer);
      else SetPixelA(i,j, fg, buffer);
    }
  }
}


void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer) {
  int x,y;

  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      if (!(rand() % rFreq)) {
        SetPixelA(x, y, color, buffer);
      }
    }
  }
}

color_t ColorCyclePalette(namedPalette_t palette, int *position, int wavelength) {
  if (wavelength > 0) {
    *position = (*position + 1) % (palette.size * wavelength);
  } else if (wavelength < 0) {
    *position = (*position - 1);
    if (*position < 0) *position = (palette.size * abs(wavelength)) - 1;
  } else {
    // For 0, we'll do random color cycling.
    *position = rand() % palette.size;
    wavelength = 1;
  }
  return namedColors[palette.palette[*position / abs(wavelength)]].color;
}

// Smooth fade palette colors.
color_t ColorCyclePaletteSmooth(namedPalette_t palette, int *position, int wavelength) {
  int startColor, inTransition;
  int endColor;
  color_t c, d, rc;

  if (wavelength > 0) {
    *position = (*position + 1) % (palette.size * wavelength);
  } else if (wavelength < 0) {
    *position = (*position - 1);
    wavelength = abs(wavelength);
    if (*position < 0) *position = (palette.size * wavelength) - 1;
  } else {
    wavelength = 1;
  }

  startColor = *position / wavelength;
  endColor = (startColor + 1) % palette.size;
  inTransition = *position % wavelength;
  c = namedColors[palette.palette[startColor]].color;
  d = namedColors[palette.palette[endColor]].color;
  rc.r = c.r - (((float)(c.r - d.r) / wavelength) * inTransition);
  rc.g = c.g - (((float)(c.g - d.g) / wavelength) * inTransition);
  rc.b = c.b - (((float)(c.b - d.b) / wavelength) * inTransition);

  return rc;
}

// Enacts the cycling of colors based on palette settings.
color_t ColorCycle(colorCycleModes_e cycleMode, int *cycleSaver, int cycleSteps, color_t a, color_t b) {
  color_t colorTemp = cBlack;
  int transition, inTransition;
  int toColor = 0;
  static color_t lastColor = CD_BLACK;
  color_t c;

  // If the mode changed, we should make sure our position in the cycle limits
  // to the mode.
  switch(cycleMode) {
    case CM_RGB:
      colorTemp = ColorCyclePalette(paletteRGB, cycleSaver, cycleSteps);
      break;

    case CM_RGBSM:
      colorTemp = ColorCyclePaletteSmooth(paletteRGB, cycleSaver, cycleSteps);
      break;

    case CM_CMY:
      colorTemp = ColorCyclePalette(paletteCMY, cycleSaver, cycleSteps);
      break;

    case CM_CMYSM:
      colorTemp = ColorCyclePaletteSmooth(paletteCMY, cycleSaver, cycleSteps);
      break;

    case CM_RWB:
      colorTemp = ColorCyclePalette(paletteRWB, cycleSaver, cycleSteps);
      break;

    case CM_RWBSM:
      colorTemp = ColorCyclePaletteSmooth(paletteRWB, cycleSaver, cycleSteps);
      break;

    case CM_SECONDARY:
      colorTemp = ColorCyclePalette(paletteSec, cycleSaver, cycleSteps);
      break;

    case CM_TERTIARY:
      colorTemp = ColorCyclePalette(paletteTer, cycleSaver, cycleSteps);
      break;

    case CM_GRAY:
      colorTemp = ColorCyclePalette(paletteGry, cycleSaver, cycleSteps);
      break;

    case CM_GRAYSM:
      colorTemp = ColorCyclePaletteSmooth(paletteGry, cycleSaver, cycleSteps);
      break;

    case CM_SYMGRAY:
      colorTemp = ColorCyclePalette(paletteSymGry, cycleSaver, cycleSteps);
      break;

    case CM_SYMGRAYSM:
      colorTemp = ColorCyclePaletteSmooth(paletteSymGry, cycleSaver, cycleSteps);
      break;

    case CM_RAINBOW:
      // Cycle steps says how man cycles it takes to transition to the next
      // color.  In the case of rainbow, we consider 6 colors along the hue
      // circle, so a cycle step of 1 will cycle through 6 colors, whereas a
      // cycle step of 2 will hit the ones in between those 6 as well, etc.
      // Cycle step of 0 will freeze our position in this case.  Negative will
      // cycle backwards through these colors.
      if (cycleSteps > 0) *cycleSaver = (*cycleSaver + 1) % (6 * cycleSteps);
      else if (cycleSteps < 0) {
        *cycleSaver = (*cycleSaver - 1);
        cycleSteps = abs(cycleSteps);
        if (*cycleSaver < 0) *cycleSaver = (6 * cycleSteps) - 1;
      } else {
        cycleSteps = 1;
      }

      transition = *cycleSaver / cycleSteps;
      inTransition = *cycleSaver % cycleSteps;
      switch(transition) {
        case 0: // R -> Y
          colorTemp.r = 255;
          colorTemp.g = (256.0 / cycleSteps) * inTransition;
          colorTemp.b = 0;
          break;

        case 1: // Y -> G
          colorTemp.r = 255 - (256.0 / cycleSteps) * inTransition;
          colorTemp.g = 255;
          colorTemp.b = 0;
          break;

        case 2: // G -> C
          colorTemp.r = 0;
          colorTemp.g = 255;
          colorTemp.b = (256.0 / cycleSteps) * inTransition;
          break;

        case 3: // C -> B
          colorTemp.r = 0;
          colorTemp.g = 255 - (256.0 / cycleSteps) * inTransition;
          colorTemp.b = 255;
          break;

        case 4: // B -> M
          colorTemp.r = (256.0 / cycleSteps) * inTransition;
          colorTemp.g = 0;
          colorTemp.b = 255;
          break;

        case 5: // M -> R
          colorTemp.r = 255;
          colorTemp.g = 0;
          colorTemp.b = 255 - (256.0 / cycleSteps) * inTransition;
          break;

        default:
          break;
      }
      break;

    case CM_RANDOM:
      // Sign of the step rate is meaningless here.
      cycleSteps = abs(cycleSteps);
      if (cycleSteps == 0) {
        *cycleSaver = 1;  // Stay on last color.  Doesn't work across program restart.
      } else {
        *cycleSaver = (*cycleSaver + 1) % cycleSteps;
      }
      if (!*cycleSaver) {
        colorTemp.r = rand() % 256;
        colorTemp.g = rand() % 256;
        colorTemp.b = rand() % 256;
        lastColor = colorTemp;
      } else {
        colorTemp = lastColor;
      }
      break;

    case CM_ABFADE:
      // Fade between colors A and B.
      if (cycleSteps > 0) *cycleSaver = (*cycleSaver + 1) % (2 * cycleSteps);
      else if (cycleSteps < 0) {
        *cycleSaver = (*cycleSaver - 1);
        cycleSteps = abs(cycleSteps);
        if (*cycleSaver < 0) *cycleSaver = (2 * cycleSteps) - 1;
      } else {
        cycleSteps = 1;
      }

      transition = *cycleSaver / cycleSteps;
      inTransition = *cycleSaver % cycleSteps;
      switch(transition) {
        case 0: // A - B
          colorTemp.r = a.r - (((float)(a.r - b.r) / cycleSteps) * inTransition);
          colorTemp.g = a.g - (((float)(a.g - b.g) / cycleSteps) * inTransition);
          colorTemp.b = a.b - (((float)(a.b - b.b) / cycleSteps) * inTransition);
          break;

        case 1: // B - A
          colorTemp.r = b.r - (((float)(b.r - a.r) / cycleSteps) * inTransition);
          colorTemp.g = b.g - (((float)(b.g - a.g) / cycleSteps) * inTransition);
          colorTemp.b = b.b - (((float)(b.b - a.b) / cycleSteps) * inTransition);
        default:
          break;
      }
      break;

    case CM_TERTOA:
     // Fade between colors A and B.
      if (cycleSteps > 0) *cycleSaver = (*cycleSaver + 1) % (2 * paletteTer.size * cycleSteps);
      else if (cycleSteps < 0) {
        *cycleSaver = (*cycleSaver - 1);
        cycleSteps = abs(cycleSteps);
        if (*cycleSaver < 0) *cycleSaver = (2 * paletteTer.size * cycleSteps) - 1;
      } else {
        cycleSteps = 1;
      }

      transition = *cycleSaver / cycleSteps;
      inTransition = *cycleSaver % cycleSteps;
      toColor = transition / 2;
      c = namedColors[paletteTer.palette[toColor]].color;
      switch(transition % 2) {
        case 0: // A - B
          colorTemp.r = a.r - (((float)(a.r - c.r) / cycleSteps) * inTransition);
          colorTemp.g = a.g - (((float)(a.g - c.g) / cycleSteps) * inTransition);
          colorTemp.b = a.b - (((float)(a.b - c.b) / cycleSteps) * inTransition);
          break;

        case 1: // B - A
          colorTemp.r = c.r - (((float)(c.r - a.r) / cycleSteps) * inTransition);
          colorTemp.g = c.g - (((float)(c.g - a.g) / cycleSteps) * inTransition);
          colorTemp.b = c.b - (((float)(c.b - a.b) / cycleSteps) * inTransition);
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
void DrawImage(SDL_Surface *image, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst, float alpha) {
  SDL_Surface *s1 = NULL;
  SDL_Surface *s2 = NULL;
  SDL_Rect offset;
  int tempx, tempy;
  int cx, cy;
  unsigned char fba[TENSOR_BYTES];

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

  // Copy to a frame buffer.
  SurfaceToFB(fba, s1);
  SDL_FreeSurface(s1);

  // Copy to the destination frame buffer using alpha blending.
  CopyFbAlpha(fb_dst, fba, alpha);
}

void CopyFbAlpha(unsigned char *dest, unsigned char *src, float alpha) {
  int x, y;
  unsigned char a;
  color_t temp;
  a = (unsigned char) (alpha * 255.0);

  for (x = 0 ; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      temp = GetPixel(x, y, src);
      temp.a = a;
      SetPixelA(x, y, temp, dest);
    }
  }
}



// Or not.
void CellFun(int set) {
  int currentSet = set; // Override global for D*
  int x, y;
  color_t pixelColor, oldColor;
  color_t fgColor = fgOutput[A_CELL][set];
  color_t bgColor = bgOutput[A_CELL][set];
  DINT(PE_CELLFUNCOUNT)++;

  for(x = 0 ; x < tensorWidth ; x++) {
    for(y = 0 ; y < tensorHeight ; y++) {
      oldColor = GetPixel(x, y, DBUFFER(PE_FRAMEBUFFER));
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2) * bgColor.g;
      pixelColor.g = oldColor.g + pixelColor.r + fgColor.b;
      pixelColor.b = DINT(PE_CELLFUNCOUNT) + fgColor.r;
      pixelColor.a = (unsigned char) (DFLOAT(PE_CF_ALPHA) * 255.0);
      SetPixelA(x, y, pixelColor,  DBUFFER(PE_FRAMEBUFFER));
    }
  }
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
  color_t color = fgOutput[A_SIDEBAR][set];
  static int sideBarCount[PATTERN_SET_COUNT];

  // Let's deal with one-shots first.
  sideBarCount[set]++;
  if (DENUM(PE_BARSEED) == OS_ONESHOT) {
    switch(DSENUM(PE_SCROLLDIR, dir_e)) {
      case DIR_LEFT: case DIR_RIGHT:
        if (sideBarCount[set] >= tensorWidth) {
          DENUM(PE_BARSEED) = OS_NO;
          sideBarCount[set] = 0;
        }
        break;
      case DIR_UP: case DIR_DOWN:
        if (sideBarCount[set] >= tensorHeight) {
          DENUM(PE_BARSEED) = OS_NO;
          sideBarCount[set] = 0;
        }
        break;
      default: break;
    }
  }

  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      for (i = 0; i < tensorHeight; i++) {
        if (DENUM(PE_MIRROR_V)) {
          SetPixelA((tensorWidth - 1) / 2, i, color, DBUFFER(PE_FRAMEBUFFER));
        } else {
          SetPixelA(tensorWidth - 1, i, color, DBUFFER(PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_RIGHT:
      for (i = 0; i < tensorHeight; i++) {
        SetPixelA(0, i, color, DBUFFER(PE_FRAMEBUFFER));
      }
      break;

    case DIR_UP:
      for (i = 0; i < tensorWidth; i++) {
        if (DENUM(PE_MIRROR_H)) {
          SetPixelA(i, (tensorHeight - 1) / 2, color, DBUFFER(PE_FRAMEBUFFER));
        } else {
          SetPixelA(i, tensorHeight - 1, color, DBUFFER(PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_DOWN:
      for (i = 0; i < tensorWidth; i++) {
        SetPixelA(i, 0, color, DBUFFER(PE_FRAMEBUFFER));
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
  static int oneShotCount[PATTERN_SET_COUNT];
  static int initial = YES;
  int i;
  color_t color = fgOutput[A_SIDEPULSE][set];

  if (initial) {
    initial = NO;
    for (i = 0 ; i < PATTERN_SET_COUNT ; i++) {
      pos[i] = 0;
      dir[i] = 1;
    }
  }

  // Deal with one-shots
  // Let's deal with one-shots first.
  oneShotCount[set]++;
  if (DENUM(PE_SIDEPULSE) == OS_ONESHOT) {
    switch(DSENUM(PE_SCROLLDIR, dir_e)) {
      case DIR_LEFT: case DIR_RIGHT:
        if (oneShotCount[set] > tensorHeight) {
          DENUM(PE_SIDEPULSE) = OS_NO;
          oneShotCount[set] = 0;
          pos[set] = 0;
          dir[set] = 1;
          return;
        }
        break;
      case DIR_UP: case DIR_DOWN:
        if (oneShotCount[set] > tensorWidth) {
          DENUM(PE_SIDEPULSE) = OS_NO;
          oneShotCount[set] = 0;
          pos[set] = 0;
          dir[set] = 1;
          return;
        }
        break;
      default: break;
    }
  }

  switch (DSENUM(PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      if (DENUM(PE_MIRROR_V)) {
        SetPixelA((tensorWidth - 1) / 2, pos[set], color, DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DENUM(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
      } else {
        SetPixelA(tensorWidth - 1, pos[set], color, DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DENUM(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorHeight - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_RIGHT:
        SetPixelA(0, pos[set], color, DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DENUM(PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
        if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_UP:
      if (DENUM(PE_MIRROR_H)) {
        SetPixelA(pos[set], (tensorHeight - 1) / 2, color, DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DENUM(PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      } else {
        SetPixelA(pos[set], tensorHeight - 1, color, DBUFFER(PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (DENUM(PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_DOWN:
      SetPixelA(pos[set], 0, color, DBUFFER(PE_FRAMEBUFFER));
      pos[set] += dir[set];
      if (DENUM(PE_MIRROR_V)) {
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

// Allows for remapping of pixel output and ip addresses without recompiling.
void LoadTensorMaps(void) {
  LoadIPMap();
  LoadPixelMap();
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
        //~ drawBox.w = GetPixelofColumn(displayCommand[i].col + 1) - drawBox.x;
        DrawSBox(drawBox, DISPLAY_COLOR_TEXTSBG_HL);
        CreateTextureCommand(&commandTexture, i, DISPLAY_COLOR_TEXTS_HL, DISPLAY_COLOR_TEXTSBG_HL);
        DrawDisplayTexture(commandTexture);
        SDL_DestroyTexture(commandTexture.texture);

        // And if it came off a different command, remove that highlight.
        if (*lastHover != INVALID) {
          drawBox = boxOld;
          //~ drawBox.w = GetPixelofColumn(displayCommand[*lastHover].col + 1) - drawBox.x;
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
    //~ boxOld.w = GetPixelofColumn(displayCommand[*lastHover].col + 1) - boxOld.x;
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

// Update the values of the text on the display window.
// TODO: Shorten this function.  Lots of repetition.
#define STATUS_BAR_LENGTH 75
#define BUFFER_BAR_LENGTH 94
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
  patternElement_e element;
  patternElement_e element_o;
  SDL_Rect tempBox;
  color_t color;

  // Initialize the texture cache.  All this crap actually lowers my CPU usage
  // from 25% to 12%.  Half of CPU time was dedicated to regenerating the text
  // textures 40x per second.
  if (!infoCount) {
    for (i = 0; i < displayCommandCount; i++) {
      element_o = element = displayCommand[i].dataSource;

      // Adjust for selected elements
      if (element < PE_INVALID) element = GetSelectElement(set, element);
      if (element > PE_INVALID) {
        switch(patternElements[element].type) {
          case ET_BOOL:
            CreateTextureBoolean(&tempTarget, SBOOL(set, element),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.b = SBOOL(set, element);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_FLOAT:
            CreateTextureFloat(&tempTarget, SFLOAT(set, element),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.f = SFLOAT(set, element);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_INT:
            CreateTextureInt(&tempTarget, SINT(set, element),
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.i = SINT(set, element);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_ENUM:
            CreateTextureString(&tempTarget,
              enumerations[patternElements[element].etype].texts[SENUM(set, element)],
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
            if (tempTarget.texture != NULL) {
              infoCount++;
              infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
              infoCache[infoCount - 1].cacheValue.e = SENUM(set, element);
              infoCache[infoCount - 1].infoText = tempTarget;
            }
            break;

          case ET_COLOR:
            infoCount++;
            break;

          default:
            // Lazy
            //~ fprintf(stderr, "No data display driver for element %i, type %i.\n", element, patternElements[element].type);
            break;
        }
      } else if (element_o < PE_INVALID) {
        // This is a place holder because these things come into and go out of
        // existance sometimes.  Ugh.
        CreateTextureInt(&tempTarget, INT_MAX - 1,
          displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
        if (tempTarget.texture != NULL) {
          infoCount++;
          infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
          infoCache[infoCount - 1].cacheValue.i = INT_MAX - 1;
          infoCache[infoCount - 1].infoText = tempTarget;
        }
      }
    }
  }

  // Draw the textures representing the values, or get new textures if the
  // values have changed.
  for (i = 0; i < displayCommandCount; i++) {
    element_o = element = displayCommand[i].dataSource;

    // Adjust for color selected elements
    if (element < PE_INVALID) element = GetSelectElement(set, element);
    if (element > PE_INVALID) {
      switch(patternElements[element].type) {
        case ET_BOOL:
          if (infoCache[thisInfo].cacheValue.b != SBOOL(set, element)) {
            infoCache[thisInfo].cacheValue.b = SBOOL(set, element);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            CreateTextureBoolean(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.b, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_FLOAT:
          if (infoCache[thisInfo].cacheValue.f != SFLOAT(set, element)) {
            infoCache[thisInfo].cacheValue.f = SFLOAT(set, element);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            CreateTextureFloat(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.f, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_INT:
          if (infoCache[thisInfo].cacheValue.i != SINT(set, element)) {
            infoCache[thisInfo].cacheValue.i = SINT(set, element);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            CreateTextureInt(&(infoCache[thisInfo].infoText),
              infoCache[thisInfo].cacheValue.i, displayCommand[i].line,
              displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_ENUM:
          if (infoCache[thisInfo].cacheValue.e != SENUM(set, element)) {
            infoCache[thisInfo].cacheValue.e = SENUM(set, element);
            SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
            CreateTextureString(&(infoCache[thisInfo].infoText),
              enumerations[patternElements[element].etype].texts[SENUM(set, element)],
              displayCommand[i].line, displayCommand[i].col + 1, PARAMETER_WIDTH);
          }
          DrawDisplayTexture(infoCache[thisInfo].infoText);
          thisInfo++;
          break;

        case ET_COLOR:
          tempBox = GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 1, 0);
          tempBox.w = PARAMETER_WIDTH * CHAR_W;
          DrawOutlineBox(tempBox, DISPLAY_COLOR_PARMS, SCOLOR(set, element));
          thisInfo++;
          break;

        default:
          // Lazy
          fprintf(stderr, "No data display driver for element %i, type %i.\n", element, patternElements[element].type);
      }
    } else if (element_o < PE_INVALID) {
      // Draw the disabled element boxes
      tempBox = GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 1, 0);
      tempBox.w = PARAMETER_WIDTH * CHAR_W;
      DrawXBox(tempBox, DISPLAY_COLOR_PARMS, DISPLAY_COLOR_PARMSBG);
      thisInfo++;
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), previewFPSA, FPS_ROW, 1, PARAMETER_WIDTH);
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), previewFPSB, FPS_ROW, 5, PARAMETER_WIDTH);
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), infoFPS, ROW_COUNT - 1, 5, PARAMETER_WIDTH);
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
      CreateTextureInt(&(infoCache[thisInfo].infoText), cycleFrameCount, ROW_PA + 1, COL_PA + 1,  PARAMETER_WIDTH);
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
      CreateTextureBoolean(&(infoCache[thisInfo].infoText), cyclePatternSets, ROW_PA + 1, COL_PA + 1,  PARAMETER_WIDTH);
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), alternateSet, ROW_PA + 9, COL_PA + 1, PARAMETER_WIDTH);
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), currentSet, ROW_PA + 8, COL_PA + 1, PARAMETER_WIDTH);
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
    CreateTextureString(&(infoCache[thisInfo].infoText), operateText[displaySet],  ROW_PA + 10, COL_PA + 1, PARAMETER_WIDTH);
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
    CreateTextureFloat(&(infoCache[thisInfo].infoText), alternateBlend, ROW_PA + 12, COL_PA + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
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
    CreateTextureBoolean(&(infoCache[thisInfo].infoText), autoBlend, ROW_PA + 13, COL_PA + 1, PARAMETER_WIDTH);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Tensor broadcast
  if (initial || infoCache[thisInfo].cacheValue.b != enableTensor) {
    if (initial) {
      infoCount++;
      infoCache = realloc(infoCache, infoCount * sizeof(infoCache_t));
    } else {
      SDL_DestroyTexture(infoCache[thisInfo].infoText.texture);
    }
    infoCache[thisInfo].cacheValue.b = enableTensor;
    CreateTextureBoolean(&(infoCache[thisInfo].infoText), enableTensor, ROW_A + 1, COL_A + 1, PARAMETER_WIDTH);
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
    CreateTextureFloat(&(infoCache[thisInfo].infoText), alternateBlendRate, ROW_PA + 14, COL_PA + 1, PARAMETER_WIDTH, PARAMETER_WIDTH / 2);
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
    CreateTextureLine(&(infoCache[thisInfo].infoText), statusTemp, ROW_COUNT - 1, 1, cBlack, cRed);
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
    CreateTextureInt(&(infoCache[thisInfo].infoText), length, ROW_COUNT - 3, 1, PARAMETER_WIDTH);
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
    snprintf(bufferTemp2, sizeof(bufferTemp2), " %-*s", BUFFER_BAR_LENGTH + 3, bufferTemp1);
    CreateTextureLine(&(infoCache[thisInfo].infoText), bufferTemp2, ROW_COUNT - 2, 0, DISPLAY_COLOR_TBUF, DISPLAY_COLOR_TBUFBG);
  }
  DrawDisplayTexture(infoCache[thisInfo].infoText);
  thisInfo++;

  // Show the color selectors
  for (i = 0; i < displayCommandCount; i++) {
    if (displayCommand[i].colorSource != A_INVALID) {
      color = fgOutput[displayCommand[i].colorSource][set];
      color.a = 255;
      DrawOutlineBox(GetBoxofLine(displayCommand[i].line, displayCommand[i].col + 7, 0),
        DISPLAY_COLOR_PARMS, color);
      if (seedPalette[displayCommand[i].colorSource].bgColorA != PE_INVALID) {
        // A background if there is one.
        color = bgOutput[displayCommand[i].colorSource][set];
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

