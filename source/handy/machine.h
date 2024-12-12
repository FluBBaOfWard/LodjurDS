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

// Bytes should be 8-bits wide
typedef unsigned char UBYTE;

// Words should be 16-bits wide
typedef unsigned short UWORD;

// Longs should be 32-bits wide
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

#endif
