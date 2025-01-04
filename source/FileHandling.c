#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "FileHandling.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/AsmExtra.h"
#include "Main.h"
#include "Gui.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "Memory.h"

static void checkLnxHeader(void);

static const char *const folderName = "lodjurds";
static const char *const settingName = "settings.cfg";

ConfigData cfg;
LnxHeader header;

//---------------------------------------------------------------------------------
int initSettings() {
	cfg.palette = 0;
	cfg.gammaValue = 0;
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | AUTOSLEEP_OFF;
	cfg.sleepTime = 60*60*5;
	cfg.controller = 0;					// Don't swap A/B

	return 0;
}

int loadSettings() {
	FILE *file;

	if (findFolder(folderName)) {
		return 1;
	}
	if ( (file = fopen(settingName, "r")) ) {
		fread(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (!strstr(cfg.magic,"cfg")) {
			infoOutput("Error in settings file.");
			return 1;
		}
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
		return 1;
	}

	gGammaValue    = cfg.gammaValue;
	gContrastValue = cfg.contrastValue;
	emuSettings    = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime      = cfg.sleepTime;
	joyCfg         = (joyCfg & ~0x400)|((cfg.controller & 1)<<10);
	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));
	setSoundChipEnable(emuSettings & SOUND_ENABLE);

	infoOutput("Settings loaded.");
	return 0;
}

void saveSettings() {
	FILE *file;

	strcpy(cfg.magic,"cfg");
	cfg.gammaValue    = gGammaValue;
	cfg.contrastValue = gContrastValue;
	cfg.emuSettings   = emuSettings & ~EMUSPEED_MASK;		// Clear speed setting.
	cfg.sleepTime     = sleepTime;
	cfg.controller    = (joyCfg>>10)&1;
	strlcpy(cfg.currentPath, currentDir, sizeof(cfg.currentPath));

	if (findFolder(folderName)) {
		return;
	}
	if ( (file = fopen(settingName, "w")) ) {
		fwrite(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		infoOutput("Settings saved.");
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
}

void loadNVRAM() {
	FILE *svsFile;
	char nvramName[FILENAME_MAX_LENGTH];
	int saveSize = 0;
	void *nvMem = NULL;

	if (0 > 0) {
		nvMem = lynxRAM;
		setFileExtension(nvramName, currentFilename, ".ram", sizeof(nvramName));
	}
	else {
		return;
	}
	if (findFolder(folderName)) {
		return;
	}
	if ( (svsFile = fopen(nvramName, "r")) ) {
		if (fread(nvMem, 1, saveSize, svsFile) != saveSize) {
			infoOutput("Bad NVRAM file:");
			infoOutput(nvramName);
		}
		fclose(svsFile);
		infoOutput("Loaded NVRAM.");
	}
	else {
		infoOutput("Couldn't open NVRAM file:");
		infoOutput(nvramName);
	}
}

void saveNVRAM() {
	FILE *svsFile;
	char nvramName[FILENAME_MAX_LENGTH];
	int saveSize = 0;
	void *nvMem = NULL;

	if (0 > 0) {
		nvMem = lynxRAM;
		setFileExtension(nvramName, currentFilename, ".ram", sizeof(nvramName));
	}
	else {
		return;
	}
	if (findFolder(folderName)) {
		return;
	}
	if ( (svsFile = fopen(nvramName, "w")) ) {
		if (fwrite(nvMem, 1, saveSize, svsFile) != saveSize) {
			infoOutput("Couldn't write correct number of bytes.");
		}
		fclose(svsFile);
		infoOutput("Saved NVRAM.");
	}
	else {
		infoOutput("Couldn't open NVRAM file:");
		infoOutput(nvramName);
	}
}

void loadState() {
	loadDeviceState(folderName);
}

void saveState() {
	saveDeviceState(folderName);
}

//---------------------------------------------------------------------------------
bool loadGame(const char *gameName) {
	if (gameName) {
		cls(0);
		drawText("     Please wait, loading.", 11, 0);
		gRomSize = loadROM(romSpacePtr, gameName, maxRomSize);
		if (gRomSize) {
			checkLnxHeader();
			checkMachine();
//			setEmuSpeed(0);
			loadCart();
			gameInserted = true;
			if (emuSettings & AUTOLOAD_NVRAM) {
//				loadNVRAM();
			}
			if (emuSettings & AUTOLOAD_STATE) {
				loadState();
			}
			powerIsOn = true;
			closeMenu();
			return false;
		}
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	ui10();
	const char *gameName = browseForFileType(FILEEXTENSIONS".zip");
	if (loadGame(gameName)) {
		backOutOfMenu();
	}
}

void checkLnxHeader() {
	LnxHeader *rHead = (LnxHeader *)romSpacePtr;
	int smRot = gScreenMode;
	if (gRomSize & sizeof(LnxHeader)
			&& rHead->magic[0] == 'L'
			&& rHead->magic[1] == 'Y'
			&& rHead->magic[2] == 'N'
			&& rHead->magic[3] == 'X'
			&& rHead->versionNumber == 1) {
		memcpy(&header, rHead, sizeof(LnxHeader));
		int headRot = header.rotation;
		if (headRot == 1 || headRot == 2) {
			smRot = headRot;
		}
	}
	else {
		memset(&header, 0, sizeof(LnxHeader));
	}
	gRotation = smRot;
	setScreenMode(smRot);
}

void checkMachine() {
	if (gMachineSet == HW_AUTO) {
		gMachine = HW_LYNX_II;
	}
	else {
		gMachine = gMachineSet;
	}
	setupEmuBackground();
}

//---------------------------------------------------------------------------------
void ejectCart() {
	gRomSize = 0x80000;
	memset(romSpacePtr, -1, gRomSize);
	gameInserted = false;
}

//---------------------------------------------------------------------------------
static int loadBIOS(void *dest, const char *fPath, const int maxSize) {
	char tempString[FILEPATH_MAX_LENGTH];
	char *sPtr;

	cls(0);
	strlcpy(tempString, fPath, sizeof(tempString));
	if ( (sPtr = strrchr(tempString, '/')) ) {
		sPtr[0] = 0;
		sPtr += 1;
		chdir("/");
		chdir(tempString);
		return loadROM(dest, sPtr, maxSize);
	}
	return 0;
}

int loadBios(void) {
	if (loadBIOS(biosSpace, cfg.biosPath, 0x200)) {
		return 1;
	}
	return 0;
}

static bool selectBIOS(char *dest, const char *fileTypes) {
	const char *biosName = browseForFileType(fileTypes);

	if (biosName) {
		strlcpy(dest, currentDir, FILEPATH_MAX_LENGTH);
		strlcat(dest, "/", FILEPATH_MAX_LENGTH);
		strlcat(dest, biosName, FILEPATH_MAX_LENGTH);
		return true;
	}
	return false;
}

void selectBios() {
	if (selectBIOS(cfg.biosPath, ".img.bin.zip")) {
		loadBios();
	}
	cls(0);
}
