#ifndef GFX_HEADER
#define GFX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ARMSuzy/ARMSuzy.h"

extern u8 gFlicker;
extern u8 gTwitch;
extern u8 gGfxMask;

extern SUZY suzy_0;
extern u16 MAPPED_RGB[0x1000];
extern u16 EMUPALBUFF[0x200];
extern u32 GFX_DISPCNT;
extern u16 GFX_BG0CNT;
extern u16 GFX_BG1CNT;

void gfxInit(void);
void lodjurRenderCallback(u8 *source, u32 *palette, bool flip, bool palChg);
void vblIrqHandler(void);
void paletteInit(u8 gammaVal, u8 contrast);
void updateLCDRefresh(void);
void gfxRefresh(void);
void lowerRefresh(void);
void lnxSuzySetButtonData(u16 value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GFX_HEADER
