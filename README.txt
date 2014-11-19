Tensor pattern generation - Version 3.9.2.a
  
"Frostbyte was an enigineer."

This pattern generator outputs UDP to the tensor wall. A preview
window shows the pattern and interface using SDL.

Controls keys are listed next to the preview.
Unmodified keys, backspace, esc, and return alter the text buffer.

Required libraries:
libsdl1.2-dev
libsdl-ttf2.0-dev
libsdl-gfx1.2-dev
libsdl-image1.2-dev
Also perhaps build-essential, cmake, and git.

Build instructions:
  git clone https://github.com/argyle77/tensorPattern.git
  cd tensorPattern
  mkdir build
  cd build
  cmake ..
  make

The binary must be run from the build directory, or the font won't be found.
  ./tensorSDL

I know the code is ugly.  It was not written at a leisurely pace, and not
edited when there wasn't a press.  There is little explanation as to what
it does, and beyond that, the interface is, at best, a puzzle.  I found it
worked best to show someone or two interested parties every so often the
bare minimum of how to use the interface, and then to step back.  It was
taken up by the puzzle solvers, the ones who'd never necessarily finished
a Rubik's cube because they'd figured out how it worked and were satisfied 
at that.

Known Issues:
Some key assignments may interfere with your windowing environment.  Sorry.
