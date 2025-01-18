#ifdef __arm__

#include "Shared/nds_asm.h"
#include "ARMSuzy/ARMSuzy.i"
#include "ARMMikey/ARMMikey.i"

	.global gFlicker
	.global gTwitch
	.global gGfxMask
	.global GFX_DISPCNT
	.global GFX_BG0CNT
	.global GFX_BG1CNT
	.global EMUPALBUFF
	.global MAPPED_RGB
	.global frameTotal
	.global suzy_0

	.global gfxInit
	.global gfxReset
	.global paletteInit
	.global gfxRefresh
	.global gfxEndFrame
	.global vblIrqHandler
	.global lnxSuzySetButtonData
	.global updateLCDRefresh
	.global setScreenRefresh
	.global lowerRefresh

	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
gfxInit:					;@ Called from machineInit
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	bl gfxWinInit

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
gfxReset:					;@ Called with CPU reset
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=gfxState
	mov r1,#5					;@ 5*4
	bl memclr_					;@ Clear GFX regs

	mov r0,#0x06000000
	add r0,r0,#(((256-GAME_HEIGHT)/2) * SCREEN_WIDTH * 2)
	add r0,r0,#SCREEN_WIDTH-GAME_WIDTH
	str r0,currentDest

	bl gfxWinInit

	ldr r0,=lodjurRenderCallback
	ldr r1,=gfxEndFrame
	ldr r2,=lynxRAM
	ldr r3,=gSOC
	ldrb r3,[r3]
	bl mikeyReset

	ldr r0,=lynxRAM
	bl suzyReset0

	ldr r0,=gGammaValue
	ldr r1,=gContrastValue
	ldrb r0,[r0]
	ldrb r1,[r1]
	bl paletteInit				;@ Do palette mapping

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
gfxWinInit:
;@----------------------------------------------------------------------------
	mov r1,#REG_BASE
	;@ Horizontal start-end
	ldr r0,=(((SCREEN_WIDTH-GAME_WIDTH)/2)<<8)+(SCREEN_WIDTH+GAME_WIDTH)/2
	strh r0,[r1,#REG_WIN0H]
	strh r0,[r1,#REG_WIN1H]
	;@ Vertical start-end
	ldr r0,=(((SCREEN_HEIGHT-GAME_HEIGHT)/2)<<8)+(SCREEN_HEIGHT+GAME_HEIGHT)/2
	strh r0,[r1,#REG_WIN0V]
	strh r0,[r1,#REG_WIN1V]

	ldr r0,=0x002C3333			;@ WinIN0/1, BG0, BG1, SPR & COL inside Win0
	str r0,[r1,#REG_WININ]		;@ WinOUT, Only BG2, BG3 & COL enabled outside Windows.
	bx lr
;@----------------------------------------------------------------------------
paletteInit:		;@ r0-r3 modified.
	.type paletteInit STT_FUNC
;@ Called by ui.c:  void paletteInit(gammaVal, contrast);
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,lr}
	mov r8,#30
	rsb r1,r1,#4
	mul r8,r1,r8
	mov r1,r0					;@ Gamma value = 0 -> 4
	mov r7,#0xF					;@ mask
	ldr r6,=MAPPED_RGB
	mov r4,#4096*2
	sub r4,r4,#2
noMap:							;@ Map 0000ggggrrrrbbbb  ->  0bbbbbgggggrrrrr
	and r0,r7,r4,lsr#5			;@ Blue ready
	bl gPrefix
	mov r5,r0,lsl#10

	and r0,r7,r4,lsr#9			;@ Green ready
	bl gPrefix
	orr r5,r5,r0,lsl#5

	and r0,r7,r4,lsr#1			;@ Red ready
	bl gPrefix
	orr r5,r5,r0
	orr r5,r5,#0x8000

	strh r5,[r6,r4]
	subs r4,r4,#2
	bpl noMap

	ldmfd sp!,{r4-r8,lr}
	bx lr

;@----------------------------------------------------------------------------
gPrefix:
	orr r0,r0,r0,lsl#4
	mov r2,r8
;@----------------------------------------------------------------------------
contrastConvert:	;@ Takes value in r0(0-0xFF), gamma in r1(0-4), contrast in r2(0-255) returns new value in r0=0x1F
;@----------------------------------------------------------------------------
	rsb r3,r2,#256
	mul r0,r3,r0
	add r0,r0,r2,lsl#7
	mov r0,r0,lsr#8
;@----------------------------------------------------------------------------
gammaConvert:	;@ Takes value in r0(0-0xFF), gamma in r1(0-4),returns new value in r0=0x1F
;@----------------------------------------------------------------------------
	rsb r2,r0,#0x100
	mul r3,r2,r2
	rsbs r2,r3,#0x10000
	rsb r3,r1,#4
	orr r0,r0,r0,lsl#8
	mul r2,r1,r2
	mla r0,r3,r0,r2
	movs r0,r0,lsr#13

	bx lr

;@----------------------------------------------------------------------------
lodjurRenderCallback:		;@ (u8 *ram, u32 *palette, bool flip, bool palChg)
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	ldr r5,=PAL_CACHE
	cmp r3,#0
	beq palCacheOk
	ldr r4,=MAPPED_RGB
	mov r6,#16
palCacheLoop:
	subs r6,r6,#1
	ldr r3,[r1,r6,lsl#2]
	mov r3,r3,lsl#1
	ldrh r3,[r4,r3]
	str r3,[r5,r6,lsl#2]
	bne palCacheLoop

palCacheOk:
	ldr r1,currentDest

	mov r4,#GAME_WIDTH/2
	cmp r2,#0
	bne rendLoopFlip
rendLoop:
	ldrb r2,[r0],#1
	and r3,r2,#0x0F
	and r2,r2,#0xF0
	ldr r3,[r5,r3,lsl#2]
	ldr r2,[r5,r2,lsr#2]
	orr r3,r2,r3,lsl#16
	str r3,[r1],#4
	subs r4,r4,#1
	bne rendLoop
	add r1,r1,#(SCREEN_WIDTH-GAME_WIDTH)*2
	str r1,currentDest
	ldmfd sp!,{r4-r6,pc}
rendLoopFlip:
	ldrb r2,[r0],#-1
	and r3,r2,#0x0F
	and r2,r2,#0xF0
	ldr r3,[r5,r3,lsl#2]
	ldr r2,[r5,r2,lsr#2]
	orr r3,r3,r2,lsl#16
	str r3,[r1],#4
	subs r4,r4,#1
	bne rendLoopFlip
	add r1,r1,#(SCREEN_WIDTH-GAME_WIDTH)*2
	str r1,currentDest
	ldmfd sp!,{r4-r6,pc}

;@----------------------------------------------------------------------------
updateLCDRefresh:
	.type updateLCDRefresh STT_FUNC
;@----------------------------------------------------------------------------
	ldr mikptr,=mikey_0
	ldrb r1,[mikptr,#mikPBkup]
	b miPBackupW
;@----------------------------------------------------------------------------
setScreenRefresh:			;@ r0 in = Lynx cycles per frame.
	.type setScreenRefresh STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	mov r4,r0
	ldr r6,=16000000			;@ Lynx main frequency = 16MHz
	mov r0,r6,lsl#1
	mov r1,r4
	swi 0x090000				;@ Division r0/r1, r0=result, r1=remainder.
	movs r0,r0,lsr#1
	adc r0,r0,#0
	mov r5,r0
	bl setLCDFPS
	ldr r0,=emuSettings
	ldr r0,[r0]
	ands r0,r0,#1<<19			;@ ALLOW_REFRESH_CHG
	moveq r0,#59
	subne r0,r5,#1
	ldr r1,=fpsNominal
	strb r0,[r1]

	ldr r0,=263*60				;@ Total scanlines for 1s
	mov r1,r5					;@ Lynx FPS.
	swi 0x090000				;@ Division r0/r1, r0=result, r1=remainder.
	ldr r1,=263
	sub r0,r1,r0
	cmp r0,#3
	movcc r0,#0
	str r0,lcdSkip

	ldmfd sp!,{r4-r6,lr}
	bx lr

;@----------------------------------------------------------------------------
vblIrqHandler:
	.type vblIrqHandler STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,lr}
	bl calculateFPS

	mov r6,#REG_BASE
	strh r6,[r6,#REG_DMA0CNT_H]	;@ DMA0 stop

	add r1,r6,#REG_DMA0SAD
	ldr r2,dmaScroll			;@ Setup DMA buffer for scrolling:
	ldmia r2!,{r4}				;@ Read
	add r3,r6,#REG_BG0HOFS		;@ DMA0 always goes here
	stmia r3,{r4}				;@ Set 1st value manually, HBL is AFTER 1st line
	ldr r4,=0x96600001			;@ noIRQ hblank 32bit repeat incsrc inc_reloaddst, 1 word
//	stmia r1,{r2-r4}			;@ DMA0 go

	add r1,r6,#REG_DMA3SAD

	ldr r2,=EMUPALBUFF			;@ DMA3 src, Palette transfer:
	mov r3,#BG_PALETTE			;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 256 words (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	ldr r0,GFX_BG0CNT
	str r0,[r6,#REG_BG0CNT]

	ldr r0,=emuSettings
	ldr r0,[r0]
	ands r0,r0,#1<<19			;@ ALLOW_REFRESH_CHG
	beq exitHzChg
	ldr r0,=pauseEmulation
	ldrb r0,[r0]
	cmp r0,#0
	bne exitHzChg
	ldr r0,lcdSkip
	cmp r0,#0
	strbmi r0,doLowRefresh
	ble exitHzChg
hz75Loop:
	ldrh r1,[r6,#REG_VCOUNT]
	cmp r1,#202
	bmi hz75Loop
	add r1,r1,r0			;@ Skip 55(?) scan lines for 75Hz.
	cmp r1,#260
	movpl r1,#260
	strh r1,[r6,#REG_VCOUNT]
exitHzChg:

	ldrb r0,frameDone
	cmp r0,#0
	beq nothingNew
	mov r0,#0
	strb r0,frameDone
nothingNew:

	blx scanKeys
	ldmfd sp!,{r4-r8,pc}

;@----------------------------------------------------------------------------
lowerRefresh:
;@----------------------------------------------------------------------------
	ldrsb r0,doLowRefresh
	cmp r0,#0
	bxpl lr
	adds r1,r0,#26
	movpl r1,#0
	strb r1,doLowRefresh
	movmi r0,#-26

	mov r2,#REG_BASE
	ldrh r1,[r2,#REG_VCOUNT]
	add r1,r1,r0
	cmp r1,#202
	movmi r1,#202
	strh r1,[r2,#REG_VCOUNT]
	bx lr

;@----------------------------------------------------------------------------
gfxRefresh:					;@ Called from C when changing scaling.
	.type gfxRefresh STT_FUNC
;@----------------------------------------------------------------------------
	ldr suzptr,=suzy_0
;@----------------------------------------------------------------------------
gfxEndFrame:				;@ Called just before screen end (~line 101)	(r0-r3 safe to use)
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}

	mov r0,#0x06000000
	add r0,r0,#(((256-GAME_HEIGHT)/2) * SCREEN_WIDTH * 2)
	add r0,r0,#SCREEN_WIDTH-GAME_WIDTH
	str r0,currentDest

	mov r0,#1
	strb r0,frameDone

	ldr r1,=fpsValue
	ldr r0,[r1]
	add r0,r0,#1
	str r0,[r1]

	ldr r1,frameTotal
	add r1,r1,#1
	str r1,frameTotal

	ldmfd sp!,{r3,lr}
	bx lr

;@----------------------------------------------------------------------------
frameTotal:			.long 0		;@ Let Gui.c see frame count for savestates

dmaScroll:		.long SCROLLBUFF2


gFlicker:		.byte 1
				.space 2
gTwitch:		.byte 0

gGfxMask:		.byte 0
frameDone:		.byte 0
doLowRefresh:	.byte 0
				.byte 0
;@----------------------------------------------------------------------------
suzyReset0:		;@ r0=ram+LUTs
;@----------------------------------------------------------------------------
	ldr suzptr,=suzy_0
	b suzyReset
;@----------------------------------------------------------------------------
lnxSuzySetButtonData:
	.type lnxSuzySetButtonData STT_FUNC
;@----------------------------------------------------------------------------
	ldr suzptr,=suzy_0
	b suzySetButtonData

;@----------------------------------------------------------------------------

gfxState:
currentDest:
	.long 0
	.long 0
	.long 0,0
lcdSkip:
	.long 0

GFX_DISPCNT:
	.long 0
GFX_BG0CNT:
	.short 0
GFX_BG1CNT:
	.short 0

#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 2
SCROLLBUFF1:
	.space 0x100*8				;@ Scrollbuffer.
SCROLLBUFF2:
	.space 0x100*8				;@ Scrollbuffer.
MAPPED_RGB:
	.space 0x2000				;@ 4096*2
EMUPALBUFF:
	.space 0x400
PAL_CACHE:
	.space 0x40					;@ 16*4

#ifdef NDS
	.section .dtcm, "ax", %progbits			;@ For the NDS
#elif GBA
	.section .bss				;@ This is IWRAM on GBA with devkitARM
#endif
;@----------------------------------------------------------------------------
suzy_0:
	.space suzySize
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
