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

CSystem::CSystem(UBYTE *gamefile, int size)
	:mMikie(NULL)
{
	// Attempt to load the cartridge

	mMikie = new CMikie(*this);

	Reset();
}

CSystem::~CSystem()
{
	// Cleanup all our objects

	if (mMikie != NULL) delete mMikie;
}

void CSystem::Reset(void)
{
	gAudioBufferPointer = 0;
//	memset(gAudioBuffer0, 128, HANDY_AUDIO_BUFFER_SIZE);
//	memset(gAudioBuffer1, 128, HANDY_AUDIO_BUFFER_SIZE);
//	memset(gAudioBuffer2, 128, HANDY_AUDIO_BUFFER_SIZE);
//	memset(gAudioBuffer3, 128, HANDY_AUDIO_BUFFER_SIZE);

	mMikie->Reset();
}
