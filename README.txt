Tensor pattern generation - Version 3.1.0 - Pattern mixer with preview.

"beep "

This pattern outputs as UDP to the tensor wall.  There is a preview window
using SDL.  This is an abbreviated set of patterns that lays out the basic
code function for later versions.

<ctrl> c - Exit
<ctrl> t - Text scroller toggle.
<ctrl> q - CellAutoFun toggle.
<ctrl> w - Bouncer toggle.
<ctrl> e - Fader toggle.

<alt> up - Increase fader decrement.
<alt> down - Decrease fader decrment.
<alt> 0 - Set fader decrement to 0.

Backspace, Return, Esc, most other keys - Alter the text buffer.



Build instructions:

  mkdir build
  cd build
  cmake ..
  make

Binaries can be found in build/bin.

