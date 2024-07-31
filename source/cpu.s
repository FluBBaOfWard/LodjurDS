#ifdef __arm__

#include "ARM6502/M6502.i"
#include "ARM6502/M6502mac.h"

#define ALLOW_REFRESH_CHG	(1<<19)

#define CYCLE_PSL (256)

	.global waitMaskIn
	.global waitMaskOut

	.global run
	.global runScanLine
	.global runFrame
	.global cpuInit
	.global cpuReset

	.syntax unified
	.arm

#ifdef GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
run:						;@ Return after X frame(s)
	.type run STT_FUNC
;@----------------------------------------------------------------------------
	ldrh r0,waitCountIn
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountIn
	bxne lr
	stmfd sp!,{r4-r11,lr}

;@----------------------------------------------------------------------------
runStart:
;@----------------------------------------------------------------------------
	ldr r0,=joy0State
	ldr r0,[r0]
	ldr r1,joyClick
	eor r1,r1,r0
	and r1,r1,r0
	str r0,joyClick

	bl refreshEMUjoypads

	ldr m6502ptr,=m6502_0
	add r0,m6502ptr,#m6502Regs
	ldmia r0,{m6502nz-m6502pc,m6502zpage}	;@ Restore M6502 state

	ldr r0,=emuSettings
	ldr r0,[r0]
	tst r0,#ALLOW_REFRESH_CHG
	beq lxFrameLoop3DS
;@----------------------------------------------------------------------------
lxFrameLoop:
;@----------------------------------------------------------------------------
	mov r0,#CYCLE_PSL
	bl m6502RunXCycles
	ldr spxptr,=sphinx0
	bl wsvDoScanline
	cmp r0,#0
	bne lxFrameLoop
;@----------------------------------------------------------------------------
lxFrameLoopEnd:
	add r0,m6502ptr,#m6502Regs
	stmia r0,{m6502nz-m6502pc}	;@ Save 6502 state

	ldrh r0,waitCountOut
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountOut
	ldmfdeq sp!,{r4-r11,lr}		;@ Exit here if doing single frame:
	bxeq lr						;@ Return to rommenu()
	b runStart

;@----------------------------------------------------------------------------
lxFrameLoop3DS:
;@----------------------------------------------------------------------------
	mov r0,#CYCLE_PSL
	bl m6502RunXCycles
	ldr spxptr,=sphinx0
	bl wsvDoScanline
	ldr r1,scanLineCount3DS
	cmp r0,#0
	cmpeq r1,#2
	subsne r1,r1,#1
	moveq r1,#199				;@ (159 * 75) / 60 = 198,75
	str r1,scanLineCount3DS
	bne lxFrameLoop3DS
	b lxFrameLoopEnd
;@----------------------------------------------------------------------------
lynxCyclesPerScanline:	.long 0
scanLineCount3DS:	.long 199
joyClick:			.long 0
waitCountIn:		.byte 0
waitMaskIn:			.byte 0
waitCountOut:		.byte 0
waitMaskOut:		.byte 0

;@----------------------------------------------------------------------------
runScanLine:				;@ Return after 1 scanline
	.type runScanLine STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr m6502ptr,=m6502_0
	bl wsScanLine
	ldmfd sp!,{r4-r11,lr}
	bx lr
;@----------------------------------------------------------------------------
lxScanLine:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	mov r0,#CYCLE_PSL
	bl M6502RestoreAndRunXCycles
	add r0,m6502ptr,#m6502Regs
	stmia r0,{m6502nz-m6502pc}	;@ Save 6502 state
	ldmfd sp!,{lr}
	ldr spxptr,=sphinx0
	b wsvDoScanline
;@----------------------------------------------------------------------------
runFrame:					;@ Return after 1 frame
	.type runFrame STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr v30ptr,=V30OpTable
;@----------------------------------------------------------------------------
lxStepLoop:
;@----------------------------------------------------------------------------
	bl wsScanLine
	cmp r0,#0
	bne lxStepLoop
	bl lxScanLine
;@----------------------------------------------------------------------------

	ldmfd sp!,{r4-r11,lr}
	bx lr
;@----------------------------------------------------------------------------
cpuInit:					;@ Called by machineInit
;@----------------------------------------------------------------------------
	stmfd sp!,{v30ptr,lr}
	ldr v30ptr,=V30OpTable

	mov r0,#CYCLE_PSL
	str r0,v30MZCyclesPerScanline
	mov r0,v30ptr
	bl V30Init

	ldmfd sp!,{v30ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
cpuReset:					;@ Called by loadCart/resetGame, r0 = type
;@----------------------------------------------------------------------------
	stmfd sp!,{v30ptr,lr}
	mov r1,r0
	ldr v30ptr,=V30OpTable

	mov r0,v30ptr
	bl V30Reset
	ldr r0,=getInterruptVector
	str r0,[v30ptr,#v30IrqVectorFunc]

	ldmfd sp!,{v30ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
m6502_0:
	.space m6502Size
	.end
#endif // #ifdef __arm__
