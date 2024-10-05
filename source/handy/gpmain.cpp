#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "../Shared/FileHelper.h"
#include "../io.h"

extern "C" {
void pokeCPU(ULONG addr, UBYTE data);
UBYTE peekCPU(ULONG addr);
UWORD peekCPUW(ULONG addr);
void susiePoke(ULONG addr, UBYTE data);
void mikiePoke(ULONG addr, UBYTE data);
UBYTE susiePeek(ULONG addr);
UBYTE mikiePeek(ULONG addr);
void GpInit(const unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void *args);
}

const char *romfile = "LYNXBOOT.IMG";
//const char *gamefile = "apb - all points bulletin (1990).lnx";
//const char *gamefile = "blue lightning (1989).lnx";
//const char *gamefile = "electrocop (1989).lnx";
//const char *gamefile = "lynx diagnostic cart v0.02 (1989)[crc-2].lnx";
//const char *gamefile = "toki (1990).lnx";

ULONG		mColourMap[4096];
unsigned short vram[2][160*102];
unsigned short *currentDest;
int bufIdx = 0;
bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

int loadFile(const char *fname, void *dest, int start, int maxSize) {
	FILE *fHandle;

	findFolder("Lynx");

	if ( !(fHandle = fopen(fname, "r" ))) {
		printf("Couldn't load file **1**\n");
		return -1;
	}
	if (start > 0) {
		fseek(fHandle, start, SEEK_SET);
	}
	int size = fread(dest, 1, maxSize, fHandle);
	fclose(fHandle);
	return size;
}

void handy_nds_render_callback(UBYTE *ram, ULONG *palette, bool flip) {
	UWORD *bitmap_tmp = currentDest;
	for (int loop=0;loop<LYNX_SCREEN_WIDTH/2;loop++) {
		int source = *ram;
		if (flip) {
			ram -= 1;
			*bitmap_tmp = (UWORD)mColourMap[palette[source & 0x0f]] | 0x8000;
			bitmap_tmp += 1;
			*bitmap_tmp = (UWORD)mColourMap[palette[source >> 4]] | 0x8000;
			bitmap_tmp += 1;
		}
		else {
			ram += 1;
			*bitmap_tmp = (UWORD)mColourMap[palette[source >> 4]] | 0x8000;
			bitmap_tmp += 1;
			*bitmap_tmp = (UWORD)mColourMap[palette[source & 0x0f]] | 0x8000;
			bitmap_tmp += 1;
		}
	}
	currentDest += 256;
}

void handy_nds_display_callback(void)
{
	bufIdx ^= 1;
	currentDest = ((unsigned short *)0x06000000);
	gScreenUpdateRequired = TRUE;
}

void pokeCPU(ULONG addr, UBYTE data) {
	if (addr < 0xFC00) {
		ramPoke(addr,data);
		return;
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(memSelector & 0x01)) {
				susiePoke(addr,data);
				return;
			}
			break;
		case 0x0100:
			if (!(memSelector & 0x02)) {
				mikiePoke(addr,data);
				return;
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					ramPoke(addr,data);
					return;
				}
				if (addr == 0xFFF9) {
					memSelector = data & 0xF;
					return;
				}
				if (!(memSelector & 0x08)) {
					rom_W(addr,data);
					return;
				} else {
					ramPoke(addr,data);
					return;
				}
			}
		case 0x0200:
			if (!(memSelector & 0x04)) {
				rom_W(addr,data);
				return;
			}
			break;
	}
	ramPoke(addr,data);
}

UBYTE peekCPU(ULONG addr) {
	if (addr < 0xFC00) {
		return ramPeek(addr);
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(memSelector & 0x01)) {
				return susiePeek(addr);
			}
			break;
		case 0x0100:
			if (!(memSelector & 0x02)) {
				return mikiePeek(addr);
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					return ramPeek(addr);
				}
				if (addr == 0xFFF9) {
					return memSelector;
				}
				if (!(memSelector & 0x08)) {
					return romPeek(addr);
				} else {
					return ramPeek(addr);
				}
			}
		case 0x0200:
			if (!(memSelector & 0x04)) {
				return romPeek(addr);
			}
			break;
	}
	return ramPeek(addr);
}

UWORD peekCPUW(ULONG addr) {
	return ((peekCPU(addr))+(peekCPU(addr+1)<<8));
}

void susiePoke(ULONG addr, UBYTE data) {
	newsystem->mSusie->Poke(addr,data);
}
void mikiePoke(ULONG addr, UBYTE data) {
	newsystem->mMikie->Poke(addr,data);
}
UBYTE susiePeek(ULONG addr) {
	return newsystem->mSusie->Peek(addr);
}
UBYTE mikiePeek(ULONG addr) {
	return newsystem->mMikie->Peek(addr);
}

void GpInit(const unsigned char *gamerom, int size) {
	newsystem = new CSystem(gamerom, size, HANDY_FILETYPE_LNX, romfile);
	newsystem->DisplaySetAttributes(handy_nds_display_callback, handy_nds_render_callback);
	currentDest = ((unsigned short *)0x06000000);
	for (int i=0;i<4096;i++) {
		mColourMap[i] = (i<<3) & 0x7c00;
		mColourMap[i] |= (i<<6) & 0x03e0;
		mColourMap[i] |= (i>>3) & 0x001f;
	}
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}

void GpMain(void *args) {

	while (newsystem != NULL) {

		for (int i=0;i<1024;i++) {
			newsystem->Update();
		}
		newsystem->SetButtonData( joy0_R() );

		if (gScreenUpdateRequired) {
			gScreenUpdateRequired = FALSE;
			return;
		}
	}

}
