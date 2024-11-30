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
#define RAM_PEEK(m)				(suzy_0.suzyRAM[(m)])
#define RAM_PEEKW(m)			(suzy_0.suzyRAM[(m)]+(suzy_0.suzyRAM[(m)+1]<<8))
#define RAM_POKE(m1,m2)			{suzy_0.suzyRAM[(m1)]=(m2);}

#define mTMPADR suzy_0.tmpAdr
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

#define mPenIndex suzy_0.penIndex

#define mSPRSYS_Mathbit suzy_0.sprSys_Mathbit

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

	// Reset ALL variables

	mSPRSYS_UnsafeAccess = 0;
	mSPRSYS_Busy = 0;
	mSPRSYS_LastCarry = 0;
	mSPRSYS_MathInProgress = 0;
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

		suzFetchSpriteData();
/*		mTMPADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
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
*/
		// Check if this is a skip sprite
		if (!(mSPRCTL1 & SkipSprite)) {
			// Initialise the collision depositary

// Although Tom Schenck says this is correct, it doesnt appear to be
//			if (mSPRCOLL < 0x10)
//			{
//				mCollision = RAM_PEEK((mSCBADR.Word+mCOLLOFF.Word) & 0xffff) & 0x0f;
//			}
/*			mCollision = 0;

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
			}*/

			TRACE_SUSIE1("PaintSprites() SPRHSIZ $%04x", mSPRHSIZ.Word);
			TRACE_SUSIE1("PaintSprites() SPRVSIZ $%04x", mSPRVSIZ.Word);
			TRACE_SUSIE1("PaintSprites() STRETCH $%04x", mSTRETCH.Word);
			TRACE_SUSIE1("PaintSprites() TILT    $%04x", mTILT.Word);

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

			TRACE_SUSIE1("PaintSprites() Superclip=%d", superclip);


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

			int hSign = (mSPRCTL1 & StartLeft) ? -1 : 1;
			int vSign = (mSPRCTL1 & StartUp) ? -1 : 1;

			// Use h/v flip to invert h/vSign
			if (mSPRCTL0 & Vflip) vSign = -vSign;
			if (mSPRCTL0 & Hflip) hSign = -hSign;
			int vQuadOff = vSign;
			int hQuadOff = hSign;

			int sprite_v = mVPOSSTRT.Word;
			int sprite_h = mHPOSSTRT.Word;

			// Loop for 4 quadrants
			for (int loop=0;loop<4;loop++)
			{
				TRACE_SUSIE1("PaintSprites() -------- Rendering Quadrant %03d --------", quadrant);

				bool render = FALSE;

				if (loop != 0) {
					// Set quadrand multipliers
					hSign = (quadrant == 0 || quadrant == 1) ? 1 : -1;
					vSign = (quadrant == 0 || quadrant == 3) ? 1 : -1;

// Preflip			TRACE_SUSIE2("PaintSprites() hSign=%d vSign=%d",hSign,vSign);

					// Use h/v flip to invert h/vSign
					if (mSPRCTL0 & Vflip) vSign = -vSign;
					if (mSPRCTL0 & Hflip) hSign = -hSign;
				}
				TRACE_SUSIE2("PaintSprites() Hflip=%d Vflip=%d", mSPRCTL0 & Hflip, mSPRCTL0 & Vflip);
				TRACE_SUSIE2("PaintSprites() Hsign=%d   Vsign=%d", hSign, vSign);
				TRACE_SUSIE2("PaintSprites() Hpos =%04x Vpos =%04x", sprite_h, sprite_v);
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
							if (suzLineRender(hSign, hQuadOff, voff)) {
								everonscreen = TRUE;
							}
							voff += vSign;

							// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
							if (mSPRCTL1 & 0x20) {
								mSPRHSIZ.Word += mSTRETCH.Word;
//								if (mSPRSYS & VStretch) mSPRVSIZ.Word += mSTRETCH.Word;
							}
							if ((mSPRCTL1 & ReloadDepth) == 0x30) {
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
			TRACE_SUSIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x", addr, data, mSystem.mCpu->GetPC());
			break;
	}
}

UBYTE CSusie::Peek(ULONG addr)
{
	UBYTE retval = 0;
	switch(addr & 0xff)
	{
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

// Cartridge reading ports
		case (RCART0 & 0xff):
			return mSystem.Peek_CARTB0();
		case (RCART1 & 0xff):
			return mSystem.Peek_CARTB1();

// Errors on illegal location accesses
		default:
			TRACE_SUSIE2("Peek(%04x) - Peek from illegal location at PC=$%04x", addr, mSystem.mCpu->GetPC());
	}

	return 0xff;
}
