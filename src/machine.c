// machine.h
// Joshua Krueger
// 2016_10_12
// Contains the 

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
#include "transforms.h"

// Internal globals
int bufferSize = 0;
point_t livePreview, altPreview;
int tw, th;
int *liveSet, *altSet;
SDL_Rect liveBox, altBox;
unsigned char *fbLiveBcast = NULL;
unsigned char *fbAltBcast = NULL;
unsigned char *fbAltBlend = NULL;

// Internal prototypes
void ResetOneShots(int set);

// Setup the machine and its memory.
void InitMachine(int fbSize, point_t live, point_t alt, int *l, int *a) {
  bufferSize = fbSize;
  fbLiveBcast = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbLiveBcast) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    exit(EXIT_FAILURE);
  }
  fbAltBcast = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbAltBcast) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    exit(EXIT_FAILURE);
  }
  fbAltBlend = (unsigned char *) malloc(bufferSize * sizeof(unsigned char));
  if (!fbAltBlend) {
    fprintf(stderr, "Unable to allocate memory to read ip address strings from ip map file!\n");
    exit(EXIT_FAILURE);
  }
  livePreview = live;
  altPreview = alt;
  liveSet = l;
  altSet = a;
  
}

void SetMachineWH(int w, int h) {
  tw = w;
  th = h;
}

// Getters
point_t GetLiveLoc(void) { return livePreview; }
point_t GetAltLoc(void ) { return altPreview; }
int GetLiveSet(void) { return *liveSet; }
int GetAltSet(void) { return *altSet; }

// Setters
void SetLiveSet(int set) { *liveSet = set; }
void SetAltSet(int set) { *altSet = set; }


// Send out the frame buffer to tensor and/or the display window.
// 11/22/2009 - You know what was uncanny?  Walter looked a lot like FB the
// other night at the decom, and spent most of his time there running Tensor...
void UpdateDisplay(bool_t isPrimary, bool_t sendToTensor, float intensity_limit) {
  int i;
  unsigned char *outBuffer;
  unsigned char fbTemp[bufferSize];
  
  outBuffer = (isPrimary ? fbLiveBcast : fbAltBcast);
  

  // Send to the left preview and to the tensor wall
  if (isPrimary) {

    // Update the left preview
    UpdatePreview(livePreview, outBuffer);

    // Apply the global output intensity limit (not seen in the preview).
    for (i = 0 ; i < bufferSize; i++) {
      fbTemp[i] = (unsigned char)((float) outBuffer[i] * intensity_limit);
    }
    
    if (sendToTensor) UpdateTensor(fbTemp);

  } else {
    // If live and alternate are the same set, we are only doing one set of
    // ops, and those are getting sent to fbLiveBcast, not fbAltBcast.
    if (*liveSet == *altSet) outBuffer = fbLiveBcast;
    
    // Update the right preview
    UpdatePreview(altPreview, outBuffer);
  }

  return;
}

// Send frame buffer to preview area.  The preview area is a square
// encompassing the actual preview, so we should center the buffer within it.
void UpdatePreview(point_t xyOffset, unsigned char *buffer) {
  color_t pixel;
  int x, y;
  int maxDim;

  // Get the outer border dimensions.
  x = (tw * PREVIEW_PIXEL_SIZE);
  y = (th * PREVIEW_PIXEL_SIZE);

  // Get the preview area square dimension.
  maxDim = max(x, y);

  // Adjust our offsets to fit inside it.
  xyOffset.x = xyOffset.x + PREVIEW_BORDER + (maxDim - x) / 2;
  xyOffset.y = xyOffset.y + PREVIEW_BORDER + (maxDim - y) / 2;

  // Draw out the pixels.
  for (x = 0; x < tw; x++) {
    for (y = 0; y < th; y++) {

      // Get pixel color from the buffer.
      pixel = GetPixel(x, y, buffer);

      // Draw the output pixel as a square of dimension PREVIEW_PIXEL_SIZE - 1.
      // This leaves us with a border around our pixels.
      DrawBox(xyOffset.x + (x * PREVIEW_PIXEL_SIZE), xyOffset.y + (y * PREVIEW_PIXEL_SIZE),
              PREVIEW_PIXEL_SIZE - 1, PREVIEW_PIXEL_SIZE - 1, pixel);
    }
  }
}

// The thing that happens at every frame.
// isPrimary = YES if we are working with the Live Preview.
// isPrimary = NO if we are working with the Alt Preview.
void DrawNextFrame(int set, bool_t isPrimary) {

  // If its the primary set, assume we process.  If secondary, only process if not also primary.
  if (isPrimary || (*liveSet != *altSet)) {
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
  int currentSet = set; // Overrides global used in D* macros.
  int i;

  // There are two inputs to this function.  The first is the frame buffer on
  // which to operate.  The second is the transform set to perform.

  // Scroll direction randomizer
  if (DBOOL(PE_SCROLLRANDEN)) {
    if (!(rand() % DINT(PE_SCROLLRANDOM))) {
      DENUM(PE_SCROLLDIR) = rand() % DIR_COUNT;
    }
  }

  // Update the colors
  for (i = 0; i < A_COUNT; i++) {
    if (seedPalette[i].fgCycleMode > PE_INVALID && DENUM(seedPalette[i].fgCycleMode)) {
      SetFGColor(i, set, ColorCycle(DENUM(seedPalette[i].fgCycleMode),
        &DINT(seedPalette[i].fgCyclePos), DINT(seedPalette[i].fgCycleRate),
        DCOLOR(seedPalette[i].fgColorA), DCOLOR(seedPalette[i].fgColorB)));
    } else if (seedPalette[i].fgColorA > PE_INVALID) {
      SetFGColor(i, set, DCOLOR(seedPalette[i].fgColorA));
    }

    if (seedPalette[i].bgCycleMode > PE_INVALID && DENUM(seedPalette[i].bgCycleMode)) {
      SetBGColor(i, set, ColorCycle(DENUM(seedPalette[i].bgCycleMode),
        &DINT(seedPalette[i].bgCyclePos), DINT(seedPalette[i].bgCycleRate),
        DCOLOR(seedPalette[i].bgColorA), DCOLOR(seedPalette[i].bgColorB)));
    } else if (seedPalette[i].bgColorA > PE_INVALID) {
      SetBGColor(i, set, DCOLOR(seedPalette[i].bgColorA));
    }

    // Apply the alphas.
    if (seedPalette[i].fgAlpha > PE_INVALID)
      SetFGColorA(i, set, (unsigned char) (DFLOAT(seedPalette[i].fgAlpha) * 255.0));
    if (seedPalette[i].bgAlpha > PE_INVALID)
      SetBGColorA(i, set, (unsigned char) (DFLOAT(seedPalette[i].bgAlpha) * 255.0));
  }

  // Slap an image down on the display.
  if (DENUM(PE_POSTIMAGE)) {
    DrawImage(currentSet, DFLOAT(PE_IMAGEANGLE), DFLOAT(PE_IMAGEXOFFSET), DFLOAT(PE_IMAGEYOFFSET),
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
    HorizontalBars(GetFGColor(A_HBARS, set), GetBGColor(A_HBARS, set), DBUFFER(PE_FRAMEBUFFER));
  }

  // Bam! Vertical bars.
  if (DENUM(PE_VBARS)) {
    VerticalBars(GetFGColor(A_VBARS, set), GetBGColor(A_VBARS, set), DBUFFER(PE_FRAMEBUFFER));
  }

  // Crossbar seeds
  if (DENUM(PE_CROSSBAR)) {
    CrossBars(currentSet);
  }

  // Random dots.  Most useful seed ever.
  if (DENUM(PE_RDOT)) {
    RandomDots(GetFGColor(A_RDOT, set), DINT(PE_RDOTCOEF), DBUFFER(PE_FRAMEBUFFER));
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

  // Seed the entire array with the foreground color.
  if (DENUM(PE_CAA)) {
    ColorAll(GetFGColor(A_COLORALLA, set), DBUFFER(PE_FRAMEBUFFER));
  }

  // Clear screen to the background color.
  if (DENUM(PE_CAB)) {
    ColorAll(GetFGColor(A_COLORALLB, set), DBUFFER(PE_FRAMEBUFFER));
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

// These happen to the buffer after its been processed, but before it broadcast,
// thereby not effecting future steps of the simulation.
void PostProcessModes(int set, bool_t isPrimary) {
  int currentSet = set; // Overrides global used in D* macros.
  unsigned char *inputBuffer = DBUFFER(PE_FRAMEBUFFER);
  unsigned char *outputBuffer = NULL;
  int i;
  
  // Select the output buffer to write to.
  outputBuffer = isPrimary ? fbLiveBcast : fbAltBcast;
  
  // Post rotation
  if (DBOOL(PE_POSTRZ)) {

    // Apply the post rotation, so that it doesn't affect the feedback buffer.
    DFLOAT(PE_POSTRZANGLE) += DFLOAT(PE_POSTRZINC);
    Rotate(DFLOAT(PE_POSTRZANGLE), DFLOAT(PE_POSTEXP), DBOOL(PE_ALIAS), outputBuffer, inputBuffer, YES);    

  } else {

    // Just copy the buffer, because we aren't rotating it.
    memcpy(outputBuffer, inputBuffer, patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }

  // Set Output intensity
  for (i = 0; i < TENSOR_BYTES; i++) {
    outputBuffer[i] = (unsigned char)((float) outputBuffer[i] * DFLOAT(PE_INTENSITY));
  }

  // Blend, or save aside for blending.
  if (isPrimary) {
    BlendAlternate(outputBuffer, fbAltBlend);
  } else {
    // For alternate blending with the post, we copy the alternate buffer into
    // a static cache so that when we are processing the primary 
    memcpy(fbAltBlend, outputBuffer, patternElements[PE_FRAMEBUFFER].size * sizeof(unsigned char));
  }
}
