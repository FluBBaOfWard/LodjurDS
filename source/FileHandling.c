#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "FileHandling.h"
#include "LNXHeader.h"
#include "BLLHeader.h"
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

static bool checkLnxHeader(const LnxHeader *headerPtr);
static bool checkBllHeader(const BllHeader *headerPtr);

static const char *const folderName = "lodjurds";
static const char *const settingName = "settings.cfg";

ConfigData cfg;
LnxHeader lnxHeader;
BllHeader bllHeader;

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings    = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gBorderEnable  = (cfg.config & 1) ^ 1;
	gGammaValue    = cfg.gammaValue;
	gContrastValue = cfg.contrastValue;
	sleepTime      = cfg.sleepTime;
	gMachineSet    = cfg.machine;
	joyCfg         = (joyCfg & ~0x400)|((cfg.controller & 1)<<10);
	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));
	setSoundChipEnable(emuSettings & SOUND_ENABLE);
//	pauseEmulation = emuSettings & AUTOPAUSE_EMULATION;
}

void updateConfigData(void) {
	strcpy(cfg.magic, "cfg");
	cfg.emuSettings   = emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	cfg.config        = (gBorderEnable & 1) ^ 1;
	cfg.gammaValue    = gGammaValue;
	cfg.contrastValue = gContrastValue;
	cfg.sleepTime     = sleepTime;
	cfg.machine       = gMachineSet;
	cfg.controller    = (joyCfg>>10)&1;
	strlcpy(cfg.currentPath, currentDir, sizeof(cfg.currentPath));
}

void initSettings() {
	memset(&cfg, 0, sizeof(ConfigData));
	cfg.emuSettings   = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | AUTOSLEEP_OFF;
	cfg.contrastValue = 3;
	cfg.sleepTime     = 60*60*5;
	cfg.machine       = HW_AUTO;

	applyConfigData();
}

int loadSettings() {
	FILE *file;
	if (!findFolder(folderName)
		&& (file = fopen(settingName, "r"))) {
		int len = fread(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (strstr(cfg.magic, "cfg") && len == sizeof(ConfigData)) {
			applyConfigData();
			infoOutput("Settings loaded.");
			return 0;
		}
		updateConfigData();
		infoOutput("Error in settings file.");
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
	return 1;
}

int saveSettings() {
	updateConfigData();

	FILE *file;
	if (!findFolder(folderName)
		&& (file = fopen(settingName, "w"))) {
		int len = fwrite(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (len == sizeof(ConfigData)) {
			infoOutput("Settings saved.");
			return 0;
		}
		infoOutput("Couldn't save settings.");
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
	return 1;
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
	if ((svsFile = fopen(nvramName, "r"))) {
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
	if ((svsFile = fopen(nvramName, "w"))) {
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
			setPowerIsOn(false);
			if ((gHasHeader = checkLnxHeader((LnxHeader *)romSpacePtr))) {
				gRomSize = lnxHeader.bank0PageSize << 8;
			}
			else if (checkBllHeader((BllHeader *)romSpacePtr)) {
				int size = bllHeader.fileSizeLow | (bllHeader.fileSizeHigh << 8);
				memcpy(lynxRAM, romSpacePtr, size);
				memcpy(romSpacePtr, BLL_ENC, sizeof(BLL_ENC));
				memcpy(romSpacePtr + sizeof(BLL_ENC), lynxRAM, size);
				gRomSize = 0x40000;
			}
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
			setPowerIsOn(true);
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

bool checkLnxHeader(const LnxHeader *lHead) {
	bool isLNX = false;
	if (lHead->magic[0] == 'L'
			&& lHead->magic[1] == 'Y'
			&& lHead->magic[2] == 'N'
			&& lHead->magic[3] == 'X'
			&& lHead->versionNumber == 1) {
		isLNX = true;
		memcpy(&lnxHeader, lHead, sizeof(LnxHeader));
	}
	else {
		memset(&lnxHeader, 0, sizeof(LnxHeader));
	}
	int smRot = gScreenMode;
	int headRot = lnxHeader.rotation;
	if (headRot == 1 || headRot == 2) {
		smRot = headRot;
	}
	gRotation = smRot;
	setScreenMode(smRot);
	return isLNX;
}

bool checkBllHeader(const BllHeader *bHead) {
	int adr = bHead->addressLow | (bHead->addressHigh << 8);
	int size = bHead->fileSizeLow | (bHead->fileSizeHigh << 8);
	if (bHead->data0 == -0x80
			&& bHead->data1 == 0x08
			&& bHead->magic[0] == 'B'
			&& bHead->magic[1] == 'S'
			&& bHead->magic[2] == '9'
			&& bHead->magic[3] == '3'
			&& (adr + size) < 0x10000) {
		memcpy(&bllHeader, bHead, sizeof(BllHeader));
		return true;
	}
	memset(&bllHeader, 0, sizeof(BllHeader));
	return false;
}

void checkMachine() {
	u8 newMachine = gMachineSet;
	if (newMachine == HW_AUTO) {
		newMachine = HW_LYNX_II;
	}
	if (gMachine != newMachine) {
		gMachine = newMachine;
		cpuInit(gMachine);
		setupEmuBackground();
	}
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
	if ((sPtr = strrchr(tempString, '/'))) {
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
