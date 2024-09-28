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

void loadFile(const char *fname, void *dest, int start, int size);

CRam::CRam(const char *homebrew)
{
	HOME_HEADER	header;

	mFileName = homebrew;
	
	// Open up the file

	if (strlen(mFileName) > 0)
	{
		loadFile(mFileName, &header, -1, sizeof(HOME_HEADER));

		// Sanity checks on the header
		if (header.magic[0]!='B' || header.magic[1]!='S' || header.magic[2]!='9' || header.magic[3]!='3')
		{
			printf("Couldn't load file **3**\n");
			exit(1);
		}
	}

	// Reset will cause the loadup

	Reset();
}

void CRam::Reset(void)
{
	for (int loop=0;loop<RAM_SIZE;loop++) {
		mRamData[loop] = DEFAULT_RAM_CONTENTS;
	}
	// Open up the file

	if (strlen(mFileName) > 0) {
		HOME_HEADER	header;
		UBYTE tmp;
		
		loadFile(mFileName, &header, -1, sizeof(HOME_HEADER));
		
/*		CFile file(mFileName,CFile::modeRead);
		file.Read((void*)&header,sizeof(HOME_HEADER));*/
		// Reverse the bytes in the header words
		tmp = (header.load_address&0xff00)>>8;
		header.load_address = (header.load_address<<8)+tmp;
		tmp = (header.size&0xff00)>>8;
		header.size = (header.size<<8)+tmp;
		// Now we can safely read/manipulate the data
		header.load_address -= 10;

		loadFile(mFileName, (void *)(mRamData+header.load_address), -1, header.size);
/*		fseek(fhandle, 0, SEEK_SET);
		fread((void*)(mRamData+header.load_address), 1, header.size, fhandle);
		fclose(fhandle);*/
/*		file.SeekToBegin();
		file.Read((void*)(mRamData+header.load_address),header.size);
		file.Close();*/
		gCPUBootAddress = header.load_address;
	}
}


//END OF FILE
