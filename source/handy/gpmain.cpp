#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "../Shared/FileHelper.h"
#include "../io.h"

extern "C" void GpInit(const unsigned char *gamerom, int size);
extern "C" void GpDelete(void);
extern "C" void GpMain(void *args);


const char *romfile = "LYNXBOOT.IMG";
//const char *gamefile = "apb - all points bulletin (1990).lnx";
//const char *gamefile = "blue lightning (1989).lnx";
//const char *gamefile = "electrocop (1989).lnx";
//const char *gamefile = "lynx diagnostic cart v0.02 (1989)[crc-2].lnx";
//const char *gamefile = "toki (1990).lnx";

unsigned short vram[2][160*102];
int bufIdx = 0;
bool gScreenUpdateRequired = FALSE;

CSystem *newsystem = NULL;

int loadFile(const char *fname, void *dest, int start, int maxSize) {
	FILE *fHandle;

	findFolder("Lynx");

	if ( !(fHandle = fopen(fname, "r" ))) {
		printf("Couldn't load file **1**\n");
		return -1;
	}
	if (start > 0) {
		fseek(fHandle, start, SEEK_SET);
	}
	int size = fread(dest, 1, maxSize, fHandle);
	fclose(fHandle);
	return size;
}

UBYTE *handy_nds_display_callback(ULONG objref)
{
	unsigned short *srcbuf = vram[bufIdx];
	bufIdx ^= 1;

	for (int y=0;y<102;y++) {
		for (int x=0;x<160;x++) {
			((unsigned short *)0x06000000)[x+(y*256)] = srcbuf[x+(y*160)] | 0x8000;
		}
	}
	gScreenUpdateRequired = TRUE;
	return (UBYTE *)vram[bufIdx];
}


void GpInit(const unsigned char *gamerom, int size) {
	newsystem = new CSystem(gamerom, size, HANDY_FILETYPE_LNX, romfile);
	newsystem->DisplaySetAttributes(0, MIKIE_PIXEL_FORMAT_16BPP_555, 160*2, handy_nds_display_callback, 0);
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
