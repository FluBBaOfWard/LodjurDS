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

#include <stdlib.h>
#include "system.h"
#include "susie.h"
#include "lynxdef.h"

//
// As the Susie sprite engine only ever sees system RAM
// wa can access this directly without the hassle of
// going through the system object, much faster
//
//#define RAM_PEEK(m)			(mSystem.Peek_RAM(m))
//#define RAM_POKE(m1,m2)		(mSystem.Poke_RAM(m1,m2))
//#define RAM_PEEKW(m)			(mSystem.PeekW_RAM(m))

#define RAM_PEEK(m)				(mRamPointer[m])
#define RAM_PEEKW(m)			(mRamPointer[m]+(mRamPointer[m+1]<<8))
#define RAM_POKE(m1,m2)			{mRamPointer[m1]=m2;}

ULONG cycles_used=0;

CSusie::CSusie(CSystem& parent)
	:mSystem(parent)
{
	Reset();
}

CSusie::~CSusie()
{
}

void CSusie::Reset(void)
{
	// Fetch pointer to system RAM, faster than object access
	// and seeing as Susie only ever sees RAM.

	mRamPointer = mSystem.GetRamPointer();

	// Reset ALL variables

	mTMPADR.Word = 0;
	mTILTACUM.Word = 0;
	mHOFF.Word = 0;
	mVOFF.Word = 0;
	mVIDBAS.Word = 0;
	mCOLLBAS.Word = 0;
	mVIDADR.Word = 0;
	mCOLLADR.Word = 0;
	mSCBNEXT.Word = 0;
	mSPRDLINE.Word = 0;
	mHPOSSTRT.Word = 0;
	mVPOSSTRT.Word = 0;
	mSPRHSIZ.Word = 0;
	mSPRVSIZ.Word = 0;
	mSTRETCH.Word = 0;
	mTILT.Word = 0;
	mSPRDOFF.Word = 0;
	mSPRVPOS.Word = 0;
	mCOLLOFF.Word = 0;
	mVSIZACUM.Word = 0;
	mHSIZACUM.Word = 0;
	mHSIZOFF.Word = 0;
	mVSIZOFF.Word = 0;
	mSCBADR.Word = 0;
	mPROCADR.Word = 0;

	mMATHABCD.Long = 0;
	mMATHEFGH.Long = 0;
	mMATHJKLM.Long = 0;
	mMATHNP.Long = 0;

	mMATH_SIGNED = FALSE;
	mMATH_OFLOW = FALSE;
	mMATH_ACCUM = FALSE;

	mSPRCTL0.Byte = 0;
	mSPRCTL1.Byte = 0;
	mSPRCOLL.Byte = 0;

	mSPRSYS.Byte = 0;

	mSUZYBUSEN = FALSE;

	mSPRINIT.Byte = 0;

	mSPRGO = FALSE;

	for(int loop=0;loop<16;loop++) mPenIndex[loop] = loop;

	mJOYSTICK.Byte = 0;
	mSWITCHES.Byte = 0;
}


void CSusie::DoMathMultiply(void)
{
	mMATH_OFLOW = FALSE;

	// Multiplies with out sign or accumulate take 44 ticks to complete.
	// Multiplies with sign and accumulate take 54 ticks to complete. 
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//

	if (mMATH_SIGNED)
	{
		SLONG result;
		result = (SLONG)((SWORD)mMATHABCD.Words.AB)*(SLONG)((SWORD)mMATHABCD.Words.CD);
		mMATHEFGH.Long = (ULONG)result;

		// Check overflow, if B31 has changed from 1->0 then its overflow time
		if (mMATH_ACCUM)
		{
			if ((mMATHJKLM.Long & 0x80000000)
			         != (result & 0x80000000/*0*/)) mMATH_OFLOW = TRUE; else mMATH_OFLOW = FALSE;
			mMATHJKLM.Long += result;
		}
	}
	else
	{
		ULONG result;
		result = (ULONG)mMATHABCD.Words.AB*(ULONG)mMATHABCD.Words.CD;
		mMATHEFGH.Long = result;

		// Check overflow, if B31 has changed from 1->0 then its overflow time
		if (mMATH_ACCUM)
		{
			if ((mMATHJKLM.Long & 0x80000000) &
			          !(result & 0x80000000/*0*/)) mMATH_OFLOW = TRUE; else mMATH_OFLOW = FALSE;
			mMATHJKLM.Long += result;
		}
	}
}

void CSusie::DoMathDivide(void)
{
	mMATH_OFLOW = FALSE;

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

	if (mMATHNP.Long)
	{
		if (mMATH_SIGNED)
		{
			mMATHABCD.Long = (SLONG)mMATHEFGH.Long/(SLONG)((SWORD)mMATHNP.Long);
			mMATHJKLM.Long = (SLONG)mMATHEFGH.Long%(SLONG)((SWORD)mMATHNP.Long);
		}
		else
		{
			mMATHABCD.Long = mMATHEFGH.Long/mMATHNP.Long;
			mMATHJKLM.Long = mMATHEFGH.Long%mMATHNP.Long;
		}
	}
	else
	{
		mMATHABCD.Long = 0xffffffff;
		mMATHJKLM.Long = 0;
		mMATH_OFLOW = TRUE;
	}
}


ULONG CSusie::PaintSprites(void)
{
	if (!mSUZYBUSEN || !mSPRGO) return 0;

	cycles_used = 0;

	do
	{
		// Step 1 load up the SCB params into Susie

		if (!mSCBNEXT.Word)
		{
			mSPRSYS.Read.Status = 0;	// Engine has finished
			mSPRGO = FALSE;
			break;
		}
		else
		{
			mSPRSYS.Read.Status = 1;
		}

		mTMPADR.Word = mSCBNEXT.Word;	// Copy SCB pointer
		mSCBADR.Word = mSCBNEXT.Word;	// Copy SCB pointer

		mSPRCTL0.Byte = RAM_PEEK(mTMPADR.Word);	// Fetch control 0
		mTMPADR.Word += 1;

		mSPRCTL1.Byte = RAM_PEEK(mTMPADR.Word);	// Fetch control 1
		mTMPADR.Word += 1;

		mSPRCOLL.Byte = RAM_PEEK(mTMPADR.Word);	// Collision num
		mTMPADR.Word += 1;

		mSCBNEXT.Word = RAM_PEEKW(mTMPADR.Word);	// Next SCB
		mTMPADR.Word += 2;

		cycles_used += 5*AVE_RDWR_CYC;

		// Initialise the collision depositary

		mCollision = 0;

		// Check if this is a skip sprite

		if (!mSPRCTL1.Bits.SkipSprite)
		{

			mSPRDLINE.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite pack data
			mTMPADR.Word += 2;

			mHPOSSTRT.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite horizontal start position
			mTMPADR.Word += 2;

			mVPOSSTRT.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite vertical start position
			mTMPADR.Word += 2;

			cycles_used += 6*AVE_RDWR_CYC;

			BOOL enable_sizing = FALSE;
			BOOL enable_stretch = FALSE;
			BOOL enable_tilt = FALSE;
		
			// Optional section defined by reload type in Control 1

			switch (mSPRCTL1.Bits.ReloadDepth)
			{
				case 1:
					enable_sizing = TRUE;

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					cycles_used += 4*AVE_RDWR_CYC;
					break;

				case 2:
					enable_sizing = TRUE;
					enable_stretch = TRUE;

					mSPRHSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word += 2;

					mSPRVSIZ.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite Vertical size
					mTMPADR.Word += 2;

					mSTRETCH.Word = RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
					mTMPADR.Word += 2;

					cycles_used += 6*AVE_RDWR_CYC;
					break;

				case 3:
					enable_sizing = TRUE;
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

					cycles_used += 8*AVE_RDWR_CYC;
					break;

				default:
					break;
			}

			// Optional Palette reload

			if (!mSPRCTL1.Bits.ReloadPalette)
			{
				for (int loop=0;loop<8;loop++)
				{
					UBYTE data = RAM_PEEK(mTMPADR.Word++);
					mPenIndex[loop*2] = (data>>4) & 0x0f;
					mPenIndex[(loop*2)+1] = data & 0x0f;
				}
				// Increment cycle count for the reads
				cycles_used += 8*AVE_RDWR_CYC;
			}

			// Now we can start painting
		
			// Quadrant drawing order is: SE,NE,NW,SW
			// start quadrant is given by sprite_control1:0 & 1

			// Setup screen start end variables

			UWORD screen_h_start = mHOFF.Word;
			UWORD screen_h_end = mHOFF.Word+LYNX_SCREEN_WIDTH;
			UWORD screen_v_start = mVOFF.Word;
			UWORD screen_v_end = mVOFF.Word+LYNX_SCREEN_HEIGHT;

			UWORD world_h_mid = screen_h_start+0x8000+(LYNX_SCREEN_WIDTH/2);
			UWORD world_v_mid = screen_v_start+0x8000+(LYNX_SCREEN_HEIGHT/2);

			BOOL superclip = FALSE;
			UWORD quadrant = 0;
			SWORD hsign,vsign;

			if (mSPRCTL1.Bits.StartLeft)
			{
				if (mSPRCTL1.Bits.StartUp) quadrant=2; else quadrant=3;
			}
			else
			{
				if (mSPRCTL1.Bits.StartUp) quadrant=1; else quadrant=0;
			}

			// Check ref is inside screen area

			if (mHPOSSTRT.Word<screen_h_start || mHPOSSTRT.Word>=screen_h_end ||
				mVPOSSTRT.Word<screen_v_start || mVPOSSTRT.Word>=screen_v_end) superclip = TRUE;

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
				UWORD sprite_v = mVPOSSTRT.Word;
				UWORD sprite_h = mHPOSSTRT.Word;

				BOOL render = FALSE;

				// Set quadrand multipliers
				hsign = (quadrant==0 || quadrant==1)?1:-1;
				vsign = (quadrant==0 || quadrant==3)?1:-1;

				// Use h/v flip to invert v/hsign

				if (mSPRCTL0.Bits.Vflip) vsign = -vsign;
				if (mSPRCTL0.Bits.Hflip) hsign = -hsign;

				// Two different rendering algorithms used, on-screen & superclip
				// when on screen we draw in x until off screen then skip to next
				// line, BUT on superclip we draw all the way to the end of any
				// given line checking each pixel is on screen.

				if (superclip)
				{
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
					UWORD	modquad = quadrant;
					static UWORD vquadflip[4] = {1,0,3,2};
					static UWORD hquadflip[4] = {3,2,1,0};

					if (mSPRCTL0.Bits.Vflip) modquad = vquadflip[modquad];
					if (mSPRCTL0.Bits.Hflip) modquad = hquadflip[modquad];

					if (enable_tilt && mTILT.Word&0x8000) modquad = hquadflip[modquad];

					switch (modquad)
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

				if (render)
				{
					// Enter the line rendering loop

					static UWORD pixel_v_pos;
					static UWORD pixel_h_pos;
					static UWORD pixel_height;
					static UWORD pixel_width;
					static ULONG pixel;
					static ULONG hoff,voff;
					static UWORD hloop,vloop;
					static BOOL  onscreen;

					// Set the vertical position
					pixel_v_pos = mVPOSSTRT.Word;
					// This is causing streaks in some sprites
//					if (vsign == -1) pixel_v_pos--;

					// Zero the stretch,tilt & acum values
		
					mTILTACUM.Word=0;

//					if (vsign == 1) mVSIZACUM.Word = mVSIZOFF.Word; else mVSIZACUM.Word = 0;
					mVSIZACUM.Word=0;
					
					for (;;)
					{
						// Vertical scaling is done here

						mVSIZACUM.Word += mSPRVSIZ.Word;
						pixel_height = mVSIZACUM.Byte.High;
						mVSIZACUM.Byte.High = 0;

						// Update the next data line pointer and initialise our line
	
						mSPRDOFF.Word = (UWORD)LineInit(0);

						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE

						if (mSPRDOFF.Word == 1)		// End of quad
						{
							mSPRDLINE.Word += mSPRDOFF.Word;
							break;
						}

						if (mSPRDOFF.Word == 0)		// End of sprite
						{
							loop = 4;	// Halt the quad loop
							break;
						}

						// Draw one horizontal line of the sprite 

						for (vloop=0;vloop<pixel_height;vloop++)
						{
							//
							// Early bailout if the sprite has moved off screen, terminate quad
							//
							if (vsign == 1 && (SWORD)pixel_v_pos >= (SWORD)screen_v_end)
							{
								break;
							}
							if (vsign == -1 && (SWORD)pixel_v_pos < (SWORD)screen_v_start)
							{
								break;
							}

							//
							// Only allow the draw to take place if the line is visible
							//
							if ((SWORD)pixel_v_pos >= (SWORD)screen_v_start && (SWORD)pixel_v_pos < (SWORD)screen_v_end)
							{
								// Work out the horizontal pixel start position, start + tilt

								pixel_h_pos = mHPOSSTRT.Word+((SWORD)mTILTACUM.Word>>8);

								// This is causing streaks in some sprites
//								if (hsign == -1) pixel_h_pos--;

					 			// Zero the horizontal scaling accumulator
//								if (hsign == 1) mHSIZACUM.Word = mHSIZOFF.Word; else mHSIZACUM.Word = 0;
								mHSIZACUM.Word = 0;

								// Update the next data line pointer and initialise our line
	
								voff = (UWORD)(pixel_v_pos-screen_v_start);
								LineInit(voff);

								// Now render an individual destination line

								while ((pixel = LineGetPixel()) != LINE_END)
								{
									// This is allowed to update every pixel
									mHSIZACUM.Word += mSPRHSIZ.Word;
									pixel_width = mHSIZACUM.Byte.High;
									mHSIZACUM.Byte.High = 0;

									for (hloop=0;hloop<pixel_width;hloop++)
									{
										onscreen = FALSE;

										// Check pixel is on screen

										if ((SWORD)pixel_h_pos >= (SWORD)screen_h_start && (SWORD)pixel_h_pos < (SWORD)screen_h_end)
										{
											hoff = (UWORD)(pixel_h_pos-screen_h_start);
											ProcessPixel(hoff,pixel);

											onscreen = TRUE;
										}
										else
										{
											// On to offscreen transition causes breakout
											if(onscreen) break;
										}
										pixel_h_pos += hsign;
									}
								}
							}
							pixel_v_pos += vsign;

							// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM

							if (enable_stretch)
							{
								mSPRHSIZ.Word += mSTRETCH.Word;
								if (mSPRSYS.Write.Vstretch) mSPRVSIZ.Word += mSTRETCH.Word;
							}

							if (enable_tilt)
							{
								// Manipulate the tilt stuff
								mTILTACUM.Word += mTILT.Word;
							}
						}

						// Update the line start for our next run thru the loop
						mSPRDLINE.Word += mSPRDOFF.Word;
					}
				}
				else
				{
					// Skip thru data to next quad
					for (;;)
					{
						// Read the start of line offset

						mSPRDOFF.Word = (UWORD)LineInit(0);

						// We dont want to process data so mSPRDLINE is useless to us
						mSPRDLINE.Word += mSPRDOFF.Word;

						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE

						if (mSPRDOFF.Word == 1) break;	// End of quad
						if (mSPRDOFF.Word == 0)		// End of sprite
						{
							loop = 4;	// Halt the quad loop
							break;
						}

					}
				}

				// Increment quadrant and mask to 2 bit value (0-3)
				quadrant++;
				quadrant &= 0x03;
			}

			// Write the collision depositary if required

			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide && mSPRCTL0.Bits.Type != sprite_background_noncollide && mSPRCTL0.Bits.Type != sprite_noncollide && mSPRCTL0.Bits.Type != sprite_background_shadow)
			{
				UWORD coldep = mSCBADR.Word+mCOLLOFF.Word;
				RAM_POKE(coldep,(UBYTE)mCollision);
			}

		}

		// Check if we abort after 1st sprite is complete

//		if (mSPRSYS.Read.StopOnCurrent)
//		{
//			mSPRSYS.Read.Status = 0;	// Engine has finished
//			mSPRGO = FALSE;
//			break;
//		}
	}
	while(1);

	return cycles_used;
}


inline void CSusie::ProcessPixel(ULONG hoff,ULONG pixel)
{
	switch (mSPRCTL0.Bits.Type)
	{
		case sprite_background_shadow:
			WritePixel(hoff,pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide)
			{
				WriteCollision(hoff,mSPRCOLL.Bits.Number);
			}
			break;
		case sprite_background_noncollide:
			WritePixel(hoff,pixel);
			break;
		case sprite_noncollide:
			if (pixel != 0x00) WritePixel(hoff,pixel);
			break;
		case sprite_boundary:
			if (pixel != 0x00 && pixel != 0x0f) WritePixel(hoff,pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide)
			{
				ULONG collision=ReadCollision(hoff);
				if (collision > mCollision)
				{
					mCollision = collision;
				}
				if (mSPRCOLL.Bits.Number > collision)
				{
					WriteCollision(hoff,mSPRCOLL.Bits.Number);
				}
			}
			break;
		case sprite_normal:
			if (pixel != 0x00) WritePixel(hoff,pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide)
			{
				ULONG collision = ReadCollision(hoff);
				if (collision > mCollision)
				{
					mCollision = collision;
				}
				if (mSPRCOLL.Bits.Number > collision)
				{
					WriteCollision(hoff,mSPRCOLL.Bits.Number);
				}
			}
			break;
		case sprite_boundary_shadow:
			if (pixel != 0x00 && pixel != 0x0f) WritePixel(hoff,pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide && pixel != 0x0e)
			{
				ULONG collision=ReadCollision(hoff);
				if (collision > mCollision)
				{
					mCollision = collision;
				}
				if (mSPRCOLL.Bits.Number > collision)
				{
					WriteCollision(hoff,mSPRCOLL.Bits.Number);
				}
			}
			break;
		case sprite_shadow:
			if (pixel != 0x00) WritePixel(hoff,pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide && pixel != 0x0e)
			{
				ULONG collision = ReadCollision(hoff);
				if (collision > mCollision)
				{
					mCollision = collision;
				}
				if (mSPRCOLL.Bits.Number > collision)
				{
					WriteCollision(hoff,mSPRCOLL.Bits.Number);
				}
			}
			break;
		case sprite_xor_shadow:
			WritePixel(hoff,ReadPixel(hoff)^pixel);
			if (!mSPRCOLL.Bits.Collide && !mSPRSYS.Write.NoCollide && pixel != 0x0e)
			{
				ULONG collision = ReadCollision(hoff);
				if (collision > mCollision)
				{
					mCollision = collision;
				}
				if (mSPRCOLL.Bits.Number > collision)
				{
					WriteCollision(hoff,mSPRCOLL.Bits.Number);
				}
			}
			break;
		default:
			break;
	}
}

inline void CSusie::WritePixel(ULONG hoff,ULONG pixel)
{
	ULONG scr_addr = mLineBaseAddress+(hoff/2);
	
	UBYTE dest = RAM_PEEK(scr_addr);
	if (!(hoff & 0x01))
	{
		// Upper nibble screen write
		dest &= 0x0f;
		dest |= pixel<<4;
	}
	else
	{
		// Lower nibble screen write
		dest &= 0xf0;
		dest |= pixel;
	}
	RAM_POKE(scr_addr,dest);

	// Increment cycle count for the read/modify/write
	cycles_used += 2*AVE_RDWR_CYC;
}

inline ULONG CSusie::ReadPixel(ULONG hoff)
{
	ULONG scr_addr = mLineBaseAddress+(hoff/2);
	
	ULONG data = RAM_PEEK(scr_addr);
	if (!(hoff & 0x01))
	{
		// Upper nibble read
		data >>= 4;
	}
	else
	{
		// Lower nibble read
		data &= 0x0f;
	}

	// Increment cycle count for the read/modify/write
	cycles_used += AVE_RDWR_CYC;

	return data;
}

inline void CSusie::WriteCollision(ULONG hoff,ULONG pixel)
{
	ULONG col_addr = mLineCollisionAddress+(hoff/2);
	
	UBYTE dest = RAM_PEEK(col_addr);
	if (!(hoff & 0x01))
	{
		// Upper nibble screen write
		dest &= 0x0f;
		dest |= pixel<<4;
	}
	else
	{
		// Lower nibble screen write
		dest &= 0xf0;
		dest |= pixel;
	}
	RAM_POKE(col_addr,dest);

	// Increment cycle count for the read/modify/write
	cycles_used += 2*AVE_RDWR_CYC;
}

inline ULONG CSusie::ReadCollision(ULONG hoff)
{
	ULONG col_addr = mLineCollisionAddress+(hoff/2);
	
	ULONG data = RAM_PEEK(col_addr);
	if (!(hoff & 0x01))
	{
		// Upper nibble read
		data >>= 4;
	}
	else
	{
		// Lower nibble read
		data &= 0x0f;
	}

	// Increment cycle count for the read/modify/write
	cycles_used += AVE_RDWR_CYC;

	return data;
}


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

	mLinePacketBitsLeft = (offset-1)*8;

	// Literals are a special case and get their count set on a line basis
	
	if (mSPRCTL1.Bits.Literal)
	{
		mLineType = line_abs_literal;
		mLineRepeatCount = ((offset-1)*8)/(mSPRCTL0.Bits.PixelBits+1);
		// Why is this necessary, is this compensating for the 1,1 offset bug
//		mLineRepeatCount--;
	}

	// Set the line base address for use in the calls to pixel painting

	if (voff > 101)
	{
//		::MessageBox(NULL,"Out of bounds (voff)","Error", MB_OK | MB_ICONERROR);
		printf("Out of bounds (voff)\n");
		voff = 0;
	}

	mLineBaseAddress = mVIDBAS.Word+(voff*(LYNX_SCREEN_WIDTH/2));
	mLineCollisionAddress = mCOLLBAS.Word+(voff*(LYNX_SCREEN_WIDTH/2));

	// Return the offset to the next line

	return offset;
}

inline ULONG CSusie::LineGetPixel()
{
	ULONG nofbits;

	if (!mLineRepeatCount)
	{
		// Normal sprites fetch their counts on a packet basis

		if (mLineType != line_abs_literal)
		{
			ULONG literal = LineGetBits(1);
			if (literal) mLineType = line_literal; else mLineType = line_packed;
		}

		// Pixel store is empty what should we do

		switch (mLineType)
		{
			case line_abs_literal:
				// This means end of line for us
				mLinePixel = LINE_END;
				return mLinePixel;
				break;
			case line_literal:
				mLineRepeatCount = LineGetBits(4);
				mLineRepeatCount++;
				break;
			case line_packed:

				//
				// From reading in between the lines only a packed line with
				// a zero size i.e 0b00000 as a header is allowable as a packet end
				//
				mLineRepeatCount = LineGetBits(4);
				if (!mLineRepeatCount)
				{
					mLinePixel = LINE_END;
				}
				else
				{
					nofbits = mSPRCTL0.Bits.PixelBits+1;
					mLinePixel = mPenIndex[(UBYTE)LineGetBits(nofbits)&0x0f];
				}
				return mLinePixel;
				break;
			default:
				return 0;
		}
	}

	if (mLinePixel != LINE_END)
	{
		mLineRepeatCount--;

		switch(mLineType)
		{
			case line_packed:
				break;
			case line_abs_literal:
				nofbits = mSPRCTL0.Bits.PixelBits+1;
				mLinePixel = mPenIndex[(UBYTE)LineGetBits(nofbits) & 0x0f];
				// Check the special case of a zero in the last pixel
				if (!mLineRepeatCount && !mLinePixel) mLinePixel = LINE_END;
				break;
			case line_literal:
				nofbits = mSPRCTL0.Bits.PixelBits+1;
				mLinePixel = mPenIndex[(UBYTE)LineGetBits(nofbits) & 0x0f];
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
	// if(bits>32) return 0;

	// Only return data IF there is enought bits left in the packet

	if (mLinePacketBitsLeft<bits) return 0;

	// Make sure shift reg has enough bits to fulfil the request

	if (mLineShiftRegCount<bits)
	{
		// This assumes data comes into LSB and out of MSB
		mLineShiftReg <<= 24;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++)<<16;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++)<<8;
		mLineShiftReg |= RAM_PEEK(mTMPADR.Word++);

		mLineShiftRegCount += 24;

		// Increment cycle count for the read
		cycles_used += 3*AVE_RDWR_CYC;
	}

	// Extract the return value
	retval = mLineShiftReg>>(mLineShiftRegCount-bits);
	retval &= (1<<bits)-1;

	// Update internal vars;
	mLineShiftRegCount -= bits;
	mLinePacketBitsLeft -= bits;

	return retval;
}


void CSusie::Poke(ULONG addr,UBYTE data)
{
	switch (addr & 0xff)
	{
		case (TMPADRL & 0xff):
			mTMPADR.Byte.Low = data;
			mTMPADR.Byte.High = 0;
			break;
		case (TMPADRH & 0xff):
			mTMPADR.Byte.High = data;
			break;
		case (TILTACUML & 0xff):
			mTILTACUM.Byte.Low = data;
			mTILTACUM.Byte.High = 0;
			break;
		case (TILTACUMH & 0xff):
			mTILTACUM.Byte.High = data;
			break;
		case (HOFFL & 0xff):
			mHOFF.Byte.Low = data;
			mHOFF.Byte.High = 0;
			break;
		case (HOFFH & 0xff):
			mHOFF.Byte.High = data;
			break;
		case (VOFFL & 0xff):
			mVOFF.Byte.Low = data;
			mVOFF.Byte.High = 0;
			break;
		case (VOFFH & 0xff):
			mVOFF.Byte.High = data;
			break;
		case (VIDBASL & 0xff):
			mVIDBAS.Byte.Low = data;
			mVIDBAS.Byte.High = 0;
			break;
		case (VIDBASH & 0xff):
			mVIDBAS.Byte.High = data;
			break;
		case (COLLBASL & 0xff):
			mCOLLBAS.Byte.Low = data;
			mCOLLBAS.Byte.High = 0;
			break;
		case (COLLBASH & 0xff):
			mCOLLBAS.Byte.High = data;
			break;
		case (VIDADRL & 0xff):
			mVIDADR.Byte.Low = data;
			mVIDADR.Byte.High = 0;
			break;
		case (VIDADRH & 0xff):
			mVIDADR.Byte.High = data;
			break;
		case (COLLADRL & 0xff):
			mCOLLADR.Byte.Low = data;
			mCOLLADR.Byte.High = 0;
			break;
		case (COLLADRH & 0xff):
			mCOLLADR.Byte.High = data;
			break;
		case (SCBNEXTL & 0xff):
			mSCBNEXT.Byte.Low = data;
			mSCBNEXT.Byte.High = 0;
			break;
		case (SCBNEXTH & 0xff):
			mSCBNEXT.Byte.High = data;
			break;
		case (SPRDLINEL & 0xff):
			mSPRDLINE.Byte.Low = data;
			mSPRDLINE.Byte.High = 0;
			break;
		case (SPRDLINEH & 0xff):
			mSPRDLINE.Byte.High = data;
			break;
		case (HPOSSTRTL & 0xff):
			mHPOSSTRT.Byte.Low = data;
			mHPOSSTRT.Byte.High = 0;
			break;
		case (HPOSSTRTH & 0xff):
			mHPOSSTRT.Byte.High = data;
			break;
		case (VPOSSTRTL & 0xff):
			mVPOSSTRT.Byte.Low = data;
			mVPOSSTRT.Byte.High = 0;
			break;
		case (VPOSSTRTH&0xff):
			mVPOSSTRT.Byte.High=data;
			break;
		case (SPRHSIZL&0xff):
			mSPRHSIZ.Byte.Low=data;
			mSPRHSIZ.Byte.High=0;
			break;
		case (SPRHSIZH&0xff):
			mSPRHSIZ.Byte.High=data;
			break;
		case (SPRVSIZL&0xff):
			mSPRVSIZ.Byte.Low=data;
			mSPRVSIZ.Byte.High=0;
			break;
		case (SPRVSIZH&0xff):
			mSPRVSIZ.Byte.High=data;
			break;
		case (STRETCHL&0xff):
			mSTRETCH.Byte.Low=data;
			mSTRETCH.Byte.High=0;
			break;
		case (STRETCHH&0xff):
			mSTRETCH.Byte.High=data;
			break;
		case (TILTL&0xff):
			mTILT.Byte.Low=data;
			mTILT.Byte.High=0;
			break;
		case (TILTH&0xff):
			mTILT.Byte.High=data;
			break;
		case (SPRDOFFL&0xff):
			mSPRDOFF.Byte.Low=data;
			mSPRDOFF.Byte.High=0;
			break;
		case (SPRDOFFH&0xff):
			mSPRDOFF.Byte.High=data;
			break;
		case (SPRVPOSL&0xff):
			mSPRVPOS.Byte.Low=data;
			mSPRVPOS.Byte.High=0;
			break;
		case (SPRVPOSH&0xff):
			mSPRVPOS.Byte.High=data;
			break;
		case (COLLOFFL&0xff):
			mCOLLOFF.Byte.Low=data;
			mCOLLOFF.Byte.High=0;
			break;
		case (COLLOFFH&0xff):
			mCOLLOFF.Byte.High=data;
			break;
		case (VSIZACUML&0xff):
			mVSIZACUM.Byte.Low=data;
			mVSIZACUM.Byte.High=0;
			break;
		case (VSIZACUMH&0xff):
			mVSIZACUM.Byte.High=data;
			break;
		case (HSIZOFFL&0xff):
			mHSIZOFF.Byte.Low=data;
			mHSIZOFF.Byte.High=0;
			break;
		case (HSIZOFFH&0xff):
			mHSIZOFF.Byte.High=data;
			break;
		case (VSIZOFFL&0xff):
			mVSIZOFF.Byte.Low=data;
			mVSIZOFF.Byte.High=0;
			break;
		case (VSIZOFFH&0xff):
			mVSIZOFF.Byte.High=data;
			break;
		case (SCBADRL&0xff):
			mSCBADR.Byte.Low=data;
			mSCBADR.Byte.High=0;
			break;
		case (SCBADRH&0xff):
			mSCBADR.Byte.High=data;
			break;
		case (PROCADRL&0xff):
			mPROCADR.Byte.Low=data;
			mPROCADR.Byte.High=0;
			break;
		case (PROCADRH&0xff):
			mPROCADR.Byte.High=data;
			break;

		case (MATHD&0xff):
			mMATHABCD.Bytes.D=data;
			mMATHABCD.Bytes.C=0;
			break;
		case (MATHC&0xff):
			mMATHABCD.Bytes.C=data;
			break;
		case (MATHB&0xff):
			mMATHABCD.Bytes.B=data;
			mMATHABCD.Bytes.A=0;
			break;
		case (MATHA&0xff):
			mMATHABCD.Bytes.A=data;
			DoMathMultiply();
			break;

		case (MATHP&0xff):
			mMATHNP.Bytes.P=data;
			mMATHNP.Bytes.N=0;
			break;
		case (MATHN&0xff):
			mMATHNP.Bytes.N=data;
			break;

		case (MATHH&0xff):
			mMATHEFGH.Bytes.H=data;
			mMATHEFGH.Bytes.G=0;
			break;
		case (MATHG&0xff):
			mMATHEFGH.Bytes.G=data;
			break;
		case (MATHF&0xff):
			mMATHEFGH.Bytes.F=data;
			mMATHEFGH.Bytes.E=0;
			break;
		case (MATHE&0xff):
			mMATHEFGH.Bytes.E=data;
			DoMathDivide();
			break;

		case (MATHM&0xff):
			mMATHJKLM.Bytes.M=data;
			mMATHJKLM.Bytes.L=0;
			mMATH_OFLOW=FALSE;
			break;
		case (MATHL&0xff):
			mMATHJKLM.Bytes.L=data;
			break;
		case (MATHK&0xff):
			mMATHJKLM.Bytes.K=data;
			mMATHJKLM.Bytes.J=0;
			break;
		case (MATHJ&0xff):
			mMATHJKLM.Bytes.J=data;
			break;

		case (SPRCTL0&0xff):
			mSPRCTL0.Byte=data;
			break;
		case (SPRCTL1&0xff):
			mSPRCTL1.Byte=data;
			break;
		case (SPRCOLL&0xff):
			mSPRCOLL.Byte=data;
			break;
		case (SPRINIT&0xff):
			mSPRINIT.Byte=data;
			break;
		case (SUZYBUSEN&0xff):
			mSUZYBUSEN=data&0x01;
			break;
		case (SPRGO & 0xff):
			mSPRGO = data & 0x01;
			break;
		case (SPRSYS & 0xff):
			mSPRSYS.Byte &= 0x01;			// Preserve B0
			mSPRSYS.Byte |= (data & 0xfa);	// Zero the unused and unsafe bits
			mMATH_SIGNED = mSPRSYS.Write.SignedMath;
			mMATH_ACCUM = mSPRSYS.Write.Accumulate;
			break;

// Cartridge writing ports

		case (RCART0 & 0xff):
			mSystem.Poke_CARTB0(data);
			break;
		case (RCART1 & 0xff):
			mSystem.Poke_CARTB1(data);
			break;
			
// These are not so important, so lets ignore them for the moment

		case (LEDS & 0xff):
		case (PPORTSTAT & 0xff):
		case (PPORTDATA & 0xff):
		case (HOWIE & 0xff):
			break;

// Errors on read only register accesses

		case (SUZYHREV & 0xff):
		case (JOYSTICK & 0xff):
		case (SWITCHES & 0xff):
//			_RPT3(_CRT_WARN, "CSusie::Poke(%04x,%02x) - Poke to read only register location at time %d\n",addr,data,cycles_used);
			printf("CSusie::Poke(%04x,%02x) - Poke to read only register location at time %d\n", addr, data, cycles_used);
			break;

// Errors on illegal location accesses

		default:
//			_RPT3(_CRT_WARN, "CSusie::Poke(%04x,%02x) - Poke to illegal location at time %d\n",addr,data,cycles_used);
			printf("CSusie::Poke(%04x,%02x) - Poke to illegal location at time %d\n",addr,data,cycles_used);
			break;
	}
}

UBYTE CSusie::Peek(ULONG addr)
{
	switch(addr & 0xff)
	{
		case (TMPADRL & 0xff):
			return mTMPADR.Byte.Low;
			break;
		case (TMPADRH&0xff):
			return mTMPADR.Byte.High;
			break;
		case (TILTACUML&0xff):
			return mTILTACUM.Byte.Low;
			break;
		case (TILTACUMH&0xff):
			return mTILTACUM.Byte.High;
			break;
		case (HOFFL&0xff):
			return mHOFF.Byte.Low;
			break;
		case (HOFFH&0xff):
			return mHOFF.Byte.High;
			break;
		case (VOFFL&0xff):
			return mVOFF.Byte.Low;
			break;
		case (VOFFH&0xff):
			return mVOFF.Byte.High;
			break;
		case (VIDBASL&0xff):
			return mVIDBAS.Byte.Low;
			break;
		case (VIDBASH&0xff):
			return mVIDBAS.Byte.High;
			break;
		case (COLLBASL&0xff):
			return mCOLLBAS.Byte.Low;
			break;
		case (COLLBASH&0xff):
			return mCOLLBAS.Byte.High;
			break;
		case (VIDADRL&0xff):
			return mVIDADR.Byte.Low;
			break;
		case (VIDADRH&0xff):
			return mVIDADR.Byte.High;
			break;
		case (COLLADRL&0xff):
			return mCOLLADR.Byte.Low;
			break;
		case (COLLADRH&0xff):
			return mCOLLADR.Byte.High;
			break;
		case (SCBNEXTL&0xff):
			return mSCBNEXT.Byte.Low;
			break;
		case (SCBNEXTH&0xff):
			return mSCBNEXT.Byte.High;
			break;
		case (SPRDLINEL&0xff):
			return mSPRDLINE.Byte.Low;
			break;
		case (SPRDLINEH&0xff):
			return mSPRDLINE.Byte.High;
			break;
		case (HPOSSTRTL&0xff):
			return mHPOSSTRT.Byte.Low;
			break;
		case (HPOSSTRTH&0xff):
			return mHPOSSTRT.Byte.High;
			break;
		case (VPOSSTRTL&0xff):
			return mVPOSSTRT.Byte.Low;
			break;
		case (VPOSSTRTH&0xff):
			return mVPOSSTRT.Byte.High;
			break;
		case (SPRHSIZL&0xff):
			return mSPRHSIZ.Byte.Low;
			break;
		case (SPRHSIZH&0xff):
			return mSPRHSIZ.Byte.High;
			break;
		case (SPRVSIZL&0xff):
			return mSPRVSIZ.Byte.Low;
			break;
		case (SPRVSIZH&0xff):
			return mSPRVSIZ.Byte.High;
			break;
		case (STRETCHL&0xff):
			return mSTRETCH.Byte.Low;
			break;
		case (STRETCHH&0xff):
			return mSTRETCH.Byte.High;
			break;
		case (TILTL&0xff):
			return mTILT.Byte.Low;
			break;
		case (TILTH&0xff):
			return mTILT.Byte.High;
			break;
		case (SPRDOFFL&0xff):
			return mSPRDOFF.Byte.Low;
			break;
		case (SPRDOFFH&0xff):
			return mSPRDOFF.Byte.High;
			break;
		case (SPRVPOSL&0xff):
			return mSPRVPOS.Byte.Low;
			break;
		case (SPRVPOSH&0xff):
			return mSPRVPOS.Byte.High;
			break;
		case (COLLOFFL&0xff):
			return mCOLLOFF.Byte.Low;
			break;
		case (COLLOFFH&0xff):
			return mCOLLOFF.Byte.High;
			break;
		case (VSIZACUML&0xff):
			return mVSIZACUM.Byte.Low;
			break;
		case (VSIZACUMH&0xff):
			return mVSIZACUM.Byte.High;
			break;
		case (HSIZOFFL&0xff):
			return mHSIZOFF.Byte.Low;
			break;
		case (HSIZOFFH&0xff):
			return mHSIZOFF.Byte.High;
			break;
		case (VSIZOFFL&0xff):
			return mVSIZOFF.Byte.Low;
			break;
		case (VSIZOFFH&0xff):
			return mVSIZOFF.Byte.High;
			break;
		case (SCBADRL&0xff):
			return mSCBADR.Byte.Low;
			break;
		case (SCBADRH&0xff):
			return mSCBADR.Byte.High;
			break;
		case (PROCADRL&0xff):
			return mPROCADR.Byte.Low;
			break;
		case (PROCADRH&0xff):
			return mPROCADR.Byte.High;
			break;

		case (MATHD&0xff):
			return mMATHABCD.Bytes.D;
			break;
		case (MATHC&0xff):
			return mMATHABCD.Bytes.C;
			break;
		case (MATHB&0xff):
			return mMATHABCD.Bytes.B;
			break;
		case (MATHA&0xff):
			return mMATHABCD.Bytes.A;
			break;

		case (MATHP&0xff):
			return mMATHNP.Bytes.P;
			break;
		case (MATHN&0xff):
			return mMATHNP.Bytes.N;
			break;

		case (MATHH&0xff):
			return mMATHEFGH.Bytes.H;
			break;
		case (MATHG&0xff):
			return mMATHEFGH.Bytes.G;
			break;
		case (MATHF&0xff):
			return mMATHEFGH.Bytes.F;
			break;
		case (MATHE&0xff):
			return mMATHEFGH.Bytes.E;
			break;

		case (MATHM&0xff):
			return mMATHJKLM.Bytes.M;
			break;
		case (MATHL&0xff):
			return mMATHJKLM.Bytes.L;
			break;
		case (MATHK&0xff):
			return mMATHJKLM.Bytes.K;
			break;
		case (MATHJ&0xff):
			return mMATHJKLM.Bytes.J;
			break;

		case (SUZYHREV&0xff):
			return 0x01;

		case (SPRSYS & 0xff):
			mSPRSYS.Read.Mathbit = mMATH_OFLOW;
			mSPRSYS.Read.MathInProgress = 0;
			return mSPRSYS.Byte;

		case (JOYSTICK & 0xff):
			if (mSPRSYS.Write.LeftHand)
			{
				return mJOYSTICK.Byte;
			}
			else
			{
				TJOYSTICK Modified = mJOYSTICK;
				Modified.Bits.Left = mJOYSTICK.Bits.Right;
				Modified.Bits.Right = mJOYSTICK.Bits.Left;
				Modified.Bits.Down = mJOYSTICK.Bits.Up;
				Modified.Bits.Up = mJOYSTICK.Bits.Down;
				return Modified.Byte;
			}
			break;


		case (SWITCHES & 0xff):
			return mSWITCHES.Byte;

// Cartridge reading ports

		case (RCART0&0xff):
			return mSystem.Peek_CARTB0();
			break;
		case (RCART1&0xff):
			return mSystem.Peek_CARTB1();
			break;

// These are not so important so lets ignore them for the moment

		case (LEDS&0xff):
		case (PPORTSTAT&0xff):
		case (PPORTDATA&0xff):
		case (HOWIE&0xff):
			break;

// Errors on write only register accesses

		case (SPRCTL0&0xff):
		case (SPRCTL1&0xff):
		case (SPRCOLL&0xff):
		case (SPRINIT&0xff):
		case (SUZYBUSEN&0xff):
		case (SPRGO&0xff):
//			_RPT2(_CRT_WARN, "CSusie::Peek(%04x) - Peek from write only register location at time %d\n",addr,cycles_used);
			printf("CSusie::Peek(%04x) - Peek from write only register location at time %d\n",addr,cycles_used);
			break;
		
// Errors on illegal location accesses

		default:
//			_RPT2(_CRT_WARN, "CSusie::Peek(%04x) - Peek from illegal location at time %d\n",addr,cycles_used);
			printf("CSusie::Peek(%04x) - Peek from illegal location at time %d\n",addr,cycles_used);
			break;
	}

	return 0xff;
}

