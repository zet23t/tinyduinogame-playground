#define LOOP_PROG asteroidsLoop

#include "asteroids_tileset.h"
extern "C" {

	typedef struct Ship_s {
		int x,y;
		int vx,vy;
		char dir;
	} Ship;

	static Ship ship = {32<<8,32<<8,0,0,0};

	static void Ship_tick() {
		ship.vx += rightStick.normX>>2;
		ship.vy += rightStick.normY>>2;
		ship.x+=ship.vx;
		ship.y+=ship.vy;
		while (ship.x > (96<<8)) ship.x-=96<<8;
		while (ship.x < 0) ship.x+=96<<8;
		while (ship.y < 0) ship.y+=64<<8;
		while (ship.y > (64<<8)) ship.y-=64<<8;

		//float fx = rightStick.normX;
		//float fy = rightStick.normY;

		ship.vx -= ship.vx>>6;
		ship.vy -= ship.vy>>6;
		/*if (fx != 0 && fy != 0) {
			ship.dir = (int)(atan2(fx,-fy)/3.141592653f/2.0f*8.0f);
			float len = sqrt(fx*fx+fy*fy);
			float nx = fx/len;
			float ny = fy/len;

		}*/
		char shipX = ship.x>>8;
		char shipY = ship.y>>8;

		if (rightStick.normX != 0 || rightStick.normY != 0) {
			if ((abs(rightStick.normX)) > (abs(rightStick.normY)<<1)) {
				if (rightStick.normX > 0) ship.dir = 2;
				else ship.dir = 6;
			} else if ((abs(rightStick.normY)) > (abs(rightStick.normX)<<1)) {
				if (rightStick.normY > 0) ship.dir = 4;
				else ship.dir = 0;
			} 
			else if (rightStick.normX > 0 && rightStick.normY > 0) ship.dir = 3;
			else if (rightStick.normX > 0 && rightStick.normY < 0) ship.dir = 1;
			else if (rightStick.normX < 0 && rightStick.normY > 0) ship.dir = 5;
			else ship.dir = 7;
			char px[2],py[2];
			switch (ship.dir) {
				case 0: px[0] =  0, py[0] =  5,px[1] =   0, py[1] =   8; break;
				case 1: px[0] = -3, py[0] =  3,px[1] =  -6, py[1] =   6; break;
				case 2: px[0] = -5, py[0] =  0,px[1] =  -8, py[1] =   0; break;
				case 3: px[0] = -3, py[0] = -3,px[1] =  -6, py[1] =  -6; break;
				case 4: px[0] =  0, py[0] = -5,px[1] =   0, py[1] =  -8; break;
				case 5: px[0] =  3, py[0] = -3,px[1] =   6, py[1] =  -6; break;
				case 6: px[0] =  5, py[0] =  0,px[1] =   8, py[1] =   0; break;
				case 7: px[0] =  3, py[0] =  3,px[1] =   6, py[1] =   6; break;
			}
			unsigned int f = (rightStick.normX>>4) * (rightStick.normX>>4) + (rightStick.normY>>4) * (rightStick.normY>>4);
			unsigned char n = 0;
			if (f > 500) RenderScreen_drawRectTexturedUV(px[1]+(px[0]>>1)+shipX-2,py[1]+(py[0]>>1)+shipY-3, 6,6,0,12-n++*6,16);
			if (f > 200) RenderScreen_drawRectTexturedUV(px[1]+shipX-2,py[1]+shipY-3, 6,6,0,12-n++*6,16);
			RenderScreen_drawRectTexturedUV(px[0]+shipX-2,py[0]+shipY-3, 6,6,0,12-n++*6,16);
		}
		RenderScreen_drawRectTexturedUV((ship.x>>8)-8,(ship.y>>8)-8,16,16,0,ship.dir*16,0);
	}

	void asteroidsLoop()
	{
		_renderScreen.imageIncludes[0] = &_image_asteroids_tileset;
		
		Ship_tick();
	}
}