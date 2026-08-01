//
//  Sound.s
//  LodjurDS
//
//  Created by Fredrik Ahlström on 2024-07-29.
//  Copyright © 2024-2026 Fredrik Ahlström. All rights reserved.
//
#ifdef __arm__

#include "ARMMikey/ARMMikey.i"

	.extern pauseEmulation

	.global soundInit
	.global soundReset
	.global soundRender
	.global soundSetMuteGUI
	.global setSoundChipEnable

;@----------------------------------------------------------------------------

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
soundInit:
	.type soundInit STT_FUNC
;@----------------------------------------------------------------------------
//	stmfd sp!,{lr}

//	ldmfd sp!,{lr}
//	bx lr

;@----------------------------------------------------------------------------
soundReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{mikptr,lr}
	ldr mikptr,=mikey_0
	bl miAudioReset			;@ sound
	ldmfd sp!,{mikptr,lr}
	bx lr

;@----------------------------------------------------------------------------
soundSetMuteGUI:
	.type   soundSetMuteGUI STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=pauseEmulation		;@ Output silence when emulation paused.
	ldrb r0,[r1]
	strb r0,muteSoundGUI
	bx lr
;@----------------------------------------------------------------------------
setSoundChipEnable:			;@ In r0=mute/unmute
	.type setSoundChipEnable STT_FUNC
;@----------------------------------------------------------------------------
	cmp r0,#0
	movne r0,#0
	moveq r0,#1
	strb r0,muteSoundChip
	bx lr
;@----------------------------------------------------------------------------
soundRender:					;@ r0=length, r1=pointer
;@----------------------------------------------------------------------------
	ldr r2,muteSound
	cmp r2,#0
	bne silenceMix

	stmfd sp!,{r0,mikptr,lr}
	ldr mikptr,=mikey_0
	bl miAudioMixer
	ldmfd sp!,{r0,mikptr,lr}
	bx lr

silenceMix:
	mov r3,r0
	ldr r2,=0x00000000
silenceLoop:
	subs r3,r3,#1
	strpl r2,[r1],#4
	bhi silenceLoop

	bx lr

;@----------------------------------------------------------------------------
pcmWritePtr:	.long 0
pcmReadPtr:		.long 0

muteSound:
muteSoundGUI:
	.byte 0
muteSoundChip:
	.byte 0
	.space 2

	.section .bss
	.align 2
//WAVBUFFER:
//	.space 0x1000
;@----------------------------------------------------------------------------
	.end
#endif // __arm__
