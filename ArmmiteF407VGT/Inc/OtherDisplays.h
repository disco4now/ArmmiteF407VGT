/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

OtherDisplay.h

Include file that contains the globals and defines for OtherDisplays.c in MMBasic.

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
void fun_sprite(void);
#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
#endif
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef OTHERDISPLAYS_HEADER
#define OTHERDISPLAYS_HEADER
extern void ConfigDisplayOther(char *p);
extern void InitDisplayOther(int fullinit);
extern void Display_Refresh(void);
extern int low_y, high_y, low_x, high_x;
extern void ReadBufferBuffFast(int x1, int y1, int x2, int y2, char* p);
extern void DrawBufferBuffFast(int x1, int y1, int x2, int y2, char* p);
extern void ReadBufferBuffFast8(int x1, int y1, int x2, int y2, char* p);
extern void DrawBufferBuffFast8(int x1, int y1, int x2, int y2, char* p);
extern unsigned short colcount[256];
extern void DrawBitmapBuff(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
extern void DrawBufferBuff(int x1, int y1, int x2, int y2, char* p);
extern void ReadBufferBuff(int x1, int y1, int x2, int y2, char* p);
extern void DrawRectangleBuff(int x1, int y1, int x2, int y2, int c);
extern unsigned char *screenbuff;
extern void ScrollBuff(int lines);
extern void ScrollBuff8(int lines);
enum {
  TRANSFER_WAIT,
  TRANSFER_COMPLETE,
  TRANSFER_ERROR
};

#endif
#endif
