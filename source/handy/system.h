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
#define HANDY_TIMER_FREQ						20
#define HANDY_AUDIO_SAMPLE_FREQ					22050
#define HANDY_AUDIO_SAMPLE_PERIOD				(HANDY_SYSTEM_FREQ/HANDY_AUDIO_SAMPLE_FREQ)
#define HANDY_AUDIO_BUFFER_FREQ					16
#define HANDY_AUDIO_BUFFER_SIZE					(HANDY_AUDIO_SAMPLE_FREQ/HANDY_AUDIO_BUFFER_FREQ)
#define HANDY_AUDIO_WAVESHAPER_TABLE_LENGTH		0x200000

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

#include "sysbase.h"
#include "lynxbase.h"
#include "lynxcart.h"
#include "mikie.h"

#define RAM_SIZE				0x10000
#define DEFAULT_RAM_CONTENTS	0xff

class CSystem : public CSystemBase
{
	public:
		CSystem(UBYTE *gamefile, int size);
		~CSystem();

	public:
		void	Reset(void);

// Low level cart access for Suzy, Mikey

		inline void  Poke_CARTB0(UBYTE data) {mCart->Poke0(data);};
		inline void  Poke_CARTB1(UBYTE data) {mCart->Poke1(data);};
		inline UBYTE Peek_CARTB0(void) {return mCart->Peek0();}
		inline UBYTE Peek_CARTB1(void) {return mCart->Peek1();}
		inline void  CartAddressStrobe(BOOL strobe) {mCart->CartAddressStrobe(strobe);};
		inline void  CartAddressData(BOOL data) {mCart->CartAddressData(data);};

// Mikey system interfacing

		void	ComLynxCable(int status) { mMikie->ComLynxCable(status); };
		void	ComLynxRxData(int data)  { mMikie->ComLynxRxData(data); };
		void	ComLynxTxCallback(void (*function)(int data, ULONG objref), ULONG objref) { mMikie->ComLynxTxCallback(function, objref); };

	public:
		CCart			*mCart;
		CMikie			*mMikie;
};


#endif
