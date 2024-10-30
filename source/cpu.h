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

/**
 * Executes one instruction and returns the number of cycles consumed
 */
int stepInstruction(void);
void cpuSetIrqPin(int state);
void cpuReset(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CPU_HEADER
