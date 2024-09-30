#ifndef IO_HEADER
#define IO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 joyCfg;
extern u32 EMUinput;

/**
 * Saves the state of io to the destination.
 * @param  *destination: Where to save the state.
 * @return The size of the state.
 */
int ioSaveState(void *destination);

/**
 * Loads the state of io from the source.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int ioLoadState(const void *source);

/**
 * Gets the state size of an io state.
 * @return The size of the state.
 */
int ioGetStateSize(void);

/**
 * Convert device input keys to target keys.
 * @param input NDS/GBA keys
 * @return The converted input.
 */
int convertInput(int input);

/**
 * get the joystick value
 */
int joy0_R(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// IO_HEADER
