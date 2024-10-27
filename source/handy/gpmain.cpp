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
void susiePoke(ULONG addr, UBYTE data);
void mikiePoke(ULONG addr, UBYTE data);
UBYTE susiePeek(ULONG addr);
UBYTE mikiePeek(ULONG addr);
void GpInit(const unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void *args);
}

u16 *currentDest;
bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

void handy_nds_render_callback(UBYTE *ram, ULONG *palette, bool flip) {
	lodjurRenderCallback(ram , palette, flip);
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
	newsystem->DisplaySetAttributes(handy_nds_render_callback);
	currentDest = ((unsigned short *)0x06000000);
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}

void GpMain(void *args) {

	int hazard = 0;
	while (newsystem != NULL) {

		for (int i=0;i<1024;i++) {
			newsystem->Update();
		}
		newsystem->SetButtonData( joy0_R() );

		hazard += 1;
		if (gScreenUpdateRequired || hazard > 30) {
			gScreenUpdateRequired = FALSE;
			return;
		}
	}

}
