extern "C" {
	#include <math.h>
	// circular radius in which the joystick position is ignored
	#define DEAD_ZONE 100.0f
	typedef struct JoystickData_s {
		short x;
		short y;
		short normX;
		short normY;
		/*short minX;
		short maxX;
		short minY;
		short maxY;*/
	} JoystickData;
	
	static void JoystickDataUpdate (struct JoystickData_s *j, short x, short y) {
		j->x = x;
		j->y = y;
		/*j->minX = j->minX < x ? j->minX : x;
		j->maxX = j->maxX > x ? j->maxX : x;
		j->minY = j->minY < y ? j->minY : y;
		j->maxY = j->maxY > y ? j->maxY : y;*/
		float fx = x;
		float fy = y;
		float len = sqrt(fx*fx+fy*fy);
		if (len < DEAD_ZONE) {
			j->normX = 0;
			j->normY = 0;
		} else {
			fx/=len;
			fy/=len;
			len-=DEAD_ZONE;
			len*=3.0f;
			if (len > 400.0f) len = 400.0f;
			j->normX = fx * len;
			j->normY = fy * len;
		}
	}

	static JoystickData leftStick;
	static JoystickData rightStick;
	static int rightButton = 0;
	static int leftButton = 0;

	void UpdateJoystick(){
		Wire.requestFrom(0x22,6);
		int data[4];
		for(int i=0;i<4;i++){
			data[i]=Wire.read();
		}
		unsigned char lsb=Wire.read();
		unsigned char buttons=~Wire.read();
		leftButton=((buttons&4) ? 2 : 0) | (leftButton >> 1);
		rightButton=((buttons&8) ? 2 : 0) | (rightButton >> 1);
		for(int i=0;i<4;i++){
			data[i]<<=2;
			data[i]|= ((lsb>>(i*2))&3);
			data[i]-=511;
		}
		JoystickDataUpdate(&rightStick, data[0], data[1]);
		JoystickDataUpdate(&leftStick, -data[2], -data[3]);
	}
}