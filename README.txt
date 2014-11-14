Tensor pattern generation - Version 3.5.0 - Functionally similar to v3.4.0.
SDL window shows preview and list of controls.
  
"Be the light you wish to see in the world."

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

