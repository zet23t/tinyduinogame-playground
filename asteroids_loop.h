#define LOOP_PROG asteroidsLoop

#include "asteroids_tileset.h"
extern "C" {

	typedef struct Ship_s {
		int x,y;
		int vx,vy;
		char dir;
		char cooldown;
	} Ship;

	typedef struct Projectile_s {
		char x, y, vx, vy, age;
	} Projectile;

	#define PROJECTILE_MAX_COUNT 10
	typedef struct ProjectileList_s {
		Projectile list[PROJECTILE_MAX_COUNT];
	} ProjectileList;

	#define ASTEROIDS_MAX_COUNT 30
	typedef struct Asteroid_s {
		int x, y;
		char vx, vy, size;
	} Asteroid;

	typedef struct AsteroidList_s {
		Asteroid list[ASTEROIDS_MAX_COUNT];
	} AsteroidList;

	static Ship ship = {32<<8,32<<8,0,0,0};
	static ProjectileList projectileList;
	static AsteroidList asteroidList;

	static void AsteroidList_spawn(char px, char py, char vx, char vy, char size) {
		Asteroid a = {px,py,vx,vy,size};
		
		for (unsigned char i = 0; i < ASTEROIDS_MAX_COUNT;i+=1) {
			if (asteroidList.list[i].size == 0) {
				asteroidList.list[i] = a;
				break;
			}
		}		
	}

	static void AsteroidList_tick() {
		for (unsigned char i = 0; i < ASTEROIDS_MAX_COUNT;i+=1) {
			if (asteroidList.list[i].size) {
				Asteroid *a = &asteroidList.list[i];
				a->x += a->vx;
				a->y += a->vy;
				if (a->x < (-10<<4)) a->x+=(114<<4);
				if (a->x > (104<<4)) a->x-=(114<<4);
				if (a->y < (-10<<4)) a->y+=(84<<4);
				if (a->y > (74<<4)) a->y-=(84<<4);
				RenderScreen_drawRectTexturedUV((a->x>>4) - 8,(a->y>>4) - 8, 16,16,0,0,48);	
			}
		}
	}

	static void ProjectileList_shoot(char px, char py, char vx, char vy) {
		Projectile p = {px,py,vx,vy,20};

		for (unsigned char i = 0; i < PROJECTILE_MAX_COUNT;i+=1) {
			if (projectileList.list[i].age == 0) {
				projectileList.list[i] = p;
				break;
			}
		}
	}

	static void ProjectileList_tick() {
		for (unsigned char i = 0; i < PROJECTILE_MAX_COUNT;i+=1) {
			if (projectileList.list[i].age > 0) {
				Projectile *p = &projectileList.list[i];
				RenderScreen_drawRectTexturedUV(p->x - 3,p->y - 3, 6,6,0,6,22);	
				p->x+=p->vx;
				p->y+=p->vy;
				p->age-=1;
				RenderScreen_drawRectTexturedUV(p->x - 3,p->y - 3, 6,6,0,0,22);	
				if (p->x > 100 || p->x < -10 || p->y > 70 || p->y < -10) p->age = 0;
			}
		}
	}

	static unsigned char determineDir8(int dx, int dy) {
		if ((abs(dx)) > (abs(dy)<<1)) {
			if (dx > 0) return 2;
			else return 6;
		} else if ((abs(dy)) > (abs(dx)<<1)) {
			if (dy > 0) return 4;
			else return 0;
		} 
		else if (dx > 0 && dy > 0) return 3;
		else if (dx > 0 && dy < 0) return 1;
		else if (dx < 0 && dy > 0) return 5;
		else return 7;
	}

	static void Ship_tick() {
		// accelerate
		ship.vx += rightStick.normX>>2;
		ship.vy += rightStick.normY>>2;
		// move
		ship.x+=ship.vx;
		ship.y+=ship.vy;
		// wrap
		while (ship.x > (96<<8)) ship.x-=96<<8;
		while (ship.x < 0) ship.x+=96<<8;
		while (ship.y < 0) ship.y+=64<<8;
		while (ship.y > (64<<8)) ship.y-=64<<8;
		// drag
		ship.vx -= ship.vx>>6;
		ship.vy -= ship.vy>>6;
		// screen pos
		char shipX = ship.x>>8;
		char shipY = ship.y>>8;
		// steer
		char steering = rightStick.normX != 0 || rightStick.normY != 0;
		if (steering) {
			// determine steering direction
			ship.dir = determineDir8(rightStick.normX,rightStick.normY);
			// exhaust directions
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
			// draw exhaust
			if (f > 500) RenderScreen_drawRectTexturedUV(px[1]+(px[0]>>1)+shipX-2,py[1]+(py[0]>>1)+shipY-3, 6,6,0,12-n++*6,16);
			if (f > 200) RenderScreen_drawRectTexturedUV(px[1]+shipX-2,py[1]+shipY-3, 6,6,0,12-n++*6,16);
			RenderScreen_drawRectTexturedUV(px[0]+shipX-2,py[0]+shipY-3, 6,6,0,12-n++*6,16);
		}
		if (ship.cooldown > 0) {
			ship.cooldown -= 1;
		}
		if ((leftStick.normX != 0 || leftStick.normY != 0) && ship.cooldown == 0) {
			unsigned char shootDir = determineDir8(leftStick.normX,leftStick.normY);
			if (!steering) ship.dir = shootDir;
			const char dirX[8] = {0,3,4,3,0,-3,-4,-3};
			const char dirY[8] = {-4,-4,0,3,4,3,0,-3};
			if (shootDir == ship.dir) {
				ship.cooldown = 5;
				ProjectileList_shoot(shipX,shipY,dirX[ship.dir] + (ship.vx >> 8),dirY[ship.dir] + (ship.vy >> 8));
			}
		}
		// draw ship
		RenderScreen_drawRectTexturedUV((ship.x>>8)-8,(ship.y>>8)-8,16,16,0,ship.dir*16,0);
	}

	void asteroidsLoop()
	{
		static char init = 0;
		if (!init) {
			init = 1;
			AsteroidList_spawn(40,30,2,3,3);
			AsteroidList_spawn(90,50,2,-2,3);
		}
		_renderScreen.imageIncludes[0] = &_image_asteroids_tileset;
		
		ProjectileList_tick();
		Ship_tick();
		AsteroidList_tick();
	}
}