//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx memory map class                                                    //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the register $FFF9 functionality to the emulator, it //
// sets which devices can be seen by the CPU.                               //
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

#define MEMMAP_CPP

#include <stdlib.h>
#include "system.h"
#include "memmap.h"

CMemMap::CMemMap(CSystem& parent)
	:mSystem(parent)
{
	Reset();
}


void CMemMap::Reset(void)
{
	mSusieEnabled = TRUE;
	mMikieEnabled = TRUE;
	mRomEnabled = TRUE;
	mVectorsEnabled = TRUE;
	mSelector = 0;
}

inline void CMemMap::Poke(ULONG addr, UBYTE data)
{
	mSusieEnabled = (data & 0x01)?FALSE:TRUE;
	mMikieEnabled = (data & 0x02)?FALSE:TRUE;
	mRomEnabled = (data & 0x04)?FALSE:TRUE;
	mVectorsEnabled = (data & 0x08)?FALSE:TRUE;
	mSelector = data & 0x0f;
}

inline UBYTE CMemMap::Peek(ULONG addr)
{
	UBYTE retval = 0;

	retval += (mSusieEnabled)?0:0x01;
	retval += (mMikieEnabled)?0:0x02;
	retval += (mRomEnabled)?0:0x04;
	retval += (mVectorsEnabled)?0:0x08;
	return retval;
}

//END OF FILE
