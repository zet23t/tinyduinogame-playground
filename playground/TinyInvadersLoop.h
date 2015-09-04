#include "images_tiny_invaders.h"
#define SHOW_FPS 1
#define LOOP_PROG tinyInvadersLoop
#define TILESET_SKY0 0
#define TS_SPRITES 1
extern "C" {

	static void tinyInvadersSetup() {
		_renderScreen.imageIncludes[TILESET_SKY0] = &_image_sky_background_opaque;
		_renderScreen.imageIncludes[TS_SPRITES] = &_image_tiny_invaders;
	}

	static void tinyInvadersLoop() {
		static char init = 0;
		if (!init) {
			init = 1;
			tinyInvadersSetup();
		}
		RenderScreen_drawRectTexturedUV(0,0, 96,64,TILESET_SKY0,0,0);
		RenderScreen_drawRectTexturedUV(10,50,5,5,TS_SPRITES,0,0);
	}
	
}
