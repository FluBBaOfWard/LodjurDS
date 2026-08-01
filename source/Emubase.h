#ifndef EMUBASE
#define EMUBASE

#ifdef __cplusplus
extern "C" {
#endif

#define ALLOW_SPEED_HACKS	(1<<17)
#define SOUND_ENABLE		(1<<18)
#define ALLOW_REFRESH_CHG	(1<<19)

typedef struct {				//(config struct)
	char magic[4];				//="CFG",0
	int emuSettings;
	int unused;					// unused
	u8 gammaValue;				// from gfx.s
	u8 config;					// from cart.s
	u8 controller;				// from io.s
	u8 contrastValue;			// from gfx.s
	u8 machine;
	u8 padding[3];
	char currentPath[256];
	char biosPath[256];
} ConfigData;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !EMUBASE
