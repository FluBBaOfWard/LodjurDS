//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// System object class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the glue to bind of of the emulation objects         //
// together via peek/poke handlers and pass thru interfaces to lower        //
// objects, all control of the emulator is done via this class. Update()    //
// does most of the work and each call emulates one CPU instruction and     //
// updates all of the relevant hardware if required. It must be remembered  //
// that if that instruction involves setting SPRGO then, it will cause a    //
// sprite painting operation and then a corresponding update of all of the  //
// hardware which will usually involve recursive calls to Update, see       //
// Mikey SPRGO code for more details.                                       //
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

#define SYSTEM_CPP

#include <stdlib.h>
#include "system.h"
#include "../memory.h"

extern UBYTE *romSpacePtr;

int loadFile(const char *fname, void *dest, int start, int size);

CSystem::CSystem(const UBYTE *gamefile, int size, ULONG filetype, const char *romfile)
	:mRom(NULL),
	mCart(NULL),
	mCpu(NULL),
	mMikie(NULL),
	mSusie(NULL)
{
	mFileType = filetype;

	mCycleCountBreakpoint = 0xffffffff;

	// Create the system objects that we'll use

	// Attempt to load the cartridge

	if (mFileType == HANDY_FILETYPE_LNX) {
//		int size = loadFile(gamefile, romSpacePtr, -1, 0x80000);
		mCart = new CCart(romSpacePtr, size);
		if (mCart->CartHeaderLess()) {
			char drive[3];
			char dir[256];
			char cartgo[256];
			mFileType = HANDY_FILETYPE_HOMEBREW;
//			_splitpath(romfile,drive,dir,NULL,NULL);
			strcpy(cartgo,drive);
			strcat(cartgo,dir);
			strcat(cartgo,"howard.o");
			int size = loadFile(cartgo, romSpacePtr, -1, 0x10000);
		}
	}
	else if (mFileType == HANDY_FILETYPE_HOMEBREW) {
		mCart = new CCart(NULL, 0);
	}
	else {
		mCart = new CCart(NULL, 0);
	}

	// These can generate exceptions
	mMikie = new CMikie(*this);
	mSusie = new CSusie(*this);

// Now the handlers are set we can instantiate the CPU as it will use handlers on reset

	mCpu = new C65C02(*this);

// Now init is complete do a reset, this will cause many things to be reset twice
// but what the hell, who cares, I don't.....

	Reset();
}

inline void CSystem::Poke_CPU(ULONG addr, UBYTE data) {
	if (addr < 0xFC00) {
		ramPoke(addr,data);
		return;
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(memSelector & 0x01)) {
				mSusie->Poke(addr,data);
				return;
			}
			break;
		case 0x0100:
			if (!(memSelector & 0x02)) {
				mMikie->Poke(addr,data);
				return;
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					ramPoke(addr,data);
					return;
				}
				if (addr == 0xFFF9) {
					memSelector = data & 0xF;
					return;
				}
				if (!(memSelector & 0x08)) {
					rom_W(addr,data);
					return;
				} else {
					ramPoke(addr,data);
					return;
				}
			}
		case 0x0200:
			if (!(memSelector & 0x04)) {
				rom_W(addr,data);
				return;
			}
			break;
	}
	ramPoke(addr,data);
};

inline UBYTE CSystem::Peek_CPU(ULONG addr) {
	if (addr < 0xFC00) {
		return ramPeek(addr);
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(memSelector & 0x01)) {
				return mSusie->Peek(addr);
			}
			break;
		case 0x0100:
			if (!(memSelector & 0x02)) {
				return mMikie->Peek(addr);
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					return ramPeek(addr);
				}
				if (addr == 0xFFF9) {
					return memSelector;
				}
				if (!(memSelector & 0x08)) {
					return romPeek(addr);
				} else {
					return ramPeek(addr);
				}
			}
		case 0x0200:
			if (!(memSelector & 0x04)) {
				return romPeek(addr);
			}
			break;
	}
	return ramPeek(addr);
};

inline UWORD CSystem::PeekW_CPU(ULONG addr) {
	return ((Peek_CPU(addr))+(Peek_CPU(addr+1)<<8));
};

CSystem::~CSystem()
{
	// Cleanup all our objects

	if (mCart != NULL) delete mCart;
	if (mCpu != NULL) delete mCpu;
	if (mMikie != NULL) delete mMikie;
	if (mSusie != NULL) delete mSusie;
}


void CSystem::Reset(void)
{
	gSystemCycleCount = 0;
	gNextTimerEvent = 0;
	gCPUBootAddress = 0;
	gEmulatorAbort = FALSE;
	gBreakpointHit = FALSE;
	gSingleStepMode = FALSE;
	gSystemIRQ = FALSE;
	gSystemNMI = FALSE;
	gSystemCPUSleep = FALSE;
	gSystemHalt = FALSE;

	gThrottleLastTimerCount = 0;
	gThrottleNextCycleCheckpoint = 0;

	gTimerCount = 0;

	gAudioBufferPointer = 0;
	gAudioLastUpdateCycle = 0;
//	memset(gAudioBuffer, 128, HANDY_AUDIO_BUFFER_SIZE);
	memset(gAudioBuffer0, 128, HANDY_AUDIO_BUFFER_SIZE);
	memset(gAudioBuffer1, 128, HANDY_AUDIO_BUFFER_SIZE);
	memset(gAudioBuffer2, 128, HANDY_AUDIO_BUFFER_SIZE);
	memset(gAudioBuffer3, 128, HANDY_AUDIO_BUFFER_SIZE);

	memset(lynxRAM, DEFAULT_RAM_CONTENTS, RAM_SIZE);

#ifdef _LYNXDBG
	gSystemHalt = TRUE;
#endif

	memSelector = 0;
	mCart->Reset();
	mMikie->Reset();
	mSusie->Reset();
	mCpu->Reset();

}
