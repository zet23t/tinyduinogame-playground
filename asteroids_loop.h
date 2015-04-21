#define LOOP_PROG asteroidsLoop
#define COOLDOWN_DELAY 8

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

	#define ASTEROIDS_MAX_COUNT 24
	typedef struct Asteroid_s {
		int x, y;
		char vx, vy;
		unsigned char sizeType;
	} Asteroid;

	typedef struct AsteroidList_s {
		Asteroid list[ASTEROIDS_MAX_COUNT];
	} AsteroidList;

	#define GAME_MODE_STARTSCREEN 0
	#define GAME_MODE_PAUSE 1
	#define GAME_MODE_PLAY 2
	#define GAME_MODE_DIED 3

	#define TIMER_WARMUP 1
	#define TIMER_LEVELDONE 2

	typedef struct Game_s {
		unsigned char mode;
		unsigned char timerMode;
		unsigned int countdown;
		char shipDamage;
	} Game;

	static Ship ship;
	static ProjectileList projectileList;
	static AsteroidList asteroidList;
	static Game game;

	static void AsteroidList_spawn(char px, char py, char vx, char vy, unsigned char size, unsigned char type) {
		Asteroid a = {px<<8,py<<8,vx,vy,size|type<<3};
		
		for (unsigned char i = 0; i < ASTEROIDS_MAX_COUNT;i+=1) {
			if (asteroidList.list[i].sizeType == 0) {
				asteroidList.list[i] = a;
				break;
			}
		}		
	}

	static unsigned char AsteroidList_tick() {
		unsigned char n = 0;
		for (unsigned char i = 0; i < ASTEROIDS_MAX_COUNT;i+=1) {
			if (asteroidList.list[i].sizeType) {
				Asteroid *a = &asteroidList.list[i];
				a->x += a->vx<<4;
				a->y += a->vy<<4;
				if (a->x < (-10<<8)) a->x+=(114<<8);
				if (a->x > (104<<8)) a->x-=(114<<8);
				if (a->y < (-10<<8)) a->y+=(84<<8);
				if (a->y > (74<<8)) a->y-=(84<<8);
				unsigned char type =(a->sizeType>>3)&3;
				unsigned char size = a->sizeType&7;
				unsigned char spriteSize = size == 1 ? 8 : (size == 2 ? 11 : 16);
				unsigned char spriteY = size == 1 ? 29 : (size == 2 ? 37 : 48);
				RenderScreen_drawRectTexturedUV((a->x>>8) - (spriteSize>>1),(a->y>>8) - (spriteSize>>1), spriteSize,spriteSize,0,spriteSize * type,spriteY);
				n+=1;
			}
		}
		return n;
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
				else {
					for (unsigned char j = 0; j < ASTEROIDS_MAX_COUNT;j+=1) {
						if (asteroidList.list[j].sizeType) {
							Asteroid *a = &asteroidList.list[j];
							char dx = (a->x >> 8) - p->x;
							char dy = (a->y >> 8) - p->y;
							unsigned int dist = dx*dx + dy*dy;
							unsigned char size = a->sizeType&7;
							if (dist < (size==3 ? 9*8 : (size==2 ? 7*7 : 4*4))) {
								unsigned char hits = a->sizeType>>6;
								// hit detected
								p->age = -7;
								if (hits >= size) {
									if (size > 1) {
										for (unsigned char k=1;k<size;k+=1) {
											AsteroidList_spawn((a->x>>8)+cheapRnd()%8-4,
												(a->y>>8)+cheapRnd()%8-4,
												a->vx+cheapRnd()%15-7,
												a->vy+cheapRnd()%15-7,
												size-1,
												cheapRnd()%4);
										}
										size-=1;
										a->sizeType = a->sizeType&(~7) | size;
									} else {
										a->sizeType = 0;
									}
								} else {
									hits+=1;
									a->sizeType = a->sizeType & (~(3<<6))|(hits<<6);
								}

								break;
							}
						}
					}
				}
			} else if (projectileList.list[i].age < 0) {
				Projectile *p = &projectileList.list[i];
				unsigned char dist = (4 + projectileList.list[i].age) * 2;
				unsigned char t = (8+p->age)*6;
				for (unsigned char i=0;i<4;i+=1) {
					int dx = ((i&1)<<1)-1;
					int dy = (i&2)-1;
					if (t > 16) {
						//if (cheapRnd()%4==0) continue;
						t = 18 + cheapRnd()%4*6;
						//dx += cheapRnd()%4-2;
						//dy += cheapRnd()%4-2;
					}
					RenderScreen_drawRectTexturedUV(p->x - 3+dx*dist,p->y - 3+dy * dist, 6,6,0,t,22);
				}
				p->age+=1;
			}
		}
	}

	static void Ship_tick() {
		char activeShield = 0;
		for (unsigned char i = 0; i < ASTEROIDS_MAX_COUNT;i+=1) {
			if (asteroidList.list[i].sizeType) {
				Asteroid *a = &asteroidList.list[i];
				int dx = (a->x - ship.x)>>8;
				int dy = (a->y - ship.y)>>8;
				int dist = dx*dx+dy*dy;
				unsigned char size = a->sizeType&7;
				if (dist < (size==3 ? 12*12 : (size==2 ? 10*10 : 7*7))) {
					if (game.shipDamage < 19)
						game.shipDamage+=1;
					a->vx+=(dx>>(size));
					a->vy+=(dy>>(size));
					ship.vx-=dx<<(size);
					ship.vy-=dy<<(size);

					activeShield = 1;
				}

			}
		}
		if (game.shipDamage >= 19) {
			game.countdown = 20;
			return;
		}
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
		if (ship.vx>>6) ship.vx -= ship.vx>>6;
		else if (ship.vx) ship.vx -= ship.vx < 0 ? -1 : 1;
		if (ship.vy>>6) ship.vy -= ship.vy>>6;
		else if (ship.vy) ship.vy -= ship.vy < 0 ? -1 : 1;
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
			unsigned char shootDir = determineDir16(leftStick.normX,leftStick.normY);
			if (!steering) ship.dir = shootDir / 2;
			const char dirX[16] = {0,1,3,4, 5,4,3,1, 0,-1,-3,-4, -5,-4,-3,-1};
			const char dirY[16] = {-5,-4,-3,-1, 0,1,3,4, 5,4,3,1, 0,-1,-3,-4};
			if (shootDir/2 == ship.dir) {
				ship.cooldown = COOLDOWN_DELAY;
				ProjectileList_shoot(shipX,shipY,dirX[shootDir] + (ship.vx >> 8),dirY[shootDir] + (ship.vy >> 8));
			}
		}
		// draw ship
		char spriteX = (ship.x>>8)-8;
		char spriteY = (ship.y>>8)-8;
		RenderScreen_drawRectTexturedUV(spriteX,spriteY,16,16,0,ship.dir*16,0);
		if (activeShield) RenderScreen_drawRectTexturedUV(spriteX,spriteY,16,16,0,48,16);
	}

	static void asteroidsSetup(unsigned char cnt) {
		_renderScreen.imageIncludes[0] = &_image_asteroids_tileset;
		memset(&ship,0,sizeof(ship));
		memset(&asteroidList,0,sizeof(asteroidList));
		memset(&projectileList,0,sizeof(projectileList));
		memset(&game,0,sizeof(game));
		ship.x = 48<<8;
		ship.y = 32<<8;
		for (int i=0;i<cnt;i+=1) {
			char x = random()%96;
			char y = random()%32;
			long dx = x - 48;
			long dy = y - 32;
			if (dx*dx+dy*dy > 200)
				AsteroidList_spawn(x,y,random()%5-2,random()%5-2,3,random()%4);	
			else i -= 1;
			
		}
	}

	static void setWarmupTimer() {
		game.timerMode = TIMER_WARMUP;
		game.countdown = 0;
	}

	static void asteroidsGamePlayLoop() {
		unsigned char asteroidsCount = 0;

		if (game.timerMode == TIMER_WARMUP &&game.countdown < 80) {
			if (game.countdown < 20) {
				unsigned long r = 60 - game.countdown*3;
				RenderScreen_drawCircle(48,32,r*4l,0xff);
			}
			RenderScreen_drawText (0, 20,    0, StringBuffer_buffer("incoming asteroids"), 0xff);
			RenderScreen_drawText (45, 28,    0, StringBuffer_buffer(""), 0xff);
			StringBuffer_amendDec(3-game.countdown/20);
			game.countdown+=1;
			if (game.countdown == 80) {
				game.timerMode = 0;
				game.countdown = 0;
			}
		}
		if (game.timerMode != TIMER_WARMUP) {
			ProjectileList_tick();
			Ship_tick();
			asteroidsCount = AsteroidList_tick();

			if( asteroidsCount > 0 ) {
				// draw threat level bar to the right
	 			char cut = (19 - asteroidsCount)<<1;
				if (cut < 0) cut = 0;
				RenderScreen_drawRectTexturedUV(92,64-38+cut,4,38-cut,0,156,cut);
				RenderScreen_drawRectTexturedUV(92,64-38,4,cut,0,152,0);
				cut = (game.shipDamage)<<1;
				RenderScreen_drawRectTexturedUV(0,64-38+cut,4,38-cut,0,149,cut);
				RenderScreen_drawRectTexturedUV(0,64-38,4,cut,0,153,0);
			}
		}
		//RenderScreen_drawRectTexturedUV((96-46)/2,0,46,11,0,44,37);

		
		if (asteroidsCount == 0 && game.timerMode == 0) {
			game.timerMode = TIMER_LEVELDONE;
			game.countdown = 20 * 5;
		}
		if (game.timerMode == TIMER_LEVELDONE) {
			RenderScreen_drawText (10, 20,    0, StringBuffer_buffer("level cleared"), 0xff);
			RenderScreen_drawText (45, 28,    0, StringBuffer_buffer(""), 0xff);
			StringBuffer_amendDec(game.countdown/20);
			game.countdown-=1;
			if (game.countdown == 0) {
				asteroidsSetup(5);
				game.mode = GAME_MODE_PLAY;
				setWarmupTimer();
			}
		} else if (asteroidsCount > 0) {
			//RenderScreen_drawText (1, 1,    0, StringBuffer_new(), 0xff);
			//StringBuffer_amendDec(asteroidsCount);
		}

		if (leftButton == 2 || rightButton == 2) {
			game.mode = GAME_MODE_PAUSE;
		}
	}

	static void asteroidsGamePauseLoop() {
		RenderScreen_drawRectTexturedUV(0,22,96,15,0,64,49);
  		RenderScreen_drawText (15, 40,    0, StringBuffer_buffer("game paused"), 0xff);
  		RenderScreen_drawText (13, 40+8,  0, StringBuffer_buffer("press button"), 0xff);
  		RenderScreen_drawText (18, 40+16, 0, StringBuffer_buffer("to continue"), 0xff);
		if (leftButton == 2 || rightButton == 2) {
			game.mode = GAME_MODE_PLAY;
		}
	}

	static void asteroidsGameStartScreen(unsigned int frameCount) {
		RenderScreen_drawRectTexturedUV(0,22,96,15,0,64,49);
		if (frameCount/20&1)
  			RenderScreen_drawText (13, 40, 0, StringBuffer_buffer("press button"), 0xff);
  		RenderScreen_drawText (13, 40+8, 0, StringBuffer_buffer("  to start  "), 0xff);
  		RenderScreen_drawText (2, 40+16, 0, StringBuffer_buffer("tinyduinogames.de"), 0xff);
		if (leftButton || rightButton) {
			asteroidsSetup(2);
			game.mode = GAME_MODE_PLAY;
			setWarmupTimer();
		}
	}

	static void asteroidsLoop()
	{
		static char init = 0;
		static unsigned int frameCount = 0;
		if (!init) {
			init = 1;
			asteroidsSetup(0);
		}
		frameCount+=1;
		switch (game.mode) {
			case GAME_MODE_DIED: break;
			case GAME_MODE_PLAY: asteroidsGamePlayLoop(); break;
			case GAME_MODE_STARTSCREEN: asteroidsGameStartScreen(frameCount); break;
			case GAME_MODE_PAUSE: asteroidsGamePauseLoop(); break;
		}	
	}
}