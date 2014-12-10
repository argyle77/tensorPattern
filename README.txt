Tensor pattern generation - Version 7.2.0
http://core9.org/tensor
http://wiki.artisansasylum.com/index.php/Tensor
http://wiki.artisansasylum.com/index.php/Tensor_2011

  
"Frostbyte was an enigineer."


This pattern generator sends UDP packets to the tensor wall. A preview window
shows the pattern to be sent and the interface for altering the pattern using
SDL2.

Controls are listed next to the previews.  Modes and parameters can be adjusted
with the mouse and mouse wheel.  Right clicking on most numerical parameters
will allow you to edit them with the keyboard.  Unmodified keys, backspace, and
esc alter the text buffer.  Clicking directly on the preview draws with the
foreground color.

There are 10 pattern sets in memory at all times which can be chose between by
pressing <ctrl> 0-9. By default the program automatically switches between them
(about every minute or so).  This can be disabled if desired.  Pattern sets can
be saved and loaded from disk.  Saved pattern sets 0-9 are automatically loaded
at startup.  Images can be loaded by placing the image file in the build
directory, editing the ImageName value in one of the *.now save files, and then 
loading that pattern into memory using <ctrl> <shift> 0-9 a-z.  Pattern sets 
can be edited live or in the alternate preview window.  The control set
corresponds to the (green) highlighted preview.

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

