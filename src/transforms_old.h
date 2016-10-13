// Structure and function definitions for manipulating transform sets.
// Joshua Krueger
// 2016_10_11

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

  // Includes
  #include "draw.h"
  #include "useful.h"
  #include "colors.h"
  #include "elements.h"

  // Typedefs
  typedef struct transform_t {

  } transform_t;
  
  typedef enum fbSelect_e {
    FBS_FG,
    FBS_BG
  } fbSelect_e;

  // Defines
  
  // Public functions
  void InitXforms(int w, int h);
  
  // Array manipulations
  void HorizontalMirror(unsigned char * buffer);
  void VerticalMirror(unsigned char * buffer);
  void ScrollCycle(int set);
  void Scroll (int set, dir_e direction, bool_t toroidal, unsigned char *fb, unsigned char plane);
  
  // Simple
  void ColorAll(color_t color, unsigned char *fb);
  void VerticalBars(color_t fg, color_t bg, unsigned char *buffer);
  void HorizontalBars(color_t fg, color_t bg, unsigned char *buffer);
  void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
  void CellFun(int set);
  void ClearBlue(int set);
  void ClearGreen(int set);
  void ClearRed(int set);
  
  // 
  void Diffuse(float diffusionCoeff, bool_t isToroid, unsigned char *buffer);
  void Multiply(float multiplier, unsigned char *buffer);
  void FadeAll(int inc, fadeModes_e fadeMode, unsigned char *buffer);
  void Rotate(double angle, double expansion, int aliasmode, unsigned char *fb_dst, unsigned char *fb_src, unsigned char exclude);
  void DrawImage(int set, double angle, float xoffset, float yoffset, double expansion, bool_t aliasmode, unsigned char *fb_dst, float alpha);
  bool_t LoadImage(int set);
  
  // More complicated things
  void CrossBars(int set);
  void SnailSeed(int set);
  void FastSnailSeed(int set);
  void DrawSideBar(int set);
  void DrawSidePulse(int set);
  void WriteSlice(int set);


  
  void LightPen(point_t pixel, int set, fbSelect_e fb, unsigned char * buffer);
  
  // Post
  void BlendAlternate(unsigned char *fba, unsigned char *fbb);
  
  // Colors
  color_t ColorCycle(colorCycleModes_e cycleMode, int *cycleSaver, int cycleSteps, color_t a, color_t b);
  
  // Getters and setters
  void SetAltBlendRate(float ab);
  void SetAltBlend(float ab);
  float GetAltBlend(void);
  float GetAltBlendRate(void);
  bool_t GetAutoBlend(void);
  void SetAutoBlend(bool_t ab);
  color_t GetBGColor(alterPalettes_e src, int set);
  color_t GetFGColor(alterPalettes_e src, int set);
  void SetBGColor(alterPalettes_e src, int set, color_t c);
  void SetFGColor(alterPalettes_e src, int set, color_t c);
  void SetBGColorA(alterPalettes_e src, int set, int a);
  void SetFGColorA(alterPalettes_e src, int set, int a);
  
#endif /* ifndef TRANSFORM_H_ */
