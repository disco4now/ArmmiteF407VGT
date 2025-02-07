/*-*****************************************************************************

ArmmiteF4 MMBasic

MiscSTM32.c

Handles  a few miscellaneous functions for the STM32 versions.


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


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"

extern void SetBacklight(int intensity);
//extern void CallCFuncT5(void);                                      // this is implemented in CFunction.c
//extern unsigned int CFuncT5;                                        // we should call the CFunction T5 interrupt function if this is non zero
extern void ConfigSDCard(char *p);
volatile MMFLOAT VCC=3.3;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
#define GenSPI hspi2
extern SPI_HandleTypeDef GenSPI;
extern void initKeyboard(void);
extern void MIPS16 InitTouch(void);
extern int CurrentSPISpeed;
extern char LCDAttrib;
extern char LCDInvert;
extern char Feather;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim1;
extern void  setterminal(int height,int width);
extern SRAM_HandleTypeDef hsram1;


///////////////////////////////////////////////////////////////////////////////////////////////
// constants and functions used in the OPTION LIST command
char *LCDList[] = {"","VGA","SSD1963_5ER_16", "SSD1963_7ER_16",  //0-3
		"SSD1963_4_16", "SSD1963_5_16", "SSD1963_5A_16", "SSD1963_7_16", "SSD1963_7A_16", "SSD1963_8_16",  //4-9 SSD P16 displays
		"USER",//10
		"ST7735","","ST7735S","","ILI9481IPS","ILI9163", "GC9A01", "ST7789","ILI9488", "ILI9481", "ILI9341", "",      //11-22 SPI
		  "ILI9341_16", "ILI9486_16", "", "IPS_4_16", ""    //23-27 P16 displays
		 };
const char *OrientList[] = {"", "LANDSCAPE", "PORTRAIT", "RLANDSCAPE", "RPORTRAIT"};
const char *CaseList[] = {"", "LOWER", "UPPER"};
const char *KBrdList[] = {"", "US", "FR", "GR", "IT", "BE", "UK", "ES" };

void PRet(void){
	MMPrintString("\r\n");
}


void PO(char *s) {
    MMPrintString("OPTION "); MMPrintString(s); MMPrintString(" ");
}

void PInt(int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    MMPrintString(s);
}

void PIntComma(int n) {
    MMPrintString(", "); PInt(n);
}

void PO2Str(char *s1, const char *s2) {
    PO(s1); MMPrintString((char *)s2); PRet();
}


void PO2Int(char *s1, int n) {
    PO(s1); PInt(n); PRet();
}

void PO3Int(char *s1, int n1, int n2) {
    PO(s1); PInt(n1); PIntComma(n2); PRet();
}
void PIntH(unsigned int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 16);
    MMPrintString(s);
}
void PIntHC(unsigned int n) {
    MMPrintString(", "); PIntH(n);
}

void PFlt(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 4,4, ' ');
	    MMPrintString(s);
}
void PFltComma(MMFLOAT n) {
    MMPrintString(", "); PFlt(n);
}
void PPinName(int n){
	char s[3]="Px";
	int pn=0,pp=1;
	if(PinDef[n].sfr==GPIOA)s[1]='A';
	else if(PinDef[n].sfr==GPIOB)s[1]='B';
	else if(PinDef[n].sfr==GPIOC)s[1]='C';
	else if(PinDef[n].sfr==GPIOD)s[1]='D';
	else if(PinDef[n].sfr==GPIOE)s[1]='E';
	else if(PinDef[n].sfr==GPIOE)s[1]='F';
	else if(PinDef[n].sfr==GPIOE)s[1]='G';
	while(PinDef[n].bitnbr!=pp){pp<<=1;pn++;}
	if(s[1]!='x'){
		MMPrintString(s);
		PInt(pn);
	} else PInt(n);
}
void PPinNameComma(int n) {
    MMPrintString(", "); PPinName(n);

}
///////////////////////////////////////////////////////////////////////////////////////////////

void printoptions(void){
//	LoadOptions();
if(HAS_64PINS)	MMPrintString("\rARMmite F405RGT6 MMBasic Version " VERSION "\r\n");
if(HAS_144PINS)	MMPrintString("\rARMmite F407ZGT6 MMBasic Version " VERSION "\r\n");
if(HAS_100PINS) MMPrintString("\rARMmite F407VGT6 MMBasic Version " VERSION "\r\n");


    if(Option.Autorun == true) PO2Str("AUTORUN", "ON");
    if(Option.Baudrate != CONSOLE_BAUDRATE) PO2Int("BAUDRATE", Option.Baudrate);
    if(Option.Restart > 0) PO2Int("RESTART", Option.Restart);
   // if(Option.Invert == 2) PO2Str("CONSOLE", "AUTO");
    if(Option.ColourCode == true) PO2Str("COLOURCODE", "ON");
    if(Option.Listcase != CONFIG_TITLE) PO2Str("CASE", CaseList[(int)Option.Listcase]);
    if(Option.Tab != 2) PO2Int("TAB", Option.Tab);
    if(Option.Height != 24 || Option.Width != 80) PO3Int("DISPLAY", Option.Height, Option.Width);

    if(Option.DISPLAY_TYPE >= SPI_PANEL_START && Option.DISPLAY_TYPE <=  SPI_PANEL_END){
   	    PO("LCDPANEL"); MMPrintString((char *)LCDList[(int)Option.DISPLAY_TYPE]); MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
        PPinNameComma(Option.LCD_CD); PPinNameComma(Option.LCD_Reset); PPinNameComma(Option.LCD_CS);
        if(LCDInvert){MMputchar(',');MMPrintString((char *)"INVERT");}
        PRet();
    }
    //if(Option.DISPLAY_TYPE >= SSD_PANEL_START && Option.DISPLAY_TYPE <= SSD_PANEL_END) {
   //     PO("LCDPANEL"); MMPrintString(LCDList[(int)Option.DISPLAY_TYPE]); MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
   //     PRet();
   // }
    if((Option.DISPLAY_TYPE >= SSD_PANEL_START && Option.DISPLAY_TYPE <= SSD_PANEL_END) || (Option.DISPLAY_TYPE>= P16_PANEL_START && Option.DISPLAY_TYPE <= P16_PANEL_END)){
        PO("LCDPANEL"); MMPrintString(LCDList[(int)Option.DISPLAY_TYPE]); MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
        PRet();
    }
    if(Option.DISPLAY_TYPE == USER ){
    	PO("LCDPANEL"); MMPrintString(LCDList[(int)Option.DISPLAY_TYPE]);
        PIntComma(HRes);
        PIntComma(VRes);
        PRet();
    }
    if(Option.TOUCH_CS) {
        PO("TOUCH"); PPinName(Option.TOUCH_CS); PPinNameComma(Option.TOUCH_IRQ);
        if(Option.TOUCH_Click) PPinNameComma(Option.TOUCH_Click);
        PRet();
        if(Option.TOUCH_XZERO != TOUCH_NOT_CALIBRATED ) {
            MMPrintString("GUI CALIBRATE "); PInt(Option.TOUCH_SWAPXY); PIntComma(Option.TOUCH_XZERO); PIntComma(Option.TOUCH_YZERO);
           PIntComma(Option.TOUCH_XSCALE * 10000); PIntComma(Option.TOUCH_YSCALE * 10000); PRet();
        }
    }
   // if(Option.DefaultBrightness!=100){
    if(HRes != 0){
    	MMPrintString("BACKLIGHT ");
    	if(Option.DefaultBrightness>100){
    		PInt(Option.DefaultBrightness-101);MMPrintString(",REVERSE \r\n");
    	}else{
    		PInt(Option.DefaultBrightness);;MMPrintString(",DEFAULT \r\n");
    	}
    }

    if(Option.DISPLAY_CONSOLE == true){
    	//PO2Str("LCDPANEL", "CONSOLE");
       // if(Option.DefaultFont != (Option.DISPLAY_TYPE==COLOURVGA? (6<<4) | 1 : 0x01 ))PInt((Option.DefaultFont>>4) +1);
    	PO("LCDPANEL CONSOLE");
        if(Option.DefaultFont != 0x01 )PInt(Option.DefaultFont);
        else if(!(Option.DefaultFC==WHITE && Option.DefaultBC==BLACK && Option.DefaultBrightness == 50 && Option.NoScroll==0))MMputchar(',');
        if(Option.DefaultFC!=WHITE)PIntHC(Option.DefaultFC);
        else if(!(Option.DefaultBC==BLACK && Option.DefaultBrightness == 50 && Option.NoScroll==0))MMputchar(',');
        if(Option.DefaultBC!=BLACK)PIntHC(Option.DefaultBC);
        else if(!(Option.DefaultBrightness == 50 && Option.NoScroll==0))MMputchar(',');
        if(Option.DefaultBrightness != 50)PIntComma(Option.DefaultBrightness>100 ? Option.DefaultBrightness-101 : Option.DefaultBrightness );
        else if(!(Option.DefaultBrightness == 50 && Option.NoScroll==0))MMputchar(',');
        if(Option.NoScroll!=0)MMPrintString(",NOSCROLL");
        PRet();

    }
    if(Option.MaxCtrls != 201) PO2Int("CONTROLS", Option.MaxCtrls - 1);
    if(!Option.SerialPullup) PO2Str("SERIAL PULLUPS", "OFF");
    if(!Option.SerialConDisabled) PO2Str("SERIAL CONSOLE", "ON");
    if(Option.KeyboardConfig != NO_KEYBOARD) PO2Str("KEYBOARD", KBrdList[(int)Option.KeyboardConfig]);
    if(Option.RTC_Calibrate)PO2Int("RTC CALIBRATE", Option.RTC_Calibrate);
    if(Option.ProgFlashSize !=0x0){PO("PROG_FLASH_SIZE");PIntH( Option.ProgFlashSize); PRet();}  //LIBRARY
    if(Option.FLASH_CS != 35){ PO("FLASH_CS"); PInt(Option.FLASH_CS);PRet();}
    if(Option.DefaultFont != 1){ PO("DefaultFont"); PInt(Option.DefaultFont);PRet();}
   // PO3Int("DISPLAY", Option.Height, Option.Width);
    if(*Option.F1key)PO2Str("F1",(char *)Option.F1key);
    if(*Option.F5key)PO2Str("F5",(char *)Option.F5key);
    if(*Option.F6key)PO2Str("F6",(char *)Option.F6key);
    if(*Option.F7key)PO2Str("F7",(char *)Option.F7key);
    if(*Option.F8key)PO2Str("F8",(char *)Option.F8key);
    if(*Option.F9key)PO2Str("F9",(char *)Option.F9key);
    PIntH(Option.magic);PRet();
    return;

}
void touchdisable(void){
	// if(!(Option.DISPLAY_TYPE==ILI9341 || Option.DISPLAY_TYPE==ILI9481)){
    if(!(Option.DISPLAY_TYPE >= SPI_PANEL_START && Option.DISPLAY_TYPE <= SPI_PANEL_END)){
        HAL_SPI_DeInit(&GenSPI);
		ExtCurrentConfig[SPI2_OUT_PIN]=EXT_DIG_IN;
		ExtCurrentConfig[SPI2_INP_PIN]=EXT_DIG_IN;
		ExtCurrentConfig[SPI2_CLK_PIN]=EXT_DIG_IN;
        ExtCfg(SPI2_OUT_PIN, EXT_NOT_CONFIG, 0);
        ExtCfg(SPI2_INP_PIN, EXT_NOT_CONFIG, 0);
        ExtCfg(SPI2_CLK_PIN, EXT_NOT_CONFIG, 0);
        CurrentSPISpeed=NONE_SPI_SPEED;
    }
	if(Option.TOUCH_CS)ExtCurrentConfig[Option.TOUCH_CS]=EXT_DIG_IN;
	if(Option.TOUCH_IRQ)ExtCurrentConfig[Option.TOUCH_IRQ]=EXT_DIG_IN;
	if(Option.TOUCH_CS)ExtCfg(Option.TOUCH_CS, EXT_NOT_CONFIG, 0);
	if(Option.TOUCH_IRQ)ExtCfg(Option.TOUCH_IRQ, EXT_NOT_CONFIG, 0);
    if(Option.TOUCH_Click!=false){
		ExtCurrentConfig[Option.TOUCH_Click]=EXT_DIG_IN;
        ExtCfg(Option.TOUCH_Click, EXT_NOT_CONFIG, 0);
    }
    Option.TOUCH_Click = Option.TOUCH_CS = Option.TOUCH_IRQ = false;
}



void MIPS16 OtherOptions(void) {
	char *tp, *ttp;

	tp = checkstring(cmdline, "RESET");
	if(tp) {
        ResetAllOptions();
		goto saveandreset;
	}
#ifndef RGT
    tp = checkstring(cmdline, "KEYBOARD");
	if(tp) {
    	//if(CurrentLinePtr) error("Invalid in a program");
		if(checkstring(tp, "DISABLE")){
			Option.KeyboardConfig = NO_KEYBOARD;
			HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
			ExtCurrentConfig[KEYBOARD_CLOCK]=EXT_DIG_IN;
			ExtCurrentConfig[KEYBOARD_DATA]=EXT_DIG_IN;
	        ExtCfg(KEYBOARD_CLOCK, EXT_NOT_CONFIG, 0);
	        ExtCfg(KEYBOARD_DATA, EXT_NOT_CONFIG, 0);
	        SaveOptions();
	        return;
		}
        else if(checkstring(tp, "US"))	Option.KeyboardConfig = CONFIG_US;
		else if(checkstring(tp, "FR"))	Option.KeyboardConfig = CONFIG_FR;
		else if(checkstring(tp, "GR"))	Option.KeyboardConfig = CONFIG_GR;
		else if(checkstring(tp, "IT"))	Option.KeyboardConfig = CONFIG_IT;
		else if(checkstring(tp, "BE"))	Option.KeyboardConfig = CONFIG_BE;
		else if(checkstring(tp, "UK"))	Option.KeyboardConfig = CONFIG_UK;
		else if(checkstring(tp, "ES"))	Option.KeyboardConfig = CONFIG_ES;
		SaveOptions();
        initKeyboard();
        return;
	}
#endif

    tp = checkstring(cmdline, "SERIAL PULLUP");
	if(tp) {
        if(checkstring(tp, "DISABLE"))
            Option.SerialPullup = 0;
        else if(checkstring(tp, "ENABLE"))
            Option.SerialPullup = 1;
        else error("Invalid Command");
        goto saveandreset;
	}
	tp=checkstring(cmdline, "RTC CALIBRATE");
	if(tp){
		Option.RTC_Calibrate=getint(tp,-511,512);
		int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
		int calibrate= -Option.RTC_Calibrate;
		if(Option.RTC_Calibrate>0){
			up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
			calibrate=512-Option.RTC_Calibrate;
		}
		HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);
		SaveOptions();
		return;
	}

	tp = checkstring(cmdline, "CONTROLS");
    if(tp) {
    	Option.MaxCtrls = getint(tp, 0, 1000) + 1;
        goto saveandreset;
    }

    tp = checkstring(cmdline, "ERROR");
	if(tp) {
		if(checkstring(tp, "CONTINUE")) {
            OptionFileErrorAbort = false;
            return;
        }
		if(checkstring(tp, "ABORT")) {
            OptionFileErrorAbort = true;
            return;
        }
	}

    tp = checkstring(cmdline, "LCDPANEL");
    if(tp) {
       //if(CurrentLinePtr) error("Invalid in a program");      G.A.
    	int i;
        if((ttp = checkstring(tp, "CONSOLE"))) {
            if(HRes == 0) error("LCD Panel not configured");
            if(ScrollLCD == (void (*)(int ))DisplayNotSet) error("No Support on this LCD Panel");
            if(!DISPLAY_LANDSCAPE) error("Landscape only");
            Option.NoScroll = 0;
           // if(!(Option.DISPLAY_TYPE==IPS_4_16))Option.NoScroll=1;
            Option.Height = VRes/gui_font_height; Option.Width = HRes/gui_font_width;
            skipspace(ttp);
            Option.DefaultFC = WHITE;
            Option.DefaultBC = BLACK;
           // Option.DefaultBrightness = 100;
            if(!(*ttp == 0 || *ttp == '\'')) {
                getargs(&ttp, 9, ",");                              // this is a macro and must be the first executable stmt in a block
                if(argc > 0) {
                    if(*argv[0] == '#') argv[0]++;                  // skip the hash if used
                   // MMPrintString("SetFont() ");PIntH((int)((getint(argv[0], 1, FONT_BUILTIN_NBR) - 1) << 4) | 1);PRet();
                    SetFont((((getint(argv[0], 1, FONT_BUILTIN_NBR) - 1) << 4) | 1));
                    Option.DefaultFont = (gui_font>>4)+1;
                }
                if(argc > 2 && *argv[2]) Option.DefaultFC = getint(argv[2], BLACK, WHITE);
                if(argc > 4 && *argv[4]) Option.DefaultBC = getint(argv[4], BLACK, WHITE);
                if(Option.DefaultFC == Option.DefaultBC) error("Same colours");
                if(argc > 6 && *argv[6]) {
                	if(Option.DefaultBrightness>100){
                		Option.DefaultBrightness = 101+getint(argv[6], 0, 100);
                	}else{
                		Option.DefaultBrightness = 101+getint(argv[6], 0, 100);
                	}
                	SetBacklight(Option.DefaultBrightness);
                }
                if(argc==9){
                    if(checkstring(argv[8],"NOSCROLL")){
                        if(!(Option.DISPLAY_TYPE >= SSD_PANEL_START && Option.DISPLAY_TYPE <= SSD_PANEL_END))Option.NoScroll=1;
                        else error("Invalid for this display");
                    } else error("Syntax ???");
                }
            } else {
                SetFont((Option.DefaultFont-1)<<4 | 0x1);   //B7-B4 is fontno.-1 , B3-B0 is scale
            }

            Option.DISPLAY_CONSOLE = 1;
           // Option.ColourCode = true;  //On by default so done change it
            SaveOptions();
            //PromptFont = Option.DefaultFont;
            PromptFont = gui_font;
            PromptFC = Option.DefaultFC;
            PromptBC = Option.DefaultBC;
            ClearScreen(Option.DefaultBC);
            //Only setterminal if CONSOLE is bigger than 80*24
            if  (Option.Width > SCREENWIDTH || Option.Height > SCREENHEIGHT){
                 setterminal((Option.Height > SCREENHEIGHT)?Option.Height:SCREENHEIGHT,(Option.Width > SCREENWIDTH)?Option.Width:SCREENWIDTH);                                                    // or height is > 24
            }
            return;
        }
        else if(checkstring(tp, "NOCONSOLE")) {
            Option.Height = SCREENHEIGHT; Option.Width = SCREENWIDTH;
            Option.DISPLAY_CONSOLE = 0;
            Option.NoScroll=0;
           // Option.ColourCode = false;   //Don't change as on by default
            Option.DefaultFC = WHITE;
            Option.DefaultBC = BLACK;
            //SetFont((Option.DefaultFont = 0x01));
            Option.DefaultFont=0x01;
            SetFont(((Option.DefaultFont-1)<<4) | 1);
            // Option.DefaultBrightness = 100;
            setterminal(Option.Height,Option.Width);
            SaveOptions();
            ClearScreen(Option.DefaultBC);
            return;
        }
        else if(checkstring(tp, "DISABLE")) {
        	  if(Option.DISPLAY_TYPE >= SPI_PANEL_START && Option.DISPLAY_TYPE<=SPI_PANEL_END){


        		            if(Option.LCD_CS){
        	            	  ExtCurrentConfig[Option.LCD_CS]=EXT_DIG_IN;
        	            	  ExtCfg(Option.LCD_CS, EXT_NOT_CONFIG, 0);
        	            	}
        	            	ExtCurrentConfig[Option.LCD_CD]=EXT_DIG_IN;
        	            	ExtCfg(Option.LCD_CD, EXT_NOT_CONFIG, 0);
        	            	ExtCurrentConfig[Option.LCD_Reset]=EXT_DIG_IN;
        	            	ExtCfg(Option.LCD_Reset, EXT_NOT_CONFIG, 0);
        	            	Option.LCD_CS = Option.LCD_CD  = Option.LCD_Reset  = 0;

        	            	SaveOptions();

        	 }else{
        	     /* release the parallel pins and stop the FSMC */
        		 if (HAS_100PINS){
        		   for(i=38;i<47;i++){ExtCurrentConfig[i]=EXT_DIG_IN;ExtCfg(i, EXT_NOT_CONFIG, 0);}
        		   for(i=55;i<58;i++){ExtCurrentConfig[i]=EXT_DIG_IN;ExtCfg(i, EXT_NOT_CONFIG, 0);}
        		   for(i=60;i<63;i++){ExtCurrentConfig[i]=EXT_DIG_IN;ExtCfg(i, EXT_NOT_CONFIG, 0);}
        		   ExtCurrentConfig[81]=EXT_DIG_IN;ExtCfg(81, EXT_NOT_CONFIG, 0);
        		   ExtCurrentConfig[82]=EXT_DIG_IN;ExtCfg(82, EXT_NOT_CONFIG, 0);
        		   ExtCurrentConfig[85]=EXT_DIG_IN;ExtCfg(85, EXT_NOT_CONFIG, 0);
        		   ExtCurrentConfig[86]=EXT_DIG_IN;ExtCfg(86, EXT_NOT_CONFIG, 0);
        		   ExtCurrentConfig[88]=EXT_DIG_IN;ExtCfg(88, EXT_NOT_CONFIG, 0);
        		 }else{
          		   ExtCurrentConfig[58]=EXT_DIG_IN;ExtCfg(58, EXT_NOT_CONFIG, 0);  //B4
          		   ExtCurrentConfig[59]=EXT_DIG_IN;ExtCfg(59, EXT_NOT_CONFIG, 0);  //B5
          		   ExtCurrentConfig[60]=EXT_DIG_IN;ExtCfg(60, EXT_NOT_CONFIG, 0);  //B6
          		   ExtCurrentConfig[63]=EXT_DIG_IN;ExtCfg(63, EXT_NOT_CONFIG, 0);  //B7
          		   ExtCurrentConfig[64]=EXT_DIG_IN;ExtCfg(64, EXT_NOT_CONFIG, 0);  //B8

        		   ExtCurrentConfig[65]=EXT_DIG_IN;ExtCfg(65, EXT_NOT_CONFIG, 0);  //B9
        		   ExtCurrentConfig[66]=EXT_DIG_IN;ExtCfg(66, EXT_NOT_CONFIG, 0);  //B10
        		   ExtCurrentConfig[67]=EXT_DIG_IN;ExtCfg(67, EXT_NOT_CONFIG, 0);  //B11
        		   ExtCurrentConfig[68]=EXT_DIG_IN;ExtCfg(68, EXT_NOT_CONFIG, 0);  //B12
        		   ExtCurrentConfig[77]=EXT_DIG_IN;ExtCfg(77, EXT_NOT_CONFIG, 0);  //B13

        		   ExtCurrentConfig[78]=EXT_DIG_IN;ExtCfg(78, EXT_NOT_CONFIG, 0);  //B14
        		   ExtCurrentConfig[79]=EXT_DIG_IN;ExtCfg(79, EXT_NOT_CONFIG, 0);  //B15
        		   ExtCurrentConfig[50]=EXT_DIG_IN;ExtCfg(50, EXT_NOT_CONFIG, 0);  //DC/RS  FSMC_A18 Was 82 now 50
        		   ExtCurrentConfig[85]=EXT_DIG_IN;ExtCfg(85, EXT_NOT_CONFIG, 0);  //B0
        		   ExtCurrentConfig[86]=EXT_DIG_IN;ExtCfg(86, EXT_NOT_CONFIG, 0);  //B1

        		   ExtCurrentConfig[114]=EXT_DIG_IN;ExtCfg(114, EXT_NOT_CONFIG, 0); //B2
        		   ExtCurrentConfig[115]=EXT_DIG_IN;ExtCfg(115, EXT_NOT_CONFIG, 0); //B3
        		   ExtCurrentConfig[118]=EXT_DIG_IN;ExtCfg(118, EXT_NOT_CONFIG, 0); //RD   FSMC_NOE
        		   ExtCurrentConfig[119]=EXT_DIG_IN;ExtCfg(119, EXT_NOT_CONFIG, 0); //WR   FSMC_NWE
        		   ExtCurrentConfig[127]=EXT_DIG_IN;ExtCfg(127, EXT_NOT_CONFIG, 0); //CS   FSMC_NE4 Was 123 now 127

        		 }
        	     HAL_SRAM_DeInit(&hsram1);
        	 }

        	Option.NoScroll=0;
        	touchdisable();
        	SaveOptions();
            if(Option.DISPLAY_CONSOLE){
                Option.Height = SCREENHEIGHT;
                Option.Width = SCREENWIDTH;
                setterminal(Option.Height,Option.Width);
            }
            Option.DISPLAY_CONSOLE = Option.DISPLAY_TYPE = Option.DISPLAY_ORIENTATION = Option.SSDspeed = LCDAttrib = LCDInvert = HRes = 0;
            Option.DefaultFC = WHITE; Option.DefaultBC = BLACK; Option.DefaultFont = 0x01;// Option.DefaultBrightness = 100;
            DrawRectangle = (void (*)(int , int , int , int , int )) DisplayNotSet;
            DrawBitmap =  (void (*)(int , int , int , int , int , int , int , unsigned char *)) DisplayNotSet;
            ScrollLCD = (void (*)(int ))DisplayNotSet;
            DrawBuffer = (void (*)(int , int , int , int , char * )) DisplayNotSet;
            ReadBuffer = (void (*)(int , int , int , int , char * )) DisplayNotSet;
        }
        else {
            if(Option.DISPLAY_TYPE) error("Display already configured");
            if(!Option.DISPLAY_TYPE)ConfigDisplaySSD(tp);
            if(!Option.DISPLAY_TYPE) ConfigDisplaySPI(tp);          // if it is not an SSD1963 then try for a SPI type
        }
        SaveOptions();
        return;
    }
    tp = checkstring(cmdline, "TOUCH");
    if(tp) {
    	//if(CurrentLinePtr) error("Invalid in a program");        //G.A.
		if(checkstring(tp, "DISABLE")) {
			touchdisable();
		} else {
            ConfigTouch(tp);
			InitTouch();
        }
        SaveOptions();
        return;
    }
    tp = checkstring(cmdline, "SERIAL CONSOLE");
    if(Feather)error("Serial Console not available on Adfruit Feather");
	if(tp) {
		if(checkstring(tp, "OFF"))		    {
			if(!CurrentLinePtr)MMPrintString("Reset the processor to connect with the USB/CDC port\r\n");
	    	while(ConsoleTxBufTail != ConsoleTxBufHead){}
			Option.SerialConDisabled = true;
			if(!CurrentLinePtr)SaveOptions();    // G.A.
	        return;
		}
		if(checkstring(tp, "ON"))           {
			if(!CurrentLinePtr)MMPrintString("Reset the processor to connect with the serial port J6\r\n");
	    	while(ConsoleTxBufTail != ConsoleTxBufHead){}
			Option.SerialConDisabled = false;
			if(!CurrentLinePtr)SaveOptions();    // G.A.
	        return;
		}
	}
	tp = checkstring(cmdline, "LIST");
    if(tp) {
    	printoptions();
    	return;
    }

    tp = checkstring(cmdline, "FLASH_CS");
           int i;
           if(tp) {
           	i=getint(tp,0,100);
           	if (HAS_64PINS){
           		if (i!=50 && i!=0) error("Only 0 or 50 are valid");
           	}else{
           		if (i!=35 && i!=77 && i!=0) error("Only 0,35 or 77 are valid");
           	}

           	Option.FLASH_CS = i ;
           	goto saveandreset;

    }

    tp = checkstring(cmdline, "RESTART");
    if(tp) {
         	Option.Restart = getint(tp, 0, 2) ;
         	SaveOptions();
         	return;
    }

	error("Unrecognised option");

saveandreset:
    // used for options that require the cpu to be reset
   // dont' reset if called in a program G.A.
  if(!CurrentLinePtr) {
    SaveOptions();
    _excep_code = RESTART_NOAUTORUN;                            // otherwise do an automatic reset
	while(ConsoleTxBufTail != ConsoleTxBufHead);
	uSec(10000);
    SoftReset();                                                // this will restart the processor
  }
}





