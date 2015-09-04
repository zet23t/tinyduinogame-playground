#include "images_tiny_invaders.h"
//#define SHOW_FPS 0
#define LOOP_PROG tinyInvadersLoop
#define TILESET_SKY0 0
#define TS_SPRITES 1
#define PLAYER_SHIP_Y 56
#define PLAYER_SHIP_SIZE_X 7
#define PLAYER_SHIP_SIZE_Y 7
#define PLAYER_SHOOT_COOLDOWN 10

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

	typedef struct MonsterRow_s {
		char x;
		char y;
		unsigned int isAliveBits;
	} MonsterRow;

	typedef struct Game_s {
		unsigned frame;
		Player player;
		unsigned char monsterAliveCount;
		MonsterRow monsterRows[MONSTER_ROWS];
		unsigned char monsterDanceDir;
		unsigned char monsterSpeed;
		unsigned char projectileCount;
		unsigned char killCount;
		unsigned char gameMode;
		Projectile projectiles[MAX_PROJECTILE_COUNT];
	} Game;

	static Game game;

	static void tinyInvadersSetup() {
		_renderScreen.imageIncludes[TILESET_SKY0] = &_image_sky_background_opaque;
		_renderScreen.imageIncludes[TS_SPRITES] = &_image_tiny_invaders;
		memset(&game,0,sizeof(Game));
		game.player.x = MAX_LEFT + (MAX_RIGHT - MAX_LEFT) / 2;

		_renderScreen.flags |= RENDERSCREEN_FLAG_NOCLEAR|RENDERSCREEN_FLAG_CLEARBITMAP;
		_renderScreen.clearFill = TILESET_SKY0;


		unsigned int aliveBits;
		for (char i=0; i < MONSTER_COLS; i+=1) aliveBits|=1<<i;
		for (char i=0; i < MONSTER_ROWS; i+=1) {
			MonsterRow *row = &game.monsterRows[i];
			row->x = MAX_LEFT + MONSTER_WIDTH;
			row->y = i * (MONSTER_HEIGHT + MONSTER_SPACING) + 8;
			row->isAliveBits = aliveBits;
		}
		game.monsterSpeed = 16;
		game.gameMode = GAME_MODE_START;
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
		if (game.monsterSpeed > 8) game.monsterSpeed /= 2;
		else if (game.monsterSpeed > 4) game.monsterSpeed -= 2;
		else if (game.monsterSpeed >= 2) game.monsterSpeed -= 1; 
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
		if (game.frame % game.monsterSpeed == 0) {
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
				row->x += game.monsterDanceDir;
				row->y += flip;
			}
			if (flip) increaseDifficulty();
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

	static void gamePlayLoop() {
		game.frame += 1;
		//RenderScreen_drawRectTexturedUV(0,0, 96,64,TILESET_SKY0,0,0);
		stepPlayer();
		stepMonsters();
		stepProjectiles();
	}

	static void gameStartLoop() {
		game.frame += 1;
		_renderScreen.clearFillOffsetX += ((game.frame % 8) == 0);
		if (leftButton == 1 || rightButton == 1) {
			tinyInvadersSetup();
			game.gameMode = GAME_MODE_PLAY;
		}

		char *str = StringBuffer_new();
		StringBuffer_amendLoad(_string_press_button);
		if ((game.frame>>4)%2 == 0)
			RenderScreen_drawText(13,40,0,str,0xff);
	}

	static void gameOverLoop() {
		game.frame += 1;
		_renderScreen.clearFillOffsetX += ((game.frame % 8) == 0);
		if (leftButton == 1 || rightButton == 1) {
			game.gameMode = GAME_MODE_START;
		}	
		stepMonsters();
		char *str = StringBuffer_new();
		StringBuffer_amendLoad(_string_gameover);
		RenderScreen_drawText(20,10,0,str,0xff);
	}

	static void tinyInvadersLoop() {
		static char init = 0;
		if (!init) {
			init = 1;
			tinyInvadersSetup();
		}
		switch (game.gameMode) {
			case GAME_MODE_START: gameStartLoop(); break;
			case GAME_MODE_PLAY: gamePlayLoop(); break;
			case GAME_MODE_GAMEOVER: gameOverLoop(); break;
		}
	}
	
}
