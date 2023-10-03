/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Configuration.h

Include file that contains the configuration details for the Micromite using MMBasic.

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#ifndef CONFIG
#define CONFIG
#include <math.h>
// The main clock frequency for the chip at bootup, it can be changed by the CPU command
#define	CLOCKFREQ		(168000000L)			    // This is set in in Configuration Bits.h
#define CONSOLE_BAUDRATE    115200 		//@@@@HKW38400
#ifndef STM32F405xx
	#define MMFLOAT double
    #define powf pow
    #define log10f log10
    #define floorf floor
    #define fabsf fabs
    #define atanf atan
    #define cosf cos
    #define expf exp
    #define logf log
    #define sinf sin
    #define sqrtf sqrt
    #define tanf tan
	#define STR_SIG_DIGITS 9                            // number of significant digits to use when converting MMFLOAT to a string
#else
	#define MMFLOAT float
	#define STR_SIG_DIGITS 6                            // number of significant digits to use when converting MMFLOAT to a string
#endif
#define STR_FLOAT_PRECISION  998
#define STR_FLOAT_DIGITS 6                            // number of significant digits to use when converting MMFLOAT to a string
#define nop	__asm__ ("NOP")

#define forever 1
#define true	1
#define false	0

#define MIPS16 __attribute__ ((optimize("-Os")))
//#define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); PRet();}
#define RoundUptoInt(a)     (((a) + (32 - 1)) & (~(32 - 1)))// round up to the nearest whole integer


#define BOOL_ALREADY_DEFINED
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXSTRLEN           255                     // maximum length of a string
#define STRINGSIZE          256                     // must be 1 more than MAXSTRLEN.  3 of these buffers are staticaly created
#define MAXERRMSG           64                      // max error msg size (MM.ErrMsg$ is truncated to this)
#define MAXDIM              6                       // maximum nbr of dimensions to an array

#define MAXFORLOOPS         50                      // maximum nbr of nested for-next loops, each entry uses 17 bytes
#define MAXDOLOOPS          50                      // maximum nbr of nested do-loops, each entry uses 12 bytes
#define MAXGOSUB            50                      // maximum nbr of nested gosubs and defined subs/functs, each entry uses 8 bytes
#define MAX_MULTILINE_IF    10                      // maximum nbr of nested multiline IFs, each entry uses 8 bytes
#define MAXTEMPSTRINGS      256                    // maximum nbr of temporary strings allowed, each entry takes up 4 bytes
#define MAXSUBFUN           256                     // maximum nbr of defined subroutines or functions in a program. each entry takes up 4 bytes
#define NBRSETTICKS         4                       // the number of SETTICK interrupts available
#define MAXBLITBUF          64                      // the maximum number of BLIT buffers
#define MAXLAYER            10                      // maximum number of sprite layers
#define BREAK_KEY           3                       // the default value (CTRL-C) for the break key.  Reset at the command prompt.
#define MAXKEYLEN           24						// Maximum length of programmable function keys
#define FNV_prime           16777619
#define FNV_offset_basis    2166136261
#define use_hash
#define MAXVARS             512                     // 8 + MAXVARLEN + MAXDIM * 2  (ie, 56 bytes) - these do not incl array members
#define MAXVARHASH				MAXVARS/2
#define MAXSUBHASH          MAXSUBFUN
#define STACKLIMIT			0x1000d800				//get this from the map
// define the maximum number of arguments to PRINT, INPUT, WRITE, ON, DIM, ERASE, DATA and READ
// each entry uses zero bytes.  The number is limited by the length of a command line
#define MAX_ARG_COUNT       50
#define MAXCFUNCTION	20

#define MAXRESTORE     10
// size of the console terminal emulator's screen
#define SCREENWIDTH     80
#define SCREENHEIGHT    24                          // this is the default and it can be changed using the OPTION command
#define NO_KEYBOARD             0
#define CONFIG_US		1
#define CONFIG_FR		2
#define CONFIG_GR		3
#define CONFIG_IT		4
#define CONFIG_BE		5
#define CONFIG_UK		6
#define CONFIG_ES		7

#endif

