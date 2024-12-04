//
//  ARMSuzy.i
//  Atari Lynx Suzy emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-09-22.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//
;@ ASM header for the Atari Lynx Suzy emulator

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
						;@ ARMSuzy.s
	.struct 0
suzyState:					;@
suzRegs:
suzTmpAdr:						;@ 0x00 Temporary Address
suzTmpAdrL:			.byte 0		;@ Low
suzTmpAdrH:			.byte 0		;@ High
suzTiltAcum:					;@ 0x02 Tilt Accumulator
suzTiltAcumL:		.byte 0		;@ Low
suzTiltAcumH:		.byte 0		;@ High
suzHOff:						;@ 0x04 Horizontal Offset
suzHOffL:			.byte 0		;@ Low
suzHOffH:			.byte 0		;@ High
suzVOff:						;@ 0x06 Vertical Offset
suzVOffL:			.byte 0		;@ Low
suzVOffH:			.byte 0		;@ High
suzVidBas:						;@ 0x08 Video Base
suzVidBasL:			.byte 0		;@ Low
suzVidBasH:			.byte 0		;@ High
suzCollBas:						;@ 0x0A Collision Base
suzCollBasL:		.byte 0		;@ Low
suzCollBasH:		.byte 0		;@ High
suzVidAdr:						;@ 0x0C Video Address
suzVidAdrL:			.byte 0		;@ Low
suzVidAdrH:			.byte 0		;@ High
suzCollAdr:						;@ 0x0E Collision Address
suzCollAdrL:		.byte 0		;@ Low
suzCollAdrH:		.byte 0		;@ High
suzSCBNext:						;@ 0x10 Sprite Control Block Next
suzSCBNextL:		.byte 0		;@ Low
suzSCBNextH:		.byte 0		;@ High
suzSprDLine:					;@ 0x12 Start of Sprite Data Line Address
suzSprDLineL:		.byte 0		;@ Low
suzSprDLineH:		.byte 0		;@ High
suzHPosStrt:					;@ 0x14 Starting Hpos
suzHPosStrtL:		.byte 0		;@ Low
suzHPosStrtH:		.byte 0		;@ High
suzVPosStrt:					;@ 0x16 Starting Vpos
suzVPosStrtL:		.byte 0		;@ Low
suzVPosStrtH:		.byte 0		;@ High
suzSprHSiz:						;@ 0x18 Sprite Horizontal Size
suzSprHSizL:		.byte 0		;@ Low
suzSprHSizH:		.byte 0		;@ High
suzSprVSiz:						;@ 0x1A Sprite Vertical Size
suzSprVSizL:		.byte 0		;@ Low
suzSprVSizH:		.byte 0		;@ High
suzStretch:						;@ 0x1C Horizontal Size Adder
suzStretchL:		.byte 0		;@ Low
suzStretchH:		.byte 0		;@ High
suzTilt:						;@ 0x1E Horizontal Position Adder
suzTiltL:			.byte 0		;@ Low
suzTiltH:			.byte 0		;@ High
suzSprDOff:						;@ 0x20 Offset to Next Sprite Data Line
suzSprDOffL:		.byte 0		;@ Low
suzSprDOffH:		.byte 0		;@ High
suzSprVPos:						;@ 0x22 Current Vertical Position
suzSprVPosL:		.byte 0		;@ Low
suzSprVPosH:		.byte 0		;@ High
suzCollOff:						;@ 0x24 Offset to Collision Depository
suzCollOffL:		.byte 0		;@ Low
suzCollOffH:		.byte 0		;@ High
suzVSizAcum:					;@ 0x26 Vertical Size Accumulator
suzVSizAcumL:		.byte 0		;@ Low
suzVSizAcumH:		.byte 0		;@ High
suzHSizOff:						;@ 0x28 Horizontal Size Offset
suzHSizOffL:		.byte 0		;@ Low
suzHSizOffH:		.byte 0		;@ High
suzVSizOff:						;@ 0x2A Vertical Size Offset
suzVSizOffL:		.byte 0		;@ Low
suzVSizOffH:		.byte 0		;@ High
suzSCBAdr:						;@ 0x2C Address of Current SCB
suzSCBAdrL:			.byte 0		;@ Low
suzSCBAdrH:			.byte 0		;@ High
suzProcAdr:						;@ 0x2E Current Spr Data Proc Address
suzProcAdrL:		.byte 0		;@ Low
suzProcAdrH:		.byte 0		;@ High
suzReserved0:		.space 0x22	;@ 0x30-0x51 Reserved
suzMathCD:
suzMathD:			.byte 0		;@ 0x52 Math D
suzMathC:			.byte 0		;@ 0x53 Math C
suzMathAB:
suzMathB:			.byte 0		;@ 0x54 Math B
suzMathA:			.byte 0		;@ 0x55 Math A
suzMathNP:
suzMathP:			.byte 0		;@ 0x56 Math P
suzMathN:			.byte 0		;@ 0x57 Math N
suzReserved1:		.space 0x08	;@ 0x58-0x5F Reserved
suzMathEFGH:
suzMathGH:
suzMathH:			.byte 0		;@ 0x60 Math H
suzMathG:			.byte 0		;@ 0x61 Math G
suzMathEF:
suzMathF:			.byte 0		;@ 0x62 Math F
suzMathE:			.byte 0		;@ 0x63 Math E
suzReserved2:		.space 0x08	;@ 0x64-0x6B Reserved
suzMathJKLM:
suzMathLM:
suzMathM:			.byte 0		;@ 0x6C Math M
suzMathL:			.byte 0		;@ 0x6D Math L
suzMathJK:
suzMathK:			.byte 0		;@ 0x6E Math K
suzMathJ:			.byte 0		;@ 0x6F Math J
suzReserved3:		.space 0x10	;@ 0x70-0x7F Reserved
suzSprCtl0:			.byte 0		;@ 0x80 Sprite Control 0
suzSprCtl1:			.byte 0		;@ 0x81 Sprite Control 1
suzSprColl:			.byte 0		;@ 0x82 Sprite Collision Number
suzSprInit:			.byte 0		;@ 0x83 Sprite Initialization
suzReserved4:		.space 0x04	;@ 0x84-0x87 Reserved
suzSuzyHRev:		.byte 0		;@ 0x88 Suzy Hardware Revision
suzSuzySRev:		.byte 0		;@ 0x89 Suzy Software Revision
suzReserved5:		.space 0x06	;@ 0x8A-0x8F Reserved
suzSuzyBusEn:		.byte 0		;@ 0x90 Suzy Bus Enable
suzSprGo:			.byte 0		;@ 0x91 Sprite Process Start Bit
suzSprSys:			.byte 0		;@ 0x92 System Control
suzReserved6:		.space 0x1D	;@ 0x93-0xAF Reserved
suzJoystick:		.byte 0		;@ 0xB0 Read Joystick and Switches
suzSwitches:		.byte 0		;@ 0xB1 Read Other Switches
suzRCart0:			.byte 0		;@ 0xB2 Read or write 8 bits of data
suzRCart1:			.byte 0		;@ 0xB3 Read or write 8 bits of data
suzReserved7:		.space 0x0C	;@ 0xB4-0xBF Reserved
suzLeds:			.byte 0		;@ 0xC0 Control LEDs
suzReserved8:		.space 0x01	;@ 0xC1 Reserved
suzPPortStat:		.byte 0		;@ 0xC2 Parallel Port Status
suzPPortData:		.byte 0		;@ 0xC3 Parallel Port Data
suzHowie:			.byte 0		;@ 0xC4 Read or write as appropriate
suzPadding:			.skip 3

;@----------------------------------------------------------------------------
suzSprSysStat:		.byte 0		;@ 0x92 System Control, read.
suzCollision:		.byte 0		;@ Collision value for current sprite.
wsvLatchedDispCtrl:	.byte 0		;@ Latched Display Control
suzSprCtl0_PixelBits:	.byte 0
suzLineType:		.byte 0	;@
suzLinePixel:		.byte 0	;@
suzPadding2:		.skip 2

suzLineRepeatCount:	.long 0	;@
suzLinePacketBitsLeft:	.long 0	;@
suzLineShiftRegCount:	.long 0	;@
suzLineShiftReg:	.long 0	;@

suzPenIndex:		.space 16	;@

mathAB_sign:		.long 0
mathCD_sign:		.long 0

sprSys_Busy:		.long 0
sprSys_UnsafeAccess:	.long 0
sprSys_Mathbit:		.long 0
sprSys_MathInProgress:	.long 0
everOnScreen:		.long 0

suzSprTypeFunc:		.long 0
suzLineBaseAddress:	.long 0		;@ Current dest line adr.
suzLineCollisionAddress:	.long 0		;@ Current collision dest line adr.
suzyCyclesUsed:		.long 0		;@ Cycles used to paint sprites.
suzyStateEnd:

dirtyTiles:			.space 4
suzyRAM:			.long 0		;@ 0x10000

suzySize:

;@----------------------------------------------------------------------------
