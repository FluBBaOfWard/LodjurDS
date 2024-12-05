//
//  ARMSuzy.h
//  Atari Lynx Suzy emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-09-22.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//

#ifdef __arm__

#ifdef GBA
	#include "../Shared/gba_asm.h"
#elif NDS
	#include "../Shared/nds_asm.h"
#endif
#include "ARMSuzy.i"
#include "../ARMMikey/ARM6502/M6502.i"

#define LINE_END		0x80

	.global suzyInit
	.global suzyReset
	.global suzySaveState
	.global suzyLoadState
	.global suzyGetStateSize
	.global suzyRead
	.global suzyWrite
	.global suzySetButtonData
	.global suzPaintSprites

	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
suzyInit:					;@ Only need to be called once
;@----------------------------------------------------------------------------
	mov r1,#0xffffff00			;@ Build chr decode tbl
	ldr r3,=CHR_DECODE			;@ 0x200
chrLutLoop:
	and r0,r1,#0x03
	and r2,r1,#0x0C
	orr r0,r0,r2,lsl#2
	and r2,r1,#0x30
	orr r0,r0,r2,lsl#4
	and r2,r1,#0xC0
	orr r0,r0,r2,lsl#6
	strh r0,[r3],#2
	adds r1,r1,#1
	bne chrLutLoop

;@----------------------------------------------------------------------------
makeTileBgr:
;@----------------------------------------------------------------------------
	mov r1,#BG_GFX
	mov r0,#0
	mov r2,#32*22
bgrLoop:
	strh r0,[r1],#2
	add r0,r0,#1
	subs r2,r2,#1
	bne bgrLoop

	bx lr
;@----------------------------------------------------------------------------
suzyReset:					;@ r0=ram, r12=suzptr
;@----------------------------------------------------------------------------
	stmfd sp!,{r0,lr}

	mov r0,suzptr
	ldr r1,=suzySize/4
	bl memclr_					;@ Clear Suzy state

	mov r0,#0x7F
	strh r0,[suzptr,#suzHSizOff]
	strh r0,[suzptr,#suzVSizOff]

	;@ Must be initialised to this due to
	;@ Stun Runner math initialisation bug
	mov r0,#-1
	strh r0,[suzptr,#suzMathAB]
	strh r0,[suzptr,#suzMathCD]
	str r0,[suzptr,#suzMathEFGH]
	str r0,[suzptr,#suzMathJKLM]
	strh r0,[suzptr,#suzMathNP]

	mov r0,#1
	str r0,[suzptr,#mathAB_sign]
	str r0,[suzptr,#mathCD_sign]

	ldmfd sp!,{r0,lr}

	str r0,[suzptr,#suzyRAM]

	bx lr

;@----------------------------------------------------------------------------
_debugIOUnmappedR:
;@----------------------------------------------------------------------------
	ldr r3,=debugIOUnmappedR
	bx r3
;@----------------------------------------------------------------------------
_debugIOUnimplR:
;@----------------------------------------------------------------------------
	ldr r3,=debugIOUnimplR
	bx r3
;@----------------------------------------------------------------------------
_debugIOUnmappedW:
;@----------------------------------------------------------------------------
	ldr r3,=debugIOUnmappedW
	bx r3
;@----------------------------------------------------------------------------
memCopy:
;@----------------------------------------------------------------------------
	ldr r3,=memcpy
;@----------------------------------------------------------------------------
thumbCallR3:
;@----------------------------------------------------------------------------
	bx r3

;@----------------------------------------------------------------------------
suzySaveState:				;@ In r0=destination, r1=suzptr. Out r0=state size.
	.type	suzySaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5,lr}
	mov r4,r0					;@ Store destination
	mov r5,r1					;@ Store suzptr (r1)

	add r1,r5,#suzyState
	mov r2,#suzyStateEnd-suzyState
	bl memCopy

	ldmfd sp!,{r4,r5,lr}
	mov r0,#suzyStateEnd-suzyState
	bx lr
;@----------------------------------------------------------------------------
suzyLoadState:				;@ In r0=suzptr, r1=source. Out r0=state size.
	.type	suzyLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5,r10,lr}
	mov r5,r0					;@ Store suzptr (r0)
	mov r4,r1					;@ Store source

	add r0,r5,#suzyState
	mov r2,#suzyStateEnd-suzyState
	bl memCopy

	bl clearDirtyTiles

	ldmfd sp!,{r4,r5,r10,lr}
;@----------------------------------------------------------------------------
suzyGetStateSize:			;@ Out r0=state size.
	.type	suzyGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r0,#suzyStateEnd-suzyState
	bx lr

	.pool

;@----------------------------------------------------------------------------
suzyRead:					;@ I/O read (0xFC00-0xFCC5)
;@----------------------------------------------------------------------------
	sub r2,r0,#0xFC00
	cmp r2,#0xC5
	ldrmi pc,[pc,r2,lsl#2]
	b suUnmappedR
io_read_tbl:
	.long suRegR				;@ 0xFC00 TMPADRL
	.long suRegR				;@ 0xFC01 TMPADRH
	.long suRegR				;@ 0xFC02 TILTACUML
	.long suRegR				;@ 0xFC03 TILTACUMH
	.long suRegR				;@ 0xFC04 HOFFL
	.long suRegR				;@ 0xFC05 HOFFH
	.long suRegR				;@ 0xFC06 VOFFL
	.long suRegR				;@ 0xFC07 VOFFH
	.long suRegR				;@ 0xFC08 VIDBASL
	.long suRegR				;@ 0xFC09 VIDBASH
	.long suRegR				;@ 0xFC0A COLLBASL
	.long suRegR				;@ 0xFC0B COLLBASH
	.long suRegR				;@ 0xFC0C VIDADRL
	.long suRegR				;@ 0xFC0D VIDADRH
	.long suRegR				;@ 0xFC0E COLLADRL
	.long suRegR				;@ 0xFC0F COLLADRH
	.long suRegR				;@ 0xFC10 SCBNEXTL
	.long suRegR				;@ 0xFC11 SCBNEXTH
	.long suRegR				;@ 0xFC12 SPRDLINEL
	.long suRegR				;@ 0xFC13 SPRDLINEH
	.long suRegR				;@ 0xFC14 HPOSSTRTL
	.long suRegR				;@ 0xFC15 HPOSSTRTH
	.long suRegR				;@ 0xFC16 VPOSSTRTL
	.long suRegR				;@ 0xFC17 VPOSSTRTH
	.long suRegR				;@ 0xFC18 SPRHSIZL
	.long suRegR				;@ 0xFC19 SPRHSIZH
	.long suRegR				;@ 0xFC1A SPRVSIZL
	.long suRegR				;@ 0xFC1B SPRVSIZH
	.long suRegR				;@ 0xFC1C STRETCHL
	.long suRegR				;@ 0xFC1D STRETCHH
	.long suRegR				;@ 0xFC1E TILTL
	.long suRegR				;@ 0xFC1F TILTH
	.long suRegR				;@ 0xFC20 SPRDOFFL
	.long suRegR				;@ 0xFC21 SPRDOFFH
	.long suRegR				;@ 0xFC22 SPRVPOSL
	.long suRegR				;@ 0xFC23 SPRVPOSH
	.long suRegR				;@ 0xFC24 COLLOFFL
	.long suRegR				;@ 0xFC25 COLLOFFH
	.long suRegR				;@ 0xFC26 VSIZACUML
	.long suRegR				;@ 0xFC27 VSIZACUMH
	.long suRegR				;@ 0xFC28 HSIZOFFL
	.long suRegR				;@ 0xFC29 HSIZOFFH
	.long suRegR				;@ 0xFC2A VSIZOFFL
	.long suRegR				;@ 0xFC2B VSIZOFFH
	.long suRegR				;@ 0xFC2C SCBADRL
	.long suRegR				;@ 0xFC2D SCBADRH
	.long suRegR				;@ 0xFC2E PROCADRL
	.long suRegR				;@ 0xFC2F PROCADRH
	.long suUnmappedR			;@ 0xFC30
	.long suUnmappedR			;@ 0xFC31
	.long suUnmappedR			;@ 0xFC32
	.long suUnmappedR			;@ 0xFC33
	.long suUnmappedR			;@ 0xFC34
	.long suUnmappedR			;@ 0xFC35
	.long suUnmappedR			;@ 0xFC36
	.long suUnmappedR			;@ 0xFC37
	.long suUnmappedR			;@ 0xFC38
	.long suUnmappedR			;@ 0xFC39
	.long suUnmappedR			;@ 0xFC3A
	.long suUnmappedR			;@ 0xFC3B
	.long suUnmappedR			;@ 0xFC3C
	.long suUnmappedR			;@ 0xFC3D
	.long suUnmappedR			;@ 0xFC3E
	.long suUnmappedR			;@ 0xFC3F
	.long suUnmappedR			;@ 0xFC40
	.long suUnmappedR			;@ 0xFC41
	.long suUnmappedR			;@ 0xFC42
	.long suUnmappedR			;@ 0xFC43
	.long suUnmappedR			;@ 0xFC44
	.long suUnmappedR			;@ 0xFC45
	.long suUnmappedR			;@ 0xFC46
	.long suUnmappedR			;@ 0xFC47
	.long suUnmappedR			;@ 0xFC48
	.long suUnmappedR			;@ 0xFC49
	.long suUnmappedR			;@ 0xFC4A
	.long suUnmappedR			;@ 0xFC4B
	.long suUnmappedR			;@ 0xFC4C
	.long suUnmappedR			;@ 0xFC4D
	.long suUnmappedR			;@ 0xFC4E
	.long suUnmappedR			;@ 0xFC4F
	.long suUnmappedR			;@ 0xFC50
	.long suUnmappedR			;@ 0xFC51
	.long suRegR				;@ 0xFC52 MATHD
	.long suRegR				;@ 0xFC53 MATHC
	.long suRegR				;@ 0xFC54 MATHB
	.long suRegR				;@ 0xFC55 MATHA
	.long suRegR				;@ 0xFC56 MATHP
	.long suRegR				;@ 0xFC57 MATHN
	.long suUnmappedR			;@ 0xFC58
	.long suUnmappedR			;@ 0xFC59
	.long suUnmappedR			;@ 0xFC5A
	.long suUnmappedR			;@ 0xFC5B
	.long suUnmappedR			;@ 0xFC5C
	.long suUnmappedR			;@ 0xFC5D
	.long suUnmappedR			;@ 0xFC5E
	.long suUnmappedR			;@ 0xFC5F
	.long suRegR				;@ 0xFC60 MATHH
	.long suRegR				;@ 0xFC61 MATHG
	.long suRegR				;@ 0xFC62 MATHF
	.long suRegR				;@ 0xFC63 MATHE
	.long suUnmappedR			;@ 0xFC64
	.long suUnmappedR			;@ 0xFC65
	.long suUnmappedR			;@ 0xFC66
	.long suUnmappedR			;@ 0xFC67
	.long suUnmappedR			;@ 0xFC68
	.long suUnmappedR			;@ 0xFC69
	.long suUnmappedR			;@ 0xFC6A
	.long suUnmappedR			;@ 0xFC6B
	.long suRegR				;@ 0xFC6C MATHM
	.long suRegR				;@ 0xFC6D MATHL
	.long suRegR				;@ 0xFC6E MATHK
	.long suRegR				;@ 0xFC6F MATHJ
	.long suUnmappedR			;@ 0xFC70
	.long suUnmappedR			;@ 0xFC71
	.long suUnmappedR			;@ 0xFC72
	.long suUnmappedR			;@ 0xFC73
	.long suUnmappedR			;@ 0xFC74
	.long suUnmappedR			;@ 0xFC75
	.long suUnmappedR			;@ 0xFC76
	.long suUnmappedR			;@ 0xFC77
	.long suUnmappedR			;@ 0xFC78
	.long suUnmappedR			;@ 0xFC79
	.long suUnmappedR			;@ 0xFC7A
	.long suUnmappedR			;@ 0xFC7B
	.long suUnmappedR			;@ 0xFC7C
	.long suUnmappedR			;@ 0xFC7D
	.long suUnmappedR			;@ 0xFC7E
	.long suUnmappedR			;@ 0xFC7F
	.long suWriteOnlyR			;@ 0xFC80 SPRCTL0
	.long suWriteOnlyR			;@ 0xFC81 SPRCTL1
	.long suWriteOnlyR			;@ 0xFC82 SPRCOLL
	.long suWriteOnlyR			;@ 0xFC83 SPRINIT
	.long suUnmappedR			;@ 0xFC84
	.long suUnmappedR			;@ 0xFC85
	.long suUnmappedR			;@ 0xFC86
	.long suUnmappedR			;@ 0xFC87
	.long suHRevR				;@ 0xFC88 SUZYHREV
	.long suRegR				;@ 0xFC89 SUZYSREV
	.long suUnmappedR			;@ 0xFC8A
	.long suUnmappedR			;@ 0xFC8B
	.long suUnmappedR			;@ 0xFC8C
	.long suUnmappedR			;@ 0xFC8D
	.long suUnmappedR			;@ 0xFC8E
	.long suUnmappedR			;@ 0xFC8F
	.long suWriteOnlyR			;@ 0xFC90 SUZYBUSEN
	.long suWriteOnlyR			;@ 0xFC91 SPRGO
	.long suSprSysR				;@ 0xFC92 SPRSYS
	.long suUnmappedR			;@ 0xFC93
	.long suUnmappedR			;@ 0xFC94
	.long suUnmappedR			;@ 0xFC95
	.long suUnmappedR			;@ 0xFC96
	.long suUnmappedR			;@ 0xFC97
	.long suUnmappedR			;@ 0xFC98
	.long suUnmappedR			;@ 0xFC99
	.long suUnmappedR			;@ 0xFC9A
	.long suUnmappedR			;@ 0xFC9B
	.long suUnmappedR			;@ 0xFC9C
	.long suUnmappedR			;@ 0xFC9D
	.long suUnmappedR			;@ 0xFC9E
	.long suUnmappedR			;@ 0xFC9F
	.long suUnmappedR			;@ 0xFCA0
	.long suUnmappedR			;@ 0xFCA1
	.long suUnmappedR			;@ 0xFCA2
	.long suUnmappedR			;@ 0xFCA3
	.long suUnmappedR			;@ 0xFCA4
	.long suUnmappedR			;@ 0xFCA5
	.long suUnmappedR			;@ 0xFCA6
	.long suUnmappedR			;@ 0xFCA7
	.long suUnmappedR			;@ 0xFCA8
	.long suUnmappedR			;@ 0xFCA9
	.long suUnmappedR			;@ 0xFCAA
	.long suUnmappedR			;@ 0xFCAB
	.long suUnmappedR			;@ 0xFCAC
	.long suUnmappedR			;@ 0xFCAD
	.long suUnmappedR			;@ 0xFCAE
	.long suUnmappedR			;@ 0xFCAF
	.long suJoystickR			;@ 0xFCB0 JOYSTICK
	.long suRegR				;@ 0xFCB1 SWITCHES
	.long cartPeek				;@ 0xFCB2 RCART0
	.long cartPeek				;@ 0xFCB3 RCART1
	.long suUnmappedR			;@ 0xFCB4
	.long suUnmappedR			;@ 0xFCB5
	.long suUnmappedR			;@ 0xFCB6
	.long suUnmappedR			;@ 0xFCB7
	.long suUnmappedR			;@ 0xFCB8
	.long suUnmappedR			;@ 0xFCB9
	.long suUnmappedR			;@ 0xFCBA
	.long suUnmappedR			;@ 0xFCBB
	.long suUnmappedR			;@ 0xFCBC
	.long suUnmappedR			;@ 0xFCBD
	.long suUnmappedR			;@ 0xFCBE
	.long suUnmappedR			;@ 0xFCBF
	.long suRegR				;@ 0xFCC0 LEDS
	.long suUnmappedR			;@ 0xFCC1
	.long suRegR				;@ 0xFCC2 PPORTSTAT
	.long suRegR				;@ 0xFCC3 PPORTDATA
	.long suRegR				;@ 0xFCC4 HOWIE

;@----------------------------------------------------------------------------
suWriteOnlyR:
;@----------------------------------------------------------------------------
;@----------------------------------------------------------------------------
suUnmappedR:
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	stmfd sp!,{suzptr,lr}
	bl _debugIOUnmappedR
	ldmfd sp!,{suzptr,lr}
	mov r0,#0x00
	bx lr
;@----------------------------------------------------------------------------
suUnknownR:
;@----------------------------------------------------------------------------
	ldr r2,=0x826EBAD0
;@----------------------------------------------------------------------------
suImportantR:
	mov r11,r11					;@ No$GBA breakpoint
	stmfd sp!,{r0,suzptr,lr}
	bl _debugIOUnimplR
	ldmfd sp!,{r0,suzptr,lr}
;@----------------------------------------------------------------------------
suRegR:
	and r0,r0,#0xFF
	add r2,suzptr,#suzRegs
	ldrb r0,[r2,r0]
	bx lr
	.pool
;@----------------------------------------------------------------------------
suHRevR:					;@ Suzy HW Revision (0xFC88)
;@----------------------------------------------------------------------------
	mov r0,#1					;@ Revision 1
	bx lr
;@----------------------------------------------------------------------------
suSprSysR:					;@ Sprite Sys (0xFC92)
;@----------------------------------------------------------------------------
	ldrb r0,[suzptr,#suzSprSys]
	and r0,r0,#0x1A				;@ StopOnCurrent, LeftHand & VStretch
	ldrb r1,[suzptr,#suzSprSysStat]
	bic r1,r1,#0x1A
	orr r0,r0,r1
	bx lr
;@----------------------------------------------------------------------------
suJoystickR:				;@ Suzy Joystick (0xFCB0)
;@----------------------------------------------------------------------------
	ldrb r0,[suzptr,#suzSprSys]
	tst r0,#0x08				;@ LeftHand
	ldrb r0,[suzptr,#suzJoystick]
	bxne lr
	adr r1,joyFlipTbl
	and r2,r0,#0xF				;@ Keep buttons
	ldrb r0,[r1,r0,lsr#4]
	orr r0,r2,r0,lsl#4
	bx lr
joyFlipTbl:
	.byte 0x0, 0x2, 0x1, 0x3, 0x8, 0xA, 0x9, 0xB
	.byte 0x4, 0x6, 0x5, 0x7, 0xC, 0xE, 0xD, 0xF
;@----------------------------------------------------------------------------
suzyWrite:					;@ I/O write (0xFC00-0xFCC5)
;@----------------------------------------------------------------------------
	sub r2,r0,#0xFC00
	cmp r2,#0xC5
	ldrmi pc,[pc,r2,lsl#2]
	b suUnmappedW
io_write_tbl:
	.long suRegLW				;@ 0xFC00 TMPADRL
	.long suRegW				;@ 0xFC01 TMPADRH
	.long suRegLW				;@ 0xFC02 TILTACUML
	.long suRegW				;@ 0xFC03 TILTACUMH
	.long suRegLW				;@ 0xFC04 HOFFL
	.long suRegW				;@ 0xFC05 HOFFH
	.long suRegLW				;@ 0xFC06 VOFFL
	.long suRegW				;@ 0xFC07 VOFFH
	.long suRegLW				;@ 0xFC08 VIDBASL
	.long suRegW				;@ 0xFC09 VIDBASH
	.long suRegLW				;@ 0xFC0A COLLBASL
	.long suRegW				;@ 0xFC0B COLLBASH
	.long suRegLW				;@ 0xFC0C VIDADRL
	.long suRegW				;@ 0xFC0D VIDADRH
	.long suRegLW				;@ 0xFC0E COLLADRL
	.long suRegW				;@ 0xFC0F COLLADRH
	.long suRegLW				;@ 0xFC10 SCBNEXTL
	.long suRegW				;@ 0xFC11 SCBNEXTH
	.long suRegLW				;@ 0xFC12 SPRDLINEL
	.long suRegW				;@ 0xFC13 SPRDLINEH
	.long suRegLW				;@ 0xFC14 HPOSSTRTL
	.long suRegW				;@ 0xFC15 HPOSSTRTH
	.long suRegLW				;@ 0xFC16 VPOSSTRTL
	.long suRegW				;@ 0xFC17 VPOSSTRTH
	.long suRegLW				;@ 0xFC18 SPRHSIZL
	.long suRegW				;@ 0xFC19 SPRHSIZH
	.long suRegLW				;@ 0xFC1A SPRVSIZL
	.long suRegW				;@ 0xFC1B SPRVSIZH
	.long suRegLW				;@ 0xFC1C STRETCHL
	.long suRegW				;@ 0xFC1D STRETCHH
	.long suRegLW				;@ 0xFC1E TILTL
	.long suRegW				;@ 0xFC1F TILTH
	.long suRegLW				;@ 0xFC20 SPRDOFFL
	.long suRegW				;@ 0xFC21 SPRDOFFH
	.long suRegLW				;@ 0xFC22 SPRVPOSL
	.long suRegW				;@ 0xFC23 SPRVPOSH
	.long suRegLW				;@ 0xFC24 COLLOFFL
	.long suRegW				;@ 0xFC25 COLLOFFH
	.long suRegLW				;@ 0xFC26 VSIZACUML
	.long suRegW				;@ 0xFC27 VSIZACUMH
	.long suRegLW				;@ 0xFC28 HSIZOFFL
	.long suRegW				;@ 0xFC29 HSIZOFFH
	.long suRegLW				;@ 0xFC2A VSIZOFFL
	.long suRegW				;@ 0xFC2B VSIZOFFH
	.long suRegLW				;@ 0xFC2C SCBADRL
	.long suRegW				;@ 0xFC2D SCBADRH
	.long suRegLW				;@ 0xFC2E PROCADRL
	.long suRegW				;@ 0xFC2F PROCADRH
	.long suUnmappedW			;@ 0xFC30
	.long suUnmappedW			;@ 0xFC31
	.long suUnmappedW			;@ 0xFC32
	.long suUnmappedW			;@ 0xFC33
	.long suUnmappedW			;@ 0xFC34
	.long suUnmappedW			;@ 0xFC35
	.long suUnmappedW			;@ 0xFC36
	.long suUnmappedW			;@ 0xFC37
	.long suUnmappedW			;@ 0xFC38
	.long suUnmappedW			;@ 0xFC39
	.long suUnmappedW			;@ 0xFC3A
	.long suUnmappedW			;@ 0xFC3B
	.long suUnmappedW			;@ 0xFC3C
	.long suUnmappedW			;@ 0xFC3D
	.long suUnmappedW			;@ 0xFC3E
	.long suUnmappedW			;@ 0xFC3F
	.long suUnmappedW			;@ 0xFC40
	.long suUnmappedW			;@ 0xFC41
	.long suUnmappedW			;@ 0xFC42
	.long suUnmappedW			;@ 0xFC43
	.long suUnmappedW			;@ 0xFC44
	.long suUnmappedW			;@ 0xFC45
	.long suUnmappedW			;@ 0xFC46
	.long suUnmappedW			;@ 0xFC47
	.long suUnmappedW			;@ 0xFC48
	.long suUnmappedW			;@ 0xFC49
	.long suUnmappedW			;@ 0xFC4A
	.long suUnmappedW			;@ 0xFC4B
	.long suUnmappedW			;@ 0xFC4C
	.long suUnmappedW			;@ 0xFC4D
	.long suUnmappedW			;@ 0xFC4E
	.long suUnmappedW			;@ 0xFC4F
	.long suUnmappedW			;@ 0xFC50
	.long suUnmappedW			;@ 0xFC51
	.long suMathDW				;@ 0xFC52 MATHD
	.long suMathCW				;@ 0xFC53 MATHC
	.long suRegLW				;@ 0xFC54 MATHB
	.long suMathAW				;@ 0xFC55 MATHA
	.long suRegLW				;@ 0xFC56 MATHP
	.long suRegW				;@ 0xFC57 MATHN
	.long suUnmappedW			;@ 0xFC58
	.long suUnmappedW			;@ 0xFC59
	.long suUnmappedW			;@ 0xFC5A
	.long suUnmappedW			;@ 0xFC5B
	.long suUnmappedW			;@ 0xFC5C
	.long suUnmappedW			;@ 0xFC5D
	.long suUnmappedW			;@ 0xFC5E
	.long suUnmappedW			;@ 0xFC5F
	.long suRegLW				;@ 0xFC60 MATHH
	.long suRegW				;@ 0xFC61 MATHG
	.long suRegLW				;@ 0xFC62 MATHF
	.long suMathEW				;@ 0xFC63 MATHE
	.long suUnmappedW			;@ 0xFC64
	.long suUnmappedW			;@ 0xFC65
	.long suUnmappedW			;@ 0xFC66
	.long suUnmappedW			;@ 0xFC67
	.long suUnmappedW			;@ 0xFC68
	.long suUnmappedW			;@ 0xFC69
	.long suUnmappedW			;@ 0xFC6A
	.long suUnmappedW			;@ 0xFC6B
	.long suRegLW				;@ 0xFC6C MATHM
	.long suRegW				;@ 0xFC6D MATHL
	.long suRegLW				;@ 0xFC6E MATHK
	.long suRegW				;@ 0xFC6F MATHJ
	.long suUnmappedW			;@ 0xFC70
	.long suUnmappedW			;@ 0xFC71
	.long suUnmappedW			;@ 0xFC72
	.long suUnmappedW			;@ 0xFC73
	.long suUnmappedW			;@ 0xFC74
	.long suUnmappedW			;@ 0xFC75
	.long suUnmappedW			;@ 0xFC76
	.long suUnmappedW			;@ 0xFC77
	.long suUnmappedW			;@ 0xFC78
	.long suUnmappedW			;@ 0xFC79
	.long suUnmappedW			;@ 0xFC7A
	.long suUnmappedW			;@ 0xFC7B
	.long suUnmappedW			;@ 0xFC7C
	.long suUnmappedW			;@ 0xFC7D
	.long suUnmappedW			;@ 0xFC7E
	.long suUnmappedW			;@ 0xFC7F
	.long suSprCtl0W			;@ 0xFC80 SPRCTL0
	.long suRegW				;@ 0xFC81 SPRCTL1
	.long suSprCollW			;@ 0xFC82 SPRCOLL
	.long suRegW				;@ 0xFC83 SPRINIT
	.long suUnmappedW			;@ 0xFC84
	.long suUnmappedW			;@ 0xFC85
	.long suUnmappedW			;@ 0xFC86
	.long suUnmappedW			;@ 0xFC87
	.long suReadOnlyW			;@ 0xFC88 SUZYHREV
	.long suRegW				;@ 0xFC89 SUZYSREV
	.long suUnmappedW			;@ 0xFC8A
	.long suUnmappedW			;@ 0xFC8B
	.long suUnmappedW			;@ 0xFC8C
	.long suUnmappedW			;@ 0xFC8D
	.long suUnmappedW			;@ 0xFC8E
	.long suUnmappedW			;@ 0xFC8F
	.long suBusEnW				;@ 0xFC90 SUZYBUSEN
	.long suRegW				;@ 0xFC91 SPRGO
	.long suSprSysW				;@ 0xFC92 SPRSYS
	.long suUnmappedW			;@ 0xFC93
	.long suUnmappedW			;@ 0xFC94
	.long suUnmappedW			;@ 0xFC95
	.long suUnmappedW			;@ 0xFC96
	.long suUnmappedW			;@ 0xFC97
	.long suUnmappedW			;@ 0xFC98
	.long suUnmappedW			;@ 0xFC99
	.long suUnmappedW			;@ 0xFC9A
	.long suUnmappedW			;@ 0xFC9B
	.long suUnmappedW			;@ 0xFC9C
	.long suUnmappedW			;@ 0xFC9D
	.long suUnmappedW			;@ 0xFC9E
	.long suUnmappedW			;@ 0xFC9F
	.long suUnmappedW			;@ 0xFCA0
	.long suUnmappedW			;@ 0xFCA1
	.long suUnmappedW			;@ 0xFCA2
	.long suUnmappedW			;@ 0xFCA3
	.long suUnmappedW			;@ 0xFCA4
	.long suUnmappedW			;@ 0xFCA5
	.long suUnmappedW			;@ 0xFCA6
	.long suUnmappedW			;@ 0xFCA7
	.long suUnmappedW			;@ 0xFCA8
	.long suUnmappedW			;@ 0xFCA9
	.long suUnmappedW			;@ 0xFCAA
	.long suUnmappedW			;@ 0xFCAB
	.long suUnmappedW			;@ 0xFCAC
	.long suUnmappedW			;@ 0xFCAD
	.long suUnmappedW			;@ 0xFCAE
	.long suUnmappedW			;@ 0xFCAF
	.long suReadOnlyW			;@ 0xFCB0 JOYSTICK
	.long suReadOnlyW			;@ 0xFCB1 SWITCHES
	.long cartPoke				;@ 0xFCB2 RCART0
	.long cartPoke				;@ 0xFCB3 RCART1
	.long suUnmappedW			;@ 0xFCB4
	.long suUnmappedW			;@ 0xFCB5
	.long suUnmappedW			;@ 0xFCB6
	.long suUnmappedW			;@ 0xFCB7
	.long suUnmappedW			;@ 0xFCB8
	.long suUnmappedW			;@ 0xFCB9
	.long suUnmappedW			;@ 0xFCBA
	.long suUnmappedW			;@ 0xFCBB
	.long suUnmappedW			;@ 0xFCBC
	.long suUnmappedW			;@ 0xFCBD
	.long suUnmappedW			;@ 0xFCBE
	.long suUnmappedW			;@ 0xFCBF
	.long suImportantW			;@ 0xFCC0 LEDS
	.long suUnmappedW			;@ 0xFCC1
	.long suImportantW			;@ 0xFCC2 PPORTSTAT
	.long suImportantW			;@ 0xFCC3 PPORTDATA
	.long suImportantW			;@ 0xFCC4 HOWIE

;@----------------------------------------------------------------------------
suUnknownW:
;@----------------------------------------------------------------------------
suImportantW:
;@----------------------------------------------------------------------------
	and r0,r0,#0xFF
	add r2,suzptr,#suzRegs
	strb r1,[r2,r0]
	ldr r2,=debugIOUnimplW
	bx r2
;@----------------------------------------------------------------------------
suReadOnlyW:
;@----------------------------------------------------------------------------
suUnmappedW:
;@----------------------------------------------------------------------------
	b _debugIOUnmappedW
;@----------------------------------------------------------------------------
suRegW:
	and r0,r0,#0xFF
	add r2,suzptr,#suzRegs
	strb r1,[r2,r0]
	bx lr
;@----------------------------------------------------------------------------
suRegLW:
	and r0,r0,#0xFE
	and r1,r1,#0xFF
	add r2,suzptr,#suzRegs
	strh r1,[r2,r0]
	bx lr

;@----------------------------------------------------------------------------
suMathDW:					;@ Math D Register (0xFC52)
;@----------------------------------------------------------------------------
	strb r1,[suzptr,#suzMathD]
	mov r1,#0				;@ Set Math C to zero.
;@----------------------------------------------------------------------------
suMathCW:					;@ Math C Register (0xFC53)
;@----------------------------------------------------------------------------
	ldrb r0,[suzptr,#suzMathD]
	orr r0,r0,r1,lsl#8
	ldrb r2,[suzptr,#suzSprSys]
	tst r2,#0x80				;@ SignedMath
	beq noSignedCD
	;@ Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
	sub r1,r0,#1
	tst r1,#0x8000
	rsbne r0,r0,#0				;@ Negate CD
	mov r1,#1
	movne r1,#-1
	str r1,[suzptr,#mathCD_sign]
noSignedCD:
	strh r0,[suzptr,#suzMathCD]
	bx lr
;@----------------------------------------------------------------------------
suMathAW:					;@ Math A Register (0xFC55)
;@----------------------------------------------------------------------------
	ldrb r0,[suzptr,#suzMathB]
	orr r0,r0,r1,lsl#8
	ldrb r2,[suzptr,#suzSprSys]
	tst r2,#0x80				;@ SignedMath
	beq noSignedAB
	;@ Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
	sub r1,r0,#1
	tst r1,#0x8000
	rsbne r0,r0,#0				;@ Negate AB
	mov r1,#1
	movne r1,#-1
	str r1,[suzptr,#mathAB_sign]
noSignedAB:
	strh r0,[suzptr,#suzMathAB]
;@----------------------------------------------------------------------------
;@ suzDoMultiply:			;@
;@----------------------------------------------------------------------------
	mov r1,#0
	str r1,[suzptr,#sprSys_Mathbit]
	ldrh r0,[suzptr,#suzMathAB]
	ldrh r1,[suzptr,#suzMathCD]
	mul r3,r1,r0
	tst r2,#0x80				;@ SignedMath
	beq noSignedMult
	ldrd r0,r1,[suzptr,#mathAB_sign]	;@ r1=mathCD_sign
//	ldr r1,[suzptr,#mathCD_sign]
	adds r0,r0,r1
	rsbeq r3,r3,#0
noSignedMult:
	str r3,[suzptr,#suzMathEFGH]

	tst r2,#0x40				;@ Accumulate
	ldrne r0,[suzptr,#suzMathJKLM]
	addne r0,r0,r3
	strne r0,[suzptr,#suzMathJKLM]

	bx lr

;@----------------------------------------------------------------------------
suMathEW:					;@ Math E Register (0xFC63)
;@----------------------------------------------------------------------------
	strb r1,[suzptr,#suzMathE]
;@----------------------------------------------------------------------------
;@ suzDoDivide:				;@
;@----------------------------------------------------------------------------
	ldrh r1,[suzptr,#suzMathNP]
	cmp r1,#0
	mov r2,#0
	moveq r2,#1
	str r2,[suzptr,#sprSys_Mathbit]
	moveq r0,#-1
	beq zeroDvide
	ldr r0,[suzptr,#suzMathEFGH]
	stmfd sp!,{lr}
	bl ui32div
	ldmfd sp!,{lr}
zeroDvide:
	str r1,[suzptr,#suzMathJKLM]
	mov r1,r0,lsr#16
	strh r1,[suzptr,#suzMathAB]
	strh r0,[suzptr,#suzMathCD]
	bx lr
;@----------------------------------------------------------------------------
suSprCtl0W:					;@ Sprite Control 0 (0xFC80)
;@----------------------------------------------------------------------------
	strb r1,[suzptr,#suzSprCtl0]
	mov r1,r1,lsr#6
	add r1,r1,#1
	strb r1,[suzptr,#suzSprCtl0_PixelBits]
	bx lr
;@----------------------------------------------------------------------------
suSprCollW:					;@ Sprite Collision Number (0xFC82)
;@----------------------------------------------------------------------------
	and r1,r1,#0x2F
	strb r1,[suzptr,#suzSprColl]
	bx lr
;@----------------------------------------------------------------------------
suBusEnW:					;@ Suzy Bus Enable (0xFC90)
;@----------------------------------------------------------------------------
	and r1,r1,#0x01
	strb r1,[suzptr,#suzSuzyBusEn]
	bx lr
;@----------------------------------------------------------------------------
suSprSysW:					;@ Sprite Sys (0xFC92)
;@----------------------------------------------------------------------------
	tst r1,#0x04				;@ Clear UnsafeAccess bit?
	ldrbne r0,[suzptr,#suzSprSysStat]
	bicne r0,r0,#0x04
	strbne r0,[suzptr,#suzSprSysStat]
	strb r1,[suzptr,#suzSprSys]
	bx lr

;@----------------------------------------------------------------------------
suzySetButtonData:			;@ r0=buttons & switches, r12=suzptr.
;@----------------------------------------------------------------------------
	strh r0,[suzptr,#suzJoystick]
	bx lr

;@----------------------------------------------------------------------------
suzPaintSprites:			;@ Out r0=cycles used
	.type	suzPaintSprites STT_FUNC
;@----------------------------------------------------------------------------
	ldr suzptr,=suzy_0
	ldrh r0,[suzptr,#suzSuzyBusEn]
	and r0,r0,r0,lsr#8
	ands r0,r0,#1					;@ Is BusEnable & SprGo set?
	bxeq lr

	stmfd sp!,{r4-r6,r9,lr}
	mov r9,#0						;@ CyclesUsed
	str r9,[suzptr,#everOnScreen]
	mov r4,#0
	mov r0,#1
	str r0,[suzptr,#sprSys_Busy]
spriteLoop:
	ldrb r0,[suzptr,#suzSCBNextH]
	cmp r0,#0
	beq exitPaintSprite

	bl suzFetchSpriteData

	ldrb r0,[suzptr,#suzSprCtl1]
	ands r0,r0,#0x04				;@ Skip sprite?
	bne skipSprite
	strb r0,[suzptr,#suzCollision]

	bl suzRenderQuads

	ldrb r0,[suzptr,#suzSprColl]
	cmp r0,#0x10
	bcs noSprCollWrite
	ldrb r0,[suzptr,#suzSprCtl0]
	ands r0,r0,#0x07				;@ Sprite Type, sprite_background_shadow
	cmpne r0,#1						;@ sprite_background_noncollide
	cmpne r0,#5						;@ sprite_noncollide
	beq noSprCollWrite
	ldrh r0,[suzptr,#suzSCBAdr]
	ldrh r1,[suzptr,#suzCollOff]
	ldr r2,[suzptr,#suzyRAM]
	add r0,r0,r1
	mov r0,r0,lsl#16
	ldrb r1,[suzptr,#suzCollision]
	strb r1,[r2,r0,lsr#16]
noSprCollWrite:

	ldrb r0,[suzptr,#suzSprGo]
	tst r0,#0x04					;@ Everon?
	beq skipSprite
	ldrh r0,[suzptr,#suzSCBAdr]
	ldrh r1,[suzptr,#suzCollOff]
	ldr r2,[suzptr,#suzyRAM]
	add r0,r0,r1
	mov r0,r0,lsl#16
	ldrb r1,[r2,r0,lsr#16]
	ldr r3,[suzptr,#everOnScreen]
	cmp r3,#0
	orreq r1,r1,#0x80
	bicne r1,r1,#0x80
	strb r1,[r2,r0,lsr#16]

skipSprite:
	ldrb r0,[suzptr,#suzSprSys]
	tst r0,#0x02					;@ Stop on Current?
	bne exitPaintSprite
	add r4,r4,#1
	cmp r4,#0x1000
	bmi spriteLoop
	;@ Something fishy going on...

exitPaintSprite:
	mov r0,#0
	str r0,[suzptr,#sprSys_Busy]
	ldrb r0,[suzptr,#suzSprGo]
	bic r0,r0,#1
	strb r0,[suzptr,#suzSprGo]

	mov r0,r9
	ldmfd sp!,{r4-r6,r9,lr}
	bx lr
;@----------------------------------------------------------------------------
suzFetchSpriteData:			;@ In/Out r9=cyclesUsed
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	ldr r5,[suzptr,#suzyRAM]
	ldrh r4,[suzptr,#suzSCBNext]
	strh r4,[suzptr,#suzSCBAdr]
//	strh r4,[suzptr,#suzTmpAdr]
	add r4,r4,r5

	ldrb r1,[r4],#1
	ldr r2,=sprTypeTbl
	and r0,r1,#7
	ldr r3,[r2,r0,lsl#2]
	str r3,[suzptr,#suzSprTypeFunc]
	bl suSprCtl0W

	ldrb r6,[r4],#1
	strb r6,[suzptr,#suzSprCtl1]

	ldrb r0,[r4],#1
	ldrb r1,[suzptr,#suzSprSys]
	and r0,r0,#0x2F
	and r1,r1,#0x20
	orr r0,r0,r1
	strb r0,[suzptr,#suzSprColl]

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzSCBNext]

	add r9,r9,#5*3				;@ 5 * SPR_RDWR_CYC

	tst r6,#0x04				;@ sprCtl1 - SkipSprite
	bne skipPalette

	mov r0,#0
	strb r0,[suzptr,#suzCollision]

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzSprDLine]

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzHPosStrt]

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzVPosStrt]

	add r9,r9,#6*3				;@ 6 * SPR_RDWR_CYC

	ands r2,r6,#0x30			;@ sprCtl1 - ReloadDepth
	beq endSpriteFetch

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzSprHSiz]

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzSprVSiz]

	add r9,r9,#4*3				;@ 4 * SPR_RDWR_CYC

	cmp r2,#0x20
	bcc endSpriteFetch

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzStretch]

	add r9,r9,#2*3				;@ 2 * SPR_RDWR_CYC

	beq endSpriteFetch

	ldrb r0,[r4],#1
	ldrb r1,[r4],#1
	orr r0,r0,r1,lsl#8
	strh r0,[suzptr,#suzTilt]

	add r9,r9,#2*3				;@ 2 * SPR_RDWR_CYC

endSpriteFetch:
	tst r6,#0x08				;@ sprCtl1 - !ReloadPalette
	bne skipPalette
	add r0,suzptr,#suzPenIndex
	mov r1,#8
penLoop:
	ldrb r2,[r4],#1
	mov r2,r2,ror#4
	orr r2,r2,r2,lsr#16+4
	strh r2,[r0],#2
	subs r1,r1,#1
	bne penLoop
	add r9,r9,#8*3				;@ 8 * SPR_RDWR_CYC

skipPalette:
	sub r4,r4,r5
	strh r4,[suzptr,#suzTmpAdr]
	ldmfd sp!,{r4-r6,lr}
	bx lr
;@----------------------------------------------------------------------------
suzRenderQuads:				;@ In/Out r9=cyclesUsed
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,r10,r11,lr}

	;@ Quadrant drawing order is: SE,NE,NW,SW
	;@ start quadrant is given by sprite_control1:0 & 1

	mov r10,#0x10000			;@ r10=hSign
	mov r1,#0x10000				;@ r1=vSign
	ldrh r0,[suzptr,#suzSprCtl0]	;@ SprCtl0 & SprCtl1
	ands r4,r0,#0x0100			;@ StartLeft, r4=start Quadrant
	movne r4,#1
	rsbne r10,r10,#0
	tst r0,#0x0200				;@ StartUp
	eorne r4,r4,#1
	rsbne r1,r1,#0
	teq r0,r0,lsl#27			;@ Check VFlip & HFlip
	rsbmi r1,r1,#0
	rsbcs r10,r10,#0

	mov r6,r10					;@ r6=hQuadOff
	mov r7,r1					;@ r7=vQuadOff
	mov r5,#4					;@ Quad count
quadLoop:
	ldrh r8,[suzptr,#suzVPosStrt]
	ldrh r0,[suzptr,#suzVOff]
	mov r8,r8,lsl#16
	sub r8,r8,r0,lsl#16			;@ r8=vOff
	orr r8,r8,r1,lsr#16			;@ Add vSign to bottom
	;@ Take the sign of the first quad (0) as the basic
	;@ sign, all other quads drawing in the other direction
	;@ get offset by 1 pixel in the other direction, this
	;@ fixes the squashed look on the multi-quad sprites.
	cmp r7,r8,lsl#16
	addne r8,r8,r8,lsl#16

	;@ Zero the stretch, tilt & acum values
	mov r0,#0
	strh r0,[suzptr,#suzTiltAcum]

	ldrh r11,[suzptr,#suzSprVSiz]
	;@ Perform the SIZOFF
	cmp r1,#0x10000
	ldrheq r0,[suzptr,#suzVSizOff]		;@ Start value of VSizAcum
	orr r11,r11,r0,lsl#16

verticalLoop:
;@------------------------------------
suzLineStart:
;@------------------------------------
	ldrh r1,[suzptr,#suzSprDLine]
	ldr r2,[suzptr,#suzyRAM]
	ldrb r0,[r2,r1]
	add r9,r9,#3				;@ SPR_RDWR_CYC

	strh r0,[suzptr,#suzSprDOff]
	cmp r0,#1
	addeq r1,r1,r0
	strheq r1,[suzptr,#suzSprDLine]
	beq exitQuad
	bcc exitQuadLoop

	add r11,r11,r11,lsl#16
	movs r0,r11,lsr#24
	beq breakV2Loop
	sub r11,r11,r0,lsl#25
v2Loop:
	cmp r8,#GAME_HEIGHT<<16
	bcs checkVBail
//	mov r0,r10					;@ hSign
	mov r1,r6					;@ hQuadOff
	mov r2,r8,lsr#16			;@ vOff
	bl suzLineRender
keepRendering:
	add r8,r8,r8,lsl#16			;@ vOff += vSign
	ldrb r0,[suzptr,#suzSprCtl1]
	movs r0,r0,lsl#27			;@ Check ReloadDepth
	bcc noStretchTilt
	bpl noTilt
	ldrh r0,[suzptr,#suzTilt]
	ldrh r1,[suzptr,#suzTiltAcum]
	add r1,r1,r0
	strh r1,[suzptr,#suzTiltAcum]
noTilt:
	ldrh r0,[suzptr,#suzStretch]
	ldrh r1,[suzptr,#suzSprHSiz]
	add r1,r1,r0
	strh r1,[suzptr,#suzSprHSiz]
	ldrb r2,[suzptr,#suzSprSys]
	tst r2,#0x10				;@ Check VStretch
	ldrhne r1,[suzptr,#suzSprVSiz]
	addne r1,r1,r0
	strhne r1,[suzptr,#suzSprVSiz]
noStretchTilt:
	adds r11,r11,#0x01000000
	bcc v2Loop
breakV2Loop:
	ldrh r0,[suzptr,#suzSprDOff]
	ldrh r1,[suzptr,#suzSprDLine]
	add r1,r1,r0
	strh r1,[suzptr,#suzSprDLine]
	b verticalLoop
exitQuad:
	mov r1,r8,lsl#16
	;@ Flip quadrant bit value (0-1)
	eors r4,r4,#1
	rsbeq r10,r10,#0			;@ hSign = -hSign
	rsbne r1,r1,#0				;@ vSign = -vSign

	subs r5,r5,#1
	bne quadLoop

exitQuadLoop:
//	mov r11,r11,lsr#16
//	strh r11,[suzptr,#suzVSizAcum]
	ldmfd sp!,{r4-r8,r10,r11,lr}
	bx lr
checkVBail:
	movsmi r1,r8,lsl#16
	bmi breakV2Loop
//	cmppl r1,#0
//	bpl breakV2Loop
	b keepRendering
;@----------------------------------------------------------------------------
suzLineRender:				;@ In r10=hSign, r1=hQuadOff, r2=vOff.
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,r10,r11,lr}

	ldr r6,[suzptr,#suzVidBas]	;@ Also suzCollBas
	ldr r7,[suzptr,#suzyRAM]
	mov r5,r6,lsl#16
	add r2,r2,r2,lsl#2			;@ *5
	add r5,r5,r2,lsl#4+16		;@ *16
	add r8,r7,r5,lsr#16
//	str r8,[suzptr,#suzLineBaseAddress]
	add r6,r6,r2,lsl#4+16		;@ *16
	add r6,r7,r6,lsr#16
	str r6,[suzptr,#suzLineCollisionAddress]

	ldrh r5,[suzptr,#suzSprDLine]
	ldrb r2,[suzptr,#suzSprDOff]
	add r5,r5,#1
	strh r5,[suzptr,#suzTmpAdr]
	sub r5,r2,#1
	mov r5,r5,lsl#3
	str r5,[suzptr,#suzLinePacketBitsLeft]

	add r9,r9,#3				;@ SPR_RDWR_CYC

	mov r7,#0
	str r7,[suzptr,#suzLineShiftRegCount]

	ldrb r6,[suzptr,#suzSprCtl1]
	ands r6,r6,#0x80			;@ Literal -> line_abs_literal
	moveq r5,r7					;@ LineRepeatCount = LinePacketBitsLeft
	strb r6,[suzptr,#suzLineType]
	str r5,[suzptr,#suzLineRepeatCount]

;@----------------------------------------------------------------------------
suzRenderLine:				;@ In r10=hSign, r1=hQuadOff.
;@----------------------------------------------------------------------------

	;@ Work out the horizontal pixel start position, start + tilt
	ldrsh r3,[suzptr,#suzHPosStrt]
	ldrsb r4,[suzptr,#suzTiltAcumH]
	ldrsh r2,[suzptr,#suzHOff]
	add r3,r3,r4
	strh r3,[suzptr,#suzHPosStrt]
	strb r7,[suzptr,#suzTiltAcumH]	;@ Zero TiltAcumH
	sub r4,r3,r2				;@ r4=hOff
	mov r4,r4,lsl#16
	orr r4,r4,r10,lsr#16
	;@ Take the sign of the first quad (0) as the basic
	;@ sign, all other quads drawing in the other direction
	;@ get offset by 1 pixel in the other direction, this
	;@ fixes the squashed look on the multi-quad sprites.
	cmp r1,r4,lsl#16
	addne r4,r4,r4,lsl#16

	ldr r11,[suzptr,#suzSprTypeFunc]
	ldrh r6,[suzptr,#suzSprHSiz]
	;@ Zero/Force the horizontal scaling accumulator
	adds r0,r10,#0x10000
	ldrhcc r0,[suzptr,#suzHSizOff]	;@ If hSign == 1
	orr r6,r6,r0,lsl#16
//	mov r7,#0					;@ Bottom part is "onScreen"
horizontalLoop:
	bl suzLineGetPixel			;@ Returns pixel in r5.
	cmp r5,#0x80
	beq exitRender
	add r6,r6,r6,lsl#16			;@ HSIZACUM.Word += SPRHSIZ.Word
	movs r0,r6,lsr#24			;@ pixel_width = HSIZACUM.Byte.High
	beq horizontalLoop
	sub r6,r6,r0,lsl#25			;@ HSIZACUM.Byte.High = 0
rendLoop:
	cmp r4,#GAME_WIDTH<<16
	bcs checkBail
	blx r11						;@ ProcessPixel(hOff, pix)
	orr r7,r7,#1				;@ onScreen = TRUE
continueRend:
	add r4,r4,r4,lsl#16			;@ hOff += hSign
	adds r6,r6,#0x01000000
	bcc rendLoop
	b horizontalLoop
checkBail:
	tst r7,#1
	beq continueRend
exitRender:
	ands r7,r7,#1				;@ onScreen
	strne r7,[suzptr,#everOnScreen]
	ldmfd sp!,{r4-r8,r10,r11,lr}
	bx lr

;@----------------------------------------------------------------------------
suzGetPixelBits:			;@ Out r0=bits.
;@----------------------------------------------------------------------------
	ldrb r0,[suzptr,#suzSprCtl0_PixelBits]
;@----------------------------------------------------------------------------
suzLineGetBits:				;@ In r0=bitCount, less or equal to 8, Out r0=bits.
;@----------------------------------------------------------------------------
	ldr r1,[suzptr,#suzLinePacketBitsLeft]
	subs r1,r1,r0
	movle r0,#0
	bxle lr
	str r1,[suzptr,#suzLinePacketBitsLeft]

	ldrd r2,r3,[suzptr,#suzLineShiftRegCount]
//	ldr r3,[suzptr,#suzLineShiftReg]
	subs r2,r2,r0
	bcc fetchNewBits
extractBits:
	str r2,[suzptr,#suzLineShiftRegCount]
	rsb r1,r0,#32
	sub r2,r1,r2
	mov r3,r3,lsl r2
	mov r0,r3,lsr r1
	bx lr

fetchNewBits:
	stmfd sp!,{r4-r5}
	ldr r4,[suzptr,#suzyRAM]
	ldrh r5,[suzptr,#suzTmpAdr]
	add r2,r2,#24
	ldrb r1,[r4,r5]!
	orr r3,r1,r3,lsl#8
	ldrb r1,[r4,#1]
	orr r3,r1,r3,lsl#8
	ldrb r1,[r4,#2]
	add r5,r5,#3
	orr r3,r1,r3,lsl#8
	strh r5,[suzptr,#suzTmpAdr]
	str r3,[suzptr,#suzLineShiftReg]

	add r9,r9,#3*3				;@ 3 * SPR_RDWR_CYC

	ldmfd sp!,{r4-r5}
	b extractBits

;@----------------------------------------------------------------------------
suzLineGetPixel:			;@ Out r5=pixel
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr r1,[suzptr,#suzLineRepeatCount]
	ldrb r2,[suzptr,#suzLineType]
	subs r0,r1,#1
	bmi fetchPacket

fetchPixel:
	movs r2,r2,lsr#1			;@ Check bit #0 & #7
	bne doLiteralLine
checkMoreLineType:
	str r0,[suzptr,#suzLineRepeatCount]
	bcc fetchPacked				;@ line_packed?
	bl suzGetPixelBits
	add r1,suzptr,#suzPenIndex
	ldrb r5,[r1,r0]
	ldmfd sp!,{pc}
fetchPacked:
	ldrb r5,[suzptr,#suzLinePixel]
	ldmfd sp!,{pc}
doLiteralLine:					;@ line_abs_literal
	ldrb r0,[suzptr,#suzSprCtl0_PixelBits]
	subs r5,r1,r0
	cmp r5,r0
	movcc r5,#0
	str r5,[suzptr,#suzLineRepeatCount]
	bl suzLineGetBits
	orrs r5,r5,r0
	moveq r5,#LINE_END
	addne r1,suzptr,#suzPenIndex
	ldrbne r5,[r1,r0]
	ldmfd sp!,{pc}

fetchPacket:
	cmp r2,#0x80				;@ line_abs_literal
	beq exitLineEnd
	mov r0,#5
	bl suzLineGetBits
	movs r5,r0,ror#4
	beq exitLineEnd
	and r0,r0,#0xF
	str r0,[suzptr,#suzLineRepeatCount]
	strb r5,[suzptr,#suzLineType]
	bl suzGetPixelBits
	tst r5,#1					;@ line_literal
	add r1,suzptr,#suzPenIndex
	ldrb r5,[r1,r0]
	strbeq r5,[suzptr,#suzLinePixel]
	ldmfd sp!,{pc}

exitLineEnd:
	mov r5,#LINE_END
	ldmfd sp!,{pc}
;@----------------------------------------------------------------------------
;@ suzProcessPixel:			;@ In r4=hoff, r5=pixel
;@----------------------------------------------------------------------------
sprTypeTbl:
	.long sprBgrShdw, sprBgrNoColl, sprBoundShdw, sprBound
	.long sprNormal,  sprNoColl,    sprXorShdw,   sprShadow

// BACKGROUND SHADOW
// 1   F is opaque
// 0   E is collideable
// 1   0 is opaque and collideable
// 0   allow collision detect
// 1   allow coll. buffer access
// 0   exclusive-or the data
sprBgrShdw:
	cmp r5,#0xE
	beq suzWritePixel
	stmfd sp!,{lr}
	bl suzWritePixel			;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzWriteCollision
// BOUNDARY_SHADOW
// 0   F is opaque
// 0   E is collideable
// 0   0 is opaque and collideable
// 1   allow collision detect
// 1   allow coll. buffer access
// 0   exclusive-or the data
sprBoundShdw:
	cmp r5,#0
	cmpne r5,#0xE				;@ This seems weird
	bxeq lr
	cmp r5,#0xF
	beq suzTestCollision		;@ Only collision
	stmfd sp!,{lr}
	bl suzWritePixel			;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzTestCollision
// BOUNDARY
// 0   F is opaque
// 1   E is collideable
// 0   0 is opaque and collideable
// 1   allow collision detect
// 1   allow coll. buffer access
// 0   exclusive-or the data
sprBound:
	cmp r5,#0
	bxeq lr
	cmp r5,#0xF
	beq suzTestCollision		;@ Only collision
	stmfd sp!,{lr}
	bl suzWritePixel			;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzTestCollision
// NORMAL
// 1   F is opaque
// 1   E is collideable
// 0   0 is opaque and collideable
// 1   allow collision detect
// 1   allow coll. buffer access
// 0   exclusive-or the data
sprNormal:
	cmp r5,#0
	bxeq lr
	stmfd sp!,{lr}
	bl suzWritePixel			;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzTestCollision
// XOR SHADOW
// 1   F is opaque
// 0   E is collideable
// 0   0 is opaque and collideable
// 1   allow collision detect
// 1   allow coll. buffer access
// 1   exclusive-or the data
sprXorShdw:
	cmp r5,#0
	bxeq lr
	cmp r5,#0xE
	beq suzXorPixel
	stmfd sp!,{lr}
	bl suzXorPixel				;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzTestCollision
// SHADOW
// 1   F is opaque
// 0   E is collideable
// 0   0 is opaque and collideable
// 1   allow collision detect
// 1   allow coll. buffer access
// 0   exclusive-or the data
sprShadow:
	cmp r5,#0
	bxeq lr
	cmp r5,#0xE
	beq suzWritePixel
	stmfd sp!,{lr}
	bl suzWritePixel			;@ r0 is not modified
	ldmfd sp!,{lr}
	b suzTestCollision
;@----------------------------------------------------------------------------
// NOCOLLIDE
// 1   F is opaque
// 0   E is collideable
// 0   0 is opaque and collideable
// 0   allow collision detect
// 0   allow coll. buffer access
// 0   exclusive-or the data
sprNoColl:
	cmp r5,#0
	bxeq lr
;@----------------------------------------------------------------------------
// BACKGROUND NOCOLLIDE
// 1   F is opaque
// 0   E is collideable
// 1   0 is opaque and collideable
// 0   allow collision detect
// 0   allow coll. buffer access
// 0   exclusive-or the data
sprBgrNoColl:
;@----------------------------------------------------------------------------
suzWritePixel:				;@ In r4=hoff, r5=pixel.
;@----------------------------------------------------------------------------
	ldrb r3,[r8,r4,lsr#17]
	tst r4,#0x10000
	andeq r3,r3,#0x0F
	orreq r3,r3,r5,lsl#4
	andne r3,r3,#0xF0
	orrne r3,r3,r5
	strb r3,[r8,r4,lsr#17]

	add r9,r9,#2*3				;@ 2*SPR_RDWR_CYC

	bx lr
;@----------------------------------------------------------------------------
suzXorPixel:				;@ In r4=hoff, r5=pixel.
;@----------------------------------------------------------------------------
	ldrb r3,[r8,r4,lsr#17]
	tst r4,#0x10000
	eoreq r3,r3,r5,lsl#4
	eorne r3,r3,r5
	strb r3,[r8,r4,lsr#17]

	add r9,r9,#3*3				;@ 3*SPR_RDWR_CYC

	bx lr
;@----------------------------------------------------------------------------
suzWriteCollision:			;@ In r4=hoff.
;@----------------------------------------------------------------------------
	ldrb r1,[suzptr,#suzSprColl]
	cmp r1,#0x10
	bxpl lr
	ldr r2,[suzptr,#suzLineCollisionAddress]
	tst r4,#0x10000
	ldrb r3,[r2,r4,lsr#17]
	andeq r3,r3,#0x0F
	orreq r3,r3,r1,lsl#4
	andne r3,r3,#0xF0
	orrne r3,r3,r1
	strb r3,[r2,r4,lsr#17]

	add r9,r9,#2*3				;@ 2*SPR_RDWR_CYC

	bx lr
;@----------------------------------------------------------------------------
suzTestCollision:			;@ In r4=hoff. Out r0=collision
;@----------------------------------------------------------------------------
	ldrb r1,[suzptr,#suzSprColl]
	cmp r1,#0x10
	bxpl lr
	ldr r2,[suzptr,#suzLineCollisionAddress]
	tst r4,#0x10000
	ldrb r3,[r2,r4,lsr#17]
	moveq r0,r3,lsr#4
	andeq r3,r3,#0x0F
	orreq r3,r3,r1,lsl#4
	andne r0,r3,#0x0F
	andne r3,r3,#0xF0
	orrne r3,r3,r1
	strb r3,[r2,r4,lsr#17]

	ldrb r2,[suzptr,#suzCollision]
	cmp r2,r0
	strbmi r0,[suzptr,#suzCollision]

	add r9,r9,#3*3				;@ 3*SPR_RDWR_CYC

	bx lr

;@----------------------------------------------------------------------------
#ifdef GBA
	.section .iwram, "ax", %progbits	;@ For the GBA
	.align 2
#endif
;@----------------------------------------------------------------------------
#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 2
CHR_DECODE:
	.space 0x200
SCROLL_BUFF:
	.space 160*4

#endif // #ifdef __arm__
