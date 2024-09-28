//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// ROM emulation class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the system ROM (512B), the interface is pretty       //
// simple: constructor, reset, peek, poke.                                  //
//                                                                          //
// Keith Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define ROM_CPP

#include <stdlib.h>
#include "machine.h"
#include "rom.h"

void loadFile(const char *fname, void *dest, int start, int size);

CRom::CRom(const char *romfile)
{
	mWriteEnable = FALSE;
	mFileName = romfile;
	Reset();

	// Check the code that has been loaded and report an error if its a
	// fake version of the bootrom

	const UBYTE mRomCheck[16] = {0x38,0x80,0x0A,0x90,0x04,0x8E,0x8B,0xFD,
								 0x18,0xE8,0x8E,0x87,0xFD,0xA2,0x02,0x8E};

	static BOOL firsttime = TRUE;

	if (firsttime)
	{
		firsttime = FALSE;
		for (ULONG loop=0; loop<16; loop++) {
			if (mRomCheck[loop] != mRomData[loop]) {
				printf(
				"\n"
				"FAKE LYNXBOOT.IMG DETECTED - CARTRIDGES WILL NOT WORK\n\n"
				"The lynx boot ROM image you have specificed has not been "
				"recognised by Handy. If it is the image that is supplied "
				"with the Handy distribution then it is a not the real "
				"LYNXBOOT.IMG and as such you will not be able to load "
				"cartridges. You will need to find the real image from "
				"somewhere. (Do not email the author asking for this image)\n");
				break;
			}
		}
	}
}

void CRom::Reset(void)
{
	for (int loop=0; loop<ROM_SIZE; loop++) {
		mRomData[loop] = DEFAULT_ROM_CONTENTS;
	}
	// Load up the file
	loadFile(mFileName, mRomData, -1, ROM_SIZE);
//	CFile file(mFileName,CFile::modeRead);

	// Read in the 512 bytes

//	file.Read(mRomData,ROM_SIZE);

//	file.Close();
}


//END OF FILE
