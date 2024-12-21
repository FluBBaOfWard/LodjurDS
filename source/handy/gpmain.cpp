#include <nds.h>

#include "system.h"

extern "C" {
void updateSound(void);
void GpInit(void);
}

CSystem *newsystem = NULL;

void updateSound(void) {
	newsystem->mMikie->UpdateSound();
}

void GpInit(void) {
	newsystem = new CSystem();
}
