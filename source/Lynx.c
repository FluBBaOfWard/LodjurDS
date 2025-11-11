#include <nds.h>

#include "Lynx.h"
#include "LynxBorder.h"
#include "Lynx2Border.h"
#include "Main.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "cpu.h"
#include "ARMMikey/ARM6502/M6502.h"
#include "ARMMikey/ARMMikey.h"
#include "ARMSuzy/ARMSuzy.h"
#include "LynxCart/LynxCart.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, lynxRAM, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
	size += m6502SaveState(statePtr+size, &m6502_0);
	size += mikeySaveState(statePtr+size, &mikey_0);
	size += suzySaveState(statePtr+size, &suzy_0);
	size += cartSaveState(statePtr+size, &cart_0);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(lynxRAM, statePtr+size, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
	size += m6502LoadState(&m6502_0, statePtr+size);
	size += mikeyLoadState(&mikey_0, statePtr+size);
	size += suzyLoadState(&suzy_0, statePtr+size);
	size += cartLoadState(&cart_0, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += sizeof(lynxRAM);
	size += m6502GetStateSize();
	size += mikeyGetStateSize();
	size += suzyGetStateSize();
	size += cartGetStateSize();
	return size;
}

static void setupBorderPalette(const unsigned short *palette, int len) {
	vramSetBankF(VRAM_F_LCD);
	if (gBorderEnable == 0) {
		memset(VRAM_F, 0, len);
	}
	else {
		memcpy(VRAM_F, palette, len);
	}
	vramSetBankF(VRAM_F_BG_EXT_PALETTE_SLOT23);
}

#define BUFFER_WIDTH SCREEN_WIDTH
#define BUFFER_HEIGHT (256)
#define ZOOM_VAL ((GAME_WIDTH<<8)/(SCREEN_WIDTH-1))

void setScreenMode(int mode) {
	swiIntrWait(1, IRQ_VBLANK);
	switch (mode) {
		case 1:			// Rotate Left
			REG_BG2X = (((SCREEN_WIDTH+SCREEN_HEIGHT)/2)-1)<<8;
			REG_BG2Y = ((BUFFER_HEIGHT-SCREEN_WIDTH)/2)<<8;
			REG_BG2PA = 0;
			REG_BG2PB = -1<<8;
			REG_BG2PC = 1<<8;
			REG_BG2PD = 0;
			break;
		case 2:			// Rotate Right
			REG_BG2X = ((SCREEN_WIDTH-SCREEN_HEIGHT)/2)<<8;
			REG_BG2Y = (((BUFFER_HEIGHT+SCREEN_WIDTH)/2)-1)<<8;
			REG_BG2PA = 0;
			REG_BG2PB = 1<<8;
			REG_BG2PC = -1<<8;
			REG_BG2PD = 0;
			break;
		case 3:			// Zoom
			REG_BG2X = ((BUFFER_WIDTH-GAME_WIDTH)/2)<<8;
			REG_BG2Y = ((BUFFER_HEIGHT<<8)-SCREEN_HEIGHT*ZOOM_VAL)/2;
			REG_BG2PA = ZOOM_VAL;
			REG_BG2PB = 0;
			REG_BG2PC = 0;
			REG_BG2PD = ZOOM_VAL;
			break;
		default:		// 1:1
			REG_BG2X = 0;
			REG_BG2Y = ((BUFFER_HEIGHT-SCREEN_HEIGHT)/2)<<8;
			REG_BG2PA = 1<<8;
			REG_BG2PB = 0;
			REG_BG2PC = 0;
			REG_BG2PD = 1<<8;
			break;
	}
}

void setupLynxBackground() {
	decompress(LynxBorderBitmap, BG_TILE_RAM(0), LZ77Vram);
//	decompress(LynxBorderMap, BG_MAP_RAM(15), LZ77Vram);
}

void setupLynxBorderPalette() {
//	setupBorderPalette(LynxBorderPal, LynxBorderPalLen);
}

void setupLynx2Background() {
	decompress(Lynx2BorderBitmap, BG_TILE_RAM(0), LZ77Vram);
//	decompress(Lynx2BorderMap, BG_MAP_RAM(15), LZ77Vram);
}

void setupLynx2BorderPalette() {
//	setupBorderPalette(Lynx2BorderPal, Lynx2BorderPalLen);
}

void setupEmuBackground() {
	if (gMachine == HW_LYNX_II) {
		setupLynx2Background();
//		setupLynx2BorderPalette();
	}
	else {
		setupLynxBackground();
//		setupLynxBorderPalette();
	}
}

void setupEmuBorderPalette() {
	if (gMachine == HW_LYNX_II) {
		setupLynx2BorderPalette();
	}
	else {
		setupLynxBorderPalette();
	}
}
