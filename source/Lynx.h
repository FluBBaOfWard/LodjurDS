#ifndef WONDERSWAN_HEADER
#define WONDERSWAN_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define HW_AUTO       (0)
#define HW_LYNX       (1)
#define HW_LYNX2      (2)
#define HW_SELECT_END (3)

/// This runs all save state functions for each chip.
int packState(void *statePtr);

/// This runs all load state functions for each chip.
void unpackState(const void *statePtr);

/// Gets the total state size in bytes.
int getStateSize(void);

/// Setup WonderSwan background for emulator screen.
void setupEmuBackground(void);

void setupEmuBorderPalette(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WONDERSWAN_HEADER
