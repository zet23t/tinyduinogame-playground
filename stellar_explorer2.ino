#include <Wire.h>
#include "util.h"
#include "SPI_C.h"
#include "TinyScreenC.h"
#include "image.h"
#include "render.h"
#include "images_test.h"
#include "images_ball.h"
#include "images_grasstile.h"

void setup() {
  Wire.begin();
  TinyScreenC();

  TinyScreenC_begin();

  TinyScreenC_setBrightness(2);
  _renderScreen.fontFormats[0] = &virtualDJ_5ptFontInfo;
  _renderScreen.imageIncludes[0] = &_image_grasstile;
  _renderScreen.imageIncludes[1] = &_image_ball;
  _renderScreen.flags|=RENDERSCREEN_FLAG_NOCLEAR;
}

void loop() {
  static unsigned char msLast = 0;
  unsigned long start = micros();
  // put your main code here, to run repeatedly:
  int coffset = micros()/10000L;
  //cheapRndSeed(coffset>>8,coffset&0xff00);

  /*RenderScreen_drawCircle (10,40,(1+(cos(micros()/1000L * 0.002f)+1)*8)*18,0xff);
  RenderScreen_drawCircle (18,40,1<<4,0xff);
  RenderScreen_drawCircle (30,40,5<<4,0xff);
  RenderScreen_drawCircle (60,40,(1+(sin(micros()/1000L * 0.002f)+1)*8)*32,0xff);*/
  //RenderScreen_drawRectTextured(-50 + (coffset+100) % 200,8,46,48,0);
  //RenderScreen_drawRectTextured(30,-50 + coffset % 200,46,48,0);
  RenderScreen_drawRectTextured(-133+ sin(micros()/1000L * 0.001f)*30,-100+ cos(micros()/1000L * 0.001f)*30,640,640,0);
  for (int x=0;x<96;x+=5) {
    //RenderScreen_drawRect (x,15 + sin(micros()/1000L * 0.001f+x*0.1f)*10 , 4,4,(x+coffset)>>3);
    //RenderScreen_drawRectTextured (x,15 + sin(micros()/1000L * 0.006f+x*0.1f)*10 , 8,8,1);
  //  RenderScreen_drawRect (x,45 + sin(-micros()/1000L * 0.01f+x*0.1f)*10 , 6,6,(-x+coffset)>>3);
  }

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
