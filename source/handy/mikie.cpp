//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Mikey chip emulation class                                               //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates all of the Mikey hardware with the exception of the  //
// CPU and memory selector. Update() does most of the work and does screen  //
// DMA and counter updates, it also schecules in which cycle the next timer //
// update will occur so that the CSystem->Update() doesnt have to call it   //
// every cycle, massive speedup but big complexity headache.                //
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

#define MIKIE_CPP

#include <stdlib.h>
#include "system.h"
#include "mikie.h"
#include "lynxdef.h"
#include "nds.h"


void CMikie::BlowOut(void)
{
//	CString addr;
	C6502_REGS regs;
	mSystem.GetRegs(regs);
//	addr.Format("CMikie::Poke() - Read/Write to counter clocks at PC=$%04x.",regs.PC);
//	::MessageBox(NULL,addr,"Runtime Error - System Halted", MB_OK | MB_ICONERROR);
	printf("CMikie::Poke() - Read/Write to counter clocks at PC=$%04x.\n",regs.PC);
	gSystemHalt = TRUE;
}


CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	int loop;
	for (loop=0;loop<16;loop++) mPalette[loop].Index = loop;

	Reset();
}


CMikie::~CMikie()
{
}


void CMikie::Reset(void)
{
	mAudioInputComparator = FALSE;	// Initialises to unknown
	mDisplayAddress = 0x00;			// Initialises to unknown
	mLynxLine = 0;
	mLynxLineDMACounter = 0;
	mLynxAddr = 0;

	mTimerStatusFlags = 0x00;		// Initialises to ZERO, i.e No IRQ's
	mTimerInterruptMask = 0x00;

	mRamPointer = mSystem.GetRamPointer();	// Fetch pointer to system RAM

	mCurrentBuffer = 0;

	mTIM_0_BKUP = 0;
	mTIM_0_ENABLE_RELOAD = 0;
	mTIM_0_ENABLE_COUNT = 0;
	mTIM_0_LINKING = 0;
	mTIM_0_CURRENT = 0;
	mTIM_0_TIMER_DONE = 0;
	mTIM_0_LAST_CLOCK = 0;
	mTIM_0_BORROW_IN = 0;
	mTIM_0_BORROW_OUT = 0;
	mTIM_0_LAST_LINK_CARRY = 0;
	mTIM_0_LAST_COUNT = 0;

	mTIM_1_BKUP = 0;
	mTIM_1_ENABLE_RELOAD = 0;
	mTIM_1_ENABLE_COUNT = 0;
	mTIM_1_LINKING = 0;
	mTIM_1_CURRENT = 0;
	mTIM_1_TIMER_DONE = 0;
	mTIM_1_LAST_CLOCK = 0;
	mTIM_1_BORROW_IN = 0;
	mTIM_1_BORROW_OUT = 0;
	mTIM_1_LAST_LINK_CARRY = 0;
	mTIM_1_LAST_COUNT = 0;

	mTIM_2_BKUP=0;
	mTIM_2_ENABLE_RELOAD=0;
	mTIM_2_ENABLE_COUNT=0;
	mTIM_2_LINKING=0;
	mTIM_2_CURRENT=0;
	mTIM_2_TIMER_DONE=0;
	mTIM_2_LAST_CLOCK=0;
	mTIM_2_BORROW_IN=0;
	mTIM_2_BORROW_OUT=0;
	mTIM_2_LAST_LINK_CARRY=0;
	mTIM_2_LAST_COUNT=0;

	mTIM_3_BKUP=0;
	mTIM_3_ENABLE_RELOAD=0;
	mTIM_3_ENABLE_COUNT=0;
	mTIM_3_LINKING=0;
	mTIM_3_CURRENT=0;
	mTIM_3_TIMER_DONE=0;
	mTIM_3_LAST_CLOCK=0;
	mTIM_3_BORROW_IN=0;
	mTIM_3_BORROW_OUT=0;
	mTIM_3_LAST_LINK_CARRY=0;
	mTIM_3_LAST_COUNT=0;

	mTIM_4_BKUP=0;
	mTIM_4_ENABLE_RELOAD=0;
	mTIM_4_ENABLE_COUNT=0;
	mTIM_4_LINKING=0;
	mTIM_4_CURRENT=0;
	mTIM_4_TIMER_DONE=0;
	mTIM_4_LAST_CLOCK=0;
	mTIM_4_BORROW_IN=0;
	mTIM_4_BORROW_OUT=0;
	mTIM_4_LAST_LINK_CARRY=0;
	mTIM_4_LAST_COUNT=0;

	mTIM_5_BKUP=0;
	mTIM_5_ENABLE_RELOAD=0;
	mTIM_5_ENABLE_COUNT=0;
	mTIM_5_LINKING=0;
	mTIM_5_CURRENT=0;
	mTIM_5_TIMER_DONE=0;
	mTIM_5_LAST_CLOCK=0;
	mTIM_5_BORROW_IN=0;
	mTIM_5_BORROW_OUT=0;
	mTIM_5_LAST_LINK_CARRY=0;
	mTIM_5_LAST_COUNT=0;

	mTIM_6_BKUP=0;
	mTIM_6_ENABLE_RELOAD=0;
	mTIM_6_ENABLE_COUNT=0;
	mTIM_6_LINKING=0;
	mTIM_6_CURRENT=0;
	mTIM_6_TIMER_DONE=0;
	mTIM_6_LAST_CLOCK=0;
	mTIM_6_BORROW_IN=0;
	mTIM_6_BORROW_OUT=0;
	mTIM_6_LAST_LINK_CARRY=0;
	mTIM_6_LAST_COUNT=0;

	mTIM_7_BKUP=0;
	mTIM_7_ENABLE_RELOAD=0;
	mTIM_7_ENABLE_COUNT=0;
	mTIM_7_LINKING=0;
	mTIM_7_CURRENT=0;
	mTIM_7_TIMER_DONE=0;
	mTIM_7_LAST_CLOCK=0;
	mTIM_7_BORROW_IN=0;
	mTIM_7_BORROW_OUT=0;
	mTIM_7_LAST_LINK_CARRY=0;
	mTIM_7_LAST_COUNT=0;

	mAUDIO_0_BKUP=0;
	mAUDIO_0_ENABLE_RELOAD=0;
	mAUDIO_0_ENABLE_COUNT=0;
	mAUDIO_0_LINKING=0;
	mAUDIO_0_CURRENT=0;
	mAUDIO_0_TIMER_DONE=0;
	mAUDIO_0_LAST_CLOCK=0;
	mAUDIO_0_BORROW_IN=0;
	mAUDIO_0_BORROW_OUT=0;
	mAUDIO_0_LAST_LINK_CARRY=0;
	mAUDIO_0_LAST_COUNT=0;
	mAUDIO_0_VOLUME=0;
	mAUDIO_0_OUTPUT=0;
	mAUDIO_0_INTEGRATE_ENABLE=0;
	mAUDIO_0_WAVESHAPER=0;

	mAUDIO_1_BKUP=0;
	mAUDIO_1_ENABLE_RELOAD=0;
	mAUDIO_1_ENABLE_COUNT=0;
	mAUDIO_1_LINKING=0;
	mAUDIO_1_CURRENT=0;
	mAUDIO_1_TIMER_DONE=0;
	mAUDIO_1_LAST_CLOCK=0;
	mAUDIO_1_BORROW_IN=0;
	mAUDIO_1_BORROW_OUT=0;
	mAUDIO_1_LAST_LINK_CARRY=0;
	mAUDIO_1_LAST_COUNT=0;
	mAUDIO_1_VOLUME=0;
	mAUDIO_1_OUTPUT=0;
	mAUDIO_1_INTEGRATE_ENABLE=0;
	mAUDIO_1_WAVESHAPER=0;

	mAUDIO_2_BKUP=0;
	mAUDIO_2_ENABLE_RELOAD=0;
	mAUDIO_2_ENABLE_COUNT=0;
	mAUDIO_2_LINKING=0;
	mAUDIO_2_CURRENT=0;
	mAUDIO_2_TIMER_DONE=0;
	mAUDIO_2_LAST_CLOCK=0;
	mAUDIO_2_BORROW_IN=0;
	mAUDIO_2_BORROW_OUT=0;
	mAUDIO_2_LAST_LINK_CARRY=0;
	mAUDIO_2_LAST_COUNT=0;
	mAUDIO_2_VOLUME=0;
	mAUDIO_2_OUTPUT=0;
	mAUDIO_2_INTEGRATE_ENABLE=0;
	mAUDIO_2_WAVESHAPER=0;

	mAUDIO_3_BKUP=0;
	mAUDIO_3_ENABLE_RELOAD=0;
	mAUDIO_3_ENABLE_COUNT=0;
	mAUDIO_3_LINKING=0;
	mAUDIO_3_CURRENT=0;
	mAUDIO_3_TIMER_DONE=0;
	mAUDIO_3_LAST_CLOCK=0;
	mAUDIO_3_BORROW_IN=0;
	mAUDIO_3_BORROW_OUT=0;
	mAUDIO_3_LAST_LINK_CARRY=0;
	mAUDIO_3_LAST_COUNT=0;
	mAUDIO_3_VOLUME=0;
	mAUDIO_3_OUTPUT=0;
	mAUDIO_3_INTEGRATE_ENABLE=0;
	mAUDIO_3_WAVESHAPER=0;

	mSTEREO = 0xff;	// All channels enabled

	// Start with an empty palette

	for (int loop=0;loop<16;loop++)
	{
		mPalette[loop].Index = loop;
	}

	// Initialise IODAT register

	mIODAT = 0x00;
	mIODIR = 0x00;
	mIODAT_REST_SIGNAL = 0x00;

	//
	// Initialise display control register vars
	//
	mDISPCTL_DMAEnable = FALSE;
	mDISPCTL_Flip = FALSE;
	mDISPCTL_FourColour = 0;
	mDISPCTL_Colour = 0;

	//
	// Initialise the UART variables
	//
	mUART_RX_IRQ_ENABLE = 0;
	mUART_TX_IRQ_ENABLE = 0;

	mUART_TX_COUNTDOWN = UART_TX_INACTIVE;
	mUART_RX_COUNTDOWN = UART_RX_INACTIVE;

	mUART_Rx_input_ptr = 0;
	mUART_Rx_output_ptr = 0;
	mUART_Rx_waiting = 0;
	mUART_Rx_framing_error = 0;
	mUART_Rx_overun_error = 0;

	mUART_SENDBREAK = 0;
	mUART_TX_DATA = 0;
	mUART_RX_DATA = 0;
	mUART_RX_READY = 0;

	mUART_PARITY_ENABLE = 0;
	mUART_PARITY_EVEN = 0;
}

ULONG CMikie::GetLfsrNext(ULONG current)
{
	// The table is built thus:
	//	Bits 0-11  LFSR					(12 Bits)
	//  Bits 12-20 Feedback switches	(9 Bits)
	//     (Order = 7,0,1,2,3,4,5,10,11)
	//  Order is mangled to make peek/poke easier as
	//  bit 7 is in a seperate register
	//
	// Total 21 bits = 2MWords @ 4 Bytes/Word = 8MB !!!!!
	//
	// If the index is a combination of Current LFSR+Feedback the
	// table will give the next value.

	static ULONG switches,lfsr,next,swloop,result;
	static const ULONG switchbits[9]={7,0,1,2,3,4,5,10,11};

	switches = current >> 12;
	lfsr = current & 0xfff;
	result = 0;
	for (swloop=0;swloop<9;swloop++) {
		if ((switches >> swloop) & 0x001) result ^= (lfsr >> switchbits[swloop]) & 0x001;
	}
	result = (result) ? 0 : 1;
	next = (switches << 12) | ((lfsr << 1) & 0xffe) | result;
	return next;
}

void CMikie::PresetForHomebrew(void)
{
	//
	// After all of that nice timer init we'll start timers running as some homebrew
	// i.e LR.O doesn't bother to setup the timers

	mTIM_0_BKUP = 0x9e;
	mTIM_0_ENABLE_RELOAD = TRUE;
	mTIM_0_ENABLE_COUNT = TRUE;

	mTIM_2_BKUP = 0x68;
	mTIM_2_ENABLE_RELOAD = TRUE;
	mTIM_2_ENABLE_COUNT = TRUE;
	mTIM_2_LINKING = 7;

	mDISPCTL_DMAEnable = TRUE;
	mDISPCTL_Flip = FALSE;
	mDISPCTL_FourColour = 0;
	mDISPCTL_Colour = TRUE;
}

void CMikie::ComLynxCable(int status)
{
	mUART_CABLE_PRESENT = status;
}

void CMikie::ComLynxRxData(int data)
{
	TRACE_MIKIE1("ComLynxRxData() - Received %04x", data);
	// Copy over the data
	if (mUART_Rx_waiting < UART_MAX_RX_QUEUE) {
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if (!mUART_Rx_waiting) mUART_RX_COUNTDOWN = UART_RX_TIME_PERIOD;

		// Receive the byte
		mUART_Rx_input_queue[mUART_Rx_input_ptr] = data;
		mUART_Rx_input_ptr = (mUART_Rx_input_ptr + 1) % UART_MAX_RX_QUEUE;
		mUART_Rx_waiting++;
		TRACE_MIKIE2("ComLynxRxData() - input ptr=%02d waiting=%02d", mUART_Rx_input_ptr, mUART_Rx_waiting);
	}
	else {
		TRACE_MIKIE0("ComLynxRxData() - UART RX Overun");
	}
}

void CMikie::ComLynxTxLoopback(int data)
{
	TRACE_MIKIE1("ComLynxTxLoopback() - Received %04x", data);

	if (mUART_Rx_waiting < UART_MAX_RX_QUEUE) {
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if (!mUART_Rx_waiting) mUART_RX_COUNTDOWN = UART_RX_TIME_PERIOD;

		// Receive the byte - INSERT into front of queue
		mUART_Rx_output_ptr = (mUART_Rx_output_ptr - 1) % UART_MAX_RX_QUEUE;
		mUART_Rx_input_queue[mUART_Rx_output_ptr] = data;
		mUART_Rx_waiting++;
		TRACE_MIKIE2("ComLynxTxLoopback() - input ptr=%02d waiting=%02d", mUART_Rx_input_ptr, mUART_Rx_waiting);
	}
	else {
		TRACE_MIKIE0("ComLynxTxLoopback() - UART RX Overun");
	}
}

void CMikie::ComLynxTxCallback(void (*function)(int data, ULONG objref), ULONG objref)
{
	mpUART_TX_CALLBACK = function;
	mUART_TX_CALLBACK_OBJECT = objref;
}

void CMikie::DisplaySetAttributes(void (*DisplayCallback)(void), void (*RenderCallback)(UBYTE *ram, ULONG *palette, bool flip))
{
	mpDisplayCallback = DisplayCallback;
	mpRenderCallback = RenderCallback;

	if (mpDisplayCallback) {
		(*mpDisplayCallback)();
	}

	// Reset screen related counters/vars
	mTIM_0_CURRENT = 0;
	mTIM_2_CURRENT = 0;

	// Fix lastcount so that timer update will definately occur
	mTIM_0_LAST_COUNT -= (1<<(4+mTIM_0_LINKING))+1;
	mTIM_2_LAST_COUNT -= (1<<(4+mTIM_2_LINKING))+1;

	// Force immediate timer update
	gNextTimerEvent = gSystemCycleCount;
}

ULONG CMikie::DisplayRenderLine(void)
{
	ULONG work_done = 0;

	// Set the timer interrupt flag
	if (mTimerInterruptMask & 0x01) {
		TRACE_MIKIE0("Update() - TIMER0 IRQ Triggered (Line Timer)");
		mTimerStatusFlags |= 0x01;
	}

	if (!mDISPCTL_DMAEnable) return 0;
//	if (mLynxLine & 0x80000000) return 0;

// Logic says it should be 101 but testing on an actual lynx shows the rest
// period is between lines 102,101,100 with the new line being latched at
// the beginning of count==99 hence the code below !!

	// Emulate REST signal
	if (mLynxLine == mTIM_2_BKUP-2 || mLynxLine == mTIM_2_BKUP-3 || mLynxLine == mTIM_2_BKUP-4)
		mIODAT_REST_SIGNAL = TRUE;
	else
		mIODAT_REST_SIGNAL = FALSE;

	if (mLynxLine == (mTIM_2_BKUP-3)) {
		if (mDISPCTL_Flip) {
			mLynxAddr = mDisplayAddress & 0xfffc;
			mLynxAddr += 3;
		}
		else {
			mLynxAddr = mDisplayAddress & 0xfffc;
		}
		// Trigger line rending to start
		mLynxLineDMACounter = 102;
	}

	// Decrement line counter logic
	if (mLynxLine) mLynxLine--;

	// Do 102 lines, nothing more, less is OK.
	if (mLynxLineDMACounter) {
//		TRACE_MIKIE1("Update() - Screen DMA, line %03d",line_count);
		mLynxLineDMACounter--;

		// Cycle hit for a 80 RAM access in rendering a line
		work_done += 80 * DMA_RDWR_CYC;

		// Mikie screen DMA can only see the system RAM....
		// (Step through bitmap, line at a time)

		if (mpRenderCallback) {
			(*mpRenderCallback)(&mRamPointer[mLynxAddr], (ULONG *)mPalette, mDISPCTL_Flip);
		}

		if (mDISPCTL_Flip) {
			mLynxAddr -= LYNX_SCREEN_WIDTH/2;
		}
		else {
			mLynxAddr += LYNX_SCREEN_WIDTH/2;
		}

	}
	return work_done;
}

ULONG CMikie::DisplayEndOfFrame(void)
{
	// Stop any further line rendering
	mLynxLineDMACounter = 0;
	mLynxLine = mTIM_2_BKUP;

	// Set the timer status flag
	if (mTimerInterruptMask & 0x04) {
		TRACE_MIKIE0("Update() - TIMER2 IRQ Triggered (Frame Timer)");
		mTimerStatusFlags |= 0x04;
	}

//	TRACE_MIKIE0("Update() - Frame end");
	// Trigger the callback to the display sub-system to render the
	// display and fetch the new pointer to be used for the lynx
	// display buffer for the forthcoming frame
	if (mpDisplayCallback) (*mpDisplayCallback)();

	return 0;
}

// Peek/Poke memory handlers

void CMikie::Poke(ULONG addr,UBYTE data)
{
	switch(addr & 0xff)
	{
		case (TIM0BKUP & 0xff):
			mTIM_0_BKUP = data;
			break;
		case (TIM1BKUP & 0xff):
			mTIM_1_BKUP = data;
			break;
		case (TIM2BKUP & 0xff):
			mTIM_2_BKUP = data;
			break;
		case (TIM3BKUP & 0xff):
			mTIM_3_BKUP = data;
			break;
		case (TIM4BKUP & 0xff):
			mTIM_4_BKUP = data;
			break;
		case (TIM5BKUP & 0xff):
			mTIM_5_BKUP = data;
			break;
		case (TIM6BKUP & 0xff):
			mTIM_6_BKUP = data;
			break;
		case (TIM7BKUP & 0xff):
			mTIM_7_BKUP = data;
			break;

		case (TIM0CTLA & 0xff):
			mTimerInterruptMask &= (0x01 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x01 : 0x00;
			mTIM_0_ENABLE_RELOAD = data & 0x10;
			mTIM_0_ENABLE_COUNT = data & 0x08;
			mTIM_0_LINKING = data & 0x07;
			if (data & 0x40) mTIM_0_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_0_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM1CTLA & 0xff):
			mTimerInterruptMask &= (0x02 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x02 : 0x00;
			mTIM_1_ENABLE_RELOAD = data & 0x10;
			mTIM_1_ENABLE_COUNT = data & 0x08;
			mTIM_1_LINKING = data & 0x07;
			if (data & 0x40) mTIM_1_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_1_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM2CTLA & 0xff):
			mTimerInterruptMask &= (0x04 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x04 : 0x00;
			mTIM_2_ENABLE_RELOAD = data & 0x10;
			mTIM_2_ENABLE_COUNT = data & 0x08;
			mTIM_2_LINKING = data & 0x07;
			if (data & 0x40) mTIM_2_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_2_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM3CTLA & 0xff):
			mTimerInterruptMask &= (0x08 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x08 : 0x00;
			mTIM_3_ENABLE_RELOAD = data & 0x10;
			mTIM_3_ENABLE_COUNT = data & 0x08;
			mTIM_3_LINKING = data & 0x07;
			if (data & 0x40) mTIM_3_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_3_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM4CTLA & 0xff):
			// Timer 4 can never generate interrupts as its timer output is used
			// to drive the UART clock generator
			mTIM_4_ENABLE_RELOAD = data & 0x10;
			mTIM_4_ENABLE_COUNT = data & 0x08;
			mTIM_4_LINKING = data & 0x07;
			if (data & 0x40) mTIM_4_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_4_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM5CTLA & 0xff):
			mTimerInterruptMask &= (0x20 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x20 : 0x00;
			mTIM_5_ENABLE_RELOAD = data & 0x10;
			mTIM_5_ENABLE_COUNT = data & 0x08;
			mTIM_5_LINKING = data & 0x07;
			if (data & 0x40) mTIM_5_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_5_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM6CTLA & 0xff):
			mTimerInterruptMask &= (0x40 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x40 : 0x00;
			mTIM_6_ENABLE_RELOAD = data & 0x10;
			mTIM_6_ENABLE_COUNT = data & 0x08;
			mTIM_6_LINKING = data & 0x07;
			if (data & 0x40) mTIM_6_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_6_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (TIM7CTLA & 0xff):
			mTimerInterruptMask &= (0x80 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x80 : 0x00;
			mTIM_7_ENABLE_RELOAD = data & 0x10;
			mTIM_7_ENABLE_COUNT = data & 0x08;
			mTIM_7_LINKING = data & 0x07;
			if (data & 0x40) mTIM_7_TIMER_DONE = 0;
			if (data & 0x48) {
				mTIM_7_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;

		case (TIM0CNT & 0xff):
			mTIM_0_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM1CNT & 0xff):
			mTIM_1_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM2CNT & 0xff):
			mTIM_2_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM3CNT & 0xff):
			mTIM_3_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM4CNT & 0xff):
			mTIM_4_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM5CNT & 0xff):
			mTIM_5_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM6CNT & 0xff):
			mTIM_6_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;
		case (TIM7CNT & 0xff):
			mTIM_7_CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			break;

		case (TIM0CTLB & 0xff):
			mTIM_0_TIMER_DONE = data & 0x08;
			mTIM_0_LAST_CLOCK = data & 0x04;
			mTIM_0_BORROW_IN = data & 0x02;
			mTIM_0_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM1CTLB & 0xff):
			mTIM_1_TIMER_DONE = data & 0x08;
			mTIM_1_LAST_CLOCK = data & 0x04;
			mTIM_1_BORROW_IN = data & 0x02;
			mTIM_1_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM2CTLB&0xff):
			mTIM_2_TIMER_DONE = data & 0x08;
			mTIM_2_LAST_CLOCK = data & 0x04;
			mTIM_2_BORROW_IN = data & 0x02;
			mTIM_2_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM3CTLB & 0xff):
			mTIM_3_TIMER_DONE = data & 0x08;
			mTIM_3_LAST_CLOCK = data & 0x04;
			mTIM_3_BORROW_IN = data & 0x02;
			mTIM_3_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM4CTLB & 0xff):
			mTIM_4_TIMER_DONE = data & 0x08;
			mTIM_4_LAST_CLOCK = data & 0x04;
			mTIM_4_BORROW_IN = data & 0x02;
			mTIM_4_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM5CTLB & 0xff):
			mTIM_5_TIMER_DONE= data & 0x08;
			mTIM_5_LAST_CLOCK= data & 0x04;
			mTIM_5_BORROW_IN = data & 0x02;
			mTIM_5_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM6CTLB & 0xff):
			mTIM_6_TIMER_DONE = data & 0x08;
			mTIM_6_LAST_CLOCK = data & 0x04;
			mTIM_6_BORROW_IN = data & 0x02;
			mTIM_6_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;
		case (TIM7CTLB & 0xff):
			mTIM_7_TIMER_DONE = data & 0x08;
			mTIM_7_LAST_CLOCK = data & 0x04;
			mTIM_7_BORROW_IN = data & 0x02;
			mTIM_7_BORROW_OUT = data & 0x01;
//			BlowOut();
			break;

		case (AUD0VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_0_VOLUME && data) {
				mAUDIO_0_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_0_VOLUME = (SBYTE)data;
			break;
		case (AUD0SHFTFB & 0xff):
			mAUDIO_0_WAVESHAPER &= 0x001fff;
			mAUDIO_0_WAVESHAPER |= (ULONG)data << 13;
			break;
		case (AUD0OUTVAL & 0xff):
			mAUDIO_0_OUTPUT = data;
			break;
		case (AUD0L8SHFT & 0xff):
			mAUDIO_0_WAVESHAPER &= 0x1fff00;
			mAUDIO_0_WAVESHAPER |= data;
			break;
		case (AUD0TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_0_BKUP && data) {
				mAUDIO_0_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_0_BKUP = data;
			break;
		case (AUD0CTL & 0xff):
			mAUDIO_0_ENABLE_RELOAD = data & 0x10;
			mAUDIO_0_ENABLE_COUNT = data & 0x08;
			mAUDIO_0_LINKING = data & 0x07;
			mAUDIO_0_INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_0_TIMER_DONE = 0;
			mAUDIO_0_WAVESHAPER &= 0x1fefff;
			mAUDIO_0_WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_0_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (AUD0COUNT & 0xff):
			mAUDIO_0_CURRENT = data;
			break;
		case (AUD0MISC & 0xff):
			mAUDIO_0_WAVESHAPER &= 0x1ff0ff;
			mAUDIO_0_WAVESHAPER |= (data & 0xf0)<<4;
			mAUDIO_0_BORROW_IN = data & 0x02;
			mAUDIO_0_BORROW_OUT = data & 0x01;
			mAUDIO_0_LAST_CLOCK = data & 0x04;
			break;

		case (AUD1VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_1_VOLUME && data)
			{
				mAUDIO_1_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_1_VOLUME = (SBYTE)data;
			break;
		case (AUD1SHFTFB & 0xff):
			mAUDIO_1_WAVESHAPER &= 0x001fff;
			mAUDIO_1_WAVESHAPER |= (ULONG)data<<13;
			break;
		case (AUD1OUTVAL & 0xff):
			mAUDIO_1_OUTPUT = data;
			break;
		case (AUD1L8SHFT & 0xff):
			mAUDIO_1_WAVESHAPER &= 0x1fff00;
			mAUDIO_1_WAVESHAPER |= data;
			break;
		case (AUD1TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_1_BKUP && data)
			{
				mAUDIO_1_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_1_BKUP = data;
			break;
		case (AUD1CTL & 0xff):
			mAUDIO_1_ENABLE_RELOAD = data & 0x10;
			mAUDIO_1_ENABLE_COUNT = data & 0x08;
			mAUDIO_1_LINKING = data & 0x07;
			mAUDIO_1_INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_1_TIMER_DONE = 0;
			mAUDIO_1_WAVESHAPER &= 0x1fefff;
			mAUDIO_1_WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_1_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (AUD1COUNT & 0xff):
			mAUDIO_1_CURRENT = data;
			break;
		case (AUD1MISC & 0xff):
			mAUDIO_1_WAVESHAPER &= 0x1ff0ff;
			mAUDIO_1_WAVESHAPER |= (data & 0xf0)<<4;
			mAUDIO_1_BORROW_IN = data & 0x02;
			mAUDIO_1_BORROW_OUT = data & 0x01;
			mAUDIO_1_LAST_CLOCK = data & 0x04;
			break;

		case (AUD2VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_2_VOLUME && data)
			{
				mAUDIO_2_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_2_VOLUME = (SBYTE)data;
			break;
		case (AUD2SHFTFB & 0xff):
			mAUDIO_2_WAVESHAPER &= 0x001fff;
			mAUDIO_2_WAVESHAPER |= (ULONG)data<<13;
			break;
		case (AUD2OUTVAL & 0xff):
			mAUDIO_2_OUTPUT = data;
			break;
		case (AUD2L8SHFT & 0xff):
			mAUDIO_2_WAVESHAPER &= 0x1fff00;
			mAUDIO_2_WAVESHAPER |= data;
			break;
		case (AUD2TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_2_BKUP && data) {
				mAUDIO_2_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_2_BKUP = data;
			break;
		case (AUD2CTL & 0xff):
			mAUDIO_2_ENABLE_RELOAD = data & 0x10;
			mAUDIO_2_ENABLE_COUNT = data & 0x08;
			mAUDIO_2_LINKING = data & 0x07;
			mAUDIO_2_INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_2_TIMER_DONE = 0;
			mAUDIO_2_WAVESHAPER &= 0x1fefff;
			mAUDIO_2_WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_2_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (AUD2COUNT & 0xff):
			mAUDIO_2_CURRENT = data;
			break;
		case (AUD2MISC & 0xff):
			mAUDIO_2_WAVESHAPER &= 0x1ff0ff;
			mAUDIO_2_WAVESHAPER|= (data & 0xf0)<<4;
			mAUDIO_2_BORROW_IN = data & 0x02;
			mAUDIO_2_BORROW_OUT = data & 0x01;
			mAUDIO_2_LAST_CLOCK = data & 0x04;
			break;

		case (AUD3VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_3_VOLUME && data) {
				mAUDIO_3_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_3_VOLUME = (SBYTE)data;
			break;
		case (AUD3SHFTFB & 0xff):
			mAUDIO_3_WAVESHAPER &= 0x001fff;
			mAUDIO_3_WAVESHAPER |= (ULONG)data<<13;
			break;
		case (AUD3OUTVAL & 0xff):
			mAUDIO_3_OUTPUT = data;
			break;
		case (AUD3L8SHFT & 0xff):
			mAUDIO_3_WAVESHAPER &= 0x1fff00;
			mAUDIO_3_WAVESHAPER |= data;
			break;
		case (AUD3TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_3_BKUP && data) {
				mAUDIO_3_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_3_BKUP = data;
			break;
		case (AUD3CTL & 0xff):
			mAUDIO_3_ENABLE_RELOAD = data & 0x10;
			mAUDIO_3_ENABLE_COUNT = data & 0x08;
			mAUDIO_3_LINKING = data & 0x07;
			mAUDIO_3_INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_3_TIMER_DONE = 0;
			mAUDIO_3_WAVESHAPER &= 0x1fefff;
			mAUDIO_3_WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_3_LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			break;
		case (AUD3COUNT & 0xff):
			mAUDIO_3_CURRENT = data;
			break;
		case (AUD3MISC & 0xff):
			mAUDIO_3_WAVESHAPER &= 0x1ff0ff;
			mAUDIO_3_WAVESHAPER |= (data & 0xf0)<<4;
			mAUDIO_3_BORROW_IN = data & 0x02;
			mAUDIO_3_BORROW_OUT = data & 0x01;
			mAUDIO_3_LAST_CLOCK = data & 0x04;
			break;

		case (ATTEN_A & 0xff):
		case (ATTEN_B & 0xff):
		case (ATTEN_C & 0xff):
		case (ATTEN_D & 0xff):
		case (MPAN & 0xff):
			break;

		case (MSTEREO & 0xff):
			data ^= 0xff;
//			if (!(mSTEREO & 0x11) && (data & 0x11))
//			{
//				mAUDIO_0_LAST_COUNT = gSystemCycleCount;
//				gNextTimerEvent = gSystemCycleCount;
//			}
//			if (!(mSTEREO & 0x22) && (data & 0x22))
//			{
//				mAUDIO_1_LAST_COUNT = gSystemCycleCount;
//				gNextTimerEvent = gSystemCycleCount;
//			}
//			if (!(mSTEREO & 0x44) && (data & 0x44))
//			{
//				mAUDIO_2_LAST_COUNT = gSystemCycleCount;
//				gNextTimerEvent = gSystemCycleCount;
//			}
//			if (!(mSTEREO & 0x88) && (data & 0x88))
//			{
//				mAUDIO_3_LAST_COUNT = gSystemCycleCount;
//				gNextTimerEvent = gSystemCycleCount;
//			}
			mSTEREO = data;
			break;

		case (INTRST & 0xff):
			mTimerStatusFlags &= ~data;
			gSystemIRQ = mTimerStatusFlags;
			gNextTimerEvent = gSystemCycleCount;
			break;

		case (INTSET & 0xff):
			mTimerStatusFlags |= data;
			gSystemIRQ = mTimerStatusFlags;
			gNextTimerEvent = gSystemCycleCount;
			break;

		case (SYSCTL1 & 0xff):
			if (!(data & 0x02)) {
//				CString addr;
				C6502_REGS regs;
				mSystem.GetRegs(regs);
//				addr.Format("CMikie::Poke(SYSCTL1) - Lynx power down occured at PC=$%04x.",regs.PC);
//				::MessageBox(NULL,addr,"Runtime Alert - System Halted", MB_OK | MB_ICONEXCLAMATION);
				printf("CMikie::Poke(SYSCTL1) - Lynx power down occured at PC=$%04x.\n",regs.PC);
				mSystem.Reset();
				gSystemHalt=TRUE;
			}
			mSystem.CartAddressStrobe((data & 0x01) ? TRUE : FALSE);
			break;

		case (MIKEYSREV & 0xff):
			break;

		case (IODIR & 0xff):
			mIODIR = data;
			break;

		case (IODAT & 0xff):
			mIODAT = data & (mIODIR ^ 0xff);
			mSystem.CartAddressData((data & 0x02) ? TRUE : FALSE);
			// Enable cart writes to BANK1 on AUDIN if AUDIN is set to output
			if (mIODIR & 0x10) mSystem.mCart->mWriteEnableBank1 = (mIODAT & 0x10) ? TRUE : FALSE;
			break;

		case (SERCTL & 0xff):
			TRACE_MIKIE2("Poke(SERCTL  ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			mUART_TX_IRQ_ENABLE = (data & 0x80) ? true : false;
			mUART_RX_IRQ_ENABLE = (data & 0x40) ? true : false;
			mUART_PARITY_ENABLE = (data & 0x10) ? true : false;
			mUART_SENDBREAK = data & 0x02;
			mUART_PARITY_EVEN = data & 0x01;

			// Reset all errors if required
			if (data & 0x08) {
				mUART_Rx_overun_error = 0;
				mUART_Rx_framing_error = 0;
			}

			if (mUART_SENDBREAK) {
				// Trigger send break, it will self sustain as long as sendbreak is set
				mUART_TX_COUNTDOWN = UART_TX_TIME_PERIOD;
				// Loop back what we transmitted
				ComLynxTxLoopback(UART_BREAK_CODE);
			}
			break;

		case (SERDAT & 0xff):
			//
			// Fake transmission, set counter to be decremented by Timer 4
			//
			// ComLynx only has one output pin, hence Rx & Tx are shorted
			// therefore any transmitted data will loopback
			//
			mUART_TX_DATA = data;
			// Calculate Parity data
			if (mUART_PARITY_ENABLE) {
				// Calc parity value
				// Leave at zero !!
			}
			else {
				// If disabled then the PAREVEN bit is sent
				if (mUART_PARITY_EVEN) data |= 0x0100;
			}
			// Set countdown to transmission
			mUART_TX_COUNTDOWN = UART_TX_TIME_PERIOD;
			// Loop back what we transmitted
			ComLynxTxLoopback(mUART_TX_DATA);
			break;

		case (SDONEACK & 0xff):
			break;
		case (CPUSLEEP & 0xff):
			TRACE_MIKIE2("Poke(CPUSLEEP,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			//
			// We must do "cycles_used" cycles of the system with the CPU sleeping
			// to compensate for the sprite painting, Mikie update will autowake the
			// CPU at the right time.
			//
			{
				TRACE_MIKIE0("*********************************************************");
				TRACE_MIKIE0("****               CPU SLEEP STARTED                 ****");
				TRACE_MIKIE0("*********************************************************");
				SLONG cycles_used = (SLONG)mSystem.PaintSprites();
				gCPUWakeupTime = gSystemCycleCount+cycles_used;
				SetCPUSleep();
				TRACE_MIKIE2("Poke(CPUSLEEP,%02x) wakeup at cycle =%012d", data, gCPUWakeupTime);
			}
			break;

		case (DISPCTL & 0xff):
			{
				TDISPCTL tmp;
				tmp.Byte = data;
				mDISPCTL_DMAEnable = tmp.Bits.DMAEnable;
				mDISPCTL_Flip = tmp.Bits.Flip;
				mDISPCTL_FourColour = tmp.Bits.FourColour;
				mDISPCTL_Colour = tmp.Bits.Colour;
			}
			break;
		case (PBKUP & 0xff):
			break;

		case (DISPADRL & 0xff):
			mDisplayAddress &= 0xff00;
			mDisplayAddress += data;
			break;

		case (DISPADRH & 0xff):
			mDisplayAddress &= 0x00ff;
			mDisplayAddress += (data<<8);
			break;

		case (Mtest0 & 0xff):
		case (Mtest1 & 0xff):
		case (Mtest2 & 0xff):
			// Test registers are unimplemented
			// lets hope no programs use them.
			break;

		case (GREEN0 & 0xff):
		case (GREEN1 & 0xff):
		case (GREEN2 & 0xff):
		case (GREEN3 & 0xff):
		case (GREEN4 & 0xff):
		case (GREEN5 & 0xff):
		case (GREEN6 & 0xff):
		case (GREEN7 & 0xff):
		case (GREEN8 & 0xff):
		case (GREEN9 & 0xff):
		case (GREENA & 0xff):
		case (GREENB & 0xff):
		case (GREENC & 0xff):
		case (GREEND & 0xff):
		case (GREENE & 0xff):
		case (GREENF & 0xff):
			mPalette[addr & 0x0f].Colours.Green = data & 0x0f;
			break;

		case (BLUERED0 & 0xff):
		case (BLUERED1 & 0xff):
		case (BLUERED2 & 0xff):
		case (BLUERED3 & 0xff):
		case (BLUERED4 & 0xff):
		case (BLUERED5 & 0xff):
		case (BLUERED6 & 0xff):
		case (BLUERED7 & 0xff):
		case (BLUERED8 & 0xff):
		case (BLUERED9 & 0xff):
		case (BLUEREDA & 0xff):
		case (BLUEREDB & 0xff):
		case (BLUEREDC & 0xff):
		case (BLUEREDD & 0xff):
		case (BLUEREDE & 0xff):
		case (BLUEREDF & 0xff):
			mPalette[addr & 0x0f].Colours.Blue = (data & 0xf0)>>4;
			mPalette[addr & 0x0f].Colours.Red = data & 0x0f;
			break;

// Errors on read only register accesses

		case (MAGRDY0 & 0xff):
		case (MAGRDY1 & 0xff):
		case (AUDIN & 0xff):
		case (MIKEYHREV & 0xff):
//			_RPT3(_CRT_WARN, "CMikie::Poke(%04x,%02x) - Poke to read only register location at time %d\n",addr,data,gSystemCycleCount);
			break;

// Errors on illegal location accesses

		default:
//			_RPT3(_CRT_WARN, "CMikie::Poke(%04x,%02x) - Poke to illegal location at time %d\n",addr,data,gSystemCycleCount);
			break;
	}
}



UBYTE CMikie::Peek(ULONG addr)
{
	switch(addr&0xff)
	{

// Timer control registers

		case (TIM0BKUP & 0xff):
			return (UBYTE)mTIM_0_BKUP;
			break;
		case (TIM1BKUP & 0xff):
			return (UBYTE)mTIM_1_BKUP;
			break;
		case (TIM2BKUP & 0xff):
			return (UBYTE)mTIM_2_BKUP;
			break;
		case (TIM3BKUP & 0xff):
			return (UBYTE)mTIM_3_BKUP;
			break;
		case (TIM4BKUP & 0xff):
			return (UBYTE)mTIM_4_BKUP;
			break;
		case (TIM5BKUP & 0xff):
			return (UBYTE)mTIM_5_BKUP;
			break;
		case (TIM6BKUP & 0xff):
			return (UBYTE)mTIM_6_BKUP;
			break;
		case (TIM7BKUP & 0xff):
			return (UBYTE)mTIM_7_BKUP;
			break;

		case (TIM0CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x01) ? 0x80 : 0x00;
				retval |= (mTIM_0_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_0_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_0_LINKING;
				return retval;
			}
			break;
		case (TIM1CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x02) ? 0x80 : 0x00;
				retval |= (mTIM_1_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_1_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_1_LINKING;
				return retval;
			}
			break;
		case (TIM2CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x04) ? 0x80 : 0x00;
				retval |= (mTIM_2_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_2_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_2_LINKING;
				return retval;
			}
			break;
		case (TIM3CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x08) ? 0x80 : 0x00;
				retval |= (mTIM_3_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_3_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_3_LINKING;
				return retval;
			}
			break;
		case (TIM4CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x10) ? 0x80 : 0x00;
				retval |= (mTIM_4_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_4_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_4_LINKING;
				return retval;
			}
			break;
		case (TIM5CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x20) ? 0x80 : 0x00;
				retval |= (mTIM_5_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_5_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_5_LINKING;
				return retval;
			}
			break;
		case (TIM6CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x40) ? 0x80 : 0x00;
				retval |= (mTIM_6_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_6_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_6_LINKING;
				return retval;
			}
			break;
		case (TIM7CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x80) ? 0x80 : 0x00;
				retval |= (mTIM_7_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_7_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_7_LINKING;
				return retval;
			}
			break;

		case (TIM0CNT & 0xff):
			Update();
			return (UBYTE)mTIM_0_CURRENT;
			break;
		case (TIM1CNT & 0xff):
			Update();
			return (UBYTE)mTIM_1_CURRENT;
			break;
		case (TIM2CNT & 0xff):
			Update();
			return (UBYTE)mTIM_2_CURRENT;
			break;
		case (TIM3CNT & 0xff):
			Update();
			return (UBYTE)mTIM_3_CURRENT;
			break;
		case (TIM4CNT & 0xff):
			Update();
			return (UBYTE)mTIM_4_CURRENT;
			break;
		case (TIM5CNT & 0xff):
			Update();
			return (UBYTE)mTIM_5_CURRENT;
			break;
		case (TIM6CNT & 0xff):
			Update();
			return (UBYTE)mTIM_6_CURRENT;
			break;
		case (TIM7CNT & 0xff):
			Update();
			return (UBYTE)mTIM_7_CURRENT;
			break;

		case (TIM0CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_0_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_0_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_0_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_0_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM1CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_1_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_1_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_1_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_1_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM2CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_2_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_2_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_2_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_2_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM3CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_3_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_3_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_3_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_3_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM4CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_4_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_4_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_4_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_4_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM5CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_5_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_5_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_5_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_5_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM6CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_6_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_6_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_6_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_6_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM7CTLB & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTIM_7_TIMER_DONE) ? 0x08 : 0x00;
				retval |= (mTIM_7_LAST_CLOCK) ? 0x04 : 0x00;
				retval |= (mTIM_7_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mTIM_7_BORROW_OUT) ? 0x01 : 0x00;
				return retval;
			}
//			BlowOut();
			break;

// Audio control registers

		case (AUD0VOL & 0xff):
			return (UBYTE)mAUDIO_0_VOLUME;
			break;
		case (AUD0SHFTFB & 0xff):
			return (UBYTE)((mAUDIO_0_WAVESHAPER >> 13) & 0xff);
			break;
		case (AUD0OUTVAL & 0xff):
			return (UBYTE)mAUDIO_0_OUTPUT;
			break;
		case (AUD0L8SHFT & 0xff):
			return (UBYTE)(mAUDIO_0_WAVESHAPER & 0xff);
			break;
		case (AUD0TBACK & 0xff):
			return (UBYTE)mAUDIO_0_BKUP;
			break;
		case (AUD0CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_0_INTEGRATE_ENABLE)? 0x20 : 0x00;
				retval |= (mAUDIO_0_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_0_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_0_WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_0_LINKING;
				return retval;
			}
			break;
		case (AUD0COUNT & 0xff):
			return (UBYTE)mAUDIO_0_CURRENT;
			break;
		case (AUD0MISC & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_0_BORROW_OUT) ? 0x01:0x00;
				retval |= (mAUDIO_0_BORROW_IN) ? 0x02:0x00;
				retval |= (mAUDIO_0_LAST_CLOCK) ? 0x08:0x00;
				retval |= (mAUDIO_0_WAVESHAPER >> 4) & 0xf0;
				return retval;
			}
			break;

		case (AUD1VOL & 0xff):
			return (UBYTE)mAUDIO_1_VOLUME;
			break;
		case (AUD1SHFTFB & 0xff):
			return (UBYTE)((mAUDIO_1_WAVESHAPER >> 13) & 0xff);
			break;
		case (AUD1OUTVAL & 0xff):
			return (UBYTE)mAUDIO_1_OUTPUT;
			break;
		case (AUD1L8SHFT & 0xff):
			return (UBYTE)(mAUDIO_1_WAVESHAPER & 0xff);
			break;
		case (AUD1TBACK & 0xff):
			return (UBYTE)mAUDIO_1_BKUP;
			break;
		case (AUD1CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_1_INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_1_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_1_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_1_WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_1_LINKING;
				return retval;
			}
			break;
		case (AUD1COUNT & 0xff):
			return (UBYTE)mAUDIO_1_CURRENT;
			break;
		case (AUD1MISC & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_1_BORROW_OUT) ? 0x01 : 0x00;
				retval |= (mAUDIO_1_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mAUDIO_1_LAST_CLOCK) ? 0x08 : 0x00;
				retval |= (mAUDIO_1_WAVESHAPER >> 4) & 0xf0;
				return retval;
			}
			break;

		case (AUD2VOL & 0xff):
			return (UBYTE)mAUDIO_2_VOLUME;
			break;
		case (AUD2SHFTFB & 0xff):
			return (UBYTE)((mAUDIO_2_WAVESHAPER >> 13) & 0xff);
			break;
		case (AUD2OUTVAL & 0xff):
			return (UBYTE)mAUDIO_2_OUTPUT;
			break;
		case (AUD2L8SHFT & 0xff):
			return (UBYTE)(mAUDIO_2_WAVESHAPER & 0xff);
			break;
		case (AUD2TBACK & 0xff):
			return (UBYTE)mAUDIO_2_BKUP;
			break;
		case (AUD2CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_2_INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_2_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_2_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_2_WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_2_LINKING;
				return retval;
			}
			break;
		case (AUD2COUNT & 0xff):
			return (UBYTE)mAUDIO_2_CURRENT;
			break;
		case (AUD2MISC & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_2_BORROW_OUT) ? 0x01 : 0x00;
				retval |= (mAUDIO_2_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mAUDIO_2_LAST_CLOCK) ? 0x08 : 0x00;
				retval |= (mAUDIO_2_WAVESHAPER >> 4) & 0xf0;
				return retval;
			}
			break;

		case (AUD3VOL & 0xff):
			return (UBYTE)mAUDIO_3_VOLUME;
			break;
		case (AUD3SHFTFB & 0xff):
			return (UBYTE)((mAUDIO_3_WAVESHAPER >> 13) & 0xff);
			break;
		case (AUD3OUTVAL & 0xff):
			return (UBYTE)mAUDIO_3_OUTPUT;
			break;
		case (AUD3L8SHFT & 0xff):
			return (UBYTE)(mAUDIO_3_WAVESHAPER & 0xff);
			break;
		case (AUD3TBACK & 0xff):
			return (UBYTE)mAUDIO_3_BKUP;
			break;
		case (AUD3CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_3_INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_3_ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_3_ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_3_WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_3_LINKING;
				return retval;
			}
			break;
		case (AUD3COUNT & 0xff):
			return (UBYTE)mAUDIO_3_CURRENT;
			break;
		case (AUD3MISC & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_3_BORROW_OUT) ? 0x01 : 0x00;
				retval |= (mAUDIO_3_BORROW_IN) ? 0x02 : 0x00;
				retval |= (mAUDIO_3_LAST_CLOCK) ? 0x08 : 0x00;
				retval |= (mAUDIO_3_WAVESHAPER >> 4) & 0xf0;
				return retval;
			}
			break;

		case (ATTEN_A & 0xff):
		case (ATTEN_B & 0xff):
		case (ATTEN_C & 0xff):
		case (ATTEN_D & 0xff):
		case (MPAN & 0xff):
			break;

		case (MSTEREO & 0xff):
			return (UBYTE) mSTEREO ^ 0xff;
			break;

// Miscellaneous registers

		case (SERCTL & 0xff):
			{
				ULONG retval = 0;
				retval |= (mUART_TX_COUNTDOWN&UART_TX_INACTIVE) ? 0xA0 : 0x00;	// Indicate TxDone & TxAllDone
				retval |= (mUART_RX_READY) ? 0x40 : 0x00;						// Indicate Rx data ready
				retval |= (mUART_Rx_overun_error) ? 0x08 : 0x0;					// Framing error
				retval |= (mUART_Rx_framing_error) ? 0x04 : 0x00;				// Rx overrun
				retval |= (mUART_RX_DATA&UART_BREAK_CODE) ? 0x02 : 0x00;		// Indicate break received
				retval |= (mUART_RX_DATA & 0x0100) ? 0x01 : 0x00;				// Add parity bit
				return (UBYTE)retval;
			}
			break;

		case (SERDAT&0xff):
			mUART_RX_COUNTDOWN=0xffffffff;		// This will clear the Rx data ready
			return (UBYTE)mUART_DATA;
			break;

		case (IODAT&0xff): 
			{
				ULONG retval = 0;
				retval |= (mIODIR&0x10) ? mIODAT & 0x10 : 0x10;									// IODIR  = output bit : input high (eeprom write done)
				retval |= (mIODIR&0x08) ? (((mIODAT & 0x08) && mIODAT_REST_SIGNAL) ? 0x00 : 0x08) : 0x00;									// REST   = output bit : input low
				retval |= (mIODIR&0x04) ? mIODAT & 0x04 : ((mUART_CABLE_PRESENT) ? 0x04 : 0x00);	// NOEXP  = output bit : input low
				retval |= (mIODIR&0x02) ? mIODAT & 0x02 : 0x00;									// CARTAD = output bit : input low
				retval |= (mIODIR&0x01) ? mIODAT & 0x01 : 0x01;									// EXTPW  = output bit : input high (Power connected)
				return (UBYTE)retval;
			}
			break;

		case (INTRST & 0xff):
		case (INTSET & 0xff):
			return (UBYTE)mTimerStatusFlags;
			break;

		case (MAGRDY0 & 0xff):
		case (MAGRDY1 & 0xff):
			return 0x00;
			break;

		case (AUDIN & 0xff):
			if (mAudioInputComparator) return 0x80; else return 0x00;
			break;

		case (MIKEYHREV & 0xff):
			return 0x01;
			break;

// Pallette registers

		case (GREEN0 & 0xff):
		case (GREEN1 & 0xff):
		case (GREEN2 & 0xff):
		case (GREEN3 & 0xff):
		case (GREEN4 & 0xff):
		case (GREEN5 & 0xff):
		case (GREEN6 & 0xff):
		case (GREEN7 & 0xff):
		case (GREEN8 & 0xff):
		case (GREEN9 & 0xff):
		case (GREENA & 0xff):
		case (GREENB & 0xff):
		case (GREENC & 0xff):
		case (GREEND & 0xff):
		case (GREENE & 0xff):
		case (GREENF & 0xff):
			return mPalette[addr & 0x0f].Colours.Green;
			break;

		case (BLUERED0 & 0xff):
		case (BLUERED1 & 0xff):
		case (BLUERED2 & 0xff):
		case (BLUERED3 & 0xff):
		case (BLUERED4 & 0xff):
		case (BLUERED5 & 0xff):
		case (BLUERED6 & 0xff):
		case (BLUERED7 & 0xff):
		case (BLUERED8 & 0xff):
		case (BLUERED9 & 0xff):
		case (BLUEREDA & 0xff):
		case (BLUEREDB & 0xff):
		case (BLUEREDC & 0xff):
		case (BLUEREDD & 0xff):
		case (BLUEREDE & 0xff):
		case (BLUEREDF & 0xff):
			return (mPalette[addr & 0x0f].Colours.Red | (mPalette[addr & 0x0f].Colours.Blue << 4));
			break;

// Errors on write only register accesses

		// For easier debugging

		case (DISPADRL & 0xff):
			return (UBYTE)(mDisplayAddress & 0xff);
		case (DISPADRH & 0xff):
			return (UBYTE)(mDisplayAddress >> 8) & 0xff;

		case (DISPCTL & 0xff):
		case (SYSCTL1 & 0xff):
		case (MIKEYSREV & 0xff):
		case (IODIR & 0xff):
		case (SDONEACK & 0xff):
		case (CPUSLEEP & 0xff):
		case (PBKUP & 0xff):
		case (Mtest0 & 0xff):
		case (Mtest1 & 0xff):
		case (Mtest2 & 0xff):
//			_RPT2(_CRT_WARN, "CMikie::Peek(%04x) - Peek from write only register location at time %d\n",addr,gSystemCycleCount);
			break;

// Register to let programs know handy is running

		case (0xfd97 & 0xff):
			return 0x42;
			break;

// Errors on illegal location accesses

		default:
//			_RPT2(_CRT_WARN, "CMikie::Peek(%04x) - Peek from illegal location at time %d\n",addr,gSystemCycleCount);
			break;
	}
	return 0xff;
}
