Tensor pattern generation - Version 3.4.0 - SDL mixer with save/recall (broken).
SDL window shows preview and list of controls.
  
"Be the light you wish to see in the world."

This pattern outputs as UDP to the tensor wall.  There is a preview window
using SDL.

Controls keys are listed next to the preview.
Unmodified keys, backspace, esc, return alter the text buffer.

Build instructions:

  mkdir build
  cd build
  cmake ..
  make

Binary must be run from the bin directory, or the font won't be found.

  cd bin
  ./tensorSDL

