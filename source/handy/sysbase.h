//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Systembase object class definition                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the systembase    //
// class that is required to get around cross dependencies between          //
// cpu/mikie/system classes                                                 //
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

#ifndef SYSBASE_H
#define SYSBASE_H


class CSystemBase
{
	// Function members

	public:
		virtual ~CSystemBase() {};

	public:
		virtual void	Reset(void) = 0;

};

#endif
