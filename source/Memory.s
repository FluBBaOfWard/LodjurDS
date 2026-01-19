#ifdef __arm__

#include "ARMMikey/ARM6502/M6502mac.h"
#include "ARMMikey/ARMMikey.i"
#include "ARMSuzy/ARMSuzy.i"

	.global empty_R
	.global empty_W
	.global empty_IO_R
	.global empty_IO_W
	.global rom_W
	.global ram6502W
	.global ram6502R
	.global mem6502W7
	.global mem6502R7


	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
empty_R:					;@ Read bad address (error)
;@----------------------------------------------------------------------------
;@----------------------------------------------------------------------------
empty_IO_R:					;@ Read bad IO address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0x10
	bx lr
;@----------------------------------------------------------------------------
empty_W:					;@ Write bad address (error)
;@----------------------------------------------------------------------------
;@----------------------------------------------------------------------------
empty_IO_W:					;@ Write bad IO address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0x18
	bx lr
;@----------------------------------------------------------------------------
rom_W:						;@ Write ROM address (error)
	.type rom_W STT_FUNC
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0xB0
	bx lr
;@----------------------------------------------------------------------------

#ifdef NDS
	.section .itcm, "ax", %progbits		;@ For the NDS ARM9
#elif GBA
	.section .iwram, "ax", %progbits	;@ For the GBA
#endif
	.align 2

;@----------------------------------------------------------------------------
pokeCPU:					;@ Mem/IO write ($FC00-$FFFF). In r0=adr, r1=val
;@----------------------------------------------------------------------------
	ldrb r3,[mikptr,#memSelector]
	and r2,r0,#0x300
	ldr pc,[pc,r2,lsr#6]
	nop
	.long checkSusieW, checkMikieW, checkRomW, checkVectorW
checkSusieW:
	tst r3,#1
	ldreq suzptr,=suzy_0
	beq suzyWrite
	b ramPoke
checkMikieW:
	tst r3,#2
	beq mikeyWrite
	b ramPoke
checkVectorW:
	ldr r2,=0xFFF8
	cmp r0,r2
	bmi checkRomW
	beq ramPoke
	add r2,r2,#1
	cmp r0,r2					;@ 0xFFF9
	andeq r1,r1,#0xF
	strbeq r1,[mikptr,#memSelector]
	bxeq lr
	tst r3,#8
	beq rom_W
	b ramPoke
checkRomW:
	tst r3,#4
	beq rom_W
;@----------------------------------------------------------------------------
ramPoke:
;@----------------------------------------------------------------------------
	strb r1,[m6502zpage,r0]
	bx lr

;@----------------------------------------------------------------------------
peekCPU:					;@ Mem/IO read ($FC00-$FFFF). In r0=adr. Out r0=val
;@----------------------------------------------------------------------------
	ldrb r3,[mikptr,#memSelector]
	and r2,r0,#0x300
	ldr pc,[pc,r2,lsr#6]
	nop
	.long checkSusieR, checkMikieR, checkRomR, checkVectorR
checkSusieR:
	tst r3,#1
	ldreq suzptr,=suzy_0
	beq suzyRead
	b ramPeek
checkMikieR:
	tst r3,#2
	beq mikeyRead
	b ramPeek
checkVectorR:
	ldr r2,=0xFFF8
	cmp r0,r2
	bmi checkRomR
	beq ramPeek
	add r2,r2,#1
	cmp r0,r2					;@ 0xFFF9
	moveq r0,r3					;@ MemSelector
	bxeq lr
	tst r3,#8
	beq romPeek
	b ramPeek
checkRomR:
	tst r3,#4
	beq romPeek
;@----------------------------------------------------------------------------
ramPeek:
;@----------------------------------------------------------------------------
	mov r1,m6502zpage
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
romPeek:					;@ Rom read ($FE00-$FFFF)
;@----------------------------------------------------------------------------
	ldr r1,=biosSpace-0xFE00
	ldrb r0,[r1,r0]!
	bx lr

;@----------------------------------------------------------------------------
ram6502W:					;@ Ram write ($0000-$DFFF)
;@----------------------------------------------------------------------------
	strb r0,[m6502zpage,addy]
	bx lr
;@----------------------------------------------------------------------------
ram6502R:					;@ Ram read ($0000-$DFFF)
;@----------------------------------------------------------------------------
	ldrb r0,[m6502zpage,addy]
	add r1,m6502zpage,addy
	bx lr
;@----------------------------------------------------------------------------
mem6502W7:					;@ Mem write ($E000-$FFFF)
;@----------------------------------------------------------------------------
	cmp addy,#0xFC00
	strbmi r0,[m6502zpage,addy]
	bxmi lr
	stmfd sp!,{r3,addy,lr}
	mov r1,r0
	mov r0,addy
	bl pokeCPU
	ldmfd sp!,{r3,addy,pc}
;@----------------------------------------------------------------------------
mem6502R7:					;@ Mem read ($E000-$FFFF)
;@----------------------------------------------------------------------------
	cmp addy,#0xFC00
	movmi r1,m6502zpage
	ldrbmi r0,[r1,addy]!
	bxmi lr
	stmfd sp!,{r3,addy,lr}
	mov r0,addy
	bl peekCPU
	ldmfd sp!,{r3,addy,pc}

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
