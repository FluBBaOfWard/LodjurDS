#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define ALLOW_SPEED_HACKS	(1<<17)
#define ENABLE_HEADPHONES	(1<<18)
#define ALLOW_REFRESH_CHG	(1<<19)

extern u8 gContrastValue;
extern u8 gBorderEnable;

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
void debugUndefinedInstruction(void);
void debugCrashInstruction(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUI_HEADER
