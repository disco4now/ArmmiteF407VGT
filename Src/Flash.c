/*-*****************************************************************************

ArmmiteF4 MMBasic

Flash.c

Handles saving and restoring from flash.

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
//#include "stm32f4xx_flash.h"
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
// The CFUNCTION data comes after the program in program memory
// and this is used to point to its start
unsigned char *CFunctionFlash = NULL;
unsigned char *CFunctionLibrary = NULL;
volatile struct option_s Option, *SOption;
char * ProgMemory;
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t SectorError = 0;
volatile union u_flash {
  uint32_t i32;
  uint8_t  i8[4];
} FlashWord, FlashWordSave;
volatile int i8p=0;
extern volatile int ConsoleTxBufHead, ConsoleTxBufTail;
int sectorsave;
// globals used when writing bytes to flash
volatile uint32_t realflashpointer;
uint32_t GetSector(uint32_t Address);
extern RTC_HandleTypeDef hrtc;
extern SPI_HandleTypeDef hspi1;

// erase the flash and init the variables used to buffer bytes for writing to the flash
void FlashWriteInit(int sector) {
//	__IO uint32_t SectorsWRPStatus = 0xFFF;
    // Unlock the Flash to enable the flash control register access
	HAL_SuspendTick();
    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    __HAL_FLASH_DATA_CACHE_RESET();
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
	HAL_FLASH_Unlock();
    i8p=0;
	FlashWord.i32=0xFFFFFFFF;
	sectorsave=sector;
    // Clear pending flags (if any)

    // Get the number of the start and end sectors

       // Device voltage range supposed to be [2.7V to 3.6V], the operation will
       //  be done by word
      if(sector == PROGRAM_FLASH){
    	  realflashpointer=FLASH_PROGRAM_ADDR;
    	  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector        = GetSector(FLASH_PROGRAM_ADDR);
    	  EraseInitStruct.NbSectors     = 1;
       	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
       		  uSec(1000);
           	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
           		  error("Program flash erase");
           	  }
       	  }
      }
/*      if(sector == SAVED_OPTIONS_FLASH){
      	  realflashpointer=FLASH_SAVED_OPTION_ADDR ;
    	  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector        = GetSector(FLASH_SAVED_OPTION_ADDR);
    	  EraseInitStruct.NbSectors     = 1;
       	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
       		  uSec(1000);
           	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
           		  error("Option flash erase");
           	  }
       	  }
     }
     if(sector == SAVED_VARS_FLASH){
       	  realflashpointer=FLASH_SAVED_VAR_ADDR ;
    	  EraseInitStruct.Banks			= FLASH_BANK_1;
    	  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector        = GetSector(FLASH_SAVED_VAR_ADDR);
    	  EraseInitStruct.NbSectors     = 1;
       	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
       		  uSec(1000);
           	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
           		  error("Saved variable flash erase");
           	  }
       	  }
    }*/

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();
	HAL_ResumeTick();
}
void FlashWriteBlock(void){
    int i;
    uint32_t address=realflashpointer-4;
    uint32_t *there = (uint32_t *)address;

    if(address % 4)error("Flash write address");
    if(sectorsave == PROGRAM_FLASH && (address<FLASH_PROGRAM_ADDR || address>=FLASH_PROGRAM_ADDR+PROG_FLASH_SIZE))error("PROGRAM_FLASH location &",address);
//    if(sectorsave == SAVED_OPTIONS_FLASH && (address<FLASH_SAVED_OPTION_ADDR || address>=FLASH_SAVED_OPTION_ADDR+0x4000))error("SAVED_OPTIONS_FLASH location &",address);
//    if(sectorsave == SAVED_VARS_FLASH && (address<FLASH_SAVED_VAR_ADDR || address>=FLASH_SAVED_VAR_ADDR+0x4000))error("SAVED_VARS_FLASH location &",address);

    	if(*there!=0xFFFFFFFF) error("flash not erased");

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, FlashWord.i32) != HAL_OK)
	{
		error("Flash write fail");
	}
	for(i=0;i<4;i++)FlashWord.i8[i]=0xFF;
}
// write a byte to flash
// this will buffer four bytes so that the write to flash can be a word
void FlashWriteByte(unsigned char b) {
	realflashpointer++;
	FlashWord.i8[i8p]=b;
	i8p++;
	i8p %= 4;
	if(i8p==0){
		FlashWriteBlock();
	}
}

void FlashWriteAlign(void) {
	  while(i8p != 0) {
		  FlashWriteByte(0x0);
	  }
	  FlashWriteWord(0xFFFFFFFF);
}

// utility routine used by SaveProgramToFlash() and cmd_var to write a byte to flash while erasing the page if needed
void FlashWriteWord(unsigned int i) {
	FlashWriteByte(i & 0xFF);
	FlashWriteByte((i>>8) & 0xFF);
	FlashWriteByte((i>>16) & 0xFF);
	FlashWriteByte((i>>24) & 0xFF);
}

// Set the pointer to a specific address
void FlashSetAddress(int address) {
	 realflashpointer=FLASH_PROGRAM_ADDR+address;
}

// flush any bytes in the buffer to flash
void FlashWriteClose(void) {
	  while(i8p != 0) {
		  FlashWriteByte(0xff);
	  }
}



/*******************************************************************************************************************
 Code to execute a CFunction saved in flash
*******************************************************************************************************************/
/*******************************************************************************************************************
 VARSAVE and VARSAVE RESTORE commands

 Variables are saved in flash as follows:
 Numeric variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     4 or 8 bytes = the value of the variable
 String variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     1 bytes = length of the variable's string
     ? bytes = the variables string

********************************************************************************************************************/


/*******************************************************************************************************************
 The variables are stored in battery backed RAM (which in total is 4K).
 The options are store separately in 80 bytes of battery backed RAM.
********************************************************************************************************************/
void cmd_var(void) {
    char *p, *buf, *bufp, *varp, *vdata, lastc;
    int i, j, nbr = 1, nbr2=1, array, type, SaveDefaultType;
    int VarList[MAX_ARG_COUNT];
    char *VarDataList[MAX_ARG_COUNT];
    char *SavedVarsFlash;
    char *w;


    if((p = checkstring(cmdline, "CLEAR"))) {
        checkend(p);
        ClearSavedVars();
        return;
    }
    if((p = checkstring(cmdline, "RESTORE"))) {
        char b[MAXVARLEN + 3];
        checkend(p);
        SavedVarsFlash = (char*)SAVED_VAR_RAM_ADDR;      // point to where the variables were saved
//        if(SavedVarsFlash[4] == 0) return;                          // zero in this location means that nothing has ever been saved
        SaveDefaultType = DefaultType;                              // save the default type
        bufp = SavedVarsFlash;   // point to where the variables were saved
        while(*bufp != 0xff) {                                      // 0xff is the end of the variable list
            type = *bufp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x7f;                     // set array to true if it is an array
            DefaultType = TypeMask(type);                           // and set the default type to this
            if(array) {
                strcpy(b, bufp);
                strcat(b, "()");
                vdata = findvar(b, type | V_EMPTY_OK | V_NOFIND_ERR);     // find an array
            } else
                vdata = findvar(bufp, type | V_FIND);               // find or create a non arrayed variable
            if(TypeMask(vartbl[VarIndex].type) != TypeMask(type)) error("$ type conflict", bufp);
            if(vartbl[VarIndex].type & T_CONST) error("$ is a constant", bufp);
            bufp += strlen((char *)bufp) + 1;                       // step over the name and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                nbr = *bufp++;
                nbr |= (*bufp++) << 8;
                nbr |= (*bufp++) << 16;
                nbr |= (*bufp++) << 24;
                nbr2 = 1;
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr2 *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                if(type & T_STR) nbr2 *= vartbl[VarIndex].size +1;
                if(type & T_NBR) nbr2 *= sizeof(MMFLOAT);
                if(type & T_INT) nbr2 *= sizeof(long long int);
                if(nbr2!=nbr)error("Array size");
            } else {
               if(type & T_STR) nbr = *bufp + 1;
               if(type & T_NBR) nbr = sizeof(MMFLOAT);
               if(type & T_INT) nbr = sizeof(long long int);
            }
            while(nbr--) *vdata++ = *bufp++;                        // copy the data
        }
        DefaultType = SaveDefaultType;
        return;
    }

     if((p = checkstring(cmdline, "SAVE"))) {
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");                  // getargs macro must be the first executable stmt in a block
        if(argc == 0) error("Invalid syntax");
        if(argc && (argc & 0x01) == 0) error("Invalid syntax");

        // before we start, run through the arguments checking for errors
        for(i = 0; i < argc; i += 2) {
            checkend(skipvar(argv[i], false));
            VarDataList[i/2] = findvar(argv[i], V_NOFIND_ERR | V_EMPTY_OK);
            VarList[i/2] = VarIndex;
            if((vartbl[VarIndex].type & (T_CONST | T_PTR)) || vartbl[VarIndex].level != 0) error("Invalid variable");
            p = &argv[i][strlen(argv[i]) - 1];                      // pointer to the last char
            if(*p == ')') {                                         // strip off any empty brackets which indicate an array
                p--;
                if(*p == ' ') p--;
                if(*p == '(')
                    *p = 0;
                else
                    error("Invalid variable");
            }
        }
        // load the current variable save table into RAM
        // while doing this skip any variables that are in the argument list for this save
        bufp = buf = GetTempMemory(SAVED_VAR_RAM_SIZE);           // build the saved variable table in RAM
        SavedVarsFlash = (char*)SAVED_VAR_RAM_ADDR;      // point to where the variables were saved
        varp = SavedVarsFlash;   // point to where the variables were saved

        while(SavedVarsFlash[4] != 0 && *varp != 0xff) {            // 0xff is the end of the variable list, SavedVarsFlash[4] = 0 means that the flash has never been written to
           	type = *varp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x7f;                     // set array to true if it is an array
            vdata = varp;                                           // save a pointer to the name
            while(*varp) varp++;                                    // skip the name
            varp++;                                                 // and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                 nbr = (varp[0] | (varp[1] << 8) | (varp[2] << 16) | (varp[3] << 24)) + 4;
            } else {
                if(type & T_STR) nbr = *varp + 1;
                if(type & T_NBR) nbr = sizeof(MMFLOAT);
                if(type & T_INT) nbr = sizeof(long long int);
            }
            for(i = 0; i < argc; i += 2) {                          // scan the argument list
                p = &argv[i][strlen(argv[i]) - 1];                  // pointer to the last char
                lastc = *p;                                         // get the last char
                if(lastc <= '%') *p = 0;                            // remove the type suffix for the compare
                if(strncasecmp(vdata, argv[i], MAXVARLEN) == 0) {   // does the entry have the same name?
                    while(nbr--) varp++;                            // found matching variable, skip over the entry in flash (ie, do not copy to RAM)
                    i = 9999;                                       // force the termination of the for loop
                }
                *p = lastc;                                         // restore the type suffix
            }
            // finished scanning the argument list, did we find a matching variable?
            // if not, copy this entry to RAM
            if(i < 9999) {
                *bufp++ = type | array;
                while(*vdata) *bufp++ = *vdata++;                   // copy the name
                *bufp++ = *vdata++;                                 // and the terminating zero byte
                while(nbr--) *bufp++ = *varp++;                     // copy the data
            }

        }


        // initialise for writing to the flash
//        FlashWriteInit(SAVED_VARS_FLASH);
        w=(char*)SAVED_VAR_RAM_ADDR;
        for(i=0;i<SAVED_VAR_RAM_SIZE;i++)*w++=0xFF;

        // now write the variables in RAM recovered from the var save list
        w=(char*)SAVED_VAR_RAM_ADDR;
        while(buf < bufp){
        	*w++=*buf++;
        }

        // now save the variables listed in this invocation of VAR SAVE
        for(i = 0; i < argc; i += 2) {
            VarIndex = VarList[i/2];                                // previously saved index to the variable
            vdata = VarDataList[i/2];                               // pointer to the variable's data
            type = TypeMask(vartbl[VarIndex].type);                 // get the variable's type
            type |= (vartbl[VarIndex].type & T_IMPLIED);            // set the implied flag
            array = (vartbl[VarIndex].dims[0] != 0);

            nbr = 1;                                                // number of elements to save
            if(array) {                                             // if this is an array calculate the number of elements
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                type |= 0x80;                                       // an array has the top bit set
            }

            if(type & T_STR) {
                if(array)
                    nbr *= (vartbl[VarIndex].size + 1);
                else
                    nbr = *vdata + 1;                               // for a simple string variable just save the string
            }
            if(type & T_NBR) nbr *= sizeof(MMFLOAT);
            if(type & T_INT) nbr *= sizeof(long long int);
            if((uint32_t)w - (uint32_t)SavedVarsFlash + 36 + nbr > SAVED_VAR_RAM_SIZE) {
//                FlashWriteClose();
                error("Not enough memory");
            }
            *w++=type;                              // save its type
            for(j = 0, p = vartbl[VarIndex].name; *p && j < MAXVARLEN; p++, j++)
                *w++=*p;                            // save the name
            *w++=0;                                 // terminate the name
            if(array) {                                             // if it is an array save the number of data bytes
               *w++=nbr; *w++=(nbr >> 8); *w++=(nbr >>16); *w++=(nbr >>24);
            }
            while(nbr--) *w++=(*vdata++);             // write the data
        }
//        FlashWriteClose();
        return;
     }
     if((p = checkstring(cmdline, "FSAVE"))) { // Save 4K of VARs to W25Q16 Windbond.
    	    // write the whole lot into the W25Q16 VAR area at page 8064
    	    // First Erase the 4K Var area in W25Q16
    	    int pageno=WBVarAddr;
    	    SPIOpen();
    	    WBEraseArea(sectorerase,pageno);
            // load the current variable save data into RAM
    	    buf = GetTempMemory(SAVED_VAR_RAM_SIZE);
    	    w = (char*)SAVED_VAR_RAM_ADDR;                  // point to where the variables were saved
    	    for(i=0;i<SAVED_VAR_RAM_SIZE;i++){
    	         buf[i]= w[i];
    	    }
            //now write to WindBond W25Q16 flash
    	    p = GetMemory(256);
    	    i=0;
    	    for(j=0;j<SAVED_VAR_RAM_SIZE;j++){
    	      p[i++]=buf[j];
              if (i>255){  //write our page
    	         WBWritePage(pageno++,p);
    	         i=0;
    	      }
            }
    	    FreeMemory(p);
    	    SPIClose();
    	    return;
      }

      if((p = checkstring(cmdline, "FRESTORE"))) { // Restore 4K of VARs from W25Q16 Windbond.

    	     //Restore 4K of VAR data for WindBond WBVarAddr to the 4K Battery Backed Ram
    	     int pageno;
    	     w=(char*)SAVED_VAR_RAM_ADDR;
    	     SPIOpen();
    	     p = GetMemory(256);
    	     i=0;j=0;
    	     for (pageno=WBVarAddr;pageno<WBVarAddr+16;pageno++){
    	        WBReadPage(pageno,p);
    	        for (i=0;i<256;i++){ w[j++]=p[i] ;}
    	     }
    	     FreeMemory(p);
    	     SPIClose();
    	     return;
      }
    error("Unknown command");
}

/**********************************************************************************************
   These routines are used to load or save the global Option structure from/to flash.
   These options are stored in the beginning of the flash used to save stored variables.
   NOT USED as Options are now in 80 bytes of RTC battery backed ram
***********************************************************************************************/


/*void SaveOptions(void) {
	int i;
    char *p, *SavedOptionsFlash;
    SavedOptionsFlash=(char*)FLASH_SAVED_OPTION_ADDR;
    p = (char *)&Option;
    for(i = 0; i < sizeof(struct option_s); i++) {
    	if(SavedOptionsFlash[i] != *p) goto skipreturn;
    	p++;
    }
    return;                                                         // exit if the option has already been set (ie, nothing to do)
    skipreturn:

    while(!(ConsoleTxBufHead == ConsoleTxBufTail)){};                    // wait for the console UART to send whatever is in its buffer
    p = (char *)&Option;
    FlashWriteInit(SAVED_OPTIONS_FLASH);                // erase the page
    for(i = 0; i < sizeof(struct option_s); i++){
    	FlashWriteByte(*p);    // write the changed page back to flash
    	p++;
    }
    FlashWriteClose();

}

void LoadOptions(void) {
    memcpy((struct option_s *)&Option, (struct option_s *)FLASH_SAVED_OPTION_ADDR, sizeof(struct option_s));
}*/

/**********************************************************************************************
   These routines are used to load or save the global Option structure from/to 80 bytes
   of RTC battery backed ram.
***********************************************************************************************/

void SaveOptions(void) {
	uint32_t *p, *q;
	int i,readback[20];
    p = q = (uint32_t *)&Option;
    for(i = 0; i < 20; i++){
    	HAL_RTCEx_BKUPWrite (&hrtc, i, *p++);
    }
    for(i = 0; i < 20; i++){
		readback[i] = HAL_RTCEx_BKUPRead (&hrtc, i);
	}
    for(i=0; i < 20; i++){
    	if(readback[i]!=*q++)error("Options not saved");
    }
}

/* SaveOptions as a function returns 1 if any changes. 0 if no change. */
/*
int SaveOptions(void) {
	uint32_t *p, *q;
	int i,readback[20],readcurrent[20];
    p = q = (uint32_t *)&Option;
    for(i = 0; i < 20; i++){
    	HAL_RTCEx_BKUPWrite (&hrtc, i, *p++);
    }
    for(i = 0; i < 20; i++){
		readback[i] = HAL_RTCEx_BKUPRead (&hrtc, i);
	}
    for(i=0; i < 20; i++){
    	if(readback[i]!=*q++)error("Options not saved");
    }
    for(i=0; i < 20; i++){
        	if(readcurrent[i]!=*q++)return 1;
    }
    return 0;
}

*/

void LoadOptions(void) {
	uint32_t *p;
    p = (uint32_t *)&Option;
	int i;
    for(i = 0; i < 20; i++){
		*p++ = HAL_RTCEx_BKUPRead (&hrtc, i);
	}
}


// reset the options to their defaults
// used on initial firmware run or if options are corrupt
void ResetAllOptions(void) {
    Option.Height = SCREENHEIGHT;
    Option.Width = SCREENWIDTH;
    Option.PIN = 0;
    Option.Baudrate = CONSOLE_BAUDRATE;
    Option.Autorun = false;
    Option.Listcase = CONFIG_TITLE;
    Option.Tab = 2;
    Option.Restart = false;
    Option.ColourCode = true;
   // Option.DISPLAY_TYPE = ILI9341_16;
    Option.DISPLAY_TYPE = 0;
    Option.DISPLAY_ORIENTATION = RLANDSCAPE;
    Option.TOUCH_SWAPXY = 0;
    Option.TOUCH_XSCALE = 0.0944;
    Option.TOUCH_YSCALE = 0.0668;
    Option.TOUCH_XZERO = 355;
    Option.TOUCH_YZERO = 205;
    Option.TOUCH_CS = 51;
    Option.TOUCH_IRQ = 34;
   	Option.LCD_CD = 0;
   	Option.LCD_Reset = 0;
   	Option.LCD_CS = 0;
   	Option.FLASH_CS=35;    // 35 default or 77 for mini
    Option.DISPLAY_CONSOLE = 0;
    Option.DefaultFont = 0x01;
    Option.DefaultFC = WHITE;
    Option.DefaultBC = BLACK;
    Option.RTC_Calibrate = 0;

    Option.SerialConDisabled = 1;
    Option.KeyboardConfig = NO_KEYBOARD ;
    Option.SSDspeed = 0;
    Option.TOUCH_Click = 0;
    Option.DefaultBrightness = 50;
    Option.SerialPullup = 1;
    Option.MaxCtrls=201;
    Option.ProgFlashSize = PROG_FLASH_SIZE;              //LIBRARY
    Option.Refresh = 0;
	Option.DISPLAY_WIDTH = 0;
	Option.DISPLAY_HEIGHT = 0;


}
//void ClearSavedVars(void) {
 //   FlashWriteInit(SAVED_VARS_FLASH);                              // initialise for writing to the flash
//}
void ClearSavedVars(void) {
	int i;
	char *w;
    w=(char*)SAVED_VAR_RAM_ADDR;
    for(i=0;i<SAVED_VAR_RAM_SIZE;i++)*w++=0xFF;
}

/*******************************************************************************************************************
 The LIBRARY commands
 The library is an area in the top of program flash that holds code saved with LIBRARY SAVE.  It includes BASIC
 code and CFunctions extracted from the library BASIC code.

 The program flash layout is:
 ===================   << PROG_FLASH_SIZE    (the total size of the flash area in bytes)
 |                 |
 |    library      |
 |   CFunctions    |
 ===================   << CFunctionLibrary   (starting address of the library CFunctions)
 |    library      |
 |     BASIC       |
 |     code        |
 ===================   << Option.ProgFlashSize   (the maximum size of a BASIC program. Indicates the library start)
 |                 |
 |     free        |
 |   program       |
 |    flash        |
 |                 |
 ~~~~~~~~~~~~~~~~~~~
 |     main        |
 |    program      |       this contains the CFunctions extracted from the main BASIC program
 |   CFunctions    |
 ===================   << CFunctionFlash   (starting address of the main program CFunctions)
 |      main       |
 |     BASIC       |
 |    program      |
 ===================   << ProgMemory   (starting address of the program flash area)

********************************************************************************************************************/
/*******************************************************************************************************************
 The WindBond W25Q16 is 2 Meg of 8192 256 byte pages.
 It is on SPI1 and F_CS is 35 on VET6 board and 77 VET6 MINI
 The usage layout is:

 ===================   << Page 8191 end of Flash and user area
 |    User         |      7808 pages
 |   Available     |   Available to User.
 |                 |   erase  Block2 is 64K  256 pages
 ~   30*64K Blocks ~   erase  Block is 32K   128 pages
 ~                 ~   erase Sector is 4K    16 pages
 ~~~~~~~~~~~~~~~~~~~   << Page 512 Start of 64K Blocks
 |  1*32K Block    |
 |                 |   << Page 384  continuation of User Area
 ~~~~~~~~~~~~~~~~~~~   << Page 383 is end of 28K area
 |   User          |     28 K  needs to be erased in 7 * 4K sectors of 16pages
 ~   Available     ~     112 pages
 |  7*4K blocks    |   << Page 272 Start of User Area
 ===================   << Page 271 End of VAR FSAVE area
 |    VAR FSAVE    |      4K 16 pages
 |    4K Backup    |   << WBVarAddr (is Page 256 Start of 4K used for VAR FSAVE backup of VAR ram)
 ===================   << Page 255 END Library Area
 |  64K (255 Pages)|     256 pages
 |  library        |   64K Block can be erased i.e. 256 pages
 |  temp storage   |   <- library can be up to 64K
 |                 |   << WBLibAddr (is Page 0 Start of 64K of Library temp storage )
 ===================

Full Erase will clear library and FSAVE area
Minimum 4K sector can be erased i.e. 16 pages

********************************************************************************************************************/

/*******************************************************************************************************************
 The WindBond W25Q16 is 2 Meg of 8192 256 byte pages.
 It is on SPI1 and F_CS is 35 on VET6 board and 77 VET6 MINI
 The Library and VAR FSAVE are used by MMBasic hence we must not use full Erase.
 The User Area is erased as 30*64K Blocks + 1*32K Block + 7*28K Blocks
 The usage layout is:
 ===================   << Page 8191 end of 2Meg Flash
 |  64K (256 pages |      64K Block can be erased i.e. 256 pages
 |    library      |
 |  flash storage  |   <- library can be up to 64K
 |                 |   << WBLibAddr (is Page 7936 (8192-256) Start of 64K of Library temp storage )
 ===================   << Page 7935 End of VAR FSAVE area
 |    VAR FSAVE    |     Minimum 4K sector can be erased i.e. 16 pages
 |    4K Backup    |   << WBVarAddr (is Page 7920 (8192- 272) Start of 4K used for VAR FSAVE backup of VAR ram)
 ===================   << Page 7919 End of user area
 |   User          |     28 K  needs to be erased in 7 * 4K sectors of 16pages
 ~   Area          ~     112 pages
 |   7*4K blocks   |   << Page 7808  continuation of User Area  7*4K Blocks
 ~~~~~~~~~~~~~~~~~~~
 |  1*32K Block    |
 |  128 pages      |   << Page 7680  continuation of User Area 1*32K Block 7806= ((16*flash_size)-2)*256
 ~~~~~~~~~~~~~~~~~~~
 |    User         |   << page 7679 is end 64K blocks
 |    Area         |   Available to User.
 |                 |   erase  Block2 is 64K  256 pages
 ~                 ~   erase  Block is 32K   128 pages
 ~  30*64K Blocks  ~   erase Sector is 4K    16 pages
 ~                 ~
 |                 |
 |                 |
 ===================   << Page0    Start of User Area

********************************************************************************************************************/




void MIPS16 cmd_library(void) {
    char *p,  *pp , *m, *MemBuff, *TempPtr, rem;
    int i, j, k, InCFun, InQuote, CmdExpected;
    unsigned int CFunDefAddr[100], *CFunHexAddr[100] , *CFunHexLibAddr[100] ;
    /********************************************************************************************************************
     ******* LIBRARY SAVE **********************************************************************************************/
    if((p = checkstring(cmdline, "SAVE"))) {
        if(CurrentLinePtr) error("Invalid in a program");
        if(*ProgMemory != 0x01) return;
        checkend(p);
        ClearRuntime();
        TempPtr = m = MemBuff = GetTempMemory(EDIT_BUFFER_SIZE);

        rem = GetCommandValue("Rem");
        InQuote = InCFun = j = 0;
        CmdExpected = true;

        // first copy the current program code residing in the Library area to RAM
        if(Option.ProgFlashSize != PROG_FLASH_SIZE) {
            p = ProgMemory + Option.ProgFlashSize;
            while(!(p[0] == 0 && p[1] == 0)) *m++ = *p++;
              *m++ = 0;                                               // terminate the last line
        }
     //dump(m, 256);
        // then copy the current contents of the program memory to RAM
        p = ProgMemory;
        while(*p != 0xff) {
            if(p[0] == 0 && p[1] == 0) break;                       // end of the program

            if(*p == T_NEWLINE) {
                TempPtr = m;
                CurrentLinePtr = p;
                *m++ = *p++;
                CmdExpected = true;                                 // if true we can expect a command next (possibly a CFunction, etc)
                if(*p == 0) {                                       // if this is an empty line we can skip it
                    p++;
                    if(*p == 0) break;                              // end of the program or module
                    m--;
                    continue;
                }
            }

            if(*p == T_LINENBR) {
//                TempPtr = m;
                *m++ = *p++; *m++ = *p++; *m++ = *p++;              // copy the line number
                skipspace(p);
            }

            if(*p == T_LABEL) {
                for(i = p[1] + 2; i > 0; i--) *m++ = *p++;          // copy the label
//                TempPtr = m;
                skipspace(p);
            }

            if(CmdExpected && ( *p == GetCommandValue("End CFunction") || *p == GetCommandValue("End CSub") || *p == GetCommandValue("End DefineFont"))) {
                InCFun = false;                                     // found an  END CSUB or END DEFINEFONT token
            }

            if(InCFun) {
                skipline(p);                                        // skip the body of a CFunction
                m = TempPtr;                                        // don't copy the new line
                continue;
            }

            if(CmdExpected && ( *p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont"))) {    // found a  CSUB or DEFINEFONT token
                CFunDefAddr[++j] = (unsigned int)m;                 // save its address so that the binary copy in the library can point to it
                while(*p) *m++ = *p++;                              // copy the declaration
                InCFun = true;
            }

            if(CmdExpected && *p == rem) {                          // found a REM token
                skipline(p);
                m = TempPtr;                                        // don't copy the new line tokens
                continue;
            }

            if(*p >= C_BASETOKEN || isnamestart(*p))
                CmdExpected = false;                                // stop looking for a CFunction, etc on this line

            if(*p == '"') InQuote = !InQuote;                       // found the start/end of a string

            if(*p == '\'' && !InQuote) {                            // found a modern remark char
                skipline(p);
                //if(*(m-3) == 0x01) {  //old line format.
                if((*(m-1) == 0x01) ||  ((*(m-2) == 0x01) && (*(m-1) == 0x20))){
                    m = TempPtr;                                    // if the comment was the only thing on the line don't copy the line at all
                    continue;
                } else
                    p--;
            }

            if(*p == ' ' && !InQuote) {                             // found a space
                if(*(m-1) == ' ') m--;                              // don't copy the space if the preceeding char was a space
            }

            if(p[0] == 0 && p[1] == 0) break;                       // end of the program
            *m++ = *p++;
        }
        //while(*p == 0) *m++ = *p++;                                 // the end of the program can have multiple zeros
        //*m++ = *p++;;                                               // copy the terminating 0xff
        //while((unsigned int)p & 0b11) p++;
        //while((unsigned int)m & 0b11) *m++ = 0xff;                  // step to the next word boundary

        //The fonts or CSUBs binary starting on a new four byte boundary so there can be a variable number 2-5
        //0x00 bytes at the end of the program. We only need two of them .
        // At the end of the program so get the two 0x00 bytes
               *m++ = *p++;
               *m++ = *p++;
               // Step the program memory up to the first 0xFF of the 4 that mark the beginning of the CSub binaries.
               while(*p != 0xff) p++;
               p++;p++; p++;p++;                                           // step over the header of the four 0xff bytes

               //step the memory to the next 4 word boundary
               // while((unsigned int)p & 0b11) p++;
               while((unsigned int)m & 0b11) *m++ = 0x00;                  // fill memory with 0x00 to the next word boundary
               *m++=0xFF;*m++=0xFF;*m++=0xFF;*m++=0xFF;                    //write 4 byte of the csub binary header

        // now copy the CFunction/CSub/Font data
        // =====================================
        // the format of a CFunction in flash is:
        //   Unsigned Int - Address of the CFunction/CSub/Font in program memory (points to the token).  For a font it is zero.
        //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
        //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  The specs for the font if it is a font.
        //   word1..wordN - The CFunction code
        // The next CFunction starts immediately following the last word of the previous CFunction

        // first, copy any CFunctions residing in the library area to RAM
        k = 0;                                                      // this is used to index CFunHexLibAddr[] for later adjustment of a CFun's address
        if(CFunctionLibrary != NULL) {
            pp = (char *)CFunctionLibrary;
            while(*(unsigned int *)pp != 0xffffffff) {
                CFunHexLibAddr[++k] = (unsigned int *)m;            // save the address for later adjustment
                j = (*(unsigned int *)(pp + 4)) + 8;                // calculate the total size of the CFunction
                while(j--) *m++ = *pp++;                            // copy it into RAM
            }
        }

        // then, copy any CFunctions in program memory to RAM
        i = 0;                                                      // this is used to index CFunHexAddr[] for later adjustment of a CFun's address
        while(*(unsigned int *)p != 0xffffffff) {
            CFunHexAddr[++i] = (unsigned int *)m;                   // save the address for later adjustment
            j = (*(unsigned int *)(p + 4)) + 8;                     // calculate the total size of the CFunction
            while(j--) *m++ = *p++;                                 // copy it into RAM
        }

        // we have now copied all the CFunctions into RAM



        // calculate the starting point of the library code (located in the top of the program space in flash)
       //TempPtr = (ProgMemory + PROG_FLASH_SIZE) - (((m - MemBuff) + (FLASH_PAGE_SIZE - 1)) & (~(FLASH_PAGE_SIZE - 1)));

        // calculate the size of the library code  to  end on a word boundary
        j=(((m - MemBuff) + (0x4 - 1)) & (~(0x4 - 1)));
        j=j+4; //Leave room at end for 0xFFFFFFFF.
        //We only have reserved 60K of flash to cache the library code in the windbond.
        //Error if we try to use too much
        if (j > 60*1024) error("Library too big, > 64K");

        // calculate the starting point of the library code (located in the top of the program space in flash)
        TempPtr = (ProgMemory + PROG_FLASH_SIZE) - j;

        // TempPtr = (ProgMemory + PROG_FLASH_SIZE) - (((m - MemBuff) + (0x4 - 1)) & (~(0x4 - 1)));

        // now adjust the addresses of the declaration in each CFunction header
        // do not adjust a font who's "address" is zero

        // first, CFunctions that were already in the library
        for(; k > 0; k--) {
            if(*CFunHexLibAddr[k]  > FONT_TABLE_SIZE) *CFunHexLibAddr[k] -= ((unsigned int)(ProgMemory + Option.ProgFlashSize) - (unsigned int)TempPtr);
        }

        // then, CFunctions that are being added to the library
        for(; i > 0; i--) {
            if(*CFunHexAddr[i]  > FONT_TABLE_SIZE) *CFunHexAddr[i] = (CFunDefAddr[i] - (unsigned int)MemBuff) + (unsigned int)TempPtr;
        }

        //******************************************************************************
        // write the whole library from ram into the W25Q16 library area
        // First Erase the library and write the header page
        int pageno=WBLibAddr;
        SPIOpen();
        WBEraseArea(blockerase,pageno);
        j=TempPtr - ProgMemory; // i.e. Option.ProgFlashSize
        p = GetMemory(256);

#ifdef WRITEHEADERPAGE
        //Write the header of AA 55 plus Option.ProgFlashSize

        p[0]=0xAA; p[1]=0x55; p[2]=0x55; p[3]=0xAA;  //The AA55 Header
        p[4]=(j >> 16) & 0xFF; p[5]=(j >> 8) & 0xFF; p[6]=j & 0xFF;

        for(k = 7; k < 256; k++){        // Build the rest of the header page
             p[k]=255;
        }
        //write our header page
        WBWritePage(pageno++,p);
        //dump(p,256,pageno-1);
#endif
         //now write the library from ram to the windbond library area
        i=0;
        for(k = 0; k < m - MemBuff; k++){        // write to the flash byte by byte
       	   p[i++]=MemBuff[k];
       	   //p[i++]=15;
       	   if (i>255){  //write our page
       	     WBWritePage(pageno++,p);
       	     //dump(p,256,pageno-1);
       	     i=0;
       	   }
        }
        //Write last page if required
        if (i!=0){
           for(;i<256;i++){p[i]=255;}
           WBWritePage(pageno++,p);
          // dump(p,256,pageno-1);
        }
        FreeMemory(p);
        SPIClose();

        // Update the amount of flash available for ordinary programs
        Option.ProgFlashSize = TempPtr - ProgMemory;
        SaveOptions();

        if(MMCharPos > 1) PRet();                    // message should be on a new line
        MMPrintString("Library Saved ");
        IntToStr(tknbuf, PROG_FLASH_SIZE-Option.ProgFlashSize, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");

        //Now call the new command that will clear the current program memory then
        //write the library code at Option.ProgFlashSize by copying it from the windbond
        //and return to the command prompt.
        cmdline = ""; CurrentLinePtr = NULL;    // keep the NEW command happy
        cmd_new();                              //  delete the program,add the library code and return to the command prompt

    }

    /********************************************************************************************************************
    ******* LIBRARY DELETE **********************************************************************************************/

     if(checkstring(cmdline, "DELETE")) {
        if(Option.ProgFlashSize == PROG_FLASH_SIZE) return;

       // Clear library from W25Q16
        SPIOpen();
        WBEraseArea(blockerase,WBLibAddr);
        SPIClose();

        Option.ProgFlashSize = PROG_FLASH_SIZE;
        SaveOptions();

        //Note: At this stage the Library code is still at the end of the program flash, but it will
        //      not be seen as Option.ProgFlashSize says its not there.

        // Clear Program Memory and also the Library at the end.
        //cmdline = ""; CurrentLinePtr = NULL;    // keep the NEW command happy
        //cmd_new();                              //  delete any program,and the library code and return to the command prompt

        return;
     }

     /********************************************************************************************************************
     ******* LIBRARY LIST **********************************************************************************************/
     if(checkstring(cmdline, "LIST ALL")) {
        if(Option.ProgFlashSize == PROG_FLASH_SIZE) return;
        ListProgram(ProgMemory + Option.ProgFlashSize, true);
        return;
     }

     if(checkstring(cmdline, "LIST")) {
        if(Option.ProgFlashSize == PROG_FLASH_SIZE) return;
        ListProgram(ProgMemory + Option.ProgFlashSize, false);
        return;
     }



    /**********************************************************************************************
          LIBRARY CHECK will check for the existence of Library code at the end of program memory.
          Is existence is normally indicated by Option.ProgFlashSize which is set when the library
          code is saved. If library code is found AND the Winbond Flash library area is also not empty
          then Option.ProgFlashSize is set to the library's location.
          This would only be required after an OPTION RESET or when running without a battery,
          in which case the Options are set to default at every power on.
    ***********************************************************************************************/
     if(checkstring(cmdline, "CHECK")) {  //See if library code exists and set Option.ProgFlashSize
            	   if(Option.ProgFlashSize == PROG_FLASH_SIZE) { //Only if not already set
            		   p=ProgMemory;
            		   //Skip to end if program code
            		   while(*p != 0xff) {
            		      if(p[0] == 0 && p[1] == 0) break;                       // end of the program
            		      p++;
            		   }
            		   //The fonts or CSUBs binary start on a new four byte boundary following
            		   //at least two 0x00 bytes at the end of the program.(can be 2-5)
            		   // At the end of the program so skip the two 0x00 bytes
            		   p++;
            		   p++;
            		   // Step the program memory up to the first 0xFF of the 4 that mark the beginning of the CSub binaries.
            		   // this will step over any extra 0x00 bytes
            		   while(*p != 0xff) p++;
            		   p++;p++; p++;p++;                                           // step over the rest of header of the four 0xff bytes
            		   // now, skip any CFunctions in program memory and Fonts
            		   while(*(unsigned int *)p != 0xffffffff) {
            		       j = (*(unsigned int *)(p + 4)) + 8;         // calculate the total size of the CFunction
            		       while(j--)  p++;                            // skip all CSUB binary
            		   }
       		           // we have now skipped all the CFunctions and Fonts
            		   //Find the next used section. Its the library
            			     while(p<ProgMemory+PROG_FLASH_SIZE) {
            			    	 if(*p != 0xff){   //Next used word is the library
           			    		    //Library needs Windbond flash. Check its there and not empty.
            			    		 SPIOpen();
            			    		 int pageno;
            			    	     pageno=WBLibAddr;
            			    	     m = GetMemory(256);
            			    		 WBReadPage(pageno,m);
            			    		 if (m[0]!=0xFF || m[1]!=0xFF || m[2]!=0xFF || m[3]!=0xFF){
            			    		   	Option.ProgFlashSize =p - ProgMemory;
            			    		   	SaveOptions();
            			    		   	MMPrintString("Library code restored. \r\n");
            			    		 }else{
            			    			MMPrintString("No library code exists \r\n");
            			    		 }
            			    		 FreeMemory(m);
            			    		 SPIClose();
               			    		 return;
            			    	 }
            			    	p += 4;
            			     }
            			     MMPrintString("No library code exists \r\n");
            			     //Check Windbond Flash is there anyway
            			     SPIOpen();
            			     SPIClose();
            			     return;

            	   }else{
            		     MMPrintString("Library code exists. \r\n");
            		     //Check Windbond Flash is there
            		     SPIOpen();
            		     SPIClose();
            	   }
                   return;
    }

    /********************************************************************************************************************
         ******* LIBRARY PLIB **********************************************************************************************/
#ifdef DUMP
        if(checkstring(cmdline, "PLIB")) {
           if(CurrentLinePtr) error("Invalid in a program");
            char *p;
            if(Option.ProgFlashSize == PROG_FLASH_SIZE) return;
           p = ProgMemory + Option.ProgFlashSize;
           //dump(p-256,256,-1);
           dump(p,256,5000);
           dump(p+256,256,5001);
           dump(p+512,256,5002);
           dump(p+768,256,5003);
           dump(p+1024,256,5004);
           //ListProgram(ProgMemory + Option.PROG_FLASH_SIZE, false);
           return;
        }

        if(checkstring(cmdline, "PPROG")) {
          if(CurrentLinePtr) error("Invalid in a program");
             char *p;
         //  if(Option.PROG_FLASH_SIZE== MAX_PROG_SIZE) return;
           p = ProgMemory ;
           dump(p,256,2000);
           dump(p+256,256,2001);
           dump(p+512,256,2002);
           dump(p+768,256,2003);
           dump(p+1024,256,2004);
           //ListProgram(ProgMemory + Option.PROG_FLASH_SIZE, false);
           return;
        }

         if(checkstring(cmdline, "PFLASH")) {
             int i;
             if(Option.ProgFlashSize == PROG_FLASH_SIZE){
              	 i=16;
             }else{
             	 i=(PROG_FLASH_SIZE-Option.ProgFlashSize)/256 +2; //
             }
            SPIOpen();
            int pageno;
       	    p = GetMemory(256);
            for (pageno=WBLibAddr;pageno<WBLibAddr+i;pageno++){
       	      WBReadPage(pageno,p);dump(p,256,pageno);
       	    }
            FreeMemory(p);
   	        SPIClose();
   	        return;
        }
#endif

    error("Unknown command");
}
    /*
        if(checkstring(cmdline, "READ")) {
        	      int j;
        	      SPIOpen();
        	      int pageno=0;
        	   	  p = GetMemory(256);
        	   	  for (j=0;j<20;j++){
        	   	    pageno=j;
        	   	    WBReadPage(pageno,p);dump(p,256,pageno);pageno++;
        	   	  }
        	      for (j=8064;j<8083;j++){
        	   	    pageno=j;
        	   	    WBReadPage(pageno,p);dump(p,256,pageno);pageno++;
        	   	  }

        	      FreeMemory(p);
        	      SPIClose();
        	      return;
        }
        if(checkstring(cmdline, "READVAR")) {
           	      SPIOpen();
           	      int pageno;
           	   	  p = GetMemory(256);
           	   	  for (pageno=WBVarAddr;pageno<WBVarAddr+16;pageno++){
           	   	      WBReadPage(pageno,p);dump(p,256,pageno);
           	   	  }
           	      FreeMemory(p);
           	      SPIClose();
           	      return;
        }
        if(checkstring(cmdline, "READPROG")) {
              	      int i=0;
              	      char *p;char *pr;
              	   	  p = GetMemory(256);
              	      pr = ProgMemory;
              	          while(i<256) {
              	           p[i]=pr[i];i++;

              	          }
              	        dump(p,256,i);
              	      FreeMemory(p);
              	      return;
        }
        if(checkstring(cmdline, "DEBUG")) {
        	     int i;
        	         if(Option.ProgFlashSize == PROG_FLASH_SIZE){
        	        	 i=16;
        	         }else{
        	        	 i=(PROG_FLASH_SIZE-Option.ProgFlashSize)/256 +2; //
        	         }
               	      SPIOpen();
               	      int pageno;
               	   	  p = GetMemory(256);
               	   	  for (pageno=WBLibAddr;pageno<WBLibAddr+i;pageno++){
               	   	      WBReadPage(pageno,p);dump(p,256,pageno);
               	   	  }
               	      FreeMemory(p);
               	      SPIClose();
               	      return;
        }
      */



/***********************************************************************************
  Copy the library stored in WB flash to the end of the Program memory
  Assumes that the FlashWriteInit() has been called and flash has been erased
  and any program has already been written. Appends starting at Option.ProgFlashSize
 **********************************************************************************/
void AppendLibrary(int msg){
        int i,j,k;
        char *p;
	    if(Option.ProgFlashSize == PROG_FLASH_SIZE)return;
	    //Check library is valid
	    SPIOpen();
	    int pageno;
	    pageno=WBLibAddr;
	    p = GetMemory(256);
#ifdef WRITEHEADERPAGE
	    WBReadPage(pageno,p);

	    if (p[0]!=0xAA || p[1]!=0x55 || p[2]!=0x55 || p[3]!=0xAA)error("Library-Invalid Header");

	    j=p[4]<<16 | p[5]<<8 | p[6];
	    if (j!=Option.ProgFlashSize)error("Library-Size mismatch");

#endif
       //Now append the library code from WindBond Flash
	   FlashSetAddress(Option.ProgFlashSize) ;
	   k=Option.ProgFlashSize;                                      // starting point
       //j=WBLibAddr+1+(PROG_FLASH_SIZE-Option.ProgFlashSize)/256+1;  //calculate last flash page
       j=WBLibAddr+(PROG_FLASH_SIZE-Option.ProgFlashSize)/256+1;  //calculate last flash page
       //for (pageno=WBLibAddr+1;pageno<=j;pageno++){
       for (pageno=WBLibAddr;pageno<=j;pageno++){
  //	   PIntComma(pageno);PIntComma(k); MMPrintString(" \r\n");
         WBReadPage(pageno,p);//dump(p,256,pageno);
         for (i=0;i<256;i++){
      	   if (k < PROG_FLASH_SIZE){
      		  FlashWriteByte(p[i]);k++;
    	   }
         }
       }
       FlashWriteClose();
       FreeMemory(p);
       SPIClose();
      // if(msg) {                                                       // if requested by the caller, print an informative message
      //        if(MMCharPos > 1) PRet();                    // message should be on a new line
      //        MMPrintString("Library Appended ");
      //        IntToStr(tknbuf, PROG_FLASH_SIZE-Option.ProgFlashSize, 10);
      //        MMPrintString(tknbuf);
      //        MMPrintString(" bytes\r\n");
      // }
      //MMPrintString("Library Appended \r\n");
}

/***********************************************************************************
  FLASH commands to read write erase W25Q16
 **********************************************************************************/
#ifdef CMD_FLASH

#define SPICR1 *(volatile unsigned int *)(0x40013000)    //SPI1_CR1 F4
#define SPICR2 *(volatile unsigned int *)(0x40013004)    //SPI1_CR2 F4
#define SPISTAT *(volatile unsigned int *)(0x40013008)   //SPI1_SR  F4
#define SPIBUF *(volatile unsigned int *)(0x4001300c)    //SPI1_DR  F4

void cmd_flash(void){
    char *p;
    char *tp;
  //  char *vp;
    int i,j;
    int userlastpage=7919;

    tp = checkstring(cmdline, "TEST");
    if (tp){
    	 int id;
    	 unsigned char s,r1,r2,r3;
    	 PIntH(SPICR1);PRet();
    	 SPIOpen();
    	 PIntH(SPICR1);PRet();
    	 PIntH(SPISTAT);PRet();

    	  PinSetBit(Option.FLASH_CS,LATCLR);
   	       SPIsend1(JEDEC);
   	       PIntH(SPICR1);PRet();
   	       s=0;
   	       uSec(100);
   	       HAL_SPI_TransmitReceive(&hspi1,&s,&r1,1,500);
   	       uSec(100);
   	       HAL_SPI_TransmitReceive(&hspi1,&s,&r2,1,500);
   	       uSec(100);
   	       HAL_SPI_TransmitReceive(&hspi1,&s,&r3,1,500);
   	       PinSetBit(Option.FLASH_CS,LATSET);
   	       PIntH(r1);PRet();
   	       PIntH(r2);PRet();
   	       PIntH(r3);PRet();
   	       id=r1*256*256+r2*256+r3 ;
   	       PIntH(id);;PRet();

   	       PinSetBit(Option.FLASH_CS,LATSET);
   	       if (id==0x0 || id==0xFFFFFF){SPIClose(); error("No flash.Check Option_FLASH_CS");}
   	       SPIClose();
   	       PIntH(SPICR1);PRet();
   	       return;
    }

    /*
    tp = checkstring(cmdline, "ERASE ALL");
    if (tp){
        if(CurrentLinePtr) error("Invalid in program");
         SPIOpen();
         // p = GetMemory(256);
         WBEraseArea(eraseall,0);
         // FreeMemory(p);
         SPIClose();
         MMPrintString("Flash Erased\r\n");
         //If library existed set it as deleted
         if(Option.ProgFlashSize != PROG_FLASH_SIZE){
        	Option.ProgFlashSize = PROG_FLASH_SIZE;
        	SaveOptions();
        	MMPrintString("Library Deleted\r\n");
        	// Clear Program Memory and also the Library at the end.
        	cmdline = ""; CurrentLinePtr = NULL;    // keep the NEW command happy
            cmd_new();                              //  delete any program,and the library code and return to the command prompt

         }
         return;
    }
    */

    tp = checkstring(cmdline, "ERASE ALL");
       if (tp){
           if(CurrentLinePtr) error("Invalid in program");
            SPIOpen();
            //Erase the 30*64K Blocks
            for (i=0;i<7680;i+=256){
              WBEraseArea(block2erase,i);
              MMPrintString(".");
            }
            //Erase the 32K Block
            i=7680;
            WBEraseArea(blockerase,i);
            MMPrintString(".");
           // Erase the 7*4K Blocks
            for (i=7808;i<7920;i+=16){
             WBEraseArea(sectorerase,i);
             MMPrintString(".");
            }

            SPIClose();
            MMPrintString("\r\nUser Flash Erased (Pages 0-7919)\r\n");
            return;
       }

    /********** ERASE pageno  *************************/
    /* Erases a page first, preserves other pages in the SECTOR */
    /**********************************************************/
       tp = checkstring(cmdline, "ERASE");
       if(tp){
      	// getargs(&tp, 3, ",");
      	 int pageno=getint(tp,0,userlastpage);
      	 int pagebase=pageno & 0xFFFFFFF0;
      	 SPIOpen();
      	 p = GetMemory(4096);
      	 WBReadSector(pagebase,p);
      	 j=(pageno-pagebase)*256;
      	 for (i=0;i<256;i++){
      	    p[j+i]=0xFF;
      	 }
      	 WBEraseArea(sectorerase,pagebase);
      	 WBWriteSector(pagebase,p);
      	 FreeMemory(p);
      	 SPIClose();
      	 return;
       }

    /*************************************************/
    /* FLASH READ pageno,dst%()                      */
    /*************************************************/
    tp = checkstring(cmdline, "READ");
    if(tp){
    	    void *ptr1 = NULL;
    	    int64_t *dest=NULL;
    	   	char *q=NULL;
    	    int i=0,j;
    	    getargs(&tp, 3, ",");
    	   	if(!(argc == 3)) error("Incorrect number of arguments");
    	    ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    	    if(vartbl[VarIndex].type & T_INT) {
    	        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    	        if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
    	       	            error("Argument 1 must be integer array");
    	       	}
    	       dest = (int64_t *)ptr1;
    	       q=(char *)&dest[0];
    	    } else error("Argument 1 must be integer array");
    	    j=(vartbl[VarIndex].dims[0] - OptionBase)+1;
    	    if(j*8 < 256 )error("Source array too small"); // 32*8 =256

   	       int pageno=getint(argv[0],0,userlastpage);
    	   p = GetMemory(256);
   	       SPIOpen();
    	   WBReadPage(pageno,p);
    	   for (i=0;i<256;i++){q[i]=p[i];}
    	   FreeMemory(p);
    	   SPIClose();
    	   return;
    }

    /*************************************************/
    /* FLASH WRITE pageno,src%()                     */
    /*************************************************/
    tp = checkstring(cmdline, "WRITE");
    if(tp){
      void *ptr2 = NULL;
      int64_t  *src=NULL;

      char *q=NULL;
      int i=0,j;
      getargs(&tp, 3, ",");
      if(!(argc == 3)) error("Incorrect number of arguments");

       ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
       if(vartbl[VarIndex].type & T_INT) {
       	      if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
       	      if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
       	           error("Argument 2 must be integer array");
              }
              src = (int64_t *)ptr2;
              q=(char *)&src[0];

       } else error("Argument 2 must be integer array");
       j=(vartbl[VarIndex].dims[0] - OptionBase)+1;
       if(j*8 < 256 )error("Destination array too small"); // 32*8 =256

       int pageno=getint(argv[0],0,userlastpage);
       p = GetMemory(256);
       for (i=0;i<256;i++){p[i]=q[i];}

       SPIOpen();
       WBWritePage(pageno,p);
       FreeMemory(p);
       SPIClose();
       return;
     }

     /********** UPDATE pageno, src%() *************************/
     /* Erases page first, preserves other pages in the SECTOR */
     /**********************************************************/
     tp = checkstring(cmdline, "UPDATE");
     if(tp){
          void *ptr2 = NULL;
          int64_t  *src=NULL;

          char *q=NULL;
          int i=0,j;
          getargs(&tp, 3, ",");
          if(!(argc == 3)) error("Incorrect number of arguments");

           ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
           if(vartbl[VarIndex].type & T_INT) {
           	      if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
           	      if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
           	           error("Argument 2 must be integer array");
                  }
                  src = (int64_t *)ptr2;
                  q=(char *)&src[0];

           } else error("Argument 2 must be integer array");
           j=(vartbl[VarIndex].dims[0] - OptionBase)+1;
           if(j*8 < 256 )error("Destination array too small"); // 32*8 =256

           int pageno=getint(argv[0],0,userlastpage);
           p = GetMemory(256);
           for (i=0;i<256;i++){p[i]=q[i];}

           int pagebase=pageno & 0xFFFFFFF0;
           SPIOpen();
           q= GetMemory(4096);
           WBReadSector(pagebase,q);
           j=(pageno-pagebase)*256;
           for (i=0;i<256;i++){
                q[j+i]=p[i];
           }
           FreeMemory(p);
           WBEraseArea(sectorerase,pagebase);
           WBWriteSector(pagebase,q);
           FreeMemory(q);
           SPIClose();
           return;
     }

     /********** PRINT pageno [, pageend] *************/
     /*************************************************/

     tp = checkstring(cmdline, "PRINT");
     if(tp){
    	 if(CurrentLinePtr) error("Invalid in program");
    	 getargs(&tp,3,",");
    	 if(argc==3){
    	   int pageno=getint(argv[0],0,8191);
    	   int pageend=getint(argv[2],pageno,8191);
    	   SPIOpen();
           p = GetMemory(256);
           for (;pageno<=pageend;pageno++){
             WBReadPage(pageno,p);
             dump(p,256,pageno);
           }

    	 }else{
    		 int pageno=getint(argv[0],0,8191);
    		 SPIOpen();
    	     p = GetMemory(256);
    	     WBReadPage(pageno,p);
    		 dump(p,256,pageno);
     	 }
    	 FreeMemory(p);
    	 SPIClose();
    	 return;
     }
     error("Syntax");
}

#endif


/*	p = checkstring(cmdline, "READPAGE");
            	if(p) {
                	char *q=NULL;
                	int i;
                	uint32_t *qq;
                	q = findvar(p, V_FIND);
                	if(!(vartbl[VarIndex].type & T_STR)) error("Invalid variable");
                	qq=(uint32_t*)q;
            		for(i=0;i<20;i++){
            			qq[i]=HAL_RTCEx_BKUPRead (&hrtc, i);
            		}
            		q=CtoM(q);
            		return;
         	}*/


/*



  if(checkstring(cmdline, "ERASEALL")) {
  	//int pin=77;  //35 or 77 for mini
  	       SPIOpen();
  	      // p = GetMemory(256);
  	       WBEraseArea(eraseall,0);
  	      // FreeMemory(p);
  	       SPIClose();
             return;
  }
  if(checkstring(cmdline, "ERASESECTOR")) {
      	       SPIOpen();
      	       WBEraseArea(sectorerase,0);
      	       SPIClose();
                 return;
  }
  if(checkstring(cmdline, "ERASEBLOCK")) {
         	      SPIOpen();
         	      WBEraseArea(blockerase,8064);
         	      SPIClose();
                return;
  }


*/


/***********************************************************************************
  W25Q16 Support Routines used by library andFlash commands
 **********************************************************************************/
void WBEraseArea(int erasemode,int pageno){

	 int add;
     unsigned char s,q;
     PinSetBit(Option.FLASH_CS,LATCLR);
       SPIsend1(writeenable);
     PinSetBit(Option.FLASH_CS,LATSET);
     uSec(100);


        // MMPrintString("Start Block Erase\r\n");
         //BLOCK ERASE
         add=(pageno<<8); //convert page number to byte number
         PinSetBit(Option.FLASH_CS,LATCLR);
          SPIsend1(erasemode); //
         //PinSetBit(pin,LATSET);
         uSec(100);
         if (erasemode!=eraseall){ //send the address
           //PinSetBit(pin,LATCLR);
            SPIsend1((add>>16) & 0xFF);
            SPIsend1((add>>8) & 0xFF);
            SPIsend1(add & 0xFF);
         }
         PinSetBit(Option.FLASH_CS,LATSET);
    	 uSec(100000);

    	// MMPrintString("Start Wait \r\n");
    	 s=0;
    	 do {
    	         uSec(100000);
    	         PinSetBit(Option.FLASH_CS,LATCLR);
    	          SPIsend1(readstatus1);
    	          HAL_SPI_TransmitReceive(&hspi1,&s,&q,1,500);
    	         PinSetBit(Option.FLASH_CS,LATSET);
    	 } while(q & 1);
    	 PinSetBit(Option.FLASH_CS,LATCLR);
    	 SPIsend1(writedisable);
    	 PinSetBit(Option.FLASH_CS,LATSET);


}
void WBWritePage(int pageno,char *p){

	 int add,i=0;
     unsigned char s,q;
     PinSetBit(Option.FLASH_CS,LATCLR);
       SPIsend1(writeenable);
     PinSetBit(Option.FLASH_CS,LATSET);
     uSec(100);
     add=(pageno<<8); //convert page number to byte number
     PinSetBit(Option.FLASH_CS,LATCLR);
	 SPIsend1(pagewrite);
     SPIsend1((add>>16) & 0xFF);
	 SPIsend1((add>>8) & 0xFF);
	 SPIsend1(add & 0xFF);

	 for (i=0;i<256;i++){
		SPIsend1(p[i]);
       //HAL_SPI_Receive(&hspi1,(uint8_t *)p[i],1,500);  //read a page
	   //HAL_SPI_Receive(&hspi1,(char *)p,256,500);  //read a page
	  }

	  PinSetBit(Option.FLASH_CS,LATSET);
	  uSec(1000);
	 //MMPrintString("Start Wait \r\n");
	  s=0;
	  do {
	      uSec(100);
	      PinSetBit(Option.FLASH_CS,LATCLR);
	        SPIsend1(readstatus1);
	        HAL_SPI_TransmitReceive(&hspi1,&s,&q,1,500);
          PinSetBit(Option.FLASH_CS,LATSET);
	  } while(q & 1);
	  //MMPrintString("End  Wait \r\n");
	  PinSetBit(Option.FLASH_CS,LATCLR);
	    SPIsend1(writedisable);
      PinSetBit(Option.FLASH_CS,LATSET);
}




void WBReadPage(int pageno,char *p){
	// MMPrintString("-------------------------------Read Start ");
	 int add,i=0;
     unsigned char s,r;
     PinSetBit(Option.FLASH_CS,LATCLR);

     add=(pageno<<8); //convert page number to byte number
	 SPIsend1(readdata);
	 SPIsend1((add>>16) & 0xFF);
	 SPIsend1((add>>8) & 0xFF);
	 SPIsend1(add & 0xFF);

	 s=0;
	 for (i=0;i<256;i++){
		 uSec(100);
		 HAL_SPI_TransmitReceive(&hspi1,&s,&r,1,500);
		 p[i]=r;
       //HAL_SPI_Receive(&hspi1,(uint8_t *)p[i],1,500);  //read a page
	   //HAL_SPI_Receive(&hspi1,(char *)p,256,500);  //read a page
	 }


     PinSetBit(Option.FLASH_CS,LATSET);
     //MMPrintString("-------------------------------Read End ");
}

#ifdef CMD_FLASH
/* Write 16 Page Sector  i.e. 4K *****************************/
void WBWriteSector(int pageno,char *p){
	 int i,j;
	 char *q;

     pageno &= 0xfffffff0;
     q = GetMemory(256);
     for (i=0;i<16;i++){
    	 for (j=0;j<256;j++){ q[j]=p[256*i+j];}
    	 WBWritePage( pageno+i, q);
     }
	 FreeMemory(q);
}


/* Read 16 Page Sector i.e. 4K *****************************/
void WBReadSector(int pageno,char *p){
	// MMPrintString("-------------------------------Read Start ");
	 int add,i=0;
     unsigned char s,r;
     PinSetBit(Option.FLASH_CS,LATCLR);
     pageno &= 0xFFFFFFF8;
     add=(pageno<<8); //convert page number to byte number
	 SPIsend1(readdata);
	 SPIsend1((add>>16) & 0xFF);
	 SPIsend1((add>>8) & 0xFF);
	 SPIsend1(add & 0xFF);

	 s=0;
	 for (i=0;i<4096;i++){
		 uSec(100);
		 HAL_SPI_TransmitReceive(&hspi1,&s,&r,1,500);
		 p[i]=r;
       //HAL_SPI_Receive(&hspi1,(uint8_t *)p[i],1,500);  //read a page
	   //HAL_SPI_Receive(&hspi1,(char *)p,256,500);  //read a page
	 }
     PinSetBit(Option.FLASH_CS,LATSET);
     //MMPrintString("-------------------------------Read End ");
}
#endif

/*********************************************************************************************
 Initialise the Flash CS pin so that its does not affect the use of SPI1. The Windbond flash
 is connected to SPI 1 so its CS needs to be high. The CS pin is 35 on the VET6 and 77 on the
 VET6 Mini. Pin 35 is also PWM 1C so can only be used for one of or the other.
 Option FLASH_CS 0 will prevent the CS pin being initialised at startup. Default value is 35.
 *********************************************************************************************/
void InitFlash_CS(){
	if(Option.FLASH_CS !=0)SetAndReserve(Option.FLASH_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);
}

void SPIOpen(void) {
	        //Option.FLASH_CS is 35 for VET6 or 77 for VET6 Mini
	        int id;
		    unsigned char s,r1,r2,r3;
	        if(ExtCurrentConfig[SPI_OUT_PIN] == EXT_COM_RESERVED) error("SPI1 Already open");

	       // hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_16;    //5.25MHz
	       // hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_8;     //10.5MHz
	       hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_4;     //21MHz
	       // hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_2;     //42MHz
	       hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	       hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	       hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;

	       ExtCfg(SPI_OUT_PIN, EXT_COM_RESERVED, 0);
	       ExtCfg(SPI_INP_PIN, EXT_COM_RESERVED, 0);
	       ExtCfg(SPI_CLK_PIN, EXT_COM_RESERVED, 0);
	       HAL_SPI_DeInit(&hspi1);
	       HAL_SPI_Init(&hspi1);

	       PinSetBit(Option.FLASH_CS,LATCLR);
	       SPIsend1(JEDEC);
	       s=0;
	       uSec(100);
	       HAL_SPI_TransmitReceive(&hspi1,&s,&r1,1,500);
	       uSec(100);
	       HAL_SPI_TransmitReceive(&hspi1,&s,&r2,1,500);
	       uSec(100);
	       HAL_SPI_TransmitReceive(&hspi1,&s,&r3,1,500);
	       PinSetBit(Option.FLASH_CS,LATSET);
	       id=r1*256*256+r2*256+r3 ;
	      // PIntH(id);
	       if (id==0x0 || id==0xFFFFFF){SPIClose(); error("No SPI flash found.Check OPTION FLASH_CS");}


}
// erase all battery backup memory and reset the options to their defaults
// used on initial firmware run if options appear corrupt or not set
void ResetAllBackupRam(void) {
	ResetAllOptions();
	SaveOptions();                                     //  and write them to flash
	ClearSavedVars();					           	   // erase saved vars
}

// erase all flash memory clear save vars and reset the options to their defaults
// used  when the user grounds PA? on startup i.e. KEY 1 power up
void ResetAllFlash(void) {
	ResetAllOptions();
	SaveOptions();                                     //  and write them to flash
	ClearSavedVars();					           	   // erase saved vars
    FlashWriteInit(PROGRAM_FLASH);                     // erase program memory
    FlashWriteByte(0); FlashWriteByte(0);              // terminate the program in flash
    FlashWriteClose();
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}
