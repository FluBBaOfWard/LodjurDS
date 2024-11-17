#ifdef __arm__

//#define EMBEDDED_ROM

#include "ARMSuzy/ARMSuzy.i"
#include "ARMMikey/ARMMikey.i"

	.global cartFlags
	.global romSpacePtr
	.global biosSpace
	.global romStart
	.global lynxRAM
	.global DIRTYTILES
	.global gRomSize
	.global maxRomSize
	.global romMask
	.global gGameID
	.global gConfig
	.global gMachine
	.global gMachineSet
	.global gSOC
	.global gLang
	.global gPaletteBank

	.global machineInit
	.global loadCart
	.global clearDirtyTiles

	.syntax unified
	.arm

	.section .rodata
	.align 2

#ifdef EMBEDDED_ROM
ROM_Space:
//	.incbin "roms/A.P.B. - All Points Bulletin (1990).lnx"
//	.incbin "roms/Batman Returns (1992).lnx"
//	.incbin "roms/desert strike - return to the gulf (1993) (telegames).lnx"
//	.incbin "roms/Double Dragon (1993) (Telegames).lnx"
//	.incbin "roms/Dracula - The Undead (1991).lnx"
//	.incbin "roms/ninja gaiden (1990).lnx"
//	.incbin "roms/ninja gaiden iii - the ancient ship of doom (1993).lnx"
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

	ldr r0,romSize
	movs r1,r0,lsr#14			;@ 16kB blocks.
	subne r1,r1,#1
	str r1,romMask				;@ romMask=romBlocks-1

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

romNum:
	.long 0						;@ romnumber
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
	.byte HW_LYNX
gSOC:
	.byte SOC_HOWARD
gLang:
	.byte 1						;@ language
gPaletteBank:
	.byte 0						;@ palettebank
gGameID:
	.byte 0						;@ Game ID
	.byte 0
	.byte 0
	.space 2					;@ alignment.

romSpacePtr:
	.long 0
gRomSize:
romSize:
	.long 0
maxRomSize:
	.long 0
romMask:
	.long 0

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
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
