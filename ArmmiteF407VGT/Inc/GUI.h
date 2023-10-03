/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

GUI.h

Supporting header file for Display.c which does all the LCD display commands and related I/O in MMBasic.


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

void cmd_backlight(void);
void cmd_ctrlval(void);
//void cmd_page(void);

void fun_msgbox(void);
void fun_ctrlval(void);
void fun_mmhpos(void);
void fun_mmvpos(void);


#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "BackLight",      T_CMD,                      0, cmd_backlight},
	{ "CtrlVal(",       T_CMD | T_FUN,              0, cmd_ctrlval   },
//	{ "Page",           T_CMD,                      0, cmd_page},

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "MsgBox(",        T_FUN | T_INT,              0, fun_msgbox	},
	{ "CtrlVal(",       T_FUN | T_NBR | T_STR,      0, fun_ctrlval	},
	{ "MM.HPos",	    T_FNA | T_INT,		0, fun_mmhpos 	},
	{ "MM.VPos",	    T_FNA | T_INT,		0, fun_mmvpos 	},

#endif

#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
  #ifndef GUI_H_INCL
    #define GUI_H_INCL
    extern void ConfigDisplaySSD(char *p);
    extern void InitDisplaySSD(int fullinit);
    extern void DrawRectangleSSD1963(int x1, int y1, int x2, int y2, int c);

    extern void ProcessTouch(void);

    extern void ResetGUI(void);
    extern void DrawKeyboard(int);
    extern void DrawFmtBox(int);

    // define the blink rate for the cursor
    #define CURSOR_OFF		350				    // cursor off time in mS
    #define CURSOR_ON		650				    // cursor on time in mS

    #define MAX_CAPTION_LINES   10                                  // maximum number of lines in a caption

    extern void HideAllControls(void);

    extern int gui_font, gui_font_width, gui_font_height;

    extern int gui_fcolour, gui_bcolour;
    extern int last_fcolour, last_bcolour;

    extern int gui_click_pin;                  // the sound pin
    extern int display_backlight;              // the brightness of the backlight (1 to 100)

    extern int CurrentX, CurrentY;             // and the current default position

    extern int gui_int_down;                   // true if the touch down has triggered an interrupt
    extern char *GuiIntDownVector;             // address of the interrupt routine or NULL if no interrupt
    extern int gui_int_up;                     // true if the release of the touch has triggered an interrupt
    extern char *GuiIntUpVector;               // address of the interrupt routine or NULL if no interrupt
    extern int DelayedDrawKeyboard;            // a flag to indicate that the pop-up keyboard should be drawn AFTER the pen down interrupt
    extern int DelayedDrawFmtBox;              // a flag to indicate that the pop-up formatted keyboard should be drawn AFTER the pen down interrupt

    extern int CurrentRef;                     // if the pen is down this is the control (or zero if not on a control)
    extern int LastRef;                        // this is the last control touched
    extern int LastX;                          // this is the x coord when the pen was lifted
    extern int LastY;                          // ditto for y

    extern MMFLOAT CtrlSavedVal;                 // a temporary place to save a control's value
    
    extern int CheckGuiFlag;                   // used by Timer.c to tell if it has to call CheckGuiTimeouts()
    extern void CheckGui(void);
    extern void CheckGuiTimeouts(void);
    extern volatile int CursorTimer;                                // used to time the flashing cursor
    extern volatile int ClickTimer;                                 // used to time the click when touch occurs
    extern volatile int TouchTimer;                                 // used to time the response to touch

    // the control table holds all the info on the GUI controls currently being managed
    struct s_ctrl {
        char page;                          // the display page
                                            // place any additional chars here as the compiler will have padded this to four bytes
        char ref, type, state;              // reference nbr, type (button, etc) and the state (disabled, etc)
        char font;                          // the font in use when the control was created (used when redrawing)
        short int x1, y1, x2, y2;           // the coordinates of the touch sensitive area
        int fc, bc;                         // foreground and background colours
        int fcc;                            // foreground colour for the caption (default colour when the control was created)
        float value;
        float min, max, inc;              // the spinbox minimum/maximum and the increment value. NOTE:  Radio buttons, gauge and LEDs also store data in these variables
        char *s;                            // the caption
        char *fmt;                          // pointer to the format string for FORMATBOX
    };

    extern struct s_ctrl *Ctrl;             // list of the controls
    extern void SetBacklight(int intensity);
    
  #endif
#endif
