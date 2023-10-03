/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

FileIO.h

Supporting header file for FileIO.c which does all the SD Card related file I/O in MMBasic.


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


//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"



/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

void cmd_save(void);
void cmd_load(void);
void cmd_mkdir(void);
void cmd_rmdir(void);
void cmd_chdir(void);
void cmd_kill(void);
void cmd_seek(void);
void cmd_files(void);
void cmd_name(void);
void fun_cwd(void);
void fun_dir(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "Save",		T_CMD,				0, cmd_save	    },
	{ "Load",		T_CMD,				0, cmd_load	    },
	{ "Mkdir",		T_CMD,				0, cmd_mkdir	},
	{ "Rmdir",		T_CMD,				0, cmd_rmdir	},
	{ "Chdir",		T_CMD,				0, cmd_chdir	},
	{ "Kill",		T_CMD,				0, cmd_kill	    },
	{ "Seek",		T_CMD,				0, cmd_seek     },
	//{ "Files",		T_CMD,				0, cmd_files    },
	{ "Name",		T_CMD,				0, cmd_name     },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "Cwd$",		T_FNA | T_STR,		0, fun_cwd		},
	{ "Dir$(",		T_FUN | T_STR,		0, fun_dir		},

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    extern void GetSdFileDescriptor(int fnbr);
    extern void FileOpen(char *fname, char *fmode, char *ffnbr);
    extern int BasicFileOpen(char *fname, int fnbr, int mode);
    extern void FileClose(int fnbr);
    extern void ForceFileClose(int fnbr);
    extern char FileGetChar(int fnbr);
    extern char FilePutChar(char c, int fnbr);
    extern void FilePutStr(int count, char *c, int fnbr);
    extern int  FileEOF(int fnbr);
    extern char *ChangeToDir(char *p);
    extern int InitSDCard(void);
    extern void ErrorCheck(int fnbr);
    extern void ErrorThrow(int e);
    extern void CheckSDCard(void);
    extern int FileLoadProgram(char *fname);
    extern int FindFreeFileNbr(void);

    extern const int ErrorMap[21];
    extern int SDCardPresent;
    extern volatile int tickspersample;
    extern void checkWAVinput(void);
    extern char *WAVInterrupt;
    extern int WAVcomplete;
    extern int WAV_fnbr;
    extern FRESULT FSerror;
    extern int ExistsFile(char *p);
    extern int ExistsDir(char *p);
    #define WAV_BUFFER_SIZE 8192
    extern int OptionFileErrorAbort;

#endif
