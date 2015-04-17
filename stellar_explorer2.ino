
#include <avr/pgmspace.h>
#include <Wire.h>
#include "util.h"
#include "joystick.h"
#include "SPI_C.h"
#include "TinyScreenC.h"
#include "image.h"
#include "render.h"
#include "images_test.h"
#include "images_ball.h"
#include "images_grasstile.h"
#include "images_ground_mud.h"
#include "images_tileset.h"
unsigned char brightness = 8;
void setup() {
  Wire.begin();
  TinyScreenC();

  TinyScreenC_begin();
  TinyScreenC_setBitDepth(0);
  TinyScreenC_setBrightness(brightness);
  _renderScreen.fontFormats[0] = &virtualDJ_5ptFontInfo;
  _renderScreen.imageIncludes[0] = &_image_tileset_opaque;
  _renderScreen.imageIncludes[1] = &_image_tileset;
  _renderScreen.imageIncludes[2] = &_image_ball;
  //SPI_setClockDivider(SPI_CLOCK_DIV2); -- no effect?
}

void handleBrightness() {
  unsigned char b = TinyScreenC_getButtons();
 // if (b&1) 
}

void loopRects() {
  for (int r = 0; r < 4;r+=1) {
    for (int g = 0; g < 8; g+=1) {
      for (int b = 0; b<8;b+=1) {
        int col = r|(g<<2)|(b<<5);
        TinyScreenC_drawRectC((r+b*4)*4,g*4,4,3, 1, col);
        TinyScreenC_drawRectC((r+b*4)*4,g*4+3,3,1, 1, col);
      }
    }
    
  }
}
static const char simpleTownMapData[] PROGMEM = {
  0x10,0x11,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x11,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x10,0x11,0x09, 0x10,0x10,0x22,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x10,0x10,0x09, 0x05,0x00,0x04,0x02, 0x06,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  
  0x10,0x10,0x10,0x07, 0x08,0x08,0x08,0x08, 0x08,0x08,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
  0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10,
};
static const char simpleTownMapDataLayer[] PROGMEM = {
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0x20,0x21,0xff,0x28, 0x29,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0x18,0x13,0xff,0xff, 0xff,0xff,0xff,0xff,
  
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x16, 0x17,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0x16,0x17,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
};
static unsigned int simpleTownMapPosX;
static unsigned int simpleTownMapPosY;
int loopSimpleTownMoveTest() {
  _renderScreen.flags|=RENDERSCREEN_FLAG_NOCLEAR;
  int n = 0;
  _renderScreen.imageIncludes[0] = &_image_tileset_opaque;
  _renderScreen.imageIncludes[1] = &_image_tileset;
  
  simpleTownMapPosX += rightStick.normX >>4;
  simpleTownMapPosY += rightStick.normY >>4;
  char screenX = -((simpleTownMapPosX >> 4)&0xf);
  char screenY = -((simpleTownMapPosY >> 4)&0xf);
  char limy = micros()/500000%6;
  char limx = micros()/500000%9;
  for (char y = 0;y<5;y+=1) {
    unsigned char mapY = ((simpleTownMapPosY >> 8) + y)&0x7;
    for (char x = 0;x<7;x+=1) {
      unsigned char mapX = ((simpleTownMapPosX >> 8) + x)&0xf;
      unsigned char mapPos = (mapY << 4) + mapX;
      unsigned char uv = pgm_read_byte(&simpleTownMapData[mapPos]);
      unsigned char v = uv & 0xf0;
      unsigned char u = (uv & 0xf)<<4;
      RenderScreen_drawRectTexturedUV(screenX + x*16,screenY + y*16,16,16,0,u,v);
      uv = pgm_read_byte(&simpleTownMapDataLayer[mapPos]);
      if (uv != 0xff) {
        v = uv & 0xf0;
        u = (uv & 0xf)<<4;
        RenderScreen_drawRectTexturedUV(screenX + x*16,screenY + y*16,16,16,1,u,v);
          
      }
      n++;
    }
    
  }
  return n;
}

void loop() {
  static unsigned char msLast = 0;
  unsigned long start = micros();
  UpdateJoystick();
  loopSimpleTownMoveTest();
  //loopRects();
  /*RenderScreen_drawRectTexturedUV(-16,-16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(0,  -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(16, -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(32, -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(48, -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(64, -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(80, -16,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(-16,0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(0,  0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(16, 0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(32, 0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(48, 0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(64, 0,16,16,0,0,16);
  RenderScreen_drawRectTexturedUV(80, 0,16,16,0,0,16);*/
  // put your main code here, to run repeatedly:
  /*int coffset = micros()/10000L;
  //cheapRndSeed(coffset>>8,coffset&0xff00);

  //RenderScreen_drawRectTextured(-50 + (coffset+100) % 200,8,46,48,0);
  //RenderScreen_drawRectTextured(30,-50 + coffset % 200,46,48,0);
  //RenderScreen_drawRectTextured(-133+ sin(micros()/1000L * 0.001f)*30,-100+ cos(micros()/1000L * 0.001f)*30,640,640,0);
  for (int x=0;x<6;x+=1) {
    for (int y=0;y<4;y+=1)  {
      int t = (x * 17 - y * 5 + (x^(y>>1)^(y<<1)));
      RenderScreen_drawRectTexturedUV(x*16,y*16,16,16,t&1,0,16);
    }
  }
  for (int x=0;x<96;x+=8) {
    //RenderScreen_drawRect (x,15 + sin(micros()/1000L * 0.001f+x*0.1f)*10 , 4,4,(x+coffset)>>3);
    RenderScreen_drawRectTextured (x,15 + sin(micros()/1000L * 0.006f+x*0.1f)*10 , 8,8,2);
    //RenderScreen_drawRect (x,45 + sin(-micros()/1000L * 0.01f+x*0.1f)*10 , 6,6,(-x+coffset)>>3);
  }*/
  RenderScreen_drawText (0, 0, 0, StringBuffer_buffer(String((int)msLast)+"ms"), 0xff);
  RenderScreen_flush();
  StringBuffer_reset();
  msLast = (micros()-start) / 1000;
  /*	long start =micros();
  	TinyScreenC_drawLineC(cheapRnd(),cheapRnd(),cheapRnd(),cheapRnd(),cheapRnd());
  	long t = micros()-start;
  	TinyScreenC_setCursor(0,0);
  	TinyScreenC_setFont(virtualDJ_5ptFontInfo);
  	TinyScreenC_fontColor(0,0xff);
  	char b[12];*/
  //TinyScreenC_printCStr(int_to_dec(t,b));
}
