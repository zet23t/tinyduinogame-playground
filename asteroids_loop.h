#define LOOP_PROG asteroidsLoop

#include "asteroids_tileset.h"
extern "C" {

	typedef struct Ship_s {
		int x,y;
		int vx,vy;
		char dir;
	} Ship;

	static Ship ship = {32,32,0,0,0};

	static void Ship_tick() {
		ship.vx += rightStick.normX>>2;
		ship.vy += rightStick.normY>>2;
		ship.x+=ship.vx;
		ship.y+=ship.vy;
		while (ship.x > (96<<8)) ship.x-=96<<8;
		while (ship.x < 0) ship.x+=96<<8;
		while (ship.y < 0) ship.y+=64<<8;
		while (ship.y > (64<<8)) ship.y-=64<<8;

		float fx = rightStick.normX;
		float fy = rightStick.normY;

		ship.vx -= ship.vx>>6;
		ship.vy -= ship.vy>>6;
		if (fx != 0 && fy != 0)
			ship.dir = (int)(atan2(fx,-fy)/3.14f/2*8);
		RenderScreen_drawRectTexturedUV((ship.x>>8)-8,(ship.y>>8)-8,16,16,0,ship.dir*16,0);
	}

	void asteroidsLoop()
	{
		_renderScreen.imageIncludes[0] = &_image_asteroids_tileset;
		
		Ship_tick();
	}
}