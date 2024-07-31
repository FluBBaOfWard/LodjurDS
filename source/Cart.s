#ifdef __arm__

#include "ARM6502/M6502.i"

	.global cartFlags
	.global romStart
	.global romSpacePtr
	.global allocatedRomMem
	.global biosBase
	.global biosSpace
	.global biosSpaceColor
	.global biosSpaceCrystal
	.global g_BIOSBASE
	.global lynxRAM
	.global DIRTYTILES
	.global wsSRAM
	.global extEeprom
	.global extEepromMem
	.global sramSize
	.global eepromSize
	.global gRomSize
	.global maxRomSize
	.global allocatedRomMemSize
	.global romMask
	.global gGameHeader
	.global gGameID
	.global cartOrientation
	.global gConfig
	.global gMachine
	.global gMachineSet
	.global gSOC
	.global gLang
	.global gPaletteBank

	.global machineInit
	.global loadCart
	.global reBankSwitchAll
	.global reBankSwitch4_F
	.global reBankSwitch1
	.global reBankSwitch2
	.global reBankSwitch3
	.global clearDirtyTiles
	.global cartRtcUpdate

	.syntax unified
	.arm

	.section .rodata
	.align 2

ROM_Space:
//	.incbin "roms/APB - All Points Bulletin (1990).lnx"
//	.incbin "roms/Batman Returns (1992).lnx"
//	.incbin "roms/Double Dragon (1993) (Telegames).lnx"
//	.incbin "roms/Dracula - The Undead (1991).lnx"
ROM_SpaceEnd:
LYNX_BIOS_INTERNAL:
	.incbin "roms/lynxboot.img"

	.align 2
;@----------------------------------------------------------------------------
machineInit: 				;@ Called from C
	.type machineInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

//	ldr r0,=romSize
//	mov r1,#ROM_SpaceEnd-ROM_Space
//	str r1,[r0]
//	ldr r0,=romSpacePtr
//	ldr r7,=ROM_Space
//	str r7,[r0]

	bl gfxInit
//	bl ioInit
	bl soundInit
	bl cpuInit

	ldmfd sp!,{r4-r11,lr}
	bx lr

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
loadCart: 					;@ Called from C:
	.type loadCart STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr m6502ptr,=m6502_0

	bl fixRomSizeAndPtr

	bl resetCartridgeBanks

	ldr r0,=lynxRAM
	ldr r1,=mem_R0
	ldr r2,=mem_W0
	add r3,m6502ptr,#m6502MemTbl
	mov r4,#8
memLoop0:
	str r1,[r3,#8*4]
	str r2,[r3,#16*4]
	str r0,[r3],#4
	subs r4,r4,#1
	bne memLoop0

	ldr r1,=mem_R7
	ldr r2,=mem_W7
	str r1,[m6502ptr,#m6502ReadTbl+7*4]
	str r2,[m6502ptr,#m6502WriteTbl+7*4]

	ldrb r0,[r4,#0x3A]			;@ LNX Orientation
	and r0,r0,#3
	strb r0,cartOrientation

	ldr r0,=lynxRAM				;@ Clear RAM
	mov r1,#0x10000/4
	bl memclr_
	bl clearDirtyTiles

//	bl hacksInit
	bl gfxReset
	bl resetCartridgeBanks
	bl ioReset
	bl soundReset
	mov r0,r4					;@ SOC
	bl cpuReset
	ldmfd sp!,{r4-r11,lr}
	bx lr


;@----------------------------------------------------------------------------
clearDirtyTiles:
;@----------------------------------------------------------------------------
	ldr r0,=DIRTYTILES			;@ Clear RAM
	mov r1,#0x800/4
	b memclr_

;@----------------------------------------------------------------------------
fixRomSizeAndPtr:
;@----------------------------------------------------------------------------
	ldr r0,romSize
	sub r1,r0,#1
	orr r1,r1,r1,lsr#1
	orr r1,r1,r1,lsr#2
	orr r1,r1,r1,lsr#4
	orr r1,r1,r1,lsr#8
	orr r1,r1,r1,lsr#16
	add r1,r1,#1				;@ RomSize Power of 2

	movs r2,r1,lsr#16			;@ 64kB blocks.
	subne r2,r2,#1
	str r2,romMask				;@ romMask=romBlocks-1

	ldr r2,romSpacePtr
	add r2,r2,r0
	sub r2,r2,r1
	str r2,romPtr
	sub r2,r2,#0x20000
	str r2,romPtr2
	sub r2,r2,#0x10000
	str r2,romPtr3
	sub r2,r2,#0x10000
	str r2,romPtr4

	bx lr
;@----------------------------------------------------------------------------
resetCartridgeBanks:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr r12,=sphinx0
	mov r0,#0xFF
	bl BankSwitch4_F_W
	mov r0,#0xFF
	bl BankSwitch1_W
	mov r0,#0xFF
	bl BankSwitch2_W
	mov r0,#0xFF
	bl BankSwitch3_W
	ldmfd sp!,{pc}
;@----------------------------------------------------------------------------
cartUnmW:
;@----------------------------------------------------------------------------
	bx lr
;@----------------------------------------------------------------------------

romInfo:						;@
emuFlags:
	.byte 0						;@ emuflags      (label this so GUI.c can take a peek) see EmuSettings.h for bitfields
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
	.byte SOC_SPHINX
gLang:
	.byte 1						;@ language
gPaletteBank:
	.byte 0						;@ palettebank
gGameID:
	.byte 0						;@ Game ID
cartOrientation:
	.byte 0						;@ 1=Vertical, 0=Horizontal
	.space 3					;@ alignment.

gGameHeader:
	.long 0
allocatedRomMem:
	.long 0
allocatedRomMemSize:
	.long 0
romSpacePtr:
	.long 0
romPtr:
	.long 0
romPtr2:
	.long 0
romPtr3:
	.long 0
romPtr4:
	.long 0
g_BIOSBASE_BNW:
	.long 0
g_BIOSBASE_COLOR:
	.long 0
g_BIOSBASE_CRYSTAL:
	.long 0
gRomSize:
romSize:
	.long 0
maxRomSize:
	.long 0
romMask:
	.long 0
biosBase:
	.long 0
sramSize:
	.long 0
eepromSize:
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
	.space 0x800
wsSRAM:
#ifdef GBA
	.space 0x10000				;@ For the GBA
#else
	.space 0x40000
#endif
biosSpace:
	.space 0x1000
extEepromMem:
	.space 0x800
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
