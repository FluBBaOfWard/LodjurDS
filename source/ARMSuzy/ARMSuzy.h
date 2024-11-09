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

typedef struct {
	u32 scanline;
	u32 nextLineChange;
	u32 lineState;

	u32 windowData;
//suzState:
//suzRegs:
	u16 tmpAdr;			// 0x00 Temporary Address
	u16 tiltAcum;		// 0x02 Tilt Accumulator
	u16 hOff;			// 0x04 Horizontal Offset
	u16 vOff;			// 0x06 Vertical Offset
	u16 vidBas;			// 0x08 Video Base
	u16 collBas;		// 0x0A Collision Base
	u16 vidAdr;			// 0x0C Video Address
	u16 collAdr;		// 0x0C Collision Address
	u16 SCBNext;		// 0x10 Sprite Control Block Next
	u16 sprDLine;		// 0x12 Start of Sprite Data Line Address
	u16 hPosStrt;		// 0x14 Starting Hpos
	u16 vPosStrt;		// 0x16 Starting Vpos
	u16 sprHSiz;		// 0x18 Srite Horizontal Size
	u16 sprVSiz;		// 0x1A Srite Vertical Size
	u16 stretch;		// 0x1C Horizontal Size Adder
	u16 tilt;			// 0x1E Horizontal Position Adder
	u16 sprDOff;		// 0x20 Offset to Next Sprite Data Line
	u16 sprVPos;		// 0x22 Current Vertical Position
	u16 collOff;		// 0x24 Offset to Collision Depository
	u16 vSizAcum;		// 0x26 Vertical Size Accumulator
	u16 hSizOff;		// 0x28 Horizontal Size Offset
	u16 vSizOff;		// 0x2A Vertical Size Offset
	u16 SCBAdr;			// 0x2C Address of Current SCB
	u16 procAdr;		// 0x2E Current Spr Data Proc Address
	u8 reserved[0x22];	// 0x30-0x51 Reserved
	u8 mathD;			// 0x52 Math D
	u8 mathC;			// 0x53 Math C
	u8 mathB;			// 0x54 Math B
	u8 mathA;			// 0x55 Math A
	u8 mathP;			// 0x56 Math P
	u8 MathN;			// 0x57 Math N
	u8 reserved1[0x08];	// 0x58-0x5F Reserved
	u8 mathH;			// 0x60 Math H
	u8 mathF;			// 0x61 Math G
	u8 mathG;			// 0x62 Math F
	u8 mathE;			// 0x63 Math E
	u8 reserved2[0x08];	// 0x64-0x6B Reserved
	u8 mathM;			// 0x6C Math M
	u8 mathL;			// 0x6D Math L
	u8 mathK;			// 0x6E Math K
	u8 mathJ;			// 0x6F Math J
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
	u8 reserved7;		// 0xB4-0xBF Reserved
	u8 leds;			// 0xC0 Read Joystick and Switches
	u8 reserved8[0x01];	// 0xC1 Reserved
	u8 pPortStat;		// 0xC2 Parallel Port Status
	u8 pPortData;		// 0xC3 Parallel Port Data
	u8 howie;			// 0xC4 Read or write as appropriate

	u8 suzLCDVSize;		// 0x2F ???
	u8 suzPadding[2];

//------------------------------
	u8 wsvLatchedDispCtrl;		// Latched Display Control
	u8 suzPadding4[3];

	u32 scrollLine;

	u8 dirtyTiles[4];
	void *gfxRAM;
	u32 *scrollBuff;

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SUZY_HEADER
