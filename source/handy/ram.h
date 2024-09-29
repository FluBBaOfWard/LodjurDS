//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// RAM object header file                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the RAM class     //
// that emulates the Handy system RAM (64K)                                 //
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

#ifndef RAM_H
#define RAM_H

#define RAM_SIZE				65536
#define RAM_ADDR_MASK			0xffff
#define DEFAULT_RAM_CONTENTS	0xff

typedef struct
{
   UWORD   jump;
   UWORD   load_address;
   UWORD   size;
   UBYTE   magic[4];
}HOME_HEADER;

class CRam : public CLynxBase
{

	// Function members

	public:
		CRam(const UBYTE *filememory, ULONG filesize);

	public:

		void	Reset(void);

		void	Poke(ULONG addr, UBYTE data){ mRamData[addr] = data;};
		UBYTE	Peek(ULONG addr){ return(mRamData[addr]);};
		ULONG	ReadCycle(void) {return 5;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG   ObjectSize(void) {return RAM_SIZE;};
		UBYTE	*GetRamPointer(void) { return mRamData; };

	// Data members

	private:
		UBYTE	mRamData[RAM_SIZE];
		const UBYTE *mFileData;
		ULONG	mFileSize;

};

#endif
