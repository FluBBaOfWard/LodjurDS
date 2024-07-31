#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 gRomSize;
extern u32 maxRomSize;
extern u32 allocatedRomMemSize;
extern u8 gConfig;
extern u8 gMachine;
extern u8 gMachineSet;
extern u8 gSOC;
extern u8 gLang;
extern u8 gPaletteBank;
extern const int sramSize;
extern const int eepromSize;

extern u8 lynxRAM[0x10000];
extern u8 wsSRAM[0x8000];
extern u8 biosSpace[0x1000];
extern u8 *romSpacePtr;
extern u8 *allocatedRomMem;
extern const void *g_BIOSBASE;

void machineInit(void);
void loadCart(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CART_HEADER
