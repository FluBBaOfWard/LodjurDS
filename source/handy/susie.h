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
//	                  Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Susie object header file                                                 //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the Suzy class    //
// which provides math and sprite support to the emulator                   //
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

#ifndef SUSIE_H
#define SUSIE_H

#ifdef TRACE_SUSIE

#define TRACE_SUSIE0(msg)					_RPT1(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_SUSIE1(msg,arg1)				_RPT2(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_SUSIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_SUSIE0(msg)
#define TRACE_SUSIE1(msg,arg1)
#define TRACE_SUSIE2(msg,arg1,arg2)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)

#endif

class CSystem;

#define SUSIE_START		0xfc00
#define SUSIE_SIZE		0x100

#define LYNX_SCREEN_WIDTH	160
#define LYNX_SCREEN_HEIGHT	102

#define LINE_END		0x80

//
// Define Lynx button values
//

#define BUTTON_A		0x0001
#define BUTTON_B		0x0002
#define BUTTON_OPT2		0x0004
#define BUTTON_OPT1		0x0008
#define BUTTON_LEFT		0x0010
#define BUTTON_RIGHT	0x0020
#define BUTTON_UP		0x0040
#define BUTTON_DOWN		0x0080
#define BUTTON_PAUSE	0x0100


enum {line_error=0,line_abs_literal,line_literal,line_packed};
enum {math_finished=0,math_divide,math_multiply,math_init_divide,math_init_multiply};

enum {sprite_background_shadow=0,
	  sprite_background_noncollide,
	  sprite_boundary_shadow,
	  sprite_boundary,
	  sprite_normal,
	  sprite_noncollide,
	  sprite_xor_shadow,
	  sprite_shadow};

// Define register typdefs

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE	Up:1;
			UBYTE	Down:1;
			UBYTE	Left:1;
			UBYTE	Right:1;
			UBYTE	Option1:1;
			UBYTE	Option2:1;
			UBYTE	Inside:1;
			UBYTE	Outside:1;
#else
			UBYTE	Outside:1;
			UBYTE	Inside:1;
			UBYTE	Option2:1;
			UBYTE	Option1:1;
			UBYTE	Right:1;
			UBYTE	Left:1;
			UBYTE	Down:1;
			UBYTE	Up:1;
#endif
		}Bits;
		UBYTE	Byte;
	};
}TJOYSTICK;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE	spare:5;
			UBYTE	Cart1IO:1;
			UBYTE	Cart0IO:1;
			UBYTE	Pause:1;
#else
			UBYTE	Pause:1;
			UBYTE	Cart0IO:1;
			UBYTE	Cart1IO:1;
			UBYTE	spare:5;
#endif
		}Bits;
		UBYTE	Byte;
	};
}TSWITCHES;

class CSusie : public CLynxBase
{
	public:
		CSusie(CSystem& parent);
		~CSusie();

		void	Reset(void);

		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
//		ULONG	ReadCycle(void) {return 9;};
//		ULONG	WriteCycle(void) {return 5;};

//		void	SetButtonData(ULONG data) {mJOYSTICK.Byte=(UBYTE)data;mSWITCHES.Byte=(UBYTE)(data>>8);};

		ULONG	PaintSprites(void);

	private:
		CSystem&	mSystem;

		int			mSPRSYS_Busy;
		int			mSPRSYS_UnsafeAccess;
		int			mSPRSYS_LastCarry;
		int			mSPRSYS_MathInProgress;

		// Joystick switches
//		TJOYSTICK	mJOYSTICK;
//		TSWITCHES	mSWITCHES;
};

#endif
