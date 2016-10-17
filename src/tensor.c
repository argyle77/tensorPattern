// Tensor functions
// Functions to make output to tensor wall easier.
// Joshua Krueger
// 2016_10_12

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "drv-tensor.h"
#include "tensor.h"
#include "useful.h"
//~ #include "draw.h"
#include "transforms_old.h"
#include "machine.h"
#include "layout.h"

// Defines
#define IP_STRING_SIZE 16

// Internal Globals
int tensorWidth, tensorHeight;
bool_t useDefaultPixelMap = YES;
point_t tensorPixelMap [TENSOR_WIDTH * TENSOR_HEIGHT];
const char **ipmap1;
const char **ipmap2;
const char **ipmap3;
SDL_Rect liveBox, altBox;
bool_t tensorEnable = NO;
float tensorLimit = GLOBAL_INTENSITY_LIMIT_DEFAULT;

// Internal Prototypes
void SetDims(void);
void LoadTensorMaps(void);
void LoadIPMap(void);
void LoadPixelMap(void);
void GenerateDefaultPixelMap(void);

// Initialize 
void InitTensor(void) {
  //tensor_landscape_p = 1;  // Landscape mode (good for single panel).
  tensor_landscape_p = 0;  // Portrait mode (normal).

  // Set the widths / heights
  SetDims();  // After set tensor_landscape_p.

  // Load the ip and pixels maps.
  LoadTensorMaps();

  // Initialize tensor communications.
  tensor_init(ipmap1, ipmap2, ipmap3);
}

// Getters
int GetTensorHeight(void) { return tensorHeight; } 
int GetTensorWidth(void) { return tensorWidth; }
int GetTensorEnable(void) { return tensorEnable; }
float GetTensorLimit(void) { return tensorLimit; }
SDL_Rect GetLiveBox(void) { return liveBox; }
SDL_Rect GetAltBox(void) { return altBox; }

// Setters
void SetTensorEnable(bool_t e) { tensorEnable = e; }
void SetTensorLimit(float l) { 
  // Verify the range.
  if (l < GLOBAL_INTENSITY_LIMIT_MIN ||
      l > GLOBAL_INTENSITY_LIMIT_MAX) {
    l = GLOBAL_INTENSITY_LIMIT_DEFAULT;
  } 
  tensorLimit = l;
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

  // Set widths and heights in the things.  Why not just global?
  InitXforms(tensorWidth, tensorHeight);
  SetMachineWH(tensorWidth, tensorHeight);
  
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

// Send an update to tensor, applying the pixel map and output limiter.
void SendTensor(unsigned char *buffer) {
  unsigned char bufA [TENSOR_BYTES];
  unsigned char bufB [TENSOR_BYTES];
  int x, y;
  color_t temp;
  
  // Apply the global output intensity limit (not seen in the preview).
  for (x = 0 ; x < TENSOR_BYTES; x++) {
    bufA[x] = (unsigned char)((float) buffer[x] * tensorLimit);
  }
    
  // Apply the pixel map
  for (y = 0; y < tensorHeight; y++) {
    for (x = 0; x < tensorWidth; x++) {
      temp = GetPixel(x, y, bufA);
      SetPixel(tensorPixelMap[x + (y * tensorWidth)].x, tensorPixelMap[x + (y * tensorWidth)].y, temp, bufB);
    }
  }

  if (tensorEnable) tensor_send(bufB);
}

// Get the color of a particular pixel from a framebuffer.
color_t GetPixel(int x, int y, unsigned char *buffer) {
  color_t colorTemp = cBlack;
  if (x >= tensorWidth || x < 0 || y >= tensorHeight || y < 0) return colorTemp;
  colorTemp.r = buffer[(y * tensorWidth * 3) + (x * 3) + 0];
  colorTemp.g = buffer[(y * tensorWidth * 3) + (x * 3) + 1];
  colorTemp.b = buffer[(y * tensorWidth * 3) + (x * 3) + 2];
  colorTemp.a = 255;
  return colorTemp;
}

// Set a single pixel a particular color.
void SetPixel(int x, int y, color_t color, unsigned char *buffer) {
  if ((x >= tensorWidth) || (y >= tensorHeight) || (x < 0) || (y < 0)) return;

  buffer[(y * tensorWidth * 3) + (x * 3) + 0] = color.r;
  buffer[(y * tensorWidth * 3) + (x * 3) + 1] = color.g;
  buffer[(y * tensorWidth * 3) + (x * 3) + 2] = color.b;
}

// Set a single pixel a particular color with alpha blending.
// Is this ever used?  Does it work?
// TODO: Uncomplicate this.
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

// Allows for remapping of pixel output and ip addresses without recompiling.
void LoadTensorMaps(void) {
  LoadIPMap();
  LoadPixelMap();
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
