#ifdef __arm__

#include "Shared/EmuMenu.i"

	.global ioReset
	.global convertInput
	.global refreshEMUjoypads
	.global ioSaveState
	.global ioLoadState
	.global ioGetStateSize

	.global joy0_R

	.global joyCfg
	.global EMUinput

	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
ioReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
ioSaveState:				;@ In r0=destination. Out r0=size.
	.type   ioSaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldmfd sp!,{lr}
	mov r0,#0x100
	bx lr
;@----------------------------------------------------------------------------
ioLoadState:				;@ In r0=source. Out r0=size.
	.type   ioLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldmfd sp!,{lr}
;@----------------------------------------------------------------------------
ioGetStateSize:				;@ Out r0=state size.
	.type   ioGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r0,#0x100
	bx lr

;@----------------------------------------------------------------------------
convertInput:			;@ Convert from device keys to target r0=input/output
	.type convertInput STT_FUNC
;@----------------------------------------------------------------------------
	mvn r1,r0
	tst r1,#KEY_L|KEY_R				;@ Keys to open menu
	orreq r0,r0,#KEY_OPEN_MENU
	bx lr
;@----------------------------------------------------------------------------
refreshEMUjoypads:			;@ Call every frame
;@----------------------------------------------------------------------------

		ldr r0,=frameTotal
		ldr r0,[r0]
		movs r0,r0,lsr#2		;@ C=frame&2 (autofire alternates every other frame)
	ldr r4,EMUinput
	mov r3,r4
	and r0,r4,#0xf0
		ldr r2,joyCfg
		andcs r3,r3,r2
		tstcs r3,r3,lsr#10		;@ NDS L?
		andcs r3,r3,r2,lsr#16
	adr r1,rlud2rldu
	ldrb r0,[r1,r0,lsr#4]

	and r1,r4,#0x08				;@ NDS Start
	orr r0,r0,r1,lsl#5			;@ Lynx Start
	tst r4,#0x400				;@ NDS X button
	orrne r0,r0,#0x04			;@ Lynx Option1
	tst r4,#0x800				;@ NDS Y button
	orrne r0,r0,#0x08			;@ Lynx Option2

	ands r1,r3,#3				;@ A/B buttons
	cmpne r1,#3
	eorne r1,r1,#3
	tst r2,#0x400				;@ Swap A/B?
	andeq r1,r3,#3
	orr r0,r0,r1

	str r0,joy0State
	bx lr
;@----------------------------------------------------------------------------
joyCfg: .long 0x00ff01ff	;@ byte0=auto mask, byte1=(saves R), byte2=R auto mask
							;@ bit 31=single/multi, 30,29=1P/2P, 27=(multi) link active, 24=reset signal received
playerCount:.long 0			;@ Number of players in multilink.
joy0State:	.long 0
abslst2baslst:	.byte 0x00,0x02,0x01,0x03, 0x04,0x06,0x05,0x07, 0x08,0x0A,0x09,0x0B, 0x0C,0x0E,0x0D,0x0F
rlud2rldu:		.byte 0x00,0x20,0x10,0x30, 0x40,0x60,0x50,0x70, 0x80,0xA0,0x90,0xB0, 0xC0,0xE0,0xD0,0xF0

EMUinput:			;@ This label here for main.c to use
	.long 0			;@ EMUjoypad (this is what Emu sees)

;@----------------------------------------------------------------------------
joy0_R:			;@ 0x2000
	.type joy0_R STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	bl refreshEMUjoypads
	ldmfd sp!,{r4,lr}
	ldr r0,joy0State

	bx lr

;@----------------------------------------------------------------------------

	.end
#endif // #ifdef __arm__
