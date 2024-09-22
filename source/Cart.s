#ifdef __arm__

//#define EMBEDDED_ROM

#include "ARMSuzy/ARMSuzy.i"
#include "ARM6502/M6502.i"

	.global machineInit
	.global loadCart
	.global romNum
	.global cartFlags
	.global romStart
	.global bankSwitchCart
	.global reBankSwitchCart
	.global BankSwitch89AB_W
	.global clearDirtyTiles

	.global romSpacePtr
	.global MEMMAPTBL_

	.global lynxRAM
	.global svVRAM
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

	.syntax unified
	.arm

	.section .rodata
	.align 2

#ifdef EMBEDDED_ROM
ROM_Space:
//	.incbin "roms/APB - All Points Bulletin (1990).lnx"
//	.incbin "roms/Batman Returns (1992).lnx"
//	.incbin "roms/Double Dragon (1993) (Telegames).lnx"
//	.incbin "roms/Dracula - The Undead (1991).lnx"
ROM_SpaceEnd:
#endif
LYNX_BIOS_INTERNAL:
	.incbin "roms/lynxboot.img"

	.align 2
;@----------------------------------------------------------------------------
machineInit: 				;@ Called from C
	.type   machineInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

#ifdef EMBEDDED_ROM
	ldr r0,=romSize
	mov r1,#ROM_SpaceEnd-ROM_Space
	str r1,[r0]
	ldr r0,=romSpacePtr
	ldr r7,=ROM_Space
	str r7,[r0]
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
	ldr m6502ptr,=m6502_0

	ldr r0,romSize
	movs r1,r0,lsr#14			;@ 16kB blocks.
	subne r1,r1,#1
	str r1,romMask				;@ romMask=romBlocks-1

	bl resetCartridgeBanks

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
	bl resetCartridgeBanks
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
	str r1,[r0,#m6502MemTbl+0*4]	;@ 0 RAM
	ldr r1,=svVRAM
	str r1,[r0,#m6502MemTbl+2*4]	;@ 0 VRAM

	ldr r1,=ram6502R
	str r1,[r0,#m6502ReadTbl+0*4]
	ldr r1,=wsvReadIO
	str r1,[r0,#m6502ReadTbl+1*4]
	ldr r1,=vram6502R
	str r1,[r0,#m6502ReadTbl+2*4]
	ldr r1,=empty_R
	str r1,[r0,#m6502ReadTbl+3*4]
	ldr r1,=mem6502R4
	str r1,[r0,#m6502ReadTbl+4*4]
	ldr r1,=mem6502R5
	str r1,[r0,#m6502ReadTbl+5*4]
	ldr r1,=mem6502R6
	str r1,[r0,#m6502ReadTbl+6*4]
	ldr r1,=mem6502R7
	str r1,[r0,#m6502ReadTbl+7*4]

	ldr r1,=ram6502W
	str r1,[r0,#m6502WriteTbl+0*4]
	ldr r1,=wsvWriteIO
	str r1,[r0,#m6502WriteTbl+1*4]
	ldr r1,=vram6502W
	str r1,[r0,#m6502WriteTbl+2*4]
	ldr r1,=empty_W
	str r1,[r0,#m6502WriteTbl+3*4]
	ldr r1,=rom_W
	str r1,[r0,#m6502WriteTbl+4*4]
	str r1,[r0,#m6502WriteTbl+5*4]
	str r1,[r0,#m6502WriteTbl+6*4]
	str r1,[r0,#m6502WriteTbl+7*4]

	bx lr
;@----------------------------------------------------------------------------
resetCartridgeBanks:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	ldr r4,romSpacePtr
	ldr r0,=bankPointers
	ldrb r2,romMask
	mov r1,#0x1F
bankLoop:
	and r3,r2,r1
	add r3,r4,r3,lsl#14
	str r3,[r0,r1,lsl#2]
	subs r1,r1,#1
	bpl bankLoop

	mov r1,#0
	bl BankSwitch89AB_W
	bl BankSwitchCDEF_W
	ldmfd sp!,{r4,pc}
;@----------------------------------------------------------------------------
reBankSwitchCart:			;@ r0 = LinkPort val, r1 = BankChip val
;@----------------------------------------------------------------------------
	ldr m6502ptr,=m6502_0
;@----------------------------------------------------------------------------
bankSwitchCart:				;@ r0 = LinkPort val, r1 = BankChip val
;@----------------------------------------------------------------------------
	mov r1,r1,lsr#5
	ldrb r2,romMask
	cmp r2,#8
	bmi BankSwitch89AB_W
	tst r1,#4
	orrne r0,r0,#0xF
	and r1,r1,#1
	orr r1,r1,r0,lsl#1
;@----------------------------------------------------------------------------
BankSwitch89AB_W:			;@ 0x8000-0xBFFF
;@----------------------------------------------------------------------------
	adr r2,bankPointers
	and r1,r1,#0x1F
	ldr r0,[r2,r1,lsl#2]
	sub r0,r0,#0x8000

	str r0,[m6502ptr,#m6502MemTbl+4*4]
	str r0,[m6502ptr,#m6502MemTbl+5*4]
	bx lr
;@----------------------------------------------------------------------------
BankSwitchCDEF_W:			;@ 0xC000-0xFFFF
;@----------------------------------------------------------------------------
	ldr r0,bankPointers+31*4
	sub r0,r0,#0xC000
	str r0,[m6502ptr,#m6502MemTbl+6*4]
	str r0,[m6502ptr,#m6502MemTbl+7*4]

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
bankPointers:
	.space 32*4

#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 8
lynxRAM:
	.space 0x10000
svVRAM:
	.space 0x2000
DIRTYTILES:
	.space 0x200
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
