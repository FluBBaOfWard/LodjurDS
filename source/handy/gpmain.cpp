#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "system.h"
#include "../Shared/FileHelper.h"

extern "C" void GpMain(void *args);


const char *romfile = "LYNXBOOT.IMG";
//const char *gamefile = "apb - all points bulletin (1990).lnx";
//const char *gamefile = "blue lightning (1989).lnx";
//const char *gamefile = "electrocop (1989).lnx";
//const char *gamefile = "lynx diagnostic cart v0.02 (1989)[crc-2].lnx";
const char *gamefile = "toki (1990).lnx";

unsigned short vram[2][160*102];
int bufIdx = 0;
/*
extern "C" {
  void write(int ff, char *bla, int fd)
  {
  }
}
*/
void loadFile(const char *fname, void *dest, int start, int size) {
	FILE *fhandle;

	findFolder("Lynx");

	if ( !(fhandle = fopen(fname, "r" ))) {
		printf("Couldn't load file **1**\n");
		exit(1);
	}
	if (start > 0) {
		fseek(fhandle, start, SEEK_SET);
	}
	fread(dest, 1, size, fhandle);
	fclose(fhandle);
}

UBYTE *handy_nds_display_callback(ULONG objref)
{
//	handy_3ds_video_flip();  //mainSurface );
	unsigned short *srcbuf = vram[bufIdx];
	bufIdx ^= 1;

	for (int y=0;y<102;y++) {
		for (int x=0;x<160;x++) {
			((unsigned short *)0x06000000)[x+(y*256)] = srcbuf[x+(y*160)] | 0x8000;
		}
	}
	return (UBYTE *)vram[bufIdx];
}


void GpMain(void *args) {

	CSystem *newsystem = new CSystem(gamefile, HANDY_FILETYPE_LNX, romfile);
	newsystem->SetScreenAttributes(MIKIE_BITMAP_NORMAL_BPP16_X1, 160, 102, 0, 0, (UBYTE*)vram[0], (UBYTE*)vram[1]);

	while (1) {
		int i,j;

		for (i=0;i<1024;i++) {
			newsystem->Update();
		}
		gTimerCount++;
		newsystem->SetButtonData( 0 );

		if (gScreenUpdateRequired) {
			unsigned short *srcbuf;

			gScreenUpdateRequired = FALSE;

			if (newsystem->GetDisplayBuffer()) {
				srcbuf = vram[0];
			}
			else {
				srcbuf = vram[1];
			}

			int y=0;
			for (i=0;i<102;i++) {
				int x=0;
				for (j=0;j<160;j++) {
					((unsigned short *)0x06000000)[x+(y*256)] = srcbuf[x+(y*160)] | 0x8000;
					x++;
				}
				y++;
			}
		}
	}

}
