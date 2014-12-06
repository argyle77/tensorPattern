// Tensor pattern generator
// Structural element types
// Joshua Krueger
// tensor@core9.org
// 2014_12_04

#ifndef ELEMENTS_H_
  #define ELEMENTS_H_

  #include <SDL.h>     // SDL_Keymod, SDL_Keycode
  #include "draw.h"    // point_t, box_t, color_t
  #include "useful.h"  // bool_t

  // Some initial Values
  #define INITIAL_MULTIPLIER 1.0
  #define INITIAL_DIFF_COEF 0.012
  #define INITIAL_FADE_INC -1
  #define INITIAL_RAND_MOD 100
  #define INITIAL_RAINBOW_INC 8
  #define INITIAL_EXPAND 1
  #define INITIAL_FLOAT_INC 0.1
  #define INITIAL_POSTROT_INC 0
  #define INITIAL_POSTROT_ANGLE 0
  #define INITIAL_PREROT_ANGLE 0
  #define INITIAL_DELAY 60
  #define INITIAL_TEXT "Frostbyte was an engineer. "
  #define INITIAL_DIR DIR_LEFT
  #define INITIAL_PREEXPAND 1.0
  #define INITIAL_FRAMECYCLECOUNT 1000
  #define INITIAL_IMAGE_ANGLE 0.0
  #define INITIAL_IMAGE_EXPAND 1.0
  #define INITIAL_IMAGE_INC 0.0
  #define INITIAL_IMAGE_XOFFSET 0.5
  #define INITIAL_IMAGE_YOFFSET 0.5
  #define INITIAL_IMAGE "Fbyte-01.jpg"
  #define TEXT_BUFFER_SIZE 2048

  // Enumerated types - In general, these are braces with INVALID and COUNT,
  // and prefixed to maintain a namespace separation.

  // Fade modes
  typedef enum fadeModes_e{
    FM_INVALID = -1,
    FM_LIMIT = 0, FM_MODULAR,
    FM_COUNT // Last.
  } fadeModes_e;

  // Color plane shift modes - should be in same order as dir_e.
  typedef enum shiftModes_e {
    SM_INVALID = -1,
    SM_UP, SM_LEFT, SM_DOWN, SM_RIGHT, SM_HOLD,
    SM_COUNT // Last.
  } shiftModes_e;

  // Scroller directions
  typedef enum dir_e {
    DIR_INVALID = -1,
    DIR_UP = 0, DIR_LEFT, DIR_DOWN, DIR_RIGHT,
    DIR_COUNT // Last.
  } dir_e;

  // Color cycle modes
  typedef enum colorCycleModes_e{
    CM_INVALID = -1,
    CM_NONE = 0, CM_RGB, CM_CMY, CM_SECONDARY, CM_TERTIARY, CM_GRAY, CM_RAINBOW,
    CM_RANDOM, CM_FGBGFADE, CM_TERTTOBLACK,
    CM_COUNT // Last.
  } colorCycleModes_e;

  // Text background modes
  typedef enum textMode_e {
    TS_INVALID = -1,
    TS_8ROW = 0, TS_9ROWSTAGGER, TS_10ROWBORDER, TS_FULLBG, TS_NOBG, TS_BLACK,
    TS_COUNT // Last.
  } textMode_e;

  // Crossbar seed types
  typedef enum crossBar_e {
    CB_INVALID = -1,
    CB_NONE = 0, CB_VERT, CB_HORZ, CB_BOTH,
    CB_COUNT // Last
  } crossBar_e;

  // Pattern set that the controls operate on
  typedef enum operateOn_e {
    OO_INVALID = -1,
    OO_CURRENT = 0, OO_ALTERNATE = 1,
    OO_COUNT // Last
  } operateOn_e;

  // List of enumerated types
  typedef enum enums_e {
    E_INVALID = -1,
    E_DIRECTIONS = 0, E_FADEMODES, E_SHIFTMODES, E_TEXTMODES, E_COLORCYCLES,
    E_COLORS, E_OPERATE, E_CROSSBAR,
    E_COUNT // Last.
  } enums_e;

  // Structure to connect enuermated types to their string labels and sizes.
  typedef struct enums_t {
    enums_e type;
    const char **texts;
    int size;
  } enums_t;

  // Pattern element data types

  // A pattern set consists of pattern elements (mode flags, parameters, text,
  // and even the frame buffer) that tell the engine how to behave while
  // constructing the next frame.  The data for each element starts as a void
  // pointer that is allocated at runtime.  Each element is allocated as an array,
  // allowing there to be multiple pattern sets, which the user can switch
  // between.

  // This patternElement enumeration should be maintained in the same order as
  // the patternSet global pattern data set array.  The program will run a check on
  // startup to make sure this occurs.
  typedef enum patternElement_e {
    PE_INVALID = -1,
    PE_CELLFUN = 0, PE_BOUNCER, PE_FADE, PE_DIFFUSE, PE_ROLLOVER, PE_SCROLL,
    PE_HBARS, PE_VBARS, PE_FGCOLORALL, PE_BGCOLORALL, PE_IMAGEALL, PE_RANDOMDOT,
    PE_FADEMODE, PE_POSTRZ, PE_PRERZ, PE_ALIAS, PE_MULTIPLY, PE_BARSEED, PE_NORED,
    PE_NOGREEN, PE_NOBLUE, PE_SHIFTRED, PE_SHIFTGREEN, PE_SHIFTBLUE, PE_SHIFTCYAN,
    PE_SHIFTYELLOW, PE_SHIFTMAGENTA, PE_POSTIMAGE, PE_FGE, PE_BGE, PE_FADEINC,
    PE_DIFFUSECOEF, PE_SCROLLDIR, PE_RANDOMDOTCOEF, PE_FGCYCLE, PE_BGCYCLE,
    PE_FGRAINBOW, PE_BGRAINBOW, PE_POSTEXP, PE_FLOATINC, PE_POSTRZANGLE,
    PE_POSTRZINC, PE_MULTIPLYBY, PE_DELAY, PE_CYCLESAVEFG, PE_CYCLESAVEBG,
    PE_FGC, PE_BGC, PE_CELLFUNCOUNT, PE_TEXTBUFFER, PE_TEXTMODE, PE_FONTFLIP,
    PE_FONTDIR, PE_TEXTOFFSET, PE_TEXTINDEX, PE_PIXELINDEX, PE_SCROLLDIRLAST,
    PE_TEXTSEED, PE_FRAMEBUFFER, PE_PRERZANGLE, PE_PRERZALIAS, PE_PRERZINC,
    PE_PRERZEXPAND, PE_FRAMECOUNT, PE_IMAGEANGLE, PE_IMAGEALIAS, PE_IMAGEINC,
    PE_IMAGEEXP, PE_IMAGEXOFFSET, PE_IMAGEYOFFSET, PE_IMAGENAME, PE_SNAIL,
    PE_SNAIL_POS, PE_FASTSNAIL, PE_FASTSNAILP, PE_CROSSBAR, PE_CBLIKELY,
    PE_MIRROR_V, PE_MIRROR_H, PE_SCROLLRANDEN, PE_SCROLLRANDOM, PE_SIDEPULSE,
    PE_INTENSITY,
    PE_COUNT // LAst
  } patternElement_e;

  // These are the supported data types for a pattern element.
  typedef enum elementType_e {
    ET_INVALID = -1,
    ET_BOOL = 0, ET_INT, ET_FLOAT, ET_COLOR, ET_ENUM, ET_STRING, ET_BUFFER,
    ET_COUNT // LAST
  } elementType_e;

  // This holds the pattern element initializers, min, and max values, which are
  // of a type dependent on the element.
  typedef union valueType_u {
    int i;           // Integer initializer
    int e;           // Enumerated intializer
    float f;         // Float
    color_t c;       // Color type - the big one.
    char *s;         // String pointer.
    bool_t b;        // Boolean flag.
    // No buffer initializer.
  } valueType_u;

  // patternElement type - Includes descriptors of the element as well as a
  // pointer (to be allocated) to the element's array of data.
  typedef struct patternElement_t {
    const patternElement_e index;  // Corresponds to patterElement_e
    const char *name;              // String name of the element.  Unique, no spaces!
    const elementType_e type;      // Element type.
    const valueType_u initial;     // Initial value.
    const valueType_u min;         // Minimum value.
    const valueType_u max;         // Maximum value.
    const int size;                // Size of array elements (string, buffer)
    const enums_e etype;           // Enumeration type for ET_ENUM.
  } patternElement_t;

  // Gui user Commands - Order unimportant.
  typedef enum command_e {
    COM_INVALID = -1, COM_NONE = 0,
    COM_CELL = 1, COM_BOUNCE_RST, COM_FADE, COM_DIFFUSE, COM_TEXT, COM_ROLL,
    COM_SCROLL, COM_HBAR, COM_VBAR, COM_FGALL, COM_BGALL, COM_RDOT, COM_CYCLESET,
    COM_FADEMODE, COM_TEXTRESET, COM_POSTROTATE, COM_MODEOFF, COM_PREROTATE,
    COM_AA, COM_MULTIPLY, COM_SIDEBAR, COM_NORED, COM_NOGREEN, COM_NOBLUE,
    COM_IMAGE, COM_STEP_INC, COM_STEP_DEC, COM_STEP_RST, COM_DIFFUSE_INC,
    COM_DIFFUSE_DEC, COM_DIFFUSE_RST, COM_EXPAND_INC, COM_EXPAND_DEC,
    COM_EXPAND_RST, COM_FADE_INC, COM_FADE_DEC, COM_FADE_RST, COM_PREROT_INC,
    COM_PREROT_DEC, COM_PREROT_RST, COM_POSTROT_INC, COM_POSTROT_DEC,
    COM_POSTROT_RST, COM_MULT_INC, COM_MULT_DEC, COM_MULT_RST, COM_POSTSPEED_INC,
    COM_POSTSPEED_DEC, COM_POSTSPEED_RST, COM_RANDOM_INC, COM_RANDOM_DEC,
    COM_RANDOM_RST, COM_CYCLE_MODE, COM_CYCLE_MODE_DOWN, COM_SCROLL_UP,
    COM_SCROLL_DOWN, COM_SCROLL_LEFT, COM_SCROLL_RIGHT, COM_SCROLL_DOWNC,
    COM_SCROLL_LEFTC, COM_SCROLL_RIGHTC, COM_SCROLL_UPC, COM_DELAY_INC,
    COM_DELAY_DEC, COM_DELAY_RST, COM_FG_INC, COM_FG_DEC, COM_BG_INC, COM_BG_DEC,
    COM_BACKSPACE, COM_DELETE, COM_TEXT_REVERSE, COM_TEXT_FLIP, COM_TEXTO_INC,
    COM_TEXTO_DEC, COM_TEXT_MODE_UP, COM_TEXT_MODE_DOWN, COM_RP_UP, COM_RP_DOWN,
    COM_GP_UP, COM_GP_DOWN, COM_BP_UP, COM_BP_DOWN, COM_CP_UP, COM_CP_DOWN,
    COM_YP_UP, COM_YP_DOWN, COM_MP_UP, COM_MP_DOWN, COM_EXIT, COM_ORIENTATION,
    COM_RETURN, COM_SCROLL_CYCLE_UP, COM_SCROLL_CYCLE_DOWN, COM_LOAD0, COM_LOAD1,
    COM_LOAD2, COM_LOAD3, COM_LOAD4, COM_LOAD5, COM_LOAD6, COM_LOAD7, COM_LOAD8,
    COM_LOAD9, COM_LOADSET0, COM_LOADSET1, COM_LOADSET2, COM_LOADSET3,
    COM_LOADSET4, COM_LOADSET5, COM_LOADSET6, COM_LOADSET7, COM_LOADSET8,
    COM_LOADSET9, COM_COPYSET0, COM_COPYSET1, COM_COPYSET2, COM_COPYSET3,
    COM_COPYSET4, COM_COPYSET5, COM_COPYSET6, COM_COPYSET7, COM_COPYSET8,
    COM_COPYSET9, COM_PREROTINC_RST, COM_PREROTINC_INC, COM_PREROTINC_DEC,
    COM_PREEXP_INC, COM_PREEXP_DEC, COM_PREEXP_RST, COM_PREALIAS, COM_RP_RST,
    COM_GP_RST, COM_BP_RST, COM_MP_RST, COM_YP_RST, COM_CP_RST, COM_FGRAINBOW_INC,
    COM_FGRAINBOW_DEC, COM_FGRAINBOW_RST, COM_BGRAINBOW_INC, COM_BGRAINBOW_DEC,
    COM_BGRAINBOW_RST, COM_FGCYCLE_RST, COM_FGCYCLE_UP, COM_FGCYCLE_DOWN,
    COM_BGCYCLE_RST, COM_BGCYCLE_UP, COM_BGCYCLE_DOWN, COM_IMAGEALL,
    COM_FCOUNT_RST, COM_FCOUNT_INC, COM_FCOUNT_DEC, COM_IMANGLE_RST,
    COM_IMANGLE_INC, COM_IMANGLE_DEC, COM_IMINC_RST, COM_IMINC_INC, COM_IMINC_DEC,
    COM_IMEXP_RST, COM_IMEXP_INC, COM_IMEXP_DEC, COM_IMALIAS, COM_IMXOFFSET_RST,
    COM_IMXOFFSET_INC, COM_IMXOFFSET_DEC, COM_IMYOFFSET_RST, COM_IMYOFFSET_INC,
    COM_IMYOFFSET_DEC, COM_ALTERNATE_INC, COM_ALTERNATE_DEC, COM_LIVE_INC,
    COM_LIVE_DEC, COM_OPERATE, COM_EXCHANGE, COM_BLEND_RST, COM_BLEND_INC,
    COM_BLEND_DEC, COM_BLENDINC_RST, COM_BLENDINC_INC, COM_BLENDINC_DEC,
    COM_BLENDSWITCH, COM_SNAIL, COM_SNAILFAST, COM_CB_RST, COM_CB_UP, COM_CB_DOWN,
    COM_CROSSB_RST, COM_CROSSB_UP, COM_CROSSB_DOWN, COM_FG_WHEEL_UP,
    COM_FG_WHEEL_DOWN, COM_BG_WHEEL_UP, COM_BG_WHEEL_DOWN, COM_MIRROR_V,
    COM_MIRROR_H, COM_SCROLL_RAND_RST, COM_SCROLL_RAND_UP, COM_SCROLL_RAND_DOWN,
    COM_SCROLL_RAND_EN, COM_BOUNCE_DOWN, COM_BOUNCE_UP, COM_SIDEPULSE,
    COM_INTENSITY_RST, COM_INTENSITY_UP, COM_INTENSITY_DOWN,
    COM_COUNT // Last
  } command_e;

  // Mouse command enumeration.
  typedef enum mouseCommand_e {
    MOUSE_INVALID = -1,
    MOUSE_CLICK = 0, MOUSE_WHEEL_UP, MOUSE_WHEEL_DOWN,
    MOUSE_COUNT // Last
  } mouseCommand_e;

  // Associate key presses and commands
  typedef struct keyCommand_t {
    SDL_Keymod mod;
    SDL_Keycode key;
    command_e command;
  } keyCommand_t;

  // Structure of a command includes its position on screen (line, col)
  // (line = -1 supresses display), command name to display, and which command
  // enumerations and keys are associated with each mouse action.
  typedef struct command_t {
    int line;
    int col;
    char *text;
    patternElement_e dataSource;  // Pattern set data to display after text.  PE_INVALID disables.
    keyCommand_t commands[MOUSE_COUNT];
  } command_t;

  // Display titles
  typedef struct displayText_t {
    int line;
    int col;
    char *text;
  } displayText_t;

  // Global constants
  extern const patternElement_t patternElements[];
  extern const int patternElementCount;

  extern const displayText_t displayText[];
  extern const int displayTextCount;
  extern const command_t otherCommands[];
  extern const int otherCommandsCount;
  extern const displayText_t labelText[];
  extern const int labelCount;
  extern const displayText_t headerText[];
  extern const int headerTextCount;
  extern const command_t displayCommand[];
  extern const int displayCommandCount;

  extern const colorName_t namedColors[];
  extern const enums_t enumerations[];
  extern const char *operateText[];

  // Global data
  extern void *patternData[];

  // Data element access macros for patternSet
  #define GLOBAL_PATTERN_ELEMENT_ARRAY patternData

  // Data element access macros - with patternElement array, element, and set selection.
  #define ABOOL(varName, varSet, varElement) ( ((unsigned char *)varName[varElement])[varSet] )
  #define AINT(varName, varSet, varElement) ( ((int *)varName[varElement])[varSet] )
  #define AFLOAT(varName, varSet, varElement) ( ((float *)varName[varElement])[varSet] )
  #define ACOLOR(varName, varSet, varElement) ( ((color_t *)varName[varElement])[varSet] )
  #define AENUM(varName, varSet, varElement) ( ((int *)varName[varElement])[varSet] )
  #define ASENUM(varName, varSet, varElement, varType) ( ((varType *)varName[varElement])[varSet] )
  #define ASTRING(varName, varSet, varElement) ( &((char *)varName[varElement])[varSet * patternElements[varElement].size] )
  #define ABUFFER(varName, varSet, varElement) ( &((unsigned char *)varName[varElement])[varSet * patternElements[varElement].size] )

  // Data element access macros - using the global patternElement array, with element and set selection.
  #define SBOOL(varSet, varElement) ( ABOOL(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SINT(varSet, varElement) ( AINT(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SFLOAT(varSet, varElement) ( AFLOAT(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SCOLOR(varSet, varElement) ( ACOLOR(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SENUM(varSet, varElement) ( AENUM(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SSENUM(varSet, varElement, varType) ( ASENUM(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement, varType) )
  #define SSTRING(varSet, varElement) ( ASTRING(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )
  #define SBUFFER(varSet, varElement) ( ABUFFER(GLOBAL_PATTERN_ELEMENT_ARRAY, varSet, varElement) )

  // Data element access macros - using the global patternElement array and the currentSet, with element selection.
  #define DBOOL(varElement) ( SBOOL(currentSet, varElement) )
  #define DINT(varElement) ( SINT(currentSet, varElement) )
  #define DFLOAT(varElement) ( SFLOAT(currentSet, varElement) )
  #define DCOLOR(varElement) ( SCOLOR(currentSet, varElement) )
  #define DENUM(varElement) ( SENUM(currentSet, varElement) )
  #define DSENUM(varElement, varType) ( SSENUM(currentSet, varElement, varType) )
  #define DSTRING(varElement) ( SSTRING(currentSet, varElement) )
  #define DBUFFER(varElement) ( SBUFFER(currentSet, varElement) )

  // And maybe even a function.
  bool_t VerifyStructuralIntegrity(void);
  bool_t AllocatePatternData(int setCount);

#endif /* ifndef ELEMENTS_H_ */
