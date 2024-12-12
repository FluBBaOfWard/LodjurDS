#include <nds.h>

#include "system.h"

extern "C" {
void runTimer4(u32 sysCount);
void mikiePoke(u32 addr, u8 data);
u8 mikiePeek(u32 addr);
void GpInit(void);
void GpDelete(void);
}

CSystem *newsystem = NULL;

void runTimer4(u32 sysCount) {
	newsystem->mMikie->UpdateTimer4(sysCount);
}
void mikiePoke(u32 addr, u8 data) {
	newsystem->mMikie->Poke(addr,data);
}
u8 mikiePeek(u32 addr) {
	return newsystem->mMikie->Peek(addr);
}

void GpInit(void) {
	newsystem = new CSystem();
}

void GpDelete() {
	if ( newsystem != NULL) {
		delete newsystem;
		newsystem = NULL;
	}
}
