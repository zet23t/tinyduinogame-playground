extern "C" {
	typedef struct {
		int x;
		int y;
	} Position;
	
	typedef struct {
		int x:4;
		int y:4;
	} Direction;

	typedef struct Tank_s {
		Position pos;
		Direction tankDir;
		Direction turretDir;
	} Tank;

	typedef struct Projectile_s {
		Position pos;
		Direction 
	} Projectile;

	typedef struct Game_s
	{
		Tank playerTank;
	} Game;
	static void TinyTankSetup() {

	}

	static void TinyTankLoop() {
		static char init = 0;
		if (!init) {
			TinyTankSetup();
		}
	}
}