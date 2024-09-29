//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx cartridge class header file                                         //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for some of  //
// the simpler cartridge API.                                               //
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

#ifndef CART_H
#define CART_H

#define DEFAULT_CART_CONTENTS	0x11

enum CTYPE {UNUSED,C64K,C128K,C256K,C512K,C1024K};

#define CART_NO_ROTATE		0
#define CART_ROTATE_LEFT	1
#define	CART_ROTATE_RIGHT	2

typedef struct
{
   UBYTE   magic[4];
   UWORD   page_size_bank0;
   UWORD   page_size_bank1;
   UWORD   version;
   UBYTE   cartname[32];
   UBYTE   manufname[16];
   UBYTE   rotation; 
   UBYTE   spare[5];
}LYNX_HEADER;


class CCart : public CLynxMemObj
{

	// Function members

	public:
		CCart(const char * gamefile);
		~CCart();

	public:

// Access for sensible members of the clan

		void	Reset(void);

		void	Poke(ULONG addr,UBYTE data);
		UBYTE	Peek(ULONG addr);
		ULONG	ReadCycle(void) {return 15;};
		ULONG	WriteCycle(void) {return 15;};
		void	BankSelect(EMMODE newbank) {mBank = newbank;}
		ULONG	ObjectSize(void) {return (mBank == bank0)?mMaskBank0+1:mMaskBank1+1;};

		void	CartGetName(const char ** name) { (*name)=mName;};
		void	CartGetManufacturer(const char ** manuf) { (*manuf)=mManufacturer;};
		ULONG	CartGetRotate(void) { return mRotation;};
		BOOL	CartHeaderLess(void) { return mHeaderLess;};

// Access for the lynx itself, it has no idea of address etc as this is done by the
// cartridge emulation hardware 
		void	CartAddressStrobe(BOOL strobe);
		void	CartAddressData(BOOL data);
		void	Poke0(UBYTE data);
		void	Poke1(UBYTE data);
		UBYTE	Peek0(void);
		UBYTE	Peek1(void);

	// Data members

	public:
		ULONG	mWriteEnableBank0;
		ULONG	mWriteEnableBank1;
		ULONG	mCartRAM;

	private:
		EMMODE	mBank;
		ULONG	mMaskBank0;
		ULONG	mMaskBank1;
		UBYTE	*mCartBank0;
		UBYTE	*mCartBank1;
		const char	*mFileName;
		const char	*mName;
		const char	*mManufacturer;
		ULONG	mRotation;
		BOOL	mHeaderLess;

		ULONG	mCounter;
		ULONG	mShifter;
		BOOL	mAddrData;
		BOOL	mStrobe;

		ULONG	mShiftCount0;
		ULONG	mCountMask0;
		ULONG	mShiftCount1;
		ULONG	mCountMask1;

};

#endif
