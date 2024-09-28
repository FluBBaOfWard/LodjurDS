/* defines.h */

#ifndef __DEFINES_H__
#define __DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*#define	LITTLE_ENDIAN*/

#ifndef u8
typedef unsigned char u8;
#endif
#ifndef s8
typedef signed char s8;
#endif
#ifndef u16
typedef unsigned short u16;
#endif
#ifndef s16
typedef short s16;
#endif
#ifndef u32
//typedef unsigned int u32;
#endif
#ifndef s32
//typedef int s32;
#endif

typedef union {
#ifdef LITTLE_ENDIAN
	struct {
		u8 low;
		u8 high;
	} abyte;
#else
	struct {
		u8 high;
		u8 low;
	} abyte;
#endif
	u16 aword;
} union_word;


typedef union {
#ifdef LITTLE_ENDIAN
    struct {
		u16 low;
		u16 high;
	} aword;
#else
    struct {
        u16 high;
        u16 low;
	} aword;
#endif
    u32	adword;
} union_dword;


#ifndef BOOL
#define	BOOL	int
#endif
#ifndef TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

#ifndef	NULL
#define	NULL	0
#endif

#ifdef __cplusplus
	}
#endif

#endif
