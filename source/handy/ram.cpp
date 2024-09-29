//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// RAM emulation class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the system RAM (64KB), the interface is pretty       //
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


#define RAM_CPP

#include <stdlib.h>
#include "system.h"
#include "ram.h"

CRam::CRam(const UBYTE *filememory, ULONG filesize)
{
	HOME_HEADER	header;

	mFileSize = filesize;

	if (filesize) {
		mFileData = filememory;

		// Sanity checks on the header
		memcpy(&header, mFileData, sizeof(HOME_HEADER));

		if (header.magic[0]!='B' || header.magic[1]!='S' || header.magic[2]!='9' || header.magic[3]!='3') {
/*
			CLynxException lynxerr;
			lynxerr.Message() << "Handy Error: File format invalid (Magic No)";
			lynxerr.Description()
				<< "The image you selected was not a recognised homebrew format." << endl
				<< "(see the Handy User Guide for more information).";
			throw(lynxerr);
*/
		}
	}
	// Reset will cause the loadup

	Reset();
}

void CRam::Reset(void)
{
	// Open up the file

	if (mFileSize >= sizeof(HOME_HEADER)) {
		HOME_HEADER	header;
		UBYTE tmp;

		// Reverse the bytes in the header words
		memcpy(&header, mFileData, sizeof(HOME_HEADER));
		tmp = (header.load_address & 0xff00) >> 8;
		header.load_address = (header.load_address << 8) + tmp;
		tmp = (header.size & 0xff00) >> 8;
		header.size = (header.size << 8) + tmp;

		// Now we can safely read/manipulate the data
		header.load_address -= 10;

		int data_size = (int)header.size;
		if (data_size > (int)mFileSize) data_size = (int)mFileSize;
		memset(mRamData, 0x00, header.load_address);
		memcpy(mRamData+header.load_address, mFileData, data_size);
		memset(mRamData+header.load_address+data_size, 0x00, RAM_SIZE-header.load_address-data_size);
		gCPUBootAddress=header.load_address;
	}
	else {
		memset(mRamData, DEFAULT_RAM_CONTENTS, RAM_SIZE);
	}
}

//END OF FILE
