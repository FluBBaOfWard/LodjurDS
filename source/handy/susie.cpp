//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Suzy emulation class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Suzy chip within the lynx. This provides math    //
// and sprite painting facilities. SpritePaint() is called from within      //
// the Mikey POKE functions when SPRGO is set and is called via the system  //
// object to keep the interface clean.                                      //
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

#define SUSIE_CPP

//#include <crtdbg.h>
//#define TRACE_SUSIE

#include "system.h"
#include "susie.h"
#include "lynxdef.h"
#include "../Gfx.h"

//
// As the Susie sprite engine only ever sees system RAM
// we can access this directly without the hassle of
// going through the system object, much faster
//
#define RAM_PEEK(m)				(suzy_0.suzyRAM[(m)])
#define RAM_POKE(m1,m2)			{suzy_0.suzyRAM[(m1)]=(m2);}

#define mHOFF suzy_0.hOff
#define mVOFF suzy_0.vOff
#define mTILTACUM suzy_0.tiltAcum
#define mSCBNEXT suzy_0.SCBNext
#define mSPRDLINE suzy_0.sprDLine
#define mHPOSSTRT suzy_0.hPosStrt
#define mVPOSSTRT suzy_0.vPosStrt
#define mSPRHSIZ suzy_0.sprHSiz
#define mSPRVSIZ suzy_0.sprVSiz
#define mSTRETCH suzy_0.stretch
#define mTILT suzy_0.tilt
#define mSPRDOFF suzy_0.sprDOff
#define mCOLLOFF suzy_0.collOff
#define mVSIZACUM suzy_0.vSizAcum
#define mVSIZOFF suzy_0.vSizOff
#define mSCBADR suzy_0.SCBAdr
#define mSPRCTL0 suzy_0.sprCtl0
#define mSPRCTL1 suzy_0.sprCtl1
#define mSPRCOLL suzy_0.sprColl
#define mSUZYBUSEN suzy_0.suzyBusEn
#define mSPRGO suzy_0.sprGo
#define mSPRSYS suzy_0.sprSys

#define mCollision suzy_0.collision
#define cycles_used suzy_0.cyclesUsed

#define mSPRSYS_Busy suzy_0.sprSys_Busy
#define everonscreen suzy_0.everOnScreen

// SprCtl0
#define Type  (0x07)
#define Vflip (0x10)
#define Hflip (0x20)

// SPRCTL1
#define StartLeft (0x01)
#define StartUp (0x02)
#define SkipSprite (0x04)
#define ReloadPalette (0x08)
#define ReloadDepth (0x30)
//#define Sizing (0x40)
//#define Literal (0x80)

// SPRSYS
#define StopOnCurrent (0x02)
#define UnsafeAccess (0x04)
#define LeftHand (0x08)
#define VStretch (0x10)
#define NoCollide (0x20)
#define Accumulate (0x40)
#define SignedMath (0x80)

CSusie::CSusie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_SUSIE0("CSusie()");
	Reset();
}

CSusie::~CSusie()
{
	TRACE_SUSIE0("~CSusie()");
}

void CSusie::Reset(void)
{
	TRACE_SUSIE0("Reset()");
}

ULONG CSusie::PaintSprites(void)
{
	TRACE_SUSIE0("********************** PaintSprites **************************");

	TRACE_SUSIE1("PaintSprites() VIDBAS  $%04x", mVIDBAS.Word);
	TRACE_SUSIE1("PaintSprites() COLLBAS $%04x", mCOLLBAS.Word);
	TRACE_SUSIE1("PaintSprites() SPRSYS  $%02x", Peek(SPRSYS));

	if (!mSUZYBUSEN || !(mSPRGO & 0x01)) {
		TRACE_SUSIE0("PaintSprites() Returned !mSUZYBUSEN || !mSPRGO");
		return 0;
	}

	cycles_used = 0;
	int	sprcount = 0;
	everonscreen = 0;

	mSPRSYS_Busy = 1;
	do {
		// Step 1 load up the SCB params into Susie

		// And thus it is documented that only the top byte of SCBNEXT is used.
		// Its mentioned under the bits that are broke section in the bluebook
		if (!(mSCBNEXT.Word & 0xff00)) {
			mSPRSYS_Busy = 0;	// Engine has finished
			mSPRGO &= ~0x01;
			break;
		}

		suzFetchSpriteData();

		// Check if this is a skip sprite
		if (!(mSPRCTL1 & SkipSprite)) {
			// Initialise the collision depositary

// Although Tom Schenck says this is correct, it doesnt appear to be
//			if (mSPRCOLL < 0x10)
//			{
//				mCollision = RAM_PEEK((mSCBADR.Word+mCOLLOFF.Word) & 0xffff) & 0x0f;
//			}
//			mCollision = 0;

			// Now we can start painting

			// Quadrant drawing order is: SE,NE,NW,SW
			// start quadrant is given by sprite_control1:0 & 1
			int quadrant = 0;
			int hSign = 1;
			int vSign = 1;
			if (mSPRCTL1 & StartLeft) {
				quadrant = 3;
				hSign = -1;
			}
			if (mSPRCTL1 & StartUp) {
				quadrant ^= 1;
				vSign = -1;
			}

			// Quadrant mapping is:	SE	NE	NW	SW
			//						0	1	2	3
			// hSign				+1	+1	-1	-1
			// vSign				+1	-1	-1	+1
			//
			//
			//		2 | 1
			//     -------
			//      3 | 0
			//

			// Use h/v flip to invert h/vSign
			if (mSPRCTL0 & Vflip) vSign = -vSign;
			if (mSPRCTL0 & Hflip) hSign = -hSign;
//			int vQuadOff = vSign;
//			int hQuadOff = hSign;
			suzRenderQuads(hSign, vSign, quadrant);
/*
			// Loop for 4 quadrants
			for (int loop=0;loop<4;loop++) {
				// Set the vertical position & offset
				int voff = (SWORD)mVPOSSTRT.Word-(SWORD)mVOFF.Word;

				// Take the sign of the first quad (0) as the basic
				// sign, all other quads drawing in the other direction
				// get offset by 1 pixel in the other direction, this
				// fixes the squashed look on the multi-quad sprites.
				if (vSign != vQuadOff) voff += vSign;

				// Zero the stretch,tilt & acum values
				mTILTACUM.Word = 0;

				// Perform the SIZOFF
				mVSIZACUM.Word = (vSign == 1) ? mVSIZOFF.Word : 0;

				for (;;) {
					// Update the next data line pointer and initialise our line
					mSPRDOFF.Word = (UWORD)suzLineStart();
					// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
					if (mSPRDOFF.Word == 1) {		// End of quad
						mSPRDLINE.Word += mSPRDOFF.Word;
						break;
					}
					if (mSPRDOFF.Word == 0) {		// End of sprite
						loop = 4;		// Halt the quad loop
						break;
					}

					// Vertical scaling is done here
					mVSIZACUM.Word += mSPRVSIZ.Word;
					int pixel_height = mVSIZACUM.Byte.High;
					mVSIZACUM.Byte.High = 0;

					// Draw one horizontal source line of the sprite
					for (;pixel_height>0;pixel_height--) {
						// Early bailout if the sprite has moved off screen, terminate quad
						if (vSign == 1 && voff >= LYNX_SCREEN_HEIGHT) break;
						if (vSign == -1 && voff < 0) break;

						// Now render an individual destination line
						suzLineRender(hSign, hQuadOff, voff);
						voff += vSign;

						// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
						if (mSPRCTL1 & 0x20) {
							if (mSPRCTL1 & 0x10) {
								// Manipulate the tilt stuff
								mTILTACUM.Word += mTILT.Word;
							}
							mSPRHSIZ.Word += mSTRETCH.Word;
							// According to the docs this increments per dest line
							// but only gets set when the source line is read
							if (mSPRSYS & VStretch) mSPRVSIZ.Word += mSTRETCH.Word;
						}
					}

					// Update the line start for our next run thru the loop
					mSPRDLINE.Word += mSPRDOFF.Word;
				}

				// Increment quadrant and mask to 2 bit value (0-3)
				quadrant++;
				quadrant &= 0x03;
				if (loop != 3) {
					// Check new quadrant to h/v flip
					if (quadrant & 1) {
						vSign = -vSign;
					}
					else {
						hSign = -hSign;
					}
				}
			}*/

			// Write the collision depositary if required
			if (mSPRCOLL < 0x10) {
				switch(mSPRCTL0 & Type)
				{
					case sprite_xor_shadow:
					case sprite_boundary:
					case sprite_normal:
					case sprite_boundary_shadow:
					case sprite_shadow:
						{
							UWORD coldep = mSCBADR.Word+mCOLLOFF.Word;
							RAM_POKE(coldep, mCollision);
							TRACE_SUSIE2("PaintSprites() COLLOFF=$%04x SCBADR=$%04x", mCOLLOFF.Word, mSCBADR.Word);
							TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x", mCollision, coldep);
						}
						break;
					default:
						break;
				}
			}

			if (mSPRGO & 0x04) { // EVERON?
				UWORD coldep = mSCBADR.Word + mCOLLOFF.Word;
				UBYTE coldat = RAM_PEEK(coldep);
				if (!everonscreen) coldat |= 0x80; else coldat &= 0x7f;
				RAM_POKE(coldep, coldat);
				TRACE_SUSIE0("PaintSprites() EVERON IS ACTIVE");
				TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x", coldat, coldep);
			}

			// Perform Sprite debugging if required, single step on sprite draw
			if (gSingleStepModeSprites) {
				char message[256];
				sprintf(message, "CSusie:PaintSprites() - Rendered Sprite %03d", sprcount);
				//if (!gError->Warning(message)) gSingleStepModeSprites = 0;
			}
		}
		else {
			TRACE_SUSIE0("PaintSprites() mSPRCTL1.Bits.SkipSprite==TRUE");
		}

		// Increase sprite number
		sprcount++;

		// Check if we abort after this sprite is complete
//		if (mSPRSYS.Read.StopOnCurrent) {
//			mSPRSYS.Read.Busy = 0; // Engine has finished
//			mSPRGO &= ~0x01;
//			break;
//		}

		// Check sprcount for looping SCB, random large number chosen
		if (sprcount > 4096) {
			// Stop the system, otherwise we may just come straight back in.....
			gSystemHalt = TRUE;
			// Display warning message
			//gError->Warning("CSusie:PaintSprites(): Single draw sprite limit exceeded (>4096). The SCB is most likely looped back on itself. Reset/Exit is recommended");
			// Signal error to the caller
			return 0;
		}
	}
	while(1);

	// Fudge factor to fix many flickering issues, also the keypress
	// problem with Hard Drivin and the strange pause in Dirty Larry.
//	cycles_used >>= 2;

	return cycles_used;
}

void CSusie::Poke(ULONG addr, UBYTE data)
{
//	switch(addr & 0xff)
//	{
//// Cartridge writing ports
//		case (RCART0 & 0xff):
//			mSystem.Poke_CARTB0(data);
//			TRACE_SUSIE2("Poke(RCART0,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
//		case (RCART1 & 0xff):
//			mSystem.Poke_CARTB1(data);
//			TRACE_SUSIE2("Poke(RCART1,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
//	}
}

UBYTE CSusie::Peek(ULONG addr)
{
//	switch(addr & 0xff)
//	{
//// Cartridge reading ports
//		case (RCART0 & 0xff):
//			return mSystem.Peek_CARTB0();
//		case (RCART1 & 0xff):
//			return mSystem.Peek_CARTB1();
//	}
	return 0xff;
}
