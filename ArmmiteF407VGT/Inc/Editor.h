/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Editor.h

Include file that contains the globals and defines for the full screen editor in MMBasic.

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


/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

void cmd_edit(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "Edit",		T_CMD,				0, cmd_edit		},
        
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

#endif




#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

void EditInputLine(void);

// General definitions used by other modules
extern char *StartEditPoint;
extern int StartEditChar;
extern char *EdBuff;						// the buffer used for editing the text
extern int EdBuffSize;						// size of the buffer in characters
extern int markmode;
// the values returned by the standard control keys
#define TAB     	0x9
#define BKSP    	0x8
#define ENTER   	0xd
#define ESC     	0x1b

// the values returned by the function keys
#define F1      	0x91
#define F2      	0x92
#define F3      	0x93
#define F4      	0x94
#define F5      	0x95
#define F6      	0x96
#define F7      	0x97
#define F8      	0x98
#define F9      	0x99
#define F10     	0x9a
#define F11     	0x9b
#define F12     	0x9c

// the values returned by special control keys
#define UP		0x80
#define DOWN		0x81
#define LEFT		0x82
#define RIGHT		0x83
#define INSERT		0x84
#define DEL		0x7f
#define HOME		0x86
#define END		0x87
#define PUP		0x88
#define PDOWN		0x89
#define NUM_ENT		ENTER
#define SLOCK		0x8c
#define ALT		0x8b
#define CTRLKEY(a) (a & 0x1f)


// definitions related to setting the tab spacing
#define CONFIG_TAB2		0b111
#define CONFIG_TAB4		0b001
#define CONFIG_TAB8		0b010
//extern const unsigned int TabOption;


#endif
