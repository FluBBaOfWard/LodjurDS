#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <maxmod9.h>

#define sample_rate 31250
#define buffer_size (256+10)

void soundInit(void);
void setMuteSoundGUI(void);
void setSoundChipEnable(bool enable);
mm_word VblSound2(mm_word length, mm_addr dest, mm_stream_formats format);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOUND_HEADER
