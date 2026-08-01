//
//  Sound.h
//  LodjurDS
//
//  Created by Fredrik Ahlström on 2024-07-29.
//  Copyright © 2024-2026 Fredrik Ahlström. All rights reserved.
//
#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <maxmod9.h>

#define sample_rate 31250
#define buffer_size (256+10)

void soundInit(void);
void soundSetMuteGUI(void);
void setSoundChipEnable(bool enable);
mm_word soundRender(mm_word length, mm_addr dest, mm_stream_formats format);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !SOUND_HEADER
