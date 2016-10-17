// drawSupport.c
// Contains some basic drawing support
// Joshua Krueger
// 2016_10_16

#include "drawSupport.h"

point_t SetPoint(int x, int y) {
  point_t point;
  point.x = x;
  point.y = y;
  return point;
}

void SetBox(box_t *box, int x, int y, int w, int h) {
  box->x = x;
  box->y = y;
  box->w = w;
  box->h = h;
}
