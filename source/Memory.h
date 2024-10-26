#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

extern u8 memSelector;

//void pokeCPU(u32 addr, u8 value);
//u8 peekCPU(u32 addr);
void rom_W(u32 addr, u8 value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// MEMORY_HEADER
