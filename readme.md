# TinyDuino Playground

This is my playground project for the [TinyDuino Video Game Kit](http://tinyduinogames.de/pages/TinyDuino/).

# On the repository structure

The master branch is a test ground for the basic functionality that I need to make games. I am creating branches
for the games I want to make, merging in the updates that I do on the master branch. I am not sure if that makes sense,
however there's a very strong inter dependence between projects and the framework code, so I wasn't sure if I wanted
to start with individual projects where only a few files are added and most of it is library code.

# Hardware specs and challenges

The TinyDuino processor has 32kb of flash memory, 2kb of RAM and 1kb of EEPROM. The flash memory is more 
or less readonly and contains the actual program binary and all data. The RAM is shared between the stack, static and
heap allocations and the EEPROM is used as a hard disk environment to store data persistently.

The processor runs at 8MHz, most instructions use 1 clock cycle. The screen has 96x64 pixels and is 16bit capable but
8bit is used (rgb 2-3-3 format) since data transfer rate to the display is quite limited. 
