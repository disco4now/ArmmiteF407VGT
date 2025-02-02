/*-*****************************************************************************

ArmmiteF4 MMBasic

SPI-LCD.c

This is the driver for SPI LCDs in MMBasic.
The core SPI LCD driver was written and developed by Peter Mather of
the Back Shed Forum (http://www.thebackshed.com/forum/forum_topics.asp?FID=16)

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
#define GenSPI hspi2


void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw);
void DrawBitmapSPI(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
int CurrentSPISpeed=NONE_SPI_SPEED;
extern char LCDAttrib;
extern char LCDInvert;
extern SPI_HandleTypeDef GenSPI;
#define SPIsend(a) {uint8_t b=a;HAL_SPI_Transmit(&GenSPI,&b,1,500);}
#define SPIqueue(a) {HAL_SPI_Transmit(&GenSPI,a,2,500);}
#define SPIqueue3(a) {HAL_SPI_Transmit(&GenSPI,a,3,500);}
#define SPIsend2(a) {SPIsend(0);SPIsend(a);}
//#define SPIsend3(a) {SPIsend(0x81);SPIsend(a);}
extern int Xoff,Yoff;

// utility function for routines that want to reserve a pin for special I/O
// this ignores any previous settings and forces the pin to its new state
// pin is the pin number
// inp is true if an input or false if an output
// init is the value used to initialise the pin if it is an output (hi or lo)
// type is the final tag for the pin in ExtCurrentConfig[]


void MIPS16 SetAndReserve(int pin, int inp, int init, int type) {
    if(pin == 0) return;                                            // do nothing if not set
    GPIO_InitTypeDef GPIO_InitDef;
    if(inp) {
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
    } else {
        PinSetBit(pin, init ? LATSET : LATCLR);                     // set LAT
    	GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
    }
	GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
	GPIO_InitDef.Pin = PinDef[pin].bitnbr;
	GPIO_InitDef.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
    ExtCurrentConfig[pin] = type;
}


void MIPS16 ConfigDisplaySPI(char *p) {
	int p1, p2, p3;
	int DISPLAY_TYPE=0;
	char code;
    getargs(&p, 11, ",");
    // getargs(&p, 9, ",");

    if(!(argc == 7 || argc == 9 || argc == 11)) error("Argument count or display type");

    if(checkstring(argv[0], "ILI9341")) {
        DISPLAY_TYPE = ILI9341;
   // } else if(checkstring(argv[0], "ILI9341_I")) {
   // 	DISPLAY_TYPE = ILI9341_I;
    } else if(checkstring(argv[0], "ILI9481")) {
    	DISPLAY_TYPE = ILI9481;
    } else if(checkstring(argv[0], "ILI9481IPS")) {
        	DISPLAY_TYPE = ILI9481IPS;
   // } else if(checkstring(argv[0], "ILI9486")) {
   // 	DISPLAY_TYPE = ILI9486;
    } else if(checkstring(argv[0], "ILI9488")) {
    	DISPLAY_TYPE = ILI9488;
    } else if(checkstring(argv[0], "ST7789")) {
    	DISPLAY_TYPE = ST7789;
    } else if(checkstring(argv[0], "ST7735")) {
    	DISPLAY_TYPE = ST7735;
    } else if(checkstring(argv[0], "ST7735S")) {
       	DISPLAY_TYPE = ST7735S;
    } else if(checkstring(argv[0], "GC9A01")) {
    	DISPLAY_TYPE = GC9A01;
	} else
        error("Invalid display type");

    if(checkstring(argv[2], "L") || checkstring(argv[2], "LANDSCAPE"))
        Option.DISPLAY_ORIENTATION = LANDSCAPE;
    else if(checkstring(argv[2], "P") || checkstring(argv[2], "PORTRAIT"))
        Option.DISPLAY_ORIENTATION = PORTRAIT;
    else if(checkstring(argv[2], "RL") || checkstring(argv[2], "RLANDSCAPE"))
        Option.DISPLAY_ORIENTATION = RLANDSCAPE;
    else if(checkstring(argv[2], "RP") || checkstring(argv[2], "RPORTRAIT"))
        Option.DISPLAY_ORIENTATION = RPORTRAIT;
    else error("Orientation");
	if((code=codecheck(argv[4])))argv[4]+=2;
	p1 = getinteger(argv[4]);
	if(code)p1=codemap(code, p1);
	if((code=codecheck(argv[6])))argv[6]+=2;
	p2 = getinteger(argv[6]);
	if(code)p2=codemap(code, p2);
    CheckPin(p1, CP_IGNORE_INUSE);
    CheckPin(p2, CP_IGNORE_INUSE);
    //if(argc >= 9) {
   	if(argc>=9 && *argv[8]){
    	if((code=codecheck(argv[8])))argv[8]+=2;
    	p3 = getinteger(argv[8]);
    	if(code)p3=codemap(code, p3);
        CheckPin(p3, CP_IGNORE_INUSE);
        Option.LCD_CS = p3;
    }else{
        Option.LCD_CS = 0;
    }
   	if(argc == 11){
    	if(checkstring(argv[10],"INVERT"))LCDInvert=1;
    }else{
    	LCDInvert=0;

    }

    Option.LCD_CD = p1;
    Option.LCD_Reset = p2;
    Option.DISPLAY_TYPE = DISPLAY_TYPE;
    Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
    InitDisplaySPI(1);
}

// initialization commands and arguments are organized in these tables
// The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80  //Bit7 of the count indicates a delay is also added.

static const uint8_t
 ILI9341Init1[] = {                   // Initialization commands forILI9341 screens
	 8,                              // 11 commands in list:
     ILI9341_DISPLAYOFF,0,
	 ILI9341_POWERCONTROL1,1,0x23,
	 ILI9341_POWERCONTROL2,1,0x10,
	 ILI9341_VCOMCONTROL1,2,0x2B,0x2B,
	 ILI9341_VCOMCONTROL2,1,0xC0,
	 ILI9341_PIXELFORMAT,1,0x55,
	 ILI9341_FRAMECONTROL,2,0x00,0x1B,
	 ILI9341_ENTRYMODE,1,0x07,
	// ILI9341_SLEEPOUT,DELAY,50,	//uSec(50000);
	// ILI9341_NORMALDISP,0,
	// ILI9341_DISPLAYON,DELAY,100   //;uSec(100000);
     }; // 255 = 500 ms delay

static const uint8_t
ILI9341Init2[] = {                   // Initialization commands forILI9341 screens
	 3,                              // 11 commands in list:
     ILI9341_SLEEPOUT,DELAY,50,	//uSec(50000);
	 ILI9341_NORMALDISP,0,
	 ILI9341_DISPLAYON,DELAY,100   //;uSec(100000);
    };

static const uint8_t
  GC9A01Init[] = {                   // Initialization commands for GC9A01 screens
    46,                              // commands in list:

	0xEF, 0,                         //Inter reg enable 2
    0xEB, 1,14,                      //
	0xFE, 0,                         //Inter reg enable 1
	0xEF, 0,                         //Inter reg enable 2
	0x84,1,0x40,
	0x85,1,0xFF,
	0x86,1,0xFF,
	0x87,1,0xFF,
	0x88,1,0x0A,
	0x89,1,0x21,

	0x8A,1,0x00,
	0x8B,1,0x80,
	0x8C,1,0x01,
	0x8D,1,0x01,
	0x8E,1,0xFF,
	0x8F,1,0xFF,
	0xB6,2,0x00,0x00,                 //Display Function Control 0x00,0x02 ???  was 00,00
	0x3A,1,0x55,                      // Pixel Interface Format  was 66
	0xBD,1,0x06,
	0xBC,1,0x00,

	0xFF,3,0x60,0x01,0x04,   //????????????? 2 or 3
	0xC3,1,0x13,                       //power control 2
	0xC4,1,0x13,                       //power control 3
	0xC9,1,0x22,                       //power control 4
	0xBE,1,0x11,
	0xE1,2,0x10,0x0E,
	0xDF,3,0x21,0x0C,0x02,
	0xF0,6,0x45,0x09,0x08,0x08,0x26,0x2A,  //Gamma 1
	0xF1,6,0x43,0x70,0x72,0x36,0x37,0x6F,  //Gamma 2
	0xF2,6,0x45,0x09,0x08,0x08,0x26,0x2A,  //Gamma 3

	0xF3,6,0x43,0x70,0x72,0x36,0x37,0x6F,  //gamma 4
	0xED,2,0x1B,0x0B,
	0xAE,1,0x77,
	0xCD,1,0x63,
	0x70,9,0x07,0x07,0x04,0x0E,0x0F,0x09,0x07,0x08,0x03,
	0xE8,1,0x34,  //frame rate  0x34
	0x62,12,0x18,0x0D,0x71,0xED,0x70,0x70,0x18,0x0F,0x71,0xEF,0x70,0x70,
	0x63,12,0x18,0x11,0x71,0xF1,0x70,0x70,0x18,0x13,0x71,0xF3,0x70,0x70,
	0x64,7,0x28,0x29,0xF1,0x01,0xF1,0x01,0x07,
	0x66,10,0x3C,0x00,0xCD,0x67,0x45,0x45,0x10,0x00,0x00,0x00,

	0x67,10,0x00,0x3C,0x00,0x00,0x00,0x01,0x54,0x10,0x32,0x98,
	0x74,7,0x10,0x85,0x80,0x00,0x00,0x4E,0x00,
	0x98,2,0x3E,0x07,
	//0x35,0,               //Tearing Effect ON Not required
	0x21,0,               //Display Inversion ON
	0x11,DELAY,120,          //Sleep OUT + 120ms
	0x29,DELAY,20            //Display ON 20ms delay
    };                    // 255 = 500 ms delay

static const uint8_t
ILI9488InitA[] = {                        // Initialization commands for ILI9488 screens
	    13,                              // 13 commands in list:
        0xe0,15,0x00,0x03,0x09,0x08,0x16,0x0a,0x3f,0x78,0x4c,0x09,0x0a,0x08,0x16,0x1a,0x0f,  // positive Gamma Control
        0xe1,15,0x00,0x16,0x19,0x03,0x0f,0x05,0x32,0x45,0x46,0x04,0x0e,0x0d,0x35,0x37,0x0f,   // Negative Gamma Control
        0XC0,2,0x17,0x15,                // Power Control 1
        0xC1,1,0x41,                     // Power Control 2
        0xC5,3,0x00,0x12,0x80,           // VCOM Control
        0x36,1,0x48,                     // Memory Access Control  MX, BGR
        0x3A,1,0x66,                     // Pixel Interface Format // 18 bit colour for SPI
        0xB0,1,0x00,                     // Interface Mode Control
        0xB1,1,0xa0,                     // Frame Rate Control
        0xB4,1,0x02,                     // Display Inversion Control
        0xB6,3,0x02,0x02,0x3B,           // Display Function Control
        0xB7,1,0xc6,                     // Entry Mode Set
        0xF7,4,0xa9,0x51,0x2c,0x82,      // Adjust Control 3
};
static const uint8_t
ILI9488InitB[] = {                        // Initialization commands for ILI9488 screens
	    3,                              // 3 commands in list:
        ILI9341_NORMALDISP,0,
        0x11,DELAY,120,                  //uSec( 120000); //Exit Sleep
        0x29,DELAY,25                    //uSec(25000);  //Display on
};


static const uint8_t
ILI9481Init[] = {                  // Initialization commands for ILI9481 screens
	    9,                        // 11 commands in list:
        0x11,DELAY,20,             //uSec(20000);
        0xD0,3,0x07,0x42,0x18,
        0xD2,2,0x01,0x02,
        0xC0,5,0x10,0x3B,0x00,0x02,0x11,
        0xB3,4,0x00,0x00,0x00,0x10,
        0xC8,12,0x00,0x32,0x36,0x45,0x06,0x16,0x37,0x75,0x77,0x54,0x0C,0x00,
        0xE0,15,0x0f,0x24,0x1c,0x0a,0x0f,0x08,0x43,0x88,0x03,0x0f,0x10,0x06,0x0f,0x07,0x00,
        0xE1,15,0x0F,0x38,0x30,0x09,0x0f,0x0f,0x4e,0x77,0x3c,0x07,0x10,0x05,0x23,0x1b,0x00,
        0x36,1,0x0A,
        0x3A,DELAY+1,0x55,120, //uSec(120000);
        0x29,0
};

static const uint8_t
ILI9481IPSInit[] = {                                    // Initialization commands for ILI9481IPS screens
	    14,  // 14 commands in list:
		0x11,DELAY,255,                             //uSec( 280000);
		0xd0,DELAY+3,0x07,0x44,0x1f,255,
		0xd1,3,0x00,0x0c,0x1a,
		0xC5,1,0x03,
		0xd2,2,0x01,0x11,
		0xe4,1,0xa0,
		0xf3,2,0x00,0x2a,
		0xc8,12,0x00,0x26,0x21,0x00,0x00,0x1f,0x65,0x23,0x77,0x00,0x0f,0x00,
		0xc0,5,0x00,0x3b,0x00,0x02,0x11,
		0xC6,1,0x83,
		0xf0,1,0x01,
		0xe4,1,0xa0,
		0x3A,1,0x66,                                // Pixel Interface Format // 18 bit colour for SPI
		0x29,DELAY,255                              //uSec(500000);




};

/*
static const uint8_t
ILI9486Init[] = {                                    // Initialization commands for ILI9486 screens
	    14,                                          // 14 commands in list:
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

*/

static const uint8_t
  ST7789Init[] = {                                    // Initialization commands for ST7789 screens
	   /* 17,                                           // 17 commands in list:
        0xB1,3,0x01,0x2C,0x2d,                        //frame rate control - normal mode
   		0xB2,3,0x01,0x2C,0x2D,                        //frame rate control - idle mode
   		0xB3,6,0x01,0x2c,0x2D,0x01,0x2C,0x2D,         //frame rate control - partial mode
   		0xB4,1,0x07,                                  //display inversion control
   		0xC0,3,0xA2,0x02,0x84,                        //power control
   		0xC1,1,0xC5,                                  //power control
   		0xC2,2,0x0A,0x00,                             //power control
   		0xC3,2,0x8A,0x2A,                             //power control
   		0xC4,2,0x8A,0xEE,                             //power control
   		0xC5,1,0x0E,                                  //power control
   		0x21,0,                                       //invert display
   		0x3A,1,0x55,                                  //set color mode  //55
   		0xe0,16,0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2D,0x25,0x29,0x2B,0x39,0x00,0x01,0x03,0x10,
   		0xe1,16,0x03,0x1d,0x07,0x06,0x2E,0x2c,0x29,0x2d,0x2E,0x2E,0x37,0x3f,0x00,0x00,0x02,0x10,
   		ILI9341_SLEEPOUT,DELAY,50,	                 //uSec(50000);
   		ILI9341_NORMALDISP,0,
   		ILI9341_DISPLAYON,0
	*/
		  /*
		  cmd_st7789[] = {                  // Initialization commands for 7735B screens
		      9,                       // 9 commands in list:
		      ST7735_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
		        150,                     //    150 ms delay
		      ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
		        255,                    //     255 = 500 ms delay
		      ST7735_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
		        0x55,                   //     16-bit color
		        10,                     //     10 ms delay
		      ST7735_MADCTL , 1      ,  //  4: Memory access ctrl (directions), 1 arg:
		        0x08,                   //     Row addr/col addr, bottom to top refresh
		      ST7735_CASET  , 4      ,  //  5: Column addr set, 4 args, no delay:
		        0x00,
		        0x00,                   //     XSTART = 0
		        0x00,
		        240,                    //      XEND = 240
		      ST7735_RASET  , 4      ,  // 6: Row addr set, 4 args, no delay:
		        0x00,
		        0x00,                   //     YSTART = 0
		        320>>8,
		        320 & 0xFF,             //      YEND = 320
		      ST7735_INVON ,   DELAY,   // 7: hack
		        10,
		      ST7735_NORON  ,   DELAY,  // 8: Normal display on, no args, w/delay
		        10,                     //     10 ms delay
		      ST7735_DISPON ,   DELAY,  // 9: Main screen turn on, no args, w/delay
		      255 };                  //     255 = 500 ms delay
		*/
		//************* ST7789 1.3¯**********//

		16,
		0x36,1,0x00,      //MEMCONTROL
		0x3A,1,0x05,
	   	0xB2,5,0x0C,0x0C,0x00,0x33,0x33,
		0xB7,1,0x35,
	   	0xBB,1,0x19,
	   	0xC0,1,0x2C,
		0xC2,1,0x01,
	    0xC3,1,0x12,
	   	0xC4,1,0x20,
	   	0xC6,1,0x0F,
	   	0xD0,2,0xA4,0xA1,
	    0xE0,14,0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23,
	    0xE1,14,0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23,
	    0x21,0,           // display inversion ON
	    0x11,DELAY,120,  //Delay (120);
	    0x29,0

};




static const uint8_t
  ST7735Init[] = {                                   // Initialization commands for ST7735S screens
	  19,                                             // 17 commands in list
      ST7735_SLPOUT,DELAY,255,                        //out of sleep mode +uSec(500000);
	  ST7735_FRMCTR1,3,0x01,0x2C,0x2d,                //frame rate control - normal mode
	  ST7735_FRMCTR2,3,0x01,0x2C,0x2D,                //frame rate control - idle mode
	  ST7735_FRMCTR3,6,0x01,0x2c,0x2D,0x01,0x2C,0x2D, //frame rate control - partial mode
	  ST7735_INVCTR,1,0x07,                           //display inversion control
	  ST7735_PWCTR1,3,0xA2,0x02,0x84 ,                //power control
	  ST7735_PWCTR2,1,0xC5,                           //power control
	  ST7735_PWCTR3,2,0x0A,0x00 ,                     //power control
	  ST7735_PWCTR4,2,0x8A,0x2A,                      //power control
	  ST7735_PWCTR5,2,0x8A,0xEE,                      //power control

	  ST7735_VMCTR1,1,0x0E,                           //power control
	  ST7735_INVOFF,0,                                 //dont invert display
      ST7735_COLMOD,1,0x05,                           //set color mode
	  ST7735_CASET,4,0,0,0,0x7F,                      //column addr set  ??
	  ST7735_RASET,4,0,0,0,0x9F,                      //row addr set     ??
	  ST7735_GMCTRP1,16,0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2D,0x25,0x29,0x2B,0x39,0x00,0x01,0x03,0x10,
	  ST7735_GMCTRN1,16,0x03,0x1d,0x07,0x06,0x2E,0x2c,0x29,0x2d,0x2E,0x2E,0x37,0x3f,0x00,0x00,0x02,0x10,
	  ST7735_NORON,DELAY,10,                          //normal display on + uSec(10000);
	  ST7735_DISPON,0

};



// Companion code to the above tables.  Reads, decodes and issues
// the LCD initialisation commands using data from static constants above.
void static SendCommandBlock(const uint8_t *addr) {
   uint8_t numCommands, numArgs;
   uint16_t ms;
   numCommands = *(addr++);               // Number of commands to follow
   while(numCommands--) {                 // For each command...
	 spi_write_command(*(addr++));        //   Read, issue command
     numArgs  = *(addr++);                //   Number of args to follow
     ms       = numArgs & DELAY;          //   If hibit set, delay follows args
     numArgs &= ~DELAY;                   //   Mask out delay bit
     while(numArgs--) {                   //   For each argument...
       spi_write_data(*(addr++));         //   Read, issue argument
     }
     if(ms) {
       ms = *(addr++);                    // Read post-command delay time (ms)
       if(ms == 255) ms = 500;            // If 255, delay for 500 ms
         uSec(ms*1000);                   //convert to uS
     }
   }
}


// initialise the display controller
// this is used in the initial boot sequence of the Micromite
void MIPS16 InitDisplaySPI(int fullinit) {

	// Allow ILI9481,ILI9341,ILI9431_I,ST7735,ILI9488,ILI9486,ST7789,GA9A01
	if(Option.DISPLAY_TYPE < SPI_PANEL_START || Option.DISPLAY_TYPE > SPI_PANEL_END ) return;
    if(fullinit) {
        SetAndReserve(Option.LCD_CD, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
        SetAndReserve(Option.LCD_Reset, P_OUTPUT, 1, EXT_BOOT_RESERVED);                         // config reset as an output
        if(Option.LCD_CS) SetAndReserve(Option.LCD_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config chip select as an output

        // open the SPI port and reserve the I/O pins
        OpenSpiChannel();
        // LCDAttrib is used to record the SPI panel attributes for use in the later functions.
        // B0 indicates ReadBuffer support, B1 is RGB565 two byte writes, B2 and B3 unused at present
        // Is set to 0 here and relevant attributes set in the driver initialisations
        LCDAttrib=0;
    }

    // the parameters for the display panel are set here
    // the initialisation sequences and the SPI driver code was written by Peter Mather (matherp on The Back Shed forum)
    switch(Option.DISPLAY_TYPE) {
         case GC9A01:
        	  LCDAttrib=2;  //B0=ReadBuffer B1=RGB565
        	  DisplayHRes = 240;
        	  DisplayVRes = 240;
        	  ResetController();
        	  SendCommandBlock(GC9A01Init);  //send the block of commands

             break;

         case ILI9488:
        	   LCDAttrib=1;  //B0=ReadBuffer B1=RGB565
               DisplayHRes = 480;
               DisplayVRes = 320;
               ResetController();
               SendCommandBlock(ILI9488InitA);  //send the block of commands
               if(LCDInvert)spi_write_cd(ILI9341_INVERTON,1,0);  //INVERT
               SendCommandBlock(ILI9488InitB);  //send the block of commands

               break;
/*
         case ILI9486:
        	  LCDAttrib=1;  //B0=ReadBuffer B1=RGB565
              DisplayHRes = 480;
              DisplayVRes = 320;
              ResetController();
              SendCommandBlock(ILI9486Init);  //send the block of commands

              break;
*/
         case ILI9481:
        	   LCDAttrib=2;  //B0=ReadBuffer B1=RGB565
               DisplayHRes = 480;
               DisplayVRes = 320;
               ResetController();
               SendCommandBlock(ILI9481Init);  //send the block of commands

               break;

         case ILI9481IPS:
                 	    LCDAttrib=0;  //B0=ReadBuffer B1=RGB565
                        DisplayHRes = 480;
                        DisplayVRes = 320;
                        ResetController();
                        SendCommandBlock(ILI9481IPSInit);  //send the block of commands
                		switch(Option.DISPLAY_ORIENTATION) {
                            	case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9481_Landscape); break;
                            	case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1,ILI9481_Portrait); break;
                            	case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,ILI9481_Landscape180); break;
                            	case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9481_Portrait180); break;
                		}

                        break;

         case ILI9341:
           	   LCDAttrib=3;  //B0=ReadBuffer B1=RGB565
		       DisplayHRes = 320;
		       DisplayVRes = 240;
		       ResetController();
     	       SendCommandBlock(ILI9341Init1);  //send the block of commands
     	       if (LCDInvert)spi_write_cd(ILI9341_INVERTON,1,0);
     	       SendCommandBlock(ILI9341Init2);  //send the block of commands

		       break;

         case ST7735S:
         case ST7735:
        	  LCDAttrib=2;  //B0=ReadBuffer B1=RGB565
        	  DisplayHRes = 160;
        	  ResetController();
        	  SendCommandBlock(ST7735Init);  //send the block of commands
        	  if (Option.DISPLAY_TYPE==ST7735){
                 DisplayVRes = 128;
        	  }else{
        		DisplayVRes = 80;
        		spi_write_cd(ST7735_INVON,1,0);
        	  }

             // switch(Option.DISPLAY_ORIENTATION) {
             // 	  case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,ST7735_Landscape ); break;
             // 	  case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1,ST7735_Portrait ); break;
             // 	  case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,ST7735_Landscape180 ); break;
             // 	  case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,ST7735_Portrait180 ); break;
             // }
             // break;

             switch(Option.DISPLAY_ORIENTATION) {
                  case LANDSCAPE:     spi_write_cd(ST7735_MADCTL, 1, ST7735_Landscape | (Option.DISPLAY_TYPE==ST7735 ? 0 : 8)); break;
                  case PORTRAIT:      spi_write_cd(ST7735_MADCTL, 1, ST7735_Portrait | (Option.DISPLAY_TYPE==ST7735 ? 0 : 8)); break;
                  case RLANDSCAPE:    spi_write_cd(ST7735_MADCTL, 1, ST7735_Landscape180 | (Option.DISPLAY_TYPE==ST7735 ? 0 : 8)); break;
                  case RPORTRAIT:     spi_write_cd(ST7735_MADCTL, 1, ST7735_Portrait180 | (Option.DISPLAY_TYPE==ST7735 ? 0 : 8)); break;
             }
             break;



         case ST7789:
        	 LCDAttrib=2;  //B0=ReadBuffer B1=RGB565  //2
        	 DisplayHRes = 240;
        	 DisplayVRes = 240;
        	 ResetController();
        	 SendCommandBlock(ST7789Init);  //send the block of commands

        	 switch(Option.DISPLAY_ORIENTATION) {
   		          case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,0x62); break;      //60
   		          case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1, 0x02); break;     //00
   		          case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,0xa0); break;      //A0
   		          case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,0xc0); break;      //C0   70??
   		     }
        	 break;
     }


    if(Option.DISPLAY_TYPE!=ST7789 && Option.DISPLAY_TYPE!=ST7735 && Option.DISPLAY_TYPE!=ST7735S && Option.DISPLAY_TYPE!=ILI9481IPS ){
        switch(Option.DISPLAY_ORIENTATION) {
          case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape); break;     //28
          case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait); break;      //48
          case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape180); break;  //E8
          case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait180); break;   //88
       }
    }

    if(Option.DISPLAY_ORIENTATION & 1) {
        VRes=DisplayVRes;
        HRes=DisplayHRes;
    } else {
        VRes=DisplayHRes;
        HRes=DisplayVRes;
    }

    // setup the pointers to the drawing primitives
    DrawRectangle = DrawRectangleSPI;
    DrawBitmap = DrawBitmapSPI;
    DrawBuffer = DrawBufferSPI;
    //if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9486 || Option.DISPLAY_TYPE == ILI9488 )ReadBuffer = ReadBufferSPI;
    if(LCDAttrib & 0x1){
    	ReadBuffer = ReadBufferSPI;
    	//if((Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9488 ) && !Option.SerialConDisabled && Option.DISPLAY_CONSOLE){
    	// Option.NoScroll=1;
    	//  ScrollLCD = ScrollLCDSPI;
    	//}
    }
    //save the options
    SaveOptions();
    //reset display and clear screen
    ResetDisplay();
    ClearScreen(gui_bcolour);
}


// set Chip Select for the LCD low
// this also checks the configuration of the SPI channel and if required reconfigures it to suit the LCD controller
void set_cs(void) {
    SpiCsLow(Option.LCD_CS, LCD_SPI_SPEED);
}



void spi_write_data(unsigned char data){
    PinSetBit(Option.LCD_CD, LATSET);
    set_cs();
    if(Option.DISPLAY_TYPE == ILI9481)	{SPIsend2(data);}
    else {SPIsend(data);}
    SpiCsHigh(Option.LCD_CS);
}


void spi_write_command(unsigned char data){
    PinSetBit(Option.LCD_CD, LATCLR);
    set_cs();
    if(Option.DISPLAY_TYPE == ILI9481)	{SPIsend2(data);}
    else {SPIsend(data);}
    SpiCsHigh(Option.LCD_CS);
}


void spi_write_cd(unsigned char command, int data, ...){
   int i;
   va_list ap;
   va_start(ap, data);
   spi_write_command(command);
   for(i = 0; i < data; i++) spi_write_data((char)va_arg(ap, int));
   va_end(ap);
}



void MIPS16 ResetController(void){
//    PinSetBit(Option.LCD_Reset, LATSET);
    uSec(100000);
    PinSetBit(Option.LCD_Reset, LATCLR);
    uSec(100000);
    PinSetBit(Option.LCD_Reset, LATSET);
    uSec(200000);
    spi_write_command(ILI9341_SOFTRESET);                           //software reset
    uSec(200000);
}


void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    if(Option.DISPLAY_TYPE == ST7789){
        if(Option.DISPLAY_ORIENTATION==1){   //L
    	          xstart+=1;
    	          xend+=1;
    	}
    	if(Option.DISPLAY_ORIENTATION==2){  //P
    	           ystart+=1;
    	           yend+=1;
    	}

        if(Option.DISPLAY_ORIENTATION==3){ //RL
           xstart+=80;
           xend+=80;
        }
    	if(Option.DISPLAY_ORIENTATION==4){ //RP
              ystart+=80;
              yend+=80;
        }
    }

    if(Option.DISPLAY_TYPE==ST7735S){
    	if(Option.DISPLAY_ORIENTATION & 1){
    		ystart+=26;
    		yend+=26;
    		xstart++;
    		xend++;
    	} else {
    		xstart+=26;
    		xend+=26;
    		ystart++;
    		yend++;
    	}
    }



    if(Option.DISPLAY_TYPE == ILI9481 ){
    	if(rw) set_cs();
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend2(ILI9341_COLADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend2(xstart >> 8);
    	SPIsend2(xstart);
    	SPIsend2(xend >> 8);
    	SPIsend2(xend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend2(ILI9341_PAGEADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend2(ystart >> 8);
    	SPIsend2(ystart);
    	SPIsend2(yend >> 8);
    	SPIsend2(yend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	if(rw) {
    		SPIsend2(ILI9341_MEMORYWRITE);
    	} else {
    		SPIsend2(ILI9341_RAMRD);
    	}
    	PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    } else {
    	if(rw) set_cs();
    	//set_cs();
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend(ILI9341_COLADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend(xstart >> 8);
    	SPIsend(xstart);
    	SPIsend(xend >> 8);
    	SPIsend(xend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend(ILI9341_PAGEADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend(ystart >> 8);
    	SPIsend(ystart);
    	SPIsend(yend >> 8);
    	SPIsend(yend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	if(rw) {
    		SPIsend(ILI9341_MEMORYWRITE);
    	} else {
    		SPIsend(ILI9341_RAMRD);
    	}
    	PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    }
}

/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives
 all drawing on the LCD is done using either one of these two functions

 ****************************************************************************************************
****************************************************************************************************/


// Draw a filled rectangle
// this is the basic drawing primitive used by most drawing routines
//    x1, y1, x2, y2 - the coordinates
//    c - the colour
void DrawRectangleSPI(int x1, int y1, int x2, int y2, int c){
	int i,t;
    unsigned char col[2], *p;
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


    DefineRegionSPI(x1, y1, x2, y2, 1);
    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
	i = x2 - x1 + 1;
   //if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481 || Option.DISPLAY_TYPE == ST7789){
	if(LCDAttrib & 0x2){
	  // convert the colours to 565 format
	 col[0]= ((c >> 16) & 0b11111000) | ((c >> 13) & 0b00000111);
	 col[1] = ((c >>  5) & 0b11100000) | ((c >>  3) & 0b00011111);

	 i*=2;
	 p=GetMemory(i);
	 for(t=0;t<i;t+=2){p[t]=col[0];p[t+1]=col[1];}               //load 1 rows width of colour
	 for(t=y1;t<=y2;t++)	HAL_SPI_Transmit(&GenSPI,p,i,500);   //now output it for each row
	// for(t=y1;t<=y2;t++){
	// for (z=0;z<=i;z++){
	//    set_cs();
	// 	 SPIsend3(p[z]);
	//	  SpiCsHigh(Option.LCD_CS);
	//  }
	//  }
   }else{  //3 bytes per pixel
	 // convert the colours to 666 format
	 col[0]= ((c >> 16) & 0b11111000);
	 col[1] = ((c >>  8) & 0b11111100) ;
	 col[2] = (c & 0b11111000);
	 i*=3;
	 p=GetMemory(i);
	 for(t=0;t<i;t+=3){p[t]=col[0];p[t+1]=col[1];p[t+2]=col[2];}
	 for(t=y1;t<=y2;t++)	HAL_SPI_Transmit(&GenSPI,p,i,500);

   }
	FreeMemory(p);
    SpiCsHigh(Option.LCD_CS);                                       //set CS high
}


void ReadBufferSPI(int x1, int y1, int x2, int y2, char* p) {
    int r, N, t;
    unsigned char h,l;
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
    N=(x2- x1+1) * (y2- y1+1) * 3;
    spi_write_cd(ILI9341_PIXELFORMAT,1,0x66); //change to RGB666 for read
    //PinSetBit(Option.LCD_CS, LATCLR);
    set_cs();
    DefineRegionSPI(x1, y1, x2, y2, 0);    //DefineRegionSPI when rw=0 doesnt change CS so needs to be set.

    HAL_SPI_TransmitReceive(&GenSPI,&h,&l,1,500);  //Dummy Read
    r=0;
	HAL_SPI_Receive(&GenSPI,(uint8_t *)p,N,500);   //Now read the data

    PinSetBit(Option.LCD_CD, LATCLR);
    SpiCsHigh(Option.LCD_CS);                  //set CS high
    // restore RGB565 mode if required
   // if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481){
    if(LCDAttrib & 0x2){
      spi_write_cd(ILI9341_PIXELFORMAT,1,0x55); //change back to rdb565
    }
    // reading RGB666 need to convert to BGR565
    r=0;
    while(N) {
        h=(uint8_t)p[r+2];
        l=(uint8_t)p[r];
        p[r]=(int8_t)(h & 0xF8);
        p[r+2]=(int8_t)(l & 0xF8);
        p[r+1]=p[r+1] & 0xFC;
        r+=3;
        N-=3;
    }
}

void DrawBufferSPI(int x1, int y1, int x2, int y2, char* p) {
	volatile int i, j, k, t;
	unsigned char rgb565=0;
    int memory;
    volatile char *q;
    union colourmap
    {
    char rgbbytes[4];
    unsigned int rgb;
    } c;

    //if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481 || Option.DISPLAY_TYPE == ST7789)rgb565=1;
    if(LCDAttrib & 0x2)rgb565=1;
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


    DefineRegionSPI(x1, y1, x2, y2, 1);
    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
	i = x2 - x1 + 1;
	i *= (y2 - y1 + 1);
	if(rgb565){
	  i*=2;
	}else{
	 i*=3;
	}
	// switch to SPI enhanced mode for the bulk transfer
	memory=FreeSpaceOnHeap()/2;
	if((i) < memory){
		q=GetMemory(i);
		k=0;
	    for(j = (x2 - x1 + 1) * (y2 - y1 + 1); j > 0; j--){
	        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
	        c.rgbbytes[1]=*p++;
	        c.rgbbytes[2]=*p++;
	    // convert the colours to 565 format
	        if(rgb565){
	          q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
	          q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
	        }else{
	        	q[k++]= ((c.rgb >> 16) & 0b11111000);
	        	q[k++] = ((c.rgb >>  8) & 0b11111100) ;
	        	q[k++] = (c.rgb  & 0b11111000) ;
	        }
	    }
		HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,i,500);
		FreeMemory((void *)q);
	} else {
		int n;
		q=GetMemory(memory);
		n=memory;
		while(i){
			if(i>=n){
				k=0;
			    for(j = n/2; j > 0; j--){
			        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
			        c.rgbbytes[1]=*p++;
			        c.rgbbytes[2]=*p++;
			          // convert the colours to 565 format
			          if(rgb565){
			       	          q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
			       	          q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
			       	  }else{
			       	        	q[k++]= ((c.rgb >> 16) & 0b11111000);
			       	        	q[k++] = ((c.rgb >>  8) & 0b11111100) ;
			       	        	q[k++] = (c.rgb  & 0b11111000) ;
			          }
			    }
				HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,n,500);
				i-=n;
			} else {
				k=0;
			    for(j = i/2; j > 0; j--){
			        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
			        c.rgbbytes[1]=*p++;
			        c.rgbbytes[2]=*p++;
			          // convert the colours to 565 format
			        if(rgb565){
			       	       q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
			       	       q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
			        }else{
			       	       q[k++]= ((c.rgb >> 16) & 0b11111000);
			       	       q[k++] = ((c.rgb >>  8) & 0b11111100) ;
			       	       q[k++] = (c.rgb  & 0b11111000) ;
			        }
			    }
				HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,i,500);
				i=0;
			}

		}
		FreeMemory((void *)q);
		//PRet();


	}

    SpiCsHigh(Option.LCD_CS);                  //set CS high

    // revert to non enhanced SPI mode
}


//Print the bitmap of a char on the video output
//    x, y - the top left of the char
//    width, height - size of the char's bitmap
//    scale - how much to scale the bitmap
//	  fc, bc - foreground and background colour
//    bitmap - pointer to the bitmap
void DrawBitmapSPI(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, n;
    unsigned char rgb565=0;
    char f[3],b[3];
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    char *p=0;
    union colourmap {
    char rgbbytes[4];
    unsigned int rgb;
    } c;
   // if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481 || Option.DISPLAY_TYPE == ST7789)rgb565=1;
    if(LCDAttrib & 0x2)rgb565=1;
    if(( bc == -1) && !(LCDAttrib & 0x1))bc=0xFFFFFF;
    //if((bc == -1) && (Option.DISPLAY_TYPE == ILI9481 || Option.DISPLAY_TYPE == ST7789 ))bc = 0xFFFFFF;

    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    // adjust when part of the bitmap is outside the displayable coordinates
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= VRes) YEnd = VRes - 1;// the height of the bitmap will extend beyond the bottom margin
    if(bc == -1) {                                                     //special case of overlay text
    	i = 0;
        j = width * height * scale * scale * 3;
        p = GetMemory(j);                                              //allocate some temporary memory
        ReadBuffer(XStart, y1, XEnd, YEnd, p);
    }
    if(rgb565==1){
    // convert the colours to 565 format
        f[0]= ((fc >> 16) & 0b11111000) | ((fc >> 13) & 0b00000111);
        f[1] = ((fc >>  5) & 0b11100000) | ((fc >>  3) & 0b00011111);
        b[0] = ((bc >> 16) & 0b11111000) | ((bc >> 13) & 0b00000111);
        b[1] = ((bc >>  5) & 0b11100000) | ((bc >>  3) & 0b00011111);

    }else{
    	    f[0]= ((fc >> 16) & 0b11111000) ;
    	    f[1] = ((fc >>  8) & 0b11111100);
    	    f[2] = (fc & 0b11111000) ;
    	    b[0] = ((bc >> 16) & 0b11111000);
    	    b[1] = ((bc >> 8) & 0b11111100);
    	    b[2] = (bc & 0b11111000);
    }
    DefineRegionSPI(XStart, y1, XEnd, YEnd, 1);


    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
    n = 0;
       for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
        for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
            if(vertCoord++ < 0) continue;                           // we are above the top of the screen
            if(vertCoord > VRes) {                                  // we have extended beyond the bottom of the screen
                if(p != NULL) FreeMemory(p);
                return;
            }
            horizCoord = x1;
            for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                    if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                    if(horizCoord > HRes) continue;                 // we are beyond the right margin
                  if(rgb565==1){
                    if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        SPIqueue((uint8_t *)&f);
                    } else {
                        if(bc == -1){
                            c.rgbbytes[0] = p[n];
                            c.rgbbytes[1] = p[n+1];
                            c.rgbbytes[2] = p[n+2];
                            b[0] = ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
                            b[1] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
                        } 
                        SPIqueue((uint8_t *)&b);

                    }
                  }else{
                	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                	    SPIqueue3((uint8_t *)&f);
                	} else {
                	    if(bc == -1){
                	       c.rgbbytes[0] = p[n];
                	       c.rgbbytes[1] = p[n+1];
                	       c.rgbbytes[2] = p[n+2];
                	       b[0] = ((c.rgb >> 16) & 0b11111000);
                	       b[1] = ((c.rgb >> 8) & 0b11111100);
                	       b[2] = (c.rgb & 0b11111000);
                	    }
                	    SPIqueue3((uint8_t *)&b);
                	}
                  }
                    n += 3;
                }
            }
        }
    }

    SpiCsHigh(Option.LCD_CS);                                       //set CS high

    // revert to non enhanced SPI mode
    if(p != NULL) FreeMemory(p);

}


// the default function for DrawRectangle() and DrawBitmap()
void DisplayNotSet(void) {
    error("Display not configured");
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// These three functions allow the SPI port to be used by multiple drivers (LCD/touch/SD card)
// The BASIC use of the SPI port does NOT use these functions
// The MX170 uses SPI channel 1 which is shared by the BASIC program
// The MX470 uses SPI channel 2 which it has exclusive control of (needed because touch can be used at any time)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// config the SPI port for output
// it will not touch the port if it has already been opened
void MIPS16 OpenSpiChannel(void) {
	if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_BOOT_RESERVED) {
        ExtCfg(SPI2_OUT_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPI2_INP_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPI2_CLK_PIN, EXT_BOOT_RESERVED, 0);
        CurrentSPISpeed=NONE_SPI_SPEED;
    }
}

void SPISpeedSet(int speed){
	 if(CurrentSPISpeed != speed){
        HAL_SPI_DeInit(&GenSPI);
    	if(speed==LCD_SPI_SPEED){
    		CurrentSPISpeed=LCD_SPI_SPEED;
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase =  SPI_PHASE_1EDGE;
    		GenSPI.Init.BaudRatePrescaler = (Option.DISPLAY_TYPE==ILI9481 ? SPI_BAUDRATEPRESCALER_4 : SPI_BAUDRATEPRESCALER_2);  //21MHz or 42MHZ
    		if(Option.DISPLAY_TYPE==ILI9481IPS)GenSPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;

    	} else if(speed==SD_SLOW_SPI_SPEED){
    		CurrentSPISpeed=SD_SLOW_SPI_SPEED;
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    		GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
    	} else if(speed==SD_FAST_SPI_SPEED){
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    		CurrentSPISpeed=SD_FAST_SPI_SPEED;
    		GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_4;
    	} else if(speed==TOUCH_SPI_SPEED){
    		if(Option.DISPLAY_TYPE!=ILI9481){
    			GenSPI.Init.CLKPolarity = SPI_POLARITY_HIGH;
    			GenSPI.Init.CLKPhase = SPI_PHASE_2EDGE;
    			CurrentSPISpeed=TOUCH_SPI_SPEED;
    			GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
    		} else {
    			GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    			GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    			CurrentSPISpeed=TOUCH_SPI_SPEED;
    			GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_64;
    		}
    	}
   	  HAL_SPI_Init(&GenSPI);
    }

}
// set the chip select for the SPI to low (enabled)
// if the SPI is currently set to a different mode or baudrate this will change it accordingly
// also, it checks if the chip select pin needs to be changed
void SpiCsLow(int pin, int speed) {
	SPISpeedSet(speed);
    if(pin) PinSetBit(pin, LATCLR);                                 // set CS low
}


// set the chip select for SPI to high (disabled)
void SpiCsHigh(int pin) {
    if(pin) PinSetBit(pin, LATSET);                                 // set CS high
}
