#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

void rom_W(u32 addr, u8 value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// MEMORY_HEADER
