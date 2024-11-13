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

#define mHOFF suzy_0.hOff
#define mVOFF suzy_0.vOff
#define mTILTACUM suzy_0.tiltAcum
#define mVIDBAS suzy_0.vidBas
#define mCOLLBAS suzy_0.collBas
#define mSCBNEXT suzy_0.SCBNext
#define mCOLLOFF suzy_0.collOff
#define mHSIZOFF suzy_0.hSizOff
#define mVSIZOFF suzy_0.vSizOff
#define mSPRCOLL suzy_0.sprColl
#define mSUZYBUSEN suzy_0.suzyBusEn

#define mLineBaseAddress suzy_0.lineBaseAddress
#define mLineCollisionAddress suzy_0.lineCollisionAddress
#define mCollision suzy_0.collision
#define cycles_used suzy_0.cyclesUsed

//ULONG cycles_used=0;

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

	mTMPADR.Word = 0;
	mSPRDLINE.Word = 0;
	mHPOSSTRT.Word = 0;
	mVPOSSTRT.Word = 0;
	mSPRHSIZ.Word = 0;
	mSPRVSIZ.Word = 0;
	mSTRETCH.Word = 0;
	mTILT.Word = 0;
	mSPRDOFF.Word = 0;
	mVSIZACUM.Word = 0;
	mHSIZACUM.Word = 0;
	mSCBADR.Word = 0;

	// Must be initialised to this due to
	// stun runner math initialisation bug
	// see whatsnew for 0.7
	mMATHABCD.Long = 0xffffffff;
	mMATHEFGH.Long = 0xffffffff;
	mMATHJKLM.Long = 0xffffffff;
	mMATHNP.Long = 0xffff;

	mMATHAB_sign = 1;
	mMATHCD_sign = 1;
	mMATHEFGH_sign = 1;

	mSPRCTL0_Type = 0;
	mSPRCTL0_Vflip = 0;
	mSPRCTL0_Hflip = 0;
	mSPRCTL0_PixelBits = 0;

	mSPRCTL1_StartLeft = 0;
	mSPRCTL1_StartUp = 0;
	mSPRCTL1_SkipSprite = 0;
	mSPRCTL1_ReloadPalette = 0;
	mSPRCTL1_ReloadDepth = 0;
	mSPRCTL1_Sizing = 0;
	mSPRCTL1_Literal = 0;

//	mSPRCOLL = 0;

	mSPRSYS_StopOnCurrent = 0;
	mSPRSYS_LeftHand = 0;
	mSPRSYS_VStretch = 0;
	mSPRSYS_NoCollide = 0;
	mSPRSYS_Accumulate = 0;
	mSPRSYS_SignedMath = 0;
	mSPRSYS_Status = 0;
	mSPRSYS_UnsafeAccess = 0;
	mSPRSYS_LastCarry = 0;
	mSPRSYS_Mathbit = 0;
	mSPRSYS_MathInProgress = 0;

	mSPRINIT.Byte = 0;

	mSPRGO = FALSE;
	mEVERON = FALSE;

	for (int loop=0;loop<16;loop++) mPenIndex[loop] = loop;

	mLineType = 0;
	mLineShiftRegCount = 0;
	mLineShiftReg = 0;
	mLineRepeatCount = 0;
	mLinePixel = 0;
	mLinePacketBitsLeft = 0;
//	mCollision = 0;
//	mLineCollisionAddress = 0;

	hquadoff = vquadoff = 0;

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

	if (mSPRSYS_SignedMath) {
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
	if (mSPRSYS_Accumulate) {
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
	int	sprcount = 0;
	int data = 0;
	int everonscreen = 0;

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

	if (!mSUZYBUSEN || !mSPRGO) {
		TRACE_SUSIE0("PaintSprites() Returned !mSUZYBUSEN || !mSPRGO");
		return 0;
	}

	cycles_used = 0;
	everonscreen = 0;

	do {
		TRACE_SUSIE1("PaintSprites() ************ Rendering Sprite %03d ************", sprcount);

		// Step 1 load up the SCB params into Susie

		// And thus it is documented that only the top byte of SCBNEXT is used.
		// Its mentioned under the bits that are broke section in the bluebook
		if (!(mSCBNEXT.Word & 0xff00)) {
			TRACE_SUSIE0("PaintSprites() mSCBNEXT==0 - FINISHED");
			mSPRSYS_Status = 0;	// Engine has finished
			mSPRGO = FALSE;
			break;
		}
		else {
			mSPRSYS_Status = 1;
		}

		mTMPADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
		mSCBADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
		TRACE_SUSIE1("PaintSprites() SCBADDR $%04x", mSCBADR.Word);

		data = RAM_PEEK(mTMPADR.Word);			// Fetch control 0
		TRACE_SUSIE1("PaintSprites() SPRCTL0 $%02x", data);
		mSPRCTL0_Type = data & 0x0007;
		mSPRCTL0_Vflip = data & 0x0010;
		mSPRCTL0_Hflip = data & 0x0020;
		mSPRCTL0_PixelBits = ((data & 0x00c0)>>6) + 1;
		mTMPADR.Word += 1;

		data = RAM_PEEK(mTMPADR.Word);			// Fetch control 1
		TRACE_SUSIE1("PaintSprites() SPRCTL1 $%02x", data);
		mSPRCTL1_StartLeft = data & 0x0001;
		mSPRCTL1_StartUp = data & 0x0002;
		mSPRCTL1_SkipSprite = data & 0x0004;
		mSPRCTL1_ReloadPalette = data & 0x0008;
		mSPRCTL1_ReloadDepth = (data & 0x0030)>>4;
		mSPRCTL1_Sizing = data & 0x0040;
		mSPRCTL1_Literal = data & 0x0080;
		mTMPADR.Word += 1;

		data = RAM_PEEK(mTMPADR.Word);			// Collision num
		TRACE_SUSIE1("PaintSprites() SPRCOLL $%02x", data);
		mSPRCOLL = (data & 0x002f) | mSPRSYS_NoCollide;
		mTMPADR.Word += 1;

		mSCBNEXT.Word = RAM_PEEKW(mTMPADR.Word);	// Next SCB
		TRACE_SUSIE1("PaintSprites() SCBNEXT $%04x", mSCBNEXT.Word);
		mTMPADR.Word += 2;

		cycles_used += 5 * SPR_RDWR_CYC;

		// Check if this is a skip sprite

		if (!mSPRCTL1_SkipSprite) {
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
			switch(mSPRCTL1_ReloadDepth)
			{
				case 1:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					// enable_sizing=TRUE; // unused

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					cycles_used += 4 * SPR_RDWR_CYC;
					break;

				case 2:
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

				case 3:
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

			if (!mSPRCTL1_ReloadPalette) {

				TRACE_SUSIE0("PaintSprites() Palette reloaded");
				for(int loop=0;loop<8;loop++) {
					UBYTE data = RAM_PEEK(mTMPADR.Word++);
					mPenIndex[loop*2] = (data>>4) & 0x0f;
					mPenIndex[(loop*2)+1] = data & 0x0f;
				}
				// Increment cycle count for the reads
				cycles_used += 8 * SPR_RDWR_CYC;
			}

			// Now we can start painting
		
			// Quadrant drawing order is: SE,NE,NW,SW
			// start quadrant is given by sprite_control1:0 & 1

			// Setup screen start end variables

			int screen_h_start = (SWORD)mHOFF.Word;
			int screen_h_end = (SWORD)mHOFF.Word + LYNX_SCREEN_WIDTH;
			int screen_v_start = (SWORD)mVOFF.Word;
			int screen_v_end = (SWORD)mVOFF.Word + LYNX_SCREEN_HEIGHT;

			int world_h_mid = screen_h_start + 0x8000 + (LYNX_SCREEN_WIDTH / 2);
			int world_v_mid = screen_v_start + 0x8000 + (LYNX_SCREEN_HEIGHT / 2);

			TRACE_SUSIE2("PaintSprites() screen_h_start $%04x screen_h_end $%04x", screen_h_start, screen_h_end);
			TRACE_SUSIE2("PaintSprites() screen_v_start $%04x screen_v_end $%04x", screen_v_start, screen_v_end);
			TRACE_SUSIE2("PaintSprites() world_h_mid    $%04x world_v_mid  $%04x", world_h_mid, world_v_mid);

			bool superclip = FALSE;
			int quadrant = 0;
			int hsign, vsign;

			if (mSPRCTL1_StartLeft) {
				if (mSPRCTL1_StartUp) quadrant = 2; else quadrant = 3;
			}
			else {
				if (mSPRCTL1_StartUp) quadrant = 1; else quadrant = 0;
			}
			TRACE_SUSIE1("PaintSprites() Quadrant=%d", quadrant);

			// Check ref is inside screen area. !! This is commented out in Mednafen!!
			if ((SWORD)mHPOSSTRT.Word<screen_h_start || (SWORD)mHPOSSTRT.Word>=screen_h_end ||
				(SWORD)mVPOSSTRT.Word<screen_v_start || (SWORD)mVPOSSTRT.Word>=screen_v_end) superclip = TRUE;

			TRACE_SUSIE1("PaintSprites() Superclip=%d",superclip);


			// Quadrant mapping is:	SE	NE	NW	SW
			//						0	1	2	3
			// hsign				+1	+1	-1	-1
			// vsign				+1	-1	-1	+1
			//
			//
			//		2 | 1
			//     -------
			//      3 | 0
			//

			// Loop for 4 quadrants

			for (int loop=0;loop<4;loop++)
			{
				TRACE_SUSIE1("PaintSprites() -------- Rendering Quadrant %03d --------", quadrant);

				int sprite_v = mVPOSSTRT.Word;
				int sprite_h = mHPOSSTRT.Word;

				bool render = FALSE;

				// Set quadrand multipliers
				hsign = (quadrant == 0 || quadrant == 1) ? 1 : -1;
				vsign = (quadrant == 0 || quadrant == 3) ? 1 : -1;

// Preflip		TRACE_SUSIE2("PaintSprites() hsign=%d vsign=%d",hsign,vsign);

				// Use h/v flip to invert h/vsign

				if (mSPRCTL0_Vflip) vsign = -vsign;
				if (mSPRCTL0_Hflip) hsign = -hsign;

				TRACE_SUSIE2("PaintSprites() Hflip=%d Vflip=%d", mSPRCTL0_Hflip, mSPRCTL0_Vflip);
				TRACE_SUSIE2("PaintSprites() Hsign=%d   Vsign=%d", hsign, vsign);
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
					// hsign				+1	+1	-1	-1
					// vsign				+1	-1	-1	+1
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
					static const int vquadflip[4] = {1,0,3,2};
					static const int hquadflip[4] = {3,2,1,0};

					if (mSPRCTL0_Vflip) modquad = vquadflip[modquad];
					if (mSPRCTL0_Hflip) modquad = hquadflip[modquad];

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
				else
				{
					render = TRUE;
				}

				// Is this quad to be rendered ??

				TRACE_SUSIE1("PaintSprites() Render status %d", render);

				int pixel_height;
				int pixel_width;
				int pixel;
				int hoff,voff;
				int hloop;
				bool onscreen;

				if (render) {
					// Set the vertical position & offset
					voff = (SWORD)mVPOSSTRT.Word-screen_v_start;

					// Zero the stretch,tilt & acum values
					mTILTACUM.Word = 0;

					// Perform the SIZOFF
					if (vsign == 1) mVSIZACUM.Word = mVSIZOFF.Word; else mVSIZACUM.Word = 0;

					// Take the sign of the first quad (0) as the basic
					// sign, all other quads drawing in the other direction
					// get offset by 1 pixel in the other direction, this
					// fixes the squashed look on the multi-quad sprites.
//					if (vsign == -1 && loop > 0) voff += vsign;
					if (loop == 0) vquadoff = vsign;
					if (vsign != vquadoff) voff += vsign;

					for (;;) {
						// Vertical scaling is done here
						mVSIZACUM.Word += mSPRVSIZ.Word;
						pixel_height = mVSIZACUM.Byte.High;
						mVSIZACUM.Byte.High = 0;

						// Update the next data line pointer and initialise our line
						mSPRDOFF.Word = (UWORD)LineInit(0);

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
							if (vsign == 1 && voff >= LYNX_SCREEN_HEIGHT) break;
							if (vsign == -1 && voff < 0) break;

							// Only allow the draw to take place if the line is visible
							if (voff >= 0 && voff < LYNX_SCREEN_HEIGHT) {
								// Work out the horizontal pixel start position, start + tilt
								mHPOSSTRT.Word += ((SWORD)mTILTACUM.Word>>8);
								mTILTACUM.Byte.High = 0;
								hoff = (int)((SWORD)mHPOSSTRT.Word) - screen_h_start;

								// Zero/Force the horizontal scaling accumulator
								mHSIZACUM.Word = (hsign == 1) ? mHSIZOFF.Word : 0;

								// Take the sign of the first quad (0) as the basic
								// sign, all other quads drawing in the other direction
								// get offset by 1 pixel in the other direction, this
								// fixes the squashed look on the multi-quad sprites.
//								if (hsign == -1 && loop > 0) hoff += hsign;
								if (loop ==0) hquadoff = hsign;
								if (hsign != hquadoff) hoff += hsign;

								// Initialise our line
								LineInit(voff);
								onscreen = FALSE;

								// Now render an individual destination line
								while ((pixel = LineGetPixel()) != LINE_END) {
									// This is allowed to update every pixel
									mHSIZACUM.Word += mSPRHSIZ.Word;
									pixel_width = mHSIZACUM.Byte.High;
									mHSIZACUM.Byte.High = 0;

									for(hloop=0;hloop<pixel_width;hloop++) {
										// Draw if onscreen but break loop on transition to offscreen
										if (hoff >= 0 && hoff < LYNX_SCREEN_WIDTH) {
											ProcessPixel(hoff,pixel);
											onscreen = TRUE;
											everonscreen = TRUE;
										}
										else {
											if (onscreen) break;
										}
										hoff += hsign;
									}
								}
							}
							voff += vsign;

							// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
							if (enable_stretch) {
								mSPRHSIZ.Word += mSTRETCH.Word;
//								if (mSPRSYS_VStretch) mSPRVSIZ.Word += mSTRETCH.Word;
							}
							if (enable_tilt) {
								// Manipulate the tilt stuff
								mTILTACUM.Word += mTILT.Word;
							}
						}
						// According to the docs this increments per dest line
						// but only gets set when the source line is read
						if (mSPRSYS_VStretch) mSPRVSIZ.Word += mSTRETCH.Word*pixel_height;

						// Update the line start for our next run thru the loop
						mSPRDLINE.Word += mSPRDOFF.Word;
					}
				}
				else {
					// Skip thru data to next quad
					for(;;) {
						// Read the start of line offset

						mSPRDOFF.Word = (UWORD)LineInit(0);

						// We dont want to process data so mSPRDLINE is useless to us
						mSPRDLINE.Word += mSPRDOFF.Word;

						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE

						if (mSPRDOFF.Word == 1) break;	// End of quad
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

			if (mSPRCOLL <= 0xF) {
				switch(mSPRCTL0_Type)
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

			if (mEVERON) {
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
//			mSPRSYS.Read.Status = 0;	// Engine has finished
//			mSPRGO = FALSE;
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

//
// Collision code modified by KW 22/11/98
// Collision buffer cler added if there is no
// apparent collision, I have a gut feeling this
// is the wrong solution to the inv07.com bug but
// it seems to work OK.
//
// Shadow-------------------------------|
// Boundary-Shadow--------------------| |
// Normal---------------------------| | |
// Boundary-----------------------| | | |
// Background-Shadow------------| | | | |
// Background-No Collision----| | | | | |
// Non-Collideable----------| | | | | | |
// Exclusive-or-Shadow----| | | | | | | |
//                        | | | | | | | |
//                        1 1 1 1 0 1 0 1   F is opaque
//                        0 0 0 0 1 1 0 0   E is collideable
//                        0 0 1 1 0 0 0 0   0 is opaque and collideable
//                        1 0 0 0 1 1 1 1   allow collision detect
//                        1 0 0 1 1 1 1 1   allow coll. buffer access
//                        1 0 0 0 0 0 0 0   exclusive-or the data
//

inline void CSusie::ProcessPixel(ULONG hoff, ULONG pixel)
{
	suzProcessPixel(hoff, pixel, mSPRCTL0_Type);

/*	switch(mSPRCTL0_Type)
	{
		// BACKGROUND SHADOW
		// 1   F is opaque
		// 0   E is collideable
		// 1   0 is opaque and collideable
		// 0   allow collision detect
		// 1   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_background_shadow:
			suzWritePixel(hoff, pixel);
			if (pixel != 0x0e) {
				suzWriteCollision(hoff, mSPRCOLL);
			}
			break;

		// BACKGROUND NOCOLLIDE
		// 1   F is opaque
		// 0   E is collideable
		// 1   0 is opaque and collideable
		// 0   allow collision detect
		// 0   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_background_noncollide:
			suzWritePixel(hoff, pixel);
			break;

		// NOCOLLIDE
		// 1   F is opaque
		// 0   E is collideable
		// 0   0 is opaque and collideable
		// 0   allow collision detect
		// 0   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_noncollide:
			if (pixel != 0x00) {
				suzWritePixel(hoff, pixel);
			}
			break;

		// BOUNDARY
		// 0   F is opaque
		// 1   E is collideable
		// 0   0 is opaque and collideable
		// 1   allow collision detect
		// 1   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_boundary:
			if (pixel != 0x00) {
				if (pixel != 0x0f) {
					suzWritePixel(hoff, pixel);
				}
				suzTestCollision(hoff, mSPRCOLL);
			}
			break;

		// NORMAL
		// 1   F is opaque
		// 1   E is collideable
		// 0   0 is opaque and collideable
		// 1   allow collision detect
		// 1   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_normal:
			if (pixel != 0x00) {
				suzWritePixel(hoff, pixel);
				suzTestCollision(hoff, mSPRCOLL);
			}
			break;

		// BOUNDARY_SHADOW
		// 0   F is opaque
		// 0   E is collideable
		// 0   0 is opaque and collideable
		// 1   allow collision detect
		// 1   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_boundary_shadow:
			if (pixel != 0x00 && pixel != 0x0e && pixel != 0x0f) {
				suzWritePixel(hoff, pixel);
			}
			if (pixel != 0x00 && pixel != 0x0e) {
				suzTestCollision(hoff, mSPRCOLL);
			}
			break;

		// SHADOW
		// 1   F is opaque
		// 0   E is collideable
		// 0   0 is opaque and collideable
		// 1   allow collision detect
		// 1   allow coll. buffer access
		// 0   exclusive-or the data
		case sprite_shadow:
			if (pixel != 0x00) {
				suzWritePixel(hoff, pixel);
				if (pixel != 0x0e) {
					suzTestCollision(hoff, mSPRCOLL);
				}
			}
			break;

		// XOR SHADOW
		// 1   F is opaque
		// 0   E is collideable
		// 0   0 is opaque and collideable
		// 1   allow collision detect
		// 1   allow coll. buffer access
		// 1   exclusive-or the data
		case sprite_xor_shadow:
			if (pixel != 0x00) {
				suzXorPixel(hoff, pixel);
				if (pixel != 0x0e) {
					suzTestCollision(hoff, mSPRCOLL);
				}
			}
			break;
		default:
			break;
	}*/
}
/*
inline void CSusie::TestCollision(ULONG hoff, ULONG pixel)
{
	if (pixel <= 0xF) {
		suzTestCollision(hoff, pixel);
	}
}
*/
inline ULONG CSusie::LineInit(ULONG voff)
{
	mLineShiftReg = 0;
	mLineShiftRegCount = 0;
	mLineRepeatCount = 0;
	mLinePixel = 0;
	mLineType = line_error;
	mLinePacketBitsLeft = 0xffff;

	// Initialise the temporary pointer

	mTMPADR = mSPRDLINE;

	// First read the Offset to the next line

	ULONG offset = LineGetBits(8);

	// Specify the MAXIMUM number of bits in this packet, it
	// can terminate early but can never use more than this
	// without ending the current packet, we count down in LineGetBits()

	mLinePacketBitsLeft = (offset - 1) * 8;

	// Literals are a special case and get their count set on a line basis

	if (mSPRCTL1_Literal) {
		mLineType = line_abs_literal;
		mLineRepeatCount = ((offset - 1) * 8) / mSPRCTL0_PixelBits;
	}

	// Set the line base address for use in the calls to pixel painting

	if (voff > 101) {
		//gError->Warning("CSusie::LineInit() Out of bounds (voff)");
		voff = 0;
	}

	mLineBaseAddress = mVIDBAS.Word + (voff * (LYNX_SCREEN_WIDTH / 2));
	mLineCollisionAddress = mCOLLBAS.Word + (voff * (LYNX_SCREEN_WIDTH / 2));

	// Return the offset to the next line of sprite data.
	return offset;
}

inline ULONG CSusie::LineGetPixel()
{
	if (!mLineRepeatCount) {
		// Normal sprites fetch their counts on a packet basis
		if (mLineType != line_abs_literal) {
			ULONG literal = LineGetBits(1);
			if (literal) mLineType = line_literal; else mLineType = line_packed;
		}

		// Pixel store is empty what should we do
		switch (mLineType) {
			case line_abs_literal:
				// This means end of line for us
				mLinePixel = LINE_END;
				return mLinePixel;		// SPEEDUP
				break;
			case line_literal:
				mLineRepeatCount = LineGetBits(4);
				mLineRepeatCount++;
				break;
			case line_packed:
				//
				// From reading in between the lines only a packed line with
				// a zero size i.e 0b0000 as a header is allowable as a packet end
				//
				mLineRepeatCount = LineGetBits(4);
				if (!mLineRepeatCount) {
					mLinePixel = LINE_END;
				}
				else {
					mLinePixel = mPenIndex[LineGetBits(mSPRCTL0_PixelBits)];
				}
				mLineRepeatCount++;
				break;
			default:
				return 0;
		}
	}

	if (mLinePixel != LINE_END) {
		mLineRepeatCount--;

		switch(mLineType) {
			case line_abs_literal:
				mLinePixel = LineGetBits(mSPRCTL0_PixelBits);
				// Check the special case of a zero in the last pixel
				if (!mLineRepeatCount && !mLinePixel)
					mLinePixel = LINE_END;
				else
					mLinePixel = mPenIndex[mLinePixel];
				break;
			case line_literal:
				mLinePixel = mPenIndex[LineGetBits(mSPRCTL0_PixelBits)];
				break;
			case line_packed:
				break;
			default:
				return 0;
		}
	}

	return mLinePixel;
}

inline ULONG CSusie::LineGetBits(ULONG bits)
{
	ULONG retval;

	// Sanity, not really needed
	// if (bits > 32) return 0;

	// Only return data IF there is enought bits left in the packet

	//if (mLinePacketBitsLeft < bits) return 0;
	if (mLinePacketBitsLeft <= bits) return 0;	// Hardware bug(<= instead of <), apparently

	// Make sure shift reg has enough bits to fulfil the request

	if (mLineShiftRegCount < bits) {
		// This assumes data comes into LSB and out of MSB
		mLineShiftReg <<= 24;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++)<<16;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++)<<8;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++);

		mLineShiftRegCount += 24;

		// Increment cycle count for the read
		cycles_used += 3 * SPR_RDWR_CYC;
	}

	// Extract the return value
	retval = mLineShiftReg >> (mLineShiftRegCount - bits);
	retval &= (1 << bits) - 1;

	// Update internal vars;
	mLineShiftRegCount -= bits;
	mLinePacketBitsLeft -= bits;

	return retval;
}

void CSusie::Poke(ULONG addr, UBYTE data)
{
	switch(addr & 0xff)
	{
		case (TMPADRL & 0xff):
			mTMPADR.Byte.Low = data;
			mTMPADR.Byte.High = 0;
			TRACE_SUSIE2("Poke(TMPADRL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TMPADRH & 0xff):
			mTMPADR.Byte.High = data;
			TRACE_SUSIE2("Poke(TMPADRH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
//		case (HOFFL & 0xff):
//			mHOFF.Byte.Low = data;
//			mHOFF.Byte.High = 0;
//			TRACE_SUSIE2("Poke(HOFFL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
//		case (HOFFH & 0xff):
//			mHOFF.Byte.High = data;
//			TRACE_SUSIE2("Poke(HOFFH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
//		case (VOFFL & 0xff):
//			mVOFF.Byte.Low = data;
//			mVOFF.Byte.High = 0;
//			TRACE_SUSIE2("Poke(VOFFL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
//		case (VOFFH & 0xff):
//			mVOFF.Byte.High = data;
//			TRACE_SUSIE2("Poke(VOFFH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
		case (SPRDLINEL & 0xff):
			mSPRDLINE.Byte.Low = data;
			mSPRDLINE.Byte.High = 0;
			TRACE_SUSIE2("Poke(SPRDLINEL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRDLINEH & 0xff):
			mSPRDLINE.Byte.High = data;
			TRACE_SUSIE2("Poke(SPRDLINEH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (HPOSSTRTL & 0xff):
			mHPOSSTRT.Byte.Low = data;
			mHPOSSTRT.Byte.High = 0;
			TRACE_SUSIE2("Poke(HPOSSTRTL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (HPOSSTRTH & 0xff):
			mHPOSSTRT.Byte.High = data;
			TRACE_SUSIE2("Poke(HPOSSTRTH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (VPOSSTRTL & 0xff):
			mVPOSSTRT.Byte.Low = data;
			mVPOSSTRT.Byte.High = 0;
			TRACE_SUSIE2("Poke(VPOSSTRTL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (VPOSSTRTH & 0xff):
			mVPOSSTRT.Byte.High = data;
			TRACE_SUSIE2("Poke(VPOSSTRTH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRHSIZL & 0xff):
			mSPRHSIZ.Byte.Low = data;
			mSPRHSIZ.Byte.High = 0;
			TRACE_SUSIE2("Poke(SPRHSIZL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRHSIZH & 0xff):
			mSPRHSIZ.Byte.High = data;
			TRACE_SUSIE2("Poke(SPRHSIZH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRVSIZL & 0xff):
			mSPRVSIZ.Byte.Low = data;
			mSPRVSIZ.Byte.High = 0;
			TRACE_SUSIE2("Poke(SPRVSIZL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRVSIZH & 0xff):
			mSPRVSIZ.Byte.High = data;
			TRACE_SUSIE2("Poke(SPRVSIZH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (STRETCHL & 0xff):
			mSTRETCH.Byte.Low = data;
			mSTRETCH.Byte.High = 0;
			TRACE_SUSIE2("Poke(STRETCHL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (STRETCHH & 0xff):
			TRACE_SUSIE2("Poke(STRETCHH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mSTRETCH.Byte.High = data;
			break;
		case (TILTL & 0xff):
			mTILT.Byte.Low = data;
			mTILT.Byte.High = 0;
			TRACE_SUSIE2("Poke(TILTL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TILTH & 0xff):
			mTILT.Byte.High = data;
			TRACE_SUSIE2("Poke(TILTH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRDOFFL & 0xff):
			TRACE_SUSIE2("Poke(SPRDOFFL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mSPRDOFF.Byte.Low = data;
			mSPRDOFF.Byte.High = 0;
			break;
		case (SPRDOFFH & 0xff):
			TRACE_SUSIE2("Poke(SPRDOFFH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mSPRDOFF.Byte.High = data;
			break;
		case (VSIZACUML & 0xff):
			mVSIZACUM.Byte.Low = data;
			mVSIZACUM.Byte.High = 0;
			TRACE_SUSIE2("Poke(VSIZACUML,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (VSIZACUMH & 0xff):
			mVSIZACUM.Byte.High = data;
			TRACE_SUSIE2("Poke(VSIZACUMH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SCBADRL & 0xff):
			mSCBADR.Byte.Low = data;
			mSCBADR.Byte.High = 0;
			TRACE_SUSIE2("Poke(SCBADRL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SCBADRH & 0xff):
			mSCBADR.Byte.High = data;
			TRACE_SUSIE2("Poke(SCBADRH,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;

		case (MATHD & 0xff):
			TRACE_SUSIE2("Poke(MATHD,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.D = data;
//			mMATHABCD.Bytes.C = 0;
			// The hardware manual says that the sign shouldnt change
			// but if I dont do this then stun runner will hang as it
			// does the init in the wrong order and if the previous
			// calc left a zero there then we'll get a sign error
			Poke(MATHC,0);
			break;
		case (MATHC & 0xff):
			TRACE_SUSIE2("Poke(MATHC,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.C = data;
			// Perform sign conversion if required
			if (mSPRSYS_SignedMath) {
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
			if (mSPRSYS_SignedMath) {
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

		case (SPRCTL0 & 0xff):
			mSPRCTL0_Type = data & 0x0007;
			mSPRCTL0_Vflip = data & 0x0010;
			mSPRCTL0_Hflip = data & 0x0020;
			mSPRCTL0_PixelBits = ((data & 0x00c0)>>6) + 1;
			TRACE_SUSIE2("Poke(SPRCTL0,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRCTL1 & 0xff):
			mSPRCTL1_StartLeft = data & 0x0001;
			mSPRCTL1_StartUp = data & 0x0002;
			mSPRCTL1_SkipSprite = data & 0x0004;
			mSPRCTL1_ReloadPalette = data & 0x0008;
			mSPRCTL1_ReloadDepth = (data & 0x0030)>>4;
			mSPRCTL1_Sizing = data & 0x0040;
			mSPRCTL1_Literal = data & 0x0080;
			TRACE_SUSIE2("Poke(SPRCTL1,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
//		case (SPRCOLL & 0xff):
//			mSPRCOLL = data & 0x002f;
//			TRACE_SUSIE2("Poke(SPRCOLL,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
		case (SPRINIT & 0xff):
			mSPRINIT.Byte = data;
			TRACE_SUSIE2("Poke(SPRINIT,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
//		case (SUZYBUSEN & 0xff):
//			mSUZYBUSEN = data & 0x01;
//			TRACE_SUSIE2("Poke(SUZYBUSEN,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
//			break;
		case (SPRGO & 0xff):
			mSPRGO = data & 0x01;
			mEVERON = data & 0x04;
			TRACE_SUSIE2("Poke(SPRGO,%02x) at PC=$%04x", data, mSystem.mCpu->GetPC());
			break;
		case (SPRSYS & 0xff):
			mSPRSYS_StopOnCurrent = data & 0x0002;
			if (data & 0x0004) mSPRSYS_UnsafeAccess = 0;
			mSPRSYS_LeftHand = data & 0x0008;
			mSPRSYS_VStretch = data & 0x0010;
			mSPRSYS_NoCollide = data & 0x0020;
			mSPRSYS_Accumulate = data & 0x0040;
			mSPRSYS_SignedMath = data & 0x0080;
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
		case (TMPADRL & 0xff):
			retval = mTMPADR.Byte.Low;
			TRACE_SUSIE2("Peek(TMPADRL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (TMPADRH & 0xff):
			retval = mTMPADR.Byte.High;
			TRACE_SUSIE2("Peek(TMPADRH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
//		case (HOFFL & 0xff):
//			retval = mHOFF.Byte.Low;
//			TRACE_SUSIE2("Peek(HOFFL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
//			return retval;
//		case (HOFFH & 0xff):
//			retval = mHOFF.Byte.High;
//			TRACE_SUSIE2("Peek(HOFFH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
//			return retval;
//		case (VOFFL & 0xff):
//			retval = mVOFF.Byte.Low;
//			TRACE_SUSIE2("Peek(VOFFL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
//			return retval;
//		case (VOFFH & 0xff):
//			retval = mVOFF.Byte.High;
//			TRACE_SUSIE2("Peek(VOFFH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
//			return retval;
		case (SPRDLINEL & 0xff):
			retval = mSPRDLINE.Byte.Low;
			TRACE_SUSIE2("Peek(SPRDLINEL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRDLINEH & 0xff):
			retval = mSPRDLINE.Byte.High;
			TRACE_SUSIE2("Peek(SPRDLINEH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (HPOSSTRTL & 0xff):
			retval = mHPOSSTRT.Byte.Low;
			TRACE_SUSIE2("Peek(HPOSSTRTL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (HPOSSTRTH & 0xff):
			retval = mHPOSSTRT.Byte.High;
			TRACE_SUSIE2("Peek(HPOSSTRTH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (VPOSSTRTL & 0xff):
			retval = mVPOSSTRT.Byte.Low;
			TRACE_SUSIE2("Peek(VPOSSTRTL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (VPOSSTRTH & 0xff):
			retval = mVPOSSTRT.Byte.High;
			TRACE_SUSIE2("Peek(VPOSSTRTH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRHSIZL & 0xff):
			retval = mSPRHSIZ.Byte.Low;
			TRACE_SUSIE2("Peek(SPRHSIZL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRHSIZH & 0xff):
			retval = mSPRHSIZ.Byte.High;
			TRACE_SUSIE2("Peek(SPRHSIZH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRVSIZL & 0xff):
			retval = mSPRVSIZ.Byte.Low;
			TRACE_SUSIE2("Peek(SPRVSIZL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRVSIZH & 0xff):
			retval = mSPRVSIZ.Byte.High;
			TRACE_SUSIE2("Peek(SPRVSIZH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (STRETCHL & 0xff):
			retval = mSTRETCH.Byte.Low;
			TRACE_SUSIE2("Peek(STRETCHL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (STRETCHH & 0xff):
			retval = mSTRETCH.Byte.High;
			TRACE_SUSIE2("Peek(STRETCHH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (TILTL & 0xff):
			retval = mTILT.Byte.Low;
			TRACE_SUSIE2("Peek(TILTL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (TILTH & 0xff):
			retval = mTILT.Byte.High;
			TRACE_SUSIE2("Peek(TILTH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRDOFFL & 0xff):
			retval = mSPRDOFF.Byte.Low;
			TRACE_SUSIE2("Peek(SPRDOFFL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SPRDOFFH & 0xff):
			retval = mSPRDOFF.Byte.High;
			TRACE_SUSIE2("Peek(SPRDOFFH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (VSIZACUML & 0xff):
			retval = mVSIZACUM.Byte.Low;
			TRACE_SUSIE2("Peek(VSIZACUML)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (VSIZACUMH & 0xff):
			retval = mVSIZACUM.Byte.High;
			TRACE_SUSIE2("Peek(VSIZACUMH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SCBADRL & 0xff):
			retval = mSCBADR.Byte.Low;
			TRACE_SUSIE2("Peek(SCBADRL)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;
		case (SCBADRH & 0xff):
			retval = mSCBADR.Byte.High;
			TRACE_SUSIE2("Peek(SCBADRH)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

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

		case (SUZYHREV & 0xff):
			retval = 0x01;
			TRACE_SUSIE2("Peek(SUZYHREV)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (SPRSYS & 0xff):
			retval = 0x0000;
			//	retval += (mSPRSYS_Status) ? 0x0001 : 0x0000;
			retval += (mikey_0.suzieDoneTime) ? 0x0001 : 0x0000;
			retval += (mSPRSYS_StopOnCurrent) ? 0x0002 : 0x0000;
			retval += (mSPRSYS_UnsafeAccess) ? 0x0004 : 0x0000;
			retval += (mSPRSYS_LeftHand) ? 0x0008 : 0x0000;
			retval += (mSPRSYS_VStretch) ? 0x0010 : 0x0000;
			retval += (mSPRSYS_LastCarry) ? 0x0020 : 0x0000;
			retval += (mSPRSYS_Mathbit) ? 0x0040 : 0x0000;
			retval += (mSPRSYS_MathInProgress) ? 0x0080 : 0x0000;
			TRACE_SUSIE2("Peek(SPRSYS)=$%02x at PC=$%04x", retval, mSystem.mCpu->GetPC());
			return retval;

		case (JOYSTICK & 0xff):
			if (mSPRSYS_LeftHand) {
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
