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

extern UBYTE *romSpacePtr;

int loadFile(const char *fname, void *dest, int start, int size);

CSystem::CSystem(const UBYTE *gamefile, int size, ULONG filetype, const char *romfile)
	:mRom(NULL),
	mCart(NULL),
	mMemMap(NULL),
	mRam(NULL),
	mCpu(NULL),
	mMikie(NULL),
	mSusie(NULL)
{
	mFileType = filetype;

	mCycleCountBreakpoint = 0xffffffff;

	// Create the system objects that we'll use

	// Attempt to load the cartridge & catch errors
//	try
//	{
		UBYTE romData[ROM_SIZE];
		loadFile(romfile, romData, -1, ROM_SIZE);
		mRom = new CRom(romData);
//	}
/*	catch(CFileException *filerr)
	{
		CLynxException lynxerr;

		if(filerr->m_cause==CFileException::fileNotFound)
		{
			lynxerr.Message() << "The Lynx Boot ROM image couldn't be located!";
			lynxerr.Description()
				<< "The lynx emulator will not run without the Boot ROM image." << endl
				<< "\"" << romfile << "\" was not found in the lynx emulator " << endl
				<< "directory (see the LynxEmu User Guide for more information).";
		}
		else
		{
			lynxerr.Message() << "The Lynx Boot ROM image couldn't be loaded!";
			lynxerr.Description()
				<< "The lynx emulator will not run without the Boot ROM image." << endl
				<< "It appears that your BOOT image may be corrupted or there is" << endl
				<< "some other error.(see the LynxEmu User Guide for more information)";
		}
		filerr->Delete();
		throw(lynxerr);
	}*/

	// An exception from this will be caught by the level above

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
			mRam = new CRam(romSpacePtr, size);
		}
		else {
			mRam = new CRam(NULL, 0);
		}
	}
	else if (mFileType == HANDY_FILETYPE_HOMEBREW) {
		mCart = new CCart(NULL, 0);
		mRam = new CRam(romSpacePtr, 0);
	}
	else {
		mCart = new CCart(NULL, 0);
		mRam = new CRam(NULL, 0);
	}

	// These can generate exceptions
	mMikie = new CMikie(*this);
	mSusie = new CSusie(*this);

	mMemMap = new CMemMap(*this);

// Now the handlers are set we can instantiate the CPU as is will use handlers on reset

	mCpu = new C65C02(*this);

// Now init is complete do a reset, this will cause many things to be reset twice
// but what the hell, who cares, I don't.....

	Reset();
}

inline void CSystem::Poke_CPU(ULONG addr, UBYTE data) {
	if ((addr & 0xFC00) != 0xFC00) {
		mRam->Poke(addr,data);
		return;
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(mMemMap->mSelector&0x01)) {
				mSusie->Poke(addr,data);
				return;
			}
			break;
		case 0x0100:
			if (!(mMemMap->mSelector&0x02)) {
				mMikie->Poke(addr,data);
				return;
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					mRam->Poke(addr,data);
					return;
				}
				if (addr == 0xFFF9) {
					mMemMap->Poke(addr,data);
					return;
				}
				if (!(mMemMap->mSelector&0x08)) {
					mRom->Poke(addr,data);
					return;
				} else {
					mRam->Poke(addr,data);
					return;
				}
			}
		case 0x0200:
			if (!(mMemMap->mSelector&0x04)) {
				mRom->Poke(addr,data);
				return;
			}
			break;
	}
	mRam->Poke(addr,data);
};

inline UBYTE CSystem::Peek_CPU(ULONG addr) {
	if ((addr & 0xFC00) != 0xFC00) {
		return mRam->Peek(addr);
	}
	switch (addr & 0x0300) {
		case 0x0000:
			if (!(mMemMap->mSelector&0x01)) {
				return mSusie->Peek(addr);
			}
			break;
		case 0x0100:
			if (!(mMemMap->mSelector&0x02)) {
				return mMikie->Peek(addr);
			}
			break;
		case 0x0300:
			if (addr >= 0xFFF8) {
				if (addr == 0xFFF8) {
					return mRam->Peek(addr);
				}
				if (addr == 0xFFF9) {
					return mMemMap->Peek(addr);
				}
				if (!(mMemMap->mSelector&0x08)) {
					return mRom->Peek(addr);
				} else {
					return mRam->Peek(addr);
				}
			}
		case 0x0200:
			if (!(mMemMap->mSelector&0x04)) {
				return mRom->Peek(addr);
			}
			break;
	}
	return mRam->Peek(addr);
};

inline void  CSystem::PokeW_CPU(ULONG addr,UWORD data) {
	Poke_CPU(addr, data & 0xff);
	Poke_CPU(addr+1, data>>8);
};

inline UWORD CSystem::PeekW_CPU(ULONG addr) {
	return ((Peek_CPU(addr))+(Peek_CPU(addr+1)<<8));
};

CSystem::~CSystem()
{
	// Cleanup all our objects

	if (mCart != NULL) delete mCart;
	if (mRom != NULL) delete mRom;
	if (mRam != NULL) delete mRam;
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

#ifdef _LYNXDBG
	gSystemHalt = TRUE;
#endif

	mMemMap->Reset();
	mCart->Reset();
	mRom->Reset();
	mRam->Reset();
	mMikie->Reset();
	mSusie->Reset();
	mCpu->Reset();

	// Homebrew hashup

	if (mFileType == HANDY_FILETYPE_HOMEBREW) {
		mMikie->PresetForHomebrew();

		C6502_REGS regs;
		mCpu->GetRegs(regs);
		regs.PC = (UWORD)gCPUBootAddress;
		mCpu->SetRegs(regs);
	}
}
