// drawSupport.h
// Contains some basic drawing support
// Joshua Krueger
// 2016_10_12

#ifndef DRAWSUPPORT_H_
  #define DRAWSUPPORT_H_
  
  // Points
  typedef struct point_t {
    int x, y;
  } point_t;
  
  // Boxes.  We'll use theirs.
  #include <SDL.h>
  #define box_t SDL_Rect  
  
  // Functions
  point_t SetPoint(int x, int y);
  void SetBox(box_t *box, int x, int y, int w, int h);
  
#endif /* DRAWSUPPORT_H_ */
