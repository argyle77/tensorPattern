// useful.c
#include "useful.h"

int min(int a, int b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

int max(int a, int b) {
  if (b < a) {
    return a;
  } else {
    return b;
  }
}
