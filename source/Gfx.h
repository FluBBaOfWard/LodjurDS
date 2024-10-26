#ifndef GFX_HEADER
#define GFX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ARMSuzy/ARMSuzy.h"
#include "ARMMikey/ARMMikey.h"

extern u8 gFlicker;
extern u8 gTwitch;
extern u8 gGfxMask;

extern SUZY suzy_0;
extern MIKEY mikey_0;
extern u16 MAPPED_RGB[0x1000];
extern u16 EMUPALBUFF[0x200];
extern u32 GFX_DISPCNT;
extern u16 GFX_BG0CNT;
extern u16 GFX_BG1CNT;

void gfxInit(void);
void lodjurRenderCallback(u8 *source, u32 *palette, bool flip);
void vblIrqHandler(void);
void paletteInit(u8 gammaVal);
void updateLCDRefresh(void);
void gfxRefresh(void);
u8 lnxSuzyRead(u16 adr);
void lnxSuzyWrite(u16 adr, u8 value);
u8 lnxMikeyRead(u16 adr);
void lnxMikeyWrite(u16 adr, u8 value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GFX_HEADER
