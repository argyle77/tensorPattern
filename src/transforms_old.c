// transform.c
// Joshua Krueger
// 2016_10_12
// Array manipulators

// Includes
#include <SDL2_rotozoom.h>
#include <SDL_image.h>
#include "transforms_old.h"
#include "colors.h"
#include "tensor.h"
#include "useful.h"
#include "elements.h"
#include "badOldDefines.h"
#include "my_font.h"
#include "machine.h"

// Globals
int tensorWidth, tensorHeight;

// Stuff that came with the manips...
color_t fgOutput[A_COUNT][PATTERN_SET_COUNT];
color_t bgOutput[A_COUNT][PATTERN_SET_COUNT];
float alternateBlend = 0, alternateBlendRate = 0.01;
bool_t autoBlend = NO;
SDL_Surface *imageSeed[PATTERN_SET_COUNT];

// Internal Prototypes
void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane);
SDL_Surface * FBToSurface(SDL_Surface *surface, unsigned char *FB);
unsigned char * SurfaceToFB(unsigned char *FB, SDL_Surface *surface);
void SetPixelByPlaneA(int x, int y, color_t newColor, unsigned char plane, unsigned char *buffer);
void CopyFbAlpha(unsigned char *dest, unsigned char *src, float alpha);
color_t ColorCyclePaletteSmooth(namedPalette_t palette, int *position, int wavelength);
color_t ColorCyclePalette(namedPalette_t palette, int *position, int wavelength);

// Must be called if tensorWidth or tensorHeight ever changes.
void InitXforms(int w, int h) {
  tensorWidth = w;
  tensorHeight = h;
}

// Getters
color_t GetFGColor(alterPalettes_e src, int set) { return fgOutput[src][set]; }  
color_t GetBGColor(alterPalettes_e src, int set) { return bgOutput[src][set]; }  
float GetAltBlend(void) { return alternateBlend; }
float GetAltBlendRate(void) { return alternateBlendRate; }
bool_t GetAutoBlend(void) { return autoBlend; }

// Setters
void SetAltBlend(float ab) { alternateBlend = ab; }
void SetAltBlendRate(float ab) { alternateBlendRate = ab; }
void SetAutoBlend(bool_t ab) { autoBlend = ab; }
void SetBGColor(alterPalettes_e src, int set, color_t c) { bgOutput[src][set] = c; }
void SetFGColor(alterPalettes_e src, int set, color_t c) { fgOutput[src][set] = c; }
void SetBGColorA(alterPalettes_e src, int set, int a) { bgOutput[src][set].a = a; }
void SetFGColorA(alterPalettes_e src, int set, int a) { fgOutput[src][set].a = a; }

void InitTransforms(void) {
 // Initialize the image array.
  int i;
  for (i = 0; i < PATTERN_SET_COUNT; i++) {
    imageSeed[i] = NULL;
  }
}

bool_t LoadImage(int set) {
  if (imageSeed[set]) SDL_FreeSurface(imageSeed[set]);
  imageSeed[set] = IMG_Load(SSTRING(set, PE_IMAGENAME));
  if (imageSeed[set]) return TRUE; else return FALSE;
  
}

// Draw on the buffer.
void LightPen(point_t pixel, int set, fbSelect_e fb, unsigned char * buffer) {
  // Mirror corrections
  if (SBOOL(set, PE_MIRROR_V)) {
    if (pixel.x > tensorWidth / 2) {
      pixel.x = (tensorWidth - 1) - pixel.x;
    }
  }  
  if (SBOOL(set, PE_MIRROR_H)) {
    if (pixel.y > tensorHeight / 2) {
      pixel.y = (tensorHeight - 1) - pixel.y;
    }
  }
  
  // FG vs BG
  if (fb == FBS_FG) {
    SetPixelA(pixel.x, pixel.y, fgOutput[A_LIGHTPEN][set], buffer);
  } else {
    SetPixelA(pixel.x, pixel.y, bgOutput[A_LIGHTPEN][set], buffer);
  }
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

// Crossing bars
typedef struct cbars_t {
  bool_t inProgress;
  dir_e orient;
  int x, y;
} cbars_t;
#define CBARSMAX 50  // Really prolly don't need more than TENSOR_WIDTH
void CrossBars(int set) {
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
      if (!(rand() % SINT(set, PE_CBLIKELY))) {
        cbars[set][i].inProgress = YES;
        // H or V?
        switch (SENUM(set, PE_CROSSBAR)) {
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
      SetPixelA(cbars[set][i].x, cbars[set][i].y, color, SBUFFER(set, PE_FRAMEBUFFER));
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

void SnailSeed(int set) {
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
    if (SINT(set, PE_SNAIL_POS) == tp) {
      SetPixelA(x, y, color, SBUFFER(set, PE_FRAMEBUFFER));
      SINT(set, PE_SNAIL_POS)++;
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH) {
      if (SENUM(set, PE_SNAIL) == OS_ONESHOT) SENUM(set, PE_SNAIL) = OS_NO;
      SINT(set, PE_SNAIL_POS) = 0;
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
    if (SINT(set, PE_FSNAILP) == tp) {
      //~ printf("tp: %i, (%i, %i)\n", tp, x, y);
      SetPixelA(x, y, color, SBUFFER(set, PE_FRAMEBUFFER));
      SetPixelA(x, y + 1, color, SBUFFER(set, PE_FRAMEBUFFER));
      SetPixelA(x + 1, y + 1, color, SBUFFER(set, PE_FRAMEBUFFER));
      SetPixelA(x + 1, y, color, SBUFFER(set, PE_FRAMEBUFFER));
      SINT(set, PE_FSNAILP)++;
      break;
    }
    tp++;
    if (tp >= TENSOR_HEIGHT * TENSOR_WIDTH / 4 + 5) {
      if (SENUM(set, PE_FSNAIL) == OS_ONESHOT) SENUM(set, PE_FSNAIL) = OS_NO;
      SINT(set, PE_FSNAILP) = 0;
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
  // What the fuck is going on here?
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

// Draw an image to the output with rotation and expansion.
// Rotation is acheived using SDL_gfx primitive rotozoom.
// Angle is given in degrees.  Offsets specify the center (0..1).
void DrawImage(int set, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst, float alpha) {
  SDL_Surface *s1 = NULL;
  SDL_Surface *s2 = NULL;
  SDL_Rect offset;
  int tempx, tempy;
  int cx, cy;
  unsigned char fba[TENSOR_BYTES];
  SDL_Surface *image = NULL;

  // If not, don't.
  image = imageSeed[set];
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

// Color all the pixels a certain color.
void ColorAll(color_t color, unsigned char *fb) {
  int x,y;
  for (x = 0; x < tensorWidth; x++) {
    for (y = 0; y < tensorHeight; y++) {
      SetPixelA(x, y, color, fb);
    }
  }
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

void ScrollCycle(int set) {
  static int count = 0;
  static dir_e direction = DIR_UP;

  if (count > SINT(set, PE_BOUNCER)) count = SINT(set, PE_BOUNCER);
  count--;
  if (count <= 0) {
    direction = (direction + 1) % DIR_COUNT;
    count = SINT(set, PE_BOUNCER);
  }

  Scroll(set, direction, SBOOL(set, PE_ROLLOVER), SBUFFER(set, PE_FRAMEBUFFER), PLANE_ALL);
}

// Scroller buffer manipulation
void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane) {
  int x, y, i, j;
  color_t rollSave[TENSOR_WIDTH];  // TENSOR_WIDTH is larger than TENSOR_HEIGHT
  unsigned char alpha;
  color_t temp;

  // Ignore SM_HOLD values that have been pre decremented.
  if ((shiftModes_e)direction == SM_HOLD) return;

  // Get the scroller alpha
  alpha = (unsigned char) (SFLOAT(set, PE_SCROLLALPHA) * 255.0);

  // If the topology of the surface is toroidal, we save aside the edge so that
  // it can be wrapped.
  if (toroidal) {
    switch(direction) {
      case DIR_UP:
        for(i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, 0, fb);
        break;

      case DIR_DOWN:
        if (SENUM(set, PE_MIRROR_H)) {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, (tensorHeight - 1)/2, fb);
        } else {
          for (i = 0; i < tensorWidth; i++) rollSave[i] = GetPixel(i, tensorHeight - 1, fb);
        }
        break;

      case DIR_RIGHT:
        if (SENUM(set, PE_MIRROR_V)) {
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
      if (SENUM(set, PE_MIRROR_H)) {
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
      if (SENUM(set, PE_MIRROR_V)) {
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
        if (SENUM(set, PE_MIRROR_V)) {
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
        if (SENUM(set, PE_MIRROR_H)) {
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
        if (SENUM(set, PE_MIRROR_V)) {
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

// Write a slice of text.
void WriteSlice(int set) {
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
  switch (SSENUM(set, PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      sliceColOrRow = tensorWidth - 1;
      if (SENUM(set, PE_MIRROR_V)) sliceColOrRow = (tensorWidth - 1) / 2;
      break;

    case DIR_UP:
      horizontal = NO;
      sliceColOrRow = tensorHeight - 1;
      if (SENUM(set, PE_MIRROR_H)) sliceColOrRow = (tensorHeight - 1) / 2;
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
  pixelBufferSize = strlen(SSTRING(set, PE_TEXTBUFFER)) * FONT_WIDTH;

  // Trivially, no text in the buffer.
  if (pixelBufferSize == 0) {
    // Just exit.  This actually causes the background to stop scrolling, which
    // might not be desireable.
    return;
  }

  // Prior to writing out a line, we increment the pixel buffer index to
  // point to the next part of the line to be written.  This depends on
  // scroll direction, text direction.
  textDirection = SENUM(set, PE_SCROLLDIR);
  if (SBOOL(set, PE_FONTDIR) == BACKWARDS) {
    switch (textDirection) {
      case DIR_LEFT: textDirection = DIR_RIGHT; break;
      case DIR_RIGHT: textDirection = DIR_LEFT; break;
      case DIR_DOWN: textDirection = DIR_UP; break;
      case DIR_UP: textDirection = DIR_DOWN; break;
      default: break;
    }
  }
  if (SENUM(set, PE_SCROLLDIRLAST) != SENUM(set, PE_SCROLLDIR)) {

    // Looks like we changed direction.
    SENUM(set, PE_SCROLLDIRLAST) = SENUM(set, PE_SCROLLDIR);

    // tensorWidth is used here to preserve continuity of the text, even
    // across text buffer wrap-around, when the direction changes (but only
    // really between UP - DOWN or RIGHT - LEFT changes).
    switch(textDirection) {
      case DIR_LEFT:
        SINT(set, PE_PIXELINDEX) = (SINT(set, PE_PIXELINDEX) + tensorWidth) % pixelBufferSize;
        break;
      case DIR_RIGHT:
        SINT(set, PE_PIXELINDEX) = SINT(set, PE_PIXELINDEX) - tensorWidth;
        if (SINT(set, PE_PIXELINDEX) < 0) {
          SINT(set, PE_PIXELINDEX) = pixelBufferSize + SINT(set, PE_PIXELINDEX);
        }
        break;
      case DIR_DOWN:
        SINT(set, PE_PIXELINDEX) = (SINT(set, PE_PIXELINDEX) + tensorHeight) % pixelBufferSize;
        break;
      case DIR_UP:
        SINT(set, PE_PIXELINDEX) = SINT(set, PE_PIXELINDEX) - tensorHeight;
        if (SINT(set, PE_PIXELINDEX) < 0) {
          SINT(set, PE_PIXELINDEX) = pixelBufferSize + SINT(set, PE_PIXELINDEX);
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
        SINT(set, PE_PIXELINDEX) = (SINT(set, PE_PIXELINDEX) + 1) % pixelBufferSize;
        break;
      case DIR_RIGHT:
      case DIR_UP:
        SINT(set, PE_PIXELINDEX)--;
        if ((SINT(set, PE_PIXELINDEX) < 0) || (SINT(set, PE_PIXELINDEX) >= pixelBufferSize)) {
          SINT(set, PE_PIXELINDEX) = pixelBufferSize - 1;
        }
        break;
      default:
        break;
    }
  }

  // Now using the pixel index, we find out where in the text buffer we are.
  // (Integer division.)
  bufferIndex = SINT(set, PE_PIXELINDEX) / FONT_WIDTH;

  // And where in that character is the left over from above...
  charCol = (FONT_WIDTH - 1) - (SINT(set, PE_PIXELINDEX) % FONT_WIDTH);

  // What the character is.  What it is, man.
  character = SSTRING(set, PE_TEXTBUFFER)[bufferIndex];

  // First we draw the seed's background according to our textStagger mode.
  useRand = 0;
  switch (SSENUM(set, PE_TEXTMODE, textMode_e)) {
    case TS_9ROWSTAGGER:
      // Stagger.  For landscape - 8 pixel font on 9 pixels high display.
      // Stagger the letter and fill in the pixel that isn't covered by the
      // letter with our background.
      sliceIndex = SINT(set, PE_TEXTOFFSET);
      if (!textStaggerFlag[bufferIndex]) sliceIndex += 8;
      if (horizontal) {
        sliceIndex = sliceIndex % tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      } else {
        sliceIndex = sliceIndex % tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      }
      useRand = textStaggerFlag[bufferIndex];
      break;

    case TS_10ROWBORDER:
      // No stagger. Draw a single line border on top & bottom of text with bg.
      sliceIndex = SINT(set, PE_TEXTOFFSET) - 1;
      if (horizontal) {
        if (sliceIndex < 0) sliceIndex = tensorHeight - 1;
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      } else {
        if (sliceIndex < 0) sliceIndex = tensorWidth - 1;
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      }
      sliceIndex = SINT(set, PE_TEXTOFFSET) + 8;
      if (horizontal) {
        sliceIndex %= tensorHeight;
        SetPixelA(sliceColOrRow, sliceIndex, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      } else {
        sliceIndex %= tensorWidth;
        SetPixelA(sliceIndex, sliceColOrRow, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
      }
      break;

    case TS_FULLBG:
      // No stagger, but background on the whole image.
      if (horizontal) {
        for (i = 0; i < tensorHeight; i++) {
          SetPixelA(sliceColOrRow, i, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
        }
      } else {
        for (i = 0; i < tensorWidth; i++) {
          SetPixelA(i, sliceColOrRow, bgColor, SBUFFER(set, PE_FRAMEBUFFER));
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
  if (SENUM(set, PE_TEXTMODE) == TS_NOBG) bgColor.a = 0;
  if (SENUM(set, PE_TEXTMODE) == TS_BLACK) bgColor = cBlack;

  // Now go through each pixel value to find out what to write.
  for (i = 0; i < FONT_HEIGHT; i++) {
    if (SBOOL(set, PE_FONTFLIP)) {
      j = (FONT_HEIGHT - 1) - i;
    } else {
      j = i;
    }
    fontPixelIndex = (j * FONT_CHARACTER_COUNT) + character;
    sliceIndex = i + useRand + SINT(set, PE_TEXTOFFSET);
    pixelOn = myfont[fontPixelIndex] & charColMasks[charCol];
    if (horizontal) {
      sliceIndex = sliceIndex % tensorHeight;
      SetPixelA(sliceColOrRow, sliceIndex, pixelOn ? fgColor : bgColor, SBUFFER(set, PE_FRAMEBUFFER));
    } else {
      sliceIndex = sliceIndex % tensorWidth;
      SetPixelA(sliceIndex, sliceColOrRow, pixelOn ? fgColor : bgColor, SBUFFER(set, PE_FRAMEBUFFER));
    }
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
      i = GetLiveSet();
      SetLiveSet(GetAltSet());
      SetAltSet(i);
    }
  }
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

// Or not.
void CellFun(int set) {
  int x, y;
  color_t pixelColor, oldColor;
  color_t fgColor = fgOutput[A_CELL][set];
  color_t bgColor = bgOutput[A_CELL][set];
  SINT(set, PE_CELLFUNCOUNT)++;

  for(x = 0 ; x < tensorWidth ; x++) {
    for(y = 0 ; y < tensorHeight ; y++) {
      oldColor = GetPixel(x, y, SBUFFER(set, PE_FRAMEBUFFER));
      pixelColor.r = ((x + 1) * (y + 1)) + (oldColor.r / 2) * bgColor.g;
      pixelColor.g = oldColor.g + pixelColor.r + fgColor.b;
      pixelColor.b = SINT(set, PE_CELLFUNCOUNT) + fgColor.r;
      pixelColor.a = (unsigned char) (SFLOAT(set, PE_CF_ALPHA) * 255.0);
      SetPixelA(x, y, pixelColor,  SBUFFER(set, PE_FRAMEBUFFER));
    }
  }
}

void DrawSideBar(int set) {
  int i;
  color_t color = fgOutput[A_SIDEBAR][set];
  static int sideBarCount[PATTERN_SET_COUNT];

  // Let's deal with one-shots first.
  sideBarCount[set]++;
  if (SENUM(set, PE_BARSEED) == OS_ONESHOT) {
    switch(SSENUM(set, PE_SCROLLDIR, dir_e)) {
      case DIR_LEFT: case DIR_RIGHT:
        if (sideBarCount[set] >= tensorWidth) {
          SENUM(set, PE_BARSEED) = OS_NO;
          sideBarCount[set] = 0;
        }
        break;
      case DIR_UP: case DIR_DOWN:
        if (sideBarCount[set] >= tensorHeight) {
          SENUM(set, PE_BARSEED) = OS_NO;
          sideBarCount[set] = 0;
        }
        break;
      default: break;
    }
  }

  switch (SSENUM(set, PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      for (i = 0; i < tensorHeight; i++) {
        if (SENUM(set, PE_MIRROR_V)) {
          SetPixelA((tensorWidth - 1) / 2, i, color, SBUFFER(set, PE_FRAMEBUFFER));
        } else {
          SetPixelA(tensorWidth - 1, i, color, SBUFFER(set, PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_RIGHT:
      for (i = 0; i < tensorHeight; i++) {
        SetPixelA(0, i, color, SBUFFER(set, PE_FRAMEBUFFER));
      }
      break;

    case DIR_UP:
      for (i = 0; i < tensorWidth; i++) {
        if (SENUM(set, PE_MIRROR_H)) {
          SetPixelA(i, (tensorHeight - 1) / 2, color, SBUFFER(set, PE_FRAMEBUFFER));
        } else {
          SetPixelA(i, tensorHeight - 1, color, SBUFFER(set, PE_FRAMEBUFFER));
        }
      }
      break;

    case DIR_DOWN:
      for (i = 0; i < tensorWidth; i++) {
        SetPixelA(i, 0, color, SBUFFER(set, PE_FRAMEBUFFER));
      }
      break;
    default:
      break;
  }
}

void DrawSidePulse(int set) {
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
  if (SENUM(set, PE_SIDEPULSE) == OS_ONESHOT) {
    switch(SSENUM(set, PE_SCROLLDIR, dir_e)) {
      case DIR_LEFT: case DIR_RIGHT:
        if (oneShotCount[set] > tensorHeight) {
          SENUM(set, PE_SIDEPULSE) = OS_NO;
          oneShotCount[set] = 0;
          pos[set] = 0;
          dir[set] = 1;
          return;
        }
        break;
      case DIR_UP: case DIR_DOWN:
        if (oneShotCount[set] > tensorWidth) {
          SENUM(set, PE_SIDEPULSE) = OS_NO;
          oneShotCount[set] = 0;
          pos[set] = 0;
          dir[set] = 1;
          return;
        }
        break;
      default: break;
    }
  }

  switch (SSENUM(set, PE_SCROLLDIR, dir_e)) {
    case DIR_LEFT:
      if (SENUM(set, PE_MIRROR_V)) {
        SetPixelA((tensorWidth - 1) / 2, pos[set], color, SBUFFER(set, PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (SENUM(set, PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
      } else {
        SetPixelA(tensorWidth - 1, pos[set], color, SBUFFER(set, PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (SENUM(set, PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorHeight - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_RIGHT:
        SetPixelA(0, pos[set], color, SBUFFER(set, PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (SENUM(set, PE_MIRROR_H)) {
          if (pos[set] >= (tensorHeight - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= (tensorHeight - 1) ) dir[set] = -1;
        }
        if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_UP:
      if (SENUM(set, PE_MIRROR_H)) {
        SetPixelA(pos[set], (tensorHeight - 1) / 2, color, SBUFFER(set, PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (SENUM(set, PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      } else {
        SetPixelA(pos[set], tensorHeight - 1, color, SBUFFER(set, PE_FRAMEBUFFER));
        pos[set] += dir[set];
        if (SENUM(set, PE_MIRROR_V)) {
          if (pos[set] >= (tensorWidth - 1) / 2) dir[set] = -1;
        } else {
          if (pos[set] >= tensorWidth - 1) dir[set] = -1;
        }
      }
      if (pos[set] <= 0) dir[set] = 1;
      break;

    case DIR_DOWN:
      SetPixelA(pos[set], 0, color, SBUFFER(set, PE_FRAMEBUFFER));
      pos[set] += dir[set];
      if (SENUM(set, PE_MIRROR_V)) {
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
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    SBUFFER(set, PE_FRAMEBUFFER)[(i * 3) + 0] = 0;
  }
}

void ClearGreen(int set) {
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    SBUFFER(set, PE_FRAMEBUFFER)[(i * 3) + 1] = 0;
  }
}

void ClearBlue(int set) {
  int i;
  for(i = 0; i < (tensorHeight * tensorWidth); i++) {
    SBUFFER(set, PE_FRAMEBUFFER)[(i * 3) + 2] = 0;
  }
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
