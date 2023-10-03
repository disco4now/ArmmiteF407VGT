/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Commands.h

Include file that contains the globals and defines for commands.c in MMBasic.

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

void cmd_clear(void);
void MIPS16 cmd_continue(void);
void MIPS16 cmd_dim(void);
void cmd_do(void);
void cmd_else(void);
void cmd_end(void);
void cmd_endfun(void);
void cmd_endsub(void);
void cmd_erase(void);
void cmd_error(void);
void cmd_exit(void);
void cmd_exitfor(void);
void cmd_for(void);
void cmd_subfun(void);
void cmd_gosub(void);
void cmd_goto(void);
void cmd_if(void);
void cmd_inc(void);
void cmd_input(void);
void cmd_let(void);
void cmd_lineinput(void);
void MIPS16 cmd_list(void);
void cmd_loop(void);
void MIPS16 cmd_new(void);
void cmd_next(void);
void cmd_null(void);
void cmd_on(void);
void cmd_print(void);
void cmd_read(void);
void cmd_restore(void);
void cmd_return(void);
void MIPS16 cmd_run(void);
void cmd_troff(void);
void cmd_tron(void);
void cmd_trace(void);
void MIPS16 cmd_const(void);
void cmd_select(void);
void cmd_case(void);
void cmd_mid(void);
void cmd_call(void);
void cmd_execute(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
    { "Call",		T_CMD,				0, cmd_call	    },
    { "Clear",		T_CMD,				0, cmd_clear	},
	{ "Continue",   T_CMD,              0, cmd_continue	},
	{ "Data",		T_CMD,				0, cmd_null		},
	{ "Dim",		T_CMD,				0, cmd_dim		},
	{ "Do",			T_CMD,				0, cmd_do		},

	{ "ElseIf",		T_CMD,				0, cmd_else		},
//	{ "Else If",	T_CMD,				0, cmd_else		},
	{ "Case Else",	T_CMD,				0, cmd_case		},
	{ "Else",		T_CMD,				0, cmd_else		},

	{ "Select Case",T_CMD,				0, cmd_select	},
	{ "End Select",	T_CMD,				0, cmd_null		},
	{ "Case",		T_CMD,				0, cmd_case		},

	{ "End Function", T_CMD,            0, cmd_endfun	},
	{ "End Sub",    T_CMD,              0, cmd_return	},
	{ "EndIf",		T_CMD,				0, cmd_null		},
//	{ "End If",		T_CMD,				0, cmd_null		},
	{ "End",		T_CMD,				0, cmd_end		},

	{ "Exit For",   T_CMD,				0, cmd_exitfor	},
	{ "Exit Sub",   T_CMD,				0, cmd_return	},
	{ "Exit Function",  T_CMD,          0, cmd_endfun	},
//	{ "Exit Do",    T_CMD,				0, cmd_exit		},
	{ "Exit",		T_CMD,				0, cmd_exit		},

	{ "Erase",		T_CMD,				0, cmd_erase	},
	{ "Error",		T_CMD,				0, cmd_error	},
	{ "For",		T_CMD,				0, cmd_for		},
	{ "Function",   T_CMD,				0, cmd_subfun	},
	{ "GoSub",		T_CMD,				0, cmd_gosub	},
	{ "GoTo",		T_CMD,				0, cmd_goto		},
	{ "If",			T_CMD,				0, cmd_if		},
	{ "Inc",		T_CMD,				0, cmd_inc		},
	{ "Line Input", T_CMD,				0, cmd_lineinput},
	{ "Input",		T_CMD,				0, cmd_input	},
	{ "Let",		T_CMD,				0, cmd_let		},
	{ "List",		T_CMD,				0, cmd_list		},
	{ "Local",		T_CMD,				0, cmd_dim		},
	{ "Loop",		T_CMD,				0, cmd_loop		},
	//{ "New",		T_CMD,				0, cmd_new		},
	{ "Next",		T_CMD,				0, cmd_next		},
	{ "On",			T_CMD,				0, cmd_on		},
	{ "Print",		T_CMD,				0, cmd_print	},
	{ "Read",		T_CMD,				0, cmd_read		},
	{ "Rem",		T_CMD,				0, cmd_null,	},
	{ "Restore",    T_CMD,				0, cmd_restore	},
	{ "Return",		T_CMD,				0, cmd_return,	},
	{ "Run",		T_CMD,				0, cmd_run		},
    { "Static",		T_CMD,				0, cmd_dim		},
	{ "Sub",		T_CMD,				0, cmd_subfun   },
	{ "Trace",		T_CMD,				0, cmd_trace	},
	{ "While",		T_CMD,				0, cmd_do		},
	{ "Const",		T_CMD,				0, cmd_const	},
	{"Execute", 	T_CMD,				0, cmd_execute	},
	{ "MID$(",		T_CMD | T_FUN,		0, cmd_mid      },
	{ "/*",		    T_CMD,				0, cmd_subfun   },
	{ "*/",		    T_CMD,				0, cmd_null   },


#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "For",		T_NA,				0, op_invalid	},
	{ "Else",		T_NA,				0, op_invalid	},
	{ "GoSub",		T_NA,				0, op_invalid	},
	{ "GoTo",		T_NA,				0, op_invalid	},
	{ "Step",		T_NA,				0, op_invalid	},
	{ "Then",		T_NA,				0, op_invalid	},
	{ "To",			T_NA,				0, op_invalid	},
	{ "Until",		T_NA,				0, op_invalid	},
	{ "While",		T_NA,				0, op_invalid	},

#endif




#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

struct s_forstack {
    char *forptr;                           // pointer to the FOR command in program memory
    char *nextptr;                          // pointer to the NEXT command in program memory
    void *var;                              // value of the FOR variable
    char vartype;                           // type of the variable
    char level;                             // the sub/function level that the loop was created
    union u_totype {
        MMFLOAT f;                          // the TO value if it is a MMFLOAT
        long long int i;                    // the TO value if it is an integer
    }  __attribute__ ((aligned (8))) tovalue;
    union u_steptype {
        MMFLOAT f;                          // the STEP value if it is a MMFLOAT
        long long int i;                    // the STEP value if it is an integer
    }  __attribute__ ((aligned (8))) stepvalue;
} __attribute__ ((aligned (8))) ;

extern struct s_forstack forstack[MAXFORLOOPS + 1] ;
extern int forindex;

struct s_dostack {
    char *evalptr;                          // pointer to the expression to be evaluated
    char *loopptr;                          // pointer to the loop statement
    char *doptr;                            // pointer to the DO statement
    char level;                             // the sub/function level that the loop was created
} __attribute__ ((aligned (8)));

extern struct s_dostack dostack[MAXDOLOOPS];
extern int doindex;

extern char *gosubstack[MAXGOSUB];
extern char *errorstack[MAXGOSUB];
extern int gosubindex;
extern char DimUsed;

//extern char *GetFileName(char* CmdLinePtr, char *LastFilePtr);
//extern void mergefile(char *fname, char *MemPtr);
extern void MIPS16 ListProgram(char *p, int all);
extern char MIPS16 *llist(char *b, char *p);
extern char *CheckIfTypeSpecified(char *p, int *type, int AllowDefaultType);

#define CONFIG_TITLE		0
#define CONFIG_LOWER		1
#define CONFIG_UPPER		2

extern unsigned int BusSpeed;
extern char *OnKeyGOSUB;
extern char EchoOption;

struct sa_data{
    char* SaveNextDataLine;
    int SaveNextData;
};
extern struct sa_data datastore[MAXRESTORE];
extern int restorepointer;

#endif
