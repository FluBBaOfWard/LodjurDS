#ifdef __arm__

#include "Shared/nds_asm.h"
#include "ARMSuzy/ARMSuzy.i"
#include "ARMMikey/ARMMikey.i"

	.global gfxState
	.global gFlicker
	.global gTwitch
	.global gGfxMask
	.global GFX_DISPCNT
	.global GFX_BG0CNT
	.global GFX_BG1CNT
	.global EMUPALBUFF
	.global MAPPED_RGB
	.global frameTotal

	.global gfxInit
	.global gfxReset
	.global paletteInit
	.global gfxRefresh
	.global gfxEndFrame
	.global vblIrqHandler
	.global lnxSuzyRead
	.global lnxSuzyWrite
	.global updateLCDRefresh
	.global setScreenRefresh


	.global suzy_0


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

	bl suzyInit
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
	str r0,currentDest

	bl gfxWinInit

	ldr r0,=lodjurRenderCallback
	ldr r1,=lodjurFrameCallback
	ldr r2,=lynxRAM
	ldr r3,=gSOC
	ldrb r3,[r3]
	bl miVideoReset

	ldr r0,=gGammaValue
	ldr r1,=gContrastValue
	ldrb r0,[r0]
	ldrb r1,[r1]
	bl paletteInit				;@ Do palette mapping

	ldr suzptr,=suzy_0

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

	ldr r0,=0x3333				;@ WinIN0/1, BG0, BG1, SPR & COL inside Win0
	strh r0,[r1,#REG_WININ]
	mov r0,#0x002C				;@ WinOUT, Only BG2, BG3 & COL enabled outside Windows.
	strh r0,[r1,#REG_WINOUT]
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
lodjurFrameCallback:		;@ (void)
;@----------------------------------------------------------------------------
	mov r0,#0x06000000
	str r0,currentDest
	ldr r1,=gScreenUpdateRequired
	mov r0,#1
	strb r0,[r1]
	bx lr

;@----------------------------------------------------------------------------
lodjurRenderCallback:		;@ (UBYTE *ram, ULONG *palette, bool flip)
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	ldr r4,=MAPPED_RGB
	ldr r5,=PAL_CACHE
	mov r6,#16
palCacheLoop:
	subs r6,r6,#1
	ldr r3,[r1,r6,lsl#2]
	mov r3,r3,lsl#1
	ldrh r3,[r4,r3]
	str r3,[r5,r6,lsl#2]
	bne palCacheLoop

	ldr r1,currentDest

	mov r4,#GAME_WIDTH/2
	cmp r2,#0
	beq rendLoop
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

;@----------------------------------------------------------------------------
updateLCDRefresh:
	.type updateLCDRefresh STT_FUNC
;@----------------------------------------------------------------------------
	adr suzptr,suzy_0
	ldrb r1,[suzptr,#suzLCDVSize]
	b svRefW
;@----------------------------------------------------------------------------
setScreenRefresh:			;@ r0 in = WS scan line count.
	.type setScreenRefresh STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	mov r4,r0
	ldr r6,=8130				;@ SV scanline frequency = 8130Hz
	mov r0,r6,lsl#1
	mov r1,r4
	swi 0x090000				;@ Division r0/r1, r0=result, r1=remainder.
	movs r0,r0,lsr#1
	adc r0,r0,#0
	mov r5,r0
	bl setLCDFPS
	ldr r0,=emuSettings
	ldr r0,[r0]
	ands r0,r0,#1<<19
	moveq r0,#59
	subne r0,r5,#1
	ldr r1,=fpsNominal
	strb r0,[r1]

	ldr r0,=15734				;@ DS scanline frequency = 15734.3Hz
	mul r0,r4,r0				;@ DS scanline freq * WS scanlines
	mov r1,r6					;@ / WS scanline freq = DS scanlines.
	swi 0x090000				;@ Division r0/r1, r0=result, r1=remainder.
	ldr r1,=263
	sub r0,r1,r0
	cmp r0,#3
	movmi r0,#0
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

	adr suzptr,suzy_0
	ldr r0,GFX_BG0CNT
	str r0,[r6,#REG_BG0CNT]
	ldr r0,GFX_DISPCNT
	ldrb r1,[suzptr,#wsvLatchedDispCtrl]
//	tst r1,#0x01
//	biceq r0,r0,#0x0100			;@ Turn off Bg
	ldrb r2,gGfxMask
	bic r0,r0,r2,lsl#8
//	strh r0,[r6,#REG_DISPCNT]

	ldr r0,[suzptr,#windowData]
	strh r0,[r6,#REG_WIN0H]
	mov r0,r0,lsr#16
	strh r0,[r6,#REG_WIN0V]
	ldr r0,=0x3333				;@ WinIN0/1, BG0, BG1, SPR & COL inside Win0
	and r2,r1,#0x30
	cmp r2,#0x20
	biceq r0,r0,#0x0200
	cmp r2,#0x30
	biceq r0,r0,#0x0002
	strh r0,[r6,#REG_WININ]

	ldr r0,=emuSettings
	ldr r0,[r0]
	ands r0,r0,#1<<19
	beq exit75Hz
	ldr r0,=pauseEmulation
	ldrb r0,[r0]
	cmp r0,#0
	bne exit75Hz
	ldr r0,lcdSkip
	cmp r0,#0
	beq exit75Hz
hz75Start:
hz75Loop:
	ldrh r1,[r6,#REG_VCOUNT]
	cmp r1,#202
	bmi hz75Loop
	add r1,r1,r0			;@ Skip 55(?) scan lines for 75Hz.
	cmp r1,#260
	movpl r1,#260
	strh r1,[r6,#REG_VCOUNT]
exit75Hz:

	ldrb r0,frameDone
	cmp r0,#0
	beq nothingNew
	mov r0,#0
	strb r0,frameDone
nothingNew:

	blx scanKeys
	ldmfd sp!,{r4-r8,pc}


;@----------------------------------------------------------------------------
gfxRefresh:					;@ Called from C when changing scaling.
	.type gfxRefresh STT_FUNC
;@----------------------------------------------------------------------------
	adr suzptr,suzy_0
;@----------------------------------------------------------------------------
gfxEndFrame:				;@ Called just before screen end (~line 101)	(r0-r3 safe to use)
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}

//	ldr r0,tmpScroll			;@ Destination
//	bl copyScrollValues

	ldr r0,tmpScroll
	ldr r1,dmaScroll
	str r0,dmaScroll
	str r1,tmpScroll

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

tmpScroll:		.long SCROLLBUFF1
dmaScroll:		.long SCROLLBUFF2


gFlicker:		.byte 1
				.space 2
gTwitch:		.byte 0

gGfxMask:		.byte 0
frameDone:		.byte 0
				.byte 0,0
;@----------------------------------------------------------------------------
suzyReset0:		;@ r0=ram+LUTs
;@----------------------------------------------------------------------------
	adr suzptr,suzy_0
	b suzyReset
;@----------------------------------------------------------------------------
lnxSuzyRead:
	.type lnxSuzyRead STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,r12,lr}
	mov r0,r12
	adr suzptr,suzy_0
	bl suzRead
	ldmfd sp!,{r3,r12,lr}
	bx lr
;@----------------------------------------------------------------------------
lnxSuzyWrite:
	.type lnxSuzyWrite STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,r12,lr}
	mov r1,r0
	mov r0,r12
	adr suzptr,suzy_0
	bl suzWrite
	ldmfd sp!,{r3,r12,lr}
	bx lr

;@----------------------------------------------------------------------------
suzy_0:
	.space suzySize
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

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
