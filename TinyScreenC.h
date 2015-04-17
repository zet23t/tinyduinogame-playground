/*
This is a copy of TinyScreen.h and TinyScreen.cpp but adapted to use C. I did
so to check how much flash memory it would use after reaching a limit in my 
project. 


TinyScreen.h - Last modified 2 March 2015

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Written by Ben Rose for TinyCircuits.

The latest version of this library can be found at https://tiny-circuits.com/
*/

#include "Arduino.h"

extern "C" {
    
  // GPIO Pins
  const uint8_t GPIO_DC = 0x01;
  const uint8_t GPIO_CS = 0x02;
  const uint8_t GPIO_RES = 0x08;
  const uint8_t GPIO_BTN1 = 0x10;
  const uint8_t GPIO_BTN2 = 0x20;
  const uint8_t GPIO_BTN3 = 0x40;
  const uint8_t GPIO_BTN4 = 0x80;
  const uint8_t GPIO_CMD_START = ~(GPIO_CS|GPIO_DC);
  const uint8_t GPIO_DATA_START = ~GPIO_CS;
  const uint8_t GPIO_TRANSFER_END = GPIO_CS;
  
  //GPIO Registers
  const uint8_t GPIO_RegData = 0x00;
  const uint8_t GPIO_RegDir = 0x01;
  const uint8_t GPIO_RegPullUp = 0x02;
  
  const uint8_t GPIO_ADDR = 0x20;
  
  typedef struct
  {
    const uint8_t width;
    const uint16_t offset;
    
  } FONT_CHAR_INFO;  
  
  typedef struct
  {
    const unsigned char height;
    const char startCh;
    const char endCh;
    const FONT_CHAR_INFO*  charDesc;
    const unsigned char* bitmap;
      
  } FONT_INFO;  
  
  #include <avr/pgmspace.h>
  #include "font.h"
  
  static void TinyScreenC(void);
  static void TinyScreenC_startData(void);
  static void TinyScreenC_startCommand(void);
  static void TinyScreenC_endTransfer(void);
  static void TinyScreenC_begin(void);
  static void TinyScreenC_on(void);
  static void TinyScreenC_off(void);
  static void TinyScreenC_setFlip(uint8_t);
  static void TinyScreenC_setMirror(uint8_t);
  static void TinyScreenC_setBitDepth(uint8_t);
  static void TinyScreenC_setBrightness(uint8_t);
  static void TinyScreenC_writeRemap(void);
  //accelerated drawing commands
  static void TinyScreenC_drawPixel(uint8_t, uint8_t, uint16_t);
  static void TinyScreenC_drawLineC(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  static void TinyScreenC_drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  static void TinyScreenC_drawRectC(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  static void TinyScreenC_drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  static void TinyScreenC_clearWindow(uint8_t, uint8_t, uint8_t, uint8_t);
  //basic graphics commands
  static void TinyScreenC_writePixel(uint16_t);
  static void TinyScreenC_writeBuffer(uint8_t *, int);
  static void TinyScreenC_setX(uint8_t, uint8_t);
  static void TinyScreenC_setY(uint8_t, uint8_t);
  static void TinyScreenC_goTo(uint8_t x, uint8_t y);
  //I2C GPIO related
  uint8_t TinyScreenC_getButtons(void);
  static void TinyScreenC_writeGPIO(uint8_t, uint8_t);
  //font
  static void TinyScreenC_setFont(const FONT_INFO&);
  static void TinyScreenC_setCursor(uint8_t, uint8_t);
  static void TinyScreenC_fontColor(uint8_t, uint8_t);
  size_t TinyScreenC_write(uint8_t);
  
  #define xMax 95
  #define yMax 63
  
  uint8_t _addr, _cursorX, _cursorY, _fontHeight, _fontFirstCh, _fontLastCh, _fontColor, _fontBGcolor, _bitDepth, _flipDisplay, _mirrorDisplay;
  const FONT_CHAR_INFO* _fontDescriptor;
  const unsigned char* _fontBitmap;
  
  
  /*
  TinyScreen.cpp - Last modified 2 March 2015
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Written by Ben Rose for TinyCircuits.
  
  The latest version of this library can be found at https://tiny-circuits.com/
  */
  
  #include <avr/pgmspace.h>
  #include "pins_arduino.h"
  #include "wiring_private.h"
  
  #include <Wire.h>
  
  /*
  TinyScreen uses an I2C GPIO chip to interface with the OLED control lines and buttons
  writeGPIO(address, data);//write to SX1505
  startCommand();//write SSD1331 chip select active with data/command signalling a command
  TinyScreenC_startData();//write SSD1331 chip select active with data/command signalling data
  TinyScreenC_endTransfer();//write SSD1331 chip select inactive
  getButtons();//read button states, return as four LSBs in a byte
  */
  
  static void TinyScreenC_writeGPIO(uint8_t regAddr, uint8_t regData)
  {
    uint8_t oldTWBR=TWBR;
    TWBR=0;
    Wire.beginTransmission(GPIO_ADDR+_addr);
    Wire.write(regAddr); 
    Wire.write(regData);
    Wire.endTransmission();
    TWBR=oldTWBR;
  }
  
  
  static void TinyScreenC_startCommand(void) {
    TinyScreenC_writeGPIO(GPIO_RegData,GPIO_CMD_START);
  }
  
  static void TinyScreenC_startData(void) {
    TinyScreenC_writeGPIO(GPIO_RegData,GPIO_DATA_START);
  }
  
  static void TinyScreenC_endTransfer(void) {
    TinyScreenC_writeGPIO(GPIO_RegData,GPIO_TRANSFER_END);
  }
  
  uint8_t TinyScreenC_getButtons(void) {
    Wire.beginTransmission(0x20);
    Wire.write(GPIO_RegData);
    Wire.endTransmission();
    Wire.requestFrom(0x20,1);
    uint8_t b=Wire.read();
    //buttons are active low and MSBs, so flip and shift
    return ((~b)>>4)&0x0F;
  }
  
  /*
  SSD1331 Basics
  goTo(x,y);//set OLED RAM to pixel address (x,y) with wrap around at x and y max
  setX(x start, x end);//set OLED RAM to x start, wrap around at x end
  setY(y start, y end);//set OLED RAM to y start, wrap around at y end
  */
  
  static void TinyScreenC_goTo(uint8_t x, uint8_t y) {
    if(x>xMax||y>yMax)return;
    TinyScreenC_setX(x,xMax);
    TinyScreenC_setY(y,yMax);
  }
  
  static void TinyScreenC_setX(uint8_t x, uint8_t end) {
    if(x>xMax)x=xMax;
    if(end>xMax)end=xMax;
    TinyScreenC_startCommand();
    SPI_transfer(0x15);//set column
    SPI_transfer(x);
    SPI_transfer(end);
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_setY(uint8_t y, uint8_t end) {
    if(y>yMax)y=yMax;
    if(end>yMax)end=yMax;
    TinyScreenC_startCommand();
    SPI_transfer(0x75);//set row
    SPI_transfer(y);
    SPI_transfer(end);
    TinyScreenC_endTransfer();
  }
  
  /*
  Hardware accelerated drawing functions:
  tinysCreenc_clearWindow(x start, y start, width, height);//clears specified OLED controller memory
  tinyScreenC_drawRect(x stary, y start, width, height, fill, 8bitcolor);//sets specified OLED controller memory to an 8 bit color, fill is a boolean
  tinyScreenC_drawRect(x stary, y start, width, height, fill, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
  drawLine(x1, y1, x2, y2, 8bitcolor);//draw a line from (x1,y1) to (x2,y2) with an 8 bit color
  drawLine(x1, y1, x2, y2, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
  */
  
  static void TinyScreenC_clearWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {  
    if(x>xMax||y>yMax)return;
    uint8_t x2=x+w-1;
    uint8_t y2=y+h-1;
    if(x2>xMax)x2=xMax;
    if(y2>yMax)y2=yMax;
    
    TinyScreenC_startCommand();
    SPI_transfer(0x25);//clear window
    SPI_transfer(x);SPI_transfer(y);
    SPI_transfer(x2);SPI_transfer(y2);
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_drawRectC(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t f, uint8_t color) 
  {
    uint8_t r=(color&0x03)<<4;//two bits
    uint8_t g=(color&0x1C)<<1;//three bits
    uint8_t b=(color&0xE0)>>2;//three bits
    if(r&0x10)r|=0x0F;//carry lsb
    if(g&0x08)g|=0x07;//carry lsb
    if(b&0x08)b|=0x07;//carry lsb
    TinyScreenC_drawRect(x,y,w,h,f,r,g,b);
  }
  
  static void TinyScreenC_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t f, uint8_t r, uint8_t g, uint8_t b) 
  {
    if(x>xMax||y>yMax)return;
    uint8_t x2=x+w-1;
    uint8_t y2=y+h-1;
    if(x2>xMax)x2=xMax;
    if(y2>yMax)y2=yMax;
    
    uint8_t fill=0;
    if(f)fill=1;
    
    TinyScreenC_startCommand();
    SPI_transfer(0x26);//set fill
    SPI_transfer(fill);
    
    SPI_transfer(0x22);//draw rectangle
    SPI_transfer(x);SPI_transfer(y);
    SPI_transfer(x2);SPI_transfer(y2);
    //outline
    SPI_transfer(b);SPI_transfer(g);SPI_transfer(r);
    //fill
    SPI_transfer(b);SPI_transfer(g);SPI_transfer(r);
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_drawLineC(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    uint8_t r=(color&0x03)<<4;//two bits
    uint8_t g=(color&0x1C)<<1;//three bits
    uint8_t b=(color&0xE0)>>2;//three bits
    if(r&0x10)r|=0x0F;//carry lsb
    if(g&0x08)g|=0x07;//carry lsb
    if(b&0x08)b|=0x07;//carry lsb
    TinyScreenC_drawLine(x0,y0,x1,y1,r,g,b);
  }
  
  static void TinyScreenC_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {  
    if(x0>xMax)x0=xMax;
    if(y0>yMax)y0=yMax;
    if(x1>xMax)x1=xMax;
    if(y1>yMax)y1=yMax;
    TinyScreenC_startCommand();
    SPI_transfer(0x21);//draw line
    SPI_transfer(x0);SPI_transfer(y0);
    SPI_transfer(x1);SPI_transfer(y1);
    SPI_transfer(b);SPI_transfer(g);SPI_transfer(r);
    TinyScreenC_endTransfer();
  }
  
  /*
  Pixel manipulation
  drawPixel(x,y,color);//set pixel (x,y) to specified color. This is slow because we need to send commands setting the x and y, then send the pixel data.
  tinysCreenC_writePixel(color);//write the current pixel to specified color. Less slow than drawPixel, but still has to ready display for pixel data
  writeBuffer(buffer,count);//optimized write of a large buffer of 8 bit data. Must be wrapped with TinyScreenC_startData() and TinyScreenC_endTransfer(), but there can be any amount of calls to writeBuffer between.
  */
  
  static void TinyScreenC_drawPixel(uint8_t x, uint8_t y, uint16_t color)
  {
    if(x>xMax||y>yMax)return;
    TinyScreenC_goTo(x,y);
    TinyScreenC_writePixel(color);
  }
  
  static void TinyScreenC_writePixel(uint16_t color) {
    TinyScreenC_startData();
    if(_bitDepth)
      SPI_transfer(color>>8);
    SPI_transfer(color);
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_writeBuffer(uint8_t *buffer,int count) {
   /* if (_bitDepth) {
      for(int j=0;j<count;j++) {
        uint8_t color = buffer[j];
        uint8_t g=(color>>2)&7;//three bits
        uint8_t b= color>>5;//three bits
        g|= g<<3;
        b = b<<2 | b >> 1;
        // r: 5bits, g: 6bits, b:5bits
        uint16_t temp = g<<5|b<<11;
        SPDR=temp>>8;
        uint8_t r=(color&0x03);//two bits
        r = r<<1 | r << 3;

        // ignore waiting since we spend enough cycles doing needed calculations anyway
        //while (!(SPSR & _BV(SPIF)));
        SPDR=temp|r;
      }
      while (!(SPSR & _BV(SPIF)));
    } else */
    {
      uint8_t temp = buffer[0];
      uint8_t j = 0;
      while(1) {
        SPDR = temp;
        if (++j == count) break;
        temp = buffer[j];
        // sync using just enough NOP instructions
        __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"); 
        //while (!(SPSR & _BV(SPIF)));
      };
      
      /*SPDR = buffer[0];
      for(int j=1;j<count;j++){
        temp=buffer[j];
        while (!(SPSR & _BV(SPIF)));
        SPDR=temp;
      }
      while (!(SPSR & _BV(SPIF)));*/
    }
  }
  static void TinyScreenC_waitForBuffer() {
    while (!(SPSR & _BV(SPIF)));
  }
  
  /* 
  TinyScreen commands
  TinyScreenC_setBrightness(brightness);//sets main current level, valid levels are 0-15
  tinyscreenc_on();//turns display tinyscreenc_on
  off();//turns display off, uses less power
  setBitDepth(depth);//boolean- 0 is 8 bit, 1 is 16 bit
  setFlip(flip);//done in hardware tinyscreenc_on the SSD1331. boolean- 0 is normal, 1 is upside down
  setMirror(mirror);//done in hardware tinyscreenc_on the SSD1331. boolean- 0 is normal, 1 is mirrored across Y axis
  */
  
  static void TinyScreenC_setBrightness(uint8_t brightness) {
    if(brightness>15)brightness=15;  
    TinyScreenC_startCommand();
    SPI_transfer(0x87);//set master current
    SPI_transfer(brightness);
    TinyScreenC_endTransfer();
  }
  static void TinyScreenC_on(void) {
    TinyScreenC_startCommand();
    SPI_transfer(0xAF);//display tinyscreenc_on
    TinyScreenC_endTransfer();
  }
  static void TinyScreenC_off(void) {
    TinyScreenC_startCommand();
    SPI_transfer(0xAE);//display off
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_setBitDepth(uint8_t b){
    _bitDepth=b;
    TinyScreenC_writeRemap();
  }
  
  static void TinyScreenC_setFlip(uint8_t f){
    _flipDisplay=f;
    TinyScreenC_writeRemap();
  }
  
  static void TinyScreenC_setMirror(uint8_t m){
    _mirrorDisplay=m;
    TinyScreenC_writeRemap();
  }
  
  /*
  The SSD1331 remap command sets a lot of driver variables, these are kept in memory
  and are all written when a change is made.
  */
  
  static void TinyScreenC_writeRemap(void){
    uint8_t remap=(1<<5)|(1<<2);
    if(_flipDisplay)
      remap|=((1<<4)|(1<<1));
    if(_mirrorDisplay)
      remap^=(1<<1);
    if(_bitDepth)
      remap|=(1<<6);
    TinyScreenC_startCommand();
    SPI_transfer(0xA0);//set remap
    SPI_transfer(remap);
    TinyScreenC_endTransfer();
  }
  
  static void TinyScreenC_begin(void) {
    //setup GPIO, reset SSD1331
    TinyScreenC_writeGPIO(GPIO_RegData,~GPIO_RES);//reset low, CS/other pins high
    TinyScreenC_writeGPIO(GPIO_RegDir,~GPIO_RES);//set reset to output
    delay(5);
    TinyScreenC_writeGPIO(GPIO_RegDir,~(GPIO_CS|GPIO_DC));//reset to input, CS/DC output
    TinyScreenC_writeGPIO(GPIO_RegPullUp,GPIO_BTN1|GPIO_BTN2|GPIO_BTN3|GPIO_BTN4);//button pullup enable
    //init SPI
    SPI_begin();
    SPI_setDataMode(SPI_MODE0);//wrong mode, works because we're only writing. this mode is compatible with SD cards.
    SPI_setClockDivider(SPI_CLOCK_DIV2);
    //datasheet SSD1331 init sequence
    uint8_t init[32]={0xAE, 0xA1, 0x00, 0xA2, 0x00, 0xA4, 0xA8, 0x3F,
    0xAD, 0x8E, 0xB0, 0x0B, 0xB1, 0x31, 0xB3, 0xF0, 0x8A, 0x64, 0x8B,
    0x78, 0x8C, 0x64, 0xBB, 0x3A, 0xBE, 0x3E, 0x81, 0x91, 0x82, 0x50, 0x83, 0x7D};
    TinyScreenC_off();
    TinyScreenC_startCommand();
    for(uint8_t i=0;i<32;i++)
      SPI_transfer(init[i]);
    TinyScreenC_endTransfer();
    //use libarary functions for remaining init
    TinyScreenC_setBrightness(5);
    TinyScreenC_writeRemap();
    TinyScreenC_clearWindow(0,0,96,64);
    delay(2);
    TinyScreenC_on();
  }
  
  /*
  TinyScreen constructor
  address sets I2C address of SX1505 to 0x20 or 0x21, which is set by the position of a resistor near SX1505 (see schematic and board design)
  */
  
  static void TinyScreenC(){
    _addr=0;
    //if(addr)_addr=1;
    _cursorX=0;
    _cursorY=0;
    _fontHeight=0;
    _fontFirstCh=0;
    _fontLastCh=0;
    _fontDescriptor=0;
    _fontBitmap=0;
    _fontColor=0xFF;
    _fontBGcolor=0x00;
    _bitDepth=0;
    _flipDisplay=0;
    _mirrorDisplay=0;
  }
  
  /*
  setCursor(x,y);//set text cursor position to (x,y);
  */
  
  static void TinyScreenC_setCursor(uint8_t x, uint8_t y){
    _cursorX=x;
    _cursorY=y;
  }
  
  static void TinyScreenC_setFont(const FONT_INFO& fontInfo){
    _fontHeight=fontInfo.height;
    _fontFirstCh=fontInfo.startCh;
    _fontLastCh=fontInfo.endCh;
    _fontDescriptor=fontInfo.charDesc;
    _fontBitmap=fontInfo.bitmap;
  }
  
  /*
  fontColor(text color, background color);//sets text and background color
  */
  
  static void TinyScreenC_fontColor(uint8_t f, uint8_t g){
    _fontColor=f;
    _fontBGcolor=g;
  }
  
  size_t TinyScreenC_write(uint8_t ch){
    if(!_fontFirstCh)return 1;
    if(ch<_fontFirstCh || ch>_fontLastCh)return 1;
    if(_cursorX>xMax || _cursorY>yMax)return 1;
    uint8_t chWidth=pgm_read_byte(&_fontDescriptor[ch-_fontFirstCh].width);
    uint8_t bytesPerRow=chWidth/8;
    if(chWidth>bytesPerRow*8)
      bytesPerRow++;
    uint16_t offset=pgm_read_word(&_fontDescriptor[ch-_fontFirstCh].offset)+(bytesPerRow*_fontHeight)-1;
    
    TinyScreenC_setX(_cursorX,_cursorX+chWidth+1);
    TinyScreenC_setY(_cursorY,_cursorY+_fontHeight);
    
    TinyScreenC_startData();
    for(uint8_t y=0; y<_fontHeight && y+_cursorY<yMax+1; y++){
      SPDR=_fontBGcolor;
      for(uint8_t byte=0; byte<bytesPerRow; byte++){
        uint8_t data=pgm_read_byte(_fontBitmap+offset-y-((bytesPerRow-byte-1)*_fontHeight));
        uint8_t bits=byte*8;
          for(uint8_t i=0; i<8 && (bits+i)<chWidth && (bits+i+_cursorX)<xMax; i++){
            while (!(SPSR & _BV(SPIF)));
            if(data&(0x80>>i)){
              SPDR=_fontColor;
             }else{
              SPDR=_fontBGcolor;
            }
        }
      }
      while (!(SPSR & _BV(SPIF)));
      if((_cursorX+chWidth)<xMax)
        SPI_transfer(_fontBGcolor);
    }
    TinyScreenC_endTransfer();
    _cursorX+=(chWidth+1);
    return 1;
  }
  
  static void TinyScreenC_printCStr(const char *str) {
    int i=0;
    while (str[i]) TinyScreenC_write(str[i++]);
  }
}
