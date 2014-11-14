Tensor pattern generation - Version 2.0.0 -  Red text scroller with SDL preview.

This pattern outputs as UDP to the tensor wall.  There is a preview window
using SDL.

Type to fill the text buffer.  Backspace to delete the last character. ESC to
erase all characters. <ctrl> c to end the program.  Text buffer is 1024 
characters long.

Build instructions:

  mkdir build
  cd build
  cmake ..
  make

Binaries can be found in build/bin.

