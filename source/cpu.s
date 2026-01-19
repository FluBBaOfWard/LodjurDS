#ifdef __arm__

#include "ARMMikey/ARMMikey.i"
#include "ARMSuzy/ARMSuzy.i"

	.global waitMaskIn
	.global waitMaskOut
	.global m6502_0
	.global mikey_0
	.global suzy_0

	.global run
	.global stepFrame
	.global stepInstruction
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
	bl lnxSuzySetButtonData

	ldr mikptr,=mikey_0
	bl mikSysUpdate
;@----------------------------------------------------------------------------

	ldrh r0,waitCountOut
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountOut
	ldmfdeq sp!,{r4-r11,lr}		;@ Exit here if doing single frame:
	bxeq lr						;@ Return to rommenu()
	b runStart

;@----------------------------------------------------------------------------
joyClick:			.long 0
waitCountIn:		.byte 0
waitMaskIn:			.byte 0
waitCountOut:		.byte 0
waitMaskOut:		.byte 0

// Executes one instruction and returns the number of cycles consumed
;@----------------------------------------------------------------------------
stepInstruction:					;@ Return after 1 instruction
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
	ldr mikptr,=mikey_0
	bl mikSysUpdate
;@----------------------------------------------------------------------------

	ldmfd sp!,{r4-r11,lr}
	bx lr

;@----------------------------------------------------------------------------
cpuInit:					;@ Called by machineInit, sets M65C02 or R65C02
	.type cpuInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	mov r4,r0

	ldr r0,=m6502_0
	bl m6502Init

	cmp r4,#HW_LYNX
	bne isRockwell

	ldr r0,=m6502_0
	ldr r2,=op1CycNop
	mov r4,#0xFF
m65C02Loop:
	mov r1,r4
	bl m6502PatchOpcode
	subs r4,r4,#8
	bpl m65C02Loop

isRockwell:
	ldmfd sp!,{r4,lr}
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
	.section .sbss				;@ This is DTCM on NDS with devkitARM
#elif GBA
	.section .bss				;@ This is IWRAM on GBA with devkitARM
#else
	.section .bss
#endif
	.align 2
;@----------------------------------------------------------------------------
m6502_0:
mikey_0:
	.space mikeySize
;@----------------------------------------------------------------------------
suzy_0:
	.space suzySize
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
