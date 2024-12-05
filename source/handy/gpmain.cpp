#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "lynxdef.h"
#include "../Shared/FileHelper.h"
#include "../io.h"
#include "../Gfx.h"
#include "../ARMMikey/ARMMikey.h"

extern "C" {
void runTimer4(u32 sysCount);
void cartPoke(u32 addr, u8 data);
void mikiePoke(u32 addr, u8 data);
u8 cartPeek(u32 addr);
u8 mikiePeek(u32 addr);
void GpInit(unsigned char *gamerom, int size);
void GpDelete(void);
void GpMain(void);
}

bool gScreenUpdateRequired = false;

CSystem *newsystem = NULL;

void runTimer4(u32 sysCount) {
	newsystem->mMikie->UpdateTimer4(sysCount);
}
void cartPoke(ULONG addr, UBYTE data) {
	switch(addr & 0xff)
	{
// Cartridge writing ports
		case (RCART0 & 0xff):
			newsystem->Poke_CARTB0(data);
			break;
		case (RCART1 & 0xff):
			newsystem->Poke_CARTB1(data);
			break;
	}
}
void mikiePoke(ULONG addr, UBYTE data) {
	newsystem->mMikie->Poke(addr,data);
}
UBYTE cartPeek(ULONG addr) {
	switch(addr & 0xff)
	{
// Cartridge reading ports
		case (RCART0 & 0xff):
			return newsystem->Peek_CARTB0();
		case (RCART1 & 0xff):
			return newsystem->Peek_CARTB1();
	}
	return 0xff;
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
