//
//  LynxCart.s
//  Atari Lynx Cartridge emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-12-08.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//

#ifdef __arm__

#ifdef GBA
	#include "../Shared/gba_asm.h"
#elif NDS
	#include "../Shared/nds_asm.h"
#endif
#include "LynxCart.i"

	.global cartInit
	.global cartReset
	.global cartSaveState
	.global cartLoadState
	.global cartGetStateSize
	.global cartRead
	.global cartWrite
	.global cartAddressStrobe
	.global cartAddressData
	.global cartRead0
	.global cartRead1
	.global cartWrite0
	.global cartWrite1

	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
cartInit:					;@ Only need to be called once
;@----------------------------------------------------------------------------

	bx lr
;@----------------------------------------------------------------------------
cartReset:					;@ r0=crtptr, r1=rom, r2=bank0 size, r3=bank1 size.
	.type	cartReset STT_FUNC
;@----------------------------------------------------------------------------
	str r1,[r0,#cartBank0]
	add r1,r1,r2
	str r1,[r0,#cartBank1]

	ldr r1,=0x7FF
	mov r12,#11
	cmp r2,#0x80000
	movcc r1,r1,lsr#1
	movcc r12,#10
	cmp r2,#0x40000
	movcc r1,r1,lsr#1
	movcc r12,#9
	cmp r2,#0x20000
	movcc r1,r1,lsr#1
	movcc r12,#8
	strh r1,[r0,#cartCountMask0]
	strb r12,[r0,#cartShiftCount0]

	ldr r1,=0x7FF
	mov r12,#11
	cmp r3,#0x80000
	movcc r1,r1,lsr#1
	movcc r12,#10
	cmp r3,#0x40000
	movcc r1,r1,lsr#1
	movcc r12,#9
	cmp r3,#0x20000
	movcc r1,r1,lsr#1
	movcc r12,#8
	strh r1,[r0,#cartCountMask1]
	strb r12,[r0,#cartShiftCount1]

	mov r1,#0
	strb r1,[r0,#cartWriteEnable0]
	strb r1,[r0,#cartWriteEnable1]
	strh r1,[r0,#cartCounter]
	strb r1,[r0,#cartShifter]
	strb r1,[r0,#cartDataBit]
	strb r1,[r0,#cartStrobe]
	strb r1,[r0,#cartCurrentBank]

	bx lr

;@----------------------------------------------------------------------------
cartSaveState:				;@ In r0=destination, r1=crtptr. Out r0=state size.
	.type	cartSaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	add r1,r1,#cartState
	mov r2,#cartStateEnd-cartState
	bl memcpy

	ldmfd sp!,{lr}
	mov r0,#cartStateEnd-cartState
	bx lr
;@----------------------------------------------------------------------------
cartLoadState:				;@ In r0=crtptr, r1=source. Out r0=state size.
	.type	cartLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	add r0,r0,#cartState
	mov r2,#cartStateEnd-cartState
	bl memcpy

	ldmfd sp!,{lr}
;@----------------------------------------------------------------------------
cartGetStateSize:			;@ Out r0=state size.
	.type	cartGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r0,#cartStateEnd-cartState
	bx lr

;@----------------------------------------------------------------------------
cartAddressStrobe:				;@ In r0=crtptr, r1=strobe
	.type	cartAddressStrobe STT_FUNC
;@----------------------------------------------------------------------------
	ands r1,r1,#1
	beq exitStrobe
	mov r2,#0
	strh r2,[r0,#cartCounter]

	ldrb r2,[r0,#cartStrobe]
	cmp r2,#0
	bne exitStrobe
	ldrb r3,[r0,#cartDataBit]
	ldrb r2,[r0,#cartShifter]
	cmp r3,#1
	adc r2,r2,r2
	strb r2,[r0,#cartShifter]
exitStrobe:
	strb r1,[r0,#cartStrobe]
	bx lr
;@----------------------------------------------------------------------------
cartAddressData:				;@ In r0=crtptr, r1=data
	.type	cartAddressData STT_FUNC
;@----------------------------------------------------------------------------
	strb r1,[r0,#cartDataBit]
	bx lr
;@----------------------------------------------------------------------------
cartRead0:					;@ In r0=crtptr
;@----------------------------------------------------------------------------
	ldrh r3,[r0,#cartCounter]
	ldrb r12,[r0,#cartShifter]
	ldrb r2,[r0,#cartShiftCount0]
	orr r12,r3,r12,lsl r2
	ldrb r2,[r0,#cartStrobe]
	cmp r2,#0
	ldrheq r2,[r0,#cartCountMask0]
	addeq r3,r3,#1
	andeq r3,r3,r2
	strheq r3,[r0,#cartCounter]
	ldr r2,[r0,#cartBank0]
	ldrb r0,[r2,r12]

	bx lr
;@----------------------------------------------------------------------------
cartRead1:					;@ In r0=crtptr
;@----------------------------------------------------------------------------
	ldrh r3,[r0,#cartCounter]
	ldrb r12,[r0,#cartShifter]
	ldrb r2,[r0,#cartShiftCount1]
	orr r12,r3,r12,lsl r2
	ldrb r2,[r0,#cartStrobe]
	cmp r2,#0
	ldrheq r2,[r0,#cartCountMask1]
	addeq r3,r3,#1
	andeq r3,r3,r2
	strheq r3,[r0,#cartCounter]
	ldr r2,[r0,#cartBank0]
	ldrb r0,[r2,r12]

	bx lr
;@----------------------------------------------------------------------------
cartWrite0:					;@ In r0=crtptr, r1=data
;@----------------------------------------------------------------------------
	ldrh r3,[r0,#cartCounter]
	ldrb r12,[r0,#cartShifter]
	ldrb r2,[r0,#cartShiftCount0]
	orr r12,r3,r12,lsl r2
	ldrb r2,[r0,#cartStrobe]
	cmp r2,#0
	ldrheq r2,[r0,#cartCountMask0]
	addeq r3,r3,#1
	andeq r3,r3,r2
	strheq r3,[r0,#cartCounter]
	ldr r2,[r0,#cartBank0]
	ldrb r3,[r0,#cartWriteEnable0]
	cmp r3,#0
	strbne r1,[r2,r12]

	bx lr
;@----------------------------------------------------------------------------
cartWrite1:					;@ In r0=crtptr, r1=data
;@----------------------------------------------------------------------------
	ldrh r3,[r0,#cartCounter]
	ldrb r12,[r0,#cartShifter]
	ldrb r2,[r0,#cartShiftCount1]
	orr r12,r3,r12,lsl r2
	ldrb r2,[r0,#cartStrobe]
	cmp r2,#0
	ldrheq r2,[r0,#cartCountMask1]
	addeq r3,r3,#1
	andeq r3,r3,r2
	strheq r3,[r0,#cartCounter]
	ldr r2,[r0,#cartBank1]
	ldrb r3,[r0,#cartWriteEnable1]
	cmp r3,#0
	strbne r1,[r2,r12]

	bx lr

;@----------------------------------------------------------------------------

#endif // #ifdef __arm__
