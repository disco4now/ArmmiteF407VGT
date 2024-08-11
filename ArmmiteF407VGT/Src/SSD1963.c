/*-*****************************************************************************

ArmmiteF4 MMBasic

SSD1963.c

Driver for the SSD1963 display controller chip
Based on code written by John Leung of TechToys Co. (www.TechToys.com.hk)


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
#include <stdarg.h>
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
int ScrollStart;


// parameters for the SSD1963 display panel (refer to the glass data sheet)
int SSD1963HorizPulseWidth, SSD1963HorizBackPorch, SSD1963HorizFrontPorch;
int SSD1963VertPulseWidth, SSD1963VertBackPorch, SSD1963VertFrontPorch;
int SSD1963PClock1, SSD1963PClock2, SSD1963PClock3;
int SSD1963Mode1, SSD1963Mode2;
int SSD1963PixelInterface, SSD1963PixelFormat;
int SSD1963rgb;
extern char LCDAttrib;
void ScrollSSD1963(int lines);
unsigned int RDpin,RDport;
//extern void WriteSSD1963CommandData(unsigned char command, int data, ...);
extern void SetBacklight(int intensity);
extern void setscroll4P(int t);
void PhysicalDrawRect_16(int x1, int y1, int x2, int y2, int c);
void InitIPS_4_16(void);
//#define dx(...) {char s[140];sprintf(s,  __VA_ARGS__); SerUSBPutS(s); SerUSBPutS("\r\n");}
void ScrollILI9341(int lines);
void InitILI9341(void);
extern void myMX_FSMC_Init(void);
typedef struct
{
  __IO uint16_t REG;
  __IO uint16_t RAM;
}LCD_CONTROLLER_TypeDef;

#define FMC_BANK1_BASE  ((uint32_t)(0x60000000 | 0x00000000))
#define FMC_RBANK1_BASE  ((uint32_t)(0x60000000 | 0x00080000))
#define FMC_BANK1       ((LCD_CONTROLLER_TypeDef *) FMC_BANK1_BASE)
#define FMC_RBANK1       ((LCD_CONTROLLER_TypeDef *) FMC_RBANK1_BASE)
extern SRAM_HandleTypeDef hsram1;
extern int LCD_BL_Period;
extern int gui_inverse;
extern void DoBlit(int x1, int y1, int x2, int y2, int w, int h);
extern TIM_HandleTypeDef htim1;


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Functions used by MMBasic to setup the display
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigDisplaySSD(char *p) {
	int DISPLAY_TYPE=0;
    getargs(&p, 9,",");
//    if((argc & 1) != 1 || argc < 3) error("Argument count");
	if(checkstring(argv[0], "SSD1963_4_16")) {                         // this is the 4" glass
		DISPLAY_TYPE = SSD1963_4_16;
    } else if(checkstring(argv[0], "SSD1963_5_16")) {                  // this is the 5" glass
    	DISPLAY_TYPE = SSD1963_5_16;
    } else if(checkstring(argv[0], "SSD1963_5A_16")) {                 // this is the 5" glass alternative version
    	DISPLAY_TYPE = SSD1963_5A_16;
    } else if(checkstring(argv[0], "SSD1963_5ER_16")) {                 // this is the 5" EastRising RGB is BGR
       	DISPLAY_TYPE = SSD1963_5ER_16;
    } else if(checkstring(argv[0], "SSD1963_7_16")) {                  // there appears to be two versions of the 7" glass in circulation, this is type 1
    	DISPLAY_TYPE = SSD1963_7_16;
    } else if(checkstring(argv[0], "SSD1963_7A_16")) {                 // this is type 2 of the 7" glass (high luminosity version)
       	DISPLAY_TYPE = SSD1963_7A_16;
    } else if(checkstring(argv[0], "SSD1963_7ER_16")) {                // this is the 7" EastRising RGB is BGR
       	DISPLAY_TYPE = SSD1963_7ER_16;
    } else if(checkstring(argv[0], "SSD1963_8_16")) {                  // this is the 8" and 9" glass (EastRising)
    	DISPLAY_TYPE = SSD1963_8_16;

    } else if(checkstring(argv[0], "ILI9341_16")) {
    	DISPLAY_TYPE = ILI9341_16;
    } else if(checkstring(argv[0], "ILI9486_16")) {
    	DISPLAY_TYPE = ILI9486_16;
    } else if(checkstring(argv[0], "IPS_4_16")) {
    	DISPLAY_TYPE = IPS_4_16;	                      /***G.A***/
    } else
        return;
    if(!(argc == 3 || argc == 5 || argc==7)) error("Argument count");
    
    if(checkstring(argv[2], "L") || checkstring(argv[2], "LANDSCAPE"))
        Option.DISPLAY_ORIENTATION = LANDSCAPE;
    else if(checkstring(argv[2], "P") || checkstring(argv[2], "PORTRAIT"))
        Option.DISPLAY_ORIENTATION = PORTRAIT;
    else if(checkstring(argv[2], "RL") || checkstring(argv[2], "RLANDSCAPE"))
        Option.DISPLAY_ORIENTATION = RLANDSCAPE;
    else if(checkstring(argv[2], "RP") || checkstring(argv[2], "RPORTRAIT"))
        Option.DISPLAY_ORIENTATION = RPORTRAIT;
    else
        error("Orientation");
		

    Option.DISPLAY_TYPE = DISPLAY_TYPE;

    // disable the SPI LCD and touch
    Option.TOUCH_CS = Option.TOUCH_IRQ = Option.LCD_CD = Option.LCD_Reset = Option.LCD_CS = Option.TOUCH_Click = 0;
    Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
    SaveOptions();
    InitDisplaySSD(1);
}



// initialise the display controller
// this is used in the initial boot sequence of the Micromite
void InitDisplaySSD(int fullinit) {
    int i;
	if(!((Option.DISPLAY_TYPE >= SSD_PANEL_START && Option.DISPLAY_TYPE <= SSD_PANEL_END) || (Option.DISPLAY_TYPE >= P16_PANEL_START &&  Option.DISPLAY_TYPE <= P16_PANEL_END )) ) return;
	// ensure LCDAttrib is cleared at start.
	SSD1963rgb=0b0;
	LCDAttrib=0;
	SaveOptions();

    switch(Option.DISPLAY_TYPE) {
        case SSD1963_4_16:
                        DisplayHRes = 480;                                  // this is the 4.3" glass
                        DisplayVRes = 272;
                        SSD1963HorizPulseWidth = 41;
                        SSD1963HorizBackPorch = 2;
                        SSD1963HorizFrontPorch = 2;
                        SSD1963VertPulseWidth = 10;
                        SSD1963VertBackPorch = 2;
                        SSD1963VertFrontPorch = 2;
                        //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                        //Typical DCLK is 9MHz.  9MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 78642 (0x13332)
                        SSD1963PClock1 = 0x01;
                        SSD1963PClock2 = 0x33;
                        SSD1963PClock3 = 0x32;
                        SSD1963Mode1 = 0x20;                                // 24-bit for 4.3" panel, data latch in rising edge for LSHIFT
                        SSD1963Mode2 = 0;                                   // Hsync+Vsync mode
                        break;
        case SSD1963_5ER_16:
        	           SSD1963rgb=0b1000;
        case SSD1963_5_16:
                        DisplayHRes = 800;                                  // this is the 5" glass
                        DisplayVRes = 480;
                        SSD1963HorizPulseWidth = 128;
                        SSD1963HorizBackPorch = 88;
                        SSD1963HorizFrontPorch = 40;
                        SSD1963VertPulseWidth = 2;
                        SSD1963VertBackPorch = 25;
                        SSD1963VertFrontPorch = 18;
                        //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                        //Typical DCLK is 33MHz.  30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
                        SSD1963PClock1 = 0x03;
                        SSD1963PClock2 = 0xff;
                        SSD1963PClock3 = 0xff;
                        SSD1963Mode1 = 0x24;                                // 24-bit for 5" panel, data latch in falling edge for LSHIFT
                        SSD1963Mode2 = 0;                                   // Hsync+Vsync mode
                        break;
        case SSD1963_5A_16:

                        DisplayHRes = 800;                                 // this is a 5" glass alternative version
                        DisplayVRes = 480;
                        SSD1963HorizPulseWidth = 128;
                        SSD1963HorizBackPorch = 88;
                        SSD1963HorizFrontPorch = 40;
                        SSD1963VertPulseWidth = 2;
                        SSD1963VertBackPorch = 25;
                        SSD1963VertFrontPorch = 18;
                        //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                        //Typical DCLK is 33MHz.  30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
                        SSD1963PClock1 = 0x04;
                        SSD1963PClock2 = 0x93;
                        SSD1963PClock3 = 0xe0;
                        SSD1963Mode1 = 0x24;                                // 24-bit for 5" panel, data latch in falling edge for LSHIFT
                        SSD1963Mode2 = 0;                                   // Hsync+Vsync mode
                        break;
        case SSD1963_7ER_16:
                	    SSD1963rgb=0b1000;
        case SSD1963_7_16:
                        DisplayHRes = 800;                                  // this is the 7" glass
                        DisplayVRes = 480;
                        SSD1963HorizPulseWidth = 1;
                        SSD1963HorizBackPorch = 210;
                        SSD1963HorizFrontPorch = 45;
                        SSD1963VertPulseWidth = 1;
                        SSD1963VertBackPorch = 34;
                        SSD1963VertFrontPorch = 10;
                        //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                        //Typical DCLK is 33.3MHz(datasheet), experiment shows 30MHz gives a stable result
                        //30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
                        //Time per line = (DISP_HOR_RESOLUTION+DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH+DISP_HOR_FRONT_PORCH)/30 us = 1056/30 = 35.2us
                        SSD1963PClock1 = 0x03;
                        SSD1963PClock2 = 0xff;
                        SSD1963PClock3 = 0xff;
                        SSD1963Mode1 = 0x10;                                // 18-bit for 7" panel
                        SSD1963Mode2 = 0x80;                                // TTL mode
                        break;
        case SSD1963_7A_16:
                        DisplayHRes = 800;                                 // this is a 7" glass alternative version (high brightness)
                        DisplayVRes = 480;
                        SSD1963HorizPulseWidth = 3;
                        SSD1963HorizBackPorch = 88;
                        SSD1963HorizFrontPorch = 37;
                        SSD1963VertPulseWidth = 3;
                        SSD1963VertBackPorch = 32;
                        SSD1963VertFrontPorch = 10;
                        SSD1963PClock1 = 0x03;
                        SSD1963PClock2 = 0xff;
                        SSD1963PClock3 = 0xff;
                        SSD1963Mode1 = 0x10;                                // 18-bit for 7" panel
                        SSD1963Mode2 = 0x80;                                // TTL mode
                        break;
        case SSD1963_8_16:
                        DisplayHRes = 800;                                  // this is the 8" glass (not documented because the 40 pin connector is non standard)
                        DisplayVRes = 480;
                        SSD1963HorizPulseWidth = 1;
                        SSD1963HorizBackPorch = 210;
                        SSD1963HorizFrontPorch = 45;
                        SSD1963VertPulseWidth = 1;
                        SSD1963VertBackPorch = 34;
                        SSD1963VertFrontPorch = 10;
                        //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                        //Typical DCLK is 33.3MHz(datasheet), experiment shows 30MHz gives a stable result
                        //30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
                        //Time per line = (DISP_HOR_RESOLUTION+DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH+DISP_HOR_FRONT_PORCH)/30 us = 1056/30 = 35.2us
                        SSD1963PClock1 = 0x03;
                        SSD1963PClock2 = 0xff;
                        SSD1963PClock3 = 0xff;
                        SSD1963Mode1 = 0x20;
                        SSD1963Mode2 = 0x00;
                        break;
        case ILI9341_16:
        	DisplayHRes=320;
			DisplayVRes=240;
			break;
        case ILI9486_16:
               	DisplayHRes=480;
       			DisplayVRes=320;
       			break;
        case IPS_4_16:                                              /***G.A***/
            DisplayHRes=800;
       		DisplayVRes=480;
       		break;
    }
   // if(Option.DISPLAY_TYPE > SSD_PANEL_8){
        SSD1963PixelInterface=3; //PIXEL data interface - 16-bit RGB565
        SSD1963PixelFormat=0b01010000; //PIXEL data interface RGB565
   // } else {
   //     SSD1963PixelInterface=0; //PIXEL data interface - 8-bit
   //     SSD1963PixelFormat=0b01110000;	//PIXEL data interface 24-bit
   // }
    if(fullinit){
         if (HAS_100PINS){
     	   for(i=38;i<47;i++)SetAndReserve(i, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   for(i=55;i<58;i++)SetAndReserve(i, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   for(i=60;i<63;i++)SetAndReserve(i, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   SetAndReserve(81, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   SetAndReserve(82, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   SetAndReserve(85, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   SetAndReserve(86, P_OUTPUT, 0, EXT_BOOT_RESERVED);
     	   SetAndReserve(88, P_OUTPUT, 0, EXT_BOOT_RESERVED);
        }else{

    	   SetAndReserve(58, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(59, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(60, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(63, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(64, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(65, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(66, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(67, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(68, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(77, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(78, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(79, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(82, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(85, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(86, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(114, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(115, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(118, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(119, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    	   SetAndReserve(123, P_OUTPUT, 0, EXT_BOOT_RESERVED);

        }
    	myMX_FSMC_Init();


        if(DISPLAY_LANDSCAPE) {
            VRes=DisplayVRes;
            HRes=DisplayHRes;
        } else {
            VRes=DisplayHRes;
            HRes=DisplayVRes;
        }

        // setup the pointers to the drawing primitives
        DrawRectangle = DrawRectangleSSD1963;
        ScrollLCD = ScrollSSD1963;
        DrawBuffer=DrawBufferSSD1963_16;
        DrawBitmap = DrawBitmapSSD1963_16;
        ReadBuffer=ReadBufferSSD1963_16;

        if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 || Option.DISPLAY_TYPE == IPS_4_16){
                       	 ScrollLCD = ScrollILI9341;
        }

        //SetFont((Option.DISPLAY_TYPE == ILI9341_16 ? 0x61: 1)); // font 7 scale 1 is 0x61 ie. (7-1)<<4|1
       //Always default the ILI9341_16 to use small font 7 unless it already configured as a console.
      // if( Option.DISPLAY_CONSOLE == 0 && Option.DISPLAY_TYPE == ILI9341_16){
    	//   SetFont( 0x61); // font 7 scale 1 is 0x61 ie. (7-1)<<4|1
       //}else{
    	//   SetFont(((Option.DefaultFont-1)<<4) | 1);
       //}
        if(Option.DISPLAY_CONSOLE)SetFont(((Option.DefaultFont-1)<<4) | 1); // font 7 scale 1 is 0x61 ie. (7-1)<<4|1
         else SetFont((Option.DISPLAY_TYPE == ILI9341_16 ? 0x61: 1)); // font 7 scale 1 is 0x61 ie. (7-1)<<4|1
        PromptFont = gui_font;
        PromptFC = gui_fcolour = Option.DefaultFC;
        PromptBC = gui_bcolour = Option.DefaultBC;
    }
    if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16  ) InitILI9341();
    else if(Option.DISPLAY_TYPE == IPS_4_16) InitIPS_4_16();
    else InitSSD1963();

    SetBacklight(Option.DefaultBrightness);
    ResetDisplay();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The SSD1963 driver
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

// Write a command byte to the SSD1963
void WriteSSD1963Command(int cmd) {
	  /* Write 16-bit Index, then write register */
	  FMC_BANK1->REG = cmd; __DSB();
}


// Write an 8 bit data word to the SSD1963
void WriteDataSSD1963(int data) {
	  /* Write 16-bit Reg */
	  FMC_RBANK1->RAM = data; __DSB();
}

void WriteSSD1963CommandData(unsigned char cmd, int data, ...){
	   int i;
	   va_list ap;
	   va_start(ap, data);
	   FMC_BANK1->REG = cmd; __DSB();
	   for(i = 0; i < data; i++){
	     FMC_RBANK1->RAM =va_arg(ap, int); __DSB();
       }
	   va_end(ap);
}



// Write sequential 16 bit command with the same 16 bit data word n times to the IPS_4_16
void WriteCmdDataIPS_4_16(int cmd,int n,int data) {
  while (n>0) {
	/* Write 16-bit Index, then write register */
	  FMC_BANK1->REG = cmd;
	  __DSB();

	/* Write 16-bit Reg */
	 FMC_RBANK1->RAM = data;
	  __DSB();

	  cmd++;
	  n--;
  }
}

// Write RGB colour over an 8 bit bus
void WriteColor(unsigned int c) {
	FMC_RBANK1->RAM = (c>>16) & 0xFF;__DSB();
	FMC_RBANK1->RAM = (c>>8) & 0xFF;__DSB();
	FMC_RBANK1->RAM = c & 0xFF;__DSB();
}
// Read RGB colour over an 8 bit bus
unsigned int ReadColor(void) {
	int c;
	c=(FMC_RBANK1->RAM & 0xFF)<<16;
	c|=(FMC_RBANK1->RAM & 0xFF)<<8;
	c|=(FMC_RBANK1->RAM & 0xFF);
	return c;
}
unsigned int ReadData(void) {
	return FMC_RBANK1->RAM;
}


/*********************************************************************
* defines start/end coordinates for memory access from host to SSD1963
* also maps the start and end points to suit the orientation
********************************************************************/
void  SetAreaILI9341(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    WriteSSD1963Command(ILI9341_COLADDRSET);
    WriteDataSSD1963(xstart >> 8);
    WriteDataSSD1963(xstart);
    WriteDataSSD1963(xend >> 8);
    WriteDataSSD1963(xend);
    WriteSSD1963Command(ILI9341_PAGEADDRSET);
    WriteDataSSD1963(ystart >> 8);
    WriteDataSSD1963(ystart);
    WriteDataSSD1963(yend >> 8);
    WriteDataSSD1963(yend);
    if(rw){
    	WriteSSD1963Command(ILI9341_MEMORYWRITE);
    } else {
    	WriteSSD1963Command(ILI9341_RAMRD);
    }
}
void  SetAreaIPS_4_16(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    WriteCmdDataIPS_4_16(0x2A00,1,xstart>>8);
    WriteCmdDataIPS_4_16(0x2A01,1,xstart & 0xFF);
    WriteCmdDataIPS_4_16(0x2A02,1,xend>>8);
    WriteCmdDataIPS_4_16(0x2A03,1,xend & 0xFF);
    WriteCmdDataIPS_4_16(0x2B00,1,ystart>>8);
    WriteCmdDataIPS_4_16(0x2B01,1,ystart & 0xFF);
    WriteCmdDataIPS_4_16(0x2B02,1,yend>>8);
    WriteCmdDataIPS_4_16(0x2B03,1,yend & 0xFF);

    if(rw){
    	WriteSSD1963Command(0x2C00);  //write to memory
    } else {
    	WriteSSD1963Command(0x2E00);  //read from memory
    }
}
void SetAreaSSD1963(int x1, int y1, int x2, int y2) {
    int start_x, start_y, end_x, end_y;

    switch(Option.DISPLAY_ORIENTATION) {
        case LANDSCAPE:
        case RLANDSCAPE: start_x = x1;
                         end_x = x2;
                         start_y = y1;
                         end_y = y2;
                         break;
        case PORTRAIT:
        case RPORTRAIT:  start_x = y1;
                         end_x = y2;
                         start_y = (DisplayVRes - 1) - x2;
                         end_y = (DisplayVRes - 1) - x1;
                         break;
        default: return;
    }

	WriteSSD1963Command(CMD_SET_COLUMN);
	WriteDataSSD1963(start_x>>8);
	WriteDataSSD1963(start_x);
	WriteDataSSD1963(end_x>>8);
	WriteDataSSD1963(end_x);
	WriteSSD1963Command(CMD_SET_PAGE);
	WriteDataSSD1963(start_y>>8);
	WriteDataSSD1963(start_y);
	WriteDataSSD1963(end_y>>8);
	WriteDataSSD1963(end_y);
}


/*********************************************************************
* Set a GPIO pin to state high(1) or low(0)
*
* PreCondition: Set the GPIO pin an output prior using this function
*
* Arguments: BYTE pin	- 	LCD_RESET
*							LCD_SPENA
*							LCD_SPCLK
*							LCD_SPDAT
*							
*			 BOOL state - 	0 for low
*							1 for high
*********************************************************************/
static void GPIO_WR(int pin, int state) {
	int _gpioStatus = 0;

	if(state==1)
		_gpioStatus = _gpioStatus|pin;
	else
		_gpioStatus = _gpioStatus&(~pin);

	WriteSSD1963Command(CMD_SET_GPIO_VAL);                                 // Set GPIO value
	WriteDataSSD1963(_gpioStatus);
}


/*********************************************************************
* SetBacklight(BYTE intensity)
* Some boards may use of PWM feature of ssd1963 to adjust the backlight 
* intensity and this function supports that.  However, most boards have
* a separate PWM input pin and that is also supported by using the variable
*  display_backlight intimer.c
*
* Input: 	intensity = 0 (off) to 100 (full on)
*
* Note: The base frequency of PWM set to around 300Hz with PLL set to 120MHz.
*		This parameter is hardware dependent
********************************************************************/
//void SetBacklightSSD1963(int intensity) {
//	WriteSSD1963CommandData(CMD_SET_PWM_CONF,6,0x0e,(intensity * 255)/100,0x01,0x00,0x00,0x00);
//	display_backlight = intensity/5;                                // this is used in timer.c
//}
void SetBacklightSSD1963(int intensity) {
	WriteSSD1963Command(CMD_SET_PWM_CONF);                                 // Set PWM configuration for backlight control
	WriteDataSSD1963(0x0E);                                                // PWMF[7:0] = 2, PWM base freq = PLL/(256*(1+5))/256 = 300Hz for a PLL freq = 120MHz
	WriteDataSSD1963((intensity * 255)/100);                               // Set duty cycle, from 0x00 (total pull-down) to 0xFF (99% pull-up , 255/256)
	WriteDataSSD1963(0x01);                                                // PWM enabled and controlled by host (mcu)
	WriteDataSSD1963(0x00);
	WriteDataSSD1963(0x00);
	WriteDataSSD1963(0x00);

    display_backlight = intensity/5;                                // this is used in timer.c

}

/*********************************************************************
* SetTearingCfg(BOOL state, BOOL mode)
* This function enable/disable tearing effect
*
* Input: 	BOOL state -	1 to enable
*							0 to disable
*			BOOL mode -		0:  the tearing effect output line consists
*								of V-blanking information only
*							1:	the tearing effect output line consists
*								of both V-blanking and H-blanking info.
********************************************************************/
void SetTearingCfg(int state, int mode)
{
	if(state == 1) {
		WriteSSD1963Command(CMD_SET_TEAR_ON);
		WriteDataSSD1963(mode&0x01);
	} else {
		WriteSSD1963Command(0x34);
	}

}


/***********************************************************************************************************************************
* Function:  void InitVideo()
* Resets LCD, Initialize IO ports, initialize SSD1963 for PCLK,	HSYNC, VSYNC etc
***********************************************************************************************************************************/
// initialization commands and arguments are organized in these tables
// storage-wise this is  more compact than the equivalent code.
// Companion function SendCommand16Block follows.

#define DELAY 0x80  //Bit7 of the count indicates a delay is also added.
#define REPEAT 0x40  //Bit6 of the count indicates same data is repeated instead of reading next byte.
// First byte is the number of commands
// Second byte is command type 0= B7-B0 as 8bit command  8=single byte shifted to B15-B8 as 16bit command 2= two bytes needed
static const uint8_t
ILI9486_16Init[] = {   // Initialization commands for ILI9486-16 screen
	    15,            // no of commands in list:
		0,             // 0= B7-B0 as 8bit command  1=single byte shifted to B15-B8 as 16bit command 2= two bytes needed

		0x01,DELAY,100,                           // uSec( 100000); // software reset 100ms pause
		0x28,0, // display off
        0xF1,6,0x36,0x04,0x00,0x3c,0x0f,0x8f,
        0xF2,9,0x18,0xa3,0x12,0x02,0xb2,0x12,0xff,0x10,0x00,
        0xf8,2,0x21,0x04,
        0xf9,2,0x00,0x08,
        0xb4,1,0x00,
        0xc1,1,0x47,
        0xC5,4,0x00,0xaf,0x80,0x00,
        0xe0,15,0x0f,0x1f,0x1c,0x0c,0x0f,0x08,0x48,0x98,0x37,0x0a,0x13,0x04,0x11,0x0d,0x00,

		0xe1,15,0x0f,0x32,0x2e,0x0b,0x0d,0x05,0x47,0x75,0x37,0x06,0x10,0x03,0x24,0x20,0x00,
        0x34,0,                                     //Tearing Effect Off
        0x3A,1,0x66,                                // Pixel Interface Format // 18 bit colour for SPI
        0x11,DELAY,150,                             //uSec( 150000);
        0x29,DELAY,255                              //uSec(500000);
};



static const uint8_t
ILI9341_16Init[] = {   // Initialization commands for ILI9486-16 screen
	    17,            // no of commands in list:
		0,             // 0= B7-B0 as 8bit command  8=single byte shifted to B15-B8 as 16bit command 2= two bytes needed

		0xCB,5,0x39,0x2c,0x00,0x34,0x02,
		0xCF,3,0x00,0xc1,0x30,
		0xE8,3,0x85,0x00,0x78,
		0xEA,2,0x00,0x00,
		0xED,4,0x64,0x03,0x12,0x81,
		0xF7,1,0x20,
		0xC0,1,0x23,            //Power control  //VRH[5:0]
		0xC1,1,0x10,            //Power control   //SAP[2:0];BT[3:0]
		0xC5,2,0x3e,0x28,       //VCM control //Contrast
		0xC7,1,0x86,            //VCM control2

		0x3A,1,0x66,            // 55= RGB565 66=RGB666
		0xB1,2,0x00,0x18,
		0xB6,3,0x08,0x82,0x27, // Display Function Control
		0xF2,1,0x00,           // 3Gamma Function Disable
		0x26,1,0x01,           //Gamma curve selected
		0xe0,15,0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00, //Set Gamma
		0xe1,15,0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xc1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f  //Set Gamma
};


static const uint8_t
NT35510_16Init[] = {   // Initialization commands for ILI9486-16 screen
	    36,            // no of commands in list:
		1,             // 0= B7-B0 as 8bit command  1=single byte shifted to B15-B8 as 16bit command 2= two bytes needed

		0xF0,5,0x55,0xAA,0x52,0x08,0x01,
		0xB6,REPEAT+3,0x34,//0x34,0x34,
		0xB0,REPEAT+3,0x0D,//0x0D,0x0D,
		0xB7,REPEAT+3,0x24,//0x24,0x24,
		0xB1,REPEAT+3,0x0D,//0x0D,0x0D,
		0xB8,REPEAT+3,0x24,//0x24,0x24,
		0xB2,1,0x00,
		0xB9,REPEAT+3,0x24,//0x24,0x24,
		0xB3,REPEAT+3,0x05,//0x05,0x05,
		0xBF,1,0x01,

		0xBA,REPEAT+3,0x34,//0x34,0x34,
		0xB5,REPEAT+3,0x0B,//0x0B,0x0B,
		0xBC,3,0x00,0xA3,0x00,
		0xBD,3,0x00,0xA3,0x00,
		0xBE,2,0x00,0x63,
		0xD1,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,

		0xD2,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,

		0xD3,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,

		0xD4,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,

		0xD5,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,


		0xD6,52,
			0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,0x00,0xD2,0x00,0xF6,0x01,0x27,
			0x01,0x4E,0x01,0x8C,0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,0x02,0xBC,
			0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,
			0x03,0xB9,0x03,0xC1,


		0xF0,5,0x55,0xAA,0x52,0x08,0x00,
		0xB0,5,0x08,0x05,0x02,0x05,0x02,
		0xB6,1,0x08,
		0xB5,1,0x50,  //480x800
		0xB7,2,0x00,0x00,
		0xB8,4,0x01,0x05,0x05,0x05,
		0xBC,3,0x00,0x00,0x00,
		0xCC,3,0x03,0x00,0x00,
		0xBD,5,0x01,0x84,0x07,0x31,0x00,

		0xBA,1,0x01,
		0xFF,4,0xAA,0x55,0x25,0x01,
		0x35,1,0x00,
		0x3A,1,0x66,    // 55=Colour 565 66=Colour 666 77=Colour 888
		0x11,DELAY,100,   // uSec( 100000);//delay(100);
		0x29,DELAY,100    // uSec( 100000);//delay(50);
};

static const uint8_t
OTM8009A_16Init[] = {   // Initialization commands for OTM8009A_16 screen
	    51,            // no of commands in list:
		2,             // 0= B7-B0 as 8bit command  1=single byte shifted to B15-B8 as 16bit command 2= two bytes needed

		0xff,0x00,3,0x80,0x09,0x01, //enable access command2
		0xff,0x80,2,0x80,0x09, //enable access command2
		0xff,0x03,1,0x01,  //DON?T KNOW ???
		0xc5,0xb1,1,0xA9, //power control
		0xc5,0x91,1,0x0F, //power control
		0xc0,0xB4,1,0x50,
		0xE1,0x00,16,0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,
		0xE2,0x00,16,0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,
	    0xD9,0x00,1,0x4E,      /* VCOM Voltage Setting */
		0xc1,0x81,1,0x66,      //osc=65HZ

		0xc1,0xa1,1,0x08,      //RGB Video Mode Setting
		0xc5,0x92,1,0x01,      //power control
		0xc5,0x95,1,0x34,      //power control
		0xd8,0x00,2,0x79,0x79, //GVDD / NGVDD setting
		0xc5,0x94,1,0x33, //power control
		0xc0,0xa3,1,0x1B,//panel timing setting
		0xc5,0x82,1,0x83, //power control
		0xc4,0x81,1,0x83,  //source driver setting
		0xc1,0xa1,1,0x0E,
		0xb3,0xa6,2,0x20,0x01,

		0xce,0x80,6,0x85,0x01,0x00,0x84,0x01,0x00,  // GOA VST
		0xce,0xa0,7,0x18,0x04,0x03,0x39,0x00,0x00,0x00,
		0xce,0xa7,7,0x18,0x03,0x03,0x3a,0x00,0x00,0x00,
		0xce,0xb0,14,0x18,0x02,0x03,0x3b,0x00,0x00,0x00,0x18,0x01,0x03,0x3c,0x00,0x00,0x00,
		0xcf,0xc0,10,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x00,0x00,0x00,
		0xcf,0xd0,1,0x00,
		0xcb,0x80,REPEAT+10,0x00,
		0xcb,0x90,REPEAT+15,0x00,
		0xcb,0xa0,REPEAT+15,0x00,
		0xcb,0xb0,REPEAT+10,0x00,

		0xcb,0xc0,1,0x00,
		0xcb,0xc1,REPEAT+5,0x04,
		0xcb,0xc6,REPEAT+9,0x00,
		0xcb,0xd0,REPEAT+6,0x00,
		0xcb,0xd6,REPEAT+5,0x04,
		0xcb,0xdb,REPEAT+4,0x00,
		0xcb,0xe0,REPEAT+10,0x00,
		0xcb,0xf0,REPEAT+10,0xff,
		0xcc,0x80,10,0x00,0x26,0x09,0x0B,0x01,0x25,0x00,0x00,0x00,0x00,
		0xcc,0x90,REPEAT+11,0x00,

		0xcc,0x9b,4,0x26,0x0A,0x0C,0x02,
		0xcc,0xa0,1,0x25,
		0xcc,0xa1,REPEAT+14,0x00,
		0xcc,0xb0,10,0x00,0x25,0x0c,0x0a,0x02,0x26,0x00,0x00,0x00,0x00,
		0xcc,0xc0,REPEAT+11,0x00,
		0xcc,0xcb,4,0x25,0x0B,0x09,0x01,
		0xcc,0xd0,1,0x26,
		0xcc,0xd1,REPEAT+14,0x00,
		0x3A,0x00,1,0x55,
		0x11,0x00,DELAY,100,

		0x29,0x00,DELAY,100

};

// Companion code to the above tables.  Reads and issues
// a series of LCD commands.
//command types
//   0= B7-B0 as 8bit command - command is not repeated for each subsequent byte of data. LCDPanel accepts multiple data bytes with each commande
//   1=single byte shifted to B15-B8 as 16bit command -command incremented and sent with each new data byte
//   2= two command bytes read to fill B15-B0 as 16bit command - command incremented and sent with each new data byte
void static SendCommand16Block(const uint8_t *addr) {
   uint8_t numCommands, numArgs,cmdType,numReps;
   uint16_t ms,cmd;
   numCommands = *(addr++);           // Number of commands to follow
   cmdType = *(addr++);               // Number of commands to follow
   while(numCommands--) {                 // For each command...
	 if (cmdType==2){
		 cmd  = (*(addr++)<<8);
         cmd |= *(addr++);
	 }else if (cmdType==1){
		 cmd  = (*(addr++)<<8);
	 }else{ //cmdType==0
		 cmd = *(addr++);
		 WriteSSD1963Command(cmd) ;        //   Read, issue command and increment address
	 }
	 numArgs  = *(addr++);                //   Number of args to follow
	 ms       = numArgs & DELAY;          //   If hibit set, delay follows args
     numArgs &= ~DELAY;                   //   Mask out delay bit
     numReps  = numArgs & REPEAT;         //   If B6 set then repeat same byte for numArgs
     numArgs &= ~REPEAT;                   //   Mask out repeat bit
     if (numArgs==0 && cmdType!=0){WriteSSD1963Command(cmd++) ;}
     while(numArgs--) { //   For each argument...
       if (numReps){
    	   if(cmdType!=0){WriteSSD1963Command(cmd++) ; }
    	   WriteDataSSD1963(*(addr));         //     Read, issue argument DONT step address
       }else{
    	   if(cmdType!=0){WriteSSD1963Command(cmd++) ; }
    	   WriteDataSSD1963(*(addr++));         //  Read, issue argument step address
       }
     }
     if (numReps){addr++;}                //move pointer to next value
     if(ms) {
       ms = *(addr++);                    // Read post-command delay time (ms)
       if(ms == 255) ms = 500;            // If 255, delay for 500 ms
         uSec(ms*1000);                   //convert to uS
     }
   }
}



void InitILI9341(void){


     if(Option.DISPLAY_TYPE ==  ILI9486_16)	{
	        SendCommand16Block(ILI9486_16Init);
			//ILI9486 initialisation
	        /*
			WriteSSD1963Command(0x01); uSec( 100000); // software reset 100ms pause
			WriteSSD1963Command(0x28); // display off
			WriteSSD1963CommandData(0xF1,6,0x36,0x04,0x00,0x3c,0x0f,0x8f);
			WriteSSD1963CommandData(0xF2,9,0x18,0xa3,0x12,0x02,0xb2,0x12,0xff,0x10,0x00);
			WriteSSD1963CommandData(0xf8,2,0x21,0x04);
			WriteSSD1963CommandData(0xf9,2,0x00,0x08);
			WriteSSD1963CommandData(0xb4,1,0x00);
			WriteSSD1963CommandData(0xc1,1,0x47);
			WriteSSD1963CommandData(0xC5,4,0x00,0xaf,0x80,0x00);
			WriteSSD1963CommandData(0xe0,15,0x0f,0x1f,0x1c,0x0c,0x0f,0x08,0x48,0x98,0x37,0x0a,0x13,0x04,0x11,0x0d,0x00);//Set Gamma
			WriteSSD1963CommandData(0xe1,15,0x0f,0x32,0x2e,0x0b,0x0d,0x05,0x47,0x75,0x37,0x06,0x10,0x03,0x24,0x20,0x00); //Set Gamma
			WriteSSD1963Command(0x34); //Tearing Effect Off
			WriteSSD1963CommandData(0x3A,1,0x66);// 55= RGB565 66=RGB666
			WriteSSD1963Command(0x11);uSec( 150000);
			WriteSSD1963Command(0x29);uSec( 500000);
            */
	 } else {

		//ILI9341_16 Initialisation
		 SendCommand16Block(ILI9341_16Init);
		 /*
		WriteSSD1963CommandData(0xCB,5,0x39,0x2c,0x00,0x34,0x02);
		WriteSSD1963CommandData(0xCF,3,0x00,0xc1,0x30);
		WriteSSD1963CommandData(0xE8,3,0x85,0x00,0x78);
		WriteSSD1963CommandData(0xEA,2,0x00,0x00);
		WriteSSD1963CommandData(0xED,4,0x64,0x03,0x12,0x81);
		WriteSSD1963CommandData(0xF7,1,0x20);
		WriteSSD1963CommandData(0xC0,1,0x23);    //Power control  //VRH[5:0]
		WriteSSD1963CommandData(0xC1,1,0x10);    //Power control   //SAP[2:0];BT[3:0]
		WriteSSD1963CommandData(0xC5,2,0x3e,0x28);    //VCM control //Contrast
		WriteSSD1963CommandData(0xC7,1,0x86);    //VCM control2
		WriteSSD1963CommandData(0x3A,1,0x66); // 55= RGB565 66=RGB666
		WriteSSD1963CommandData(0xB1,2,0x00,0x18);
		WriteSSD1963CommandData(0xB6,3,0x08,0x82,0x27); // Display Function Control
		WriteSSD1963CommandData(0xF2,1,0x00);    // 3Gamma Function Disable
		WriteSSD1963CommandData(0x26,1,0x01);    //Gamma curve selected
		WriteSSD1963CommandData(0xe0,15,0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00);//Set Gamma
		WriteSSD1963CommandData(0xe1,15,0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xc1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f); //Set Gamma
		*/

	 }
		uSec(120000);
		int i=0;
		switch(Option.DISPLAY_ORIENTATION) {
        	case LANDSCAPE:     i=ILI9341_Landscape; break;
        	case PORTRAIT:      i=ILI9341_Portrait; break;
        	case RLANDSCAPE:    i=ILI9341_Landscape180; break;
        	case RPORTRAIT:     i=ILI9341_Portrait180; break;
		}
		WriteSSD1963Command(0x36);    // Memory Access Control
		WriteDataSSD1963(i);
		WriteSSD1963Command(0x11);    //Exit Sleep
		uSec(120000);
		WriteSSD1963Command(0x29); //display on
		WriteSSD1963Command(0x2c); //display on
		ClearScreen(Option.DefaultBC);
}
// ----------- OTM8009A ad NT35510 Initialisation  -- Always uses 16bit commands
void InitIPS_4_16(void){
        int t=0;
		GPIO_WR(LCD_RESET,1);
		uSec(50000);
		GPIO_WR(LCD_RESET,0);
		uSec(50000);
		GPIO_WR(LCD_RESET,1);
		uSec(50000);

		//read the id to see if OTM8009A or NT35510
		WriteSSD1963Command(0xDA00);
		t=FMC_RBANK1->RAM ; // dummy read
		t=FMC_RBANK1->RAM ; // read id
		//MMPrintString("ID1=");PIntH(t);PRet();
        if ((t & 0x7F) == 0x55){   //was ((t & 0x7F) == 0x55) //((t & 0x71) == 0x51)
        	// NT35510 IPS Display detected. Identified in code by (LCDAttrib==1)
        	LCDAttrib = 1;
            SaveOptions();
            SendCommand16Block(NT35510_16Init);
 		}else{
		//============ OTM8009A+HSD3.97 20140613 ===============================================//
 			LCDAttrib = 0;
			SendCommand16Block(OTM8009A_16Init);
		}

		//It is a natural PORTrait LCD, but we will fudge it to make it
		//work for 1=landscape,2=portrait 3=RLandscape,4=RPortrait

        #define OTM8009A_MADCTL_MY  0x80
        #define OTM8009A_MADCTL_MX  0x40
        #define OTM8009A_MADCTL_MV  0x20

		int i=0;
		switch(Option.DISPLAY_ORIENTATION) {
		   	case LANDSCAPE:     i=OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MV; break;
		   	case PORTRAIT:      i=0x0 ; break;
		   	case RLANDSCAPE:    i=OTM8009A_MADCTL_MY | OTM8009A_MADCTL_MV; break;
		   	case RPORTRAIT:     i=OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MY; break;
		}

		WriteCmdDataIPS_4_16(0x3600,1,i);   //set Memory Access Control
		ClearScreen(Option.DefaultBC);

}
void InitSSD1963(void) {
	int myDisplayHRes=DisplayHRes;
	//LCD_BL_Period=100-LCD_BL_Period;
    // IMPORTANT: At this stage the SSD1963 is running at a slow speed and cannot respond to high speed commands
    //            So we use slow speed versions of WriteSSD1963Command/WriteDataSSD1963 with a 3 uS delay between each control signal change
    
	// Set MN(multipliers) of PLL, VCO = crystal freq * (N+1)
	// PLL freq = VCO/M with 250MHz < VCO < 800MHz
	// The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq
    
	WriteSSD1963Command(CMD_SET_PLL_MN);	                            // Set PLL with OSC = 10MHz (hardware), command is 0xE3
	WriteDataSSD1963(0x23);				                            // Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz
	WriteDataSSD1963(0x02);				                            // Divider M = 2, PLL = 360/(M+1) = 120MHz
	WriteDataSSD1963(0x54);				                            // Validate M and N values

	WriteSSD1963Command(CMD_PLL_START);	                            // Start PLL command
	WriteDataSSD1963(0x01);				                            // enable PLL

	uSec(10000);						                                // wait stablize
	WriteSSD1963Command(CMD_PLL_START);	                            // Start PLL command again
	WriteDataSSD1963(0x03);				                            // now, use PLL output as system clock

	WriteSSD1963Command(CMD_SOFT_RESET);	                            // Soft reset
	uSec(10000);

#define parallel_write_data WriteDataSSD1963
#define TFT_Write_Data WriteDataSSD1963

	// Configure for the TFT panel, varies from individual manufacturer
	WriteSSD1963Command(CMD_SET_PCLK);		                            // set pixel clock (LSHIFT signal) frequency
	WriteDataSSD1963(SSD1963PClock1);                                  // paramaters set by DISPLAY INIT
	WriteDataSSD1963(SSD1963PClock2);
	WriteDataSSD1963(SSD1963PClock3);
    //	uSec(10000);

    
	// Set panel mode, varies from individual manufacturer
	WriteSSD1963Command(CMD_SET_PANEL_MODE);
	WriteDataSSD1963(SSD1963Mode1);                                        // parameters set by DISPLAY INIT
	WriteDataSSD1963(SSD1963Mode2);
	WriteDataSSD1963((myDisplayHRes - 1) >> 8);                              //Set panel size
	WriteDataSSD1963(myDisplayHRes - 1);
	WriteDataSSD1963((DisplayVRes - 1) >> 8);
	WriteDataSSD1963(DisplayVRes - 1);
	WriteDataSSD1963(0x00);				                                //RGB sequence


	// Set horizontal period
	WriteSSD1963Command(CMD_SET_HOR_PERIOD);
	#define HT (myDisplayHRes + SSD1963HorizPulseWidth + SSD1963HorizBackPorch + SSD1963HorizFrontPorch)
	WriteDataSSD1963((HT - 1) >> 8);
	WriteDataSSD1963(HT - 1);
	#define HPS (SSD1963HorizPulseWidth + SSD1963HorizBackPorch)
	WriteDataSSD1963((HPS - 1) >> 8);
	WriteDataSSD1963(HPS - 1);
	WriteDataSSD1963(SSD1963HorizPulseWidth - 1);
	WriteDataSSD1963(0x00);
	WriteDataSSD1963(0x00);
	WriteDataSSD1963(0x00);
	
	// Set vertical period
	WriteSSD1963Command(CMD_SET_VER_PERIOD);
	#define VT (SSD1963VertPulseWidth + SSD1963VertBackPorch + SSD1963VertFrontPorch + DisplayVRes)
	WriteDataSSD1963((VT - 1) >> 8);
	WriteDataSSD1963(VT - 1);
	#define VSP (SSD1963VertPulseWidth + SSD1963VertBackPorch)
	WriteDataSSD1963((VSP - 1) >> 8);
	WriteDataSSD1963(VSP - 1);
	WriteDataSSD1963(SSD1963VertPulseWidth - 1);
	WriteDataSSD1963(0x00);
	WriteDataSSD1963(0x00);
	
	// Set pixel data interface
	WriteSSD1963Command(CMD_SET_DATA_INTERFACE);
    WriteDataSSD1963(SSD1963PixelInterface);                                                // 8-bit colour format
	WriteSSD1963Command(CMD_SET_PIXEL_FORMAT);
    WriteDataSSD1963(SSD1963PixelFormat);                                                // 8-bit colour format

    // initialise the GPIOs
	WriteSSD1963Command(CMD_SET_GPIO_CONF);                                // Set all GPIOs to output, controlled by host
	WriteDataSSD1963(0x0f);				                                // Set GPIO0 as output
	WriteDataSSD1963(0x01);				                                // GPIO[3:0] used as normal GPIOs

	// LL Reset to LCD!!!
	GPIO_WR(LCD_SPENA, 1);
	GPIO_WR(LCD_SPCLK, 1);
	GPIO_WR(LCD_SPDAT,1);
	GPIO_WR(LCD_RESET,1);
	GPIO_WR(LCD_RESET,0);
	uSec(1000);
	GPIO_WR(LCD_RESET,1);

    // setup the pixel write order
    WriteSSD1963Command(CMD_SET_ADDR_MODE);
    switch(Option.DISPLAY_ORIENTATION) {
        case LANDSCAPE:     WriteDataSSD1963(SSD1963_LANDSCAPE  | SSD1963rgb); break;
        case PORTRAIT:      WriteDataSSD1963(SSD1963_PORTRAIT   | SSD1963rgb); break;
        case RLANDSCAPE:    WriteDataSSD1963(SSD1963_RLANDSCAPE | SSD1963rgb); break;
        case RPORTRAIT:     WriteDataSSD1963(SSD1963_RPORTRAIT  | SSD1963rgb); break;
    }

    // Set the scrolling area
	WriteSSD1963Command(CMD_SET_SCROLL_AREA);
	WriteDataSSD1963(0);
	WriteDataSSD1963(0);
	WriteDataSSD1963(DisplayVRes >> 8);
	WriteDataSSD1963(DisplayVRes);
	WriteDataSSD1963(0);
	WriteDataSSD1963(0);
    ScrollStart = 0;
 
	ClearScreen(Option.DefaultBC);
   // SetBacklightSSD1963(Option.DefaultBrightness);
	WriteSSD1963Command(CMD_ON_DISPLAY);	                                // Turn on display; show the image on display	
}


/**********************************************************************************************
Draw a filled rectangle on the video output with physical frame buffer coordinates
     x1, y1 - the start physical frame buffer coordinate
     x2, y2 - the end physical frame buffer coordinate
     c - the colour to use for both the fill
 This is only called by DrawRectangleSSD1963() below
***********************************************************************************************/
void PhysicalDrawRect_16(int x1, int y1, int x2, int y2, int c) {
    int i;
    if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16) {
    	WriteSSD1963Command(ILI9341_PIXELFORMAT);
    	WriteDataSSD1963(0x55); //change to RGB565 for write rectangle
    	SetAreaILI9341(x1, y1 , x2, y2, 1);
    }else if(Option.DISPLAY_TYPE==IPS_4_16) {
    	if(LCDAttrib==1)WriteCmdDataIPS_4_16(0x3A00,1,0x55);
    	SetAreaIPS_4_16(x1, y1 , x2, y2, 1);
    } else {
    	SetAreaSSD1963(x1, y1 , x2, y2);                                // setup the area to be filled
    	WriteSSD1963Command(CMD_WR_MEMSTART);
    }
    uint16_t j=((c>>8) & 0xf800) | ((c>>5) & 0x07e0) | ((c>>3) & 0x001f);
    for(i = (x2 - x1 + 1) * (y2 - y1 + 1); i > 0; i--){
    	  FMC_RBANK1->RAM = j;
    	  __DSB();
    }
    if(LCDAttrib==1)WriteCmdDataIPS_4_16(0x3A00,1,0x66);
    if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16){
    	WriteSSD1963Command(ILI9341_PIXELFORMAT);
    	WriteDataSSD1963(0x66);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Drawing primitives used by the functions in GUI.c and Draw.c
//
////////////////////////////////////////////////////////////////////////////////////////////////////////


/**********************************************************************************************
Draw a filled rectangle on the video output with logical (MMBasic) coordinates
     x1, y1 - the start coordinate
     x2, y2 - the end coordinate
     c - the colour to use for both the fill
***********************************************************************************************/
void DrawRectangleSSD1963(int x1, int y1, int x2, int y2, int c) {
    int t;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= HRes) x1 = HRes - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= HRes) x2 = HRes - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= VRes) y1 = VRes - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= VRes) y2 = VRes - 1;

    t = y2 - y1;                                                    // get the distance between the top and bottom

    // set y1 to the physical location in the frame buffer (only really has an effect when scrolling is in action)
    if(Option.DISPLAY_ORIENTATION == RLANDSCAPE)
        y1 = (y1 + (VRes - ScrollStart)) % VRes;
    else
        y1 = (y1 + ScrollStart) % VRes;
    y2 = y1 + t;
    if(Option.DISPLAY_TYPE >= SSD_PANEL_START){
        if(y2 >= VRes) {                                                // if the box splits over the frame buffer boundary
            PhysicalDrawRect_16(x1, y1, x2, VRes - 1, c);                  // draw the top part
            PhysicalDrawRect_16(x1, 0, x2, y2 - VRes , c);                 // and the bottom part
        } else
            PhysicalDrawRect_16(x1, y1, x2, y2, c);                        // the whole box is within the frame buffer - much easier
    }
}
void DrawBufferSSD1963_16(int x1, int y1, int x2, int y2, char* p) {
    int i;
	int t,toggle=0;
	unsigned int bl=0;
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2, x, y;
    union colourmap
    {
    char rgbbytes[4];
    unsigned int rgb;
    } c;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;

    t = yy2 - yy1;                                                    // get the distance between the top and bottom

    if(Option.DISPLAY_ORIENTATION == RLANDSCAPE)
        yy1 = (yy1 + (VRes - ScrollStart)) % VRes;
    else
        yy1 = (yy1 + ScrollStart) % VRes;
    yy2 = yy1 + t;                                                    // and set y2 to the same
    	if(yy2 >= VRes) {  //Only SSD1963 with hardware scrolling can get here
        	SetAreaSSD1963(xx1, yy1, xx2, VRes - 1); // if the box splits over the frame buffer boundary
        	WriteSSD1963Command(CMD_WR_MEMSTART);
    		for(i = (xx2 - xx1 + 1) * ((VRes - 1) - yy1 + 1); i > 0; i--){
    			c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    			c.rgbbytes[1]=*p++;
    			c.rgbbytes[2]=*p++;
    			FMC_RBANK1->RAM = ((c.rgb>>8) & 0xf800) | ((c.rgb>>5) & 0x07e0) | ((c.rgb>>3) & 0x001f);
    			__DSB();
				}

        	SetAreaSSD1963(xx1, 0, xx2, yy2 - VRes );
        	WriteSSD1963Command(CMD_WR_MEMSTART);

    		for(i = (xx2 - xx1 + 1) * (yy2 - VRes + 1); i > 0; i--){
    			c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    			c.rgbbytes[1]=*p++;
    			c.rgbbytes[2]=*p++;
    			FMC_RBANK1->RAM = ((c.rgb>>8) & 0xf800) | ((c.rgb>>5) & 0x07e0) | ((c.rgb>>3) & 0x001f);
    			__DSB();
				}
    	} else {
    		// the whole box is within the frame buffer - much easier
        	if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ) {
        		LCDAttrib=2;
        		SetAreaILI9341(xx1, yy1 , xx2, yy2, 1);
        	}else if(Option.DISPLAY_TYPE==IPS_4_16) {
        		SetAreaIPS_4_16(xx1, yy1 , xx2, yy2, 1);
        	} else {
        		SetAreaSSD1963(xx1, yy1 , xx2, yy2);                                // setup the area to be filled
        		WriteSSD1963Command(CMD_WR_MEMSTART);
        	}
   		for(y=y1;y<=y2;y++){
    			for(x=x1;x<=x2;x++){

    				if(x>=0 && x<HRes && y>=0 && y<VRes){

    					  c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    					  c.rgbbytes[1]=*p++;
    					  c.rgbbytes[2]=*p++;

    					 if(LCDAttrib==0){
    					    FMC_RBANK1->RAM = ((c.rgb>>8) & 0xf800) | ((c.rgb>>5) & 0x07e0) | ((c.rgb>>3) & 0x001f);
    					   __DSB();
    					 }else{ //NT35510 and ILI9341_16 in RGB666
						    if(toggle==0){
						    	FMC_RBANK1->RAM = ((c.rgb>>8) & 0xf800) | ((c.rgb>>8) & 0x00fc);__DSB(); // RG
						    	bl=(c.rgb & 0x00f8); //save blue
						    	toggle=1;
						    }else{
						    	FMC_RBANK1->RAM = (bl<<8) | ((c.rgb>>16) & 0x00f8) ;__DSB();  // BR
						    	FMC_RBANK1->RAM = ((c.rgb) & 0xfc00) | ((c.rgb) & 0x00f8) ;__DSB();
						    	toggle=0;
						    }

    					 }
    				} else {
    					p+=3;
    				}
    			}
    		}
   	 if(LCDAttrib==2)LCDAttrib=0;
   	 if((LCDAttrib==1) && (toggle==1)){
   			// extra packet needed
   			FMC_RBANK1->RAM = (bl<<8) | ((c.rgb>>8) & 0x00f8) ; __DSB();  //
   	 }
  }
}
void ReadBufferSSD1963_16(int x1, int y1, int x2, int y2, char* p) {
     int x, y, t, i,nr=0 ;
	int toggle=0,t1=0;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;

     t = yy2 - yy1;                                                    // get the distance between the top and bottom
    if(Option.DISPLAY_ORIENTATION == RLANDSCAPE)
        yy1 = (yy1 + (VRes - ScrollStart)) % VRes;
    else
        yy1 = (yy1 + ScrollStart) % VRes;
    yy2 = yy1 + t;                                                    // and set y2 to the same

    if(yy2 >= VRes) {      /////////Only the SSD1963 with hardware scrolling can get here

    	SetAreaSSD1963(xx1, yy1, xx2, VRes - 1); // if the box splits over the frame buffer boundary
    	WriteSSD1963Command(CMD_RD_MEMSTART);
    	t=FMC_RBANK1->RAM ; __DSB();
        *p++=(t & 0x1F)<<3; *p++=(t & 0x7e0)>>3; *p++=(t & 0xf800)>>8;
        i=(xx2 - xx1 + 1) * ((VRes - 1) - yy1 + 1);
        for( ; i > 1; i--){
        	t=FMC_RBANK1->RAM ; __DSB();
            *p++=(t & 0x1F)<<3; *p++=(t & 0x7e0)>>3; *p++=(t & 0xf800)>>8;
        }

    	SetAreaSSD1963(xx1, 0, xx2, yy2 - VRes );
    	WriteSSD1963Command(CMD_RD_MEMSTART);
    	t=FMC_RBANK1->RAM ; __DSB();
        *p++=(t & 0x1F)<<3; *p++=(t & 0x7e0)>>3; *p++=(t & 0xf800)>>8;
         for(i = (xx2 - xx1 + 1) * (yy2 - VRes + 1); i > 1; i--){
        	t=FMC_RBANK1->RAM ; __DSB();
            *p++=(t & 0x1F)<<3; *p++=(t & 0x7e0)>>3; *p++=(t & 0xf800)>>8;
        }
    } else {
    	// the whole box is within the frame buffer - much easier
    	if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ) {
    		SetAreaILI9341(xx1, yy1 , xx2, yy2, 0);
    		t=FMC_RBANK1->RAM ; // dummy read
    	}else if(Option.DISPLAY_TYPE==IPS_4_16) {
    		SetAreaIPS_4_16(xx1, yy1 , xx2, yy2, 0);
    		t=FMC_RBANK1->RAM ; // dummy read
    	} else {
    		SetAreaSSD1963(xx1, yy1 , xx2, yy2);                                // setup the area to be filled
    		WriteSSD1963Command(CMD_RD_MEMSTART);
    	}
        for(y=y1;y<=y2;y++){
            for(x=x1;x<=x2;x++){
                if(x>=0 && x<HRes && y>=0 && y<VRes){
                	 if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ) {
                	      if(toggle==0){
                	         t=(FMC_RBANK1->RAM); __DSB();
                	         t<<=8 ;
                	         t1=FMC_RBANK1->RAM ; __DSB();
                	         t|=(t1>>8);
                	         t1 &=0xFF;
                	         toggle=1;
                	      } else {
                	         t=FMC_RBANK1->RAM; __DSB();
                	       	t|= (t1<<16);
                	        toggle=0;
                	      }
                	      *p++=(t & 0xf8); *p++=(t & 0xfc00)>>8; *p++=(t & 0xf80000)>>16;

                	}else if(Option.DISPLAY_TYPE==IPS_4_16) {
                		if(toggle==0){     //RGBR 8bit each
                		  	t=(FMC_RBANK1->RAM); __DSB(); // RG
                		   	t1=FMC_RBANK1->RAM ; __DSB(); // BR

                		    *p++=(t1 & 0xF800)>>8;  //BLUE
                		    *p++=(t & 0xFC);      //GREEN
                			*p++=(t & 0xF800)>>8;   //RED

                		    nr=(t1 & 0xF8);       //save next red

                		    if(LCDAttrib==1){  //NT35510 does not need toggle=1
                		      toggle=0;
                		    }else{
                		      toggle=1;
                		    }
                		} else {

                		 	t=FMC_RBANK1->RAM; __DSB(); //get the second  GB
                		 	*p++=(t & 0xF8);       //Blue
                		 	*p++=(t & 0xFC00)>>8;  //Green   FIX  HERE
                		 	*p++=nr ;              //add the red

                		  	toggle=0;
                		}


                	} else {
                    	t=FMC_RBANK1->RAM ; __DSB();
                        *p++=(t & 0x1F)<<3; *p++=(t & 0x7e0)>>3; *p++=(t & 0xf800)>>8;
                	}
                } else p+=3; //don't read data for pixels outside the screen
            }
        }

    } ////////////////////////////////////////////////////////////////////////////////
}void fun_getscanline(void){
	if(Option.DISPLAY_TYPE > SSD_PANEL_END &&  Option.DISPLAY_TYPE < P16_PANEL_START) error("Invalid on this display");
	int t;
	int x=0,c;
		getargs(&ep, 3,",");
		if(argc == 0){
			    if(Option.DISPLAY_TYPE == IPS_4_16){
					WriteSSD1963Command(0x4500 );
					t=FMC_RBANK1->RAM ; // dummy read
					t=(FMC_RBANK1->RAM <<8);
					if(LCDAttrib==1 && x==0){
						WriteSSD1963Command(0x4501 );
						x=FMC_RBANK1->RAM ; // dummy read
					}
					t|=FMC_RBANK1->RAM;
				}else if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ){
					WriteSSD1963Command(CMD_GET_SCANLINE);
					t=FMC_RBANK1->RAM ; // dummy read
					t=(FMC_RBANK1->RAM <<8);
					t|=FMC_RBANK1->RAM;
				}else{
					WriteSSD1963Command(CMD_GET_SCANLINE);
					t=(FMC_RBANK1->RAM <<8);
					t|=FMC_RBANK1->RAM;
			   }

		}else{
			t=0;
			x = getinteger(argv[0]);
			if (x==1)c=0xDA;	//Read ID1
			if (x==2)c=0xDB;	//Read ID2
			if (x==3)c=0xDC;	//Read ID3
			if(Option.DISPLAY_TYPE == IPS_4_16){
					WriteSSD1963Command(c<<8 );
					x=FMC_RBANK1->RAM ; // dummy read
					t=(FMC_RBANK1->RAM);

			}else if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ){
					WriteSSD1963Command(c);
					t=FMC_RBANK1->RAM ; // dummy read
					t=(FMC_RBANK1->RAM );
			}

		}

	iret=t;
	targ = T_INT;
}

/***********************************************************************************************
Print the bitmap of a char on the video output
	x1, y1 - the top left of the char
    width, height - size of the char's bitmap
    scale - how much to scale the bitmap
	fg, bg - foreground and background colour
    bitmap - pointer to the bitmap
***********************************************************************************************/
void DrawBitmapSSD1963_16(int x1, int y1, int width, int height, int scale, int fg, int bg, unsigned char *bitmap ){
    int i, j, k, m, n, y, yt;
    int vertCoord, horizCoord, XStart, XEnd;//, fg16, bg16,ig16;
    int toggle=0;
    unsigned int p1=0,bl=0,fg16, bg16,ig16;
    char *buff;
    union car
    {
    char rgbbytes[4];
    unsigned int rgb;
    } c;
    buff=NULL;
    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    // adjust when part of the bitmap is outside the displayable coordinates
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    if(bg==-1){
        buff=GetMemory(width * height * scale * scale * 3 );
        ReadBuffer(XStart, y1, XEnd, (y1 + (height * scale) - 1), buff);
        n=0;
    }
    fg16=((fg>>8) & 0xf800) | ((fg>>5) & 0x07e0) | ((fg>>3) & 0x001f);
    bg16=((bg>>8) & 0xf800) | ((bg>>5) & 0x07e0) | ((bg>>3) & 0x001f);
    // set y and yt to the physical location in the frame buffer (only is important when scrolling is in action)
    if(Option.DISPLAY_ORIENTATION == RLANDSCAPE) yt = y = (y1 + (VRes - ScrollStart)) % VRes;
    else yt = y = (y1 + ScrollStart) % VRes;
    
	if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ) {
		LCDAttrib=2;
		SetAreaILI9341(XStart, y, XEnd, (y + (height * scale) - 1)  % VRes, 1);
	}else if(Option.DISPLAY_TYPE==IPS_4_16 ) {
		SetAreaIPS_4_16(XStart, y, XEnd, (y + (height * scale) - 1)  % VRes, 1);
	} else {
		SetAreaSSD1963(XStart, y, XEnd, (y + (height * scale) - 1)  % VRes);
		WriteSSD1963Command(CMD_WR_MEMSTART);
	}


		for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
			for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
				if(vertCoord++ < 0) continue;                           // we are above the top of the screen
				if(vertCoord > VRes){
					if(buff!=NULL)FreeMemory(buff);
					return;                            // we have extended beyond the bottom of the screen
				}
				// if we have scrolling in action we could run over the end of the frame buffer
				// if so, terminate this area and start a new one at the top of the frame buffer
				if(y++ == VRes) {
					if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9486_16 ) {
						LCDAttrib=2;
						SetAreaILI9341(XStart, 0, XEnd, ((yt + (height * scale) - 1)  % VRes) - y, 1);
					}else if(Option.DISPLAY_TYPE==IPS_4_16) {
						SetAreaIPS_4_16(XStart, 0, XEnd, ((yt + (height * scale) - 1)  % VRes) - y, 1);
					} else {
						SetAreaSSD1963(XStart, 0, XEnd, ((yt + (height * scale) - 1)  % VRes) - y);
						WriteSSD1963Command(CMD_WR_MEMSTART);
					}
				}
				horizCoord = x1;
				for(k = 0; k < width; k++) {                            // step through each bit in a scan line
					for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
						if(horizCoord++ < 0) continue;                  // we have not reached the left margin
						if(horizCoord > HRes) continue;                 // we are beyond the right margin
						if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                                p1=fg16;


						} else {
							 if(buff!=NULL){
								c.rgbbytes[0]=buff[n];
								c.rgbbytes[1]=buff[n+1];
								c.rgbbytes[2]=buff[n+2];
								ig16=((c.rgb>>8) & 0xf800) | ((c.rgb>>5) & 0x07e0) | ((c.rgb>>3) & 0x001f);
								p1=ig16;

							 } else {
								p1=bg16;
							 }
						}
						if(LCDAttrib==0){
							FMC_RBANK1->RAM = p1; __DSB();
						}else{
						     if(toggle==0){

                                  FMC_RBANK1->RAM = (p1  & 0xf800) | ((p1>>3) & 0xfc);
                                  __DSB(); // RG
								  bl=((p1<<3) & 0xf8); //save blue
								  toggle=1;
						      }else{

                                  FMC_RBANK1->RAM =((bl<<8 ) & 0xf800) | ((p1>>8) & 0xf8) ; // BR
                                  __DSB();
								  FMC_RBANK1->RAM =((p1<<5 ) & 0xfc00) | ((p1<<3) & 0xf8) ; // GB
								  __DSB();
								 toggle=0;
							}
						}
						n+=3;
					}

				}
			}
		}
       if(LCDAttrib==2)LCDAttrib=0;
	   if((LCDAttrib==1) && (toggle==1)){
		// extra packet needed
		//p2= (((bl<<8 ) & 0xf800) | ((p1>>8) & 0xf8));
		FMC_RBANK1->RAM = ((bl<<8 ) & 0xf800) | ((p1>>8) & 0xf8);  // BR
		__DSB();
	   }
	   if(buff!=NULL)FreeMemory(buff);
}

/**********************************************************************************************
 Scroll the image by a number of scan lines
 Will only work in landscape or reverse landscape
 lines - the number of scan lines to scroll
        if positive the display will scroll up
        if negative it will scroll down
***********************************************************************************************/
void ScrollSSD1963(int lines) {
    int t;
    t = ScrollStart;

    if(lines >= 0) {
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
        while(lines--) {
            if(Option.DISPLAY_ORIENTATION == LANDSCAPE) {
                if(++t >= VRes) t = 0;
            } else if(Option.DISPLAY_ORIENTATION == RLANDSCAPE) {
                if(--t < 0) t = VRes - 1;
            }
        }
    } else {
        while(lines++) {
            if(Option.DISPLAY_ORIENTATION == LANDSCAPE) {
                if(--t < 0) t = VRes - 1;
            } else if(Option.DISPLAY_ORIENTATION == RLANDSCAPE) {
                if(++t >= VRes) t = 0;
            }
        }
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line introduced at the top
    }
	WriteSSD1963Command(CMD_SET_SCROLL_START);
	WriteDataSSD1963(t >> 8);
	WriteDataSSD1963(t);

    ScrollStart = t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Functions used to support the use of a SSD1963 display as the console
//
////////////////////////////////////////////////////////////////////////////////////////////////////////




/**********************************************************************************************
 Control the cursor
 This does all the work of displaying, flashing and removing the cursor.

 It is called by FullScreenEditor() and MMgetchar()
 Both of these functions keep calling ShowCursor() with the argument true while they are in a
 loop waiting for a character - this function will then display and flash the cursor.   When
 a character is received these functions will call ShowCursor() with the argument false which
 is the signal to remove the cursor from the screen.

 Note that CursorTimer is incremented by the millisecond interrupt
***********************************************************************************************/

void ShowCursor(int show) {
	static int visible = false;
    int newstate;

    if(!Option.DISPLAY_CONSOLE) return;
	newstate = ((CursorTimer <= CURSOR_ON) && show);                // what should be the state of the cursor?
	if(visible == newstate) return;									// we can skip the rest if the cursor is already in the correct state
	visible = newstate;                                             // draw the cursor BELOW the font

   if(gui_font_height!=10)DrawLine(CurrentX, CurrentY + gui_font_height, CurrentX + gui_font_width, CurrentY + gui_font_height, 2, visible ? gui_fcolour : gui_bcolour);
   else DrawLine(CurrentX, CurrentY + gui_font_height-2, CurrentX + gui_font_width, CurrentY + gui_font_height-2, 2, visible ? gui_fcolour : gui_bcolour);
}
/*
// Now allows different cursor in INS and OVR mode
//Use when editing Command line.
void ShowCursor(int show,int ins) {
	static int visible = false;
    int newstate;

    if(!Option.DISPLAY_CONSOLE) return;
	newstate = ((CursorTimer <= CURSOR_ON) && show);                // what should be the state of the cursor?
	if(visible == newstate) return;									// we can skip the rest if the cursor is already in the correct state
	visible = newstate;                                             // draw the cursor BELOW the font
	if(ins)	DrawLine(CurrentX + (gui_font_width>>2), CurrentY + gui_font_height, CurrentX + (gui_font_width>>1), CurrentY + gui_font_height, 3, visible ? gui_fcolour : gui_bcolour);
	else DrawLine(CurrentX, CurrentY + gui_font_height, CurrentX + gui_font_width, CurrentY + gui_font_height, 2, visible ? gui_fcolour : gui_bcolour);

}

*/

/******************************************************************************************
 Print a char on the LCD display (SSD1963 and in landscape only).  It handles control chars
 such as newline and will wrap at the end of the line and scroll the display if necessary.
 The char is printed at the current location defined by CurrentX and CurrentY
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Now updated so that \b will move to end of previous line when back spacing past beginning of
 the current line. Used when editing command line.

 *****************************************************************************************/
void DisplayPutC(char c) {

    if(!BasicRunning || !Option.DISPLAY_CONSOLE) return;

    // if it is printable and it is going to take us off the right hand end of the screen do a CRLF
    if(c >= FontTable[gui_font >> 4][2] && c < FontTable[gui_font >> 4][2] + FontTable[gui_font >> 4][3]) {
        if(CurrentX + gui_font_width > HRes) {
            DisplayPutC('\r');
            DisplayPutC('\n');
        }
    }

    // handle the standard control chars
    switch(c) {
        case '\b':
        	        CurrentX -= gui_font_width;
                    //if(CurrentX < 0)CurrentX = 0;
                    if(CurrentX < 0){  //Go to end of previous line
                    	CurrentY -= gui_font_height ;                  //Go up one line
                    	if (CurrentY < 0) CurrentY = 0;
                    	CurrentX = (Option.Width-1) * gui_font_width;  //go to last character

                    }
                    return;
        case '\r':  CurrentX = 0;
                    return;
        case '\n':  CurrentY += gui_font_height;
        if(CurrentY + gui_font_height >= VRes) {
            //if(Option.NoScroll && Option.DISPLAY_CONSOLE){ClearScreen(gui_bcolour);CurrentX=0;CurrentY=0;}
            if(Option.NoScroll && Option.DISPLAY_CONSOLE){ClearScreen(gui_bcolour);CurrentX=0;CurrentY=0;}
            else {
                ScrollLCD(CurrentY + gui_font_height - VRes);
                CurrentY -= (CurrentY + gui_font_height - VRes);
            }
        }
        return;

        case '\t':  do {
                        DisplayPutC(' ');
                    } while((CurrentX/gui_font_width) % Option.Tab);
                    return;

    }

    GUIPrintChar(gui_font, gui_fcolour, gui_bcolour, c, ORIENT_NORMAL);            // print it
}



/******************************************************************************************
 Print a string on the LCD display (SSD1963 and in landscape only).   It handles control
 chars such as newline and will wrap at the end of the line and scroll the display if
 necessary.

 The string is printed at the current location defined by CurrentX and CurrentY
*****************************************************************************************/
void DisplayPutS(char *s) {
    while(*s) DisplayPutC(*s++);
}

void ScrollILI9341(int lines){
	if(lines>0){
		DoBlit(0, lines, 0, 0, HRes, VRes-lines);
		DrawRectangle(0, VRes-lines, HRes - 1, VRes - 1, gui_bcolour); // erase the line to be scrolled off
    } else if(lines<0){
    	lines=-lines;
		DoBlit(0, 0, 0, lines, HRes, VRes-lines);
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
    }
}
