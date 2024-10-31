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

#ifndef MIKIE_H
#define MIKIE_H

//#define	TRACE_MIKIE

#ifdef TRACE_MIKIE

#define TRACE_MIKIE0(msg)					_RPT1(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_MIKIE1(msg,arg1)				_RPT2(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_MIKIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_MIKIE0(msg)
#define TRACE_MIKIE1(msg,arg1)
#define TRACE_MIKIE2(msg,arg1,arg2)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)

#endif

#include "nds.h"
#include "../Cpu.h"

#define gSystemCycleCount mikey_0.systemCycleCount
#define gNextTimerEvent mikey_0.nextTimerEvent
#define mIODAT_REST_SIGNAL mikey_0.iodatRestSignal

class CSystem;

#define UART_TX_INACTIVE	0x80000000
#define UART_RX_INACTIVE	0x80000000
#define UART_BREAK_CODE		0x00008000
#define	UART_MAX_RX_QUEUE	32
#define UART_TX_TIME_PERIOD	(11)
#define UART_RX_TIME_PERIOD	(11)
#define UART_RX_NEXT_DELAY	(44)

class CMikie : public CLynxBase
{
	public:
		CMikie(CSystem& parent);
		~CMikie();

		void	Reset(void);

		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
		void	PresetForHomebrew(void);
		ULONG	GetLfsrNext(ULONG current);

		void	ComLynxCable(int status);
		void	ComLynxRxData(int data);
		void	ComLynxTxLoopback(int data);
		void	ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref);

		void	ResetAudio(MAUDIO& audio);

		void	UpdateTimer4(void);
		void	UpdateSound(void);

	private:
		CSystem		&mSystem;

		//
		// Serial related variables
		//
		ULONG		mUART_RX_IRQ_ENABLE;
		ULONG		mUART_TX_IRQ_ENABLE;

		ULONG		mUART_RX_COUNTDOWN;
		ULONG		mUART_TX_COUNTDOWN;

		ULONG		mUART_SENDBREAK;
		ULONG		mUART_TX_DATA;
		ULONG		mUART_RX_DATA;
		ULONG		mUART_RX_READY;

		ULONG		mUART_PARITY_ENABLE;
		ULONG		mUART_PARITY_EVEN;

		int			mUART_CABLE_PRESENT;
		void		(*mpUART_TX_CALLBACK)(int data,ULONG objref);
		ULONG		mUART_TX_CALLBACK_OBJECT;

		int			mUART_Rx_input_queue[UART_MAX_RX_QUEUE];
		unsigned int mUART_Rx_input_ptr;
		unsigned int mUART_Rx_output_ptr = 0;
		int			mUART_Rx_waiting;
		int			mUART_Rx_framing_error;
		int			mUART_Rx_overun_error;

};


#endif
