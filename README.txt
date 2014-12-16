Tensor pattern generation - Version 8.1.0
http://core9.org/tensor
http://wiki.artisansasylum.com/index.php/Tensor
http://wiki.artisansasylum.com/index.php/Tensor_2011

  
"Frostbyte was an enigineer."


This pattern generator sends UDP packets to the tensor wall. A preview window
shows the pattern to be sent and the interface for altering the pattern using
SDL2.

Controls are listed next to the previews.  Modes and parameters can be adjusted
with the mouse and mouse wheel.  Left clicking toggles some modes, opens
selection dialogs, or permits keyboard editing of most parameters.  Right
clicking resets parameters to default values.  Unmodified keys, backspace, and
delete alter the text buffer.  Clicking (left or right) directly on the preview
draws with the pen foreground or background color.  Mouse wheel over the
previews allows selection of the pen color.  Mouse wheel over floating point
numbers causes them to be incremented or decremented by the "Float step," which
can be adjusted under "Auxillary" on the far lower right.  Most seeds (anything
that draws) and most modes (anything that alters the next frame) can be either
turned on to persist, or activated briefly (one-shot).  The effects of 
persistent seeds can be lessened by adjusting the seed's alpha value.  Mouse
wheel down over most seeds activates one-shot, whereas mouse up activates 
persistent.

There are 10 pattern sets in memory at all times which can be chosen between by
pressing <ctrl> 0-9.  The program can be made to automatically cycle through
them.  Pattern sets can be saved and loaded from disk.  Saved pattern sets 0-9 
are automatically loaded at startup.  Images can be loaded by placing the image
file in the build directory, editing the ImageName value in one of the *.now
save files, and then loading that pattern into memory using <ctrl> <shift> 0-9
a-z.  Pattern sets can be edited live or in the alternate preview window.  The
controls operate on the currently (green) highlighted preview.

If Tensor topology has changed, the IP addresses for the sections can be edited
in tensorIP.map, and if necessary, the pixels themselves can be remapped using
tensorPixel.map.

If the patterns are too bright on the tensor wall itself, a global intensity 
limit can be set on the commandline, by supplying a floating point argument
greater than 0.0 and less than 1.0.  Default is 1.0 (100%).  This limit will 
not effect the preview window.  A per-patternset intensity limit is available
as well through the gui and will effect the preview output.

Required libraries:
libsdl2-dev (>=2.0.2)
libsdl2-ttf-dev (>=2.0.0)
libsdl2-gfx-dev (>=1.0.0-2)
libsdl2-image-dev (>=2.0.0)

Hint, also perhaps build-essential, cmake, git, and maybe pkgconfig.
(Ubuntu and derivatives - specifically tested on Linux Mint 17, 32 and 64 bit
xfce editions)

Build instructions:
  git clone https://github.com/argyle77/tensorPattern.git
  cd tensorPattern
  mkdir build
  cd build
  cmake ..
  make

The binary must be run from the build directory, or the fonts, images, and save
files won't be found.
  ./tensorSDL

