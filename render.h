extern "C" {
//#define RENDERSCREEN_BENCH 1

#define RENDERSCREEN_CULLING 1

#define RENDERCOMMAND_TRECT 1
#define RENDERCOMMAND_TCIRCLE 2
#define RENDERCOMMAND_TSTAR 3
#define RENDERCOMMAND_TTEXT 4

#define RENDERCOMMAND_COLORED 0x10
#define RENDERCOMMAND_TEXTURED 0x20

#define RENDERSCREEN_MAX_COMMANDS 60
#define RENDERSCREEN_WIDTH 96
#define RENDERSCREEN_HEIGHT 64

#define RENDERSCREEN_TEXT_FORMAT_COUNT 1
#define RENDERSCREEN_TILEMAP_COUNT 1
#define RENDERSCREEN_TEXTURE_COUNT 8

  
  typedef struct RenderCommand_s {
    unsigned char type, color, nextIndex;
    union {
      struct {
        // rect
        unsigned char x1, y1, w, y2;
        char u,v;
      } rect;
      struct {
        // circle
        char x, y;
        unsigned char r;
        unsigned short r2;
      } circle;
      struct {
        // star
        unsigned char x, y, size;
      } star;
      struct {
        // text
        unsigned char x,y,fontIndex;
        char *str;
      } text;
    };
  } RenderCommand;

  typedef struct RenderTileMapData_s
  {
    unsigned char *dataMap;
    unsigned char tileSizeX;
    unsigned char tileSizeY;
    unsigned char dataMapWidth;
    unsigned char dataMapHeight;
  } RenderTileMapData;
  #define RENDERSCREEN_FLAG_NOCLEAR 1

  typedef struct RenderScreen_s {
    const ImageInclude *imageIncludes[RENDERSCREEN_TEXTURE_COUNT];
    const FONT_INFO *fontFormats[RENDERSCREEN_TEXT_FORMAT_COUNT];
    RenderTileMapData *tileMap;
    RenderCommand commands[RENDERSCREEN_MAX_COMMANDS];
    unsigned char commandCount;
    unsigned char flags;
  } RenderScreen;

  static RenderScreen _renderScreen;

  static RenderCommand* RenderScreen_drawRect (int x, int y, int w, int h, int color) {
    if (_renderScreen.commandCount >= RENDERSCREEN_MAX_COMMANDS) return 0;
    if (x>=RENDERSCREEN_WIDTH || y >= RENDERSCREEN_HEIGHT || w <= 0 || h <= 0) return 0;
    int x2 = x+w;
    int y2 = y+h;
    if (x2 <= 0 || y2 <= 0) return 0;
    int u = 0,v = 0;
    if (x2 > RENDERSCREEN_WIDTH) x2 = RENDERSCREEN_WIDTH;
    if (y2 > RENDERSCREEN_HEIGHT) y2 = RENDERSCREEN_HEIGHT;
    if (x < 0) u = -x, x = 0;
    if (y < 0) v = -y, y = 0;
    RenderCommand *command = &_renderScreen.commands[_renderScreen.commandCount++];
    command->type = RENDERCOMMAND_TRECT;
    command->color = color;
    command->rect.x1 = x;
    command->rect.y1 = y;
    command->rect.w = x2-x;
    command->rect.y2 = y2;
    command->rect.u = u;
    command->rect.v = v;
    return command;
  }

  static RenderCommand* RenderScreen_drawRectTextured (int x, int y, int w, int h, unsigned char imageId) {
    RenderCommand *cmd = RenderScreen_drawRect(x,y,w,h,imageId);
    if (cmd) cmd->type|=RENDERCOMMAND_TEXTURED;
    return cmd;
  }

  static void RenderScreen_drawRectTexturedUV (int x, int y, int w, int h, unsigned char imageId, int u, int v) {
    RenderCommand *cmd = RenderScreen_drawRectTextured(x,y,w,h,imageId);
    if (cmd) {
      cmd->rect.u+=u;
      cmd->rect.v+=v;
    }
  }

  static void RenderScreen_drawCircle (int x, int y, unsigned long r16, int color) {
    if (_renderScreen.commandCount >= RENDERSCREEN_MAX_COMMANDS) return;
    int r = r16 >> 4;
    if (x + r <= 0 || x-r >= RENDERSCREEN_WIDTH || y + r <= 0 || y - r >= RENDERSCREEN_HEIGHT) return;
    RenderCommand *command = &_renderScreen.commands[_renderScreen.commandCount++];
    command->type = RENDERCOMMAND_TCIRCLE;
    command->color = color;
    command->circle.x = x;
    command->circle.y = y;
    command->circle.r = r;
    unsigned long r2 = ((r16 * r16) >> 8);
    if (r2 > 0xffff) r2 = 0xffff;
    command->circle.r2 = r2;
  }

  static void RenderScreen_drawText (int x, int y, unsigned char fontIndex, char *string, unsigned char color) {
    if (_renderScreen.commandCount >= RENDERSCREEN_MAX_COMMANDS) return;
    if (y >= RENDERSCREEN_HEIGHT || x >= RENDERSCREEN_WIDTH) return;
    const FONT_INFO *info = _renderScreen.fontFormats[fontIndex];
    if (y + info->height < 0) return;
    RenderCommand *command = &_renderScreen.commands[_renderScreen.commandCount++];
    command->type = RENDERCOMMAND_TTEXT;
    command->color = color;
    command->text.x = x;
    command->text.y = y;
    command->text.fontIndex = fontIndex;
    command->text.str = string;
  }
  
  static void RenderScreen_putString(char y, char fontX, char fontY, const char * string, const FONT_INFO *fontInfo, unsigned char *lineBuffer, unsigned char color)
  {
    uint8_t fontHeight = fontInfo->height;
    //if(y >= fontY && y < fontY + fontHeight) // checked in caller already
    {
      const FONT_CHAR_INFO* fontDescriptor = fontInfo->charDesc;
      const unsigned char* fontBitmap = fontInfo->bitmap;
      uint8_t fontFirstCh = fontInfo->startCh;
      uint8_t fontLastCh = fontInfo->endCh;
      uint8_t stringChar = 0;
      uint8_t ch = string[stringChar++];
      while(ch)
      {
        uint8_t chWidth = pgm_read_byte(&fontDescriptor[ch - fontFirstCh].width);
        uint8_t bytesPerRow = chWidth / 8;
        if(chWidth > bytesPerRow * 8)
          bytesPerRow++;
        uint16_t offset = pgm_read_word(&fontDescriptor[ch - fontFirstCh].offset) + (bytesPerRow * fontHeight) - 1;
        const unsigned char *coffset = offset + fontBitmap - (y - fontY);
        for(uint8_t byte = 0; byte < bytesPerRow; byte++)
        {
          uint8_t data = pgm_read_byte(coffset - ((bytesPerRow - byte - 1) * fontHeight));
          uint8_t bits = byte * 8;
          for(uint8_t i = 0; i < 8 && (bits + i) < chWidth && fontX < 96; ++i)
          {
            if((data & (0x80 >> i)) && fontX >= 0)
            {
              lineBuffer[fontX] = color;
            }
            ++fontX;
          }
        }
        fontX += 1;
        ch = string[stringChar++];
      }
    }
  }


  static char RenderScreen_fillLine(RenderCommand *command,char y,unsigned char lineBuffer[RENDERSCREEN_WIDTH]) {
    unsigned char t = command->type & 0xf;
    unsigned char fill = command->type & 0xf0;
    if (t == RENDERCOMMAND_TRECT) {
      if (y >= command->rect.y1 && y<command->rect.y2) {
        if (fill == 0) {
          memset(&lineBuffer[command->rect.x1],command->color,command->rect.w);
        } else if (fill == RENDERCOMMAND_TEXTURED) {
          ImageInclude_readLineInto(_renderScreen.imageIncludes[command->color], lineBuffer, 
            command->rect.x1, command->rect.x1+command->rect.w, y - command->rect.y1 + command->rect.v, command->rect.u);
        }
      }
      return command->rect.y2 <= y;
    }
    if (t == RENDERCOMMAND_TCIRCLE) {
      char dy = y - command->circle.y;
      unsigned short dy2 = dy*dy;
      unsigned short r2 = command->circle.r2;
      if (dy2 < r2) {
        char x = command->circle.x;
        char r = command->circle.r;
        char startx = x - r - 1;
        char tox = x + r + 2;
        if (tox > RENDERSCREEN_WIDTH) tox = RENDERSCREEN_WIDTH;
        if (startx < 0) startx = 0;
        int dx;
        unsigned char color = command->color;
        char firstHit = 0;
        for (int sx = startx; sx < tox;++sx) {
          dx = sx - x;
          if (dx*dx + dy2 < r2) 
          {
            if (firstHit == 0 && dx < 0) {
              unsigned char fillUpTo = x - dx + 1;
              if (fillUpTo > RENDERSCREEN_WIDTH) fillUpTo = RENDERSCREEN_WIDTH;
              memset(&lineBuffer[sx],color,fillUpTo-sx);
              sx = fillUpTo - 1; 
            } else 
            {

              lineBuffer[sx] = color;
            }
            firstHit = 1;
          } else if (firstHit) break;
        }
        return 0;
      }
      return dy >= command->circle.r;
    }
    if (t == RENDERCOMMAND_TTEXT) {
      if (y < command->text.y) return 0;
      const FONT_INFO *fontInfo = _renderScreen.fontFormats[command->text.fontIndex];
      if (y >= command->text.y + fontInfo->height) return 1;
      RenderScreen_putString(y, command->text.x, command->text.y, command->text.str, fontInfo, lineBuffer, command->color);
      return 0;
    }
    return 1;
  }

#define RENDERSCREEN_SLICE 15
  static char RenderScreen_onVLine(RenderCommand *command, char y) {
    unsigned char t = command->type & 0xf;
    if (t == RENDERCOMMAND_TRECT) {
      if (y + RENDERSCREEN_SLICE < command->rect.y1) return -1;
      if (y >= command->rect.y2) return 1;
      return 0;
    }
    if (t == RENDERCOMMAND_TCIRCLE) {
      if (y + RENDERSCREEN_SLICE < command->circle.y-command->circle.r - 1) return -1;
      if (y >= command->circle.y + command->circle.r + 1) return 1;
      return 0;
    }
    if (t == RENDERCOMMAND_TTEXT) {
      if (y + RENDERSCREEN_SLICE < command->text.y) return -1;
      if (y >= command->text.y + _renderScreen.fontFormats[command->text.fontIndex]->height) return 1;
      return 0;
    }
    return 1;
  }

  static void RenderScreen_flush() {
    #ifdef RENDERSCREEN_BENCH
    long start = micros();
    static unsigned char ttotal = 0;
    #endif
    unsigned char lineBuffer[RENDERSCREEN_WIDTH];
    TinyScreenC_goTo(0, 0);
    TinyScreenC_startData();
    unsigned char clear = (_renderScreen.flags & RENDERSCREEN_FLAG_NOCLEAR) == 0;
    unsigned char first; // position of first element in the list that's in the current slice
    for (char i = 0; i < RENDERSCREEN_HEIGHT; i += 1) {
      if (clear) memset(lineBuffer, 0, RENDERSCREEN_WIDTH);
      #ifdef RENDERSCREEN_BENCH
      unsigned char n = 0;
      #endif
      #ifdef RENDERSCREEN_CULLING
      if ((i&(RENDERSCREEN_SLICE)) == 0) {
        // filter the element that are in the next slice.
        first = 0xff; // invalid position in case no element is visible
        unsigned char p = 0, last = 0xff;
        for (char j=0;j<_renderScreen.commandCount;j+=1) {
          RenderCommand *command = &_renderScreen.commands[j];
          char cmp = RenderScreen_onVLine(command,i);
          if (cmp == 0) { 
            // element is intersecting the slice, let's put it into our linked list
            command->nextIndex = 0xff;
            if (first == 0xff) {
              last = first = p;
            } else {
              _renderScreen.commands[last].nextIndex = p;
              last = p;
            }
          } else if (cmp == 1) {
            // element is not important any more, remove it
            continue;
          }
          // remove elements that are no longer needed by overwriting.
          _renderScreen.commands[p++] = _renderScreen.commands[j];
        }
        // set new command count
        _renderScreen.commandCount = p;  
      }
      unsigned char prev = 0xff;
      for (unsigned char j=first;j<_renderScreen.commandCount;j = _renderScreen.commands[j].nextIndex) {
        if (RenderScreen_fillLine(&_renderScreen.commands[j],i,lineBuffer)) {
          // remove the element from the current slice linked list
          if (prev == 0xff) {
            first = _renderScreen.commands[j].nextIndex;
          } else {
            _renderScreen.commands[prev].nextIndex = _renderScreen.commands[j].nextIndex;
          }
        } else {
          prev = j;
        }
        #ifdef RENDERSCREEN_BENCH
        n+=1;
        #endif
      }
      #else
      for (unsigned char j=0;j<_renderScreen.commandCount;j +=1) 
        RenderScreen_fillLine(&_renderScreen.commands[j],i,lineBuffer);
      #endif
      #ifdef RENDERSCREEN_BENCH
      lineBuffer[_renderScreen.commandCount] ^= 0xf;
      lineBuffer[n]^= 0xff;
      lineBuffer[ttotal] ^= 0xf0;
      if (i&1) lineBuffer[50] ^= 0xff;
      #endif
      TinyScreenC_writeBuffer(lineBuffer, RENDERSCREEN_WIDTH);
    }
    TinyScreenC_waitForBuffer();
    TinyScreenC_endTransfer();
    _renderScreen.commandCount = 0;
    #ifdef RENDERSCREEN_BENCH
    ttotal = (micros() - start)/1000;
    #endif
    
  }
}
