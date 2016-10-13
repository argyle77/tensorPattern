// tensor.h
// Joshua Krueger
// 2016_20_12
// Contains the definitions for the tensor wall / simulation description

#ifndef TENSOR_H_
  #define TENSOR_H_
  
  #include "colors.h"
  #include "useful.h"
  
  #define PANELS_WIDE 3
  #define PANELS_HIGH 1
  #define PANEL_WIDTH 9
  #define PANEL_HEIGHT 20
  #define PIXEL_BYTES 3
  
  #define TENSOR_WIDTH (PANELS_WIDE * PANEL_WIDTH)
  #define TENSOR_HEIGHT (PANELS_HIGH * PANEL_HEIGHT)
  
  #define TENSOR_BYTES (TENSOR_WIDTH * TENSOR_HEIGHT * PIXEL_BYTES)
  
  #define TENSOR_ADDR_PER_PANEL 6
  #define TENSOR_PANEL_COUNT (PANELS_WIDE * PANELS_HIGH)
  
  // Intensity limit
  #define GLOBAL_INTENSITY_LIMIT_DEFAULT 1.0
  #define GLOBAL_INTENSITY_LIMIT_MAX 1.0
  #define GLOBAL_INTENSITY_LIMIT_MIN 0.0
  
  // Public Functions
  void InitTensor(void);
  void UpdateTensor(unsigned char *buffer);
  color_t GetPixel(int x, int y, unsigned char *buffer);
  void SetPixel(int x, int y, color_t color, unsigned char *buffer);
  void SetPixelA(int x, int y, color_t color, unsigned char *buffer);
  void SetDims(void); 
  
  // Getters, setters
  int GetTensorWidth(void);
  int GetTensorHeight(void);
  int GetTensorEnable(void);
  void SetTensorEnable(bool_t e);
  void SetTensorLimit(float l);
  float GetTensorLimit(void);
  
#endif /* TENSOR_H_ */
