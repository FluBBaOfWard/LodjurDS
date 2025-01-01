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

#define mAUDIO_0 mikey_0.audio0
#define mAUDIO_1 mikey_0.audio1
#define mAUDIO_2 mikey_0.audio2
#define mAUDIO_3 mikey_0.audio3
#define mSTEREO mikey_0.stereo

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

void CMikie::UpdateSound(void) {
	int divide = 0;
	int decval = 0;
	u32 tmp;

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
