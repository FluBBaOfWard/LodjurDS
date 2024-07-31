#include <nds.h>

#include "Lynx.h"
#include "LynxBorder.h"
#include "Lynx2Border.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "cpu.h"
#include "ARM6502/M6502.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, lynxRAM, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
//	size += sphinxSaveState(statePtr+size, &sphinx0);
	size += m6502SaveState(statePtr+size, &m6502_0);
	memcpy(statePtr+size, wsSRAM, sizeof(wsSRAM));
	size += sizeof(wsSRAM);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(lynxRAM, statePtr+size, sizeof(lynxRAM));
	size += sizeof(lynxRAM);
//	size += sphinxLoadState(&sphinx0, statePtr+size);
	size += m6502LoadState(&m6502_0, statePtr+size);
	memcpy(wsSRAM, statePtr+size, sizeof(wsSRAM));
	size += sizeof(wsSRAM);
}

int getStateSize() {
	int size = 0;
	size += sizeof(lynxRAM);
//	size += sphinxGetStateSize();
	size += m6502GetStateSize();
	size += sizeof(wsSRAM);
	return size;
}

static void setupBorderPalette(const void *palette, int len) {
	vramSetBankF(VRAM_F_LCD);
	if (gBorderEnable == 0) {
		memset(VRAM_F, 0, len);
	}
	else {
		memcpy(VRAM_F, palette, len);
	}
	memcpy(VRAM_F + 0xF0, MAPPED_BNW, sizeof(MAPPED_BNW));
	vramSetBankF(VRAM_F_BG_EXT_PALETTE_SLOT23);
}

void setupLynxBackground() {
	decompress(LynxBorderTiles, BG_TILE_RAM(1), LZ77Vram);
	decompress(LynxBorderMap, BG_MAP_RAM(15), LZ77Vram);
}

void setupLynxBorderPalette() {
	setupBorderPalette(LynxBorderPal, LynxBorderPalLen);
}

void setupLynx2Background() {
	decompress(Lynx2BorderTiles, BG_TILE_RAM(1), LZ77Vram);
	decompress(Lynx2BorderMap, BG_MAP_RAM(15), LZ77Vram);
}

void setupLynx2BorderPalette() {
	setupBorderPalette(Lynx2BorderPal, Lynx2BorderPalLen);
}

void setupEmuBackground() {
	if (gMachine == HW_LYNX2) {
		setupLynx2Background();
		setupLynx2BorderPalette();
	}
	else {
		setupLynxBackground();
		setupLynxBorderPalette();
	}
}

void setupEmuBorderPalette() {
	if (gMachine == HW_LYNX2) {
		setupLynx2BorderPalette();
	}
	else {
		setupLynxBorderPalette();
	}
}
