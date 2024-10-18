/////////////////////////////////////////////////////////////////////////////////
//                       Handy DS - An Atari Lynx Emulator                     //
//                                                                             //
//                          Based upon Handy/SDL v0.5                          //
//                             Copyright (c) 2005                              //
//                                SDLemu Team                                  //
//                          Based upon Handy v0.95 WIN32                       //
//                            Copyright (c) 1996,1997                          //
//                                  K. Wilkins                                 //
/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
// Copyright (c) 2004 SDLemu Team                                              //
//                                                                             //
// This software is provided 'as-is', without any express or implied warranty. //
// In no event will the authors be held liable for any damages arising from    //
// the use of this software.                                                   //
//                                                                             //
// Permission is granted to anyone to use this software for any purpose,       //
// including commercial applications, and to alter it and redistribute it      //
// freely, subject to the following restrictions:                              //
//                                                                             //
// 1. The origin of this software must not be misrepresented; you must not     //
//    claim that you wrote the original software. If you use this software     //
//    in a product, an acknowledgment in the product documentation would be    //
//    appreciated but is not required.                                         //
//                                                                             //
// 2. Altered source versions must be plainly marked as such, and must not     //
//    be misrepresented as being the original software.                        //
//                                                                             //
// 3. This notice may not be removed or altered from any source distribution.  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#define MIKIE_CPP

#include <stdlib.h>
#include "system.h"
#include "mikie.h"
#include "lynxdef.h"
#include "nds.h"

#define mTIM_0 mikey_0.timer0
#define mTIM_1 mikey_0.timer1
#define mTIM_2 mikey_0.timer2
#define mTIM_3 mikey_0.timer3
#define mTIM_4 mikey_0.timer4
#define mTIM_5 mikey_0.timer5
#define mTIM_6 mikey_0.timer6
#define mTIM_7 mikey_0.timer7


void CMikie::BlowOut(void)
{
	TRACE_MIKIE1("CMikie::Poke() - Read/Write to counter clocks at PC=$%04x.\n",mSystem.mCpu->GetPC());
	gSystemHalt = TRUE;
}

void CMikie::ResetTimer(MTIMER& timer)
{
	timer.ENABLE_RELOAD = 0;
	timer.ENABLE_COUNT = 0;
	timer.LINKING = 0;
	timer.CURRENT = 0;
	timer.CTLB = 0;
	timer.LAST_LINK_CARRY = 0;
	timer.LAST_COUNT = 0;
}

void CMikie::ResetAudio(MAUDIO& audio)
{
	audio.BKUP = 0;
	audio.ENABLE_RELOAD = 0;
	audio.ENABLE_COUNT = 0;
	audio.LINKING = 0;
	audio.CURRENT = 0;
	audio.CTLB = 0;
	audio.LAST_LINK_CARRY = 0;
	audio.LAST_COUNT = 0;
	audio.VOLUME = 0;
	audio.OUTPUT = 0;
	audio.INTEGRATE_ENABLE = 0;
	audio.WAVESHAPER = 0;
}

CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_MIKIE0("CMikie()");

	mpRamPointer = NULL;

	mpDisplayCallback = NULL;
	mpRenderCallback = NULL;

	mUART_CABLE_PRESENT = FALSE;
	mpUART_TX_CALLBACK = NULL;

	Reset();
}

CMikie::~CMikie()
{
	TRACE_MIKIE0("~CMikie()");
}

void CMikie::Reset(void)
{
	TRACE_MIKIE0("Reset()");

	mAudioInputComparator = FALSE;	// Initialises to unknown
	mLynxLine = 0;
	mLynxLineDMACounter = 0;
	mLynxAddr = 0;

	mTimerStatusFlags = 0x00;		// Initialises to ZERO, i.e No IRQ's
	mTimerInterruptMask = 0x00;

	mpRamPointer = mSystem.GetRamPointer();	// Fetch pointer to system RAM

	ResetTimer(mTIM_0);
	ResetTimer(mTIM_1);
	ResetTimer(mTIM_2);
	ResetTimer(mTIM_3);
	ResetTimer(mTIM_4);
	ResetTimer(mTIM_5);
	ResetTimer(mTIM_6);
	ResetTimer(mTIM_7);

	ResetAudio(mAUDIO_0);
	ResetAudio(mAUDIO_1);
	ResetAudio(mAUDIO_2);
	ResetAudio(mAUDIO_3);

	mSTEREO = 0xff;	// All channels enabled

	// Initialise IODAT register

	mikey_0.ioDat = 0x00;
	mikey_0.ioDir = 0x00;
	mIODAT_REST_SIGNAL = 0x00;

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
	TRACE_MIKIE0("PresetForHomebrew()");

	//
	// After all of that nice timer init we'll start timers running as some homebrew
	// i.e LR.O doesn't bother to setup the timers

	mikey_0.tim0Bkup = 0x9e;
	mTIM_0.ENABLE_RELOAD = TRUE;
	mTIM_0.ENABLE_COUNT = TRUE;

	mikey_0.tim2Bkup = 0x68;
	mTIM_2.ENABLE_RELOAD = TRUE;
	mTIM_2.ENABLE_COUNT = TRUE;
	mTIM_2.LINKING = 7;
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
}

ULONG CMikie::DisplayRenderLine(void)
{
	ULONG work_done = 0;

	// Set the timer interrupt flag
	if (mTimerInterruptMask & 0x01) {
		TRACE_MIKIE0("Update() - TIMER0 IRQ Triggered (Line Timer)");
		mTimerStatusFlags |= 0x01;
	}

	if (!(mikey_0.dispCtl & 1)) return 0;
//	if (mLynxLine & 0x80000000) return 0;

// Logic says it should be 101 but testing on an actual lynx shows the rest
// period is between lines 102,101,100 with the new line being latched at
// the beginning of count==99 hence the code below !!

	// Emulate REST signal
	u32 tim2 = mikey_0.tim2Bkup;
	if (mLynxLine == tim2-2 || mLynxLine == tim2-3 || mLynxLine == tim2-4)
		mIODAT_REST_SIGNAL = TRUE;
	else
		mIODAT_REST_SIGNAL = FALSE;

	if (mLynxLine == tim2-3) {
		mLynxAddr = mikey_0.dispAdr & 0xfffc;
		if (mikey_0.dispCtl & 2) {
			mLynxAddr += 3;
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
			(*mpRenderCallback)(&mpRamPointer[mLynxAddr], mikey_0.palette, mikey_0.dispCtl & 2);
		}

		if (mikey_0.dispCtl & 2) {
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
	mLynxLine = mikey_0.tim2Bkup;

	// Set the timer status flag
	if (mTimerInterruptMask & 0x04) {
		TRACE_MIKIE0("Update() - TIMER2 IRQ Triggered (Frame Timer)");
		mTimerStatusFlags |= 0x04;
	}

//	TRACE_MIKIE0("Update() - Frame end");
	// Trigger the callback to the display sub-system to render the
	// display.
	if (mpDisplayCallback) (*mpDisplayCallback)();

	return 0;
}

// Peek/Poke memory handlers

void CMikie::Poke(ULONG addr, UBYTE data)
{
	switch(addr & 0xff)
	{
		case (TIM0CTLA & 0xff):
			mTimerInterruptMask &= (0x01 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x01 : 0x00;
			mTIM_0.ENABLE_RELOAD = data & 0x10;
			mTIM_0.ENABLE_COUNT = data & 0x08;
			mTIM_0.LINKING = data & 0x07;
			if (data & 0x40) mTIM_0.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_0.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM0CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM1CTLA & 0xff):
			mTimerInterruptMask &= (0x02 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x02 : 0x00;
			mTIM_1.ENABLE_RELOAD = data & 0x10;
			mTIM_1.ENABLE_COUNT = data & 0x08;
			mTIM_1.LINKING = data & 0x07;
			if (data & 0x40) mTIM_1.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_1.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM1CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM2CTLA & 0xff):
			mTimerInterruptMask &= (0x04 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x04 : 0x00;
			mTIM_2.ENABLE_RELOAD = data & 0x10;
			mTIM_2.ENABLE_COUNT = data & 0x08;
			mTIM_2.LINKING = data & 0x07;
			if (data & 0x40) mTIM_2.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_2.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM2CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM3CTLA & 0xff):
			mTimerInterruptMask &= (0x08 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x08 : 0x00;
			mTIM_3.ENABLE_RELOAD = data & 0x10;
			mTIM_3.ENABLE_COUNT = data & 0x08;
			mTIM_3.LINKING = data & 0x07;
			if (data & 0x40) mTIM_3.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_3.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM3CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM4CTLA & 0xff):
			// Timer 4 can never generate interrupts as its timer output is used
			// to drive the UART clock generator
			mTIM_4.ENABLE_RELOAD = data & 0x10;
			mTIM_4.ENABLE_COUNT = data & 0x08;
			mTIM_4.LINKING = data & 0x07;
			if (data & 0x40) mTIM_4.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_4.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM4CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM5CTLA & 0xff):
			mTimerInterruptMask &= (0x20 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x20 : 0x00;
			mTIM_5.ENABLE_RELOAD = data & 0x10;
			mTIM_5.ENABLE_COUNT = data & 0x08;
			mTIM_5.LINKING = data & 0x07;
			if (data & 0x40) mTIM_5.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_5.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM5CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM6CTLA & 0xff):
			mTimerInterruptMask &= (0x40 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x40 : 0x00;
			mTIM_6.ENABLE_RELOAD = data & 0x10;
			mTIM_6.ENABLE_COUNT = data & 0x08;
			mTIM_6.LINKING = data & 0x07;
			if (data & 0x40) mTIM_6.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_6.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM6CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM7CTLA & 0xff):
			mTimerInterruptMask &= (0x80 ^ 0xff);
			mTimerInterruptMask |= (data & 0x80) ? 0x80 : 0x00;
			mTIM_7.ENABLE_RELOAD = data & 0x10;
			mTIM_7.ENABLE_COUNT = data & 0x08;
			mTIM_7.LINKING = data & 0x07;
			if (data & 0x40) mTIM_7.CTLB &= ~TIMER_DONE;
			if (data & 0x48) {
				mTIM_7.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM7CTLA,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (TIM0CNT & 0xff):
			mTIM_0.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM0CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM1CNT & 0xff):
			mTIM_1.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM1CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM2CNT & 0xff):
			mTIM_2.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM2CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM3CNT & 0xff):
			mTIM_3.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM3CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM4CNT & 0xff):
			mTIM_4.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM4CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM5CNT & 0xff):
			mTIM_5.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM5CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM6CNT & 0xff):
			mTIM_6.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM6CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM7CNT & 0xff):
			mTIM_7.CURRENT = data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM7CNT ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (TIM0CTLB & 0xff):
			mTIM_0.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM0CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM1CTLB & 0xff):
			mTIM_1.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM1CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM2CTLB & 0xff):
			mTIM_2.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM2CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM3CTLB & 0xff):
			mTIM_3.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM3CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM4CTLB & 0xff):
			mTIM_4.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM4CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM5CTLB & 0xff):
			mTIM_5.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM5CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM6CTLB & 0xff):
			mTIM_6.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM6CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (TIM7CTLB & 0xff):
			mTIM_7.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE);
			TRACE_MIKIE2("Poke(TIM7CTLB ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD0VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_0.VOLUME && data) {
				mAUDIO_0.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_0.VOLUME = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD0VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0SHFTFB & 0xff):
			mAUDIO_0.WAVESHAPER &= 0x001fff;
			mAUDIO_0.WAVESHAPER |= (ULONG)data << 13;
			TRACE_MIKIE2("Poke(AUD0SHFTB,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0OUTVAL & 0xff):
			mAUDIO_0.OUTPUT = data;
			TRACE_MIKIE2("Poke(AUD0OUTVAL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0L8SHFT & 0xff):
			mAUDIO_0.WAVESHAPER &= 0x1fff00;
			mAUDIO_0.WAVESHAPER |= data;
			TRACE_MIKIE2("Poke(AUD0L8SHFT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_0.BKUP && data) {
				mAUDIO_0.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_0.BKUP = data;
			TRACE_MIKIE2("Poke(AUD0TBACK,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0CTL & 0xff):
			mAUDIO_0.ENABLE_RELOAD = data & 0x10;
			mAUDIO_0.ENABLE_COUNT = data & 0x08;
			mAUDIO_0.LINKING = data & 0x07;
			mAUDIO_0.INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_0.CTLB &= ~TIMER_DONE;
			mAUDIO_0.WAVESHAPER &= 0x1fefff;
			mAUDIO_0.WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_0.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD0CTL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0COUNT & 0xff):
			mAUDIO_0.CURRENT = data;
			TRACE_MIKIE2("Poke(AUD0COUNT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD0MISC & 0xff):
			mAUDIO_0.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_0.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_0.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD0MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD1VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_1.VOLUME && data) {
				mAUDIO_1.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_1.VOLUME = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD1VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1SHFTFB & 0xff):
			mAUDIO_1.WAVESHAPER &= 0x001fff;
			mAUDIO_1.WAVESHAPER |= (ULONG)data << 13;
			TRACE_MIKIE2("Poke(AUD1SHFTFB,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1OUTVAL & 0xff):
			mAUDIO_1.OUTPUT = data;
			TRACE_MIKIE2("Poke(AUD1OUTVAL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1L8SHFT & 0xff):
			mAUDIO_1.WAVESHAPER &= 0x1fff00;
			mAUDIO_1.WAVESHAPER |= data;
			TRACE_MIKIE2("Poke(AUD1L8SHFT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_1.BKUP && data) {
				mAUDIO_1.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_1.BKUP = data;
			TRACE_MIKIE2("Poke(AUD1TBACK,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1CTL & 0xff):
			mAUDIO_1.ENABLE_RELOAD = data & 0x10;
			mAUDIO_1.ENABLE_COUNT = data & 0x08;
			mAUDIO_1.LINKING = data & 0x07;
			mAUDIO_1.INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_1.CTLB &= ~TIMER_DONE;
			mAUDIO_1.WAVESHAPER &= 0x1fefff;
			mAUDIO_1.WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_1.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD1CTL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1COUNT & 0xff):
			mAUDIO_1.CURRENT = data;
			TRACE_MIKIE2("Poke(AUD1COUNT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD1MISC & 0xff):
			mAUDIO_1.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_1.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_1.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD1MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD2VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_2.VOLUME && data) {
				mAUDIO_2.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_2.VOLUME = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD2VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2SHFTFB & 0xff):
			mAUDIO_2.WAVESHAPER &= 0x001fff;
			mAUDIO_2.WAVESHAPER |= (ULONG)data << 13;
			TRACE_MIKIE2("Poke(AUD2VSHFTFB,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2OUTVAL & 0xff):
			mAUDIO_2.OUTPUT = data;
			TRACE_MIKIE2("Poke(AUD2OUTVAL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2L8SHFT & 0xff):
			mAUDIO_2.WAVESHAPER &= 0x1fff00;
			mAUDIO_2.WAVESHAPER |= data;
			TRACE_MIKIE2("Poke(AUD2L8SHFT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_2.BKUP && data) {
				mAUDIO_2.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_2.BKUP = data;
			TRACE_MIKIE2("Poke(AUD2TBACK,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2CTL & 0xff):
			mAUDIO_2.ENABLE_RELOAD = data & 0x10;
			mAUDIO_2.ENABLE_COUNT = data & 0x08;
			mAUDIO_2.LINKING = data & 0x07;
			mAUDIO_2.INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_2.CTLB &= ~TIMER_DONE;
			mAUDIO_2.WAVESHAPER &= 0x1fefff;
			mAUDIO_2.WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_2.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD2CTL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2COUNT & 0xff):
			mAUDIO_2.CURRENT = data;
			TRACE_MIKIE2("Poke(AUD2COUNT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD2MISC & 0xff):
			mAUDIO_2.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_2.WAVESHAPER |= (data&0xf0) << 4;
			mAUDIO_2.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD2MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD3VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mAUDIO_3.VOLUME && data) {
				mAUDIO_3.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_3.VOLUME = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD3VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3SHFTFB & 0xff):
			mAUDIO_3.WAVESHAPER &= 0x001fff;
			mAUDIO_3.WAVESHAPER |= (ULONG)data << 13;
			TRACE_MIKIE2("Poke(AUD3SHFTFB,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3OUTVAL & 0xff):
			mAUDIO_3.OUTPUT = data;
			TRACE_MIKIE2("Poke(AUD3OUTVAL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3L8SHFT & 0xff):
			mAUDIO_3.WAVESHAPER &= 0x1fff00;
			mAUDIO_3.WAVESHAPER |= data;
			TRACE_MIKIE2("Poke(AUD3L8SHFT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3TBACK & 0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if (!mAUDIO_3.BKUP && data) {
				mAUDIO_3.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mAUDIO_3.BKUP = data;
			TRACE_MIKIE2("Poke(AUD3TBACK,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3CTL & 0xff):
			mAUDIO_3.ENABLE_RELOAD = data & 0x10;
			mAUDIO_3.ENABLE_COUNT = data & 0x08;
			mAUDIO_3.LINKING = data & 0x07;
			mAUDIO_3.INTEGRATE_ENABLE = data & 0x20;
			if (data & 0x40) mAUDIO_3.CTLB &= ~TIMER_DONE;
			mAUDIO_3.WAVESHAPER &= 0x1fefff;
			mAUDIO_3.WAVESHAPER |= (data & 0x80) ? 0x001000 : 0x000000;
			if (data & 0x48) {
				mAUDIO_3.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD3CTL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3COUNT & 0xff):
			mAUDIO_3.CURRENT = data;
			TRACE_MIKIE2("Poke(AUD3COUNT,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;
		case (AUD3MISC & 0xff):
			mAUDIO_3.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_3.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_3.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD3MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (MSTEREO & 0xff):
			data ^= 0xff;
			mSTEREO = data;
			TRACE_MIKIE2("Poke(MSTEREO,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (INTRST & 0xff):
			mTimerStatusFlags &= ~data;
			gNextTimerEvent = gSystemCycleCount;
			TRACE_MIKIE2("Poke(INTRST  ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (INTSET & 0xff):
			TRACE_MIKIE2("Poke(INTSET  ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			mTimerStatusFlags |= data;
			gNextTimerEvent = gSystemCycleCount;
			break;

		case (SYSCTL1 & 0xff):
			TRACE_MIKIE2("Poke(SYSCTL1 ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			if (!(data & 0x02)) {
				TRACE_MIKIE1("CMikie::Poke(SYSCTL1) - Lynx power down occured at PC=$%04x.\n",mSystem.mCpu->GetPC());
				mSystem.Reset();
				gSystemHalt = TRUE;
			}
			mSystem.CartAddressStrobe((data & 0x01) ? TRUE : FALSE);
			break;

		case (IODAT & 0xff):
			TRACE_MIKIE2("Poke(IODAT   ,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			mikey_0.ioDat = data;
			mSystem.CartAddressData((data & 0x02) ? TRUE : FALSE);
			// Enable cart writes to BANK1 on AUDIN if AUDIN is set to output
			if (mikey_0.ioDir & 0x10) mSystem.mCart->mWriteEnableBank1 = (data & 0x10) ? TRUE : FALSE;
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
			TRACE_MIKIE2("Poke(SERDAT ,%04x) at PC=%04x", data, mSystem.mCpu->GetPC());
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

		case (CPUSLEEP & 0xff):
			gSuzieDoneTime = gSystemCycleCount+mSystem.PaintSprites();
			SetCPUSleep();
			TRACE_MIKIE3("Poke(CPUSLEEP,%02x) at PC=%04x, wakeup at cycle =%012d", data, mSystem.mCpu->GetPC(), gSuzieDoneTime);
			break;

		case (TIM0BKUP & 0xff):
		case (TIM1BKUP & 0xff):
		case (TIM2BKUP & 0xff):
		case (TIM3BKUP & 0xff):
		case (TIM4BKUP & 0xff):
		case (TIM5BKUP & 0xff):
		case (TIM6BKUP & 0xff):
		case (TIM7BKUP & 0xff):
		case (ATTEN_A & 0xff):
		case (ATTEN_B & 0xff):
		case (ATTEN_C & 0xff):
		case (ATTEN_D & 0xff):
		case (MPAN & 0xff):
		case (MIKEYSREV & 0xff):
		case (IODIR & 0xff):
		case (SDONEACK & 0xff):
		case (DISPCTL & 0xff):
		case (PBKUP & 0xff):
		case (DISPADRL & 0xff):
		case (DISPADRH & 0xff):
		case (Mtest0 & 0xff):
		case (Mtest1 & 0xff):
		case (Mtest2 & 0xff):
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

// Errors on read only register accesses

		case (MAGRDY0 & 0xff):
		case (MAGRDY1 & 0xff):
		case (AUDIN & 0xff):
		case (MIKEYHREV & 0xff):

// Errors on illegal location accesses

		default:
			lnxWriteMikey(addr, data);
			break;
	}
}

UBYTE CMikie::Peek(ULONG addr)
{
	switch(addr & 0xff)
	{

// Timer control registers
		case (TIM0CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x01) ? 0x80 : 0x00;
				retval |= (mTIM_0.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_0.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_0.LINKING;
				TRACE_MIKIE2("Peek(TIM0CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM1CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x02) ? 0x80 : 0x00;
				retval |= (mTIM_1.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_1.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_1.LINKING;
				TRACE_MIKIE2("Peek(TIM1CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM2CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x04) ? 0x80 : 0x00;
				retval |= (mTIM_2.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_2.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_2.LINKING;
				TRACE_MIKIE2("Peek(TIM2CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM3CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x08) ? 0x80 : 0x00;
				retval |= (mTIM_3.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_3.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_3.LINKING;
				TRACE_MIKIE2("Peek(TIM3CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM4CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x10) ? 0x80 : 0x00;
				retval |= (mTIM_4.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_4.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_4.LINKING;
				TRACE_MIKIE2("Peek(TIM4CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM5CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x20) ? 0x80 : 0x00;
				retval |= (mTIM_5.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_5.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_5.LINKING;
				TRACE_MIKIE2("Peek(TIM5CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM6CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x40) ? 0x80 : 0x00;
				retval |= (mTIM_6.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_6.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_6.LINKING;
				TRACE_MIKIE2("Peek(TIM6CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM7CTLA & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mTimerInterruptMask & 0x80) ?0x80 : 0x00;
				retval |= (mTIM_7.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mTIM_7.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= mTIM_7.LINKING;
				TRACE_MIKIE2("Peek(TIM7CTLA ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (TIM0CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM0CNT  ,%02x) at PC=%04x", mTIM_0.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_0.CURRENT;
		case (TIM1CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM1CNT  ,%02x) at PC=%04x", mTIM_1.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_1.CURRENT;
		case (TIM2CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM2CNT  ,%02x) at PC=%04x", mTIM_2_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_2.CURRENT;
		case (TIM3CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM3CNT  ,%02x) at PC=%04x", mTIM_3_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_3.CURRENT;
		case (TIM4CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM4CNT  ,%02x) at PC=%04x", mTIM_4_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_4.CURRENT;
		case (TIM5CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM5CNT  ,%02x) at PC=%04x", mTIM_5_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_5.CURRENT;
		case (TIM6CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM6CNT  ,%02x) at PC=%04x", mTIM_6_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_6.CURRENT;
		case (TIM7CNT & 0xff):
			Update();
			TRACE_MIKIE2("Peek(TIM7CNT  ,%02x) at PC=%04x", mTIM_7_CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_7.CURRENT;

		case (TIM0CTLB & 0xff):
			{
				UBYTE retval = (mTIM_0.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM0CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM1CTLB & 0xff):
			{
				UBYTE retval = (mTIM_1.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM1CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM2CTLB & 0xff):
			{
				UBYTE retval = (mTIM_2.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM2CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM3CTLB & 0xff):
			{
				UBYTE retval = (mTIM_3.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM3CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM4CTLB & 0xff):
			{
				UBYTE retval = (mTIM_4.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM4CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM5CTLB & 0xff):
			{
				UBYTE retval = (mTIM_5.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM5CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM6CTLB & 0xff):
			{
				UBYTE retval = (mTIM_6.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM6CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM7CTLB & 0xff):
			{
				UBYTE retval = (mTIM_7.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				TRACE_MIKIE2("Peek(TIM7CTLB ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

// Audio control registers

		case (AUD0VOL & 0xff):
			TRACE_MIKIE2("Peek(AUD0VOL,%02x) at PC=%04x", (UBYTE)mAUDIO_0.VOLUME, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0.VOLUME;
		case (AUD0SHFTFB & 0xff):
			TRACE_MIKIE2("Peek(AUD0SHFTFB,%02x) at PC=%04x", (UBYTE)(mAUDIO_0.WAVESHAPER >> 13) & 0xff, mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_0.WAVESHAPER >> 13) & 0xff);
		case (AUD0OUTVAL & 0xff):
			TRACE_MIKIE2("Peek(AUD0OUTVAL,%02x) at PC=%04x", (UBYTE)mAUDIO_0.OUTPUT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0.OUTPUT;
		case (AUD0L8SHFT & 0xff):
			TRACE_MIKIE2("Peek(AUD0L8SHFT,%02x) at PC=%04x", (UBYTE)(mAUDIO_0.WAVESHAPER & 0xff), mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_0.WAVESHAPER&0xff);
		case (AUD0TBACK & 0xff):
			TRACE_MIKIE2("Peek(AUD0TBACK,%02x) at PC=%04x", (UBYTE)mAUDIO_0.BKUP, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0.BKUP;
		case (AUD0CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_0.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_0.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_0.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_0.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_0.LINKING;
				TRACE_MIKIE2("Peek(AUD0CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD0COUNT & 0xff):
			TRACE_MIKIE2("Peek(AUD0COUNT,%02x) at PC=%04x", (UBYTE)mAUDIO_0.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0.CURRENT;
		case (AUD0MISC & 0xff):
			{
				UBYTE retval = (mAUDIO_0.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				retval |= (mAUDIO_0.WAVESHAPER >> 4) & 0xf0;
				TRACE_MIKIE2("Peek(AUD0MISC,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD1VOL & 0xff):
			TRACE_MIKIE2("Peek(AUD1VOL,%02x) at PC=%04x", (UBYTE)mAUDIO_1.VOLUME, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1.VOLUME;
		case (AUD1SHFTFB & 0xff):
			TRACE_MIKIE2("Peek(AUD1SHFTFB,%02x) at PC=%04x", (UBYTE)(mAUDIO_1.WAVESHAPER >> 13) & 0xff, mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_1.WAVESHAPER >> 13) & 0xff);
		case (AUD1OUTVAL & 0xff):
			TRACE_MIKIE2("Peek(AUD1OUTVAL,%02x) at PC=%04x", (UBYTE)mAUDIO_1.OUTPUT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1.OUTPUT;
		case (AUD1L8SHFT & 0xff):
			TRACE_MIKIE2("Peek(AUD1L8SHFT,%02x) at PC=%04x", (UBYTE)(mAUDIO_1.WAVESHAPER & 0xff), mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_1.WAVESHAPER & 0xff);
		case (AUD1TBACK & 0xff):
			TRACE_MIKIE2("Peek(AUD1TBACK,%02x) at PC=%04x", (UBYTE)mAUDIO_1.BKUP, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1.BKUP;
		case (AUD1CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_1.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_1.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_1.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_1.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_1.LINKING;
				TRACE_MIKIE2("Peek(AUD1CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD1COUNT & 0xff):
			TRACE_MIKIE2("Peek(AUD1COUNT,%02x) at PC=%04x", (UBYTE)mAUDIO_1.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1.CURRENT;
		case (AUD1MISC & 0xff):
			{
				UBYTE retval = (mAUDIO_1.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				retval |= (mAUDIO_1.WAVESHAPER >> 4) & 0xf0;
				TRACE_MIKIE2("Peek(AUD1MISC,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD2VOL & 0xff):
			TRACE_MIKIE2("Peek(AUD2VOL,%02x) at PC=%04x", (UBYTE)mAUDIO_2.VOLUME, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2.VOLUME;
		case (AUD2SHFTFB & 0xff):
			TRACE_MIKIE2("Peek(AUD2SHFTFB,%02x) at PC=%04x", (UBYTE)(mAUDIO_2.WAVESHAPER >> 13) & 0xff, mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_2.WAVESHAPER >> 13) & 0xff);
		case (AUD2OUTVAL & 0xff):
			TRACE_MIKIE2("Peek(AUD2OUTVAL,%02x) at PC=%04x", (UBYTE)mAUDIO_2.OUTPUT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2.OUTPUT;
		case (AUD2L8SHFT&0xff):
			TRACE_MIKIE2("Peek(AUD2L8SHFT,%02x) at PC=%04x", (UBYTE)(mAUDIO_2.WAVESHAPER & 0xff), mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_2.WAVESHAPER & 0xff);
		case (AUD2TBACK & 0xff):
			TRACE_MIKIE2("Peek(AUD2TBACK,%02x) at PC=%04x", (UBYTE)mAUDIO_2.BKUP, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2.BKUP;
		case (AUD2CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_2.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_2.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_2.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_2.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_2.LINKING;
				TRACE_MIKIE2("Peek(AUD2CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD2COUNT & 0xff):
			TRACE_MIKIE2("Peek(AUD2COUNT,%02x) at PC=%04x", (UBYTE)mAUDIO_2.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2.CURRENT;
		case (AUD2MISC & 0xff):
			{
				UBYTE retval = (mAUDIO_2.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				retval |= (mAUDIO_2.WAVESHAPER >> 4) & 0xf0;
				TRACE_MIKIE2("Peek(AUD2MISC,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD3VOL & 0xff):
			TRACE_MIKIE2("Peek(AUD3VOL,%02x) at PC=%04x", (UBYTE)mAUDIO_3.VOLUME, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3.VOLUME;
		case (AUD3SHFTFB & 0xff):
			TRACE_MIKIE2("Peek(AUD3SHFTFB,%02x) at PC=%04x", (UBYTE)(mAUDIO_3.WAVESHAPER >> 13) & 0xff, mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_3.WAVESHAPER >> 13) & 0xff);
		case (AUD3OUTVAL & 0xff):
			TRACE_MIKIE2("Peek(AUD3OUTVAL,%02x) at PC=%04x", (UBYTE)mAUDIO_3.OUTPUT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3.OUTPUT;
		case (AUD3L8SHFT & 0xff):
			TRACE_MIKIE2("Peek(AUD3L8SHFT,%02x) at PC=%04x", (UBYTE)(mAUDIO_3.WAVESHAPER & 0xff), mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_3.WAVESHAPER & 0xff);
		case (AUD3TBACK & 0xff):
			TRACE_MIKIE2("Peek(AUD3TBACK,%02x) at PC=%04x", (UBYTE)mAUDIO_3.BKUP, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3.BKUP;
		case (AUD3CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_3.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mAUDIO_3.ENABLE_RELOAD) ? 0x10 : 0x00;
				retval |= (mAUDIO_3.ENABLE_COUNT) ? 0x08 : 0x00;
				retval |= (mAUDIO_3.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= mAUDIO_3.LINKING;
				TRACE_MIKIE2("Peek(AUD3CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD3COUNT & 0xff):
			TRACE_MIKIE2("Peek(AUD3COUNT,%02x) at PC=%04x", (UBYTE)mAUDIO_3.CURRENT, mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3.CURRENT;
		case (AUD3MISC & 0xff):
			{
				UBYTE retval = (mAUDIO_3.CTLB & (BORROW_OUT | BORROW_IN | LAST_CLOCK | TIMER_DONE));
				retval |= (mAUDIO_3.WAVESHAPER >> 4) & 0xf0;
				TRACE_MIKIE2("Peek(AUD3MISC,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (MSTEREO & 0xff):
			TRACE_MIKIE2("Peek(MSTEREO,%02x) at PC=%04x", (UBYTE)mSTEREO ^ 0xff, mSystem.mCpu->GetPC());
			return (UBYTE) mSTEREO ^ 0xff;

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
				TRACE_MIKIE2("Peek(SERCTL  ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return (UBYTE)retval;
			}
			break;

		case (SERDAT & 0xff):
			mUART_RX_READY = 0;
			TRACE_MIKIE2("Peek(SERDAT  ,%02x) at PC=%04x", (UBYTE)mUART_RX_DATA, mSystem.mCpu->GetPC());
			return (UBYTE)(mUART_RX_DATA & 0xff);

		case (IODAT & 0xff):
			{
				ULONG retval = 0;
				retval |= (mikey_0.ioDir&0x10) ? mikey_0.ioDat & 0x10 : 0x10;									// IODIR  = output bit : input high (eeprom write done)
				retval |= (mikey_0.ioDir&0x08) ? (((mikey_0.ioDat & 0x08) && mIODAT_REST_SIGNAL) ? 0x00 : 0x08) : 0x00;									// REST   = output bit : input low
				retval |= (mikey_0.ioDir&0x04) ? mikey_0.ioDat & 0x04 : ((mUART_CABLE_PRESENT) ? 0x04 : 0x00);	// NOEXP  = output bit : input low
				retval |= (mikey_0.ioDir&0x02) ? mikey_0.ioDat & 0x02 : 0x00;									// CARTAD = output bit : input low
				retval |= (mikey_0.ioDir&0x01) ? mikey_0.ioDat & 0x01 : 0x01;									// EXTPW  = output bit : input high (Power connected)
				TRACE_MIKIE2("Peek(IODAT   ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return (UBYTE)retval;
			}
			break;

		case (INTRST & 0xff):
		case (INTSET & 0xff):
			TRACE_MIKIE2("Peek(INTSET  ,%02x) at PC=%04x", mTimerStatusFlags, mSystem.mCpu->GetPC());
			return (UBYTE)mTimerStatusFlags;

		case (TIM0BKUP & 0xff):
		case (TIM1BKUP & 0xff):
		case (TIM2BKUP & 0xff):
		case (TIM3BKUP & 0xff):
		case (TIM4BKUP & 0xff):
		case (TIM5BKUP & 0xff):
		case (TIM6BKUP & 0xff):
		case (TIM7BKUP & 0xff):

		case (MAGRDY0 & 0xff):
		case (MAGRDY1 & 0xff):
		case (AUDIN & 0xff):
//			if (mAudioInputComparator) return 0x80; else return 0x00;
		case (ATTEN_A & 0xff):
		case (ATTEN_B & 0xff):
		case (ATTEN_C & 0xff):
		case (ATTEN_D & 0xff):
		case (MPAN & 0xff):

		case (MIKEYHREV & 0xff):

// Palette registers

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

		// For easier debugging

		case (DISPADRL & 0xff):
		case (DISPADRH & 0xff):

		// Errors on write only register accesses

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
		// Register to let programs know handy is running
		case (0xfd97 & 0xff):

// Errors on illegal location accesses
		default:
			return lnxReadMikey(addr);
			break;
	}
	return lnxReadMikey(addr);
}

void CMikie::Update(void)
{
	int divide = 0;
	int decval = 0;
	ULONG tmp;
	ULONG mikie_work_done=0;

	//
	// To stop problems with cycle count wrap we will check and then correct the
	// cycle counter.
	//

	if (gSystemCycleCount > 0xf0000000) {
		gSystemCycleCount -= 0x80000000;
		gAudioLastUpdateCycle -= 0x80000000;
		mTIM_0.LAST_COUNT -= 0x80000000;
		mTIM_1.LAST_COUNT -= 0x80000000;
		mTIM_2.LAST_COUNT -= 0x80000000;
		mTIM_3.LAST_COUNT -= 0x80000000;
		mTIM_4.LAST_COUNT -= 0x80000000;
		mTIM_5.LAST_COUNT -= 0x80000000;
		mTIM_6.LAST_COUNT -= 0x80000000;
		mTIM_7.LAST_COUNT -= 0x80000000;
		mAUDIO_0.LAST_COUNT -= 0x80000000;
		mAUDIO_1.LAST_COUNT -= 0x80000000;
		mAUDIO_2.LAST_COUNT -= 0x80000000;
		mAUDIO_3.LAST_COUNT -= 0x80000000;
		// Only correct if sleep is active
		if (gSuzieDoneTime) {
			gSuzieDoneTime -= 0x80000000;
		}
	}

	gNextTimerEvent = 0xffffffff;

	//
	// Check if the CPU needs to be woken up from sleep mode
	//
	if (gSuzieDoneTime) {
		if (gSystemCycleCount >= gSuzieDoneTime) {
			ClearCPUSleep();
			gSuzieDoneTime=0;
		}
		else {
			if (gSuzieDoneTime > gSystemCycleCount) gNextTimerEvent = gSuzieDoneTime;
		}
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

	//
	// Timer 0 of Group A
	//

	//
	// Optimisation, assume T0 (Line timer) is never in one-shot,
	// never placed in link mode
	//

	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
//	if (mTIM_0.ENABLE_COUNT && (mTIM_0.ENABLE_RELOAD || !(mTIM_0.CTLB & TIMER_DONE)))
	if (mTIM_0.ENABLE_COUNT) {
		// Timer 0 has no linking
//		if (mTIM_0.LINKING != 0x07)
		{
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4+mTIM_0.LINKING);
			decval = (gSystemCycleCount - mTIM_0.LAST_COUNT) >> divide;

			if (decval) {
				mTIM_0.LAST_COUNT += decval << divide;
				mTIM_0.CURRENT -= decval;

				if (mTIM_0.CURRENT & 0x80000000) {
					// Set carry out
					mTIM_0.CTLB |= BORROW_OUT;

//					// Reload if neccessary
//					if (mTIM_0.ENABLE_RELOAD) {
						mTIM_0.CURRENT += mikey_0.tim0Bkup+1;
//					}
//					else {
//						mTIM_0.CURRENT = 0;
//					}

					mTIM_0.CTLB |=  TIMER_DONE;

					// Interupt flag setting code moved into DisplayRenderLine()

					// Line timer has expired, render a line, we cannot incrememnt
					// the global counter at this point as it will screw the other timers
					// so we save under work done and inc at the end.
					mikie_work_done += DisplayRenderLine();

				}
				else {
					mTIM_0.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mTIM_0.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mTIM_0.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}

			// Prediction for next timer event cycle number

			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_0.CURRENT & 0x80000000) ? 1 : ((mTIM_0.CURRENT+1)<<divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
//				TRACE_MIKIE1("Update() - TIMER 0 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
	}

	//
	// Timer 2 of Group A
	//

	//
	// Optimisation, assume T2 (Frame timer) is never in one-shot
	// always in linked mode i.e clocked by Line Timer
	//

	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
//	if (mTIM_2.ENABLE_COUNT && (mTIM_2.ENABLE_RELOAD || !(mTIM_2.CTLB & TIMER_DONE)))
	if (mTIM_2.ENABLE_COUNT) {
		decval = 0;

//		if (mTIM_2.LINKING == 0x07)
		{
			if (mTIM_0.CTLB & BORROW_OUT) decval = 1;
			mTIM_2.LAST_LINK_CARRY = (mTIM_0.CTLB & BORROW_OUT);
		}
//		else {
//			// Ordinary clocked mode as opposed to linked mode
//			// 16MHz clock downto 1us == cyclecount >> 4
//			divide = (4 + mTIM_2.LINKING);
//			decval = (gSystemCycleCount - mTIM_2.LAST_COUNT) >> divide;
//		}

		if (decval) {
//			mTIM_2.LAST_COUNT += decval << divide;
			mTIM_2.CURRENT -= decval;
			if (mTIM_2.CURRENT & 0x80000000) {
				// Set carry out
				mTIM_2.CTLB |= BORROW_OUT;

//				// Reload if neccessary
//				if (mTIM_2.ENABLE_RELOAD) {
					mTIM_2.CURRENT += mikey_0.tim2Bkup+1;
//				}
//				else {
//					mTIM_2.CURRENT = 0;
//				}
				mTIM_2.CTLB |= TIMER_DONE;

				// Interupt flag setting code moved into DisplayEndOfFrame(), also
				// park any CPU cycles lost for later inclusion
				mikie_work_done += DisplayEndOfFrame();
			}
			else {
				mTIM_2.CTLB &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mTIM_2.CTLB |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mTIM_2.CTLB &= ~(BORROW_OUT | BORROW_IN);
		}

		// Prediction for next timer event cycle number
// We dont need to predict this as its the frame timer and will always
// be beaten by the line timer on Timer 0
//		if (mTIM_2.LINKING != 7) {
//			tmp = gSystemCycleCount + ((mTIM_2.CURRENT + 1) << divide);
//			if (tmp < gNextTimerEvent) gNextTimerEvent = tmp;
//		}
//		TRACE_MIKIE1("Update() - mTIM_2_CURRENT = %012d", mTIM_2.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_2_BKUP    = %012d", mikey_0.tim2Bkup);
//		TRACE_MIKIE1("Update() - mTIM_2_LASTCNT = %012d", mTIM_2.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_2_LINKING = %012d", mTIM_2.LINKING);
	}

	//
	// Timer 4 of Group A
	//
	// For the sake of speed it is assumed that Timer 4 (UART timer)
	// never uses one-shot mode, never uses linking, hence the code
	// is commented out. Timer 4 is at the end of a chain and seems
	// no reason to update its carry in-out variables
	//

	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
//	if (mTIM_4.ENABLE_COUNT && (mTIM_4.ENABLE_RELOAD || !(mTIM_4.CTLB & TIMER_DONE)))
	if (mTIM_4.ENABLE_COUNT) {
		decval = 0;

//		if (mTIM_4.LINKING == 0x07) {
//			if (mTIM_2.CTLB & BORROW_OUT) && !mTIM_4.LAST_LINK_CARRY) decval = 1;
//			if (mTIM_2.CTLB & BORROW_OUT) decval = 1;
//			mTIM_4.LAST_LINK_CARRY = (mTIM_2.CTLB & BORROW_OUT);
//		}
//		else
		{
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			// Additional /8 (+3) for 8 clocks per bit transmit
			divide = 4 + 3 + mTIM_4.LINKING;
			decval = (gSystemCycleCount - mTIM_4.LAST_COUNT) >> divide;
		}

		if (decval) {
			mTIM_4.LAST_COUNT += decval << divide;
			mTIM_4.CURRENT -= decval;
			if (mTIM_4.CURRENT & 0x80000000) {
				// Set carry out
				mTIM_4.CTLB |= BORROW_OUT;

				//
				// Update the UART counter models for Rx & Tx
				//

				//
				// According to the docs IRQ's are level triggered and hence will always assert
				// what a pain in the arse
				//
				// Rx & Tx are loopedback due to comlynx structure

				//
				// Receive
				//
				if (!mUART_RX_COUNTDOWN) {
					// Fetch a byte from the input queue
					if (mUART_Rx_waiting > 0) {
						mUART_RX_DATA = mUART_Rx_input_queue[mUART_Rx_output_ptr];
						mUART_Rx_output_ptr = (mUART_Rx_output_ptr + 1) % UART_MAX_RX_QUEUE;
						mUART_Rx_waiting--;
						TRACE_MIKIE2("Update() - RX Byte output ptr=%02d waiting=%02d", mUART_Rx_output_ptr, mUART_Rx_waiting);
					}
					else {
						TRACE_MIKIE0("Update() - RX Byte but no data waiting ????");
					}

					// Retrigger input if more bytes waiting
					if (mUART_Rx_waiting > 0) {
						mUART_RX_COUNTDOWN = UART_RX_TIME_PERIOD + UART_RX_NEXT_DELAY;
						TRACE_MIKIE1("Update() - RX Byte retriggered, %d waiting", mUART_Rx_waiting);
					}
					else {
						mUART_RX_COUNTDOWN = UART_RX_INACTIVE;
						TRACE_MIKIE0("Update() - RX Byte nothing waiting, deactivated");
					}

					// If RX_READY already set then we have an overrun
					// as previous byte hasnt been read
					if (mUART_RX_READY) mUART_Rx_overun_error = 1;

					// Flag byte as being recvd
					mUART_RX_READY = 1;
				}
				else if(!(mUART_RX_COUNTDOWN&UART_RX_INACTIVE)) {
					mUART_RX_COUNTDOWN--;
				}

				if (!mUART_TX_COUNTDOWN) {
					if (mUART_SENDBREAK) {
						mUART_TX_DATA = UART_BREAK_CODE;
						// Auto-Respawn new transmit
						mUART_TX_COUNTDOWN = UART_TX_TIME_PERIOD;
						// Loop back what we transmitted
						ComLynxTxLoopback(mUART_TX_DATA);
					}
					else {
						// Serial activity finished
						mUART_TX_COUNTDOWN = UART_TX_INACTIVE;
					}

					// If a networking object is attached then use its callback to send the data byte.
					if (mpUART_TX_CALLBACK) {
						TRACE_MIKIE0("Update() - UART_TX_CALLBACK");
						(*mpUART_TX_CALLBACK)(mUART_TX_DATA, mUART_TX_CALLBACK_OBJECT);
					}

				}
				else if (!(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)) {
					mUART_TX_COUNTDOWN--;
				}

				// Set the timer status flag
				// Timer 4 is the uart timer and doesn't generate IRQ's using this method

				// 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters

				// Reload if neccessary
//				if (mTIM_4.ENABLE_RELOAD) {
					mTIM_4.CURRENT += mikey_0.tim4Bkup + 1;
					// The low reload values on TIM4 coupled with a longer
					// timer service delay can sometimes cause
					// an underun, check and fix
					if (mTIM_4.CURRENT & 0x80000000) {
						mTIM_4.CURRENT = mikey_0.tim4Bkup;
						mTIM_4.LAST_COUNT = gSystemCycleCount;
					}
//				}
//				else {
//					mTIM_4.CURRENT = 0;
//				}
//				mTIM_4.CTLB |= TIMER_DONE;
			}
//			else {
//				mTIM_4.CTLB &= ~BORROW_OUT;
//			}
//			// Set carry in as we did a count
//			mTIM_4.BORROW_IN = TRUE;
		}
//		else {
//			// Clear carry in as we didn't count
//			mTIM_4.BORROW_IN = FALSE;
//			// Clear carry out
//			mTIM_4.CTLB &= ~BORROW_OUT;
//		}
//
//		// Prediction for next timer event cycle number
//
//		if (mTIM_4.LINKING != 7) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_4.CURRENT&0x80000000)?1:((mTIM_4.CURRENT+1)<<divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 4 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
//		}
//		TRACE_MIKIE1("Update() - mTIM_4_CURRENT = %012d", mTIM_4.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_4_BKUP    = %012d", mikey_0.tim4Bkup);
//		TRACE_MIKIE1("Update() - mTIM_4_LASTCNT = %012d", mTIM_4.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_4_LINKING = %012d", mTIM_4.LINKING);
	}

	// Emulate the UART bug where UART IRQ is level sensitive
	// in that it will continue to generate interrupts as long
	// as they are enabled and the interrupt condition is true

	// If Tx is inactive i.e ready for a byte to eat and the
	// IRQ is enabled then generate it always
	if ((mUART_TX_COUNTDOWN & UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
		mTimerStatusFlags |= 0x10;
	}
	// Is data waiting and the interrupt enabled, if so then
	// what are we waiting for....
	if (mUART_RX_READY && mUART_RX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
		mTimerStatusFlags |= 0x10;
	}

	//
	// Timer 1 of Group B
	//
	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
	if (mTIM_1.ENABLE_COUNT && (mTIM_1.ENABLE_RELOAD || !(mTIM_1.CTLB & TIMER_DONE))) {
		if (mTIM_1.LINKING != 0x07) {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + mTIM_1.LINKING);
			decval = (gSystemCycleCount - mTIM_1.LAST_COUNT) >> divide;

			if (decval) {
				mTIM_1.LAST_COUNT += decval << divide;
				mTIM_1.CURRENT -= decval;
				if (mTIM_1.CURRENT & 0x80000000) {
					// Set carry out
					mTIM_1.CTLB |= BORROW_OUT;

					// Set the timer status flag
					if (mTimerInterruptMask & 0x02) {
						TRACE_MIKIE0("Update() - TIMER1 IRQ Triggered");
						mTimerStatusFlags |= 0x02;
					}

					// Reload if neccessary
					if (mTIM_1.ENABLE_RELOAD) {
						mTIM_1.CURRENT += mikey_0.tim1Bkup + 1;
					}
					else {
						mTIM_1.CURRENT = 0;
					}
					mTIM_1.CTLB |= TIMER_DONE;
				}
				else {
					mTIM_1.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mTIM_1.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mTIM_1.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}
		}

		// Prediction for next timer event cycle number

		if (mTIM_1.LINKING != 7) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_1.CURRENT & 0x80000000) ? 1 : ((mTIM_1.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 1 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
//		TRACE_MIKIE1("Update() - mTIM_1_CURRENT = %012d", mTIM_1.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_1_BKUP    = %012d", mikey_0.tim1Bkup);
//		TRACE_MIKIE1("Update() - mTIM_1_LASTCNT = %012d", mTIM_1.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_1_LINKING = %012d", mTIM_1.LINKING);
	}

	//
	// Timer 3 of Group A
	//
	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
	if (mTIM_3.ENABLE_COUNT && (mTIM_3.ENABLE_RELOAD || !(mTIM_3.CTLB & TIMER_DONE))) {
		decval = 0;

		if (mTIM_3.LINKING == 0x07) {
			if (mTIM_1.CTLB & BORROW_OUT) decval = 1;
			mTIM_3.LAST_LINK_CARRY = (mTIM_1.CTLB & BORROW_OUT);
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + mTIM_3.LINKING);
			decval = (gSystemCycleCount - mTIM_3.LAST_COUNT) >> divide;
		}

		if (decval) {
			mTIM_3.LAST_COUNT += decval << divide;
			mTIM_3.CURRENT -= decval;
			if (mTIM_3.CURRENT & 0x80000000) {
				// Set carry out
				mTIM_3.CTLB |= BORROW_OUT;

				// Set the timer status flag
				if (mTimerInterruptMask & 0x08) {
					TRACE_MIKIE0("Update() - TIMER3 IRQ Triggered");
					mTimerStatusFlags |= 0x08;
				}

				// Reload if neccessary
				if (mTIM_3.ENABLE_RELOAD) {
					mTIM_3.CURRENT += mikey_0.tim3Bkup + 1;
				}
				else {
					mTIM_3.CURRENT = 0;
				}
				mTIM_3.CTLB |= TIMER_DONE;
			}
			else {
				mTIM_3.CTLB &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mTIM_3.CTLB |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mTIM_3.CTLB &= ~(BORROW_OUT | BORROW_IN);
		}

		// Prediction for next timer event cycle number

		if (mTIM_3.LINKING != 7) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_3.CURRENT & 0x80000000) ? 1 : ((mTIM_3.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 3 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
//		TRACE_MIKIE1("Update() - mTIM_3_CURRENT = %012d", mTIM_3.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_3_BKUP    = %012d", mikey_0.tim3Bkup);
//		TRACE_MIKIE1("Update() - mTIM_3_LASTCNT = %012d", mTIM_3.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_3_LINKING = %012d", mTIM_3.LINKING);
	}

	//
	// Timer 5 of Group A
	//
	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
	if (mTIM_5.ENABLE_COUNT && (mTIM_5.ENABLE_RELOAD || !(mTIM_5.CTLB & TIMER_DONE))) {
		decval = 0;

		if (mTIM_5.LINKING == 0x07) {
			if (mTIM_3.CTLB & BORROW_OUT) decval = 1;
			mTIM_5.LAST_LINK_CARRY = (mTIM_3.CTLB & BORROW_OUT);
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + mTIM_5.LINKING);
			decval = (gSystemCycleCount - mTIM_5.LAST_COUNT) >> divide;
		}

		if (decval) {
			mTIM_5.LAST_COUNT += decval<<divide;
			mTIM_5.CURRENT -= decval;
			if (mTIM_5.CURRENT & 0x80000000) {
				// Set carry out
				mTIM_5.CTLB |= BORROW_OUT;

				// Set the timer status flag
				if (mTimerInterruptMask & 0x20) {
					TRACE_MIKIE0("Update() - TIMER5 IRQ Triggered");
					mTimerStatusFlags |= 0x20;
				}

				// Reload if neccessary
				if (mTIM_5.ENABLE_RELOAD) {
					mTIM_5.CURRENT += mikey_0.tim5Bkup + 1;
				}
				else {
					mTIM_5.CURRENT = 0;
				}
				mTIM_5.CTLB |= TIMER_DONE;
			}
			else {
				mTIM_5.CTLB &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mTIM_5.CTLB |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mTIM_5.CTLB &= ~(BORROW_OUT | BORROW_IN);
		}

		// Prediction for next timer event cycle number

		if (mTIM_5.LINKING != 7) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_5.CURRENT & 0x80000000) ? 1 : ((mTIM_5.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 5 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
//		TRACE_MIKIE1("Update() - mTIM_5_CURRENT = %012d", mTIM_5.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_5_BKUP    = %012d", mikey_0.tim5Bkup);
//		TRACE_MIKIE1("Update() - mTIM_5_LASTCNT = %012d", mTIM_5.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_5_LINKING = %012d", mTIM_5.LINKING);
	}

	//
	// Timer 7 of Group A
	//
	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
	if (mTIM_7.ENABLE_COUNT && (mTIM_7.ENABLE_RELOAD || !(mTIM_7.CTLB & TIMER_DONE))) {
		decval = 0;

		if (mTIM_7.LINKING == 0x07) {
			if (mTIM_5.CTLB & BORROW_OUT) decval = 1;
			mTIM_7.LAST_LINK_CARRY = (mTIM_5.CTLB & BORROW_OUT);
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + mTIM_7.LINKING);
			decval = (gSystemCycleCount - mTIM_7.LAST_COUNT) >> divide;
		}

		if (decval) {
			mTIM_7.LAST_COUNT += decval << divide;
			mTIM_7.CURRENT -= decval;
			if (mTIM_7.CURRENT & 0x80000000) {
				// Set carry out
				mTIM_7.CTLB |= BORROW_OUT;

				// Set the timer status flag
				if (mTimerInterruptMask & 0x80) {
					TRACE_MIKIE0("Update() - TIMER7 IRQ Triggered");
					mTimerStatusFlags |= 0x80;
				}

				// Reload if neccessary
				if (mTIM_7.ENABLE_RELOAD) {
					mTIM_7.CURRENT += mikey_0.tim7Bkup + 1;
				}
				else {
					mTIM_7.CURRENT = 0;
				}
				mTIM_7.CTLB |= TIMER_DONE;
			}
			else {
				mTIM_7.CTLB &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mTIM_7.CTLB |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mTIM_7.CTLB &= ~(BORROW_OUT | BORROW_IN);
		}

		// Prediction for next timer event cycle number

		if (mTIM_7.LINKING != 7) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_7.CURRENT & 0x80000000) ? 1 : ((mTIM_7.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 7 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
//		TRACE_MIKIE1("Update() - mTIM_7_CURRENT = %012d", mTIM_7.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_7_BKUP    = %012d", mikey_0.tim7Bkup);
//		TRACE_MIKIE1("Update() - mTIM_7_LASTCNT = %012d", mTIM_7.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_7_LINKING = %012d", mTIM_7.LINKING);
	}

	//
	// Timer 6 has no group
	//
	// KW bugfix 13/4/99 added (mTIM_x.ENABLE_RELOAD ||  ..)
	if (mTIM_6.ENABLE_COUNT && (mTIM_6.ENABLE_RELOAD || !(mTIM_6.CTLB & TIMER_DONE))) {
//		if (mTIM_6.LINKING != 0x07)
		{
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + mTIM_6.LINKING);
			decval = (gSystemCycleCount - mTIM_6.LAST_COUNT) >> divide;

			if (decval) {
				mTIM_6.LAST_COUNT += decval << divide;
				mTIM_6.CURRENT -= decval;
				if (mTIM_6.CURRENT & 0x80000000) {
					// Set carry out
					mTIM_6.CTLB |= BORROW_OUT;

					// Set the timer status flag
					if (mTimerInterruptMask & 0x40) {
						TRACE_MIKIE0("Update() - TIMER6 IRQ Triggered");
						mTimerStatusFlags |= 0x40;
					}

					// Reload if neccessary
					if (mTIM_6.ENABLE_RELOAD) {
						mTIM_6.CURRENT += mikey_0.tim6Bkup + 1;
					}
					else {
						mTIM_6.CURRENT = 0;
					}
					mTIM_6.CTLB |= TIMER_DONE;
				}
				else {
					mTIM_6.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mTIM_6.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mTIM_6.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}
		}

		// Prediction for next timer event cycle number
		// (Timer 6 doesn't support linking)

//		if (mTIM_6.LINKING != 7)
		{
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_6.CURRENT & 0x80000000) ? 1 : ((mTIM_6.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 6 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
		}
//		TRACE_MIKIE1("Update() - mTIM_6_CURRENT = %012d", mTIM_6.CURRENT);
//		TRACE_MIKIE1("Update() - mTIM_6_BKUP    = %012d", mikey_0.tim6Bkup);
//		TRACE_MIKIE1("Update() - mTIM_6_LASTCNT = %012d", mTIM_6.LAST_COUNT);
//		TRACE_MIKIE1("Update() - mTIM_6_LINKING = %012d", mTIM_6.LINKING);
	}

	//
	// If sound is enabled then update the sound subsystem
	//
	if (gAudioEnabled) {
//		static SLONG sample = 0;
		// ULONG mix = 0; // unused

		//
		// Catch audio buffer up to current time
		//

		// Mix the sample

		/*
		sample = 0;
		if (mSTEREO & 0x11) { sample += mAUDIO_0.OUTPUT; mix++; }
		if (mSTEREO & 0x22) { sample += mAUDIO_1.OUTPUT; mix++; }
		if (mSTEREO & 0x44) { sample += mAUDIO_2.OUTPUT; mix++; }
		if (mSTEREO & 0x88) { sample += mAUDIO_3.OUTPUT; mix++; }
		if (mix) {
			sample += 128 * mix; // Correct for sign
			sample /= mix;	// Keep the audio volume at max
		}
		else {
			sample = 128;
		}

//		sample += (mSTEREO & 0x11) ? mAUDIO_0.OUTPUT : 0;
//		sample += (mSTEREO & 0x22) ? mAUDIO_1.OUTPUT : 0;
//		sample += (mSTEREO & 0x44) ? mAUDIO_2.OUTPUT : 0;
//		sample += (mSTEREO & 0x88) ? mAUDIO_3.OUTPUT : 0;
//		sample = sample >> 2;
//		sample += 128;
		*/

		for (;gAudioLastUpdateCycle+HANDY_AUDIO_SAMPLE_PERIOD<gSystemCycleCount;gAudioLastUpdateCycle+=HANDY_AUDIO_SAMPLE_PERIOD) {
			// Output audio sample
//			gAudioBuffer[gAudioBufferPointer++] = (UBYTE)sample;
			gAudioBuffer0[gAudioBufferPointer] = (mSTEREO & 0x11) ? mAUDIO_0.OUTPUT : 0;
			gAudioBuffer1[gAudioBufferPointer] = (mSTEREO & 0x22) ? mAUDIO_1.OUTPUT : 0;
			gAudioBuffer2[gAudioBufferPointer] = (mSTEREO & 0x33) ? mAUDIO_2.OUTPUT : 0;
			gAudioBuffer3[gAudioBufferPointer++] = (mSTEREO & 0x44) ? mAUDIO_3.OUTPUT : 0;

			// Check buffer overflow condition, stick at the endpoint
			// teh audio output system will reset the input pointer
			// when it reads out the data.

			// We should NEVER overflow, this buffer holds 0.25 seconds
			// of data if this happens the the multimedia system above
			// has failed so the corruption of the buffer contents wont matter

			gAudioBufferPointer%=HANDY_AUDIO_BUFFER_SIZE;
		}

		//
		// Audio 0
		//
//		if (mAUDIO_0.ENABLE_COUNT && !(mAUDIO_0.CTLB & TIMER_DONE) && mAUDIO_0.VOLUME && mAUDIO_0.BKUP)
		if (mAUDIO_0.ENABLE_COUNT && (mAUDIO_0.ENABLE_RELOAD || !(mAUDIO_0.CTLB & TIMER_DONE)) && mAUDIO_0.VOLUME && mAUDIO_0.BKUP) {
			decval = 0;

			if (mAUDIO_0.LINKING == 0x07) {
				if (mTIM_7.CTLB & BORROW_OUT) decval = 1;
				mAUDIO_0.LAST_LINK_CARRY = (mTIM_7.CTLB & BORROW_OUT);
			}
			else {
				// Ordinary clocked mode as opposed to linked mode
				// 16MHz clock downto 1us == cyclecount >> 4
				divide = (4 + mAUDIO_0.LINKING);
				decval = (gSystemCycleCount - mAUDIO_0.LAST_COUNT) >> divide;
			}

			if (decval) {
				mAUDIO_0.LAST_COUNT += decval << divide;
				mAUDIO_0.CURRENT -= decval;
				if (mAUDIO_0.CURRENT & 0x80000000) {
					// Set carry out
					mAUDIO_0.CTLB |= BORROW_OUT;

					// Reload if neccessary
					if (mAUDIO_0.ENABLE_RELOAD) {
						mAUDIO_0.CURRENT += mAUDIO_0.BKUP + 1;
						if (mAUDIO_0.CURRENT & 0x80000000) mAUDIO_0.CURRENT = 0;
					}
					else {
						// Set timer done
						mAUDIO_0.CTLB |= TIMER_DONE;
						mAUDIO_0.CURRENT = 0;
					}

					//
					// Update audio circuitry
					//
					mAUDIO_0.WAVESHAPER = GetLfsrNext(mAUDIO_0.WAVESHAPER);

					if (mAUDIO_0.INTEGRATE_ENABLE) {
						SLONG temp = mAUDIO_0.OUTPUT;
						if (mAUDIO_0.WAVESHAPER & 0x0001) temp += mAUDIO_0.VOLUME; else temp -= mAUDIO_0.VOLUME;
						if (temp > 127) temp = 127;
						if (temp <- 128) temp = -128;
						mAUDIO_0.OUTPUT = (SBYTE)temp;
					}
					else {
						if (mAUDIO_0.WAVESHAPER & 0x0001) mAUDIO_0.OUTPUT = mAUDIO_0.VOLUME; else mAUDIO_0.OUTPUT = -mAUDIO_0.VOLUME;
					}
				}
				else {
					mAUDIO_0.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mAUDIO_0.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mAUDIO_0.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}

			// Prediction for next timer event cycle number

			if (mAUDIO_0.LINKING != 7) {
				// Sometimes timeupdates can be >2x rollover in which case
				// then CURRENT may still be negative and we can use it to
				// calc the next timer value, we just want another update ASAP
				tmp = (mAUDIO_0.CURRENT&0x80000000) ? 1 : ((mAUDIO_0.CURRENT + 1) << divide);
				tmp += gSystemCycleCount;
				if (tmp < gNextTimerEvent) {
					gNextTimerEvent = tmp;
					TRACE_MIKIE1("Update() - AUDIO 0 Set NextTimerEvent = %012d", gNextTimerEvent);
				}
			}
//			TRACE_MIKIE1("Update() - mAUDIO_0_CURRENT = %012d",mAUDIO_0.CURRENT);
//			TRACE_MIKIE1("Update() - mAUDIO_0_BKUP    = %012d",mAUDIO_0.BKUP);
//			TRACE_MIKIE1("Update() - mAUDIO_0_LASTCNT = %012d",mAUDIO_0.LAST_COUNT);
//			TRACE_MIKIE1("Update() - mAUDIO_0_LINKING = %012d",mAUDIO_0.LINKING);
		}

		//
		// Audio 1
		//
//		if (mAUDIO_1.ENABLE_COUNT && !(mAUDIO_1.CTLB & TIMER_DONE) && mAUDIO_1.VOLUME && mAUDIO_1.BKUP)
		if (mAUDIO_1.ENABLE_COUNT && (mAUDIO_1.ENABLE_RELOAD || !(mAUDIO_1.CTLB & TIMER_DONE)) && mAUDIO_1.VOLUME && mAUDIO_1.BKUP) {
			decval = 0;

			if (mAUDIO_1.LINKING == 0x07) {
				if (mAUDIO_0.CTLB & BORROW_OUT) decval = 1;
				mAUDIO_1.LAST_LINK_CARRY = (mAUDIO_0.CTLB & BORROW_OUT);
			}
			else {
				// Ordinary clocked mode as opposed to linked mode
				// 16MHz clock downto 1us == cyclecount >> 4
				divide = (4 + mAUDIO_1.LINKING);
				decval = (gSystemCycleCount - mAUDIO_1.LAST_COUNT) >> divide;
			}

			if (decval) {
				mAUDIO_1.LAST_COUNT += decval<<divide;
				mAUDIO_1.CURRENT -= decval;
				if (mAUDIO_1.CURRENT & 0x80000000) {
					// Set carry out
					mAUDIO_1.CTLB |= BORROW_OUT;

					// Reload if neccessary
					if (mAUDIO_1.ENABLE_RELOAD) {
						mAUDIO_1.CURRENT += mAUDIO_1.BKUP+1;
						if (mAUDIO_1.CURRENT & 0x80000000) mAUDIO_1.CURRENT = 0;
					}
					else {
						// Set timer done
						mAUDIO_1.CTLB |= TIMER_DONE;
						mAUDIO_1.CURRENT = 0;
					}

					//
					// Update audio circuitry
					//
					mAUDIO_1.WAVESHAPER = GetLfsrNext(mAUDIO_1.WAVESHAPER);

					if (mAUDIO_1.INTEGRATE_ENABLE) {
						SLONG temp = mAUDIO_1.OUTPUT;
						if (mAUDIO_1.WAVESHAPER & 0x0001) temp += mAUDIO_1.VOLUME; else temp -= mAUDIO_1.VOLUME;
						if (temp > 127) temp = 127;
						if (temp < -128) temp = -128;
						mAUDIO_1.OUTPUT = (SBYTE)temp;
					}
					else {
						if (mAUDIO_1.WAVESHAPER & 0x0001) mAUDIO_1.OUTPUT = mAUDIO_1.VOLUME; else mAUDIO_1.OUTPUT = -mAUDIO_1.VOLUME;
					}
				}
				else {
					mAUDIO_1.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mAUDIO_1.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mAUDIO_1.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}

			// Prediction for next timer event cycle number

			if (mAUDIO_1.LINKING != 7) {
				// Sometimes timeupdates can be >2x rollover in which case
				// then CURRENT may still be negative and we can use it to
				// calc the next timer value, we just want another update ASAP
				tmp = (mAUDIO_1.CURRENT & 0x80000000) ? 1 : ((mAUDIO_1.CURRENT + 1) << divide);
				tmp += gSystemCycleCount;
				if (tmp < gNextTimerEvent) {
					gNextTimerEvent = tmp;
					TRACE_MIKIE1("Update() - AUDIO 1 Set NextTimerEvent = %012d", gNextTimerEvent);
				}
			}
//			TRACE_MIKIE1("Update() - mAUDIO_1_CURRENT = %012d",mAUDIO_1.CURRENT);
//			TRACE_MIKIE1("Update() - mAUDIO_1_BKUP    = %012d",mAUDIO_1.BKUP);
//			TRACE_MIKIE1("Update() - mAUDIO_1_LASTCNT = %012d",mAUDIO_1.LAST_COUNT);
//			TRACE_MIKIE1("Update() - mAUDIO_1_LINKING = %012d",mAUDIO_1.LINKING);
		}

		//
		// Audio 2
		//
//		if (mAUDIO_2.ENABLE_COUNT && !(mAUDIO_2.CTLB & TIMER_DONE) && mAUDIO_2.VOLUME && mAUDIO_2.BKUP)
		if (mAUDIO_2.ENABLE_COUNT && (mAUDIO_2.ENABLE_RELOAD || !(mAUDIO_2.CTLB & TIMER_DONE)) && mAUDIO_2.VOLUME && mAUDIO_2.BKUP) {
			decval = 0;

			if (mAUDIO_2.LINKING == 0x07) {
				if (mAUDIO_1.CTLB & BORROW_OUT) decval = 1;
				mAUDIO_2.LAST_LINK_CARRY = (mAUDIO_1.CTLB & BORROW_OUT);
			}
			else {
				// Ordinary clocked mode as opposed to linked mode
				// 16MHz clock downto 1us == cyclecount >> 4
				divide = (4 + mAUDIO_2.LINKING);
				decval = (gSystemCycleCount - mAUDIO_2.LAST_COUNT) >> divide;
			}

			if (decval) {
				mAUDIO_2.LAST_COUNT += decval<<divide;
				mAUDIO_2.CURRENT -= decval;
				if (mAUDIO_2.CURRENT & 0x80000000) {
					// Set carry out
					mAUDIO_2.CTLB |= BORROW_OUT;

					// Reload if neccessary
					if (mAUDIO_2.ENABLE_RELOAD) {
						mAUDIO_2.CURRENT += mAUDIO_2.BKUP + 1;
						if (mAUDIO_2.CURRENT & 0x80000000) mAUDIO_2.CURRENT = 0;
					}
					else {
						// Set timer done
						mAUDIO_2.CTLB |= TIMER_DONE;
						mAUDIO_2.CURRENT = 0;
					}

					//
					// Update audio circuitry
					//
					mAUDIO_2.WAVESHAPER = GetLfsrNext(mAUDIO_2.WAVESHAPER);

					if (mAUDIO_2.INTEGRATE_ENABLE) {
						SLONG temp = mAUDIO_2.OUTPUT;
						if (mAUDIO_2.WAVESHAPER&0x0001) temp += mAUDIO_2.VOLUME; else temp -= mAUDIO_2.VOLUME;
						if (temp > 127) temp = 127;
						if (temp < -128) temp = -128;
						mAUDIO_2.OUTPUT = (SBYTE)temp;
					}
					else {
						if (mAUDIO_2.WAVESHAPER & 0x0001) mAUDIO_2.OUTPUT = mAUDIO_2.VOLUME; else mAUDIO_2.OUTPUT = -mAUDIO_2.VOLUME;
					}
				}
				else {
					mAUDIO_2.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mAUDIO_2.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mAUDIO_2.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}

			// Prediction for next timer event cycle number

			if (mAUDIO_2.LINKING != 7) {
				// Sometimes timeupdates can be >2x rollover in which case
				// then CURRENT may still be negative and we can use it to
				// calc the next timer value, we just want another update ASAP
				tmp = (mAUDIO_2.CURRENT & 0x80000000) ? 1 : ((mAUDIO_2.CURRENT + 1) << divide);
				tmp += gSystemCycleCount;
				if (tmp < gNextTimerEvent) {
					gNextTimerEvent = tmp;
					TRACE_MIKIE1("Update() - AUDIO 2 Set NextTimerEvent = %012d", gNextTimerEvent);
				}
			}
//			TRACE_MIKIE1("Update() - mAUDIO_2_CURRENT = %012d",mAUDIO_2.CURRENT);
//			TRACE_MIKIE1("Update() - mAUDIO_2_BKUP    = %012d",mAUDIO_2.BKUP);
//			TRACE_MIKIE1("Update() - mAUDIO_2_LASTCNT = %012d",mAUDIO_2.LAST_COUNT);
//			TRACE_MIKIE1("Update() - mAUDIO_2_LINKING = %012d",mAUDIO_2.LINKING);
		}

		//
		// Audio 3
		//
//		if (mAUDIO_3.ENABLE_COUNT && !(mAUDIO_3.CTLB & TIMER_DONE) && mAUDIO_3.VOLUME && mAUDIO_3.BKUP)
		if (mAUDIO_3.ENABLE_COUNT && (mAUDIO_3.ENABLE_RELOAD || !(mAUDIO_3.CTLB & TIMER_DONE)) && mAUDIO_3.VOLUME && mAUDIO_3.BKUP) {
			decval = 0;

			if (mAUDIO_3.LINKING == 0x07) {
				if (mAUDIO_2.CTLB & BORROW_OUT) decval = 1;
				mAUDIO_3.LAST_LINK_CARRY = (mAUDIO_2.CTLB & BORROW_OUT);
			}
			else {
				// Ordinary clocked mode as opposed to linked mode
				// 16MHz clock downto 1us == cyclecount >> 4
				divide = (4 + mAUDIO_3.LINKING);
				decval = (gSystemCycleCount - mAUDIO_3.LAST_COUNT) >> divide;
			}

			if (decval) {
				mAUDIO_3.LAST_COUNT += decval << divide;
				mAUDIO_3.CURRENT -= decval;
				if (mAUDIO_3.CURRENT & 0x80000000) {
					// Set carry out
					mAUDIO_3.CTLB |= BORROW_OUT;

					// Reload if neccessary
					if (mAUDIO_3.ENABLE_RELOAD) {
						mAUDIO_3.CURRENT += mAUDIO_3.BKUP + 1;
						if (mAUDIO_3.CURRENT & 0x80000000) mAUDIO_3.CURRENT = 0;
					}
					else {
						// Set timer done
						mAUDIO_3.CTLB |= TIMER_DONE;
						mAUDIO_3.CURRENT = 0;
					}

					//
					// Update audio circuitry
					//
					mAUDIO_3.WAVESHAPER = GetLfsrNext(mAUDIO_3.WAVESHAPER);

					if (mAUDIO_3.INTEGRATE_ENABLE) {
						SLONG temp = mAUDIO_3.OUTPUT;
						if (mAUDIO_3.WAVESHAPER & 0x0001) temp += mAUDIO_3.VOLUME; else temp -= mAUDIO_3.VOLUME;
						if (temp > 127) temp = 127;
						if (temp < -128) temp = -128;
						mAUDIO_3.OUTPUT = (SBYTE)temp;
					}
					else {
						if (mAUDIO_3.WAVESHAPER & 0x0001) mAUDIO_3.OUTPUT = mAUDIO_3.VOLUME; else mAUDIO_3.OUTPUT = -mAUDIO_3.VOLUME;
					}
				}
				else {
					mAUDIO_3.CTLB &= ~BORROW_OUT;
				}
				// Set carry in as we did a count
				mAUDIO_3.CTLB |= BORROW_IN;
			}
			else {
				// Clear carry in as we didn't count
				// Clear carry out
				mAUDIO_3.CTLB &= ~(BORROW_OUT | BORROW_IN);
			}

			// Prediction for next timer event cycle number

			if (mAUDIO_3.LINKING != 7) {
				// Sometimes timeupdates can be >2x rollover in which case
				// then CURRENT may still be negative and we can use it to
				// calc the next timer value, we just want another update ASAP
				tmp = (mAUDIO_3.CURRENT & 0x80000000) ? 1 : ((mAUDIO_3.CURRENT + 1) << divide);
				tmp += gSystemCycleCount;
				if (tmp < gNextTimerEvent) {
					gNextTimerEvent = tmp;
					TRACE_MIKIE1("Update() - AUDIO 3 Set NextTimerEvent = %012d", gNextTimerEvent);
				}
			}
//			TRACE_MIKIE1("Update() - mAUDIO_3_CURRENT = %012d",mAUDIO_3.CURRENT);
//			TRACE_MIKIE1("Update() - mAUDIO_3_BKUP    = %012d",mAUDIO_3.BKUP);
//			TRACE_MIKIE1("Update() - mAUDIO_3_LASTCNT = %012d",mAUDIO_3.LAST_COUNT);
//			TRACE_MIKIE1("Update() - mAUDIO_3_LINKING = %012d",mAUDIO_3.LINKING);
		}
	}

//	if (gSystemCycleCount == gNextTimerEvent) gError->Warning("CMikie::Update() - gSystemCycleCount==gNextTimerEvent, system lock likely");
//	TRACE_MIKIE1("Update() - NextTimerEvent = %012d",gNextTimerEvent);

	gSystemIRQ = (mTimerStatusFlags) ? true : false;
	mSystem.setIrqPin(gSystemIRQ);
	if (gSystemIRQ && gSystemCPUSleep) { ClearCPUSleep(); }

	// Now all the timer updates are done we can increment the system
	// counter for any work done within the Update() function, gSystemCycleCounter
	// cannot be updated until this point otherwise it screws up the counters.
	gSystemCycleCount += mikie_work_done;
}
