//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// 65C02 Disassembly definition file                                        //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the opcode to string conversion for use in the //
// disassembler window used in the debug version of the emulator            //
//                                                                          //
// Keith Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define S_ILLEGAL	"???"
#define S_ADC		"ADC"
#define S_AND		"AND"
#define S_ASL		"ASL"
#define S_BBR0		"BBR0"
#define S_BBR1		"BBR2"
#define S_BBR2		"BBR2"
#define S_BBR3		"BBR3"
#define S_BBR4		"BBR4"
#define S_BBR5		"BBR5"
#define S_BBR6		"BBR6"
#define S_BBR7		"BBR7"
#define S_BBS0		"BBS0"
#define S_BBS1		"BBS1"
#define S_BBS2		"BBS2"
#define S_BBS3		"BBS3"
#define S_BBS4		"BBS4"
#define S_BBS5		"BBS5"
#define S_BBS6		"BBS6"
#define S_BBS7		"BBS7"
#define S_BCC		"BCC"
#define S_BCS		"BCS"
#define S_BEQ		"BEQ"
#define S_BIT		"BIT"
#define S_BMI		"BMI"
#define S_BNE		"BNE"
#define S_BPL		"BPL"
#define S_BRA		"BRA"
#define S_BRK		"BRK"
#define S_BVC		"BVC"
#define S_BVS		"BVS"
#define S_CLC		"CLC"
#define S_CLD		"CLD"
#define S_CLI		"CLI"
#define S_CLV		"CLV"
#define S_CMP		"CMP"
#define S_CPX		"CPX"
#define S_CPY		"CPY"
#define S_DEA		"DEA"
#define S_DEC		"DEC"
#define S_DEX		"DEX"
#define S_DEY		"DEY"
#define S_EOR		"EOR"
#define S_INA		"INA"
#define S_INC		"INC"
#define S_INX		"INX"
#define S_INY		"INY"
#define S_JMP		"JMP"
#define S_JSR		"JSR"
#define S_LDA		"LDA"
#define S_LDX		"LDX"
#define S_LDY		"LDY"
#define S_LSR		"LSR"
#define S_NOP		"NOP"
#define S_ORA		"ORA"
#define S_PHA		"PHA"
#define S_PHP		"PHP"
#define S_PLA		"PLA"
#define S_PLP		"PLP"
#define S_PHX		"PHX"
#define S_PHY		"PHY"
#define S_PLX		"PLX"
#define S_PLY		"PLY"
#define S_RMB0		"RMB0"
#define S_RMB1		"RMB1"
#define S_RMB2		"RMB2"
#define S_RMB3		"RMB3"
#define S_RMB4		"RMB4"
#define S_RMB5		"RMB5"
#define S_RMB6		"RMB6"
#define S_RMB7		"RMB7"
#define S_ROL		"ROL"
#define S_ROR		"ROR"
#define S_RTI		"RTI"
#define S_RTS		"RTS"
#define S_SBC		"SBC"
#define S_SEC		"SEC"
#define S_SED		"SED"
#define S_SEI		"SEI"
#define S_SMB0		"SMB0"
#define S_SMB1		"SMB1"
#define S_SMB2		"SMB2"
#define S_SMB3		"SMB3"
#define S_SMB4		"SMB4"
#define S_SMB5		"SMB5"
#define S_SMB6		"SMB6"
#define S_SMB7		"SMB7"
#define S_STA		"STA"
#define S_STX		"STX"
#define S_STY		"STY"
#define S_STZ		"STZ"
#define S_TAX		"TAX"
#define S_TAY		"TAY"
#define S_TRB		"TRB"
#define S_TSB		"TSB"
#define S_TSX		"TSX"
#define S_TXA		"TXA"
#define S_TXS		"TXS"
#define S_TYA		"TYA"



typedef struct 
{
	char opcode[5];
	int	 mode;
} DISASS6502;


static int mOperandSizes[]={ 1, 1, 2, 3, 2, 2, 2, 3, 3, 3, 1, 2, 3, 2, 2, 3, 2 };


static DISASS6502 mLookupTable[256]=
{
// 0x00
  {S_BRK, impl}, {S_ORA, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},	
  {S_TSB,   zp}, {S_ORA,   zp}, {S_ASL,   zp}, {S_RMB0,  zp},
  {S_PHP, impl}, {S_ORA,  imm}, {S_ASL, accu}, {S_ILLEGAL,0},
  {S_TSB, absl}, {S_ORA, absl}, {S_ASL, absl}, {S_BBR0,zrel},
// 0x10
  {S_BPL,  rel}, {S_ORA, indy}, {S_ORA,  ind}, {S_ILLEGAL,0},
  {S_TRB,   zp}, {S_ORA,  zpx}, {S_ASL,  zpx}, {S_RMB1,  zp},
  {S_CLC, impl}, {S_ORA, absy}, {S_INC, accu}, {S_ILLEGAL,0},
  {S_TRB, absl}, {S_ORA, absx}, {S_ASL, absx}, {S_BBR1,zrel},
// 0x20
  {S_JSR, absl}, {S_AND, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_BIT,   zp}, {S_AND,   zp}, {S_ROL,   zp}, {S_RMB2,  zp},
  {S_PLP, impl}, {S_AND,  imm}, {S_ROL, accu}, {S_ILLEGAL,0},
  {S_BIT, absl}, {S_AND, absl}, {S_ROL, absl}, {S_BBR2,zrel},
// 0x30
  {S_BMI,  rel}, {S_AND, indy}, {S_AND,  ind}, {S_ILLEGAL,0},
  {S_BIT,  zpx}, {S_AND,  zpx}, {S_ROL,  zpx}, {S_RMB3,  zp},
  {S_SEC, impl}, {S_AND, absy}, {S_DEC, accu}, {S_ILLEGAL,0},
  {S_BIT, absx}, {S_AND, absx}, {S_ROL, absx}, {S_BBR3,zrel},
// 0x40
  {S_RTI, impl}, {S_EOR, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_EOR,   zp}, {S_LSR,   zp}, {S_RMB4,  zp},
  {S_PHA, impl}, {S_EOR,  imm}, {S_LSR, accu}, {S_ILLEGAL,0},
  {S_JMP, absl}, {S_EOR, absl}, {S_LSR, absl}, {S_BBR4,zrel},
// 0x50
  {S_BVC,  rel}, {S_EOR, indy}, {S_EOR,  ind}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_EOR,  zpx}, {S_LSR,  zpx}, {S_RMB5,  zp},
  {S_CLI, impl}, {S_EOR, absy}, {S_PHY, impl}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_EOR, absx}, {S_LSR, absx}, {S_BBR5,zrel},
// 0x60
  {S_RTS, impl}, {S_ADC, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_STZ,   zp}, {S_ADC,   zp}, {S_ROR,   zp}, {S_RMB6,  zp},
  {S_PLA, impl}, {S_ADC,  imm}, {S_ROR, accu}, {S_ILLEGAL,0},
  {S_JMP, iabs}, {S_ADC, absl}, {S_ROR, absl}, {S_BBR6,zrel},
// 0x70
  {S_BVS,  rel}, {S_ADC, indy}, {S_ADC,  ind}, {S_ILLEGAL,0},
  {S_STZ,  zpx}, {S_ADC,  zpx}, {S_ROR,  zpx}, {S_RMB7,  zp},
  {S_SEI, impl}, {S_ADC, absy}, {S_PLY, impl}, {S_ILLEGAL,0},
  {S_JMP,iabsx}, {S_ADC, absx}, {S_ROR, absx}, {S_BBR7,zrel},
// 0x80
  {S_BRA,  rel}, {S_STA, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_STY,   zp}, {S_STA,   zp}, {S_STX,   zp}, {S_SMB0,  zp},
  {S_DEY, impl}, {S_BIT,  imm}, {S_TXA, impl}, {S_ILLEGAL,0},
  {S_STY, absl}, {S_STA, absl}, {S_STX, absl}, {S_BBS0,zrel},
// 0x90
  {S_BCC,  rel}, {S_STA, indy}, {S_STA,  ind}, {S_ILLEGAL,0},
  {S_STY,  zpx}, {S_STA,  zpx}, {S_STX,  zpy}, {S_SMB1,  zp},
  {S_TYA, impl}, {S_STA, absy}, {S_TXS, impl}, {S_ILLEGAL,0},
  {S_STZ, absl}, {S_STA, absx}, {S_STZ, absx}, {S_BBS1,zrel},
// 0xA0
  {S_LDY,  imm}, {S_LDA, indx}, {S_LDX,  imm}, {S_ILLEGAL,0},
  {S_LDY,   zp}, {S_LDA,   zp}, {S_LDX,   zp}, {S_SMB2,  zp},
  {S_TAY, impl}, {S_LDA,  imm}, {S_TAX, impl}, {S_ILLEGAL,0},
  {S_LDY, absl}, {S_LDA, absl}, {S_LDX, absl}, {S_BBS2,zrel},
// 0xB0
  {S_BCS,  rel}, {S_LDA, indy}, {S_LDA,  ind}, {S_ILLEGAL,0},
  {S_LDY,  zpx}, {S_LDA,  zpx}, {S_LDX,  zpy}, {S_SMB3,  zp},
  {S_CLV, impl}, {S_LDA, absy}, {S_TSX, impl}, {S_ILLEGAL,0},
  {S_LDY, absx}, {S_LDA, absx}, {S_LDX, absy}, {S_BBS3,zrel},
// 0xC0
  {S_CPY,  imm}, {S_CMP, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_CPY,   zp}, {S_CMP,   zp}, {S_DEC,   zp}, {S_SMB4,  zp},
  {S_INY, impl}, {S_CMP,  imm}, {S_DEX, impl}, {S_ILLEGAL,0},
  {S_CPY, absl}, {S_CMP, absl}, {S_DEC, absl}, {S_BBS4,zrel},
// 0xD0
  {S_BNE,  rel}, {S_CMP, indy}, {S_CMP,  ind}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_CMP,  zpx}, {S_DEC,  zpx}, {S_SMB5,  zp},
  {S_CLD, impl}, {S_CMP, absy}, {S_PHX, impl}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_CMP, absx}, {S_DEC, absx}, {S_BBS5,zrel},
// 0xE0
  {S_CPX,  imm}, {S_SBC, indx}, {S_ILLEGAL,0}, {S_ILLEGAL,0},
  {S_CPX,   zp}, {S_SBC,   zp}, {S_INC,   zp}, {S_SMB6,  zp},
  {S_INX, impl}, {S_SBC,  imm}, {S_NOP, impl}, {S_ILLEGAL,0},
  {S_CPX, absl}, {S_SBC, absl}, {S_INC, absl}, {S_BBS6,zrel},
// 0xF0
  {S_BEQ,  rel}, {S_SBC, indy}, {S_SBC,  ind}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_SBC,  zpx}, {S_INC,  zpx}, {S_SMB7,  zp},
  {S_SED, impl}, {S_SBC, absy}, {S_PLX, impl}, {S_ILLEGAL,0},
  {S_ILLEGAL,0}, {S_SBC, absx}, {S_INC, absx}, {S_BBS7,zrel},
};
