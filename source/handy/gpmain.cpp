#include <nds.h>

#include "system.h"

extern "C" {
void runTimer4(u32 sysCount);
void mikiePoke(u32 addr, u8 data);
u8 mikiePeek(u32 addr);
void GpInit(u8 *gamerom, u32 size);
void GpDelete(void);
}

CSystem *newsystem = NULL;

void runTimer4(u32 sysCount) {
	newsystem->mMikie->UpdateTimer4(sysCount);
}
void mikiePoke(u32 addr, u8 data) {
	newsystem->mMikie->Poke(addr,data);
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
