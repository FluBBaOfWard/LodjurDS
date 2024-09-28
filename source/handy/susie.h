//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
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

class CSystem;

#define SUSIE_START		0xfc00
#define SUSIE_SIZE		0x100

#define LYNX_SCREEN_WIDTH	160
#define LYNX_SCREEN_HEIGHT	102

#define LINE_END		0x80

//
// Define button values
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
enum {
	background_shadow = 0,
	background_nocoll,
	boundary_shadow,
	boundary,
	normal,
	non_collide,
	x__or,shadow
	};
enum {math_finished = 0,math_divide,math_multiply,math_init_divide,math_init_multiply};

enum {sprite_background_shadow = 0,
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
			UBYTE	Low;
			UBYTE	High;
		}Byte;
		UWORD	Word;
	};
}UUWORD;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	Type:3;
			UBYTE	Reserved:1;
			UBYTE	Vflip:1;
			UBYTE	Hflip:1;
			UBYTE	PixelBits:2;
		}Bits;
		UBYTE	Byte;
	};
}TSPRCTL0;


typedef struct
{
	union
	{
		struct
		{
			UBYTE	StartLeft:1;
			UBYTE	StartUp:1;
			UBYTE	SkipSprite:1;
			UBYTE	ReloadPalette:1;
			UBYTE	ReloadDepth:2;
			UBYTE	Sizing:1;
			UBYTE	Literal:1;
		}Bits;
		UBYTE	Byte;
	};
}TSPRCTL1;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	Number:4;
			UBYTE	unused2:1;
			UBYTE	Collide:1;
			UBYTE	unused1:2;
		}Bits;
		UBYTE	Byte;
	};
}TSPRCOLL;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	unused:1;
			UBYTE	StopOnCurrent:1;
			UBYTE	ClearUnsafe:1;
			UBYTE	LeftHand:1;
			UBYTE	Vstretch:1;
			UBYTE	NoCollide:1;
			UBYTE	Accumulate:1;
			UBYTE	SignedMath:1;
		}Write;

		struct
		{
			UBYTE	Status:1;
			UBYTE	StopOnCurrent:1;
			UBYTE	UnsafeAccess:1;
			UBYTE	LeftHand:1;
			UBYTE	Vstretch:1;
			UBYTE	LastCarry:1;
			UBYTE	Mathbit:1;
			UBYTE	MathInProgress:1;
		}Read;
		UBYTE	Byte;
	};
}TSPRSYS;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	Ac4:1;
			UBYTE	Ac3:1;
			UBYTE	Ac2:1;
			UBYTE	Ac1:1;
			UBYTE	reserved:1;
			UBYTE	Fc3:1;
			UBYTE	Fc2:1;
			UBYTE	Fc1:1;
		}Bits;
		UBYTE	Byte;
	};
}TSPRINIT;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	Outside:1;
			UBYTE	Inside:1;
			UBYTE	Option2:1;
			UBYTE	Option1:1;
			UBYTE	Right:1;
			UBYTE	Left:1;
			UBYTE	Down:1;
			UBYTE	Up:1;
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
			UBYTE	Pause:1;
			UBYTE	Cart0IO:1;
			UBYTE	Cart1IO:1;
			UBYTE	spare:5;
		}Bits;
		UBYTE	Byte;
	};
}TSWITCHES;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	D;
			UBYTE	C;
			UBYTE	B;
			UBYTE	A;
		}Bytes;
		struct
		{
			UWORD	CD;
			UWORD	AB;
		}Words;
		ULONG	Long;
	};
}TMATHABCD;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	H;
			UBYTE	G;
			UBYTE	F;
			UBYTE	E;
		}Bytes;
		struct
		{
			UWORD	GH;
			UWORD	EF;
		}Words;
		ULONG	Long;
	};
}TMATHEFGH;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	M;
			UBYTE	L;
			UBYTE	K;
			UBYTE	J;
		}Bytes;
		struct
		{
			UWORD	LM;
			UWORD	JK;
		}Words;
		ULONG	Long;
	};
}TMATHJKLM;

typedef struct
{
	union
	{
		struct
		{
			UBYTE	P;
			UBYTE	N;
			UBYTE	xx1;
			UBYTE	xx2;
		}Bytes;
		struct
		{
			UWORD	NP;
			UWORD	xx1;
		}Words;
		ULONG	Long;
	};
}TMATHNP;


class CSusie : public CLynxMemObj
{
	public:
		CSusie(CSystem& parent);
		~CSusie();

		void	Reset(void);
		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
		ULONG	ReadCycle(void) {return 9;};
		ULONG	WriteCycle(void) {return 5;};
		ULONG	ObjectSize(void) {return SUSIE_SIZE;};

		void	SetButtonData(ULONG data) {mJOYSTICK.Byte = (UBYTE)data;mSWITCHES.Byte = (UBYTE)(data>>8);};
		ULONG	GetButtonData(void) {return mJOYSTICK.Byte+(mSWITCHES.Byte<<8);};

		ULONG	PaintSprites(void);

	private:
		void	DoMathDivide(void);
		void	DoMathMultiply(void);
		ULONG	LineInit(ULONG voff);
		ULONG	LineGetPixel(void);
		ULONG	LineGetBits(ULONG bits);

		void	ProcessPixel(ULONG hoff,ULONG pixel);
		void	WritePixel(ULONG hoff,ULONG pixel);
		ULONG	ReadPixel(ULONG hoff);
		void	WriteCollision(ULONG hoff,ULONG pixel);
		ULONG	ReadCollision(ULONG hoff);

	private:
		CSystem&	mSystem;

		UUWORD		mTMPADR;		// ENG
		UUWORD		mTILTACUM;		// ENG
		UUWORD		mHOFF;			// CPU
		UUWORD		mVOFF;			// CPU
		UUWORD		mVIDBAS;		// CPU
		UUWORD		mCOLLBAS;		// CPU
		UUWORD		mVIDADR;		// ENG
		UUWORD		mCOLLADR;		// ENG
		UUWORD		mSCBNEXT;		// SCB
		UUWORD		mSPRDLINE;		// SCB
		UUWORD		mHPOSSTRT;		// SCB
		UUWORD		mVPOSSTRT;		// SCB
		UUWORD		mSPRHSIZ;		// SCB
		UUWORD		mSPRVSIZ;		// SCB
		UUWORD		mSTRETCH;		// ENG
		UUWORD		mTILT;			// ENG
		UUWORD		mSPRDOFF;		// ENG
		UUWORD		mSPRVPOS;		// ENG
		UUWORD		mCOLLOFF;		// CPU
		UUWORD		mVSIZACUM;		// ENG
		UUWORD		mHSIZACUM;		// Keiths creation
		UUWORD		mHSIZOFF;		// CPU
		UUWORD		mVSIZOFF;		// CPU
		UUWORD		mSCBADR;		// ENG
		UUWORD		mPROCADR;		// ENG

		TMATHABCD	mMATHABCD;		// ENG
		TMATHEFGH	mMATHEFGH;		// ENG
		TMATHJKLM	mMATHJKLM;		// ENG
		TMATHNP		mMATHNP;		// ENG
		BOOL		mMATH_SIGNED;	// Keiths creation
		BOOL		mMATH_OFLOW;	// Keiths creation
		BOOL		mMATH_ACCUM;	// Keiths creation

		TSPRCTL0	mSPRCTL0;		// SCB
		TSPRCTL1	mSPRCTL1;		// SCB
		TSPRCOLL	mSPRCOLL;		// SCB

		TSPRSYS		mSPRSYS;		// CPU

		BOOL		mSUZYBUSEN;		// CPU

		TSPRINIT	mSPRINIT;		// CPU

		BOOL		mSPRGO;			// CPU

		UBYTE		mPenIndex[16];	// SCB

		// Line rendering related variables

		ULONG		mLineType;
		ULONG		mLineShiftRegCount;
		ULONG		mLineShiftReg;
		ULONG		mLineRepeatCount;
		ULONG		mLinePixel;
		ULONG		mLinePacketBitsLeft;

		ULONG		mCollision;

		UBYTE		*mRamPointer;

		ULONG		mLineBaseAddress;
		ULONG		mLineCollisionAddress;

		// Joystick switches

		TJOYSTICK	mJOYSTICK;
		TSWITCHES	mSWITCHES;
};


#endif
