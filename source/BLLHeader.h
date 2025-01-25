#ifndef BLLHEADER
#define BLLHEADER

/// BllHeader
typedef struct
{
	/// 0x00 = -0x80
	const s8 data0;
	/// 0x01 = 0x08
	const s8 data1;
	/// 0x02, 0x03, load/run address of code (excluding header).
	const u8 addressHigh;
	const u8 addressLow;
	/// 0x04, 0x05, complete file size in Bytes.
	const u8 fileSizeHigh;
	const u8 fileSizeLow;
	/// 0x06 - 0x09, "BS93" in Big endian
	const char magic[4];
} BllHeader;

#endif	// BLLHEADER
