Tensor pattern generation - Version 3.8.1
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

I know the code is ugly.  It was not written at a leisurely pace, and not
edited when there wasn't a press.  There is little explanation as to what
it does, and beyond that, the interface is, at best, a puzzle.  I found it
worked best to show someone or two interested parties every so often the
bare minimum of how it worked, and then to step back.  It was taken up by
the puzzle solvers, the ones who'd never necessarily finished a Rubik's cube
because they'd figured out how it worked and were satisfied at that.

