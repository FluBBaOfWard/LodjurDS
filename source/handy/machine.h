//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Core machine definitions header file                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for the core //
// definitions used throughout the Handy code. Additionally it provides     //
// a generic memory object definition.                                      //
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


#ifndef MACHINE_H
#define MACHINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Bytes should be 8-bits wide
typedef signed char SBYTE;
typedef unsigned char UBYTE;

// Words should be 16-bits wide
typedef signed short SWORD;
typedef unsigned short UWORD;

// Longs should be 32-bits wide
typedef signed long SLONG;
typedef unsigned long ULONG;

// Read/Write Cycle definitions
#define CPU_RDWR_CYC	5
#define DMA_RDWR_CYC	4
#define SPR_RDWR_CYC	3
// Ammended to 2 on 28/04/00, 16Mhz = 62.5nS cycle
//
//    2 cycles is 125ns - PAGE MODE CYCLE
//    4 cycles is 250ns - NORMAL MODE CYCLE
//

// Average length of a read/write cycle
#define AVE_RDWR_CYC	5

//
// Generic Memory object base class.
//

#include "lynxbase.h"

class CLynxMemObj
{
	// Function members

	public:
		virtual ~CLynxMemObj() {};

	public:
	virtual void	Reset(void) {};
		virtual void	Poke(ULONG addr, UBYTE data) = 0;
		virtual UBYTE	Peek(ULONG addr) = 0;
		virtual void	PokeW(ULONG addr, UWORD data) {};	// ONLY mSystem overloads these, they are never used by the clients
		virtual UWORD	PeekW(ULONG addr) {return 0;};
		virtual ULONG	ReadCycle(void) {return 0;};
		virtual ULONG	WriteCycle(void) {return 0;};
		virtual void	BankSelect(EMMODE newbank){};
		virtual ULONG	ObjectSize(void) {return 1;};
};

#endif
