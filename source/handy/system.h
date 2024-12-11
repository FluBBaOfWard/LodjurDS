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

#include "machine.h"

#define HANDY_SYSTEM_FREQ						16000000
#define HANDY_AUDIO_SAMPLE_FREQ					22050
#define HANDY_AUDIO_SAMPLE_PERIOD				(HANDY_SYSTEM_FREQ/HANDY_AUDIO_SAMPLE_FREQ)
#define HANDY_AUDIO_BUFFER_FREQ					16
#define HANDY_AUDIO_BUFFER_SIZE					(HANDY_AUDIO_SAMPLE_FREQ/HANDY_AUDIO_BUFFER_FREQ)

//
// Define the global variable list
//
#define gAudioLastUpdateCycle mikey_0.audioLastUpdateCycle
#ifdef SYSTEM_CPP
	ULONG	gAudioEnabled = FALSE;
	UBYTE	*gAudioBuffer0; // UBYTE	gAudioBuffer0[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer1; // UBYTE	gAudioBuffer1[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer2; // UBYTE	gAudioBuffer2[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer3; // UBYTE	gAudioBuffer3[HANDY_AUDIO_BUFFER_SIZE];
	ULONG	gAudioBufferPointer = 0;
#else
	extern ULONG	gAudioEnabled;
	extern UBYTE	*gAudioBuffer0; // extern UBYTE	gAudioBuffer0[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer1; // extern UBYTE	gAudioBuffer1[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer2; // extern UBYTE	gAudioBuffer2[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer3; // extern UBYTE	gAudioBuffer3[HANDY_AUDIO_BUFFER_SIZE];
	extern ULONG	gAudioBufferPointer;
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

// Mikey system interfacing

		void	ComLynxCable(int status) { mMikie->ComLynxCable(status); };
		void	ComLynxRxData(int data)  { mMikie->ComLynxRxData(data); };
		void	ComLynxTxCallback(void (*function)(int data, ULONG objref), ULONG objref) { mMikie->ComLynxTxCallback(function, objref); };

	public:
		CMikie			*mMikie;
};


#endif
