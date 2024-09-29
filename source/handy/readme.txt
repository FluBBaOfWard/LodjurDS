Handy was written by Keith Wilkins
http://homepage.ntlworld.com/dystopia/

GP32 and Dreamcast ports by Christian Nowak
http://chn.roarvgm.com/

The ports run very slow on the GP32 as well as the Dreamcast, so be 
patient when testing them :) To compile for the GP32, use devkitadv 
and the addon which is available on my website. You need to change 
the file gp32.mk so that it links the emulator with -lstdc++, such as:

GPLIBS=-lgpsdk -lgpgraphic -lgpmem -lgpos -lgpstdlib -lgpstdio -lgpsound -lgpfont -lgpg_ex01 -lstdc++

Have a look or two at dcmain.cpp and gpmain.cpp as a starting point.

Have fun.

/chris
