Tensor pattern generation - Version 5.0.0
  
"Frostbyte was an enigineer."

This pattern generator outputs UDP to the tensor wall. A preview
window shows the pattern and interface using SDL.

Controls keys are listed next to the preview. Modes and parameters
can be adjusted with the mouse and mouse wheel.  Unmodified keys,
backspace, and esc alter the text buffer.

Required libraries:
libsdl2-dev (>=2.0.2)
libsdl2-ttf-dev (>=2.0.0)
libsdl2-gfx-dev (>=1.0.0-2)
libsdl2-image-dev (>=2.0.0)

Hint, also perhaps build-essential, cmake, and git.
(Ubuntu and derivatives)

Build instructions:
  git clone https://github.com/argyle77/tensorPattern.git
  cd tensorPattern
  mkdir build
  cd build
  cmake ..
  make

The binary must be run from the build directory, or the font won't be found.
  ./tensorSDL

Known Issues:
Some key assignments may interfere with your windowing environment.  Sorry.

