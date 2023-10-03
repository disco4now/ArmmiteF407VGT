/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

SerialFileIO.h

Include file that contains the functions for handling serial file I/O in MMBasic.

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

#include "ff.h"
/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

void cmd_open(void);
void cmd_close(void);

void fun_eof(void);
void fun_loc(void);
void fun_lof(void);
void fun_inputstr(void);

#endif



/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
// the format is:
//    TEXT      	TYPE                P  FUNCTION TO CALL
// where type is always T_CMD
// and P is the precedence (which is only used for operators and not commands)


	{ "Open",		T_CMD,				0, cmd_open		},
	{ "Close",		T_CMD,				0, cmd_close	},

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
// the format is:
//    TEXT      	TYPE                P  FUNCTION TO CALL
// where type is T_NA, T_FUN, T_FNA or T_OPER augmented by the types T_STR and/or T_NBR
// and P is the precedence (which is only used for operators)
	{ "Eof(",		T_FUN | T_INT,		0, fun_eof		},
	{ "Loc(",		T_FUN | T_INT,		0, fun_loc		},
	{ "Lof(",		T_FUN | T_INT,		0, fun_lof		},
//	{ "As",			T_NA,			0, op_invalid           },
	{ "Input$(",            T_FUN | T_STR,		0, fun_inputstr         },

#endif



#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef FILES_HEADER
#define FILES_HEADER

extern void MMgetline(int filenbr, char *p);
//extern void MMfputs(char *p, int filenbr);
//extern void MMfopen(char *fname, char *mode, int fnbr);
//extern void MMfclose(int fnbr);
extern int MMfgetc(int fnbr);
extern char MMfputc(char c, int fnbr);
extern void CloseAllFiles(void);
extern int MMfeof(int fnbr);

#define MAXCOMPORTS 4

#define MAXOPENFILES  10
union uFileTable {
    unsigned int com;
    FIL *fptr;
};

extern union uFileTable FileTable[MAXOPENFILES + 1];

#endif
#endif


