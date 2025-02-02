/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Flash.h

Include file that contains the globals and defines for flash save/load in MMBasic.


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
#include "stdint.h"
#define FLASH_BASE_ADDR      (uint32_t)(FLASH_BASE)
#define FLASH_END_ADDR       (uint32_t)(0x08200000)

		#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
		#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
		#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
		#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
		#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
		#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
		#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
		//#define FLASH_PROGRAM_ADDR       ADDR_FLASH_SECTOR_7   /* Start Basic Program flash area */
        #define FLASH_PROGRAM_ADDR       ADDR_FLASH_SECTOR_10   /* Start Basic Program flash area */
        #define FLASH_LIBRARY_ADDR       ADDR_FLASH_SECTOR_11   /* Start Basic Library flash area */
		#define FLASH_SAVED_OPTION_ADDR  ADDR_FLASH_SECTOR_1   /* Start of Saved Options flash area */
		#define FLASH_SAVED_VAR_ADDR     ADDR_FLASH_SECTOR_2   /* Start of Saved Variables flash area */
		//#define SAVEDVARS_FLASH_SIZE 16384  // amount of flash reserved for saved variables 16K
        #define SAVEDVARS_FLASH_SIZE 0x1000  // amount of flash reserved for saved variables  4K for now
	#define SAVED_VAR_RAM_ADDR     ((uint32_t)0x40024000)   /* Start of Saved Variables flash area */
//	#define FLASH_PROGRAM_ADDR       ADDR_FLASH_SECTOR_0_BANK2   /* Start Basic Program flash area */
	#define SAVED_VAR_RAM_SIZE 0x1000  // amount of flash reserved for saved variables
    #define RTC_RAM_ADDR     ((uint32_t)0x40024000)   /* Start of RTC ram area */
    #define RTC_RAM_SIZE 0x1000  // amount of  RTC ram

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE) && !defined(FLASH_INCLUDED)
#define FLASH_INCLUDED

    // IMPORTANT: Change the string constant in cmd_memory() if you change PROG_FLASH_SIZE
#define EDIT_BUFFER_SIZE  ((unsigned int)(RAMEND - (unsigned int)RAMBase - 1024))  // this is the maximum RAM that we can get
//#define SAVED_OPTIONS_FLASH 2
//#define SAVED_VARS_FLASH 2
#define PROGRAM_FLASH 1
#define SAVED_VARS_FLASH 2
#define LIBRARY_FLASH 3
#define SAVED_OPTIONS_FLASH 4

struct option_s {
    char Autorun;
    char Tab;
    char Restart;
    char Listcase;
    char Height;
    char Width;
    char  ColourCode;
    char DISPLAY_TYPE;
    char DISPLAY_ORIENTATION;
    unsigned char TOUCH_CS;
    unsigned char TOUCH_IRQ;
    char TOUCH_SWAPXY;
    // for the SPI LCDs
    unsigned char LCD_CD;
    unsigned char LCD_CS;
    unsigned char LCD_Reset;
    char SerialConDisabled;
    char SSDspeed;
    char DISPLAY_CONSOLE;
    char DefaultFont;
    char KeyboardConfig;
    unsigned char TOUCH_Click;
    char DefaultBrightness;         // default backlight brightness
    char SerialPullup;
    char fulltime;
    char Refresh;
    unsigned char FLASH_CS;
    unsigned char NoScroll;         //NoScroll from picomites added @beta3
    unsigned char dummy;        //27
    short MaxCtrls;       //28        2  // maximum number of controls allowed
    short RTC_Calibrate;  //32  (30)bytes  2
    int DISPLAY_WIDTH;    //36  (32)      4
    int DISPLAY_HEIGHT;   //40  (36)     4
    uint32_t  PIN;        //44  (40)      4
    uint32_t  Baudrate;   //48  (44)      4
    MMFLOAT TOUCH_XSCALE; //56  (48)     8
    MMFLOAT TOUCH_YSCALE; //64  (56)     8
    unsigned int ProgFlashSize;    // 64  4 used to store the size of the program flash (also start of the LIBRARY code)
    int DefaultFC, DefaultBC;      // 68  4  the default colours
    short  TOUCH_XZERO;            // 72  2
    short  TOUCH_YZERO;            // 74  2
    uint32_t magic;               // 76 4 =80  i.e 4 bytes
    unsigned char F1key[MAXKEYLEN]; //80+64 =144
    unsigned char F5key[MAXKEYLEN]; //144+64=208
    unsigned char F6key[MAXKEYLEN]; //206+64=270
    unsigned char F7key[MAXKEYLEN]; //270+64=334
    unsigned char F8key[MAXKEYLEN]; //334+64=398
    unsigned char F9key[MAXKEYLEN]; //398+64=462

};

extern volatile struct option_s Option, *SOption;
extern unsigned char *CFunctionFlash, *CFunctionLibrary;
extern volatile uint32_t  realflashpointer;
void ResetAllOptions(void);
void ResetAllFlash(void);
void ResetAllBackupRam(void);


void SaveOptions(void);
void LoadOptions(void);
void FlashWriteInit(int sector);
void FlashWriteByte(unsigned char b);
void FlashWriteWord(unsigned int i);
void FlashWriteAlign(void);
void FlashSetAddress(int address);  //new
void FlashWriteClose(void);
void UpdateFlash(uint32_t address, uint32_t data);
int GetFlashOption(const unsigned int *w) ;
void SetFlashOption(const unsigned int *w, int x) ;
uint32_t GetSector(uint32_t Address);
void cmd_var(void);
void MIPS16 cmd_library(void);  //LIBRARY
void MIPS16 cmd_flash(void);   //W25Q16
long long int CallCFunction(char *CmdPtr, char *ArgList, char *DefP, char *CallersLinePtr);
extern char * ProgMemory;
extern void ClearSavedVars(void);
extern void ClearRTCRam(void);
void RoundDoubleFloat(MMFLOAT *ff);

//extern void AppendLibrary();
extern void InitFlash_CS();
void WBReadPage(int pageno,char *p);
void WBReadSector(int pageno,char *p);
void WBWritePage(int pageno,char *p);
void WBWriteSector(int pageno,char *p);
void WBEraseArea(int erasemode,int pageno);
void SPIOpen(void);
#define WBLibAddr 7936   //First page of the Library 64K
#define WBVarAddr 7920   //First page of the Var Save 4K
//#define WBLibAddr 0   //First page of the Library 64K
//#define WBVarAddr 256   //First page of the Var Save 4K
//#define WBUserEndAddr 272   //Last page of the User Area
#define writeenable 0x06
#define writedisable 0x04
#define pageprogram 0x02
#define writestatus1 0x01
#define readstatus1 0x05
#define readdata 0x03
#define pagewrite 0x02
#define fastread 0x0B
#define sectorerase 0x20  // 4K 16 pages
#define blockerase 0x52   // 32K 128 pages
#define block2erase 0xD8  //64K 256 pages
#define eraseall 0xC7
#define JEDEC 0x9F
#define SPIsend1(a) {uint8_t b=a;HAL_SPI_Transmit(&hspi1,&b,1,500);}
#define SPIqueue256(a) {HAL_SPI_Transmit(&hspi1,a,3,500);}


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#endif


/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "VAR",	    	T_CMD,				0, cmd_var	},
	{ "Library",        T_CMD,              0, cmd_library  },                 //LIBRARY
	//{ "Flash",        T_CMD,              0, cmd_flash  },                    //W23Q16
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

#endif
