/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

MM_Misc.h

Include file that contains the globals and defines for Misc.c in MMBasic.
These are miscelaneous commands and functions that do not easily sit anywhere else.


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
// format:
//      void cmd_???(void)
//      void fun_???(void)
//      void op_???(void)

void cmd_autosave(void);
void cmd_option(void);
void cmd_pause(void);
void cmd_timer(void);
void cmd_date(void);
void cmd_time(void);
void cmd_ireturn(void);
void cmd_poke(void);
void cmd_settick(void);
void cmd_watchdog(void);
void cmd_cpu(void);
void cmd_cfunction(void);
void cmd_longString(void);
void cmd_sort(void);
void cmd_csubinterrupt(void);
void fun_timer(void);
void fun_date(void);
void fun_time(void);
void fun_device(void);
void fun_keydown(void);
void fun_peek(void);
void fun_restart(void);
void fun_day(void);
void fun_info(void);
void fun_LLen(void);
void fun_LGetByte(void);
void fun_LGetStr(void);
void fun_LCompare(void);
void fun_LInstr(void);
void fun_epoch(void);
void fun_datetime(void);
void fun_json(void);
void cmd_JumpToBootloader(void);
void fun_format(void);
//void fun_backup(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	//{ "AutoSave",		T_CMD,				0, cmd_autosave	},
	{ "Option",			T_CMD,				0, cmd_option	},
	{ "Pause",			T_CMD,				0, cmd_pause	},
	{ "Timer",			T_CMD | T_FUN,      0, cmd_timer	},
	{ "Date$",			T_CMD | T_FUN,      0, cmd_date		},
	{ "Time$",			T_CMD | T_FUN,      0, cmd_time		},
	{ "IReturn",		T_CMD,				0, cmd_ireturn 	},
	{ "Poke",			T_CMD,				0, cmd_poke		},
	{ "SetTick",		T_CMD,				0, cmd_settick 	},
	{ "WatchDog",		T_CMD,				0, cmd_watchdog	},
	{ "CPU",			T_CMD,				0, cmd_cpu 	},
	{ "Sort",			T_CMD,				0, cmd_sort 	},
    { "DefineFont",     T_CMD,				0, cmd_cfunction},
    { "End DefineFont", T_CMD,				0, cmd_null 	},
	{ "LongString",		T_CMD,				0, cmd_longString	},
	{ "Interrupt", 	    T_CMD,             	0, cmd_csubinterrupt},
	{ "CSub",           T_CMD,              0, cmd_cfunction},
	{ "End CSub",       T_CMD,              0, cmd_null     },
	{ "CFunction",      T_CMD,              0, cmd_cfunction},
	{ "End CFunction",  T_CMD,              0, cmd_null     },


#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "Timer",	T_FNA | T_NBR,		0, fun_timer	},
	{ "LInStr(",		T_FUN | T_INT,		0, fun_LInstr		},
	{ "LCompare(",		T_FUN | T_INT,		0, fun_LCompare		},
	{ "LLen(",		T_FUN | T_INT,		0, fun_LLen		},
	{ "LGetStr$(",		T_FUN | T_STR,		0, fun_LGetStr		},
	{ "As",		T_NA,			0, op_invalid	},
	{ "LGetByte(",		T_FUN | T_INT,		0, fun_LGetByte		},
	{ "Date$",	T_FNA | T_STR,		0, fun_date	},
	{ "Day$(",	T_FUN | T_STR,		0, fun_day	},
	{ "Peek(",		T_FUN  | T_INT | T_STR | T_NBR,			0, fun_peek		},
	{ "Time$",	T_FNA | T_STR,		0, fun_time	},
	{ "MM.Device$",	T_FNA | T_STR,		0, fun_device   },
	{ "MM.Watchdog",T_FNA | T_INT,		0, fun_restart	},
	{ "Epoch(",		T_FUN  | T_INT,			0, fun_epoch		},
	{ "DateTime$(",		T_FUN | T_STR,		0, fun_datetime		},
//	{ "MM.Info$(",		T_FUN | T_INT  | T_NBR| T_STR,		0, fun_info		},
	{ "MM.Info(",		T_FUN | T_INT  | T_NBR| T_STR,		0, fun_info		},
//	{ "MM.Backup",   T_FNA | T_INT,		0, fun_backup 	},
	{ "JSON$(",		T_FUN | T_STR,          0, fun_json		},
//	{ "Format$(",	T_FUN  | T_STR,			0, fun_format	},

#endif


#define IsDigitinline(a)	( a >= '0' && a <= '9' )

#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    // General definitions used by other modules

    #ifndef MISC_HEADER
    #define MISC_HEADER

   	extern void MIPS16 OtherOptions(void);

    extern char *InterruptReturn;
    extern int check_interrupt(void);
    extern char *GetIntAddress(char *p);
    extern void MIPS16 CrunchData(char **p, int c);

    // struct for the interrupt configuration
    #define T_LOHI   1
    #define T_HILO   2
    #define T_BOTH   3
    struct s_inttbl {
            int pin;                                   // the pin on which the interrupt is set
            int last;					// the last value of the pin (ie, hi or low)
            char *intp;					// pointer to the interrupt routine
            int lohi;                                  // trigger condition (T_LOHI, T_HILO, etc).
    };
    #define NBRINTERRUPTS	    10			// number of interrupts that can be set
    extern struct s_inttbl inttbl[NBRINTERRUPTS];
	extern MMFLOAT optionangle;

    extern int TickPeriod[NBRSETTICKS];
    extern volatile int TickTimer[NBRSETTICKS];
    extern char *TickInt[NBRSETTICKS];
    extern volatile unsigned char TickActive[NBRSETTICKS];

	extern unsigned int CurrentCpuSpeed;
	extern unsigned int PeripheralBusSpeed;
	extern char *OnKeyGOSUB;
	extern char EchoOption;
	extern unsigned int GetPeekAddr(char *p);
	extern unsigned int GetPokeAddr(char *p);
#endif
#endif
