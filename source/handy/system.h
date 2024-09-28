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
#define HANDY_FILETYPE_ILLEGAL	2

//
// Define the global variable list
//

#ifdef SYSTEM_CPP
	ULONG	gSystemCycleCount = 0;
	ULONG	gNextTimerEvent = 0;
	ULONG	gCPUBootAddress = 0;
	BOOL	gScreenUpdateRequired = FALSE;
	BOOL	gEmulatorAbort = FALSE;
	BOOL	gBreakpointHit = FALSE;
	BOOL	gSingleStepMode = FALSE;
	BOOL	gSystemIRQ = FALSE;
	BOOL	gSystemNMI = FALSE;
	BOOL	gSystemCPUSleep = FALSE;
	BOOL	gSystemCPUSleep_Saved = FALSE;
	BOOL	gSystemHalt = FALSE;
	ULONG	gFrameCount = 0;
	ULONG	gThrottleMaxPercentage = 100;
	ULONG	gThrottleLastTimerCount = 0;
	ULONG	gThrottleNextCycleCheckpoint = 0;

	volatile ULONG gTimerCount = 0;
	volatile ULONG gEmulationSpeed = 0;
	volatile ULONG gFramesPerSecond = 0;

	BOOL	gAudioEnabled = FALSE;
	UBYTE	gAudioBuffer[2][HANDY_AUDIO_BUFFER_SIZE];
	ULONG	gAudioPlaybackBufferNumber = 0;
	ULONG	gAudioLastUpdateCycle = 0;
	ULONG	*gAudioWaveShaperLookupTable = NULL;

	BOOL	gWindowInFocus = TRUE;
#else

	extern ULONG	gSystemCycleCount;
	extern ULONG	gNextTimerEvent;
	extern ULONG	gCPUBootAddress;
	extern BOOL		gScreenUpdateRequired;
	extern BOOL		gEmulatorAbort;
	extern BOOL		gBreakpointHit;
	extern BOOL		gSingleStepMode;
	extern BOOL		gSystemIRQ;
	extern BOOL		gSystemNMI;
	extern BOOL		gSystemCPUSleep;
	extern BOOL		gSystemCPUSleep_Saved;
	extern BOOL		gSystemHalt;
	extern ULONG	gFrameCount;
	extern ULONG	gThrottleMaxPercentage;
	extern ULONG	gThrottleLastTimerCount;
	extern ULONG	gThrottleNextCycleCheckpoint;

	extern volatile ULONG gTimerCount;
	extern volatile ULONG gEmulationSpeed;
	extern volatile ULONG gFramesPerSecond;

	extern BOOL		gAudioEnabled;
	extern UBYTE	gAudioBuffer[2][HANDY_AUDIO_BUFFER_SIZE];
	extern ULONG	gAudioPlaybackBufferNumber;
	extern ULONG	gAudioLastUpdateCycle;
	extern ULONG	*gAudioWaveShaperLookupTable;

	extern BOOL		gWindowInFocus;
#endif


class CSystem;

#include "sysbase.h"
#include "ram.h"
#include "rom.h"
#include "memmap.h"
#include "lynxcart.h"
#include "susie.h"
#include "mikie.h"
#include "c65c02.h"

#define TOP_START	0xfc00
#define TOP_MASK	0x03ff
#define TOP_SIZE	0x400
#define SYSTEM_SIZE	65536

class CSystem : public CSystemBase
{
	public:
		CSystem(const char *gamefile, ULONG filetype, const char *romfile);
		~CSystem();

	public:
		void	Reset(void);

		//
		// **********************************
		// **** WARNING WARNIING WARNING ****
		// **********************************
		//
		// This function IS recursively called when sprite painting
		// occurs. Call points are:
		//
		//		CLynxWindow::Update()
		//		CMikie::Poke(CPUSLEEP)
		//

		inline void Update(void)
		{
			// 
			// Only update if there is a predicted timer event
			//
			if (gSystemCycleCount >= gNextTimerEvent)
			{
				mMikie->Update();
			}
			//
			// Step the processor through 1 instruction
			//
			mCpu->Update();

#ifdef _LYNXDBG
			// Check breakpoint
			static ULONG lastcycle = 0;
			if (lastcycle < mCycleCountBreakpoint && gSystemCycleCount >= mCycleCountBreakpoint) gBreakpointHit = TRUE;
			lastcycle = gSystemCycleCount;

			// Check single step mode
			if (gSingleStepMode) gBreakpointHit = TRUE;
#endif
		}

		//
		// We MUST have separate CPU & RAM peek & poke handlers as all CPU accesses must
		// go thru the address generator at $FFF9
		//
		// BUT, Mikie video refresh & Susie see the whole system as RAM
		//
		// Complete and utter wankers, its taken me 1 week to find the 2 lines
		// in all the documentation that mention this fact, the mother of all
		// bugs has been found and FIXED.......

		//
		// CPU
		//
		inline void  Poke_CPU(ULONG addr, UBYTE data);
		inline UBYTE Peek_CPU(ULONG addr);
		inline void  PokeW_CPU(ULONG addr,UWORD data);
		inline UWORD PeekW_CPU(ULONG addr);

		//
		// RAM
		//
		inline void  Poke_RAM(ULONG addr, UBYTE data) { mRam->Poke(addr,data);};
		inline UBYTE Peek_RAM(ULONG addr) { return mRam->Peek(addr);};
		inline void  PokeW_RAM(ULONG addr,UWORD data) { mRam->Poke(addr,data&0xff);addr++;mRam->Poke(addr,data>>8);};
		inline UWORD PeekW_RAM(ULONG addr) {return ((mRam->Peek(addr))+(mRam->Peek(addr+1)<<8));};

// High level cart access for debug etc

		inline void  Poke_CART(ULONG addr, UBYTE data) {mCart->Poke(addr,data);};
		inline UBYTE Peek_CART(ULONG addr) {return mCart->Peek(addr);};
		inline void  CartBank(EMMODE bank) {mCart->BankSelect(bank);};
		inline ULONG CartSize(void) {return mCart->ObjectSize();};
		inline void	 CartGetName(const char ** name) { mCart->CartGetName(name);};
		inline void	 CartGetManufacturer(const char ** manuf) { mCart->CartGetManufacturer(manuf);};
		inline ULONG CartGetRotate(void) {return mCart->CartGetRotate();};

// Low level cart access for Suzy, Mikey

		inline void  Poke_CARTB0(UBYTE data) {mCart->Poke0(data);};
		inline void  Poke_CARTB1(UBYTE data) {mCart->Poke1(data);};
		inline UBYTE Peek_CARTB0(void) {return mCart->Peek0();}
		inline UBYTE Peek_CARTB1(void) {return mCart->Peek1();}
		inline void  CartAddressStrobe(BOOL strobe) {mCart->CartAddressStrobe(strobe);};
		inline void  CartAddressData(BOOL data) {mCart->CartAddressData(data);};

// Low level CPU access

		void	SetRegs(C6502_REGS &regs) {mCpu->SetRegs(regs);};
		void	GetRegs(C6502_REGS &regs) {mCpu->GetRegs(regs);};
		void	SetCPUBreakpoint(ULONG breakpoint) {mCpu->SetBreakpoint(breakpoint);};

		inline void SetCPUSleep(void) {gSystemCPUSleep = TRUE;};
		inline void ClearCPUSleep(void) {gSystemCPUSleep = FALSE;gSystemCPUSleep_Saved = FALSE;};

// Mikey system interfacing

		ULONG   GetDisplayBuffer(void) {return mMikie->GetDisplayBuffer();};
		void	SetScreenAttributes(ULONG Mode,ULONG XSize,ULONG YSize,ULONG XOffset,ULONG YOffset,UBYTE *Bits0,UBYTE *Bits1)
			{ mMikie->SetScreenAttributes(Mode,XSize,YSize,XOffset,YOffset,Bits0,Bits1); };

// Suzy system interfacing

		ULONG	PaintSprites(void) {return mSusie->PaintSprites();};

// Miscellaneous

		void	SetButtonData(ULONG data) {mSusie->SetButtonData(data);};
		ULONG	GetButtonData(void) {return mSusie->GetButtonData();};
		void	SetCycleBreakpoint(ULONG breakpoint) {mCycleCountBreakpoint = breakpoint;};
		UBYTE	*GetRamPointer(void) {return mRam->GetRamPointer();};

	public:
		ULONG			mCycleCountBreakpoint;

	private:
//		CLynxMemObj		*mMemoryHandlers[8][SYSTEM_SIZE];

		CRom			*mRom;
		CCart			*mCart;
		CMemMap			*mMemMap;
		CRam			*mRam;
		C65C02			*mCpu;
		CMikie			*mMikie;
		CSusie			*mSusie;

		ULONG			mFileType;
};


#endif
