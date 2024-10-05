#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

extern u8 memSelector;

void rom_W(u32 addr, u8 value);
void ramPoke(u32 addr, u8 value);
u8 ramPeek(u32 addr);
u8 romPeek(u32 addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// MEMORY_HEADER
