//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// 65C02 Macro definitions                                                  //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This file contains all of the required address mode and operand          //
// macro definitions for the 65C02 emulation                                //
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

//
// Addressing mode decoding
//

#define	xIMMEDIATE()			{mOperand=mPC;mPC++;}
#define	xABSOLUTE()				{mOperand=CPU_PEEKW(mPC);mPC+=2;}
#define xZEROPAGE()				{mOperand=CPU_PEEK(mPC);mPC++;}
#define xZEROPAGE_X()			{mOperand=CPU_PEEK(mPC)+mX;mPC++;mOperand&=0xff;}
#define xZEROPAGE_Y()			{mOperand=CPU_PEEK(mPC)+mY;mPC++;mOperand&=0xff;}
#define xABSOLUTE_X()			{mOperand=CPU_PEEKW(mPC);mPC+=2;mOperand+=mX;}
#define	xABSOLUTE_Y()			{mOperand=CPU_PEEKW(mPC);mPC+=2;mOperand+=mY;}
#define xINDIRECT_ABSOLUTE_X()	{mOperand=CPU_PEEKW(mPC);mPC+=2;mOperand+=mX;mOperand=CPU_PEEKW(mOperand);}
#define xRELATIVE()				{mOperand=CPU_PEEK(mPC);mPC++;mOperand=(mPC+mOperand)&0xffff;}
#define xINDIRECT_X()			{mOperand=CPU_PEEK(mPC);mPC++;mOperand=mOperand+mX;mOperand=CPU_PEEKW(mOperand);}
#define xINDIRECT_Y()			{mOperand=CPU_PEEK(mPC);mPC++;mOperand=CPU_PEEKW(mOperand);mOperand=mOperand+mY;}
#define xINDIRECT_ABSOLUTE()	{mOperand=CPU_PEEKW(mPC);mPC+=2;mOperand=CPU_PEEKW(mOperand);}
#define xINDIRECT()				{mOperand=CPU_PEEK(mPC);mPC++;mOperand=CPU_PEEKW(mOperand);}

//
// Opcode execution 
//

#define	xADC()\
{\
	UBYTE	value=CPU_PEEK(mOperand);\
	UBYTE	oldA=mA;\
	if(!mD)\
	{\
		SWORD sum=(SWORD)((SBYTE)mA)+(SWORD)((SBYTE)value)+(mC?1:0);\
		mV=((sum > 127) || (sum < -128));\
		sum=(SWORD)mA + (SWORD)value + (mC?1:0);\
		mA=(UBYTE)sum;\
		mC=(sum>0xff);\
		mZ=!mA;\
		mN=mA & 0x80;\
	}\
	else\
	{\
		SWORD sum=mBCDTable[0][mA]+mBCDTable[0][value]+(mC?1:0);\
		mC=(sum > 99);\
		mA=mBCDTable[1][sum & 0xff];\
		mZ=!mA;\
		mN=mA&0x80;\
		mV=((oldA^mA)&0x80) && ((mA^value)&0x80);\
	}\
}

#define xAND()\
{\
	mA&=CPU_PEEK(mOperand);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define xASL()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	mC=value&0x80;\
	value<<=1;\
	mZ=!value;\
	mN=value&0x80;\
	CPU_POKE(mOperand,value);\
}

#define xASLA()\
{\
	mC=mA&0x80;\
	mA<<=1;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define xBCC()\
{\
	if(!mC)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBCS()\
{\
	if(mC)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBEQ()\
{\
	if(mZ)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBIT()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	mZ=!(mA&value);\
\
	if(mOpcode!=0x89)\
	{\
		mN=value&0x80;\
		mV=value&0x40;\
	}\
}

#define	xBMI()\
{\
	if(mN)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBNE()\
{\
	if(!mZ)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBPL()\
{\
	if(!mN)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBRA()\
{\
	SBYTE offset=CPU_PEEK(mPC);\
	mPC++;\
	mPC+=offset;\
}

#define	xBRK()\
{\
	mPC++;\
	CPU_POKE(0x0100+mSP,mPC>>8);\
	mSP--;\
	CPU_POKE(0x0100+mSP,mPC&0x00ff);\
	mSP--;\
	CPU_POKE(0x0100+mSP,PS());\
	mSP--;\
\
	mB=TRUE;\
	mD=FALSE;\
	mI=TRUE;\
\
	mPC=CPU_PEEKW(IRQ_VECTOR);\
}

#define	xBVC()\
{\
	if(!mV)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xBVS()\
{\
	if(mV)\
	{\
		SBYTE offset=CPU_PEEK(mPC);\
		mPC++;\
		mPC+=offset;\
	}\
	else\
	{\
		mPC++;\
	}\
}

#define	xCLC()\
{\
	mC=FALSE;\
}

#define	xCLD()\
{\
	mD=FALSE;\
}

#define	xCLI()\
{\
	mI=FALSE;\
}

#define	xCLV()\
{\
	mV=FALSE;\
}

//
// Alternate CMP code
//
//#define	xCMP()\
//{\
//	UBYTE value=CPU_PEEK(mOperand);\
//	if(mA+0x100-value>0xff) mC=TRUE; else mC=FALSE;\
//	value=mA+0x100-value;\
//	mZ=!value;\
//	mN=value&0x0080;\
//}
//
//#define	xCPX()\
//{\
//	UBYTE value=CPU_PEEK(mOperand);\
//	if(mX+0x100-value>0xff) mC=TRUE; else mC=FALSE;\
//	value=mX+0x100-value;\
//	mZ=!value;\
//	mN=value&0x0080;\
//}
//
//#define	xCPY()\
//{\
//	UBYTE value=CPU_PEEK(mOperand);\
//	if(mY+0x100-value>0xff) mC=TRUE; else mC=FALSE;\
//	value=mY+0x100-value;\
//	mZ=!value;\
//	mN=value&0x0080;\
//}

#define	xCMP()\
{\
	UWORD value=(UWORD)mA-CPU_PEEK(mOperand);\
	mZ=!value;\
	mN=value&0x0080;\
	mC=!(value&0x0100);\
}

#define	xCPX()\
{\
	UWORD value=(UWORD)mX-CPU_PEEK(mOperand);\
	mZ=!value;\
	mN=value&0x0080;\
	mC=!(value&0x0100);\
}

#define	xCPY()\
{\
	UWORD value=(UWORD)mY-CPU_PEEK(mOperand);\
	mZ=!value;\
	mN=value&0x0080;\
	mC=!(value&0x0100);\
}

#define	xDEC()\
{\
	UBYTE value=CPU_PEEK(mOperand)-1;\
	CPU_POKE(mOperand,value);\
	mZ=!value;\
	mN=value&0x80;\
}

#define	xDECA()\
{\
	mA--;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xDEX()\
{\
	mX--;\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xDEY()\
{\
	mY--;\
	mZ=!mY;\
	mN=mY&0x80;\
}

#define	xEOR()\
{\
	mA^=CPU_PEEK(mOperand);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xINC()\
{\
	UBYTE value=CPU_PEEK(mOperand)+1;\
	CPU_POKE(mOperand,value);\
	mZ=!value;\
	mN=value&0x80;\
}

#define	xINCA()\
{\
	mA++;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xINX()\
{\
	mX++;\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xINY()\
{\
	mY++;\
	mZ=!mY;\
	mN=mY&0x80;\
}

#define	xJMP()\
{\
	mPC=mOperand;\
}

#define	xJSR()\
{\
	CPU_POKE(0x0100+mSP,(mPC-1)>>8);\
	mSP--;\
	CPU_POKE(0x0100+mSP,(mPC-1)&0xff);\
	mSP--;\
	mPC=mOperand;\
}

#define	xLDA()\
{\
	mA=CPU_PEEK(mOperand);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xLDX()\
{\
	mX=CPU_PEEK(mOperand);\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xLDY()\
{\
	mY=CPU_PEEK(mOperand);\
	mZ=!mY;\
	mN=mY&0x80;\
}

#define	xLSR()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	mC=value&0x01;\
	value=(value>>1)&0x7f;\
	CPU_POKE(mOperand,value);\
	mZ=!value;\
	mN=value&0x80;\
}

#define	xLSRA()\
{\
	mC=mA&0x01;\
	mA=(mA>>1)&0x7f;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xNOP()\
{\
}

#define	xORA()\
{\
	mA|=CPU_PEEK(mOperand);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xPHA()\
{\
	CPU_POKE(0x0100+mSP,mA);\
	mSP--;\
}

#define	xPHP()\
{\
	CPU_POKE(0x0100+mSP,PS());\
	mSP--;\
}

#define	xPHX()\
{\
	CPU_POKE(0x0100+mSP,mX);\
	mSP--;\
}

#define	xPHY()\
{\
	CPU_POKE(0x0100+mSP,mY);\
	mSP--;\
}

#define	xPLA()\
{\
	mSP++;\
	mA=CPU_PEEK(mSP+0x0100);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xPLP()\
{\
	mSP++;\
	PS(CPU_PEEK(mSP+0x0100));\
}

#define	xPLX()\
{\
	mSP++;\
	mX=CPU_PEEK(mSP+0x0100);\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xPLY()\
{\
	mSP++;\
	mY=CPU_PEEK(mSP+0x0100);\
	mZ=!mY;\
	mN=mY&0x80;\
}

#define	xROL()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	BOOL oldC=mC;\
	mC=value&0x80;\
	value=(value<<1)|(oldC?1:0);\
	CPU_POKE(mOperand,value);\
	mZ=!value;\
	mN=value&0x80;\
}

#define	xROLA()\
{\
	BOOL oldC=mC;\
	mC=mA&0x80;\
	mA=(mA<<1)|(oldC?1:0);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xROR()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	BOOL oldC=mC;\
	mC=value&0x01;\
	value=((value>>1)&0x7f)|(oldC?0x80:0x00);\
	CPU_POKE(mOperand,value);\
	mZ=!value;\
	mN=value&0x80;\
}

#define	xRORA()\
{\
	BOOL oldC=mC;\
	mC=mA&0x01;\
	mA=((mA>>1)&0x7f)|(oldC?0x80:0x00);\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xRTI()\
{\
	mSP++;\
	PS(CPU_PEEK(mSP+0x0100));\
	mSP++;\
	mPC=CPU_PEEK(mSP+0x0100);\
	mSP++;\
	mPC|=CPU_PEEK(mSP+0x0100)<<8;\
	gSystemCPUSleep=gSystemCPUSleep_Saved;\
	mIRQActive--;\
}

#define	xRTS()\
{\
	mSP++;\
	mPC=CPU_PEEK(mSP+0x0100);\
	mSP++;\
	mPC|=CPU_PEEK(mSP+0x0100)<<8;\
	mPC++;\
}

#define	xSBC()\
{\
	UBYTE oldA=mA;\
	if(!mD)\
	{\
		UBYTE value=~(CPU_PEEK(mOperand));\
		SWORD difference=(SWORD)((SBYTE)mA)+(SWORD)((SBYTE)value)+(mC?1:0);\
		mV=((difference>127)||(difference<-128));\
		difference=((SWORD)mA)+((SWORD)value)+ (mC?1:0);\
		mA=(UBYTE)difference;\
		mC=(difference>0xff);\
		mZ=!mA;\
		mN=mA & 0x80;\
	}\
	else\
	{\
		UBYTE value=CPU_PEEK(mOperand);\
		SWORD difference=mBCDTable[0][mA]-mBCDTable[0][value]-(mC?0:1);\
		if(difference<0) difference+=100;\
		mA=mBCDTable[1][difference];\
		mZ=!mA;\
		mN=mA&0x80;\
		mC=(oldA>=(value+(mC?0:1)));\
		mV=((oldA^mA)&0x80)&&((mA^value)&0x80);\
	}\
}

#define	xSEC()\
{\
	mC=TRUE;\
}

#define	xSED()\
{\
	mD=TRUE;\
}

#define	xSEI()\
{\
	mI=TRUE;\
}

#define	xSTA()\
{\
	CPU_POKE(mOperand,mA);\
}

#define	xSTP()\
{\
	gSystemCPUSleep=TRUE;\
}

#define	xSTX()\
{\
	CPU_POKE(mOperand,mX);\
}

#define	xSTY()\
{\
	CPU_POKE(mOperand,mY);\
}

#define	xSTZ()\
{\
	CPU_POKE(mOperand,0);\
}

#define	xTAX()\
{\
	mX=mA;\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xTAY()\
{\
	mY=mA;\
	mZ=!mY;\
	mN=mY&0x80;\
}

#define	xTRB()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	mZ=!(mA&value);\
	value=value&(mA^0xff);\
	CPU_POKE(mOperand,value);\
}

#define	xTSB()\
{\
	UBYTE value=CPU_PEEK(mOperand);\
	mZ=!(mA&value);\
	value=value|mA;\
	CPU_POKE(mOperand,value);\
}

#define	xTSX()\
{\
	mX=mSP;\
	mZ=!mX;\
	mN=mX&0x80;\
}

#define	xTXA()\
{\
	mA=mX;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xTXS()\
{\
	mSP=mX;\
}

#define	xTYA()\
{\
	mA=mY;\
	mZ=!mA;\
	mN=mA&0x80;\
}

#define	xWAI()\
{\
	gSystemCPUSleep=TRUE;\
}

