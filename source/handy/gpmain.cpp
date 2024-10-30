#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "../Shared/FileHelper.h"
#include "../io.h"
#include "../Gfx.h"

extern "C" {
u32 paintSprites(void);
void runTimer4(void);
void susiePoke(u32 addr, u8 data);
void mikiePoke(u32 addr, u8 data);
u8 susiePeek(u32 addr);
u8 mikiePeek(u32 addr);
void GpInit(const unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void);
}

u16 *currentDest;
bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

u32 paintSprites() {
	return newsystem->mSusie->PaintSprites();
}
void runTimer4() {
	newsystem->mMikie->UpdateTimer4();
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
	currentDest = ((unsigned short *)0x06000000);
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}

void GpMain() {
	int hazard = 0;
	while (newsystem != NULL) {

		newsystem->SetButtonData( joy0_R() );
		for (int i=0;i<1024;i++) {
			mikSysUpdate();
		}

		hazard += 1;
		if (gScreenUpdateRequired || hazard > 30) {
			gScreenUpdateRequired = FALSE;
			return;
		}
	}
}
