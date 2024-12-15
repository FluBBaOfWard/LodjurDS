//
//  LynxCart.h
//  Atari Lynx cartridge emulation for ARM32.
//
//  Created by Fredrik Ahlström on 2024-12-08.
//  Copyright © 2024 Fredrik Ahlström. All rights reserved.
//

#ifndef LYNXCART_HEADER
#define LYNXCART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u8 *cartBank0;		// Pointer to cart bank 0
	u8 *cartBank1;		// Pointer to cart bank 1
	u16 cartCountMask0;	//
	u16 cartCountMask1;	//
	u8 cartShiftCount0;	//
	u8 cartShiftCount1;	//
	u8 cartWriteEnable0;//
	u8 cartWriteEnable1;//
//cartState:
	u16 cartCounter;	//
	u8 cartShifter;		//
	u8 cartDataBit;		// Strobe data bit
	u8 cartStrobe;		// Strobe
	u8 cartCurrentBank;	// Current Bank
	u8 padding0[2];
} CART;

void cartReset(CART *chip, const void *source);

/**
 * Saves the state of the chip to the destination.
 * @param  *destination: Where to save the state.
 * @param  *chip: The CART chip to save.
 * @return The size of the state.
 */
int cartSaveState(void *destination, const CART *chip);

/**
 * Loads the state of the chip from the source.
 * @param  *chip: The CART chip to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int cartLoadState(CART *chip, const void *source);

/**
 * Gets the state size of a CART chip.
 * @return The size of the state.
 */
int cartGetStateSize(void);

void cartAddressData(CART *chip, bool data);

void cartAddressStrobe(CART *chip, bool strobe);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LYNXCART_HEADER
