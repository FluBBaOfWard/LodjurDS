#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "../Shared/FileHelper.h"
#include "../io.h"
#include "../Gfx.h"
#include "../ARMMikey/ARMMikey.h"

extern "C" {
u32 paintSprites(void);
void runTimer4(u32 sysCount);
void susiePoke(u32 addr, u8 data);
void mikiePoke(u32 addr, u8 data);
u8 susiePeek(u32 addr);
u8 mikiePeek(u32 addr);
void GpInit(unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void);
}

bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

u32 paintSprites() {
	return newsystem->mSusie->PaintSprites();
}
void runTimer4(u32 sysCount) {
	newsystem->mMikie->UpdateTimer4(sysCount);
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

void GpInit(unsigned char *gamerom, int size) {
	newsystem = new CSystem(gamerom, size, HANDY_FILETYPE_LNX, NULL);
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}

void GpMain() {
	while (newsystem != NULL) {

		lnxSuzySetButtonData( joy0_R() );
		mikSysUpdate();

		if (gScreenUpdateRequired) {
			gScreenUpdateRequired = FALSE;
			return;
		}
	}
}
