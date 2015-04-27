# Project purpose

Programming games for the TinyDuino system is a challenge. The severe 
limitations regarding RAM and flash memory makes it difficult to develop 
games with this system. 

The TinyScreen itself has a small resolution, however drawing to the screen
takes substantial amounts of time. Moreover, drawing has to happen one line
after another, all lines through from top to bottom. This property makes
it additionally complicated to draw on the screen in a flickerfree way.

*This project aims to require as little resources as possible while providing 
an interface to draw efficiently as well as easily on the screen.*

# Approach

* Include everything into the main file via #include
	* The compiler can optimize the code better when everything is in one file
	* The code execution is significantly faster - calling externally defined functions is expensive
* Avoid C++
	* ... I am simply not experienced enough at writing C++ code
	* it's easy to waste precious flash memory for a bit more comfort
	* I have saved quite some memory by porting existing code into plain C code, thus gaining the attitude that it might be worth the effort




