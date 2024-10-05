#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "../Shared/FileHelper.h"
#include "../io.h"

extern "C" {
UWORD peekCPUW(ULONG addr);
void susiePoke(ULONG addr, UBYTE data);
void mikiePoke(ULONG addr, UBYTE data);
UBYTE susiePeek(ULONG addr);
UBYTE mikiePeek(ULONG addr);
void GpInit(const unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void *args);
}

ULONG		mColourMap[4096];
unsigned short vram[2][160*102];
unsigned short *currentDest;
int bufIdx = 0;
bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

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
	newsystem = new CSystem(gamerom, size, HANDY_FILETYPE_LNX, NULL);
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
