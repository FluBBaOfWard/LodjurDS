#ifndef LNXHEADER
#define LNXHEADER

typedef enum {
	EEPROM_NONE = 0,
	EEPROM_93C46,
	EEPROM_93C56,
	EEPROM_93C66,
	EEPROM_93C76,
	EEPROM_93C86,
} EEMPROM_TYPE

/// LnxHeader
typedef struct
{
	/// 0x00 - 0x03, "LYNX" in Big endian
	const char magic[4];
	/// 0x04, 0x05, page size in something kiloBytes?
	const u16 bank0PageSize;
	/// 0x06, 0x07, page size in something kiloBytes?
	const u16 bank1PageSize;
	/// 0x08, 0x09, Version number of?
	const u16 versionNumber;
	/// 0x0A - 0x29, Name of game
	const char cartName[32];
	/// 0x2A - 0x39, Name of manufacturer
	const char manufacturer[16];
	// 0x3A, 1=left, 2=right
	const u8 rotation;
	/// 0x3B AUDIN Used (1=yes)
	const u8 audinEn;
	/// 0x3C EEPROM info
	union
	{
		struct
		{
			EEMPROM_TYPE eepromType:3;
			u8	unused:3;
			u8	sd_real:1;
			u8	eepWidth:1;
		} eepBits;
		u8	eepromInfo;
	};
	// 0x3D-0x3F So far unused
	const u8 unused[3];
} LnxHeader;

#endif	// LNXHEADER
