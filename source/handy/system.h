//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// System object header file                                                //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and inline code for   //
// the system object, this object if what binds together all of the Handy   //
// hardware enmulation objects, its the glue that holds the system together //
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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "nds.h"

#define HANDY_SYSTEM_FREQ						16000000
#define HANDY_AUDIO_SAMPLE_FREQ					22050
#define HANDY_AUDIO_SAMPLE_PERIOD				(HANDY_SYSTEM_FREQ/HANDY_AUDIO_SAMPLE_FREQ)
#define HANDY_AUDIO_BUFFER_FREQ					16
#define HANDY_AUDIO_BUFFER_SIZE					(HANDY_AUDIO_SAMPLE_FREQ/HANDY_AUDIO_BUFFER_FREQ)

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
// Define the global variable list
//
#define gAudioLastUpdateCycle mikey_0.audioLastUpdateCycle
#ifdef SYSTEM_CPP
	u32	gAudioEnabled = false;
	u8	*gAudioBuffer0;
	u8	*gAudioBuffer1;
	u8	*gAudioBuffer2;
	u8	*gAudioBuffer3;
	u32	gAudioBufferPointer = 0;
#else
	extern u32	gAudioEnabled;
	extern u8	*gAudioBuffer0;
	extern u8	*gAudioBuffer1;
	extern u8	*gAudioBuffer2;
	extern u8	*gAudioBuffer3;
	extern u32	gAudioBufferPointer;
#endif


class CSystem;

#include "mikie.h"

class CSystem
{
	public:
		CSystem(void);
		~CSystem();

	public:
		void	Reset(void);

	public:
		CMikie	*mMikie;
};


#endif
