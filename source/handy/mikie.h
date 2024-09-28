//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Mikey class header file                                                  //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and some of the code  //
// for the Mikey chip within the Lynx. The most crucial code is the         //
// Update() function which as you can probably guess updates all of the     //
// Mikey hardware counters and screen DMA from the prevous time it was      //
// called. Yes I know how to spell Mikey but I cant be bothered to change   //
// it everywhere.                                                           //
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

#ifndef MIKIE_H
#define MIKIE_H

class CSystem;

#define MIKIE_START	0xfd00
#define MIKIE_SIZE	0x100

//
// Define counter types and defines
//

#define CTRL_A_IRQEN	0x80
#define CTRL_A_RTD		0x40
#define CTRL_A_RELOAD	0x10
#define CTRL_A_COUNT	0x08
#define CTRL_A_DIVIDE	0x07

#define CTRL_B_TDONE	0x08
#define CTRL_B_LASTCK	0x04
#define CTRL_B_CIN		0x02
#define CTRL_B_COUT		0x01

#define LINE_TIMER		0x00
#define SCREEN_TIMER	0x02

#define LINE_WIDTH		160
#define	LINE_SIZE		80

typedef struct
{
	UBYTE	backup;
	UBYTE	count;
	UBYTE	controlA;
	UBYTE	controlB;
	BOOL	linkedlastcarry;
}MTIMER;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	DMAEnable:1;
			UBYTE	Flip:1;
			UBYTE	FourColour:1;
			UBYTE	Colour:1;
			UBYTE	unused:4;
		}Bits;
		UBYTE	Byte;
	};
}TDISPCTL;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	Green:4;
			UBYTE	Red:4;
			UBYTE	Blue:4;
		}Colours;
		ULONG	Index;
	};
}TPALETTE;

//
// Emumerated types for possible mikie windows independant modes
//
enum
{	
	MIKIE_BAD_MODE=0,

	MIKIE_BITMAP_NORMAL_BPP8_X1,
	MIKIE_BITMAP_NORMAL_BPP16_X1,

	MIKIE_BITMAP_ROTATE_L_BPP8_X1,
	MIKIE_BITMAP_ROTATE_L_BPP16_X1,

	MIKIE_BITMAP_ROTATE_R_BPP8_X1,
	MIKIE_BITMAP_ROTATE_R_BPP16_X1,

	MIKIE_SCREEN_NORMAL_BPP8_X1,
	MIKIE_SCREEN_NORMAL_BPP8_X2,
	MIKIE_SCREEN_NORMAL_BPP8_X3,

	MIKIE_SCREEN_ROTATE_L_BPP8_X1,
	MIKIE_SCREEN_ROTATE_L_BPP8_X2,
	MIKIE_SCREEN_ROTATE_L_BPP8_X3,

	MIKIE_SCREEN_ROTATE_R_BPP8_X1,
	MIKIE_SCREEN_ROTATE_R_BPP8_X2,
	MIKIE_SCREEN_ROTATE_R_BPP8_X3,

	MIKIE_SCREEN_NORMAL_BPP16_X1,
	MIKIE_SCREEN_NORMAL_BPP16_X2,
	MIKIE_SCREEN_NORMAL_BPP16_X3,

	MIKIE_SCREEN_ROTATE_L_BPP16_X1,
	MIKIE_SCREEN_ROTATE_L_BPP16_X2,
	MIKIE_SCREEN_ROTATE_L_BPP16_X3,

	MIKIE_SCREEN_ROTATE_R_BPP16_X1,
	MIKIE_SCREEN_ROTATE_R_BPP16_X2,
	MIKIE_SCREEN_ROTATE_R_BPP16_X3,
};


class CMikie : public CLynxMemObj
{
	public:
		CMikie(CSystem& parent);
		~CMikie();
	
		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
		ULONG	ReadCycle(void) {return 5;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG	ObjectSize(void) {return MIKIE_SIZE;};
		void	Reset(void);
		void	PresetForHomebrew(void);

		void	SetScreenAttributes(ULONG Mode,ULONG XSize,ULONG YSize,ULONG XOffset,ULONG YOffset,UBYTE *Bits0,UBYTE *Bits1)
		{
			mScreenMode=Mode;

			mScreenXsize=XSize;
			mScreenYsize=YSize;

			mImageXoffset=XOffset;
			mImageYoffset=YOffset;

			//
			// Modify bitmap addresses to the correct start points
			//
			// To get the correct bitmap orientation copy lines from
			// the end first so the 1st address needs to be the start
			// of the last line of the bitmap.
			//

			mBitmapBits0=Bits0;
			mBitmapBits1=Bits1;

			switch(mScreenMode)
			{
				case MIKIE_BITMAP_NORMAL_BPP8_X1:
					mBitmapBits0 += (LYNX_SCREEN_WIDTH*LYNX_SCREEN_HEIGHT)-LYNX_SCREEN_WIDTH;
					mBitmapBits1 += (LYNX_SCREEN_WIDTH*LYNX_SCREEN_HEIGHT)-LYNX_SCREEN_WIDTH;
					break;
				case MIKIE_BITMAP_NORMAL_BPP16_X1:
					mBitmapBits0 += (LYNX_SCREEN_WIDTH*LYNX_SCREEN_HEIGHT*2)-(LYNX_SCREEN_WIDTH*2);
					mBitmapBits1 += (LYNX_SCREEN_WIDTH*LYNX_SCREEN_HEIGHT*2)-(LYNX_SCREEN_WIDTH*2);
					break;

				case MIKIE_BITMAP_ROTATE_L_BPP8_X1:
				case MIKIE_BITMAP_ROTATE_L_BPP16_X1:
				case MIKIE_BITMAP_ROTATE_R_BPP8_X1:
				case MIKIE_BITMAP_ROTATE_R_BPP16_X1:
				case MIKIE_SCREEN_NORMAL_BPP8_X1:
				case MIKIE_SCREEN_NORMAL_BPP8_X2:
				case MIKIE_SCREEN_NORMAL_BPP8_X3:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X1:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X2:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X3:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X1:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X2:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X3:
				case MIKIE_SCREEN_NORMAL_BPP16_X1:
				case MIKIE_SCREEN_NORMAL_BPP16_X2:
				case MIKIE_SCREEN_NORMAL_BPP16_X3:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X1:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X2:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X3:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X1:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X2:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X3:
					break;
				default:
					break;
			}

			//
			// Calculate the colour lookup tabes for the relevant mode
			//

			TPALETTE Spot;

			switch(mScreenMode)
			{
				case MIKIE_BITMAP_NORMAL_BPP8_X1:
				case MIKIE_BITMAP_ROTATE_L_BPP8_X1:
				case MIKIE_BITMAP_ROTATE_R_BPP8_X1:
				case MIKIE_SCREEN_NORMAL_BPP8_X1:
				case MIKIE_SCREEN_NORMAL_BPP8_X2:
				case MIKIE_SCREEN_NORMAL_BPP8_X3:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X1:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X2:
				case MIKIE_SCREEN_ROTATE_L_BPP8_X3:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X1:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X2:
				case MIKIE_SCREEN_ROTATE_R_BPP8_X3:
					for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
					{
						mColourMap[Spot.Index] = (Spot.Colours.Red<<4)&0xe0;
						mColourMap[Spot.Index] |= (Spot.Colours.Green<<1)&0x1c;
						mColourMap[Spot.Index] |= (Spot.Colours.Blue>>2)&0x03;
					}
					break;

				case MIKIE_BITMAP_NORMAL_BPP16_X1:
				case MIKIE_BITMAP_ROTATE_L_BPP16_X1:
				case MIKIE_BITMAP_ROTATE_R_BPP16_X1:
				case MIKIE_SCREEN_NORMAL_BPP16_X1:
				case MIKIE_SCREEN_NORMAL_BPP16_X2:
				case MIKIE_SCREEN_NORMAL_BPP16_X3:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X1:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X2:
				case MIKIE_SCREEN_ROTATE_L_BPP16_X3:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X1:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X2:
				case MIKIE_SCREEN_ROTATE_R_BPP16_X3:
					for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
					{
						mColourMap[Spot.Index] =  (Spot.Colours.Blue<<11)&(0x7c00);
						mColourMap[Spot.Index] |= (Spot.Colours.Green<<6)&(0x03e0);
						mColourMap[Spot.Index] |= (Spot.Colours.Red<<1)&(0x001f);
					}
					break;
				default:
					for(Spot.Index=0;Spot.Index<4096;Spot.Index++) mColourMap[Spot.Index] = 0;
					break;
			}

			// Reset screen related counters/vars

			mTIM_0_CURRENT = 0;
			mTIM_2_CURRENT = 0;

			// Fix lastcount so that timer update will definatly occur

			mTIM_0_LAST_COUNT -= (1<<(4+mTIM_0_LINKING))+1;
			mTIM_2_LAST_COUNT -= (1<<(4+mTIM_2_LINKING))+1;

			// Force immediate timer update

			gNextTimerEvent = gSystemCycleCount;
		}
		
		ULONG   GetDisplayBuffer(void) {return (mCurrentBuffer)?0:1;};

		void	BlowOut(void);

		inline void	Update(void)
		{
			static ULONG lynx_addr = 0,line_count = 0;
			static UBYTE *bitmap_addr = NULL,*bitmap_tmp = NULL;
			static BOOL	 line_start = FALSE;
			//static BOOL clocked = FALSE;
			//static BOOL ourcarry = FALSE;
			static ULONG source;
			static ULONG loop;
//			static ULONG loop2;
			static ULONG tmp;
			//static UBYTE bytedata;
			//static UWORD worddata;
			static SLONG divide = 0;
			static SLONG decval = 0;

			//
			// To stop problems with cycle count wrap we will check and then correct the
			// cycle counter.
			//

			if (gSystemCycleCount > 0xf0000000)
			{
				gSystemCycleCount -= 0x80000000;
				gThrottleNextCycleCheckpoint -= 0x80000000;
				gAudioLastUpdateCycle -= 0x80000000;
				mTIM_0_LAST_COUNT -= 0x80000000;
				mTIM_1_LAST_COUNT -= 0x80000000;
				mTIM_2_LAST_COUNT -= 0x80000000;
				mTIM_3_LAST_COUNT -= 0x80000000;
				mTIM_4_LAST_COUNT -= 0x80000000;
				mTIM_5_LAST_COUNT -= 0x80000000;
				mTIM_6_LAST_COUNT -= 0x80000000;
				mTIM_7_LAST_COUNT -= 0x80000000;
				mAUDIO_0_LAST_COUNT -= 0x80000000;
				mAUDIO_1_LAST_COUNT -= 0x80000000;
				mAUDIO_2_LAST_COUNT -= 0x80000000;
				mAUDIO_3_LAST_COUNT -= 0x80000000;
			}

			//	Timer updates, rolled out flat in group order
			//
			//	Group A:
			//	Timer 0 -> Timer 2 -> Timer 4. 
			//
			//	Group B:
			//	Timer 1 -> Timer 3 -> Timer 5 -> Timer 7 -> Audio 0 -> Audio 1-> Audio 2 -> Audio 3 -> Timer 1. 
			//

			//
			// Within each timer code block we will predict the cycle count number of
			// the next timer event
			//
			// We don't need to count linked timers as the timer they are linked
			// from will always generate earlier events.
			//
			// As Timer 4 (UART) will generate many events we will ignore it
			//
			// We set the next event to the end of time at first and let the timers
			// overload it. Any writes to timer controls will force next event to
			// be immediate and hence a new preidction will be done. The prediction
			// causes overflow as opposed to zero i.e. current+1
			// (In reality T0 line counter should always be running.)
			//

			gNextTimerEvent = 0xffffffff;
			
			//
			// Timer 0 of Group A
			//

			//
			// Optimisation, assume T0 (Line timer) is never in one-shot,
			// never placed in link mode
			//

//			if (mTIM_0_ENABLE_COUNT && !mTIM_0_TIMER_DONE)
			if (mTIM_0_ENABLE_COUNT)
			{
				// Timer 0 has no linking
//				if (mTIM_0_LINKING != 0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_0_LINKING);
					decval = (gSystemCycleCount-mTIM_0_LAST_COUNT)>>divide;

					if (decval)
					{
						mTIM_0_LAST_COUNT += decval<<divide;
						mTIM_0_CURRENT -= decval;

						if (mTIM_0_CURRENT & 0x80000000)
						{
							// Set carry out
							mTIM_0_BORROW_OUT = TRUE;
	
							// Set the timer status flag
							if (mTimerInterruptMask & 0x01) mTimerStatusFlags |= 0x01;

//							// Reload if neccessary
//							if (mTIM_0_ENABLE_RELOAD)
//							{
								mTIM_0_CURRENT += mTIM_0_BKUP+1;
//							}
//							else
//							{
//								// Set timer done
//								mTIM_0_TIMER_DONE = TRUE;
//								mTIM_0_CURRENT = 0;
//							}

							// Set timers related to the screen
							if (mBitmapBits0 && mBitmapBits1) line_start = TRUE;

						}
						else
						{
							mTIM_0_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mTIM_0_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_0_BORROW_IN = FALSE;
						// Clear carry out
						mTIM_0_BORROW_OUT = FALSE;
					}
				}

				// Prediction for next timer event cycle number

//				if (mTIM_0_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_0_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}
	
			//
			// Timer 2 of Group A
			//

			//
			// Optimisation, assume T1 (Line timer) is never in one-shot
			// always in linked mode
			//
			
//			if (mTIM_2_ENABLE_COUNT && !mTIM_2_TIMER_DONE)
			if (mTIM_2_ENABLE_COUNT)
			{
				decval = 0;
		
//				if (mTIM_2_LINKING == 0x07)
				{
					if (mTIM_0_BORROW_OUT) decval = 1;
					mTIM_2_LAST_LINK_CARRY = mTIM_0_BORROW_OUT;
				}
//				else
//				{
//					// Ordinary clocked mode as opposed to linked mode
//					// 16MHz clock downto 1us == cyclecount >> 4 
//					divide = (4+mTIM_2_LINKING);
//					decval = (gSystemCycleCount-mTIM_2_LAST_COUNT)>>divide;
//				}
		
				if (decval)
				{
					mTIM_2_LAST_COUNT += decval<<divide;
					mTIM_2_CURRENT -= decval;
					if (mTIM_2_CURRENT & 0x80000000)
					{
						// Set carry out
						mTIM_2_BORROW_OUT = TRUE;
		
						// Set the timer status flag
						if (mTimerInterruptMask & 0x04) mTimerStatusFlags |= 0x04;
		
//						// Reload if neccessary
//						if (mTIM_2_ENABLE_RELOAD)
//						{
							mTIM_2_CURRENT += mTIM_2_BKUP+1;
//						}
//						else
//						{
//							// Set timer done
//							mTIM_2_TIMER_DONE = TRUE;
//							mTIM_2_CURRENT = 0;
//						}
		
						// Set timers related to the screen
						line_count = mTIM_2_BKUP+1;
						bitmap_addr = (mCurrentBuffer)?mBitmapBits1:mBitmapBits0;
					}
					else
					{
						mTIM_2_BORROW_OUT = FALSE;
					}
					// Set carry in as we did a count
					mTIM_2_BORROW_IN = TRUE;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_2_BORROW_IN = FALSE;
					// Clear carry out
					mTIM_2_BORROW_OUT = FALSE;
				}

				// Prediction for next timer event cycle number

//				if (mTIM_2_LINKING != 7)
//				{
//					tmp = gSystemCycleCount+((mTIM_2_CURRENT+1)<<divide);
//					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
//				}
			}
		
			//
			// Timer 4 of Group A
			//
			// For the sake of speed it is assumed that Timer 4 (UART timer)
			// never uses one-shot mode, never uses linking, hence the code
			// is commented out. Timer 4 is at the end of a chain and seems
			// no reason to update its carry in-out variables
			//

//			if (mTIM_4_ENABLE_COUNT && !mTIM_4_TIMER_DONE)
			if (mTIM_4_ENABLE_COUNT) {
				decval = 0;
		
//				if (mTIM_4_LINKING == 0x07) {
//					if (mTIM_2_BORROW_OUT && !mTIM_4_LAST_LINK_CARRY) decval = 1;
//					if (mTIM_2_BORROW_OUT) decval = 1;
//					mTIM_4_LAST_LINK_CARRY = mTIM_2_BORROW_OUT;
//				}
//				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4
					// Additional /8 (+3) for 8 clocks per bit transmit
					divide = 4 + 3 + mTIM_4_LINKING;
					decval = (gSystemCycleCount-mTIM_4_LAST_COUNT)>>divide;
				}
		
				if (decval) {
					mTIM_4_LAST_COUNT += decval<<divide;
					mTIM_4_CURRENT -= decval;
					if (mTIM_4_CURRENT & 0x80000000) {
						// Set carry out
						mTIM_4_BORROW_OUT = TRUE;
		
						//
						// Update the UART counter models for Rx & Tx
						//
		
						//
						// According to the docs IRQ's are level triggered and hence will always assert
						// what a pain in the arse
						//
						// Rx & Tx are loopedback due to comlynx structure

						if (!mUART_RX_COUNTDOWN) {
//							if (mUART_RX_IRQ_ENABLE && (mTimerInterruptMask & 0x10)) mTimerStatusFlags |= 0x10;
							if (mUART_RX_IRQ_ENABLE) mTimerStatusFlags |= 0x10;
						}
						else {
							mUART_RX_COUNTDOWN--;
						}
		
		
						if (!mUART_TX_COUNTDOWN) {
//							if (mUART_TX_IRQ_ENABLE && (mTimerInterruptMask & 0x10)) mTimerStatusFlags |= 0x10;
							if (mUART_TX_IRQ_ENABLE) mTimerStatusFlags |= 0x10;
						}
						else {
							mUART_TX_COUNTDOWN--;
						}
		
		
						// Set the timer status flag
						// Timer 4 is the uart timer and doesn't generate IRQ's using this method
		
						// 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters
		
						// Reload if neccessary
//						if (mTIM_4_ENABLE_RELOAD)
//						{
							mTIM_4_CURRENT += mTIM_4_BKUP+1;
//						}
//						else
//						{
//							// Set timer done
//							mTIM_4_TIMER_DONE = TRUE;
//							mTIM_4_CURRENT = 0;
//						}
					}
//					else
//					{
//						mTIM_4_BORROW_OUT = FALSE;
//					}
//					// Set carry in as we did a count
//					mTIM_4_BORROW_IN = TRUE;
				}
//				else
//				{
//					// Clear carry in as we didn't count
//					mTIM_4_BORROW_IN = FALSE;
//					// Clear carry out
//					mTIM_4_BORROW_OUT = FALSE;
//				}
//
//				// Prediction for next timer event cycle number
//
//				if (mTIM_4_LINKING != 7)
//				{
//					tmp = gSystemCycleCount+((mTIM_4_CURRENT+1)<<divide);
//					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
//				}
			}
		
			//
			// Timer 1 of Group B
			//
			if (mTIM_1_ENABLE_COUNT && !mTIM_1_TIMER_DONE)
			{
				if (mTIM_1_LINKING != 0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_1_LINKING);
					decval = (gSystemCycleCount-mTIM_1_LAST_COUNT)>>divide;
		
					if (decval)
					{
						mTIM_1_LAST_COUNT += decval<<divide;
						mTIM_1_CURRENT -= decval;
						if (mTIM_1_CURRENT & 0x80000000)
						{
							// Set carry out
							mTIM_1_BORROW_OUT = TRUE;
		
							// Set the timer status flag
							if (mTimerInterruptMask & 0x02) mTimerStatusFlags |= 0x02;
		
							// Reload if neccessary
							if (mTIM_1_ENABLE_RELOAD)
							{
								mTIM_1_CURRENT += mTIM_1_BKUP+1;
							}
							else
							{
								// Set timer done
								mTIM_1_TIMER_DONE = TRUE;
								mTIM_1_CURRENT = 0;
							}
						}
						else
						{
							mTIM_1_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mTIM_1_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_1_BORROW_IN = FALSE;
						// Clear carry out
						mTIM_1_BORROW_OUT = FALSE;
					}
				}

				// Prediction for next timer event cycle number

				if (mTIM_1_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_1_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}
		
			//
			// Timer 3 of Group A
			//
			if (mTIM_3_ENABLE_COUNT && !mTIM_3_TIMER_DONE)
			{
				decval = 0;
		
				if (mTIM_3_LINKING == 0x07)
				{
					if (mTIM_1_BORROW_OUT) decval = 1;
					mTIM_3_LAST_LINK_CARRY = mTIM_1_BORROW_OUT;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_3_LINKING);
					decval = (gSystemCycleCount-mTIM_3_LAST_COUNT)>>divide;
				}
		
				if (decval)
				{
					mTIM_3_LAST_COUNT += decval<<divide;
					mTIM_3_CURRENT -= decval;
					if (mTIM_3_CURRENT & 0x80000000)
					{
						// Set carry out
						mTIM_3_BORROW_OUT = TRUE;
		
						// Set the timer status flag
						if (mTimerInterruptMask & 0x08) mTimerStatusFlags |= 0x08;
		
						// Reload if neccessary
						if (mTIM_3_ENABLE_RELOAD)
						{
							mTIM_3_CURRENT += mTIM_3_BKUP+1;
						}
						else
						{
							// Set timer done
							mTIM_3_TIMER_DONE = TRUE;
							mTIM_3_CURRENT = 0;
						}
					}
					else
					{
						mTIM_3_BORROW_OUT = FALSE;
					}
					// Set carry in as we did a count
					mTIM_3_BORROW_IN = TRUE;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_3_BORROW_IN = FALSE;
					// Clear carry out
					mTIM_3_BORROW_OUT = FALSE;
				}

				// Prediction for next timer event cycle number

				if (mTIM_3_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_3_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}
		
			//
			// Timer 5 of Group A
			//
			if (mTIM_5_ENABLE_COUNT && !mTIM_5_TIMER_DONE)
			{
				decval = 0;
		
				if (mTIM_5_LINKING == 0x07)
				{
					if (mTIM_3_BORROW_OUT) decval = 1;
					mTIM_5_LAST_LINK_CARRY = mTIM_3_BORROW_OUT;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_5_LINKING);
					decval = (gSystemCycleCount-mTIM_5_LAST_COUNT)>>divide;
				}
		
				if (decval)
				{
					mTIM_5_LAST_COUNT += decval<<divide;
					mTIM_5_CURRENT -= decval;
					if (mTIM_5_CURRENT & 0x80000000)
					{
						// Set carry out
						mTIM_5_BORROW_OUT = TRUE;
		
						// Set the timer status flag
						if (mTimerInterruptMask & 0x20) mTimerStatusFlags |= 0x20;
		
						// Reload if neccessary
						if (mTIM_5_ENABLE_RELOAD)
						{
							mTIM_5_CURRENT += mTIM_5_BKUP+1;
						}
						else
						{
							// Set timer done
							mTIM_5_TIMER_DONE = TRUE;
							mTIM_5_CURRENT = 0;
						}
					}
					else
					{
						mTIM_5_BORROW_OUT = FALSE;
					}
					// Set carry in as we did a count
					mTIM_5_BORROW_IN = TRUE;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_5_BORROW_IN = FALSE;
					// Clear carry out
					mTIM_5_BORROW_OUT = FALSE;
				}

				// Prediction for next timer event cycle number

				if (mTIM_5_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_5_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}
		
			//
			// Timer 7 of Group A
			//
			if (mTIM_7_ENABLE_COUNT && !mTIM_7_TIMER_DONE)
			{
				decval = 0;
		
				if (mTIM_7_LINKING == 0x07)
				{
					if (mTIM_5_BORROW_OUT) decval = 1;
					mTIM_7_LAST_LINK_CARRY = mTIM_5_BORROW_OUT;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_7_LINKING);
					decval = (gSystemCycleCount-mTIM_7_LAST_COUNT)>>divide;
				}
		
				if (decval)
				{
					mTIM_7_LAST_COUNT += decval<<divide;
					mTIM_7_CURRENT -= decval;
					if (mTIM_7_CURRENT & 0x80000000)
					{
						// Set carry out
						mTIM_7_BORROW_OUT = TRUE;
		
						// Set the timer status flag
						if (mTimerInterruptMask & 0x80) mTimerStatusFlags |= 0x80;
		
						// Reload if neccessary
						if (mTIM_7_ENABLE_RELOAD)
						{
							mTIM_7_CURRENT += mTIM_7_BKUP+1;
						}
						else
						{
							// Set timer done
							mTIM_7_TIMER_DONE = TRUE;
							mTIM_7_CURRENT = 0;
						}
		
					}
					else
					{
						mTIM_7_BORROW_OUT = FALSE;
					}
					// Set carry in as we did a count
					mTIM_7_BORROW_IN = TRUE;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_7_BORROW_IN = FALSE;
					// Clear carry out
					mTIM_7_BORROW_OUT = FALSE;
				}

				// Prediction for next timer event cycle number

				if (mTIM_7_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_7_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}
		
			//
			// Timer 6 has no group
			//
			if (mTIM_6_ENABLE_COUNT && !mTIM_6_TIMER_DONE)
			{
//				if (mTIM_6_LINKING != 0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide = (4+mTIM_6_LINKING);
					decval = (gSystemCycleCount-mTIM_6_LAST_COUNT)>>divide;
		
					if (decval)
					{
						mTIM_6_LAST_COUNT += decval<<divide;
						mTIM_6_CURRENT -= decval;
						if (mTIM_6_CURRENT & 0x80000000)
						{
							// Set carry out
							mTIM_6_BORROW_OUT = TRUE;
		
							// Set the timer status flag
							if (mTimerInterruptMask & 0x40) mTimerStatusFlags |= 0x40;
		
							// Reload if neccessary
							if (mTIM_6_ENABLE_RELOAD)
							{
								mTIM_6_CURRENT += mTIM_6_BKUP+1;
							}
							else
							{
								// Set timer done
								mTIM_6_TIMER_DONE = TRUE;
								mTIM_6_CURRENT = 0;
							}
						}
						else
						{
							mTIM_6_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mTIM_6_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_6_BORROW_IN = FALSE;
						// Clear carry out
						mTIM_6_BORROW_OUT = FALSE;
					}
				}

				// Prediction for next timer event cycle number
				// (Timer 6 doesn't support linking)

//				if (mTIM_6_LINKING != 7)
				{
					tmp = gSystemCycleCount+((mTIM_6_CURRENT+1)<<divide);
					if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
				}
			}

			//
			// If sound is enabled then update the sound subsystem
			//
			if (gAudioEnabled)
			{
				static ULONG audio_buffer_pointer = 0;
				static SLONG sample = 0;
				
				//
				// Catch audio buffer up to current time
				//
				for (;gAudioLastUpdateCycle+HANDY_AUDIO_SAMPLE_PERIOD<gSystemCycleCount;gAudioLastUpdateCycle += HANDY_AUDIO_SAMPLE_PERIOD)
				{
					// Mix the sample
					sample = 0;
					sample += (mSTEREO&0x11)?mAUDIO_0_OUTPUT:0;
					sample += (mSTEREO&0x22)?mAUDIO_1_OUTPUT:0;
					sample += (mSTEREO&0x44)?mAUDIO_2_OUTPUT:0;
					sample += (mSTEREO&0x88)?mAUDIO_3_OUTPUT:0;
					sample = sample>>2;
					sample += 128;
					// Output audio sample
					gAudioBuffer[gAudioPlaybackBufferNumber^0x01][audio_buffer_pointer++] = (UBYTE)sample;

					// Check buffer overflow condition
					if (audio_buffer_pointer >= HANDY_AUDIO_BUFFER_SIZE)
					{
						gAudioPlaybackBufferNumber ^= 0x01;
						audio_buffer_pointer = 0;
					}
				}

				//
				// Audio 0 
				//
//				if (mAUDIO_0_ENABLE_COUNT && !mAUDIO_0_TIMER_DONE)
				if (mAUDIO_0_ENABLE_COUNT && !mAUDIO_0_TIMER_DONE && mAUDIO_0_VOLUME && mAUDIO_0_BKUP)
				{
					decval = 0;
		
					if (mAUDIO_0_LINKING == 0x07)
					{
						if (mTIM_7_BORROW_OUT) decval = 1;
						mAUDIO_0_LAST_LINK_CARRY = mTIM_7_BORROW_OUT;
					}
					else
					{
						// Ordinary clocked mode as opposed to linked mode
						// 16MHz clock downto 1us == cyclecount >> 4 
						divide = (4+mAUDIO_0_LINKING);
						decval = (gSystemCycleCount-mAUDIO_0_LAST_COUNT)>>divide;
					}
		
					if (decval)
					{
						mAUDIO_0_LAST_COUNT += decval<<divide;
						mAUDIO_0_CURRENT -= decval;
						if (mAUDIO_0_CURRENT & 0x80000000)
						{
							// Set carry out
							mAUDIO_0_BORROW_OUT = TRUE;
		
							// Reload if neccessary
							if (mAUDIO_0_ENABLE_RELOAD)
							{
								mAUDIO_0_CURRENT += mAUDIO_0_BKUP+1;
								if (mAUDIO_0_CURRENT & 0x80000000) mAUDIO_0_CURRENT = 0;
							}
							else
							{
								// Set timer done
								mAUDIO_0_TIMER_DONE = TRUE;
								mAUDIO_0_CURRENT = 0;
							}

							//
							// Update audio circuitry
							//

							mAUDIO_0_WAVESHAPER = gAudioWaveShaperLookupTable[mAUDIO_0_WAVESHAPER];

							if (mAUDIO_0_INTEGRATE_ENABLE)
							{
								SLONG temp = mAUDIO_0_OUTPUT;
								if (mAUDIO_0_WAVESHAPER & 0x0001) temp += mAUDIO_0_VOLUME; else temp -= mAUDIO_0_VOLUME;
								if (temp > 127) temp = 127;
								if (temp < -128) temp = -128;
								mAUDIO_0_OUTPUT = (SBYTE)temp;
							}
							else
							{
								if (mAUDIO_0_WAVESHAPER & 0x0001) mAUDIO_0_OUTPUT = mAUDIO_0_VOLUME; else mAUDIO_0_OUTPUT = -mAUDIO_0_VOLUME;
							}
						}
						else
						{
							mAUDIO_0_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mAUDIO_0_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mAUDIO_0_BORROW_IN = FALSE;
						// Clear carry out
						mAUDIO_0_BORROW_OUT = FALSE;
					}

					// Prediction for next timer event cycle number

					if (mAUDIO_0_LINKING != 7)
					{
						tmp = gSystemCycleCount+((mAUDIO_0_CURRENT+1)<<divide);
						if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
					}
				}

				//
				// Audio 1 
				//
//				if (mAUDIO_1_ENABLE_COUNT && !mAUDIO_1_TIMER_DONE)
				if (mAUDIO_1_ENABLE_COUNT && !mAUDIO_1_TIMER_DONE && mAUDIO_1_VOLUME && mAUDIO_1_BKUP)
				{
					decval = 0;
		
					if (mAUDIO_1_LINKING == 0x07)
					{
						if (mAUDIO_0_BORROW_OUT) decval = 1;
						mAUDIO_1_LAST_LINK_CARRY = mAUDIO_0_BORROW_OUT;
					}
					else
					{
						// Ordinary clocked mode as opposed to linked mode
						// 16MHz clock downto 1us == cyclecount >> 4 
						divide = (4+mAUDIO_1_LINKING);
						decval = (gSystemCycleCount-mAUDIO_1_LAST_COUNT)>>divide;
					}
		
					if (decval)
					{
						mAUDIO_1_LAST_COUNT += decval<<divide;
						mAUDIO_1_CURRENT -= decval;
						if (mAUDIO_1_CURRENT & 0x80000000)
						{
							// Set carry out
							mAUDIO_1_BORROW_OUT = TRUE;
		
							// Reload if neccessary
							if (mAUDIO_1_ENABLE_RELOAD)
							{
								mAUDIO_1_CURRENT += mAUDIO_1_BKUP+1;
								if (mAUDIO_1_CURRENT & 0x80000000) mAUDIO_1_CURRENT = 0;
							}
							else
							{
								// Set timer done
								mAUDIO_1_TIMER_DONE = TRUE;
								mAUDIO_1_CURRENT = 0;
							}

							//
							// Update audio circuitry
							//

							mAUDIO_1_WAVESHAPER = gAudioWaveShaperLookupTable[mAUDIO_1_WAVESHAPER];

							if (mAUDIO_1_INTEGRATE_ENABLE)
							{
								SLONG temp = mAUDIO_1_OUTPUT;
								if (mAUDIO_1_WAVESHAPER & 0x0001) temp += mAUDIO_1_VOLUME; else temp -= mAUDIO_1_VOLUME;
								if (temp > 127) temp = 127;
								if (temp < -128) temp = -128;
								mAUDIO_1_OUTPUT = (SBYTE)temp;
							}
							else
							{
								if (mAUDIO_1_WAVESHAPER & 0x0001) mAUDIO_1_OUTPUT = mAUDIO_1_VOLUME; else mAUDIO_1_OUTPUT = -mAUDIO_1_VOLUME;
							}
						}
						else
						{
							mAUDIO_1_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mAUDIO_1_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mAUDIO_1_BORROW_IN = FALSE;
						// Clear carry out
						mAUDIO_1_BORROW_OUT = FALSE;
					}

					// Prediction for next timer event cycle number

					if (mAUDIO_1_LINKING != 7)
					{
						tmp = gSystemCycleCount+((mAUDIO_1_CURRENT+1)<<divide);
						if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
					}
				}

				//
				// Audio 2 
				//
//				if (mAUDIO_2_ENABLE_COUNT && !mAUDIO_2_TIMER_DONE)
				if (mAUDIO_2_ENABLE_COUNT && !mAUDIO_2_TIMER_DONE && mAUDIO_2_VOLUME && mAUDIO_2_BKUP)
				{
					decval = 0;
		
					if (mAUDIO_2_LINKING == 0x07)
					{
						if (mAUDIO_1_BORROW_OUT) decval = 1;
						mAUDIO_2_LAST_LINK_CARRY = mAUDIO_1_BORROW_OUT;
					}
					else
					{
						// Ordinary clocked mode as opposed to linked mode
						// 16MHz clock downto 1us == cyclecount >> 4 
						divide = (4+mAUDIO_2_LINKING);
						decval = (gSystemCycleCount-mAUDIO_2_LAST_COUNT)>>divide;
					}
		
					if (decval)
					{
						mAUDIO_2_LAST_COUNT += decval<<divide;
						mAUDIO_2_CURRENT -= decval;
						if (mAUDIO_2_CURRENT & 0x80000000)
						{
							// Set carry out
							mAUDIO_2_BORROW_OUT = TRUE;
		
							// Reload if neccessary
							if (mAUDIO_2_ENABLE_RELOAD)
							{
								mAUDIO_2_CURRENT += mAUDIO_2_BKUP+1;
								if (mAUDIO_2_CURRENT & 0x80000000) mAUDIO_2_CURRENT = 0;
							}
							else
							{
								// Set timer done
								mAUDIO_2_TIMER_DONE = TRUE;
								mAUDIO_2_CURRENT = 0;
							}

							//
							// Update audio circuitry
							//

							mAUDIO_2_WAVESHAPER = gAudioWaveShaperLookupTable[mAUDIO_2_WAVESHAPER];

							if (mAUDIO_2_INTEGRATE_ENABLE)
							{
								SLONG temp = mAUDIO_2_OUTPUT;
								if (mAUDIO_2_WAVESHAPER & 0x0001) temp += mAUDIO_2_VOLUME; else temp -= mAUDIO_2_VOLUME;
								if (temp > 127) temp = 127;
								if (temp < -128) temp = -128;
								mAUDIO_2_OUTPUT = (SBYTE)temp;
							}
							else
							{
								if (mAUDIO_2_WAVESHAPER & 0x0001) mAUDIO_2_OUTPUT = mAUDIO_2_VOLUME; else mAUDIO_2_OUTPUT = -mAUDIO_2_VOLUME;
							}
						}
						else
						{
							mAUDIO_2_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mAUDIO_2_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mAUDIO_2_BORROW_IN = FALSE;
						// Clear carry out
						mAUDIO_2_BORROW_OUT = FALSE;
					}

					// Prediction for next timer event cycle number

					if (mAUDIO_2_LINKING != 7)
					{
						tmp = gSystemCycleCount+((mAUDIO_2_CURRENT+1)<<divide);
						if (tmp<gNextTimerEvent) gNextTimerEvent = tmp;
					}
				}

				//
				// Audio 3
				//
//				if (mAUDIO_3_ENABLE_COUNT && !mAUDIO_3_TIMER_DONE)
				if (mAUDIO_3_ENABLE_COUNT && !mAUDIO_3_TIMER_DONE && mAUDIO_3_VOLUME && mAUDIO_3_BKUP)
				{
					decval = 0;
		
					if (mAUDIO_3_LINKING == 0x07)
					{
						if (mAUDIO_2_BORROW_OUT) decval = 1;
						mAUDIO_3_LAST_LINK_CARRY = mAUDIO_2_BORROW_OUT;
					}
					else
					{
						// Ordinary clocked mode as opposed to linked mode
						// 16MHz clock downto 1us == cyclecount >> 4 
						divide = (4+mAUDIO_3_LINKING);
						decval = (gSystemCycleCount-mAUDIO_3_LAST_COUNT)>>divide;
					}
		
					if (decval)
					{
						mAUDIO_3_LAST_COUNT += decval<<divide;
						mAUDIO_3_CURRENT -= decval;
						if (mAUDIO_3_CURRENT & 0x80000000)
						{
							// Set carry out
							mAUDIO_3_BORROW_OUT = TRUE;
		
							// Reload if neccessary
							if (mAUDIO_3_ENABLE_RELOAD)
							{
								mAUDIO_3_CURRENT += mAUDIO_3_BKUP+1;
								if (mAUDIO_3_CURRENT & 0x80000000) mAUDIO_3_CURRENT = 0;
							}
							else
							{
								// Set timer done
								mAUDIO_3_TIMER_DONE = TRUE;
								mAUDIO_3_CURRENT = 0;
							}

							//
							// Update audio circuitry
							//
							mAUDIO_3_WAVESHAPER = gAudioWaveShaperLookupTable[mAUDIO_3_WAVESHAPER];

							if (mAUDIO_3_INTEGRATE_ENABLE)
							{
								SLONG temp = mAUDIO_3_OUTPUT;
								if (mAUDIO_3_WAVESHAPER&0x0001) temp += mAUDIO_3_VOLUME; else temp -= mAUDIO_3_VOLUME;
								if (temp > 127) temp = 127;
								if (temp < -128) temp = -128;
								mAUDIO_3_OUTPUT = (SBYTE)temp;
							}
							else
							{
								if (mAUDIO_3_WAVESHAPER&0x0001) mAUDIO_3_OUTPUT = mAUDIO_3_VOLUME; else mAUDIO_3_OUTPUT = -mAUDIO_3_VOLUME;
							}
						}
						else
						{
							mAUDIO_3_BORROW_OUT = FALSE;
						}
						// Set carry in as we did a count
						mAUDIO_3_BORROW_IN = TRUE;
					}
					else
					{
						// Clear carry in as we didn't count
						mAUDIO_3_BORROW_IN = FALSE;
						// Clear carry out
						mAUDIO_3_BORROW_OUT = FALSE;
					}

					// Prediction for next timer event cycle number

					if (mAUDIO_3_LINKING != 7)
					{
						tmp = gSystemCycleCount+((mAUDIO_3_CURRENT+1)<<divide);
						if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
					}
				}
			}

			// Update system IRQ status as a result of timer activity
			// OR is required to ensure serial IRQ's are not masked accidentally
		
			gSystemIRQ=mTimerStatusFlags;

			//
			// Perform screen bitmap update
			//
			if (line_start && mDISPCTL_DMAEnable)
			{
				if (line_count > 102)
				{
					// VBL Period emulation
		
					line_start = FALSE;
					line_count--;
		
					// This is a waste as these variables get set 3 times
					// but I can't think of a better place to put it.
					// If the address is set by the timer overflow then frame
					// flipping doesnt work correctly. The three blank lines
					// are usually used to set the frame address.
		
					if (mDISPCTL_Flip)
					{
						lynx_addr = mDisplayAddress & 0xfffc;
						lynx_addr += 3;
					}
					else
					{
						lynx_addr = mDisplayAddress & 0xfffc;
					}
				}
				else
				{
					// Mikie screen DMA can only see the system RAM....
					// (Step through bitmap, line at a time)
		
					// Speedup, code change:
					//
					//    from
					// source=mSystem.Peek_RAM(lynx_addr++);
					//    to
					// source=mRamPointer[lynx_addr++];
					//

					switch (mScreenMode)
					{

						case MIKIE_BITMAP_NORMAL_BPP8_X1:
							for (loop=0;loop<LYNX_SCREEN_WIDTH/2;loop++)
							{
								source = mRamPointer[lynx_addr];
								if (mDISPCTL_Flip)
								{
									lynx_addr--;
									*(bitmap_addr) = (UBYTE)mColourMap[mPalette[source&0x0f].Index];
									bitmap_addr += 1;
									*(bitmap_addr) = (UBYTE)mColourMap[mPalette[source>>4].Index];
									bitmap_addr += 1;
								}
								else
								{
									lynx_addr++;
									*(bitmap_addr) = (UBYTE)mColourMap[mPalette[source>>4].Index];
									bitmap_addr += 1;
									*(bitmap_addr) = (UBYTE)mColourMap[mPalette[source&0x0f].Index];
									bitmap_addr += 1;
								}
							}
							bitmap_addr -= LYNX_SCREEN_WIDTH*2;
							break;

						case MIKIE_BITMAP_NORMAL_BPP16_X1:
							for (loop=0;loop<LYNX_SCREEN_WIDTH/2;loop++)
							{
								source = mRamPointer[lynx_addr];
								if (mDISPCTL_Flip)
								{
									lynx_addr--;
									*((UWORD*)(bitmap_addr)) = (UWORD)mColourMap[mPalette[source&0x0f].Index];
									bitmap_addr += 2;
									*((UWORD*)(bitmap_addr)) = (UWORD)mColourMap[mPalette[source>>4].Index];
									bitmap_addr += 2;
								}
								else
								{
									lynx_addr++;
									*((UWORD*)(bitmap_addr)) = (UWORD)mColourMap[mPalette[source>>4].Index];
									bitmap_addr += 2;
									*((UWORD*)(bitmap_addr)) = (UWORD)mColourMap[mPalette[source&0x0f].Index];
									bitmap_addr += 2;
								}
							}
							bitmap_addr -= (LYNX_SCREEN_WIDTH*2)*2;
							break;
	
						case MIKIE_BITMAP_ROTATE_L_BPP8_X1:
						case MIKIE_BITMAP_ROTATE_L_BPP16_X1:
						case MIKIE_BITMAP_ROTATE_R_BPP8_X1:
						case MIKIE_BITMAP_ROTATE_R_BPP16_X1:
						case MIKIE_SCREEN_NORMAL_BPP8_X1:
						case MIKIE_SCREEN_NORMAL_BPP8_X2:
						case MIKIE_SCREEN_NORMAL_BPP8_X3:
						case MIKIE_SCREEN_ROTATE_L_BPP8_X1:
						case MIKIE_SCREEN_ROTATE_L_BPP8_X2:
						case MIKIE_SCREEN_ROTATE_L_BPP8_X3:
						case MIKIE_SCREEN_ROTATE_R_BPP8_X1:
						case MIKIE_SCREEN_ROTATE_R_BPP8_X2:
						case MIKIE_SCREEN_ROTATE_R_BPP8_X3:
						case MIKIE_SCREEN_NORMAL_BPP16_X1:
						case MIKIE_SCREEN_NORMAL_BPP16_X2:
						case MIKIE_SCREEN_NORMAL_BPP16_X3:
						case MIKIE_SCREEN_ROTATE_L_BPP16_X1:
						case MIKIE_SCREEN_ROTATE_L_BPP16_X2:
						case MIKIE_SCREEN_ROTATE_L_BPP16_X3:
						case MIKIE_SCREEN_ROTATE_R_BPP16_X1:
						case MIKIE_SCREEN_ROTATE_R_BPP16_X2:
						case MIKIE_SCREEN_ROTATE_R_BPP16_X3:
						case MIKIE_BAD_MODE:
						default:
							break;
					}

					// Cycle hit for a 80 RAM access
					gSystemCycleCount += 80*5;
		
					// Check for end of line
			
					line_start = FALSE;
					if (!--line_count)
					{
						// Flip screen buffers
						mCurrentBuffer = (mCurrentBuffer)?0:1;
						// Indicate to upstairs the old buffer is ready
						gScreenUpdateRequired = TRUE;
					}
				}
			}
		}

	private:
		CSystem		&mSystem;

		// Hardware storage
		
		ULONG		mDisplayAddress;
		BOOL		mAudioInputComparator;
		ULONG		mTimerStatusFlags;
		ULONG		mTimerInterruptMask;

		TPALETTE	mPalette[16];
		ULONG		mColourMap[4096];

		ULONG		mIODAT;
		ULONG		mIODIR;

		ULONG		mDISPCTL_DMAEnable;
		ULONG		mDISPCTL_Flip;
		ULONG		mDISPCTL_FourColour;
		ULONG		mDISPCTL_Colour;

		ULONG		mTIM_0_BKUP;
		ULONG		mTIM_0_ENABLE_RELOAD;
		ULONG		mTIM_0_ENABLE_COUNT;
		ULONG		mTIM_0_LINKING;
		ULONG		mTIM_0_CURRENT;
		ULONG		mTIM_0_TIMER_DONE;
		ULONG		mTIM_0_LAST_CLOCK;
		ULONG		mTIM_0_BORROW_IN;
		ULONG		mTIM_0_BORROW_OUT;
		ULONG		mTIM_0_LAST_LINK_CARRY;
		ULONG		mTIM_0_LAST_COUNT;

		ULONG		mTIM_1_BKUP;
		ULONG		mTIM_1_ENABLE_RELOAD;
		ULONG		mTIM_1_ENABLE_COUNT;
		ULONG		mTIM_1_LINKING;
		ULONG		mTIM_1_CURRENT;
		ULONG		mTIM_1_TIMER_DONE;
		ULONG		mTIM_1_LAST_CLOCK;
		ULONG		mTIM_1_BORROW_IN;
		ULONG		mTIM_1_BORROW_OUT;
		ULONG		mTIM_1_LAST_LINK_CARRY;
		ULONG		mTIM_1_LAST_COUNT;

		ULONG		mTIM_2_BKUP;
		ULONG		mTIM_2_ENABLE_RELOAD;
		ULONG		mTIM_2_ENABLE_COUNT;
		ULONG		mTIM_2_LINKING;
		ULONG		mTIM_2_CURRENT;
		ULONG		mTIM_2_TIMER_DONE;
		ULONG		mTIM_2_LAST_CLOCK;
		ULONG		mTIM_2_BORROW_IN;
		ULONG		mTIM_2_BORROW_OUT;
		ULONG		mTIM_2_LAST_LINK_CARRY;
		ULONG		mTIM_2_LAST_COUNT;

		ULONG		mTIM_3_BKUP;
		ULONG		mTIM_3_ENABLE_RELOAD;
		ULONG		mTIM_3_ENABLE_COUNT;
		ULONG		mTIM_3_LINKING;
		ULONG		mTIM_3_CURRENT;
		ULONG		mTIM_3_TIMER_DONE;
		ULONG		mTIM_3_LAST_CLOCK;
		ULONG		mTIM_3_BORROW_IN;
		ULONG		mTIM_3_BORROW_OUT;
		ULONG		mTIM_3_LAST_LINK_CARRY;
		ULONG		mTIM_3_LAST_COUNT;

		ULONG		mTIM_4_BKUP;
		ULONG		mTIM_4_ENABLE_RELOAD;
		ULONG		mTIM_4_ENABLE_COUNT;
		ULONG		mTIM_4_LINKING;
		ULONG		mTIM_4_CURRENT;
		ULONG		mTIM_4_TIMER_DONE;
		ULONG		mTIM_4_LAST_CLOCK;
		ULONG		mTIM_4_BORROW_IN;
		ULONG		mTIM_4_BORROW_OUT;
		ULONG		mTIM_4_LAST_LINK_CARRY;
		ULONG		mTIM_4_LAST_COUNT;

		ULONG		mTIM_5_BKUP;
		ULONG		mTIM_5_ENABLE_RELOAD;
		ULONG		mTIM_5_ENABLE_COUNT;
		ULONG		mTIM_5_LINKING;
		ULONG		mTIM_5_CURRENT;
		ULONG		mTIM_5_TIMER_DONE;
		ULONG		mTIM_5_LAST_CLOCK;
		ULONG		mTIM_5_BORROW_IN;
		ULONG		mTIM_5_BORROW_OUT;
		ULONG		mTIM_5_LAST_LINK_CARRY;
		ULONG		mTIM_5_LAST_COUNT;

		ULONG		mTIM_6_BKUP;
		ULONG		mTIM_6_ENABLE_RELOAD;
		ULONG		mTIM_6_ENABLE_COUNT;
		ULONG		mTIM_6_LINKING;
		ULONG		mTIM_6_CURRENT;
		ULONG		mTIM_6_TIMER_DONE;
		ULONG		mTIM_6_LAST_CLOCK;
		ULONG		mTIM_6_BORROW_IN;
		ULONG		mTIM_6_BORROW_OUT;
		ULONG		mTIM_6_LAST_LINK_CARRY;
		ULONG		mTIM_6_LAST_COUNT;

		ULONG		mTIM_7_BKUP;
		ULONG		mTIM_7_ENABLE_RELOAD;
		ULONG		mTIM_7_ENABLE_COUNT;
		ULONG		mTIM_7_LINKING;
		ULONG		mTIM_7_CURRENT;
		ULONG		mTIM_7_TIMER_DONE;
		ULONG		mTIM_7_LAST_CLOCK;
		ULONG		mTIM_7_BORROW_IN;
		ULONG		mTIM_7_BORROW_OUT;
		ULONG		mTIM_7_LAST_LINK_CARRY;
		ULONG		mTIM_7_LAST_COUNT;

		ULONG		mAUDIO_0_BKUP;
		ULONG		mAUDIO_0_ENABLE_RELOAD;
		ULONG		mAUDIO_0_ENABLE_COUNT;
		ULONG		mAUDIO_0_LINKING;
		ULONG		mAUDIO_0_CURRENT;
		ULONG		mAUDIO_0_TIMER_DONE;
		ULONG		mAUDIO_0_LAST_CLOCK;
		ULONG		mAUDIO_0_BORROW_IN;
		ULONG		mAUDIO_0_BORROW_OUT;
		ULONG		mAUDIO_0_LAST_LINK_CARRY;
		ULONG		mAUDIO_0_LAST_COUNT;
		SBYTE		mAUDIO_0_VOLUME;
		SBYTE		mAUDIO_0_OUTPUT;
		ULONG		mAUDIO_0_INTEGRATE_ENABLE;
		ULONG		mAUDIO_0_WAVESHAPER;

		ULONG		mAUDIO_1_BKUP;
		ULONG		mAUDIO_1_ENABLE_RELOAD;
		ULONG		mAUDIO_1_ENABLE_COUNT;
		ULONG		mAUDIO_1_LINKING;
		ULONG		mAUDIO_1_CURRENT;
		ULONG		mAUDIO_1_TIMER_DONE;
		ULONG		mAUDIO_1_LAST_CLOCK;
		ULONG		mAUDIO_1_BORROW_IN;
		ULONG		mAUDIO_1_BORROW_OUT;
		ULONG		mAUDIO_1_LAST_LINK_CARRY;
		ULONG		mAUDIO_1_LAST_COUNT;
		SBYTE		mAUDIO_1_VOLUME;
		SBYTE		mAUDIO_1_OUTPUT;
		ULONG		mAUDIO_1_INTEGRATE_ENABLE;
		ULONG		mAUDIO_1_WAVESHAPER;

		ULONG		mAUDIO_2_BKUP;
		ULONG		mAUDIO_2_ENABLE_RELOAD;
		ULONG		mAUDIO_2_ENABLE_COUNT;
		ULONG		mAUDIO_2_LINKING;
		ULONG		mAUDIO_2_CURRENT;
		ULONG		mAUDIO_2_TIMER_DONE;
		ULONG		mAUDIO_2_LAST_CLOCK;
		ULONG		mAUDIO_2_BORROW_IN;
		ULONG		mAUDIO_2_BORROW_OUT;
		ULONG		mAUDIO_2_LAST_LINK_CARRY;
		ULONG		mAUDIO_2_LAST_COUNT;
		SBYTE		mAUDIO_2_VOLUME;
		SBYTE		mAUDIO_2_OUTPUT;
		ULONG		mAUDIO_2_INTEGRATE_ENABLE;
		ULONG		mAUDIO_2_WAVESHAPER;

		ULONG		mAUDIO_3_BKUP;
		ULONG		mAUDIO_3_ENABLE_RELOAD;
		ULONG		mAUDIO_3_ENABLE_COUNT;
		ULONG		mAUDIO_3_LINKING;
		ULONG		mAUDIO_3_CURRENT;
		ULONG		mAUDIO_3_TIMER_DONE;
		ULONG		mAUDIO_3_LAST_CLOCK;
		ULONG		mAUDIO_3_BORROW_IN;
		ULONG		mAUDIO_3_BORROW_OUT;
		ULONG		mAUDIO_3_LAST_LINK_CARRY;
		ULONG		mAUDIO_3_LAST_COUNT;
		SBYTE		mAUDIO_3_VOLUME;
		SBYTE		mAUDIO_3_OUTPUT;
		ULONG		mAUDIO_3_INTEGRATE_ENABLE;
		ULONG		mAUDIO_3_WAVESHAPER;

		ULONG		mSTEREO;

		UBYTE		*mBitmapBits0;
		UBYTE		*mBitmapBits1;
		ULONG		mCurrentBuffer;
		UBYTE		*mRamPointer;

		ULONG		mScreenMode;
		ULONG		mScreenXsize;
		ULONG		mScreenYsize;
		ULONG		mImageXoffset;
		ULONG		mImageYoffset;

		//
		// Serial related variables
		//
		BOOL		mUART_RX_IRQ_ENABLE;
		BOOL		mUART_TX_IRQ_ENABLE;

		ULONG		mUART_RX_COUNTDOWN;
		ULONG		mUART_TX_COUNTDOWN;

		ULONG		mUART_DATA;
};


#endif
