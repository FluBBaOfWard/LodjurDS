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
#include "../Cpu.h"
#include "../Gfx.h"

//
// As the Susie sprite engine only ever sees system RAM
// we can access this directly without the hassle of
// going through the system object, much faster
//
#define RAM_PEEK(m)				(mRamPointer[(m)])
#define RAM_PEEKW(m)			(mRamPointer[(m)]+(mRamPointer[(m)+1]<<8))
#define RAM_POKE(m1,m2)			{mRamPointer[(m1)]=(m2);}

#define mTMPADR suzy_0.tmpAdr
#define mHOFF suzy_0.hOff
#define mVOFF suzy_0.vOff
#define mTILTACUM suzy_0.tiltAcum
//#define mVIDBAS suzy_0.vidBas
//#define mCOLLBAS suzy_0.collBas
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
#define mHSIZOFF suzy_0.hSizOff
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

#define mPenIndex suzy_0.penIndex

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

	// Fetch pointer to system RAM, faster than object access
	// and seeing as Susie only ever sees RAM.

	mRamPointer = mSystem.GetRamPointer();

	// Reset ALL variables

	// Must be initialised to this due to
	// Stun Runner math initialisation bug
	// see whatsnew for 0.7
	mMATHABCD.Long = 0xffffffff;
	mMATHEFGH.Long = 0xffffffff;
	mMATHJKLM.Long = 0xffffffff;
	mMATHNP.Long = 0xffff;

	mMATHAB_sign = 1;
	mMATHCD_sign = 1;
	mMATHEFGH_sign = 1;

	mSPRSYS_UnsafeAccess = 0;
	mSPRSYS_Busy = 0;
	mSPRSYS_LastCarry = 0;
	mSPRSYS_Mathbit = 0;
	mSPRSYS_MathInProgress = 0;

	mJOYSTICK.Byte = 0;
	mSWITCHES.Byte = 0;
}


void CSusie::DoMathMultiply(void)
{
	mSPRSYS_Mathbit = FALSE;

	// Multiplies without sign or accumulate take 44 ticks to complete.
	// Multiplies with sign and accumulate take 54 ticks to complete.
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//


	ULONG result;

	// Basic multiply is ALWAYS unsigned, sign conversion is done later
	result = (ULONG)mMATHABCD.Words.AB * (ULONG)mMATHABCD.Words.CD;
	mMATHEFGH.Long = result;

	if (mSPRSYS & SignedMath) {
		TRACE_SUSIE0("DoMathMultiply() - SIGNED");
		// Add the sign bits, only >0 is +ve result
		mMATHEFGH_sign = mMATHAB_sign+mMATHCD_sign;
		if (!mMATHEFGH_sign)
		{
			mMATHEFGH.Long ^= 0xffffffff;
			mMATHEFGH.Long++;
		}
	}
	else {
		TRACE_SUSIE0("DoMathMultiply() - UNSIGNED");
	}

	TRACE_SUSIE2("DoMathMultiply() AB=$%04x * CD=$%04x", mMATHABCD.Words.AB, mMATHABCD.Words.CD);

	// Check overflow, if B31 has changed from 1->0 then its overflow time
	if (mSPRSYS & Accumulate) {
		TRACE_SUSIE0("DoMathMultiply() - ACCUMULATED JKLM+=EFGH");
		ULONG tmp = mMATHJKLM.Long + mMATHEFGH.Long;
		// Let sign change indicate overflow
		if ((tmp & 0x80000000) != (mMATHJKLM.Long & 0x80000000)) {
			TRACE_SUSIE0("DoMathMultiply() - OVERFLOW DETECTED");
//			mSPRSYS_Mathbit = TRUE;
		}
		else {
//			mSPRSYS_Mathbit = FALSE;
		}
		// Save accumulated result
		mMATHJKLM.Long = tmp;
	}

	TRACE_SUSIE1("DoMathMultiply() Results (raw - no sign) Result=$%08x", result);
	TRACE_SUSIE1("DoMathMultiply() Results (Multi) EFGH=$%08x", mMATHEFGH.Long);
	TRACE_SUSIE1("DoMathMultiply() Results (Accum) JKLM=$%08x", mMATHJKLM.Long);
}

void CSusie::DoMathDivide(void)
{
	mSPRSYS_Mathbit = FALSE;

	//
	// Divides take 176 + 14*N ticks
	// (N is the number of most significant zeros in the divisor.)
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//

	// Divide is ALWAYS unsigned arithmetic...
	if (mMATHNP.Long) {
		TRACE_SUSIE0("DoMathDivide() - UNSIGNED");
		mMATHABCD.Long = mMATHEFGH.Long / mMATHNP.Long;
		mMATHJKLM.Long = mMATHEFGH.Long % mMATHNP.Long;
	}
	else {
		TRACE_SUSIE0("DoMathDivide() - DIVIDE BY ZERO ERROR");
		mMATHABCD.Long = 0xffffffff;
		mMATHJKLM.Long = 0;
		mSPRSYS_Mathbit = TRUE;
	}
	TRACE_SUSIE2("DoMathDivide() EFGH=$%08x / NP=%04x", mMATHEFGH.Long, mMATHNP.Long);
	TRACE_SUSIE1("DoMathDivide() Results (div) ABCD=$%08x", mMATHABCD.Long);
	TRACE_SUSIE1("DoMathDivide() Results (mod) JKLM=$%08x", mMATHJKLM.Long);
}


ULONG CSusie::PaintSprites(void)
{
	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("**************************************************************");
	TRACE_SUSIE0("********************** PaintSprites **************************");
	TRACE_SUSIE0("**************************************************************");
	TRACE_SUSIE0("                                                              ");

	TRACE_SUSIE1("PaintSprites() VIDBAS  $%04x", mVIDBAS.Word);
	TRACE_SUSIE1("PaintSprites() COLLBAS $%04x", mCOLLBAS.Word);
	TRACE_SUSIE1("PaintSprites() SPRSYS  $%02x", Peek(SPRSYS));

	if (!mSUZYBUSEN || !(mSPRGO & 0x01)) {
		TRACE_SUSIE0("PaintSprites() Returned !mSUZYBUSEN || !mSPRGO");
		return 0;
	}

	cycles_used = 0;
	int	sprcount = 0;
	int everonscreen = 0;

	do {
		TRACE_SUSIE1("PaintSprites() ************ Rendering Sprite %03d ************", sprcount);

		// Step 1 load up the SCB params into Susie

		// And thus it is documented that only the top byte of SCBNEXT is used.
		// Its mentioned under the bits that are broke section in the bluebook
		if (!(mSCBNEXT.Word & 0xff00)) {
			TRACE_SUSIE0("PaintSprites() mSCBNEXT==0 - FINISHED");
			mSPRSYS_Busy = 0;	// Engine has finished
			mSPRGO &= ~0x01;
			break;
		}
		else {
			mSPRSYS_Busy = 1;
		}

		mTMPADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
		mSCBADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
		TRACE_SUSIE1("PaintSprites() SCBADDR $%04x", mSCBADR.Word);

		int data = RAM_PEEK(mTMPADR.Word);			// Fetch control 0
		TRACE_SUSIE1("PaintSprites() SPRCTL0 $%02x", data);
		lnxSuzyWrite(0xFC80, data);
		mTMPADR.Word += 1;

		data = RAM_PEEK(mTMPADR.Word);			// Fetch control 1
		TRACE_SUSIE1("PaintSprites() SPRCTL1 $%02x", data);
		mSPRCTL1 = data;
		mTMPADR.Word += 1;

		data = RAM_PEEK(mTMPADR.Word);			// Collision num
		TRACE_SUSIE1("PaintSprites() SPRCOLL $%02x", data);
		mSPRCOLL = (data & 0x2f) | (mSPRSYS & NoCollide);
		mTMPADR.Word += 1;

		mSCBNEXT.Word = RAM_PEEKW(mTMPADR.Word);	// Next SCB
		TRACE_SUSIE1("PaintSprites() SCBNEXT $%04x", mSCBNEXT.Word);
		mTMPADR.Word += 2;

		cycles_used += 5 * SPR_RDWR_CYC;

		// Check if this is a skip sprite

		if (!(mSPRCTL1 & SkipSprite)) {
			// Initialise the collision depositary

// Although Tom Schenck says this is correct, it doesnt appear to be
//			if (mSPRCOLL <= 0xF)
//			{
//				mCollision = RAM_PEEK((mSCBADR.Word+mCOLLOFF.Word) & 0xffff) & 0x0f;
//			}
			mCollision = 0;

			mSPRDLINE.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite pack data
			TRACE_SUSIE1("PaintSprites() SPRDLINE $%04x",mSPRDLINE.Word);
			mTMPADR.Word += 2;

			mHPOSSTRT.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite horizontal start position
			TRACE_SUSIE1("PaintSprites() HPOSSTRT $%04x",mHPOSSTRT.Word);
			mTMPADR.Word += 2;

			mVPOSSTRT.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite vertical start position
			TRACE_SUSIE1("PaintSprites() VPOSSTRT $%04x",mVPOSSTRT.Word);
			mTMPADR.Word += 2;

			cycles_used += 6 * SPR_RDWR_CYC;

			// bool enable_sizing = FALSE; // unused
			bool enable_stretch = FALSE;
			bool enable_tilt = FALSE;

			// Optional section defined by reload type in Control 1

			TRACE_SUSIE1("PaintSprites() mSPRCTL1.Bits.ReloadDepth=%d", mSPRCTL1_ReloadDepth);
			switch(mSPRCTL1 & ReloadDepth)
			{
				case 0x10:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					// enable_sizing=TRUE; // unused

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					cycles_used += 4 * SPR_RDWR_CYC;
					break;

				case 0x20:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					TRACE_SUSIE0("PaintSprites() Stretch Enabled");
					// enable_sizing = TRUE; // unused
					enable_stretch = TRUE;

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					mSTRETCH.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
					mTMPADR.Word += 2;

					cycles_used += 6 * SPR_RDWR_CYC;
					break;

				case 0x30:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					TRACE_SUSIE0("PaintSprites() Stretch Enabled");
					TRACE_SUSIE0("PaintSprites() Tilt Enabled");
					// enable_sizing = TRUE;  // unused
					enable_stretch = TRUE;
					enable_tilt = TRUE;

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					mSTRETCH.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
					mTMPADR.Word += 2;

					mTILT.Word = RAM_PEEKW(mTMPADR.Word);		// Sprite tilt
					mTMPADR.Word += 2;

					cycles_used += 8 * SPR_RDWR_CYC;
					break;

				default:
					break;
			}

			TRACE_SUSIE1("PaintSprites() SPRHSIZ $%04x", mSPRHSIZ.Word);
			TRACE_SUSIE1("PaintSprites() SPRVSIZ $%04x", mSPRVSIZ.Word);
			TRACE_SUSIE1("PaintSprites() STRETCH $%04x", mSTRETCH.Word);
			TRACE_SUSIE1("PaintSprites() TILT    $%04x", mTILT.Word);


			// Optional Palette reload

			if (!(mSPRCTL1 & ReloadPalette)) {
				TRACE_SUSIE0("PaintSprites() Palette reloaded");
				for (int loop=0;loop<8;loop++) {
					UBYTE pen = RAM_PEEK(mTMPADR.Word++);
					mPenIndex[loop*2] = (pen>>4) & 0x0f;
					mPenIndex[(loop*2)+1] = pen & 0x0f;
				}
				// Increment cycle count for the reads
				cycles_used += 8 * SPR_RDWR_CYC;
			}

			// Now we can start painting
		
			// Quadrant drawing order is: SE,NE,NW,SW
			// start quadrant is given by sprite_control1:0 & 1

			// Setup screen start end variables

			int screen_h_start = (SWORD)mHOFF.Word;
			int screen_h_end = screen_h_start + LYNX_SCREEN_WIDTH;
			int screen_v_start = (SWORD)mVOFF.Word;
			int screen_v_end = screen_v_start + LYNX_SCREEN_HEIGHT;

			int world_h_mid = screen_h_start + 0x8000 + (LYNX_SCREEN_WIDTH / 2);
			int world_v_mid = screen_v_start + 0x8000 + (LYNX_SCREEN_HEIGHT / 2);

			TRACE_SUSIE2("PaintSprites() screen_h_start $%04x screen_h_end $%04x", screen_h_start, screen_h_end);
			TRACE_SUSIE2("PaintSprites() screen_v_start $%04x screen_v_end $%04x", screen_v_start, screen_v_end);
			TRACE_SUSIE2("PaintSprites() world_h_mid    $%04x world_v_mid  $%04x", world_h_mid, world_v_mid);

			int quadrant = 0;

			if (mSPRCTL1 & StartLeft) quadrant = 3;
			if (mSPRCTL1 & StartUp) quadrant ^= 1;

			TRACE_SUSIE1("PaintSprites() Quadrant=%d", quadrant);

			// Check ref is inside screen area. !! This is commented out in Mednafen!!
			bool superclip = ((SWORD)mHPOSSTRT.Word < screen_h_start
							  || (SWORD)mHPOSSTRT.Word >= screen_h_end
							  || (SWORD)mVPOSSTRT.Word < screen_v_start
							  || (SWORD)mVPOSSTRT.Word >= screen_v_end);

			TRACE_SUSIE1("PaintSprites() Superclip=%d",superclip);


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

			int hQuadOff, vQuadOff;

			// Loop for 4 quadrants
			for (int loop=0;loop<4;loop++)
			{
				TRACE_SUSIE1("PaintSprites() -------- Rendering Quadrant %03d --------", quadrant);

				int sprite_v = mVPOSSTRT.Word;
				int sprite_h = mHPOSSTRT.Word;

				bool render = FALSE;

				// Set quadrand multipliers
				int hSign = (quadrant == 0 || quadrant == 1) ? 1 : -1;
				int vSign = (quadrant == 0 || quadrant == 3) ? 1 : -1;

// Preflip		TRACE_SUSIE2("PaintSprites() hSign=%d vSign=%d",hSign,vSign);

				// Use h/v flip to invert h/vSign
				if (mSPRCTL0 & Vflip) vSign = -vSign;
				if (mSPRCTL0 & Hflip) hSign = -hSign;
				if (loop == 0) {
					vQuadOff = vSign;
					hQuadOff = hSign;
				}
				TRACE_SUSIE2("PaintSprites() Hflip=%d Vflip=%d", mSPRCTL0 & Hflip, mSPRCTL0 & Vflip);
				TRACE_SUSIE2("PaintSprites() Hsign=%d   Vsign=%d", hSign, vSign);
				TRACE_SUSIE2("PaintSprites() Hpos =%04x Vpos =%04x", mHPOSSTRT.Word, mVPOSSTRT.Word);
				TRACE_SUSIE2("PaintSprites() Hsizoff =%04x Vsizoff =%04x", mHSIZOFF.Word, mVSIZOFF.Word);

				// Two different rendering algorithms used, on-screen & superclip
				// when on screen we draw in x until off screen then skip to next
				// line, BUT on superclip we draw all the way to the end of any
				// given line checking each pixel is on screen.

				if (superclip) {
					// Check on the basis of each quad, we only render the quad
					// IF the screen is in the quad, relative to the centre of
					// the screen which is calculated below.

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
					// Quadrant mapping for superclipping must also take into account
					// the hflip, vflip bits & negative tilt to be able to work correctly
					//
					int	modquad = quadrant;

					if (mSPRCTL0 & Vflip) modquad ^= 1;
					if (mSPRCTL0 & Hflip) modquad ^= 3;

					// This is causing Eurosoccer to fail!!
					//if (enable_tilt && mTILT.Word & 0x8000) modquad = hquadflip[modquad];
					//if (quadrant == 0 && sprite_v == 219 && sprite_h == 890)
					//printf("%d:%d %d %d\n", quadrant, modquad, sprite_h, sprite_v);

					switch(modquad)
					{
						case 3:
							if ((sprite_h >= screen_h_start || sprite_h < world_h_mid) && (sprite_v < screen_v_end   || sprite_v > world_v_mid)) render = TRUE;
							break;
						case 2:
							if ((sprite_h >= screen_h_start || sprite_h < world_h_mid) && (sprite_v >= screen_v_start || sprite_v < world_v_mid)) render = TRUE;
							break;
						case 1:
							if ((sprite_h < screen_h_end   || sprite_h > world_h_mid) && (sprite_v >= screen_v_start || sprite_v < world_v_mid)) render = TRUE;
							break;
						default:
							if ((sprite_h < screen_h_end   || sprite_h > world_h_mid) && (sprite_v < screen_v_end   || sprite_v > world_v_mid)) render = TRUE;
							break;
					}
				}
				else {
					render = TRUE;
				}

				// Is this quad to be rendered ??

				TRACE_SUSIE1("PaintSprites() Render status %d", render);

				if (render) {
					// Set the vertical position & offset
					int voff = (SWORD)mVPOSSTRT.Word-screen_v_start;

					// Zero the stretch,tilt & acum values
					mTILTACUM.Word = 0;

					// Perform the SIZOFF
					if (vSign == 1) mVSIZACUM.Word = mVSIZOFF.Word;
					else mVSIZACUM.Word = 0;

					// Take the sign of the first quad (0) as the basic
					// sign, all other quads drawing in the other direction
					// get offset by 1 pixel in the other direction, this
					// fixes the squashed look on the multi-quad sprites.
					if (vSign != vQuadOff) voff += vSign;

					for (;;) {
						// Vertical scaling is done here
						mVSIZACUM.Word += mSPRVSIZ.Word;
						int pixel_height = mVSIZACUM.Byte.High;
						mVSIZACUM.Byte.High = 0;

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

						// Draw one horizontal line of the sprite
						for (int vloop=0;vloop<pixel_height;vloop++) {
							// Early bailout if the sprite has moved off screen, terminate quad
							if (vSign == 1 && voff >= LYNX_SCREEN_HEIGHT) break;
							if (vSign == -1 && voff < 0) break;

							// Now render an individual destination line
							if (suzLineRender(screen_h_start, hSign, hQuadOff, voff)) {
								everonscreen = TRUE;
							}
							voff += vSign;

							// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
							if (enable_stretch) {
								mSPRHSIZ.Word += mSTRETCH.Word;
//								if (mSPRSYS & VStretch) mSPRVSIZ.Word += mSTRETCH.Word;
							}
							if (enable_tilt) {
								// Manipulate the tilt stuff
								mTILTACUM.Word += mTILT.Word;
							}
						}
						// According to the docs this increments per dest line
						// but only gets set when the source line is read
						if (mSPRSYS & VStretch) mSPRVSIZ.Word += mSTRETCH.Word*pixel_height;

						// Update the line start for our next run thru the loop
						mSPRDLINE.Word += mSPRDOFF.Word;
					}
				}
				else {
					// Skip thru data to next quad
					for(;;) {
						// Read the start of line offset
						mSPRDOFF.Word = (UWORD)suzLineStart();
						mSPRDLINE.Word += mSPRDOFF.Word;
						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
						if (mSPRDOFF.Word == 1) {		// End of quad
							break;
						}
						if (mSPRDOFF.Word == 0) {		// End of sprite
							loop = 4;		// Halt the quad loop
							break;
						}
					}
				}

				// Increment quadrant and mask to 2 bit value (0-3)
				quadrant++;
				quadrant &= 0x03;
			}

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
	switch(addr & 0xff)
	{
		case (MATHD & 0xff):
			TRACE_SUSIE2("Poke(MATHD,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.D = data;
//			mMATHABCD.Bytes.C = 0;
			// The hardware manual says that the sign shouldnt change
			// but if I dont do this then Stun Runner will hang as it
			// does the init in the wrong order and if the previous
			// calc left a zero there then we'll get a sign error
			Poke(MATHC,0);
			break;
		case (MATHC & 0xff):
			TRACE_SUSIE2("Poke(MATHC,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.C = data;
			// Perform sign conversion if required
			if (mSPRSYS & SignedMath) {
				// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
				if ((mMATHABCD.Words.CD - 1) & 0x8000) {
					UWORD conv;
					conv = mMATHABCD.Words.CD ^ 0xffff;
					conv++;
					mMATHCD_sign = -1;
					TRACE_SUSIE2("MATH CD signed conversion complete %04x to %04x", mMATHABCD.Words.CD, conv);
					mMATHABCD.Words.CD = conv;
				}
				else
				{
					mMATHCD_sign = 1;
				}
			}
			break;
		case (MATHB & 0xff):
			TRACE_SUSIE2("Poke(MATHB,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.B = data;
			mMATHABCD.Bytes.A = 0;
			break;
		case (MATHA & 0xff):
			TRACE_SUSIE2("Poke(MATHA,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.A = data;
			// Perform sign conversion if required
			if (mSPRSYS & SignedMath) {
				// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
				if ((mMATHABCD.Words.AB - 1) & 0x8000) {
					UWORD conv;
					conv = mMATHABCD.Words.AB ^ 0xffff;
					conv++;
					mMATHAB_sign = -1;
					TRACE_SUSIE2("MATH AB signed conversion complete %04x to %04x", mMATHABCD.Words.AB, conv);
					mMATHABCD.Words.AB = conv;
				}
				else {
					mMATHAB_sign = 1;
				}
			}
			DoMathMultiply();
			break;

		case (MATHP & 0xff):
			mMATHNP.Bytes.P = data;
			mMATHNP.Bytes.N = 0;
			TRACE_SUSIE2("Poke(MATHP,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHN & 0xff):
			mMATHNP.Bytes.N = data;
			TRACE_SUSIE2("Poke(MATHN,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;

		case (MATHH & 0xff):
			mMATHEFGH.Bytes.H = data;
			mMATHEFGH.Bytes.G = 0;
			TRACE_SUSIE2("Poke(MATHH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHG & 0xff):
			mMATHEFGH.Bytes.G = data;
			TRACE_SUSIE2("Poke(MATHG,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHF & 0xff):
			mMATHEFGH.Bytes.F = data;
			mMATHEFGH.Bytes.E = 0;
			TRACE_SUSIE2("Poke(MATHF,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHE & 0xff):
			mMATHEFGH.Bytes.E = data;
			TRACE_SUSIE2("Poke(MATHE,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			DoMathDivide();
			break;

		case (MATHM & 0xff):
			mMATHJKLM.Bytes.M = data;
			mMATHJKLM.Bytes.L = 0;
			mSPRSYS_Mathbit = FALSE;
			TRACE_SUSIE2("Poke(MATHM,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHL & 0xff):
			mMATHJKLM.Bytes.L = data;
			TRACE_SUSIE2("Poke(MATHL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHK & 0xff):
			mMATHJKLM.Bytes.K = data;
			mMATHJKLM.Bytes.J = 0;
			TRACE_SUSIE2("Poke(MATHK,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (MATHJ & 0xff):
			mMATHJKLM.Bytes.J = data;
			TRACE_SUSIE2("Poke(MATHJ,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;

		case (SPRSYS & 0xff):
			if (data & UnsafeAccess) mSPRSYS_UnsafeAccess = 0;
			mSPRSYS = data;
			TRACE_SUSIE2("Poke(SPRSYS,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;

// Cartridge writing ports

		case (RCART0 & 0xff):
			mSystem.Poke_CARTB0(data);
			TRACE_SUSIE2("Poke(RCART0,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (RCART1 & 0xff):
			mSystem.Poke_CARTB1(data);
			TRACE_SUSIE2("Poke(RCART1,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;

// Errors on illegal location accesses

		default:
			lnxSuzyWrite(addr, data);
			TRACE_SUSIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x", addr, data, mSystem.mCpu->GetPC());
			break;
	}
}

UBYTE CSusie::Peek(ULONG addr)
{
	UBYTE retval = 0;
	switch(addr & 0xff)
	{
		case (MATHD & 0xff):
			retval = mMATHABCD.Bytes.D;
			TRACE_SUSIE2("Peek(MATHD)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHC & 0xff):
			retval = mMATHABCD.Bytes.C;
			TRACE_SUSIE2("Peek(MATHC)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHB & 0xff):
			retval = mMATHABCD.Bytes.B;
			TRACE_SUSIE2("Peek(MATHB)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHA & 0xff):
			retval = mMATHABCD.Bytes.A;
			TRACE_SUSIE2("Peek(MATHA)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (MATHP & 0xff):
			retval = mMATHNP.Bytes.P;
			TRACE_SUSIE2("Peek(MATHP)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHN & 0xff):
			retval = mMATHNP.Bytes.N;
			TRACE_SUSIE2("Peek(MATHN)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (MATHH & 0xff):
			retval = mMATHEFGH.Bytes.H;
			TRACE_SUSIE2("Peek(MATHH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHG & 0xff):
			retval = mMATHEFGH.Bytes.G;
			TRACE_SUSIE2("Peek(MATHG)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHF & 0xff):
			retval = mMATHEFGH.Bytes.F;
			TRACE_SUSIE2("Peek(MATHF)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHE & 0xff):
			retval = mMATHEFGH.Bytes.E;
			TRACE_SUSIE2("Peek(MATHE)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (MATHM & 0xff):
			retval = mMATHJKLM.Bytes.M;
			TRACE_SUSIE2("Peek(MATHM)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHL & 0xff):
			retval = mMATHJKLM.Bytes.L;
			TRACE_SUSIE2("Peek(MATHL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHK & 0xff):
			retval = mMATHJKLM.Bytes.K;
			TRACE_SUSIE2("Peek(MATHK)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (MATHJ & 0xff):
			retval = mMATHJKLM.Bytes.J;
			TRACE_SUSIE2("Peek(MATHJ)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (SPRSYS & 0xff):
			retval = 0x0000;
			//	retval |= (mSPRSYS_Busy) ? 0x0001 : 0x0000;
			retval |= (mikey_0.suzieDoneTime) ? 0x0001 : 0x0000;
			retval |= (mSPRSYS & StopOnCurrent);
			retval |= (mSPRSYS_UnsafeAccess) ? 0x0004 : 0x0000;
			retval |= (mSPRSYS & LeftHand);
			retval |= (mSPRSYS & VStretch);
			retval |= (mSPRSYS_LastCarry) ? 0x0020 : 0x0000;
			retval |= (mSPRSYS_Mathbit) ? 0x0040 : 0x0000;
			retval |= (mSPRSYS_MathInProgress) ? 0x0080 : 0x0000;
			TRACE_SUSIE2("Peek(SPRSYS)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (JOYSTICK & 0xff):
			if (mSPRSYS & LeftHand) {
				retval = mJOYSTICK.Byte;
			}
			else {
				TJOYSTICK Modified = mJOYSTICK;
				Modified.Bits.Left = mJOYSTICK.Bits.Right;
				Modified.Bits.Right = mJOYSTICK.Bits.Left;
				Modified.Bits.Down = mJOYSTICK.Bits.Up;
				Modified.Bits.Up = mJOYSTICK.Bits.Down;
				retval = Modified.Byte;
			}
//			TRACE_SUSIE2("Peek(JOYSTICK)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (SWITCHES & 0xff):
			retval = mSWITCHES.Byte;
//			TRACE_SUSIE2("Peek(SWITCHES)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

// Cartridge reading ports

		case (RCART0 & 0xff):
			retval = mSystem.Peek_CARTB0();
//			TRACE_SUSIE2("Peek(RCART0)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (RCART1 & 0xff):
			retval = mSystem.Peek_CARTB1();
//			TRACE_SUSIE2("Peek(RCART1)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

// Errors on illegal location accesses

		default:
			TRACE_SUSIE2("Peek(%04x) - Peek from illegal location at PC=$%04x", addr, mSystem.mCpu->GetPC());
			return lnxSuzyRead(addr);
	}

	return 0xff;
}
