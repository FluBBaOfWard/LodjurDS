#ifdef __arm__

#include "ARM6502/M6502mac.h"

	.global memSelector

	.global pokeCPU
	.global peekCPU
	.global ramPoke
	.global ramPeek
	.global romPeek
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
memSelector:
	.byte 0
	.align 2

#ifdef NDS
//	.section .itcm						;@ For the NDS ARM9
#elif GBA
	.section .iwram, "ax", %progbits	;@ For the GBA
#endif
	.align 2

;@----------------------------------------------------------------------------
pokeCPU:
	.type pokeCPU STT_FUNC
;@----------------------------------------------------------------------------
	cmp r0,#0xFC00
	bpl checkIOW
	ldr r2,=lynxRAM
	strb r1,[r2,r0]
	bx lr
checkIOW:
	ldrb r3,memSelector
	and r2,r0,#0x300
	ldr pc,[pc,r2,lsr#6]
	nop
	.long checkSusieW, checkMikieW, checkRomW, checkVectorW
checkSusieW:
	tst r3,#1
	ldreq pc,=susiePoke
	b ramPoke
checkMikieW:
	tst r3,#2
	ldreq pc,=mikiePoke
	b ramPoke
checkVectorW:
	ldr r2,=0xFFF8
	cmp r0,r2
	bmi checkRomW
	beq ramPoke
	add r2,r2,#1
	cmp r0,r2			;@ 0xFFF9
	andeq r1,r1,#0xF
	strbeq r1,memSelector
	bxeq lr
	tst r3,#8
	beq rom_W
	b ramPoke
checkRomW:
	tst r3,#4
	beq rom_W
	b ramPoke

;@----------------------------------------------------------------------------
peekCPU:
	.type peekCPU STT_FUNC
;@----------------------------------------------------------------------------
	cmp r0,#0xFC00
	bpl checkIOR
	ldr r2,=lynxRAM
	ldrb r0,[r2,r0]
	bx lr
checkIOR:
	ldrb r3,memSelector
	and r2,r0,#0x300
	ldr pc,[pc,r2,lsr#6]
	nop
	.long checkSusieR, checkMikieR, checkRomR, checkVectorR
checkSusieR:
	tst r3,#1
	ldreq pc,=susiePeek
	b ramPeek
checkMikieR:
	tst r3,#2
	ldreq pc,=mikiePeek
	b ramPeek
checkVectorR:
	ldr r2,=0xFFF8
	cmp r0,r2
	bmi checkRomR
	beq ramPeek
	add r2,r2,#1
	cmp r0,r2			;@ 0xFFF9
	ldrbeq r0,memSelector
	bxeq lr
	tst r3,#8
	beq romPeek
	b ramPeek
checkRomR:
	tst r3,#4
	beq romPeek
	b ramPeek

;@----------------------------------------------------------------------------
ramPoke:
	.type ramPoke STT_FUNC
;@----------------------------------------------------------------------------
	ldr r2,=lynxRAM
	strb r1,[r2,r0]
	bx lr
;@----------------------------------------------------------------------------
ramPeek:
	.type ramPeek STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=lynxRAM
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
romPeek:
	.type romPeek STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=biosSpace
	mov r0,r0,lsl#23
	ldrb r0,[r1,r0,lsr#23]!
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
	bx lr
;@----------------------------------------------------------------------------
mem6502W7:					;@ Mem read ($E000-$FFFF)
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	mov r1,r0
	mov r0,r12
	bl pokeCPU
	ldmfd sp!,{r3,pc}
;@----------------------------------------------------------------------------
mem6502R7:					;@ Mem read ($E000-$FFFF)
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	mov r0,r12
	bl peekCPU
	ldmfd sp!,{r3,pc}

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
