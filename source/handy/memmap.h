//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx memory map object header file                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the memory map    //
// object, this object controls which pieces of lynx hardware are           //
// accesible by the CPU at any given time, it is the code for addr $FFF9    //
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

#ifndef MEMMAP_H
#define MEMMAP_H

#define MEMMAP_SIZE				0x1


class CMemMap : public CLynxBase
{

	// Function members

	public:
		CMemMap(CSystem& parent);

	public:
		void	Reset(void);
		void	Poke(ULONG addr,UBYTE data);
		UBYTE	Peek(ULONG addr);
		ULONG	ReadCycle(void) {return 5;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG	ObjectSize(void) {return MEMMAP_SIZE;};

	// Data members

	public:
		UBYTE			mSelector;

	private:
		BOOL			mMikieEnabled;
		BOOL			mSusieEnabled;
		BOOL			mRomEnabled;
		BOOL			mVectorsEnabled;

		CSystem& mSystem;
};

#endif
