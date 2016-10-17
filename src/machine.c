// machine.h
// Joshua Krueger
// 2016_10_12
// Contains the infrastructure for generating the frames, and what is, for now,
// the only pattern of transforms.

// Includes
#include <stdlib.h> // malloc, NULL
#include <stdio.h> // fprintf
#include <SDL.h>
#include "useful.h"
#include "machine.h"
#include "tensor.h"
#include "layout.h"
#include "guidraw.h"
#include "elements.h"
#include "transforms_old.h"

// Internal globals
int bufferSize = 0;
point_t livePreviewLoc, altPreviewLoc;
int tw, th;
int *liveSetp, *altSetp;
int *liveRefresh, *altRefresh;
SDL_Rect liveBox, altBox;
unsigned char *fbLiveBcast = NULL;
unsigned char *fbAltBcast = NULL;
unsigned char *fbAltBlend = NULL;

// Internal prototypes
void ResetOneShots(int set);
void UpdatePreview(bool_t isLive);

// Setup the machine and its memory.
bool_t InitMachine(int fbSize, point_t liveLoc, point_t altLoc, int *liveSetPtr, int *altSetPtr) {
  bufferSize = fbSize;
  
  fbLiveBcast = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbLiveBcast) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    return FAIL;
  }
  fbAltBcast = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbAltBcast) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    return FAIL;
  }
  fbAltBlend = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbAltBlend) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    return FAIL;
  }
  
  livePreviewLoc = liveLoc;
  altPreviewLoc = altLoc;
  liveSetp = liveSetPtr;
  altSetp = altSetPtr;  
  SetLiveSet(*liveSetp);
  SetAltSet(*altSetp);
  return PASS;
}

void SetMachineWH(int w, int h) {
  tw = w;
  th = h;
}

// Getters
point_t GetLiveLoc(void) { return livePreviewLoc; }
point_t GetAltLoc(void ) { return altPreviewLoc; }
int GetLiveSet(void) { return *liveSetp; }
int GetAltSet(void) { return *altSetp; }
int *GetLiveRefresh(void) { return liveRefresh; }
int *GetAltRefresh(void) { return altRefresh; }


// Setters
void SetLiveSet(int set) {
  // TODO: Check set before using it like this.
  *liveSetp = set; 
  liveRefresh = &SINT(set, PE_DELAY);
}
void SetAltSet(int set) {
  // TODO: Check set before using it like this.
  *altSetp = set;
  altRefresh = &SINT(set, PE_DELAY);
}

// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
// Send out the frame buffer to tensor
void UpdateTensor(void) {
  SendTensor(fbLiveBcast);
}

// Send out the frame the display window.
// Send frame buffer to preview area.  The preview area is a square
// encompassing the actual preview, so we should center the buffer within it.
void UpdatePreview(bool_t isLive) {
  point_t xyOffset;
  unsigned char *buffer;
  
  // Send live broadbast to the left (live) preview window
  if (isLive) {
    xyOffset = livePreviewLoc;
    buffer = fbLiveBcast;
    
  // Otherwise, send alt broadcast to the right (alt) preview window
  } else {
    xyOffset = altPreviewLoc;
    // If live and alternate happen to be the same set, ProcessNextStep doesn't
    // give us an alt buffer to avoid double time execution with both alt and
    // live timers contuing to fire, but in this case, live buffer and alt 
    // buffer would be identical anyway, so grab the one that exists.
    buffer = (*altSetp != *liveSetp) ? fbAltBcast : fbLiveBcast;
  }
  
  DrawPreview(xyOffset, buffer);
}

// The thing that happens at every frame.
// isPrimary = YES if we are working with the Live Preview.
// isPrimary = NO if we are working with the Alt Preview.
void ProcessNextStep(int set, bool_t isPrimary) {

  // TODO Fix how ProcessModes and PostProcessModes treats the 
  //      liveSet == altStep skip.  Buffers still need to be filled.
  // If its the primary set, assume we process.  If secondary, only process if not also primary.
  if (isPrimary || (*liveSetp != *altSetp)) {
    if (!SBOOL(set, PE_PAUSE)) {
      ProcessModes(set);
      PostProcessModes(set, isPrimary);
    }
  }
}

// Time to make a mess of the array.
// TODO: Time to take these transformations / seeds out of this fixed order
// and make them into sets of arbitrary length and order of transformations

// A "set" as below is a parameter set that includes switches and parameters
// for all the various transformations of one of the 10 sets in memory.  Not
// only does the set contain the switches and parameters, but also the frame
// buffer data itself.
// In the future, sets will contain a list of pointers to functions to execute
// (taking place of the old switches), along with their parameters.  The frame
// buffers will go elsewhere...
void ProcessModes(int set) {
  int i;

  // There are two inputs to this function.  The first is the frame buffer on
  // which to operate.  The second is the transform set to perform.

  // Scroll direction randomizer
  if (SBOOL(set, PE_SCROLLRANDEN)) {
    if (!(rand() % SINT(set, PE_SCROLLRANDOM))) {
      SENUM(set, PE_SCROLLDIR) = rand() % DIR_COUNT;
    }
  }

  // Update the colors
  for (i = 0; i < A_COUNT; i++) {
    if (seedPalette[i].fgCycleMode > PE_INVALID && SENUM(set, seedPalette[i].fgCycleMode)) {
      SetFGColor(i, set, ColorCycle(SENUM(set, seedPalette[i].fgCycleMode),
        &SINT(set, seedPalette[i].fgCyclePos), SINT(set, seedPalette[i].fgCycleRate),
        SCOLOR(set, seedPalette[i].fgColorA), SCOLOR(set, seedPalette[i].fgColorB)));
    } else if (seedPalette[i].fgColorA > PE_INVALID) {
      SetFGColor(i, set, SCOLOR(set, seedPalette[i].fgColorA));
    }

    if (seedPalette[i].bgCycleMode > PE_INVALID && SENUM(set, seedPalette[i].bgCycleMode)) {
      SetBGColor(i, set, ColorCycle(SENUM(set, seedPalette[i].bgCycleMode),
        &SINT(set, seedPalette[i].bgCyclePos), SINT(set, seedPalette[i].bgCycleRate),
        SCOLOR(set, seedPalette[i].bgColorA), SCOLOR(set, seedPalette[i].bgColorB)));
    } else if (seedPalette[i].bgColorA > PE_INVALID) {
      SetBGColor(i, set, SCOLOR(set, seedPalette[i].bgColorA));
    }

    // Apply the alphas.
    if (seedPalette[i].fgAlpha > PE_INVALID)
      SetFGColorA(i, set, (unsigned char) (SFLOAT(set, seedPalette[i].fgAlpha) * 255.0));
    if (seedPalette[i].bgAlpha > PE_INVALID)
      SetBGColorA(i, set, (unsigned char) (SFLOAT(set, seedPalette[i].bgAlpha) * 255.0));
  }

  // Slap an image down on the display.
  if (SENUM(set, PE_POSTIMAGE)) {
    DrawImage(set, SFLOAT(set, PE_IMAGEANGLE), SFLOAT(set, PE_IMAGEXOFFSET), SFLOAT(set, PE_IMAGEYOFFSET),
      SFLOAT(set, PE_IMAGEEXP), SENUM(set, PE_IMAGEALIAS) != OS_NO, SBUFFER(set, PE_FRAMEBUFFER), SFLOAT(set, PE_IMAGEALPHA));
    SFLOAT(set, PE_IMAGEANGLE) += SFLOAT(set, PE_IMAGEINC);
  }

  // Scroller.
  if (SENUM(set, PE_SCROLL)) {
    Scroll(set, SENUM(set, PE_SCROLLDIR), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_ALL);
  }

  // Planar scrollers.  The enables are built into the enumerations.  Scroll()
  // will ignore them if they are SM_HOLD.
  Scroll(set, SENUM(set, PE_SHIFTRED), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_RED);
  Scroll(set, SENUM(set, PE_SHIFTGREEN), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_GREEN);
  Scroll(set, SENUM(set, PE_SHIFTBLUE), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_BLUE);
  Scroll(set, SENUM(set, PE_SHIFTCYAN), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_CYAN);
  Scroll(set, SENUM(set, PE_SHIFTMAGENTA), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_MAGENTA);
  Scroll(set, SENUM(set, PE_SHIFTYELLOW), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_YELLOW);

  // Draw a solid bar up the side we are scrolling from.
  if (SENUM(set, PE_BARSEED)) {
    DrawSideBar(set);
  }

  // Draw a pulse up the side we are scrolling from.
  if (SENUM(set, PE_SIDEPULSE)) {
    DrawSidePulse(set);
  }

  // Write a column of text on the side opposite of the scroll direction.
  if (SBOOL(set, PE_TEXTSEED)) {
    // Scroll provided by scroller if its on.
    WriteSlice(set);
  }

  // Cellular automata manips?  Not actually cellular auto.  Never finished this.
  if (SENUM(set, PE_CELLFUN)) {
    // Give each pixel a color value.
    CellFun(set);
  }

  // Bouncy bouncy (ick).
  if (SINT(set, PE_BOUNCER)) {
    ScrollCycle(set);
  }

  // Bam!  Draw some horizontal bars.
  if (SENUM(set, PE_HBARS)) {
    HorizontalBars(GetFGColor(A_HBARS, set), GetBGColor(A_HBARS, set), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Bam! Vertical bars.
  if (SENUM(set, PE_VBARS)) {
    VerticalBars(GetFGColor(A_VBARS, set), GetBGColor(A_VBARS, set), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Crossbar seeds
  if (SENUM(set, PE_CROSSBAR)) {
    CrossBars(set);
  }

  // Random dots.  Most useful seed ever.
  if (SENUM(set, PE_RDOT)) {
    RandomDots(GetFGColor(A_RDOT, set), SINT(set, PE_RDOTCOEF), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Snail seed.
  if (SENUM(set, PE_SNAIL)) {
    SnailSeed(set);
  } else {
    SINT(set, PE_SNAIL_POS) = 0;
  }

  // Fast snail seed.
  if (SENUM(set, PE_FSNAIL)) {
    FastSnailSeed(set);
  } else {
    SINT(set, PE_FSNAILP) = 0;
  }

  // Fader
  if (SENUM(set, PE_FADE)) {
    FadeAll(SINT(set, PE_FADEINC), SENUM(set, PE_FADEMODE), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Averager
  if (SENUM(set, PE_DIFFUSE)) {
    Diffuse(SFLOAT(set, PE_DIFFUSECOEF), SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Multiplier
  if (SENUM(set, PE_MULTIPLY)) {
    Multiply(SFLOAT(set, PE_MULTIPLYBY), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Rotozoomer
  if (SENUM(set, PE_PRERZ)) {
    Rotate(SFLOAT(set, PE_PRERZANGLE), SFLOAT(set, PE_PRERZEXPAND), SENUM(set, PE_PRERZALIAS) != OS_NO, SBUFFER(set, PE_FRAMEBUFFER), SBUFFER(set, PE_FRAMEBUFFER), NO);
    SFLOAT(set, PE_PRERZANGLE) += SFLOAT(set, PE_PRERZINC);
  }

  // Zero the red.
  if (SENUM(set, PE_NORED)) {
    ClearRed(set);
  }

  // Zero the blue.
  if (SENUM(set, PE_NOBLUE)) {
    ClearBlue(set);
  }

  // Zero the green.
  if (SENUM(set, PE_NOGREEN)) {
    ClearGreen(set);
  }

  // Mirrors
  if (SENUM(set, PE_MIRROR_V)) {
    VerticalMirror(SBUFFER(set, PE_FRAMEBUFFER));
  }
  if (SENUM(set, PE_MIRROR_H)) {
    HorizontalMirror(SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Seed the entire array with the foreground color.
  if (SENUM(set, PE_CAA)) {
    ColorAll(GetFGColor(A_COLORALLA, set), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Clear screen to the background color.
  if (SENUM(set, PE_CAB)) {
    ColorAll(GetFGColor(A_COLORALLB, set), SBUFFER(set, PE_FRAMEBUFFER));
  }

  // Reset the one shots.
  ResetOneShots(set);
}

void ResetOneShots(int set) {
  int i;

  for (i = 0; i < patternElementCount; i++) {
    if (patternElements[i].etype == E_ONESHOT) {
      // The switch is to handle the special cases.
      switch(i) {
        case PE_PRERZALIAS: case PE_IMAGEALIAS: case PE_SNAIL: case PE_FSNAIL:
        case PE_BARSEED: case PE_SIDEPULSE:
          break;
        case PE_PRERZ:
          if ((SENUM(set, i) == OS_YES) && (SENUM(set, PE_PRERZALIAS) == OS_ONESHOT))
            SENUM(set, PE_PRERZALIAS) = OS_NO;
          if (SENUM(set, i) == OS_ONESHOT) SENUM(set, i) = OS_NO;
          break;
        case PE_POSTIMAGE:
          if ((SENUM(set, i) == OS_YES) && (SENUM(set, PE_IMAGEALIAS) == OS_ONESHOT))
            SENUM(set, PE_IMAGEALIAS) = OS_NO;
          if (SENUM(set, i) == OS_ONESHOT) SENUM(set, i) = OS_NO;
          break;
        default:
          if (SENUM(set, i) == OS_ONESHOT) SENUM(set, i) = OS_NO;
          break;
      }
    }
  }
}

// These happen to the buffer after its been processed, but before it broadcast,
// thereby not effecting future steps of the simulation.
void PostProcessModes(int set, bool_t isPrimary) {
  unsigned char *inputBuffer = SBUFFER(set, PE_FRAMEBUFFER);
  unsigned char *outputBuffer = NULL;
  int i;
  
  // Select the output buffer to write to.
  outputBuffer = isPrimary ? fbLiveBcast : fbAltBcast;
  
  // Post rotation
  if (SBOOL(set, PE_POSTRZ)) {

    // Apply the post rotation, so that it doesn't affect the feedback buffer.
    SFLOAT(set, PE_POSTRZANGLE) += SFLOAT(set, PE_POSTRZINC);
    Rotate(SFLOAT(set, PE_POSTRZANGLE), SFLOAT(set, PE_POSTEXP), SBOOL(set, PE_ALIAS), outputBuffer, inputBuffer, YES);    

  } else {

    // Just copy the buffer, because we aren't rotating it.
    memcpy(outputBuffer, inputBuffer, patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  // Set Output intensity
  for (i = 0; i < TENSOR_BYTES; i++) {
    outputBuffer[i] = (unsigned char)((float) outputBuffer[i] * SFLOAT(set, PE_INTENSITY));
  }

  // Blend, or save aside for blending.
  if (isPrimary) {
    BlendAlternate(outputBuffer, fbAltBlend);
  } else {
    // For alternate blending with the post, we copy the alternate buffer into
    // a static cache so that when we are processing the primary, we can use
    // the data from the secondary
    memcpy(fbAltBlend, outputBuffer, patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }
}
