#ifdef __arm__

//#define EMBEDDED_ROM

#include "ARMSuzy/ARMSuzy.i"
#include "ARMMikey/ARMMikey.i"
#include "LynxCart/LynxCart.i"

	.global cartFlags
	.global romSpacePtr
	.global biosSpace
	.global lynxRAM
	.global DIRTYTILES
	.global gRomSize
	.global maxRomSize
	.global gConfig
	.global gMachineSet
	.global gMachine
	.global gSOC
	.global cart_0

	.global machineInit
	.global loadCart
	.global clearDirtyTiles
	.global lnxCartRead
	.global lnxCartWrite

	.syntax unified
	.arm

	.section .rodata
	.align 2

#ifdef EMBEDDED_ROM
ROM_Space:
//	.incbin "roms/A.P.B. - All Points Bulletin (1990).lnx"
//	.incbin "roms/Baseball Heroes (USA).lyx"
//	.incbin "roms/Batman Returns (1992).lnx"
//	.incbin "roms/Blue Lightning (1989).lnx"
//	.incbin "roms/Desert Strike - Return to the Gulf (1993) (Telegames).lnx"
//	.incbin "roms/Double Dragon (1993) (Telegames).lnx"
//	.incbin "roms/Dracula - The Undead (1991).lnx"
//	.incbin "roms/Ninja Gaiden (1990).lnx"
//	.incbin "roms/Ninja Gaiden III - the Ancient Ship of Doom (1993).lnx"
//	.incbin "roms/Shadow of the Beast (1992)[crc-2].lnx"
ROM_SpaceEnd:
LYNX_BIOS_INTERNAL:
	.incbin "roms/lynxboot.img"
#endif

	.align 2
;@----------------------------------------------------------------------------
machineInit: 				;@ Called from C
	.type   machineInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

#ifdef EMBEDDED_ROM
	ldr r0,=romSize
	ldr r1,=(ROM_SpaceEnd-ROM_Space)
	str r1,[r0]
	ldr r0,=romSpacePtr
	ldr r1,=ROM_Space
	str r1,[r0]
	ldr r0,=biosSpace
	adr r1,LYNX_BIOS_INTERNAL
	mov r2,#0x200
	bl memcpy
#endif
	bl memoryMapInit
	bl gfxInit
//	bl ioInit
	bl soundInit
	bl cpuInit

	ldmfd sp!,{r4-r11,lr}
	bx lr

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
loadCart: 					;@ Called from C
	.type   loadCart STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr mikptr,=mikey_0

	ldr r2,romSize
	and r0,r2,#0x40				;@ Header present?
	ldr r1,romSpacePtr
	add r1,r1,r0
	ldr r0,=cart_0
	mov r3,#0x10000
	bl cartReset

	ldrb r5,gMachine
	cmp r5,#HW_LYNX_II
	moveq r4,#SOC_HOWARD2
	movne r4,#SOC_HOWARD
	strb r4,gSOC


	ldr r0,=lynxRAM				;@ Clear RAM
	mov r1,#0x10000/4
	bl memclr_
	bl clearDirtyTiles

//	bl hacksInit
	bl gfxReset
	bl ioReset
	bl soundReset
	bl cpuReset
	ldmfd sp!,{r4-r11,lr}
	bx lr

;@----------------------------------------------------------------------------
clearDirtyTiles:
;@----------------------------------------------------------------------------
	ldr r0,=DIRTYTILES			;@ Clear RAM
	mov r1,#0x200/4
	b memclr_

;@----------------------------------------------------------------------------
memoryMapInit:
;@----------------------------------------------------------------------------
	ldr r0,=m6502_0

	ldr r1,=lynxRAM
	str r1,[r0,#m6502MemTbl+0*4]
	str r1,[r0,#m6502MemTbl+1*4]
	str r1,[r0,#m6502MemTbl+2*4]
	str r1,[r0,#m6502MemTbl+3*4]
	str r1,[r0,#m6502MemTbl+4*4]
	str r1,[r0,#m6502MemTbl+5*4]
	str r1,[r0,#m6502MemTbl+6*4]
	str r1,[r0,#m6502MemTbl+7*4]

	ldr r1,=ram6502R
	str r1,[r0,#m6502ReadTbl+0*4]
	str r1,[r0,#m6502ReadTbl+1*4]
	str r1,[r0,#m6502ReadTbl+2*4]
	str r1,[r0,#m6502ReadTbl+3*4]
	str r1,[r0,#m6502ReadTbl+4*4]
	str r1,[r0,#m6502ReadTbl+5*4]
	str r1,[r0,#m6502ReadTbl+6*4]
	ldr r1,=mem6502R7
	str r1,[r0,#m6502ReadTbl+7*4]

	ldr r1,=ram6502W
	str r1,[r0,#m6502WriteTbl+0*4]
	str r1,[r0,#m6502WriteTbl+1*4]
	str r1,[r0,#m6502WriteTbl+2*4]
	str r1,[r0,#m6502WriteTbl+3*4]
	str r1,[r0,#m6502WriteTbl+4*4]
	str r1,[r0,#m6502WriteTbl+5*4]
	str r1,[r0,#m6502WriteTbl+6*4]
	ldr r1,=mem6502W7
	str r1,[r0,#m6502WriteTbl+7*4]

	bx lr
;@----------------------------------------------------------------------------
lnxCartRead:				;@ r0=adr
;@----------------------------------------------------------------------------
	and r2,r0,#0xFF
	cmp r2,#0xB2
	ldr r0,=cart_0
	beq cartRead0
	cmp r2,#0xB3
	beq cartRead1
	bx lr
;@----------------------------------------------------------------------------
lnxCartWrite:				;@ r0=adr, r1=data
;@----------------------------------------------------------------------------
	and r2,r0,#0xFF
	cmp r2,#0xB2
	ldr r0,=cart_0
	beq cartWrite0
	cmp r2,#0xB3
	beq cartWrite1
	bx lr
;@----------------------------------------------------------------------------

romInfo:						;@
emuFlags:
	.byte 0						;@ emuflags      (label this so Gui.c can take a peek) see EmuSettings.h for bitfields
//scaling:
	.byte 0						;@ (display type)
	.byte 0,0					;@ (sprite follow val)
cartFlags:
	.byte 0 					;@ cartflags
gConfig:
	.byte 0						;@ Config, bit 7=BIOS on/off
gMachineSet:
	.byte HW_AUTO
gMachine:
	.byte HW_LYNX_II
gSOC:
	.byte SOC_HOWARD
	.space 3					;@ alignment.

romSpacePtr:
	.long 0
gRomSize:
romSize:
	.long 0
maxRomSize:
	.long 0
;@----------------------------------------------------------------------------
#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 8
lynxRAM:
	.space 0x10000
DIRTYTILES:
	.space 0x200
biosSpace:
	.space 0x200
cart_0:
	.space cartSize
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
