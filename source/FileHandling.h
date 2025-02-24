#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"
#include "Lynx.h"

#define FILEEXTENSIONS ".lyx.lnx.bin.o"

extern ConfigData cfg;

int initSettings(void);
int loadSettings(void);
void saveSettings(void);
bool loadGame(const char *gameName);
void checkMachine(void);
void loadNVRAM(void);
void saveNVRAM(void);
void loadState(void);
void saveState(void);
void ejectCart(void);
void selectGame(void);
void selectBios(void);
int loadBios(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
