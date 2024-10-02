//
//  SVVideo.h
//  Watara Supervision video emulation for GBA/NDS.
//
//  Created by Fredrik Ahlström on 2004-11-30.
//  Copyright © 2004-2024 Fredrik Ahlström. All rights reserved.
//

#ifndef SVVIDEO_HEADER
#define SVVIDEO_HEADER

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
//wsvState:
//wsvRegs:
	u8 wsvLCDXSize;				// 0x00 LCD X Size
	u8 wsvLCDYSize;				// 0x01 LCD Y Size
	u8 wsvXScroll;				// 0x02 X Scroll
	u8 wsvYScroll;				// 0x03 Y Scroll
	u8 wsvMirr00;				// 0x04 Mirror of reg 0x00
	u8 wsvMirr01;				// 0x05 Mirror of reg 0x01
	u8 wsvMirr02;				// 0x06 Mirror of reg 0x02
	u8 wsvMirr03;				// 0x07 Mirror of reg 0x03

	u8 wsvDMASrcLow;			// 0x08 DMA Source Low
	u8 wsvDMASrcHigh;			// 0x09 DMA Source High
	u8 wsvDMADstLow;			// 0x0A DMA Destination Low
	u8 wsvDMADstHigh;			// 0x0B DMA Destination High
	u8 wsvDMALen;				// 0x0C DMA Length
	u8 wsvDMACtrl;				// 0x0D DMA Control

	u8 wsvPadding0[2];			// 0x0E-0x0F ??

	u8 wsvCh1FreqLow;			// 0x10 Channel 1 Frequency Low (Right only)
	u8 wsvCh1FreqHigh;			// 0x11 Channel 1 Frequency High
	u8 wsvCh1Duty;				// 0x12 Channel 1 Duty cycle
	u8 wsvCh1Len;				// 0x13 Channel 1 Length
	u8 wsvCh2FreqLow;			// 0x14 Channel 2 Frequency Low (Left only)
	u8 wsvCh2FreqHigh;			// 0x15 Channel 2 Frequency High
	u8 wsvCh2Duty;				// 0x16 Channel 2 Duty cycle
	u8 wsvCh2Len;				// 0x17 Channel 2 Length

	u8 wsvCh3AdrLow;			// 0x18 Channel 3 Address Low
	u8 wsvCh3AdrHigh;			// 0x19 Channel 3 Address High
	u8 wsvCh3Len;				// 0x1A Channel 3 Length
	u8 wsvCh3Ctrl;				// 0x1B Channel 3 Control
	u8 wsvCh3Trigg;				// 0x1C Channel 3 Trigger
	u8 wsvPadding1[3];			// 0x1D - 0x1F ???

	u8 wsvController;			// 0x20 Controller
	u8 wsvLinkPortDDR;			// 0x21 Link Port DDR
	u8 wsvLinkPortData;			// 0x22 Link Port Data
	u8 wsvIRQTimer;				// 0x23 IRQ Timer
	u8 wsvTimerIRQReset;		// 0x24 Timer IRQ Reset
	u8 wsvSndDMAIRQReset;		// 0x25 Sound DMA IRQ Reset
	u8 wsvSystemControl;		// 0x26 System Control
	u8 wsvIRQStatus;			// 0x27 IRQ Status
	u8 wsvCh4FreqVol;			// 0x28 Channel 4 Frquency and volume
	u8 wsvCh4Len;				// 0x29 Channel 4 Length
	u8 wsvCh4Ctrl;				// 0x2A Channel 4 Control
	u8 wsvPadding2;				// 0x2B ???
	u8 wsvMirr028;				// 0x2C Mirror of Reg 0x28
	u8 wsvMirr029;				// 0x2D Mirror of Reg 0x29
	u8 wsvMirr02A;				// 0x2E Mirror of Reg 0x2A
	u8 wsvPadding3;				// 0x2F ???

//------------------------------
	u32 wsvNMITimer;
	u32 wsvTimerValue;
	u32 sndDmaSource;			// Original Sound DMA source address
	u32 sndDmaLength;			// Original Sound DMA length

	u32 ch1Counter;				// Ch1 Counter
	u32 ch2Counter;				// Ch2 Counter
	u32 ch3Counter;				// Ch3 Counter
	u32 ch4Counter;				// Ch4 Counter
	u32 ch4LFSR;				// Ch4 Noise LFSR
	u32 ch3Address;				// Ch3 sample address (physical)
	u32 ch4Feedback;			// Ch4 Noise Feedback

	u8 wsvNMIStatus;			// NMI pin out status
	u8 wsvLinkPortVal;			// Link Port Value
	u8 wsvSOC;					// HOWARD or HOWARD2
	u8 wsvLatchedDispCtrl;		// Latched Display Control
	u8 wsvLowBattery;
	u8 wsvPadding4[3];

	u32 scrollLine;

	void *nmiFunction;			// NMI callback
	void *irqFunction;			// IRQ callback

	u8 dirtyTiles[4];
	void *gfxRAM;
	u32 *scrollBuff;

} SUZY;

void svVideoReset(void *irqFunction(), void *ram, int soc);

/**
 * Saves the state of the chip to the destination.
 * @param  *destination: Where to save the state.
 * @param  *chip: The SUZY chip to save.
 * @return The size of the state.
 */
int svVideoSaveState(void *destination, const SUZY *chip);

/**
 * Loads the state of the chip from the source.
 * @param  *chip: The SUZY chip to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int svVideoLoadState(SUZY *chip, const void *source);

/**
 * Gets the state size of a SUZY chip.
 * @return The size of the state.
 */
int svVideoGetStateSize(void);

void svDoScanline(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SVVIDEO_HEADER
