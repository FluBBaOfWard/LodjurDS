//
//  ARMSuzy.h
//  Atari Lynx Suzy emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-09-22.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//

#ifndef SUZY_HEADER
#define SUZY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define HW_AUTO       (0)
#define HW_LYNX       (1)
#define HW_LYNX_II    (2)
#define HW_SELECT_END (3)

#define SOC_HOWARD    (0)
#define SOC_HOWARD2   (1)

/** Game screen width in pixels */
#define GAME_WIDTH  (160)
/** Game screen height in pixels */
#define GAME_HEIGHT (102)

typedef	union {
	struct {
		u8	Low;
		u8	High;
	} Byte;
	u16	Word;
} U16_ST;

typedef union {
	struct {
		u8	D;
		u8	C;
	};
	struct {
		u8	B;
		u8	A;
	};
	struct {
		u8	P;
		u8	N;
	};
	u16	CD;
	u16	AB;
	u16	NP;
} MATH_U16;

typedef union {
	struct {
		u8	H;
		u8	G;
		u8	F;
		u8	E;
	};
	struct {
		u8	M;
		u8	L;
		u8	K;
		u8	J;
	};
	struct {
		u16	GH;
		u16	EF;
	};
	struct {
		u16	LM;
		u16	JK;
	};
	u32	Long;
} MATH_U32;

typedef struct {
//suzState:
//suzRegs:
	U16_ST tmpAdr;		// 0x00 Temporary Address
	U16_ST tiltAcum;	// 0x02 Accumulator for tilt value
	U16_ST hOff;		// 0x04 Offset to Horizontal edge of screen
	U16_ST vOff;		// 0x06 Offset to Vertical edge of screen
	U16_ST vidBas;		// 0x08 Base Address of Video Build Buffer
	U16_ST collBas;		// 0x0A Base Address of Collision Build Buffer
	U16_ST vidAdr;		// 0x0C Current Video Build Address
	U16_ST collAdr;		// 0x0E Current Collision Build Address
	U16_ST SCBNext;		// 0x10 Sprite Control Block Next
	U16_ST sprDLine;	// 0x12 Start of Sprite Data Line Address
	U16_ST hPosStrt;	// 0x14 Starting Hpos
	U16_ST vPosStrt;	// 0x16 Starting Vpos
	U16_ST sprHSiz;		// 0x18 Srite Horizontal Size
	U16_ST sprVSiz;		// 0x1A Srite Vertical Size
	U16_ST stretch;		// 0x1C Horizontal Size Adder
	U16_ST tilt;		// 0x1E Horizontal Position Adder
	U16_ST sprDOff;		// 0x20 Offset to Next Sprite Data Line
	U16_ST sprVPos;		// 0x22 Current Vertical Position
	U16_ST collOff;		// 0x24 Offset to Collision Depository
	U16_ST vSizAcum;	// 0x26 Vertical Size Accumulator
	U16_ST hSizOff;		// 0x28 Horizontal Size Offset
	U16_ST vSizOff;		// 0x2A Vertical Size Offset
	U16_ST SCBAdr;		// 0x2C Address of Current SCB
	U16_ST procAdr;		// 0x2E Current Spr Data Proc Address
	u8 reserved0[0x22];	// 0x30-0x51 Reserved
	MATH_U16 mathCD;	// 0x52 Math D & C
	MATH_U16 mathAB;	// 0x54 Math B & A
	MATH_U16 mathNP;	// 0x56 Math P & N
	u8 reserved1[0x08];	// 0x58-0x5F Reserved
	MATH_U32 mathEFGH;	// 0x60 Math H, G, F & E
	u8 reserved2[0x08];	// 0x64-0x6B Reserved
	MATH_U32 mathJKLM;	// 0x6C Math M, L, K & J
	u8 reserved3[0x10];	// 0x70-0x7F Reserved
	u8 sprCtl0;			// 0x80 Sprite Control 0
	u8 sprCtl1;			// 0x81 Sprite Control 1
	u8 sprColl;			// 0x82 Sprite Collision Number
	u8 sprInit;			// 0x83 Sprite Initialization
	u8 reserved4[0x04];	// 0x84-0x87 Reserved
	u8 suzyHRev;		// 0x88 Suzy Hardware Revision
	u8 suzySRev;		// 0x89 Suzy Software Revision
	u8 reserved5[0x06];	// 0x8A-0x8F Reserved
	u8 suzyBusEn;		// 0x90 Suzy Bus Enable
	u8 sprGo;			// 0x91 Sprite Process Start Bit
	u8 sprSys;			// 0x92 System Control
	u8 reserved6[0x1D];	// 0x93-0xAF Reserved
	u8 joystick;		// 0xB0 Read Joystick and Switches
	u8 switches;		// 0xB1 Read Other Switches
	u8 rCart0;			// 0xB2 Read or write 8 bits of data
	u8 rCart1;			// 0xB3 Read or write 8 bits of data
	u8 reserved7[0x0C];	// 0xB4-0xBF Reserved
	u8 leds;			// 0xC0 Control LEDs
	u8 reserved8[0x01];	// 0xC1 Reserved
	u8 pPortStat;		// 0xC2 Parallel Port Status
	u8 pPortData;		// 0xC3 Parallel Port Data
	u8 howie;			// 0xC4 Read or write as appropriate
	u8 padding[3];

//------------------------------
	u8 sprSysStat;		// 0x92 System Control, read.
	u8 collision;		// Collision value for current sprite.
	u8 wsvLatchedDispCtrl;		// Latched Display Control
	u8 sprCtl0_PixelBits;
	u8 lineType;
	u8 linePixel;
	u8 padding2[2];

	// Line rendering related variables
	u32 lineRepeatCount;
	u32 linePacketBitsLeft;
	u32 lineShiftRegCount;
	u32 lineShiftReg;

	u8 penIndex[16];

	int mathAB_sign;
	int mathCD_sign;

	int sprSys_Busy;
	int sprSys_UnsafeAccess;
	int sprSys_Mathbit;
	int sprSys_MathInProgress;
	int everOnScreen;

	void (*sprTypeFunc)(void);
	u32 lineBaseAddress;
	u32 lineCollisionAddress;
	u32 cyclesUsed;				// Cycles used to paint sprites.

	u8 dirtyTiles[4];
	u8 *suzyRAM;

} SUZY;

void suzyReset(void *irqFunction(), void *ram, int soc);

/**
 * Saves the state of the chip to the destination.
 * @param  *destination: Where to save the state.
 * @param  *chip: The SUZY chip to save.
 * @return The size of the state.
 */
int suzySaveState(void *destination, const SUZY *chip);

/**
 * Loads the state of the chip from the source.
 * @param  *chip: The SUZY chip to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int suzyLoadState(SUZY *chip, const void *source);

/**
 * Gets the state size of a SUZY chip.
 * @return The size of the state.
 */
int suzyGetStateSize(void);

u32 suzPaintSprites(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SUZY_HEADER
