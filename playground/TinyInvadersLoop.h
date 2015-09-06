#include "images_tiny_invaders.h"
//#define SHOW_FPS 0
#define LOOP_PROG tinyInvadersLoop
#define TS_SKY0 0
#define TS_CLOUDS 4
#define TS_SPRITES 1
#define TS_CITYLAYER 2
#define TS_ROADLAYER 3
#define PLAYER_SHIP_Y 56
#define PLAYER_SHIP_SIZE_X 7
#define PLAYER_SHIP_SIZE_Y 7
#define PLAYER_SHOOT_COOLDOWN 15

#define PROJECTILE_U 0
#define PROJECTILE_V 7
#define PROJECTILE_W 3
#define PROJECTILE_H 3
#define PROJECTILE_TYPE_PLAYER 1
#define PROJECTILE_TYPE_ENEMY 2

#define MAX_LEFT 8
#define MAX_RIGHT 88
#define MAX_PROJECTILE_COUNT 16

#define MONSTER_ROWS 3
#define MONSTER_COLS 6
#define MONSTER_WIDTH 8
#define MONSTER_HEIGHT 6
#define MONSTER_SPACING 4
#define MONSTER0_U 0
#define MONSTER0_V 42
#define MONSTER_ROW_WIDTH (MONSTER_COLS * (MONSTER_WIDTH + MONSTER_SPACING))

#define GAME_MODE_START 1
#define GAME_MODE_PLAY 2
#define GAME_MODE_PAUSE 3
#define GAME_MODE_GAMEOVER 4
#define GAME_MODE_CREDITS 5
#define GAME_MODE_WON 6

#define EXPLOSION_SPRITE_U 25
#define EXPLOSION_SPRITE_V 0
#define EXPLOSION_SPRITE_W 6
#define EXPLOSION_SPRITE_H 6
#define EXPLOSION_SPRITE_N 4

#define FIREWORKS_SPRITE_U 36
#define FIREWORKS_SPRITE_V 6
#define FIREWORKS_SPRITE_W 3
#define FIREWORKS_SPRITE_H 3
#define FIREWORKS_SPRITE_N 4

#define SPRITE_TYPE_EXPLOSION 1
#define SPRITE_TYPE_FIREWORKSHOT 2

#define MAX_SPRITE_COUNT 16

extern "C" {
	typedef struct Player_s {
		char x;
		char cooldown;
	} Player;

	typedef struct Projectile_s {
		char x;
		char y;
		char type;
	} Projectile;

	typedef struct SpriteAnim_s {
		char x; 
		char y;
		char frame;
		char type;
		union {
			struct {
				char vx:4;
				char vy:4;
			};
		};
	} SpriteAnim;

	typedef struct MonsterRow_s {
		char x;
		char y;
		unsigned int isAliveBits;
	} MonsterRow;

	typedef struct Game_s {
		unsigned int frame;
		unsigned int parallax;
		Player player;
		unsigned char monsterAliveCount;
		MonsterRow monsterRows[MONSTER_ROWS];
		unsigned char monsterDanceDir;
		unsigned char monsterMoveTimeSlit;
		unsigned char monsterSpeed;
		unsigned char projectileCount;
		unsigned char killCount;
		unsigned char gameMode;
		unsigned char spriteCount;
		SpriteAnim sprites[MAX_SPRITE_COUNT];
		Projectile projectiles[MAX_PROJECTILE_COUNT];
	} Game;

	static Game game;

	static void tinyInvadersSetup() {
		_renderScreen.imageIncludes[TS_SKY0] = &_image_sky_background_opaque;
		_renderScreen.imageIncludes[TS_SPRITES] = &_image_tiny_invaders;
		_renderScreen.imageIncludes[TS_ROADLAYER] = &_image_roadlayer_opaque;
		_renderScreen.imageIncludes[TS_CITYLAYER] = &_image_citylayer;
		_renderScreen.imageIncludes[TS_CLOUDS] = &_image_clouds;
		memset(&game,0,sizeof(Game));
		game.player.x = MAX_LEFT + (MAX_RIGHT - MAX_LEFT) / 2;

		_renderScreen.flags |= RENDERSCREEN_FLAG_NOCLEAR|RENDERSCREEN_FLAG_CLEARBITMAP;
		_renderScreen.clearFill = TS_SKY0;


		unsigned int aliveBits;
		for (char i=0; i < MONSTER_COLS; i+=1) aliveBits|=1<<i;
		for (char i=0; i < MONSTER_ROWS; i+=1) {
			MonsterRow *row = &game.monsterRows[i];
			row->x = MAX_LEFT + MONSTER_WIDTH;
			row->y = i * (MONSTER_HEIGHT + MONSTER_SPACING) + 8;
			row->isAliveBits = aliveBits;
		}
		game.monsterMoveTimeSlit = 16;
		game.monsterSpeed = 1;
		game.gameMode = GAME_MODE_START;
 	}

 	static SpriteAnim* spawnSprite(char x, char y, char type, char frame) {
 		if (game.spriteCount >= MAX_SPRITE_COUNT) return NULL;
 		SpriteAnim *anim = &game.sprites[game.spriteCount++];
 		memset(anim,0,sizeof(SpriteAnim));
 		anim->x = x;
 		anim->y = y;
 		anim->type = type;
 		anim->frame = frame;

 		return anim;
 	}

 	static SpriteAnim* spawnFirework(char x, char y) {
 		SpriteAnim *anim = spawnSprite(x,y,SPRITE_TYPE_FIREWORKSHOT,cheapRnd()%8);
 		if (anim) {
 			anim->vx = cheapRnd()%4-1;
 			anim->vy = cheapRnd()%4+1;
 		}
 	}

 	static void stepSprites() {
 		//char *chr = StringBuffer_new();
 		//StringBuffer_amendDec(game.spriteCount);
 		//RenderScreen_drawText(0,0,0,chr,0xff);
 		for (char i=game.spriteCount-1;i>=0;i-=1) {
 			SpriteAnim *anim = &game.sprites[i];
 			char del = 0;
 			char drawFrame = 0;
 			char u,v,w,h;
 			switch (anim->type & 0xf) {
 				case SPRITE_TYPE_EXPLOSION: 
 					drawFrame = (anim->frame >> 1);
 					del = (drawFrame >= EXPLOSION_SPRITE_N);
 					u = EXPLOSION_SPRITE_U;
 					v = EXPLOSION_SPRITE_V;
 					w = EXPLOSION_SPRITE_W;
 					h = EXPLOSION_SPRITE_H;
 					anim->y -= anim->vy;
 					anim->x += anim->vx;
 					if (anim->frame % 2 == 0 && anim->vy > 1) {
 						anim->vy-=1;
 					}
 					
 					anim->frame+=1;
 					break;
 				case SPRITE_TYPE_FIREWORKSHOT:
 					anim->y -= anim->vy;
 					if (anim->frame % 2 == 0 && anim->vy > 1) {
 						anim->vy-=1;
 					}
 					anim->x += anim->vx;
 					anim->frame +=1;
 					u = FIREWORKS_SPRITE_U;
 					v = FIREWORKS_SPRITE_V;
 					w = FIREWORKS_SPRITE_W;
 					h = FIREWORKS_SPRITE_H;
 					drawFrame = FIREWORKS_SPRITE_N - (anim->frame >> 2);
 					del = (drawFrame < 0);
 					if (del) {
						SpriteAnim *e = spawnSprite(anim->x,anim->y,SPRITE_TYPE_EXPLOSION,0);
						if (e) {
							e->vx = anim->vx;
							e->vy = anim->vy;
						}
 					}
 					break;
 			}
 			if (!del) {
 				u += drawFrame * w;
 				RenderScreen_drawRectTexturedUV(anim->x - w/2, anim->y - h/2,w,h,TS_SPRITES,u,v);
 			} else {
 				game.sprites[i] = game.sprites[--game.spriteCount];
 			}

 		}
 	}
	static char shoot(char x, char y, char type) {
		if (game.projectileCount >= MAX_PROJECTILE_COUNT) return 0;
		Projectile *p = &game.projectiles[game.projectileCount++];
		p->x = x;
		p->y = y;
		p->type = type;
		return 1;
	}
	static void increaseDifficulty() {
		if (game.monsterMoveTimeSlit > 8) game.monsterMoveTimeSlit /= 2;
		else if (game.monsterMoveTimeSlit > 4) game.monsterMoveTimeSlit -= 2;
		else if (game.monsterMoveTimeSlit >= 2) game.monsterMoveTimeSlit -= 1; 
		else if (game.monsterSpeed < 16 && game.monsterAliveCount < 10) {
			game.monsterSpeed += 1;
		}
	}

	static void stepMonsters() {
		game.monsterAliveCount = 0;
		if (game.monsterDanceDir == 0) game.monsterDanceDir = 1;
		char minX = MAX_RIGHT;
		char maxX = MAX_LEFT;
		char maxY = 0;
		for (char i=0; i < MONSTER_ROWS; i+=1) {
			MonsterRow *row = &game.monsterRows[i];
			char x = row->x - MONSTER_WIDTH/2;
			char x1 = x + MONSTER_WIDTH;
			char y = row->y;
			char y1 = row->y+MONSTER_HEIGHT;
			for (char j = 0; j < MONSTER_COLS;j+=1) {
				if (row->isAliveBits & (1<<j)) {
					if (x < minX) minX = x;
					if (x1 > maxX) maxX = x1;
					if (y1 > maxY) maxY = y1;
					game.monsterAliveCount+=1;
					//RenderScreen_drawRect(x,row->y,5,5,0xff,RENDERCOMMAND_COLORED);
					RenderScreen_drawRectTexturedUV(
						x,y,
						MONSTER_WIDTH,MONSTER_HEIGHT,
						TS_SPRITES,
						MONSTER0_U + (MONSTER_WIDTH * ((game.frame >> 2)%2)), MONSTER0_V);
					for (char k = 0; k < game.projectileCount;k+=1) {
						Projectile *p = &game.projectiles[k];
						if (p->x >= x && p->y >= y && p->x < x1 && p->y < y1) {
							// kill monster & projectile
							row->isAliveBits &= ~(1<<j);
							game.projectiles[k] = game.projectiles[--game.projectileCount];
							game.killCount += 1;
							spawnSprite(x+MONSTER_WIDTH/2,y+MONSTER_HEIGHT/2,SPRITE_TYPE_EXPLOSION,0);
							if (game.killCount % 2 == 0)
								increaseDifficulty();
							break;
						}
					}
				}
				x += MONSTER_WIDTH + MONSTER_SPACING;
				x1 += MONSTER_WIDTH + MONSTER_SPACING;
				
			}
		}
		if (maxY > PLAYER_SHIP_Y) {
			game.gameMode = GAME_MODE_GAMEOVER;
			return;
		}
		if (game.frame % game.monsterMoveTimeSlit == 0) {
			char flip = 0;
			if (minX < MAX_LEFT) {
				flip = 1;
				game.monsterDanceDir = 1;
			}
			if (maxX > MAX_RIGHT) {
				flip = 1;
				game.monsterDanceDir = -1;
			}
			for (char i=0; i < MONSTER_ROWS; i+=1) {
				MonsterRow *row = &game.monsterRows[i];
				row->x += game.monsterDanceDir * ((game.frame % (game.monsterSpeed >> 1 | 1) != 0) + ((game.monsterSpeed >> 2) + 1));
				row->y += flip;
			}
			if (flip) increaseDifficulty();
		}
		if (game.monsterAliveCount == 0) {
			game.gameMode = GAME_MODE_WON;
			game.frame = 0;
		}
		//RenderScreen_drawRect(MAX_LEFT + (MAX_RIGHT - MAX_LEFT - MONSTER_ROW_WIDTH) / 2,5,
		//	MONSTER_ROW_WIDTH,2,0xff,RENDERCOMMAND_COLORED);

		//char *cnt = StringBuffer_new();
		//StringBuffer_amendDec(game.monsterAliveCount);
		//RenderScreen_drawText (0, 9, 0, cnt, 0xff);
	}

	static void stepProjectiles() {
		for (char i = game.projectileCount - 1; i>=0;i-=1) {
			Projectile *p = &game.projectiles[i];
			char isOut = 0;
			switch (p->type) {
				case PROJECTILE_TYPE_PLAYER:
					p->y -= 3;
					isOut = p->y <= 0;
					break;
				case PROJECTILE_TYPE_ENEMY:
					p->y += 1;
					isOut = p->y >= 64;
					break;
			}

			RenderScreen_drawRectTexturedUV(
				p->x - (PROJECTILE_W/2),
				p->y - (PROJECTILE_H/2),
				PROJECTILE_W,PROJECTILE_H,
				TS_SPRITES,
				PROJECTILE_U + ((game.frame+i)&1) * PROJECTILE_W,PROJECTILE_V);
			if (isOut) {
				game.projectiles[i] = game.projectiles[--game.projectileCount];
			}
		}
	}
	static void stepPlayer() {
		char cmd = (rightStick.normX >> 2) + (leftStick.normX >> 2);
		if (cmd > 0 && game.player.x < MAX_RIGHT) game.player.x+=1;
		if (cmd < 0 && game.player.x > MAX_LEFT) game.player.x-=1;
		if ((leftButton || rightButton) && game.player.cooldown == 0 
				&& shoot(game.player.x,PLAYER_SHIP_Y,PROJECTILE_TYPE_PLAYER)) {
			game.player.cooldown = PLAYER_SHOOT_COOLDOWN;

		}

		game.player.cooldown -= (game.player.cooldown > 0);

		RenderScreen_drawRectTexturedUV(game.player.x - (PLAYER_SHIP_SIZE_X/2),
			PLAYER_SHIP_Y,PLAYER_SHIP_SIZE_X,PLAYER_SHIP_SIZE_Y,TS_SPRITES,0,0);
	}
	static void stepBackground(char moving) {
		game.frame += 1;
		game.parallax += moving ? 1 : 0;
		_renderScreen.clearFillOffsetX = game.parallax>>4;
		if (game.frame % 4 == 0) {
			if (!moving) {
				if (_renderScreen.clearFillOffsetY > 0) 
					_renderScreen.clearFillOffsetY-=1;
			} else {
				if (_renderScreen.clearFillOffsetY < 64) 
					_renderScreen.clearFillOffsetY+=1;
			}
		}
		RenderScreen_drawRectTexturedUV(0,63-16-6,96,16,TS_CLOUDS,(game.parallax>>3)%_image_clouds.width,0);	
		RenderScreen_drawRectTexturedUV(0,63-16,96,16,TS_CITYLAYER,(game.parallax>>2)%_image_citylayer.width,0);	
		RenderScreen_drawRectTexturedUV(0,63,96,1,TS_ROADLAYER,(game.parallax)%32,0);	
	}


	static void gamePlayLoop() {
		stepBackground(0);
		//RenderScreen_drawRectTexturedUV(0,0, 96,64,TS_SKY0,0,0);
		stepPlayer();
		stepMonsters();
		stepProjectiles();
		stepSprites();
	}

	static void gameStartLoop() {
		stepBackground(1);
		if (leftButton == 1 || rightButton == 1) {
			tinyInvadersSetup();
			game.gameMode = GAME_MODE_PLAY;
		}

		stepSprites();
		char *str = StringBuffer_new();
		StringBuffer_amendLoad(_string_press_button);
		if ((game.frame>>4)%2 == 0)
			RenderScreen_drawText(13,40,0,str,0xff);
	}

	static void gameOverLoop() {
		stepBackground(1);
		stepMonsters();
		stepSprites();
		char *str = StringBuffer_new();
		StringBuffer_amendLoad(_string_gameover);
		RenderScreen_drawText(20,10,0,str,0xff);
		if (leftButton == 1 || rightButton == 1) {
			game.gameMode = GAME_MODE_START;
		}	
	}
	static void gameWonLoop() {
		stepBackground(1);
		stepPlayer();
		stepSprites();
		if (cheapRnd() > (unsigned short)55000)
			spawnFirework(16 + cheapRnd() % 64,52 - cheapRnd()%8);

		char *str = StringBuffer_new();
		StringBuffer_amendLoad(_string_gamewon);
		if ((game.frame>>3)%2 == 0)
			RenderScreen_drawText(23,40,0,str,0xff);
		if ((leftButton == 1 || rightButton == 1) && game.frame > 30) {
			game.gameMode = GAME_MODE_START;
		}	
	}

	static void tinyInvadersLoop() {
		static char init = 0;
		if (!init) {
			init = 1;
			_renderScreen.clearFillOffsetY = 64;
			tinyInvadersSetup();
		}
		switch (game.gameMode) {
			case GAME_MODE_START: gameStartLoop(); break;
			case GAME_MODE_PLAY: gamePlayLoop(); break;
			case GAME_MODE_GAMEOVER: gameOverLoop(); break;
			case GAME_MODE_WON: gameWonLoop(); break;
		}
	}
	
}
