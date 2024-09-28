//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx Cartridge Class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Lynx cartridge interface, given a filename it    //
// will contstruct a cartridge object via the constructor.                  //
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

#define CART_CPP

#include <stdlib.h>
#include "system.h"
#include "lynxcart.h"

void loadFile(const char * fname, void * dest, int start, int size);

CCart::CCart(const char * gamefile)
{
	LYNX_HEADER header;

	mWriteEnable = FALSE;
	mFileName = gamefile;
	mName = "<Cart not loaded>";
	mManufacturer = "<Cart not loaded>";

	// Open up the file

	if (strlen(mFileName) > 0) {
		loadFile(mFileName, &header, -1, sizeof(LYNX_HEADER));

/*		CFile file(mFileName,CFile::modeRead);
		file.Read((void*)&header,sizeof(LYNX_HEADER));
		file.Close();*/

		// Sanity checks on the header

		if (header.magic[0] != 'L' || header.magic[1] != 'Y' || header.magic[2] != 'N' || header.magic[3] != 'X' || header.version != 1) {
			printf("Couldn't load file **5**\n");
			exit(1);
		}

		// Setup name & manufacturer

		mName = (char *)header.cartname;
		mManufacturer = (char *)header.manufname;

		// Setup rotation

		mRotation = header.rotation;
		if (mRotation != CART_NO_ROTATE && mRotation != CART_ROTATE_LEFT && mRotation != CART_ROTATE_RIGHT)
			mRotation = CART_NO_ROTATE;
	}
	else
	{
		header.page_size_bank0 = 0x000;
		header.page_size_bank1 = 0x000;

		// Setup name & manufacturer

		mName = "<No cart loaded>";
		mManufacturer = "<No cart loaded>";

		// Setup rotation

		mRotation = CART_NO_ROTATE;
	}
  

	// Set the filetypes

	CTYPE banktype0,banktype1;

	switch(header.page_size_bank0)
	{
		case 0x000:
			banktype0=UNUSED;
			mMaskBank0=0;
			mShiftCount0=0;
			mCountMask0=0;
			break;
		case 0x100:
			banktype0=C64K;
			mMaskBank0=0x00ffff;
			mShiftCount0=8;
			mCountMask0=0x0ff;
			break;
		case 0x200:
			banktype0=C128K;
			mMaskBank0=0x01ffff;
			mShiftCount0=9;
			mCountMask0=0x1ff;
			break;
		case 0x400:
			banktype0=C256K;
			mMaskBank0=0x03ffff;
			mShiftCount0=10;
			mCountMask0=0x3ff;
			break;
		case 0x800:
			banktype0=C512K;
			mMaskBank0=0x07ffff;
			mShiftCount0=11;
			mCountMask0=0x7ff;
			break;
		default:
			printf("Couldn't load file **6**\n");
			exit(1);
			break;
	}

	switch(header.page_size_bank1)
	{
		case 0x000:
			banktype1=UNUSED;
			mMaskBank1=0;
			mShiftCount1=0;
			mCountMask1=0;
			break;
		case 0x100:
			banktype1=C64K;
			mMaskBank1=0x00ffff;
			mShiftCount1=8;
			mCountMask1=0x0ff;
			break;
		case 0x200:
			banktype1=C128K;
			mMaskBank1=0x01ffff;
			mShiftCount1=9;
			mCountMask1=0x1ff;
			break;
		case 0x400:
			banktype1=C256K;
			mMaskBank1=0x03ffff;
			mShiftCount1=10;
			mCountMask1=0x3ff;
			break;
		case 0x800:
			banktype1=C512K;
			mMaskBank1=0x07ffff;
			mShiftCount1=11;
			mCountMask1=0x7ff;
			break;
		default:
			printf("Couldn't load file **7**\n");
			exit(1);
			break;
	}

	// Make some space for the new carts

	mCartBank0 = new UBYTE[mMaskBank0+1];
	mCartBank1 = new UBYTE[mMaskBank1+1];

	// Set default bank

	mBank = bank0;

	Reset();  
}

CCart::~CCart()
{
	delete[] mCartBank0;
	delete[] mCartBank1;
}


void CCart::Reset(void)
{
	LYNX_HEADER header;
	ULONG loop;

	// Initialiase

	for (loop=0;loop<mMaskBank0+1;loop++) {
		mCartBank0[loop] = DEFAULT_CART_CONTENTS;
	}
	for (loop=0;loop<mMaskBank1+1;loop++) {
		mCartBank1[loop] = DEFAULT_CART_CONTENTS;
	}
	if (strlen(mFileName) > 0)
	{
		// Open up the file
		loadFile(mFileName, &header, -1, sizeof(LYNX_HEADER));

//		CFile file(mFileName,CFile::modeRead);

		// Read past the header

//		file.Read((void*)&header,sizeof(LYNX_HEADER));

		// Load from file

//		if(mMaskBank0) file.Read(mCartBank0,mMaskBank0+1);
//		if(mMaskBank1) file.Read(mCartBank1,mMaskBank1+1);
		if(mMaskBank0) loadFile(mFileName, mCartBank0, sizeof(LYNX_HEADER), mMaskBank0+1);
		if(mMaskBank1) loadFile(mFileName, mCartBank1, sizeof(LYNX_HEADER), mMaskBank1+1);
/*		if(mMaskBank0) fread(mCartBank0, 1, mMaskBank0+1, fhandle);
		if(mMaskBank1) fread(mCartBank1, 1, mMaskBank1+1, fhandle);*/

//		file.Close();

		// As this is a cartridge boot unset the boot address

		gCPUBootAddress = 0;
	}

	//
	// Check if this is a headerless cart
	//
	mHeaderLess = TRUE;
	for (loop=0;loop<32;loop++) {
		if (mCartBank0[loop & mMaskBank0] != 0x00) mHeaderLess = FALSE;
	}

	mCounter = 0;
	mShifter = 0;
	mAddrData = 0;
	mStrobe = 0;
}

inline void CCart::Poke(ULONG addr, UBYTE data)
{
	if (mWriteEnable)
	{
		if (mBank == bank0)
		{
			mCartBank0[addr & mMaskBank0] = data;
		}
		else
		{
			mCartBank1[addr & mMaskBank1] = data;
		}
	}
}


inline UBYTE CCart::Peek(ULONG addr)
{
	if (mBank == bank0)
	{
		return mCartBank0[addr & mMaskBank0];
	}
	else
	{
		return mCartBank1[addr & mMaskBank1];
	}
}


void CCart::CartAddressStrobe(BOOL strobe)
{
	static BOOL last_strobe = 0;

	mStrobe = strobe;

	if (mStrobe) mCounter = 0;

	//
	// Either of the two below seem to work OK.
	//
	// if(!strobe && last_strobe)
	//
	if (mStrobe && !last_strobe)
	{
		// Clock a bit into the shifter
		mShifter = mShifter<<1;
		mShifter += mAddrData?1:0;
		mShifter &= 0xff;
	}
	last_strobe = mStrobe;
}

void CCart::CartAddressData(BOOL data)
{
	mAddrData = data;
}


void CCart::Poke0(UBYTE data)
{
	if (mWriteEnable)
	{
		ULONG address = (mShifter<<mShiftCount0)+(mCounter & mCountMask0);
		mCartBank0[address & mMaskBank0] = data;
	}
	if (!mStrobe)
	{
		mCounter++;
		mCounter &= 0x07ff;
	}
}

void CCart::Poke1(UBYTE data)
{
	if (mWriteEnable)
	{
		ULONG address = (mShifter<<mShiftCount1)+(mCounter & mCountMask1);
		mCartBank1[address & mMaskBank1] = data;
	}
	if (!mStrobe)
	{
		mCounter++;
		mCounter &= 0x07ff;
	}
}


UBYTE CCart::Peek0(void)
{
	ULONG address = (mShifter<<mShiftCount0)+(mCounter & mCountMask0);
	UBYTE data = mCartBank0[address & mMaskBank0];

	if (!mStrobe)
	{
		mCounter++;
		mCounter &= 0x07ff;
	}

	return data;
}

UBYTE CCart::Peek1(void)
{
	ULONG address = (mShifter<<mShiftCount1)+(mCounter & mCountMask1);
	UBYTE data = mCartBank1[address & mMaskBank1];

	if (!mStrobe)
	{
		mCounter++;
		mCounter &= 0x07ff;
	}

	return data;
}

