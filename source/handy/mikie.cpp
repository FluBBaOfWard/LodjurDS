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

#define mTIM_4 mikey_0.timer4
#define mAUDIO_0 mikey_0.audio0
#define mAUDIO_1 mikey_0.audio1
#define mAUDIO_2 mikey_0.audio2
#define mAUDIO_3 mikey_0.audio3
#define mSTEREO mikey_0.stereo

void CMikie::ResetAudio(MAUDIO& audio)
{
	audio.BKUP = 0;
	audio.CURRENT = 0;
	audio.CTLB = 0;
	audio.LAST_COUNT = 0;
	audio.INTEGRATE_ENABLE = 0;
	audio.WAVESHAPER = 0;
}

CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_MIKIE0("CMikie()");

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

	ResetAudio(mAUDIO_0);
	ResetAudio(mAUDIO_1);
	ResetAudio(mAUDIO_2);
	ResetAudio(mAUDIO_3);

	mSTEREO = 0xff;	// All channels enabled

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
		if ((switches >> swloop) & 0x001)
			result ^= (lfsr >> switchbits[swloop]) & 0x001;
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
	mikey_0.tim0CtlA = (ENABLE_COUNT | ENABLE_RELOAD);

	mikey_0.tim2Bkup = 0x68;
	mikey_0.tim2CtlA = (LINKING | ENABLE_COUNT | ENABLE_RELOAD);
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

// Peek/Poke memory handlers

void CMikie::Poke(ULONG addr, UBYTE data)
{
	switch(addr & 0xff)
	{
		case (AUD0VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mikey_0.aud0Vol && data) {
				mAUDIO_0.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mikey_0.aud0Vol = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD0VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
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
			mikey_0.aud0Ctl = data;
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
		case (AUD0MISC & 0xff):
			mAUDIO_0.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_0.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_0.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD0MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD1VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mikey_0.aud1Vol && data) {
				mAUDIO_1.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mikey_0.aud1Vol = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD1VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
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
			mikey_0.aud1Ctl = data;
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
		case (AUD1MISC & 0xff):
			mAUDIO_1.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_1.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_1.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD1MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD2VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mikey_0.aud2Vol && data) {
				mAUDIO_2.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mikey_0.aud2Vol = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD2VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
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
			mikey_0.aud2Ctl = data;
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
		case (AUD2MISC & 0xff):
			mAUDIO_2.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_2.WAVESHAPER |= (data&0xf0) << 4;
			mAUDIO_2.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD2MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
			break;

		case (AUD3VOL & 0xff):
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if (!mikey_0.aud3Vol && data) {
				mAUDIO_3.LAST_COUNT = gSystemCycleCount;
				gNextTimerEvent = gSystemCycleCount;
			}
			mikey_0.aud3Vol = (SBYTE)data;
			TRACE_MIKIE2("Poke(AUD3VOL,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
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
			mikey_0.aud0Ctl = data;
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
		case (AUD3MISC & 0xff):
			mAUDIO_3.WAVESHAPER &= 0x1ff0ff;
			mAUDIO_3.WAVESHAPER |= (data & 0xf0) << 4;
			mAUDIO_3.CTLB = data & (BORROW_OUT | BORROW_IN | LAST_CLOCK);
			TRACE_MIKIE2("Poke(AUD3MISC,%02x) at PC=%04x", data, mSystem.mCpu->GetPC());
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

		default:
			break;
	}
}

UBYTE CMikie::Peek(ULONG addr)
{
	switch(addr & 0xff)
	{
// Audio control registers
		case (AUD0CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_0.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= (mAUDIO_0.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mikey_0.aud0Ctl & (CLOCK_SEL | ENABLE_COUNT | ENABLE_RELOAD));
				TRACE_MIKIE2("Peek(AUD0CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD1CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_1.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= (mAUDIO_1.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mikey_0.aud1Ctl & (CLOCK_SEL | ENABLE_COUNT | ENABLE_RELOAD));
				TRACE_MIKIE2("Peek(AUD1CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD2CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_2.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= (mAUDIO_2.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mikey_0.aud2Ctl & (CLOCK_SEL | ENABLE_COUNT | ENABLE_RELOAD));
				TRACE_MIKIE2("Peek(AUD2CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD3CTL & 0xff):
			{
				UBYTE retval = 0;
				retval |= (mAUDIO_3.WAVESHAPER & 0x001000) ? 0x80 : 0x00;
				retval |= (mAUDIO_3.INTEGRATE_ENABLE) ? 0x20 : 0x00;
				retval |= (mikey_0.aud3Ctl & (CLOCK_SEL | ENABLE_COUNT | ENABLE_RELOAD));
				TRACE_MIKIE2("Peek(AUD3CTL,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return retval;
			}
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
				retval |= (mikey_0.ioDir&0x08) ? (((mikey_0.ioDat & 0x08) && mIODAT_REST_SIGNAL) ? 0x00 : 0x08) : 0x00;	// REST = output bit : input low
				retval |= (mikey_0.ioDir&0x04) ? mikey_0.ioDat & 0x04 : ((mUART_CABLE_PRESENT) ? 0x04 : 0x00);	// NOEXP  = output bit : input low
				retval |= (mikey_0.ioDir&0x02) ? mikey_0.ioDat & 0x02 : 0x00;									// CARTAD = output bit : input low
				retval |= (mikey_0.ioDir&0x01) ? mikey_0.ioDat & 0x01 : 0x01;									// EXTPW  = output bit : input high (Power connected)
				TRACE_MIKIE2("Peek(IODAT   ,%02x) at PC=%04x", retval, mSystem.mCpu->GetPC());
				return (UBYTE)retval;
			}
			break;
		default:
			return 0xFF;
			break;
	}
	return 0xFF;
}

void CMikie::UpdateTimer4(void) {
	int divide = 0;
	int decval = 0;
	ULONG tmp;
	//
	// Timer 4 of Group A
	//
	// For the sake of speed it is assumed that Timer 4 (UART timer)
	// never uses one-shot mode, never uses linking, hence the code
	// is commented out.
	//
	if (mikey_0.tim4CtlA & ENABLE_COUNT) {
		decval = 0;

//		if ((mikey_0.tim4CtlA & CLOCK_SEL) == LINKING) {
//			if (mikey_0.tim4CtlB & BORROW_OUT) decval = 1;
//		}
//		else
		{
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			// Additional /8 (+3) for 8 clocks per bit transmit
			divide = 4 + 3 + (mikey_0.tim4CtlA & CLOCK_SEL);
			decval = (gSystemCycleCount - mTIM_4.LAST_COUNT) >> divide;
		}

		if (decval) {
			mTIM_4.LAST_COUNT += decval << divide;
			mTIM_4.CURRENT -= decval;
			if (mTIM_4.CURRENT & 0x80000000) {
				// Set carry out
				mikey_0.tim4CtlB |= BORROW_OUT;

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
				else if(!(mUART_RX_COUNTDOWN & UART_RX_INACTIVE)) {
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
//				if (mikey_0.tim4CtlA & ENABLE_RELOAD) {
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
//				mikey_0.tim4CtlB |= TIMER_DONE;
			}
//			else {
//				mikey_0.tim4CtlB &= ~BORROW_OUT;
//			}
//			// Set carry in as we did a count
//			mikey_0.tim4CtlB |= BORROW_IN;
		}
//		else {
//			// Clear carry in/out as we didn't count
//			mikey_0.tim4CtlB &= ~(BORROW_OUT | BORROW_IN);
//		}
//
//		// Prediction for next timer event cycle number
//
//		if ((mikey_0.tim4CtlA & CLOCK_SEL) != LINKING) {
			// Sometimes timeupdates can be >2x rollover in which case
			// then CURRENT may still be negative and we can use it to
			// calc the next timer value, we just want another update ASAP
			tmp = (mTIM_4.CURRENT & 0x80000000) ? 1 : ((mTIM_4.CURRENT + 1) << divide);
			tmp += gSystemCycleCount;
			if (tmp < gNextTimerEvent) {
				gNextTimerEvent = tmp;
				TRACE_MIKIE1("Update() - TIMER 4 Set NextTimerEvent = %012d", gNextTimerEvent);
			}
//		}
	}

	// Emulate the UART bug where UART IRQ is level sensitive
	// in that it will continue to generate interrupts as long
	// as they are enabled and the interrupt condition is true

	// If Tx is inactive i.e ready for a byte to eat and the
	// IRQ is enabled then generate it always
	if ((mUART_TX_COUNTDOWN & UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
		mikey_0.timerStatusFlags |= 0x10;
	}
	// Is data waiting and the interrupt enabled, if so then
	// what are we waiting for....
	if (mUART_RX_READY && mUART_RX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
		mikey_0.timerStatusFlags |= 0x10;
	}
}

void CMikie::UpdateSound(void) {
	int divide = 0;
	int decval = 0;
	ULONG tmp;

//	static SLONG sample = 0;
	// ULONG mix = 0; // unused

	//
	// Catch audio buffer up to current time
	//

	// Mix the sample

	/*
	sample = 0;
	if (mSTEREO & 0x11) { sample += mikey_0.aud0OutVal; mix++; }
	if (mSTEREO & 0x22) { sample += mikey_0.aud1OutVal; mix++; }
	if (mSTEREO & 0x44) { sample += mikey_0.aud2OutVal; mix++; }
	if (mSTEREO & 0x88) { sample += mikey_0.aud3OutVal; mix++; }
	if (mix) {
		sample += 128 * mix; // Correct for sign
		sample /= mix;	// Keep the audio volume at max
	}
	else {
		sample = 128;
	}

//	sample += (mSTEREO & 0x11) ? mikey_0.aud0OutVal : 0;
//	sample += (mSTEREO & 0x22) ? mikey_0.aud1OutVal : 0;
//	sample += (mSTEREO & 0x44) ? mikey_0.aud2OutVal : 0;
//	sample += (mSTEREO & 0x88) ? mikey_0.aud3OutVal : 0;
//	sample = sample >> 2;
//	sample += 128;
	*/

	for (;gAudioLastUpdateCycle+HANDY_AUDIO_SAMPLE_PERIOD < gSystemCycleCount;gAudioLastUpdateCycle += HANDY_AUDIO_SAMPLE_PERIOD) {
		// Output audio sample
//		gAudioBuffer[gAudioBufferPointer++] = (UBYTE)sample;
		gAudioBuffer0[gAudioBufferPointer] = (mSTEREO & 0x11) ? mikey_0.aud0OutVal : 0;
		gAudioBuffer1[gAudioBufferPointer] = (mSTEREO & 0x22) ? mikey_0.aud1OutVal : 0;
		gAudioBuffer2[gAudioBufferPointer] = (mSTEREO & 0x33) ? mikey_0.aud2OutVal : 0;
		gAudioBuffer3[gAudioBufferPointer++] = (mSTEREO & 0x44) ? mikey_0.aud3OutVal : 0;

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
//	if ((mikey_0.aud0Ctl & ENABLE_COUNT) && !(mAUDIO_0.CTLB & TIMER_DONE) && mAUDIO_0.VOLUME && mAUDIO_0.BKUP)
	if ((mikey_0.aud0Ctl & ENABLE_COUNT) && ((mikey_0.aud0Ctl & ENABLE_RELOAD) || !(mAUDIO_0.CTLB & TIMER_DONE)) && mikey_0.aud0Vol && mAUDIO_0.BKUP) {
		decval = 0;

		if ((mikey_0.aud0Ctl & CLOCK_SEL) == LINKING) {
			if (mikey_0.tim7CtlB & BORROW_OUT) decval = 1;
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + (mikey_0.aud0Ctl & CLOCK_SEL));
			decval = (gSystemCycleCount - mAUDIO_0.LAST_COUNT) >> divide;
		}

		if (decval) {
			mAUDIO_0.LAST_COUNT += decval << divide;
			mAUDIO_0.CURRENT -= decval;
			if (mAUDIO_0.CURRENT & 0x80000000) {
				// Set carry out
				mAUDIO_0.CTLB |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud0Ctl & ENABLE_RELOAD) {
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
					SLONG temp = mikey_0.aud0OutVal;
					if (mAUDIO_0.WAVESHAPER & 0x0001) temp += mikey_0.aud0Vol; else temp -= mikey_0.aud0Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud0OutVal = (SBYTE)temp;
				}
				else {
					if (mAUDIO_0.WAVESHAPER & 0x0001) mikey_0.aud0OutVal = mikey_0.aud0Vol; else mikey_0.aud0OutVal = -mikey_0.aud0Vol;
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

		if ((mikey_0.aud0Ctl & CLOCK_SEL) != LINKING) {
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
	}

	//
	// Audio 1
	//
//	if ((mikey_0.aud1Ctl & ENABLE_COUNT) && !(mAUDIO_1.CTLB & TIMER_DONE) && mikey_0.aud1Vol && mAUDIO_1.BKUP)
	if ((mikey_0.aud1Ctl & ENABLE_COUNT) && ((mikey_0.aud1Ctl & ENABLE_RELOAD) || !(mAUDIO_1.CTLB & TIMER_DONE)) && mikey_0.aud1Vol && mAUDIO_1.BKUP) {
		decval = 0;

		if ((mikey_0.aud1Ctl & CLOCK_SEL) == LINKING) {
			if (mAUDIO_0.CTLB & BORROW_OUT) decval = 1;
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + (mikey_0.aud1Ctl & CLOCK_SEL));
			decval = (gSystemCycleCount - mAUDIO_1.LAST_COUNT) >> divide;
		}

		if (decval) {
			mAUDIO_1.LAST_COUNT += decval<<divide;
			mAUDIO_1.CURRENT -= decval;
			if (mAUDIO_1.CURRENT & 0x80000000) {
				// Set carry out
				mAUDIO_1.CTLB |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud1Ctl & ENABLE_RELOAD) {
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
					SLONG temp = mikey_0.aud1OutVal;
					if (mAUDIO_1.WAVESHAPER & 0x0001) temp += mikey_0.aud1Vol; else temp -= mikey_0.aud1Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud1OutVal = (SBYTE)temp;
				}
				else {
					if (mAUDIO_1.WAVESHAPER & 0x0001) mikey_0.aud1OutVal = mikey_0.aud1Vol; else mikey_0.aud1OutVal = -mikey_0.aud1Vol;
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

		if ((mikey_0.aud1Ctl & CLOCK_SEL) != LINKING) {
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
	}

	//
	// Audio 2
	//
//	if ((mikey_0.aud2Ctl & ENABLE_COUNT) && !(mAUDIO_2.CTLB & TIMER_DONE) && mikey_0.aud2Vol && mAUDIO_2.BKUP)
	if ((mikey_0.aud2Ctl & ENABLE_COUNT) && ((mikey_0.aud2Ctl & ENABLE_RELOAD) || !(mAUDIO_2.CTLB & TIMER_DONE)) && mikey_0.aud2Vol && mAUDIO_2.BKUP) {
		decval = 0;

		if ((mikey_0.aud2Ctl & CLOCK_SEL) == LINKING) {
			if (mAUDIO_1.CTLB & BORROW_OUT) decval = 1;
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + (mikey_0.aud2Ctl & CLOCK_SEL));
			decval = (gSystemCycleCount - mAUDIO_2.LAST_COUNT) >> divide;
		}

		if (decval) {
			mAUDIO_2.LAST_COUNT += decval<<divide;
			mAUDIO_2.CURRENT -= decval;
			if (mAUDIO_2.CURRENT & 0x80000000) {
				// Set carry out
				mAUDIO_2.CTLB |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud2Ctl & ENABLE_RELOAD) {
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
					SLONG temp = mikey_0.aud2OutVal;
					if (mAUDIO_2.WAVESHAPER&0x0001) temp += mikey_0.aud2Vol; else temp -= mikey_0.aud2Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud2OutVal = (SBYTE)temp;
				}
				else {
					if (mAUDIO_2.WAVESHAPER & 0x0001) mikey_0.aud2OutVal = mikey_0.aud2Vol; else mikey_0.aud2OutVal = -mikey_0.aud2Vol;
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

		if ((mikey_0.aud2Ctl & CLOCK_SEL) != LINKING) {
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
	}

	//
	// Audio 3
	//
//	if ((mikey_0.aud3Ctl & ENABLE_COUNT) && !(mAUDIO_3.CTLB & TIMER_DONE) && mikey_0.aud3Vol && mAUDIO_3.BKUP)
	if ((mikey_0.aud3Ctl & ENABLE_COUNT) && ((mikey_0.aud3Ctl & ENABLE_RELOAD) || !(mAUDIO_3.CTLB & TIMER_DONE)) && mikey_0.aud3Vol && mAUDIO_3.BKUP) {
		decval = 0;

		if ((mikey_0.aud3Ctl & CLOCK_SEL) == LINKING) {
			if (mAUDIO_2.CTLB & BORROW_OUT) decval = 1;
		}
		else {
			// Ordinary clocked mode as opposed to linked mode
			// 16MHz clock downto 1us == cyclecount >> 4
			divide = (4 + (mikey_0.aud3Ctl & CLOCK_SEL));
			decval = (gSystemCycleCount - mAUDIO_3.LAST_COUNT) >> divide;
		}

		if (decval) {
			mAUDIO_3.LAST_COUNT += decval << divide;
			mAUDIO_3.CURRENT -= decval;
			if (mAUDIO_3.CURRENT & 0x80000000) {
				// Set carry out
				mAUDIO_3.CTLB |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud3Ctl & ENABLE_RELOAD) {
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
					SLONG temp = mikey_0.aud3OutVal;
					if (mAUDIO_3.WAVESHAPER & 0x0001) temp += mikey_0.aud3Vol; else temp -= mikey_0.aud3Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud3OutVal = (SBYTE)temp;
				}
				else {
					if (mAUDIO_3.WAVESHAPER & 0x0001) mikey_0.aud3OutVal = mikey_0.aud3Vol; else mikey_0.aud3OutVal = -mikey_0.aud3Vol;
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

		if ((mikey_0.aud3Ctl & CLOCK_SEL) != LINKING) {
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
	}
}
