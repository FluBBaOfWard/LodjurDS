#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 gContrastValue;
extern u8 gBorderEnable;
extern u8 gScreenMode;
extern u8 gRotation;

void setupGUI(void);
void enterGUI(void);
void exitGUI(void);
void quickSelectGame(void);
void nullUINormal(int key);
void nullUIDebug(int key);
void ejectGame(void);
void resetGame(void);

void uiNullNormal(void);
void uiAbout(void);

void debugIOUnmappedR(u16 port);
void debugIOUnmappedW(u16 port, u8 val);
void debugIOUnimplR(u16 port);
void debugIOUnimplW(u16 port, u8 val);
void debugIOMirroredR(u16 port);
void debugIOMirroredW(u16 port, u8 val);
void debugUndefinedInstruction(void);
void debugPowerOff(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUI_HEADER
