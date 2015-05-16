# png2c.lua

Lua script to be run from parent directory. Requires gd and lua file system lib.

Takes all PNG files in the images directory, converts colors to TinyDuino
screen colors (2-3-3 format). Result is stored in a filed named images_{branchname}.h 
in th playground directory. If the images has a transparent color index, this
is stored in the generated structure as well. Since transparent images are expensive
to draw, an additional structure is generated that is opaque.