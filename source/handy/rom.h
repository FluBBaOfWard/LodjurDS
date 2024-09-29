//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// ROM object header file                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and inline code for   //
// the class the emulates the internal 512 byte ROM embedded in Mikey       //
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

#ifndef ROM_H
#define ROM_H

#define ROM_SIZE				0x200
#define ROM_ADDR_MASK			0x01ff
#define DEFAULT_ROM_CONTENTS	0x88

#define BROM_START		0xfe00
#define BROM_SIZE		0x200
#define VECTOR_START	0xfffa
#define VECTOR_SIZE		0x6

class CRom : public CLynxBase
{
	// Function members

	public:
		/** Supply the Lynx boot image, is 512 bytes long. */
		CRom(const UBYTE *romData);

	public:
		void	Reset(void);

		void	Poke(ULONG addr,UBYTE data) { if (mWriteEnable) mRomData[addr & ROM_ADDR_MASK] = data;};
		UBYTE	Peek(ULONG addr) { return(mRomData[addr & ROM_ADDR_MASK]);};
		ULONG	ReadCycle(void) {return 5;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG	ObjectSize(void) {return ROM_SIZE;};

	// Data members

	public:
		BOOL	mWriteEnable = FALSE;
	private:
		UBYTE	mRomData[ROM_SIZE];
};

#endif
