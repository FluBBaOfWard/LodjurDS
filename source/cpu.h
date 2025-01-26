#ifndef CPU_HEADER
#define CPU_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ARMMikey/ARMMikey.h"

extern M6502Core m6502_0;
extern MIKEY mikey_0;
extern u8 waitMaskIn;
extern u8 waitMaskOut;

void run(void);
void stepFrame(void);

void cpuReset(void);

/** 1 is Lynx (M65C02), 2 is Lynx II (R65C02) */
void cpuInit(int lynxType);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CPU_HEADER
