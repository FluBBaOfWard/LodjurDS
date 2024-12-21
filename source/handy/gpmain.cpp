#include <nds.h>

#include "system.h"

extern "C" {
//void runTimer4(u32 sysCount);
void updateSound(void);
void GpInit(void);
void GpDelete(void);
}

CSystem *newsystem = NULL;

//void runTimer4(u32 sysCount) {
//	newsystem->mMikie->UpdateTimer4(sysCount);
//}
void updateSound(void) {
	newsystem->mMikie->UpdateSound();
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
