/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Touch.h

Supporting header file for Touch.c which does all the touch screen related I/O in MMBasic.

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

void fun_touch(void);

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

	{ "Touch(",	    T_FUN | T_INT,		0, fun_touch 	},

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

    #define CAL_ERROR_MARGIN        16
    #define TARGET_OFFSET           30
    #define TOUCH_SAMPLES           10
    #define TOUCH_DISCARD           2

    #define GET_X_AXIS              0
    #define GET_Y_AXIS              1
    #define PENIRQ_ON               3

    #define TOUCH_ERROR             -1

    #define CMD_MEASURE_X           0b10010000
    #define CMD_MEASURE_Y           0b11010000
    #define CMD_PENIRQ_ON           0b10010000

    extern void MIPS16 ConfigTouch(char *p);
    extern void MIPS16 InitTouch(void);
    extern void MIPS16 GetCalibration(int x, int y, int *xval, int *yval);

//    extern volatile int TouchX, TouchY;
    extern volatile int TouchState, TouchDown, TouchUp;
    extern int TOUCH_GETIRQTRIS;
    
    #define TOUCH_NOT_CALIBRATED    -9999
    #define TOUCH_ERROR             -1

    // these are defined so that the state of the touch PEN IRQ can be determined with the minimum of CPU cycles
    #define TOUCH_DOWN  (!(PinRead(Option.TOUCH_IRQ)))

    extern int GetTouchValue(int cmd);
    
    extern int (*GetTouchAxis)(int a);
    extern int GetTouchAxis2046(int);  // this needs looking at - the old mx470 code did not take any args
    extern int GetTouch(int cmd);
        
#endif
