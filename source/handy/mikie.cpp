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

#include "mikie.h"
#include "system.h"
#include "../Cpu.h"

#define mTIM_4 mikey_0.timer4
#define mAUDIO_0 mikey_0.audio0
#define mAUDIO_1 mikey_0.audio1
#define mAUDIO_2 mikey_0.audio2
#define mAUDIO_3 mikey_0.audio3
#define mSTEREO mikey_0.stereo

#define mUART_RX_COUNTDOWN mikey_0.uart_RX_COUNTDOWN
#define mUART_TX_COUNTDOWN mikey_0.uart_TX_COUNTDOWN
#define mUART_TX_DATA mikey_0.uart_TX_DATA
#define mUART_RX_READY mikey_0.uart_RX_READY

#define mpUART_TX_CALLBACK mikey_0.txFunction
#define mUART_Rx_input_queue mikey_0.uart_Rx_input_queue
#define mUART_Rx_input_ptr mikey_0.uart_Rx_input_ptr
#define mUART_Rx_output_ptr mikey_0.uart_Rx_output_ptr
#define mUART_Rx_waiting mikey_0.uart_Rx_waiting

CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_MIKIE0("CMikie()");

	Reset();
}

CMikie::~CMikie()
{
	TRACE_MIKIE0("~CMikie()");
}

void CMikie::Reset(void)
{
	TRACE_MIKIE0("Reset()");
}

u32 CMikie::GetLfsrNext(u32 current)
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

	static const u32 switchbits[9]={7,0,1,2,3,4,5,10,11};

	u32 switches = current >> 12;
	u32 lfsr = current & 0xfff;
	u32 result = 0;
	for (int swloop=0;swloop<9;swloop++) {
		if ((switches >> swloop) & 0x001)
			result ^= (lfsr >> switchbits[swloop]) & 0x001;
	}
	result = (result) ? 0 : 1;
	return (switches << 12) | ((lfsr << 1) & 0xffe) | result;
}

void CMikie::UpdateTimer4(u32 sysCycCount) {
	int divide = 0;
	int decval = 0;
	u32 tmp;
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
			decval = (sysCycCount - mTIM_4.LAST_COUNT) >> divide;
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
						mikey_0.uart_RX_DATA = mUART_Rx_input_queue[mUART_Rx_output_ptr];
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
					if (mUART_RX_READY) mikey_0.uart_Rx_overun_error = 1;

					// Flag byte as being recvd
					mUART_RX_READY = 1;
				}
				else if(!(mUART_RX_COUNTDOWN & UART_RX_INACTIVE)) {
					mUART_RX_COUNTDOWN--;
				}

				if (!mUART_TX_COUNTDOWN) {
					if (mikey_0.uart_SENDBREAK) {
						mUART_TX_DATA = UART_BREAK_CODE;
						// Auto-Respawn new transmit
						mUART_TX_COUNTDOWN = UART_TX_TIME_PERIOD;
						// Loop back what we transmitted
						ComLynxTxLoopback(&mikey_0, mUART_TX_DATA);
					}
					else {
						// Serial activity finished
						mUART_TX_COUNTDOWN = UART_TX_INACTIVE;
					}

					// If a networking object is attached then use its callback to send the data byte.
					if (mpUART_TX_CALLBACK) {
						TRACE_MIKIE0("Update() - UART_TX_CALLBACK");
						(*mpUART_TX_CALLBACK)(mUART_TX_DATA, mikey_0.txCallbackObj);
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
						mTIM_4.LAST_COUNT = sysCycCount;
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
			tmp += sysCycCount;
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
	if ((mUART_TX_COUNTDOWN & UART_TX_INACTIVE) && mikey_0.uart_TX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
		mikey_0.timerStatusFlags |= 0x10;
	}
	// Is data waiting and the interrupt enabled, if so then
	// what are we waiting for....
	if (mUART_RX_READY && mikey_0.uart_RX_IRQ_ENABLE) {
		TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
		mikey_0.timerStatusFlags |= 0x10;
	}
}

void CMikie::UpdateSound(void) {
	int divide = 0;
	int decval = 0;
	u32 tmp;

//	static s32 sample = 0;
	// u32 mix = 0; // unused

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
//		gAudioBuffer[gAudioBufferPointer++] = (u8)sample;
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

		gAudioBufferPointer %= HANDY_AUDIO_BUFFER_SIZE;
	}

	//
	// Audio 0
	//
//	if ((mikey_0.aud0Ctl & ENABLE_COUNT) && !(mikey_0.aud0Misc & TIMER_DONE) && mAUDIO_0.VOLUME && mAUDIO_0.BKUP)
	if ((mikey_0.aud0Ctl & ENABLE_COUNT) && ((mikey_0.aud0Ctl & ENABLE_RELOAD) || !(mikey_0.aud0Misc & TIMER_DONE)) && mikey_0.aud0Vol && mAUDIO_0.BKUP) {
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
				mikey_0.aud0Misc |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud0Ctl & ENABLE_RELOAD) {
					mAUDIO_0.CURRENT += mAUDIO_0.BKUP + 1;
					if (mAUDIO_0.CURRENT & 0x80000000) mAUDIO_0.CURRENT = 0;
				}
				else {
					// Set timer done
					mikey_0.aud0Misc |= TIMER_DONE;
					mAUDIO_0.CURRENT = 0;
				}

				//
				// Update audio circuitry
				//
				mAUDIO_0.WAVESHAPER = GetLfsrNext(mAUDIO_0.WAVESHAPER);

				if (mikey_0.aud0Ctl & INTEGRATE) {
					s32 temp = mikey_0.aud0OutVal;
					if (mAUDIO_0.WAVESHAPER & 0x0001)
						temp += mikey_0.aud0Vol;
					else
						temp -= mikey_0.aud0Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud0OutVal = (s8)temp;
				}
				else {
					if (mAUDIO_0.WAVESHAPER & 0x0001)
						mikey_0.aud0OutVal = mikey_0.aud0Vol;
					else
						mikey_0.aud0OutVal = -mikey_0.aud0Vol;
				}
			}
			else {
				mikey_0.aud0Misc &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mikey_0.aud0Misc |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mikey_0.aud0Misc &= ~(BORROW_OUT | BORROW_IN);
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
//	if ((mikey_0.aud1Ctl & ENABLE_COUNT) && !(mikey_0.aud1Misc & TIMER_DONE) && mikey_0.aud1Vol && mAUDIO_1.BKUP)
	if ((mikey_0.aud1Ctl & ENABLE_COUNT) && ((mikey_0.aud1Ctl & ENABLE_RELOAD) || !(mikey_0.aud1Misc & TIMER_DONE)) && mikey_0.aud1Vol && mAUDIO_1.BKUP) {
		decval = 0;

		if ((mikey_0.aud1Ctl & CLOCK_SEL) == LINKING) {
			if (mikey_0.aud0Misc & BORROW_OUT) decval = 1;
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
				mikey_0.aud1Misc |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud1Ctl & ENABLE_RELOAD) {
					mAUDIO_1.CURRENT += mAUDIO_1.BKUP+1;
					if (mAUDIO_1.CURRENT & 0x80000000) mAUDIO_1.CURRENT = 0;
				}
				else {
					// Set timer done
					mikey_0.aud1Misc |= TIMER_DONE;
					mAUDIO_1.CURRENT = 0;
				}

				//
				// Update audio circuitry
				//
				mAUDIO_1.WAVESHAPER = GetLfsrNext(mAUDIO_1.WAVESHAPER);

				if (mikey_0.aud1Ctl & INTEGRATE) {
					s32 temp = mikey_0.aud1OutVal;
					if (mAUDIO_1.WAVESHAPER & 0x0001)
						temp += mikey_0.aud1Vol;
					else
						temp -= mikey_0.aud1Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud1OutVal = (s8)temp;
				}
				else {
					if (mAUDIO_1.WAVESHAPER & 0x0001)
						mikey_0.aud1OutVal = mikey_0.aud1Vol;
					else
						mikey_0.aud1OutVal = -mikey_0.aud1Vol;
				}
			}
			else {
				mikey_0.aud1Misc &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mikey_0.aud1Misc |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mikey_0.aud1Misc &= ~(BORROW_OUT | BORROW_IN);
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
//	if ((mikey_0.aud2Ctl & ENABLE_COUNT) && !(mikey_0.aud2Misc & TIMER_DONE) && mikey_0.aud2Vol && mAUDIO_2.BKUP)
	if ((mikey_0.aud2Ctl & ENABLE_COUNT) && ((mikey_0.aud2Ctl & ENABLE_RELOAD) || !(mikey_0.aud2Misc & TIMER_DONE)) && mikey_0.aud2Vol && mAUDIO_2.BKUP) {
		decval = 0;

		if ((mikey_0.aud2Ctl & CLOCK_SEL) == LINKING) {
			if (mikey_0.aud1Misc & BORROW_OUT) decval = 1;
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
				mikey_0.aud2Misc |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud2Ctl & ENABLE_RELOAD) {
					mAUDIO_2.CURRENT += mAUDIO_2.BKUP + 1;
					if (mAUDIO_2.CURRENT & 0x80000000) mAUDIO_2.CURRENT = 0;
				}
				else {
					// Set timer done
					mikey_0.aud2Misc |= TIMER_DONE;
					mAUDIO_2.CURRENT = 0;
				}

				//
				// Update audio circuitry
				//
				mAUDIO_2.WAVESHAPER = GetLfsrNext(mAUDIO_2.WAVESHAPER);

				if (mikey_0.aud2Ctl & INTEGRATE) {
					s32 temp = mikey_0.aud2OutVal;
					if (mAUDIO_2.WAVESHAPER&0x0001)
						temp += mikey_0.aud2Vol;
					else
						temp -= mikey_0.aud2Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud2OutVal = (s8)temp;
				}
				else {
					if (mAUDIO_2.WAVESHAPER & 0x0001)
						mikey_0.aud2OutVal = mikey_0.aud2Vol;
					else
						mikey_0.aud2OutVal = -mikey_0.aud2Vol;
				}
			}
			else {
				mikey_0.aud2Misc &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mikey_0.aud2Misc |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mikey_0.aud2Misc &= ~(BORROW_OUT | BORROW_IN);
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
//	if ((mikey_0.aud3Ctl & ENABLE_COUNT) && !(mikey_0.aud3Misc & TIMER_DONE) && mikey_0.aud3Vol && mAUDIO_3.BKUP)
	if ((mikey_0.aud3Ctl & ENABLE_COUNT) && ((mikey_0.aud3Ctl & ENABLE_RELOAD) || !(mikey_0.aud3Misc & TIMER_DONE)) && mikey_0.aud3Vol && mAUDIO_3.BKUP) {
		decval = 0;

		if ((mikey_0.aud3Ctl & CLOCK_SEL) == LINKING) {
			if (mikey_0.aud2Misc & BORROW_OUT) decval = 1;
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
				mikey_0.aud3Misc |= BORROW_OUT;

				// Reload if neccessary
				if (mikey_0.aud3Ctl & ENABLE_RELOAD) {
					mAUDIO_3.CURRENT += mAUDIO_3.BKUP + 1;
					if (mAUDIO_3.CURRENT & 0x80000000) mAUDIO_3.CURRENT = 0;
				}
				else {
					// Set timer done
					mikey_0.aud3Misc |= TIMER_DONE;
					mAUDIO_3.CURRENT = 0;
				}

				//
				// Update audio circuitry
				//
				mAUDIO_3.WAVESHAPER = GetLfsrNext(mAUDIO_3.WAVESHAPER);

				if (mikey_0.aud3Ctl & INTEGRATE) {
					s32 temp = mikey_0.aud3OutVal;
					if (mAUDIO_3.WAVESHAPER & 0x0001)
						temp += mikey_0.aud3Vol;
					else
						temp -= mikey_0.aud3Vol;
					if (temp > 127) temp = 127;
					if (temp < -128) temp = -128;
					mikey_0.aud3OutVal = (s8)temp;
				}
				else {
					if (mAUDIO_3.WAVESHAPER & 0x0001)
						mikey_0.aud3OutVal = mikey_0.aud3Vol;
					else
						mikey_0.aud3OutVal = -mikey_0.aud3Vol;
				}
			}
			else {
				mikey_0.aud3Misc &= ~BORROW_OUT;
			}
			// Set carry in as we did a count
			mikey_0.aud3Misc |= BORROW_IN;
		}
		else {
			// Clear carry in as we didn't count
			// Clear carry out
			mikey_0.aud3Misc &= ~(BORROW_OUT | BORROW_IN);
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
