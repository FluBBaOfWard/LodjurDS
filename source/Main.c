#include <nds.h>
#include <maxmod9.h>

#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/FileHelper.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "FileHandling.h"
#include "EmuFont.h"
#include "Lynx.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"
#include "Sound.h"

static void checkTimeOut(void);
static void setupGraphics(void);
static void setupStream(void);

bool powerIsOn = false;
bool gameInserted = false;
static int sleepTimer = 60*60*5;	// 5 min
static bool vBlankOverflow = false;

static mm_ds_system sys;
static mm_stream myStream;

uint16 *map0sub;
uint16 *map1sub;

static const u8 guiPalette[] = {
	0x00,0x00,0xC0, 0x00,0x00,0x00, 0x81,0x81,0x81, 0x93,0x93,0x93, 0xA5,0xA5,0xA5, 0xB7,0xB7,0xB7, 0xC9,0xC9,0xC9, 0xDB,0xDB,0xDB,
	0xED,0xED,0xED, 0xFF,0xFF,0xFF, 0x00,0x00,0xC0, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x00,0x00,0x00, 0x00,0x00,0x00, 0x50,0x78,0x78, 0x60,0x90,0x90, 0x78,0xB0,0xB0, 0x88,0xC8,0xC8, 0x90,0xE0,0xE0, 0xA0,0xF0,0xF0,
	0xB8,0xF8,0xF8, 0xEF,0xFF,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x00,0x00,0x00, 0x00,0x00,0x00, 0x21,0x21,0x21, 0x33,0x33,0x33, 0x45,0x45,0x45, 0x47,0x47,0x47, 0x59,0x59,0x59, 0x6B,0x6B,0x6B,
	0x7D,0x7D,0x7D, 0x8F,0x8F,0x8F, 0x20,0x20,0xE0, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x00,0x00,0x00, 0x00,0x00,0x00, 0x81,0x81,0x81, 0x93,0x93,0x93, 0xA5,0xA5,0xA5, 0xB7,0xB7,0xB7, 0xC9,0xC9,0xC9, 0xDB,0xDB,0xDB,
	0xED,0xED,0xED, 0xFF,0xFF,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x00,0x00,0x00, 0x00,0x00,0x00, 0x70,0x70,0x20, 0x88,0x88,0x40, 0xA0,0xA0,0x60, 0xB8,0xB8,0x80, 0xD0,0xD0,0x90, 0xE8,0xE8,0xA0,
	0xF7,0xF7,0xC0, 0xFF,0xFF,0xE0, 0x00,0x00,0x60, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};

//---------------------------------------------------------------------------------
void myVblank(void) {
//---------------------------------------------------------------------------------
	vBlankOverflow = true;
//	DC_FlushRange(EMUPALBUFF, 0x400);
	vblIrqHandler();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	if (argc > 1) {
		enableExit = true;
	}
	maxRomSize = 0x80000 + 0x10000;
	u8 *memPtr = malloc(maxRomSize);
	romSpacePtr = memPtr;
	setupGraphics();

	setupStream();
	irqSet(IRQ_VBLANK, myVblank);
	SetYtrigger(230);
	irqSet(IRQ_VCOUNT, lowerRefresh);
	irqEnable(IRQ_VCOUNT);
	setupGUI();
	getInput();
	initSettings();
	machineInit();
	loadCart();
	if (initFileHelper()) {
		loadSettings();
		loadBios();
		if (argc > 1) {
			loadGame(argv[1]);
			setMuteSoundGUI();
		}
		redrawUI();
	}
	else {
		infoOutput("fatInitDefault() failure.");
	}

	while (1) {
		waitVBlank();
		checkTimeOut();
		guiRunLoop();
		if (powerIsOn && !pauseEmulation) {
			run();
		}
	}
	free(memPtr);
	return 0;
}

//---------------------------------------------------------------------------------
void pausVBlank(int count) {
//---------------------------------------------------------------------------------
	while (--count) {
		waitVBlank();
	}
}

//---------------------------------------------------------------------------------
void waitVBlank() {
//---------------------------------------------------------------------------------
	// Workaround for bug in Bios.
	if (!vBlankOverflow) {
		swiIntrWait(1, IRQ_VBLANK);
	}
	vBlankOverflow = false;
}

//---------------------------------------------------------------------------------
static void checkTimeOut() {
//---------------------------------------------------------------------------------
	if (EMUinput) {
		sleepTimer = sleepTime;
	}
	else {
		sleepTimer--;
		if (sleepTimer < 0) {
			sleepTimer = sleepTime;
			// systemSleep doesn't work as expected.
			//systemSleep();	
		}
	}
}

//---------------------------------------------------------------------------------
void setEmuSpeed(int speed) {
//---------------------------------------------------------------------------------
	if (speed == 0) {		// Normal Speed
		waitMaskIn = 0x00;
		waitMaskOut = 0x00;
	}
	else if (speed == 1) {	// Double speed
		waitMaskIn = 0x00;
		waitMaskOut = 0x01;
	}
	else if (speed == 2) {	// Max speed (4x)
		waitMaskIn = 0x00;
		waitMaskOut = 0x03;
	}
	else if (speed == 3) {	// 50% speed
		waitMaskIn = 0x01;
		waitMaskOut = 0x00;
	}
}

//---------------------------------------------------------------------------------
void setPowerIsOn(bool power) {
//---------------------------------------------------------------------------------
	if (powerIsOn != power) {
		powerIsOn = power;
		if (gMachine == HW_LYNX_II) {
			u32 *src = (u32 *)BG_TILE_RAM(0);
			u32 *dst = (u32 *)(BG_TILE_RAM(0) + 16 + 256*32);
			if (powerIsOn) {
				src += 4;
			}
			for (int y=0; y<8; y++) {
				for (int x=0; x<4; x++) {
					*dst++ = *src++;
				}
				src += 124;
				dst += 124;
			}
		}
	}
}

//---------------------------------------------------------------------------------
static void setupGraphics() {
//---------------------------------------------------------------------------------

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_MAIN_BG_0x06040000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06060000);
	vramSetBankE(VRAM_E_MAIN_SPRITE);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_SUB_SPRITE);

	// Set up the main display
	GFX_DISPCNT = MODE_5_2D
//				 | DISPLAY_BG0_ACTIVE
				 | DISPLAY_BG2_ACTIVE
//				 | DISPLAY_BG_EXT_PALETTE
				 ;
	videoSetMode(GFX_DISPCNT);
	GFX_BG0CNT = BG_32x32 | BG_MAP_BASE(0) | BG_COLOR_16 | BG_TILE_BASE(2) | BG_PRIORITY(0);
	REG_BG0CNT = GFX_BG0CNT;
	// Background 2 for border
//	REG_BG2CNT = BG_32x32 | BG_MAP_BASE(15) | BG_COLOR_256 | BG_TILE_BASE(4) | BG_PRIORITY(2);
	REG_BG2CNT = BG_BMP16_256x256 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	setScreenMode(0);

	// Set up the sub display
	videoSetModeSub(MODE_0_2D
					| DISPLAY_BG0_ACTIVE
					| DISPLAY_BG1_ACTIVE
					);
	// Set up two backgrounds for menu
	REG_BG0CNT_SUB = BG_32x32 | BG_MAP_BASE(0) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG1CNT_SUB = BG_32x32 | BG_MAP_BASE(1) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG1HOFS_SUB = 0;
	REG_BG1VOFS_SUB = 0;
	map0sub = BG_MAP_RAM_SUB(0);
	map1sub = BG_MAP_RAM_SUB(1);

	setupEmuBackground();

	decompress(EmuFontTiles, BG_GFX_SUB+0x1200, LZ77Vram);
	setupMenuPalette();
}

void setupMenuPalette() {
	convertPalette(BG_PALETTE_SUB, guiPalette, sizeof(guiPalette)/3, gGammaValue);
}

//---------------------------------------------------------------------------------
static void setupStream(void) {
//---------------------------------------------------------------------------------

	//----------------------------------------------------------------
	// initialize maxmod without any soundbank (unusual setup)
	//----------------------------------------------------------------
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );

	//----------------------------------------------------------------
	// open stream
	//----------------------------------------------------------------
	myStream.sampling_rate	= sample_rate;				// sampling rate =
	myStream.buffer_length	= buffer_size;				// buffer length =
	myStream.callback		= VblSound2;				// set callback function
	myStream.format			= MM_STREAM_16BIT_STEREO;	// format = stereo 16-bit
	myStream.timer			= MM_TIMER0;				// use hardware timer 0
	myStream.manual			= false;					// use manual filling
	mmStreamOpen( &myStream );

	//----------------------------------------------------------------
	// when using 'automatic' filling, your callback will be triggered
	// every time half of the wave buffer is processed.
	//
	// so: 
	// 25000 (rate)
	// ----- = ~21 Hz for a full pass, and ~42hz for half pass
	// 1200  (length)
	//----------------------------------------------------------------
	// with 'manual' filling, you must call mmStreamUpdate
	// periodically (and often enough to avoid buffer underruns)
	//----------------------------------------------------------------
}
