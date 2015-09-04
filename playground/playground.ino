
#include <avr/pgmspace.h>
#include <SPI.h>
#include <Wire.h>
#include "util.h"
#include "joystick.h"
#include "SPI_C.h"
#include "TinyScreenC.h"
#include "image.h"
#include "render.h"

#include "strings.h"

#include "test_loops.h"

unsigned char brightness = 8;
void setup() {
  Wire.begin();
  TinyScreenC();

  TinyScreenC_begin();
  TinyScreenC_setBitDepth(0);
  TinyScreenC_setBrightness(brightness);
  _renderScreen.fontFormats[0] = &virtualDJ_5ptFontInfo;
  //SPI_setClockDivider(SPI_CLOCK_DIV2); -- no effect?
}
void handleBrightness() {
  static char previous = 0,showTimeout = 0;
  unsigned char b = TinyScreenC_getButtons();
  if ((b&9) && !previous) {
    if ((b&8) && brightness < 15) brightness+=1;
    if ((b&1) && brightness > 0) brightness-=1;
    TinyScreenC_setBrightness(brightness);
    showTimeout = 40;
  }
  if (showTimeout > 0) {
    showTimeout -= 1;
    char *text = StringBuffer_load(_string_brightness);
    StringBuffer_amendDec(brightness*100/15);
    StringBuffer_amendLoad(_string_percent);
    RenderScreen_drawText (10, 29, 0, text, 0);
    RenderScreen_drawText (9, 28, 0, text, 0xff);
  }
  previous = b;
}



void loop() {
  static unsigned char msLast = 0;

  unsigned long start = micros();
  
  UpdateJoystick();
  LOOP_PROG();
  handleBrightness();
  #ifdef SHOW_FPS
  char *fps = StringBuffer_new();
  StringBuffer_amendDec(msLast);
  StringBuffer_amendLoad(_string_ms);
  RenderScreen_drawText (0, 0, 0, fps, 0xff);
  #endif
  RenderScreen_flush();
  StringBuffer_reset();
  msLast = (micros()-start) / 1000;
  // cap to 20fps
  while (micros() - start < 50000);
}
