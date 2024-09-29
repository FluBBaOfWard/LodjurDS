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

#ifdef TRACE_CART

#define TRACE_CART0(msg)					_RPT1(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_CART1(msg,arg1)				_RPT2(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_CART2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_CART3(msg,arg1,arg2,arg3)		_RPT4(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_CART0(msg)
#define TRACE_CART1(msg,arg1)
#define TRACE_CART2(msg,arg1,arg2)
#define TRACE_CART3(msg,arg1,arg2,arg3)

#endif

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
   char    cartname[32];
   char    manufname[16];
   UBYTE   rotation;
   UBYTE   spare[5];
}LYNX_HEADER;


class CCart : public CLynxBase
{

	// Function members

	public:
		/** gameData is used directly so don't get rid of it before running the destructor */
		CCart(UBYTE *gameData, ULONG gameSize);
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

		const char *CartGetName(void) { return mName;};
		const char *CartGetManufacturer(void) { return mManufacturer; };
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
		char	mName[33];
		char	mManufacturer[17];
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
