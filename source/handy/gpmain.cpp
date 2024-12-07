#include <nds.h>

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
void GpInit(u8 *gamerom, u32 size);
void GpDelete(void);
void GpMain(void);
}

CSystem *newsystem = NULL;

void runTimer4(u32 sysCount) {
	newsystem->mMikie->UpdateTimer4(sysCount);
}
void cartPoke(u32 addr, u8 data) {
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
void mikiePoke(u32 addr, u8 data) {
	newsystem->mMikie->Poke(addr,data);
}
UBYTE cartPeek(u32 addr) {
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
UBYTE mikiePeek(u32 addr) {
	return newsystem->mMikie->Peek(addr);
}

void GpInit(u8 *gamerom, u32 size) {
	newsystem = new CSystem(gamerom, size);
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}

void GpMain() {
	if (newsystem != NULL) {
		lnxSuzySetButtonData( joy0_R() );
		mikSysUpdate();
	}
}
