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

#define HANDY_FILETYPE_LNX		0
#define HANDY_FILETYPE_HOMEBREW	1
#define HANDY_FILETYPE_SNAPSHOT	2
#define HANDY_FILETYPE_ILLEGAL	3

#define HANDY_SCREEN_WIDTH	160
#define HANDY_SCREEN_HEIGHT	102
//
// Define the global variable list
//
#define gAudioLastUpdateCycle mikey_0.audioLastUpdateCycle
#ifdef SYSTEM_CPP
	ULONG	gCPUBootAddress = 0;
	ULONG	gSingleStepModeSprites = FALSE;
	BOOL	gEmulatorAbort = FALSE;
	BOOL	gBreakpointHit = FALSE;
	BOOL	gSingleStepMode = FALSE;
	BOOL	gSystemNMI = FALSE;
	BOOL	gSystemHalt = FALSE;

	ULONG	gAudioEnabled = FALSE;
	UBYTE	*gAudioBuffer0; // UBYTE	gAudioBuffer0[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer1; // UBYTE	gAudioBuffer1[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer2; // UBYTE	gAudioBuffer2[HANDY_AUDIO_BUFFER_SIZE];
	UBYTE	*gAudioBuffer3; // UBYTE	gAudioBuffer3[HANDY_AUDIO_BUFFER_SIZE];
	ULONG	gAudioBufferPointer = 0;

	BOOL	gWindowInFocus = TRUE;
#else

	extern ULONG	gCPUBootAddress;
	extern ULONG	gSingleStepModeSprites;
	extern BOOL		gEmulatorAbort;
	extern BOOL		gBreakpointHit;
	extern BOOL		gSingleStepMode;
	extern BOOL		gSystemNMI;
	extern BOOL		gSystemHalt;

	extern ULONG	gAudioEnabled;
	extern UBYTE	*gAudioBuffer0; // extern UBYTE	gAudioBuffer0[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer1; // extern UBYTE	gAudioBuffer1[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer2; // extern UBYTE	gAudioBuffer2[HANDY_AUDIO_BUFFER_SIZE];
	extern UBYTE	*gAudioBuffer3; // extern UBYTE	gAudioBuffer3[HANDY_AUDIO_BUFFER_SIZE];
	extern ULONG	gAudioBufferPointer;

	extern BOOL		gWindowInFocus;
#endif


class CSystem;

#include "sysbase.h"
#include "lynxbase.h"
#include "lynxcart.h"
#include "susie.h"
#include "mikie.h"
#include "../memory.h"
#include "../cpu.h"

#define RAM_SIZE				0x10000
#define DEFAULT_RAM_CONTENTS	0xff

extern "C" {
	extern UBYTE lynxRAM[RAM_SIZE];
}

class CSystem : public CSystemBase
{
	public:
		CSystem(const UBYTE *gamefile, int size, ULONG filetype, const char *romfile);
		~CSystem();

	public:
		void	Reset(void);

		//
		// We MUST have separate CPU & RAM peek & poke handlers as all CPU accesses must
		// go thru the address generator at $FFF9
		//
		// BUT, Mikie video refresh & Susie see the whole system as RAM
		//

// High level cart access for debug etc

		inline void  Poke_CART(ULONG addr, UBYTE data) {mCart->Poke(addr,data);};
		inline UBYTE Peek_CART(ULONG addr) {return mCart->Peek(addr);};
		inline void  CartBank(EMMODE bank) {mCart->BankSelect(bank);};
		inline ULONG CartSize(void) {return mCart->ObjectSize();};
		inline const char *CartGetName(void) { return mCart->CartGetName();};
		inline const char *CartGetManufacturer(void) { return mCart->CartGetManufacturer();};
		inline ULONG CartGetRotate(void) {return mCart->CartGetRotate();};

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

// Suzy system interfacing

		ULONG	PaintSprites(void) {return mSusie->PaintSprites();};

// Miscellaneous

		void	SetButtonData(ULONG data) {mSusie->SetButtonData(data);};
		ULONG	GetButtonData(void) {return mSusie->GetButtonData();};
		void	SetCycleBreakpoint(ULONG breakpoint) {mCycleCountBreakpoint = breakpoint;};
		UBYTE	*GetRamPointer(void) {return lynxRAM;};

	public:
		ULONG			mCycleCountBreakpoint;
		CCart			*mCart;
		CMikie			*mMikie;
		CSusie			*mSusie;

		ULONG			mFileType;
};


#endif
