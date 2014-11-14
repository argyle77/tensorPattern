Tensor pattern generation - Version 3.8.0
Added to info display, added image load, fps control, bi-planes.
Released to pdf and decom 2010 lists.
  
"Frostbyte was an enigineer."

This pattern outputs as UDP to the tensor wall.  There is a preview window
using SDL.

Controls keys are listed next to the preview.
Unmodified keys, backspace, esc, return alter the text buffer.

NOTE: SDL GFX is required for this version.  There is no cmake module for
ensuring that this is on the system, so be sure to install libsdl-gfx1.2-dev

Build instructions:

  mkdir build
  cd build
  cmake ..
  make

Binary must be run from the bin directory, or the font won't be found.

  cd bin
  ./tensorSDL

