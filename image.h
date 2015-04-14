
typedef struct ImageInclude_s {
	const unsigned char width;
	const unsigned char height;
	const unsigned char *data;
	const short transparentIndex;
} ImageInclude;

static void ImageInclude_readLineInto(const ImageInclude *img, unsigned char *lineBuffer, 
	unsigned char start, unsigned char end, unsigned char imgY, char imgX)
{
	unsigned char width = pgm_read_byte(&img->width);
	unsigned char height = pgm_read_byte(&img->height);
	unsigned int data = pgm_read_word(&img->data);
	int transparentIndex = pgm_read_word(&img->transparentIndex);
	imgY %= height;
	imgX %= width;
	unsigned int offset = data + imgY * width + imgX;
	for (int p=start,n=imgX;p<end;n+=1,p+=1) {
		unsigned char c = pgm_read_byte(offset+n);
		if (c != transparentIndex) lineBuffer[p] = c;
		if (n == width) n = 0;
	}
}