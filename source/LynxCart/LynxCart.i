//
//  LynxCart.i
//  Atari Lynx Cartridge emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-12-08.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//
;@ ASM header for the Atari Lynx Cartridge emulator

						;@ LynxCart.s
	.struct 0
cartStruct:						;@
cartBank0:			.long 0		;@ Pointer to cart bank 0
cartBank1:			.long 0		;@ Pointer to cart bank 1
cartCountMask0:		.short 0	;@
cartCountMask1:		.short 0	;@
cartShiftCount0:	.byte 0		;@
cartShiftCount1:	.byte 0		;@
cartWriteEnable0:	.byte 0		;@
cartWriteEnable1:	.byte 0		;@
cartState:						;@
cartRegs:
cartCounter:		.short 0	;@
cartShifter:		.byte 0		;@
cartDataBit:		.byte 0		;@ Strobe data bit
cartStrobe:			.byte 0		;@ Strobe
cartCurrentBank:	.byte 0		;@ Current Bank
cartPadding0:		.skip 2
cartStateEnd:

cartSize:

;@----------------------------------------------------------------------------
