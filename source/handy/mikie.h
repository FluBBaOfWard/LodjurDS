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
#include "../Gfx.h"

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

#define UART_TX_INACTIVE	0x80000000
#define UART_RX_INACTIVE	0x80000000
#define UART_BREAK_CODE		0x00008000
#define	UART_MAX_RX_QUEUE	32
#define UART_TX_TIME_PERIOD	(11)
#define UART_RX_TIME_PERIOD	(11)
#define UART_RX_NEXT_DELAY	(44)

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE unused:4;
			UBYTE Colour:1;
			UBYTE FourColour:1;
			UBYTE Flip:1;
			UBYTE DMAEnable:1;
#else
			UBYTE DMAEnable:1;
			UBYTE Flip:1;
			UBYTE FourColour:1;
			UBYTE Colour:1;
			UBYTE unused:4;
#endif
		} Bits;
		UBYTE Byte;
	};
}TDISPCTL;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE unused:8;
			UBYTE unused2:8;
			UBYTE unused3:4;
			UBYTE Blue:4;
			UBYTE Red:4;
			UBYTE Green:4;
#else
			UBYTE Green:4;
			UBYTE Red:4;
			UBYTE Blue:4;
#endif
		} Colours;
		ULONG Index;
	};
}TPALETTE;

class CMikie : public CLynxBase
{
	public:
		CMikie(CSystem& parent);
		~CMikie();

		void	Reset(void);

		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
		ULONG	ReadCycle(void) {return 5;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG	ObjectSize(void) {return MIKIE_SIZE;};
		void	PresetForHomebrew(void);
		ULONG	GetLfsrNext(ULONG current);

		void	ComLynxCable(int status);
		void	ComLynxRxData(int data);
		void	ComLynxTxLoopback(int data);
		void	ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref);

		void	DisplaySetAttributes(void (*DisplayCallback)(void), void (*mpRenderCallback)(UBYTE *ram, ULONG *palette, bool flip));

		void	BlowOut(void);
		void	ResetTimer(MTIMER& timer);
		void	ResetAudio(MAUDIO& audio);

		ULONG	DisplayRenderLine(void);
		ULONG	DisplayEndOfFrame(void);

		inline void SetCPUSleep(void) {gSystemCPUSleep=TRUE;};
		inline void ClearCPUSleep(void) {gSystemCPUSleep=FALSE;};

		void	Update(void);

	private:
		CSystem		&mSystem;

		// Hardware storage
		
		ULONG		mDisplayAddress;
		ULONG		mAudioInputComparator;
		ULONG		mTimerStatusFlags;
		ULONG		mTimerInterruptMask;

		TPALETTE	mPalette[16];

		ULONG		mIODAT;
		ULONG		mIODIR;
		ULONG		mIODAT_REST_SIGNAL;

		ULONG		mDISPCTL_DMAEnable;
		ULONG		mDISPCTL_Flip;
		ULONG		mDISPCTL_FourColour;
		ULONG		mDISPCTL_Colour;

//		MTIMER		mTIM_0;
//		MTIMER		mTIM_1;
//		MTIMER		mTIM_2;
//		MTIMER		mTIM_3;
//		MTIMER		mTIM_4;
//		MTIMER		mTIM_5;
//		MTIMER		mTIM_6;
//		MTIMER		mTIM_7;

		MAUDIO		mAUDIO_0;
		MAUDIO		mAUDIO_1;
		MAUDIO		mAUDIO_2;
		MAUDIO		mAUDIO_3;

		ULONG		mSTEREO;

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
		unsigned int mUART_Rx_output_ptr =0;
		int			mUART_Rx_waiting;
		int			mUART_Rx_framing_error;
		int			mUART_Rx_overun_error;

		//
		// Screen related
		//

		ULONG		mCurrentBuffer;
	
		UBYTE		*mpRamPointer;
		ULONG		mLynxLine;
		ULONG		mLynxLineDMACounter;
		ULONG		mLynxAddr;

		void		(*mpDisplayCallback)(void);
		void		(*mpRenderCallback)(UBYTE *ram, ULONG *palette, bool flip);
};


#endif
