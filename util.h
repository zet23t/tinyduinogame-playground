extern "C" {
	#define STRINGBUFFER_SIZE 80
	typedef struct StringBuffer_s {
		char buffer[STRINGBUFFER_SIZE];
		unsigned char posPointer;
	} StringBuffer;
	static StringBuffer _stringBuffer;
	
	static void StringBuffer_reset() {
		_stringBuffer.posPointer = 0;
	}

	static char* StringBuffer_new() {
		if (STRINGBUFFER_SIZE - _stringBuffer.posPointer > 0) {
			char *chrstr = &_stringBuffer.buffer[_stringBuffer.posPointer++];
			chrstr[0] = 0;
			return chrstr;
		}
		return 0;
	}
	
	static char* StringBuffer_buffer(char *str) {
		int len = strlen(str)+1;
		if (STRINGBUFFER_SIZE - _stringBuffer.posPointer > len) {
			char *chrstr = &_stringBuffer.buffer[_stringBuffer.posPointer];
			strcpy(chrstr,str);
			_stringBuffer.posPointer+=len;
			return chrstr;
		} else {
			return 0;
		}
	}
	
	static void StringBuffer_amend(char *str) {
		_stringBuffer.posPointer-=1;
		if (StringBuffer_buffer(str) == 0) {
			_stringBuffer.posPointer+=1;	
		}
	}

	static void StringBuffer_amendDec(long i) {
		char const digit[] = "0123456789";
	    char* p = &_stringBuffer.buffer[--_stringBuffer.posPointer];
	    if(i<0){
	        *p++ = '-';
	        i = -i;
	        _stringBuffer.posPointer++;
	    }
	    int shifter = i;
	    do{ //Move to where representation ends
	        ++p;
	        shifter = shifter/10;
	        _stringBuffer.posPointer++;
	    }while(shifter);
	    *p = '\0';
	    do{ //Move back, inserting digits as u go
	        *--p = digit[i%10];
	        i = i/10;
	    }while(i);
	    _stringBuffer.posPointer++;
	}
	
	static char* StringBuffer_load(const char * src) {
		int len = strlen_P(src) + 1;
		if (STRINGBUFFER_SIZE - _stringBuffer.posPointer > len) {
			char *str = &_stringBuffer.buffer[_stringBuffer.posPointer];
			_stringBuffer.posPointer+=len;
			return strcpy_P(str, src);
		} else {
			return 0;
		}
	}

	static void StringBuffer_amendLoad(const char *str) {
		_stringBuffer.posPointer-=1;
		if (StringBuffer_load(str) == 0) {
			_stringBuffer.posPointer+=1;	
		}
	}
	
	static uint32_t SquareRoot(uint32_t a_nInput)
	{
	    uint32_t op  = a_nInput;
	    uint32_t res = 0;
	    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type
	
	
	    // "one" starts at the highest power of four <= than the argument.
	    while (one > op)
	    {
	        one >>= 2;
	    }
	
	    while (one != 0)
	    {
	        if (op >= res + one)
	        {
	            op = op - (res + one);
	            res = res +  2 * one;
	        }
	        res >>= 1;
	        one >>= 2;
	    }
	    return res;
	}


	static int clamp(int val, int min, int max) {
		return val < min ? min : (val > max ? max : val);
	}

	/**
	 * Determines for x,y coordinate in which sector of 8 the point lies.
	 * 0 is north, 4 is east. Etc.
	 */
	static unsigned char determineDir8(int dx, int dy) {
		if ((abs(dx)) > (abs(dy)<<1)) {
			if (dx > 0) return 2;
			else return 6;
		} else if ((abs(dy)) > (abs(dx)<<1)) {
			if (dy > 0) return 4;
			else return 0;
		} 
		else if (dx > 0 && dy > 0) return 3;
		else if (dx > 0 && dy < 0) return 1;
		else if (dx < 0 && dy > 0) return 5;
		else return 7;
	}

	/**
	 * Determines for x,y coordinate in which sector of 16 the point lies.
	 * 0 is north, 8 is east. Etc.
	 */
	static unsigned char determineDir16(int dx, int dy) {
		if ((abs(dx)) > (abs(dy)<<2)) {
			if (dx > 0) return 4;
			else return 12;
		} else if ((abs(dy)) > (abs(dx)<<2)) {
			if (dy > 0) return 8;
			else return 0;
		} else if ((abs(dx)) > (abs(dy)<<1)) {
			if (dx > 0) return dy > 0 ? 5 : 3;
			else return dy > 0 ? 11 : 13;
		} else if ((abs(dy)) > (abs(dx)<<1)) {
			if (dy > 0) return dx > 0 ? 7 : 9;
			else return dx > 0 ? 1 : 15;
		} 
		else if (dx > 0 && dy > 0) return 6;
		else if (dx > 0 && dy < 0) return 2;
		else if (dx < 0 && dy > 0) return 10;
		else return 14;
	}
	
	// random number generator
	// some random numbers to improve seeding
	static const unsigned short randMix[] = {
		3961, 57166, 37426, 345, 
		4727, 31105, 32898, 58696, 
		40631, 49699, 28450, 16900, 
		55992, 41894, 14318, 18762};
	// RNG registers
	static unsigned short cheapRndA = 0x1923,cheapRndB = 0x4232,cheapRndC = 0x2393;
	// A quite cheap random number generator
	static unsigned short cheapRnd() {
		unsigned short t;
		cheapRndA ^= (cheapRndA >> 5);
		cheapRndA ^= cheapRndA << 1;
	
	    t = cheapRndA;
	    cheapRndA = cheapRndB;
	    cheapRndB = cheapRndC;
	    cheapRndC = t ^ cheapRndA ^ cheapRndB;
	
	    return cheapRndC;
	}
	// Seeding the RNG, here with two seed values because it's handy this way
	static void cheapRndSeed(unsigned short x, unsigned short y) {
		//cheapRndA = x^132456789 ^ pgm_read_word_near(randMix + x*3%sizeof(randMix));
		cheapRndA = x ^ randMix[((x*7)>>2)&15];
		cheapRndB = y ^ randMix[((y*3)>>2)&15];
		cheapRndC = (x+y) ^ randMix[(cheapRndA^cheapRndB)&15];
		//cheapRnd();
	}

	static unsigned char cheapRndA8 = 0x1923,cheapRndB8 = 0x4232,cheapRndC8 = 0x2393;
	// A quite cheap random number generator
	static unsigned char cheapRnd8() {
		unsigned char t;
		//cheapRndA8 ^= (cheapRndA >> 5);
		//cheapRndA8 ^= cheapRndA << 1;
		cheapRndA8 += 41;
	
	    t = cheapRndA8;
	    cheapRndA8 = cheapRndB8;
	    cheapRndB8 = cheapRndC8;
	    cheapRndC8 = t ^ cheapRndA8 ^ cheapRndB8;
	
	    return cheapRndC8;
	}
	// Seeding the RNG, here with two seed values because it's handy this way
	static void cheapRndSeed8(unsigned short x, unsigned short y) {
		//cheapRndA = x^132456789 ^ pgm_read_word_near(randMix + x*3%sizeof(randMix));
		cheapRndA8 = x ^ randMix[(x-3281)&15];
		cheapRndB8 = y ^ randMix[(y+17)&15];
		cheapRndC8 = (x+y) ^ randMix[(y-x)&15];
		//cheapRnd();
	}
/*
	void FatalError(const char *msg) {
		display.setFont(virtualDJ_5ptFontInfo);
		display.fontColor(BLACK,RED);
		display.setCursor(0,0);
		display.print(msg);
	//	Serial.begin(9600);
	//	Serial.println(msg);
		while(1);
	}*/
	
	int calcParallaxX(int camX, int x, char layer) {
		long dx = x - camX;
		//if (dx > 256) dx = 256;
		//else if (dx < -256) dx = -256;
		return (camX) + ((dx-48)/layer)+48;
	}
	int calcParallaxY(int camY, int y, char layer) {
		long dy = y - camY;
		//if (dy > 256) dy = 256;
		//else if (dy < -256) dy = -256;
		return (camY) + ((dy-32)/layer+32);
	}
}