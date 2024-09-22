//
//  SVVideo.i
//  Watara Supervision video emulation for GBA/NDS.
//
//  Created by Fredrik Ahlström on 2004-11-30.
//  Copyright © 2004-2024 Fredrik Ahlström. All rights reserved.
//
;@ ASM header for the Watara Supervision video emulator

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

	suzptr		.req r12
						;@ SVVideo.s
	.struct 0
scanline:			.long 0		;@ These 3 must be first in state.
nextLineChange:		.long 0
lineState:			.long 0

windowData:			.long 0
suzyState:					;@
svvRegs:
svvLCDHSize:		.byte 0		;@ 0x00 LCD Horizontal Size
svvLCDVSize:		.byte 0		;@ 0x01 LCD Vertical Size
svvHScroll:			.byte 0		;@ 0x02 Horizontal Scroll
svvVScroll:			.byte 0		;@ 0x03 Vertical Scroll
svvMirr00:			.byte 0		;@ 0x04 Mirror of reg 0x00
svvMirr01:			.byte 0		;@ 0x05 Mirror of reg 0x01
svvMirr02:			.byte 0		;@ 0x06 Mirror of reg 0x02
svvMirr03:			.byte 0		;@ 0x07 Mirror of reg 0x03

wsvDMACBus:
wsvDMACBusLow:		.byte 0		;@ 0x08 DMA CBus Low
wsvDMACBusHigh:		.byte 0		;@ 0x09 DMA CBus High
wsvDMAVBus:
wsvDMAVBusLow:		.byte 0		;@ 0x0A DMA VBus Low
wsvDMAVBusHigh:		.byte 0		;@ 0x0B DMA VBus High
wsvDMALen:			.byte 0		;@ 0x0C DMA Length
wsvDMACtrl:			.byte 0		;@ 0x0D DMA Control

wsvPadding0:		.space 2	;@ 0x0E-0x0F ??

wsvCh1Freq:						;@ Channel 1 (Right only)
wsvCh1FreqLow:		.byte 0		;@ 0x10 Channel 1 Frequency Low
wsvCh1FreqHigh:		.byte 0		;@ 0x11 Channel 1 Frequency High
wsvCh1Ctrl:			.byte 0		;@ 0x12 Channel 1 Volume/Duty cycle
wsvCh1Len:			.byte 0		;@ 0x13 Channel 1 Length
wsvCh2Freq:						;@ Channel 2 (Left only)
wsvCh2FreqLow:		.byte 0		;@ 0x14 Channel 2 Frequency Low
wsvCh2FreqHigh:		.byte 0		;@ 0x15 Channel 2 Frequency High
wsvCh2Ctrl:			.byte 0		;@ 0x16 Channel 2 Volume/Duty cycle
wsvCh2Len:			.byte 0		;@ 0x17 Channel 2 Length

wsvCh3Adr:
wsvCh3AdrLow:		.byte 0		;@ 0x18 Channel 3 Address Low
wsvCh3AdrHigh:		.byte 0		;@ 0x19 Channel 3 Address High
wsvCh3Len:			.byte 0		;@ 0x1A Channel 3 Length
wsvCh3Ctrl:			.byte 0		;@ 0x1B Channel 3 Control
wsvCh3Trigg:		.byte 0		;@ 0x1C Channel 3 Trigger
wsvPadding1:		.space 3	;@ 0x1D - 0x1F ???

wsvController:		.byte 0		;@ 0x20 Controller
wsvLinkPortDDR:		.byte 0		;@ 0x21 Link Port DDR
wsvLinkPortData:	.byte 0		;@ 0x22 Link Port Data
wsvIRQTimer:		.byte 0		;@ 0x23 IRQ Timer
wsvTimerIRQReset:	.byte 0		;@ 0x24 Timer IRQ Reset
wsvSndDMAIRQReset:	.byte 0		;@ 0x25 Sound DMA IRQ Reset
wsvSystemControl:	.byte 0		;@ 0x26 System Control
wsvIRQStatus:		.byte 0		;@ 0x27 IRQ Status
wsvCh4FreqVol:		.byte 0		;@ 0x28 Channel 4 Frequency and volume
wsvCh4Len:			.byte 0		;@ 0x29 Channel 4 Length
wsvCh4Ctrl:			.byte 0		;@ 0x2A Channel 4 Control
wsvPadding2:		.byte 0		;@ 0x2B ???
wsvMirr028:			.byte 0		;@ 0x2C Mirror of Reg 0x28
wsvMirr029:			.byte 0		;@ 0x2D Mirror of Reg 0x29
wsvMirr02A:			.byte 0		;@ 0x2E Mirror of Reg 0x2A
wsvPadding3:		.byte 0		;@ 0x2F ???


;@----------------------------------------------------------------------------
wsvNMITimer:		.long 0
wsvTimerValue:		.long 0
sndDmaCounter:		.long 0		;@ Sound DMA Counter
sndDmaLength:		.long 0		;@ Sound DMA length

ch1Counter:			.long 0		;@ Ch1 Counter
ch2Counter:			.long 0		;@ Ch2 Counter
ch3Counter:			.long 0		;@ Ch3 Counter
ch4Counter:			.long 0		;@ Ch4 Counter
ch4LFSR:			.long 0		;@ Ch4 Noise LFSR
ch3Address:			.long 0		;@ Ch3 sample address (physical)
ch4Feedback:		.long 0		;@ Ch4 Noise Feedback

wsvNMIStatus:		.byte 0		;@ NMI Status
wsvLinkPortVal:		.byte 0		;@ Link Port Value
wsvSOC:				.byte 0		;@ HOWARD or HOWARD2
wsvLatchedDispCtrl:	.byte 0		;@ Latched Display Control
wsvLowBattery:		.byte 0
wsvPadding4:		.space 3

scrollLine: 		.long 0		;@ Last line scroll was updated.
suzyStateEnd:

nmiFunction:		.long 0		;@ NMI function
irqFunction:		.long 0		;@ IRQ function

dirtyTiles:			.space 4
gfxRAM:				.long 0		;@ 0x2000
scrollBuff:			.long 0

suzySize:

;@----------------------------------------------------------------------------

