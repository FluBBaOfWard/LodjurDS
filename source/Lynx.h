#ifndef LYNX_HEADER
#define LYNX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int packState(void *statePtr);

/// This runs all load state functions for each chip.
void unpackState(const void *statePtr);

/// Gets the total state size in bytes.
int getStateSize(void);

/// Setup Lynx background for emulator screen.
void setupEmuBackground(void);

void setupEmuBorderPalette(void);

void setScreenMode(int mode);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LYNX_HEADER
