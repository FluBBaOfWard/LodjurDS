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

CRom::CRom(const UBYTE *romData)
{
	mWriteEnable = FALSE;
	Reset();

	// Load up the file
	memcpy(mRomData, romData, ROM_SIZE);
	// Read in the 512 bytes

	// Check the code that has been loaded and report an error if its a
	// fake version of the bootrom

	const UBYTE mRomCheck[16] = {0x38,0x80,0x0A,0x90,0x04,0x8E,0x8B,0xFD,
								 0x18,0xE8,0x8E,0x87,0xFD,0xA2,0x02,0x8E};

	for (ULONG loop=0; loop<16; loop++) {
		if (mRomCheck[loop] != mRomData[loop]) {
			printf("FAKE LYNXBOOT.IMG - CARTRIDGES WILL NOT WORK\n\n"
				   "PLEASE READ THE ACCOMPANYING README.TXT FILE\n\n"
				   "(Do not email the author asking for this image)\n");
			break;
		}
	}
}

void CRom::Reset(void)
{
	// Nothing to do here
}

//END OF FILE
