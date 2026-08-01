//
//  Memory.h
//  LodjurDS
//
//  Created by Fredrik Ahlström on 2024-07-29.
//  Copyright © 2024-2026 Fredrik Ahlström. All rights reserved.
//
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

#endif // !MEMORY_HEADER
