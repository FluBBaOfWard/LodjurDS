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

CCart::CCart(UBYTE *gameData, ULONG gameSize)
{
	LYNX_HEADER header;

	mWriteEnableBank0 = FALSE;
	mWriteEnableBank1 = FALSE;
	mCartRAM = FALSE;
	mHeaderLess = FALSE;

	// Open up the file

	if (gameSize) {
		// Checkout the header bytes
		memcpy(&header, gameData, sizeof(LYNX_HEADER));

#ifdef MSB_FIRST
		header.page_size_bank0 = ((header.page_size_bank0>>8) | (header.page_size_bank0<<8));
		header.page_size_bank1 = ((header.page_size_bank1>>8) | (header.page_size_bank1<<8));
		header.version         = ((header.version>>8) | (header.version<<8));
#endif
		// Sanity checks on the header

		if (header.magic[0] != 'L' || header.magic[1] != 'Y' || header.magic[2] != 'N' || header.magic[3] != 'X' || header.version != 1) {

			header.page_size_bank0 = 0;
			header.page_size_bank1 = 0;

			// Setup name & manufacturer
			strlcpy(mName, "<No cart loaded>", sizeof(mName));
			strlcpy(mManufacturer, "<No cart loaded>", sizeof(mManufacturer));

			// Setup rotation
			mRotation = CART_NO_ROTATE;
		}
		else {

			// Setup name & manufacturer
			strlcpy(mName, (char *)&header.cartname, sizeof(mName));
			strlcpy(mManufacturer, (char *)&header.manufname, sizeof(mManufacturer));

			// Setup rotation

			mRotation = header.rotation;
			if (mRotation != CART_NO_ROTATE && mRotation != CART_ROTATE_LEFT && mRotation != CART_ROTATE_RIGHT) {
				mRotation = CART_NO_ROTATE;
			}
		}
	}
	else {
		header.page_size_bank0 = 0x000;
		header.page_size_bank1 = 0x000;

		// Setup name & manufacturer
		strlcpy(mName, "<No cart loaded>", sizeof(mName));
		strlcpy(mManufacturer, "<No cart loaded>", sizeof(mManufacturer));

		// Setup rotation
		mRotation = CART_NO_ROTATE;
	}


	// Set the filetypes

//	CTYPE banktype0;  // unused
	CTYPE banktype1;

	switch(header.page_size_bank0)
	{
		case 0x000:
//			banktype0=UNUSED;
			mMaskBank0=0;
			mShiftCount0=0;
			mCountMask0=0;
			break;
		case 0x100:
//			banktype0=C64K;
			mMaskBank0=0x00ffff;
			mShiftCount0=8;
			mCountMask0=0x0ff;
			break;
		case 0x200:
//			banktype0=C128K;
			mMaskBank0=0x01ffff;
			mShiftCount0=9;
			mCountMask0=0x1ff;
			break;
		case 0x400:
//			banktype0=C256K;
			mMaskBank0=0x03ffff;
			mShiftCount0=10;
			mCountMask0=0x3ff;
			break;
		case 0x800:
//			banktype0=C512K;
			mMaskBank0=0x07ffff;
			mShiftCount0=11;
			mCountMask0=0x7ff;
			break;
		default:
			mMaskBank0 = 0;
			mShiftCount0 = 0;
			mCountMask0 = 0;
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
			banktype1 = UNUSED;
			mMaskBank1 = 0;
			mShiftCount1 = 0;
			mCountMask1 = 0;
			break;
	}

	// Set default bank
	mBank = bank0;

	// Initialize
	int cartsize = int(gameSize - sizeof(LYNX_HEADER));
	if (cartsize < 0) cartsize = 0;
	int bank0size = (cartsize == 0) ? 0 : (int)(mMaskBank0+1);
	int bank1size = (cartsize == 0) ? 0 : (int)(mMaskBank1+1);

	mCartBank0 = gameData+sizeof(LYNX_HEADER);
	mCartBank1 = mCartBank0+bank0size;

	memset(mCartBank0 + bank0size, DEFAULT_CART_CONTENTS, mMaskBank0+1 - bank0size);
	memset(mCartBank1 + bank1size, DEFAULT_CART_CONTENTS, mMaskBank1+1 - bank1size);

	// Copy the cart banks from the image
	if (gameSize) {
		// As this is a cartridge boot unset the boot address

		gCPUBootAddress = 0;

		//
		// Check if this is a headerless cart
		//
		mHeaderLess = TRUE;
		for (int loop=0;loop<32;loop++) {
			if (mCartBank0[loop & mMaskBank0] != 0x00) mHeaderLess = FALSE;
		}
		TRACE_CART1("CCart() - mHeaderLess=%d", mHeaderLess);
	}

	// Dont allow an empty Bank1 - Use it for shadow SRAM/EEPROM
	if (banktype1 == UNUSED) {
		// Delete the single byte allocated  earlier
		// Allocate some new memory for us
		TRACE_CART0("CCart() - Bank1 being converted to 64K SRAM");
		banktype1 = C64K;
		mMaskBank1 = 0x00ffff;
		mShiftCount1 = 8;
		mCountMask1 = 0x0ff;
		memset(mCartBank1, DEFAULT_RAM_CONTENTS, mMaskBank1+1);
		mWriteEnableBank1 = TRUE;
		mCartRAM = TRUE;
	}
}

CCart::~CCart()
{
}

void CCart::Reset(void)
{
	mCounter = 0;
	mShifter = 0;
	mAddrData = 0;
	mStrobe = 0;
}

inline void CCart::Poke(ULONG addr, UBYTE data)
{
	if (mBank == bank0) {
		if (mWriteEnableBank0) mCartBank0[addr & mMaskBank0] = data;
	}
	else {
		if (mWriteEnableBank1) mCartBank1[addr & mMaskBank1] = data;
	}
}


inline UBYTE CCart::Peek(ULONG addr)
{
	if (mBank == bank0) {
		return mCartBank0[addr & mMaskBank0];
	}
	else {
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
	if (mStrobe && !last_strobe) {
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
	if (mWriteEnableBank0) {
		ULONG address = (mShifter<<mShiftCount0)+(mCounter & mCountMask0);
		mCartBank0[address & mMaskBank0] = data;
	}
	if (!mStrobe) {
		mCounter++;
		mCounter &= 0x07ff;
	}
}

void CCart::Poke1(UBYTE data)
{
	if (mWriteEnableBank1) {
		ULONG address = (mShifter<<mShiftCount1)+(mCounter & mCountMask1);
		mCartBank1[address & mMaskBank1] = data;
	}
	if (!mStrobe) {
		mCounter++;
		mCounter &= 0x07ff;
	}
}


UBYTE CCart::Peek0(void)
{
	ULONG address = (mShifter<<mShiftCount0)+(mCounter & mCountMask0);
	UBYTE data = mCartBank0[address & mMaskBank0];

	if (!mStrobe) {
		mCounter++;
		mCounter &= 0x07ff;
	}

	return data;
}

UBYTE CCart::Peek1(void)
{
	ULONG address = (mShifter<<mShiftCount1)+(mCounter & mCountMask1);
	UBYTE data = mCartBank1[address & mMaskBank1];

	if (!mStrobe) {
		mCounter++;
		mCounter &= 0x07ff;
	}

	return data;
}

