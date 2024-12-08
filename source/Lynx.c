#include <nds.h>

#include "Lynx.h"
#include "LynxBorder.h"
#include "Lynx2Border.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "cpu.h"
#include "ARMMikey/ARM6502/M6502.h"
#include "ARMMikey/ARMMikey.h"
#include "ARMSuzy/ARMSuzy.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, lynxRAM, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
	size += m6502SaveState(statePtr+size, &m6502_0);
	size += mikeySaveState(statePtr+size, &mikey_0);
	size += suzySaveState(statePtr+size, &suzy_0);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(lynxRAM, statePtr+size, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
	size += m6502LoadState(&m6502_0, statePtr+size);
	size += mikeyLoadState(&mikey_0, statePtr+size);
	size += suzyLoadState(&suzy_0, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += sizeof(lynxRAM);
	size += m6502GetStateSize();
	size += mikeyGetStateSize();
	size += suzyGetStateSize();
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

void setScreenMode(int mode) {
	switch (mode) {
		case 1:
			REG_BG2X = (((SCREEN_WIDTH+SCREEN_HEIGHT)/2)-1)<<8;
			REG_BG2Y = 0;
			REG_BG2PA = 0;
			REG_BG2PB = -1<<8;
			REG_BG2PC = 1<<8;
			REG_BG2PD = 0;
			break;
		case 2:
			REG_BG2X = ((SCREEN_WIDTH-SCREEN_HEIGHT)/2)<<8;
			REG_BG2Y = (256-1)<<8;
			REG_BG2PA = 0;
			REG_BG2PB = 1<<8;
			REG_BG2PC = -1<<8;
			REG_BG2PD = 0;
			break;
		case 3:
			REG_BG2X = ((SCREEN_WIDTH-GAME_WIDTH)/2)<<8;
			REG_BG2Y = (256*0x100-GAME_HEIGHT*((GAME_WIDTH<<8)/SCREEN_WIDTH)*2)/2;
			REG_BG2PA = (GAME_WIDTH<<8)/SCREEN_WIDTH;
			REG_BG2PB = 0;
			REG_BG2PC = 0;
			REG_BG2PD = (GAME_WIDTH<<8)/SCREEN_WIDTH;
			break;
		default:
			REG_BG2X = 0;
			REG_BG2Y = ((256-SCREEN_HEIGHT)/2)<<8;
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
