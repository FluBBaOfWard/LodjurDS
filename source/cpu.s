#ifdef __arm__

#include "ARMMikey/ARM6502/M6502.i"
#include "ARMSuzy/ARMSuzy.i"

#define CYCLE_PSL (246*2)

	.global waitMaskIn
	.global waitMaskOut
	.global m6502_0

	.global run
	.global stepFrame
	.global stepInstruction
	.global cpuSetIrqPin
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
	ldr r0,=EMUinput
	ldr r0,[r0]
	ldr r3,joyClick
	eor r3,r3,r0
	and r3,r3,r0
	str r0,joyClick

	bl refreshEMUjoypads

	ldr m6502ptr,=m6502_0
	add r1,m6502ptr,#m6502Regs
	ldmia r1,{m6502nz-m6502pc,m6502zpage}	;@ Restore M6502 state
;@----------------------------------------------------------------------------
svFrameLoop:
;@----------------------------------------------------------------------------
	mov r0,#CYCLE_PSL
	bl m6502RunXCycles
	ldr suzptr,=suzy_0
	bl svDoScanline
	cmp r0,#0
	bne svFrameLoop

;@----------------------------------------------------------------------------
	add r0,m6502ptr,#m6502Regs
	stmia r0,{m6502nz-m6502pc}	;@ Save M6502 state

	ldrh r0,waitCountOut
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountOut
	ldmfdeq sp!,{r4-r11,lr}		;@ Exit here if doing single frame:
	bxeq lr						;@ Return to rommenu()
	b runStart

;@----------------------------------------------------------------------------
m6502CyclesPerScanline:	.long 0
joyClick:			.long 0
waitCountIn:		.byte 0
waitMaskIn:			.byte 0
waitCountOut:		.byte 0
waitMaskOut:		.byte 0

;@----------------------------------------------------------------------------
stepInstruction:					;@ Return after 1 instruction
	.type stepInstruction STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr m6502ptr,=m6502_0
;@----------------------------------------------------------------------------
	mov r0,#8
	bl m6502RestoreAndRunXCycles
	mov r0,cycles,asr#CYC_SHIFT
	rsb r0,r0,#8
;@----------------------------------------------------------------------------
	add r1,m6502ptr,#m6502Regs
	stmia r1,{m6502nz-m6502pc}	;@ Save M6502 state

	ldmfd sp!,{r4-r11,lr}
	bx lr
;@----------------------------------------------------------------------------
stepFrame:					;@ Return after 1 frame
	.type stepFrame STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr m6502ptr,=m6502_0
	add r1,m6502ptr,#m6502Regs
	ldmia r1,{m6502nz-m6502pc,m6502zpage}	;@ Restore M6502 state
;@----------------------------------------------------------------------------
lxStepLoop:
;@----------------------------------------------------------------------------
	mov r0,#CYCLE_PSL
	bl m6502RunXCycles
	ldr suzptr,=suzy_0
	bl svDoScanline
	cmp r0,#0
	bne lxStepLoop
;@----------------------------------------------------------------------------
	add r0,m6502ptr,#m6502Regs
	stmia r0,{m6502nz-m6502pc}	;@ Save M6502 state

	ldmfd sp!,{r4-r11,lr}
	bx lr

;@----------------------------------------------------------------------------
cpuSetIrqPin:
	.type cpuSetIrqPin STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{m6502ptr,lr}
	ldr m6502ptr,=m6502_0
	bl m6502SetIRQPin
	ldmfd sp!,{m6502ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
cpuInit:					;@ Called by machineInit
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	mov r0,#CYCLE_PSL
	str r0,m6502CyclesPerScanline
	ldr r0,=m6502_0
	bl m6502Init

	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
cpuReset:					;@ Called by loadCart/resetGame
	.type cpuReset STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=m6502_0
	bl m6502Reset

	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
#ifdef NDS
	.section .dtcm, "ax", %progbits			;@ For the NDS
#elif GBA
	.section .iwram, "ax", %progbits		;@ For the GBA
#else
	.section .text
#endif
	.align 2
;@----------------------------------------------------------------------------
m6502_0:
	.space m6502Size
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
