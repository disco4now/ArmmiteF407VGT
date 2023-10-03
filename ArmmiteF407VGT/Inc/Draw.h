/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Draw.h

Supporting header file for Draw.c which does the basic LCD display commands and related I/O in MMBasic.

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

void cmd_gui(void);
void cmd_text(void);
void cmd_pixel(void);
void cmd_circle(void);
void cmd_line(void);
void cmd_box(void);
void cmd_rbox(void);
void cmd_polygon(void);
void cmd_triangle(void);
void cmd_blit(void);
void fun_pixel(void);
void fun_getscanline(void);
void cmd_refresh(void);

void cmd_cls(void);
void cmd_font(void);
void cmd_colour(void);
void cmd_bezier(void);
void cmd_arc(void);
void fun_rgb(void);
void fun_mmhres(void);
void fun_mmvres(void);
void fun_mmcharwidth(void);
void fun_mmcharheight(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "GUI",            T_CMD,                      0, cmd_gui	},
	{ "Triangle",       T_CMD,                      0, cmd_triangle	},
	{ "Blit",           T_CMD,                      0, cmd_blit	},
	{ "Text",           T_CMD,                      0, cmd_text	},
	{ "Pixel",          T_CMD,                      0, cmd_pixel	},
	{ "Circle",         T_CMD,                      0, cmd_circle	},
	{ "Line",           T_CMD,                      0, cmd_line	},
	{ "Box",            T_CMD,                      0, cmd_box	},
	{ "RBox",           T_CMD,                      0, cmd_rbox	},
	{ "CLS",            T_CMD,                      0, cmd_cls	},
	{ "Font",           T_CMD,                      0, cmd_font	},
	{ "Colour",         T_CMD,                      0, cmd_colour	},
//	{ "Color",          T_CMD,                      0, cmd_colour	},
	{ "Bezier",         T_CMD,                      0, cmd_bezier	},
	{ "Arc",            T_CMD,                      0, cmd_arc	},
	{ "Polygon",        T_CMD,                  	0, cmd_polygon	},
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "RGB(",           T_FUN | T_INT,		0, fun_rgb	        },
	{ "MM.HRes",	    T_FNA | T_INT,		0, fun_mmhres 	    },
	{ "MM.VRes",	    T_FNA | T_INT,		0, fun_mmvres 	    },
//	{ "MM.FontWidth",   T_FNA | T_INT,		0, fun_mmcharwidth 	},
//	{ "MM.FontHeight",  T_FNA | T_INT,		0, fun_mmcharheight },
 	{ "Pixel(",	        T_FUN | T_INT,		0, fun_pixel,	    },
	{ "GetScanLine(",	T_FUN | T_INT,		0, fun_getscanline,	},

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
  #ifndef DRAW_H_INCL
    #define DRAW_H_INCL

    extern void GUIPrintString(int x, int y, int font, int jh, int jv, int jo, int fc, int bc, char *str);
    extern void GUIPrintChar(int font, int fc, int bc, char c, int jo);
    extern int GetJustification(char *p, int *jh, int *jv, int *jo);
    extern void cmd_guiBasic(void);
    extern void DrawLine(int x1, int y1, int x2, int y2, int w, int c);
    extern void DrawBox(int x1, int y1, int x2, int y2, int w, int c, int fill);
    extern void DrawRBox(int x1, int y1, int x2, int y2, int radius, int c, int fill);
    extern void DrawCircle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect);
    extern void DrawPixel(int x, int y, int c);
    extern void ClearScreen(int c);
    extern void SetFont(int fnt);
    extern void ResetDisplay(void);
    extern int GetFontWidth(int fnt);
    extern int GetFontHeight(int fnt);
    extern int rgb(int r, int g, int b);
    extern void cmd_guiMX170(void);
    extern void (*DrawRectangle)(int x1, int y1, int x2, int y2, int c);
    extern void (*DrawBitmap)(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
    extern void (*ScrollLCD) (int lines);
    extern void (*DrawBuffer)(int x1, int y1, int x2, int y2, char *c);
    extern void (*ReadBuffer)(int x1, int y1, int x2, int y2, char *c);
    
    extern void DrawRectangleUser(int x1, int y1, int x2, int y2, int c);
    extern void DrawBitmapUser(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
    extern void getargaddress(char *p, long long int **ip, MMFLOAT **fp, int *n);
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill);
    #define RGB(red, green, blue) (unsigned int) (((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111))
    #define swap(a, b) {int t = a; a = b; b = t;}

    #define BLACK               RGB(0,    0,      0)
    #define BLUE                RGB(0,    0,      255)
    #define GREEN               RGB(0,    255,    0)
    #define CYAN                RGB(0,    255,    255)
    #define RED                 RGB(255,  0,      0)
    #define MAGENTA             RGB(255,  0,      255)
    #define YELLOW              RGB(255,  255,    0)
    #define BROWN               RGB(255,  128,    0)
    #define GRAY                RGB(128,  128,    128)
    #define LITEGRAY            RGB(210,  210,    210)
    #define WHITE               RGB(255,  255,    255)

    #define JUSTIFY_LEFT        0
    #define JUSTIFY_CENTER      1
    #define JUSTIFY_RIGHT       2

    #define JUSTIFY_TOP         0
    #define JUSTIFY_MIDDLE      1
    #define JUSTIFY_BOTTOM      2

    #define ORIENT_NORMAL       0
    #define ORIENT_VERT         1
    #define ORIENT_INVERTED     2
    #define ORIENT_CCW90DEG     3
    #define ORIENT_CW90DEG      4
    
    extern int gui_font;
    extern int gui_font_width, gui_font_height;

    extern int gui_fcolour;
    extern int gui_bcolour;
    
    extern int DisplayHRes, DisplayVRes;        // resolution of the display
    extern int HRes, VRes;                      // the programming charteristics of the display

    #define FONT_BUILTIN_NBR     7              // the number of built in fonts
    #define FONT_TABLE_SIZE     16              // the total size of the font table (builtin + loadable)
    extern unsigned char *FontTable[];
    extern char *blitbuffptr[MAXBLITBUF];

  #endif
#endif

    

