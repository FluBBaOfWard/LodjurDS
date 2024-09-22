#ifndef CPU_HEADER
#define CPU_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ARM6502/M6502.h"

extern M6502Core m6502_0;
extern u8 waitMaskIn;
extern u8 waitMaskOut;

void run(void);
void stepFrame(void);
void cpuReset(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CPU_HEADER
