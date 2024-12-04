//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//	                  Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Susie object header file                                                 //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the Suzy class    //
// which provides math and sprite support to the emulator                   //
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

#ifndef SUSIE_H
#define SUSIE_H

class CSystem;

#define LYNX_SCREEN_WIDTH	160
#define LYNX_SCREEN_HEIGHT	102

enum {sprite_background_shadow=0,
	  sprite_background_noncollide,
	  sprite_boundary_shadow,
	  sprite_boundary,
	  sprite_normal,
	  sprite_noncollide,
	  sprite_xor_shadow,
	  sprite_shadow};

class CSusie : public CLynxBase
{
	public:
		CSusie(CSystem& parent);
		~CSusie();

		void	Reset(void);

		UBYTE	Peek(ULONG addr);
		void	Poke(ULONG addr,UBYTE data);
//		ULONG	ReadCycle(void) {return 9;};
//		ULONG	WriteCycle(void) {return 5;};

		ULONG	PaintSprites(void);

	private:
		CSystem&	mSystem;
};

#endif
