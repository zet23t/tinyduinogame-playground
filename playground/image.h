
typedef struct ImageInclude_s {
	const unsigned char width;
	const unsigned char height;
	const unsigned char *data;
	const short transparentIndex;
} ImageInclude;

typedef struct ImageIncludeDrawData_s {
	unsigned char width;
	unsigned char height;
	unsigned int data;
	int transparentIndex;	
} ImageIncludeDrawData;

/** Fills a data structure with data read from flash memory. Saves a few cycles.
 * To be used in conjunction with ImageInclude_readLineIntoPrepared
 */
static void ImageInclude_prepare(const ImageInclude *img, ImageIncludeDrawData *into) {
	into->width = pgm_read_byte(&img->width);
	into->height = pgm_read_byte(&img->height);
	into->data = pgm_read_word(&img->data);
	into->transparentIndex = pgm_read_word(&img->transparentIndex);
}

/** Reads flash memory into a given line buffer. DOES NOT MODULO IMAGE COORDINATES!
 */
static void ImageInclude_readLineIntoPrepared(const ImageInclude *img, ImageIncludeDrawData *drawData, 
	unsigned char *lineBuffer, unsigned char start, unsigned char end, unsigned char imgY, unsigned char imgX) 
{
	unsigned int offset = drawData->data + imgY * drawData->width;
	int transparentIndex = drawData->transparentIndex;
	if (transparentIndex == -1) {
		unsigned char remains = end - start;
		unsigned char range = drawData->width - imgX;
		while (remains > 0) {
			if (range > remains) range = remains;
			memcpy_P(&lineBuffer[start], (void*)(offset + imgX), range);
			remains-=range;
			start+=range;
			range = drawData->width;
			imgX = 0;
		}
	} else {
		for (int p=start,n=imgX;p<end;n+=1,p+=1) {
			if (n == drawData->width) n = 0;
			unsigned char c = pgm_read_byte((void*)(offset+n));
			if (c != transparentIndex) lineBuffer[p] = c;
		}
	}
}

/** Reads flash memory into a given line buffer.
 */
static void ImageInclude_readLineInto(const ImageInclude *img, unsigned char *lineBuffer,
	unsigned char start, unsigned char end, unsigned char imgY, unsigned char imgX)
{
	unsigned char width = pgm_read_byte(&img->width);
	unsigned char height = pgm_read_byte(&img->height);
	unsigned int data = pgm_read_word(&img->data);
	int transparentIndex = pgm_read_word(&img->transparentIndex);
	imgY %= height;
	imgX %= width;
	unsigned int offset = data + imgY * width;
	if (transparentIndex == -1) {
		unsigned char remains = end - start;
		unsigned char range = width - imgX;
		while (remains > 0) {
			if (range > remains) range = remains;
			memcpy_P(&lineBuffer[start], (void*)(offset + imgX), range);
			remains-=range;
			start+=range;
			range = width;
			imgX = 0;
		}
	} else {
		for (int p=start,n=imgX;p<end;n+=1,p+=1) {
			if (n == width) n = 0;
			unsigned char c = pgm_read_byte((void*)(offset+n));
			if (c != transparentIndex) lineBuffer[p] = c;
		}
	}
}
/*
Tile set related image drawing. Right now not important for me. Implement later.

static void ImageInclude_readTileMapInto(const ImageInclude *img, unsigned char *lineBuffer,
	unsigned char offsetY, unsigned char offsetX, unsigned char mapX, unsigned char mapY,
	unsigned char uvShiftX, unsigned char uvShiftY, unsigned char *uvOffsetMap, unsigned char mapWidth)
{
	unsigned char width = pgm_read_byte(&img->width);
	unsigned int data = pgm_read_word(&img->data);

	unsigned char x = 0;
	unsigned char step = 1<<uvShiftX;
	while (x<96) {
		unsigned int offset = data + offsetY * width;
		x+=step;
	}

}*/
