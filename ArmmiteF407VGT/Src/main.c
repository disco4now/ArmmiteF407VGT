
/*-*****************************************************************************

MMBasic  for STM32F407xGT6 (Armmite F4)
Supports STM32F405RGT6,STM32F407VGT6,STM32F407ZGT6


main.c

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

/* USER CODE BEGIN Header */
/**
  * In addition the software components from STMicroelectronics are provided
  * subject to the license as detailed below:
  *
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define DEFINE_PINDEF_TABLE
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "memory.h"
#include "usbd_cdc.h"
#include "stm32f4xx_hal_rcc_ex.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

DAC_HandleTypeDef hdac;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

CAN_HandleTypeDef hcan;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
UART_HandleTypeDef huart3;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
extern FATFS FatFs;  /* File system object for SD card logical drive */
extern char SDPath[4]; /* SD card logical drive path */
extern void InitFileIO(void);
extern int CurrentSPISpeed;
FIL fil;

struct s_PinDef *PinDef;
int PromptFont, PromptFC, PromptBC;                             // the font and colours selected at the prompt
char BreakKey = BREAK_KEY;                                          // defaults to CTRL-C.  Set to zero to disable the break function
char IgnorePIN = false;
char WatchdogSet = false;
char Feather ;

uint8_t RxBuffer, TxBuffer;
#define progress  "\rprogress\r\n"
int MMCharPos;
int MMPromptPos;
char LCDAttrib;
volatile int MMAbort = false;
int use_uart;

//char lastcmd[STRINGSIZE*4];            // used to store the last command in case it is needed by the EDIT command (leaves about 512bytes in CCRAM after this)
//void InsertLastcmd( char *s);
#define CMD_BUFFER_SIZE STRINGSIZE*4


unsigned int __attribute__((section(".my_section"))) _excep_dummy; // for some reason persistent does not work on the first variable
unsigned int __attribute__((section(".my_section"))) _excep_code;  //  __attribute__ ((persistent));  // if there was an exception this is the exception code
unsigned int __attribute__((section(".my_section"))) _excep_addr;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _excep_cause;  //  __attribute__ ((persistent));  // and this is the address
/********  Enable one of these two lines to determine where to place the CMD_BUFFER  *******/
char __attribute__((section(".backup"))) lastcmd[CMD_BUFFER_SIZE];  //  RTC Battery Backed RAM of 4K  Used for CMD_BUFFER
//char lastcmd[CMD_BUFFER_SIZE];                                    // CMD_BUFFER in CCRAM
/******************************************************************************************/
char *InterruptReturn = NULL;
int BasicRunning = false;
char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
volatile int ConsoleRxBufHead = 0;
volatile int ConsoleRxBufTail = 0;
char ConsoleTxBuf[CONSOLE_TX_BUF_SIZE];
volatile int ConsoleTxBufHead = 0;
volatile int ConsoleTxBufTail = 0;

extern volatile unsigned int ScrewUpTimer;
extern jmp_buf jmprun;

extern void dacclose(void);
extern void ADCclose(void);

uint8_t BlinkSpeed = 0, str[20];
extern void printoptions(void);
uint32_t PROG_FLASH_SIZE = 0x20000;
void initConsole(void);
char SerialConDisabled;

char canopen=0;	  //CAN has no pins assigned

#define DISPLAY_CLS             1
#define REVERSE_VIDEO           3
#define CLEAR_TO_EOL            4
#define CLEAR_TO_EOS            5
#define SCROLL_DOWN             6
#define DRAW_LINE               7
//extern void MX470PutS(char *s, int fc, int bc);
//extern void MX470Cursor(int x, int y);
extern void MX470Display(int fn);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
//#ifdef RGT
//if(HAS_64PINS){
void SystemClock_Config12(void);
//}else{
void SystemClock_Config(void);
//}
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
//static void MX_ADC1_Init(void);
//static void MX_ADC2_Init(void);
//static void MX_ADC3_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM9_Init(void);
//static void MX_UART4_Init(void);
//static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
//static void MX_FSMC_Init(void);
static void MX_RNG_Init(void);
static void MX_CAN1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM5_Init(void);
//static void MX_TIM6_Init(void);
//static void MX_TIM7_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM12_Init(void);
//static void MX_USART6_UART_Init(void);
//static void MX_USART3_UART_Init(void);
static void MX_SDIO_SD_Init(void);
/* USER CODE BEGIN PFP */
//static void myMX_FSMC_Init(void);
extern void myMX_FSMC_Init(void);
void SerialConsolePutC(int c);
int MMgetchar(void);
void CheckAbort(void);
int MMInkey(void);
void shortpause(unsigned int ticks);
void SerUSBPutC(char c);
void SoftReset(void);
int kbhitConsole(void);
int gui_font;
int gui_fcolour;
int gui_bcolour;
void initConsole(void);
void executelocal(char *p);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


void InsertLastcmd( char *s);


  //int terminal_width,terminal_height;
  extern void  setterminal(int height,int width);
  extern void cleanend(void);



  /* cleanend() is called from checkabort() at CNTRL+C and also from cmd_end().
   * It is also called from error() before the command prompt is displayed.
   * It is used to clean up any background tasks that are running and may affected
   * MMBasic at the command prompt before control is returned to the command prompt.
   * It is generally a subset of ClearExternalIO() but attempts to leave the environment
   * in place to allow debugging of the current program and to allow use of the CONTINUE command.
   *
   */



  void cleanend(void){
  	  //int i;
	  dacclose();
	  ADCclose();
    // memset(inpbuf,0,STRINGSIZE);
    // int lastgui=gui_font_height;
  	//SetFont(Option.DefaultFont);
  	//adjust=gui_font_height-lastgui;
     // if(mouse1)i2c_disable();                                                  // close I2C
     // if(mouse2)i2c2_disable();                                                  // close I2C
     // if(mouse3)i2c3_disable();                                                  // close I2C

      //OnKeyGOSUB=NULL;							            // set the next stmt to the interrupt location
     // com1_interrupt=NULL;									// set the next stmt to the interrupt location
     // com1_TX_interrupt=NULL;
     // com2_interrupt=NULL;									// set the next stmt to the interrupt location
     // com2_TX_interrupt=NULL;
     // com3_interrupt=NULL;									// set the next stmt to the interrupt location
     // com3_TX_interrupt=NULL;
    //  com4_interrupt=NULL;									// set the next stmt to the interrupt location
    //  com4_TX_interrupt=NULL;

 //     KeyInterrupt=NULL;									    // set the next stmt to the interrupt location
 //     WAVInterrupt=NULL;									    // set the next stmt to the interrupt location
 //     COLLISIONInterrupt=NULL;							    // set the next stmt to the interrupt location
 //     ADCInterrupt=NULL;									    // set the next stmt to the interrupt location
 //     DACInterrupt=NULL;									    // set the next stmt to the interrupt location
  //    IrInterrupt=NULL;									    // set the next stmt to the interrupt location
  //    for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
  //    	inttbl[i].intp=NULL;							// set the next stmt to the interrupt location
  //    }
  //    for(i = 0; i < NBRSETTICKS; i++) {
  //    	TickInt[i]=NULL;
  //    }
 //     keyselect=0;

  //	  CloseAudio(1);
  //    CloseAllFiles();
      longjmp(mark, 1);
  }

#ifdef DUMP

// dump a memory area to the console
// for debugging
void dump(char *p, int nbr,int page) {
    char buf1[80], buf2[80], *b1, *b2, *pt;
    b1 = buf1; b2 = buf2;
    PRet();
    MMPrintString("---------------------Dump Page ");
    PInt(page);
    MMPrintString("----------------  \r\n");
    MMPrintString("   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF\r\n");
    b1 += sprintf(b1, "%8x: ", (unsigned int)p);
    for(pt = p; (unsigned int)pt % 16 != 0; pt--) {
        b1 += sprintf(b1, "   ");
        b2 += sprintf(b2, " ");
    }
    while(nbr > 0) {
        b1 += sprintf(b1, "%02x ", *p);
        b2 += sprintf(b2, "%c", (*p >= ' ' && *p < 0x7f) ? *p : '.');
        p++;
        nbr--;
        if((unsigned int)p % 16 == 0) {
            MMPrintString(buf1);
            MMPrintString("   ");
            MMPrintString(buf2);
            b1 = buf1; b2 = buf2;
            b1 += sprintf(b1, "\r\n%8x: ", (unsigned int)p);
        }
    }
    if(b2 != buf2) {
        MMPrintString(buf1);
        MMPrintString("   ");
        for(pt = p; (unsigned int)pt % 16 != 0; pt++) {
            MMPrintString("   ");
        }
      MMPrintString(buf2);
    }
    PRet();
    uSec(50000);
}
#endif

/*
// a crude memory dump that does not use sprintf()
void cdump(char *p, int nbr,int page) {
    while(nbr--) {
        if(((int)p & 0b11111) == 0) PRet();
        if(*p == 0)
            MMPrintString("= ");
        else if(*p == T_NEWLINE)
            MMPrintString("@ ");
        else if(*p == T_LINENBR)
            MMPrintString("% ");
        else if(isprint(*p))
            { MMputchar(*p); MMputchar(' '); }
        else MMPrintString(". ");
        p++;
    }
    PRet();
}
*/


/**
 * @brief Transforms input beginning with * into a corresponding RUN command.
 *
 * e.g.
 *   *foo              =>  RUN "foo"
 *   *"foo bar"        =>  RUN "foo bar"
 *   *foo --wombat     =>  RUN "foo", "--wombat"
 *   *foo "wom"        =>  RUN "foo", Chr$(34) + "wom" + Chr$(34)
 *   *foo "wom" "bat"  =>  RUN "foo", Chr$(34) + "wom" + Chr$(34) + " " + Chr$(34) + "bat" + Chr$(34)
 *   *foo --wom="bat"  =>  RUN "foo", "--wom=" + Chr$(34) + "bat" + Chr$(34)
 */
static void transform_star_command(char *input) {
    char *src =  input;
    while (isspace((unsigned char)*src)) src++; // Skip leading whitespace.

    if (*src != '*') error("Internal fault");
    src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace((unsigned char)*end)) *end-- = '\0';

    // Allocate extra space to avoid string overrun.
    char *tmp = (char *) GetTempMemory(STRINGSIZE + 32);
    strcpy(tmp, "RUN");
    char *dst = tmp + 3;

    if (*src == '"') {
        // Everything before the second quote is the name of the file to RUN.
        *dst++ = ' ';
        *dst++ = *src++; // Leading quote.
        while (*src && *src != '"') *dst++ = *src++;
        if (*src == '"') *dst++ = *src++; // Trailing quote.
    } else {
        // Everything before the first space is the name of the file to RUN.
        int count = 0;
        while (*src && !isspace((unsigned char)*src)) {
            if (++count == 1) {
                *dst++ = ' ';
                *dst++ = '\"';
            }
            *dst++ = *src++;
        }
        if (count) *dst++ = '\"';
    }

    while (isspace((unsigned char)*src)) src++; // Skip whitespace.

    // Anything else is arguments.
    if (*src) {
        *dst++ = ',';
        *dst++ = ' ';

        // If 'src' starts with double-quote then replace with: Chr$(34) +
        if (*src == '"') {
            memcpy(dst, "Chr$(34) + ", 11);
            dst += 11;
            src++;
        }

        *dst++ = '\"';

        // Copy from 'src' to 'dst'.
        while (*src) {
            if (*src == '"') {
                // Close current set of quotes to insert a Chr$(34)
                memcpy(dst, "\" + Chr$(34)", 12);
                dst += 12;

                // Open another set of quotes unless this was the last character.
                if (*(src + 1)) {
                    memcpy(dst, " + \"", 4);
                    dst += 4;
                }
                src++;
            } else {
                *dst++ = *src++;
            }
            if (dst - tmp >= STRINGSIZE) error("String too long");
        }

        // End with a double quote unless 'src' ended with one.
        if (*(src - 1) != '"') *dst++ = '\"';

        *dst = '\0';
    }

    if (dst - tmp >= STRINGSIZE) error("String too long");

    // Copy transformed string back into the input buffer.
    memcpy(input, tmp, STRINGSIZE);
    input[STRINGSIZE - 1] = '\0';

    ClearSpecificTempMemory(tmp);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  	ProgMemory=(char*)FLASH_PROGRAM_ADDR;
	int i,pullup;
	Feather=false;
	static int ErrorInPrompt;
	//SOption=(volatile struct option_s *)FLASH_SAVED_OPTION_ADDR;
    SavedMemoryBufferSize=0;
	//RAMBase = (void *)((unsigned int)RAMBASE);
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* Test for Feather 64Pin PA9 and PA10 are high (3.3v)  */
#ifdef FEATHER
    //if(HAS_64PINS){
	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    GPIO_InitTypeDef GPIO_InitStruct;
  	    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  	  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	  	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  	   	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  	    HAL_Delay(200);
  	    if(HAL_GPIO_ReadPin(GPIOA,  GPIO_PIN_9) && HAL_GPIO_ReadPin(GPIOA,  GPIO_PIN_10)){
             Feather = true;
  	  	}else{
  	         Feather = false;
  	  	}
  	  	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  	  	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  	  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	  	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  	// }

#endif
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

if(Feather){
  SystemClock_Config12();   //feather with 12MHz HSE
}else{
  SystemClock_Config();
}


  MX_RTC_Init();

  /* USER CODE BEGIN SysInit */
  HAL_FLASH_Unlock();
  CurrentCpuSpeed=SystemCoreClock;
  PeripheralBusSpeed=SystemCoreClock/2;
//  ResetAllFlash();              // init the options if this is the very first startup
  LoadOptions();
  if(Option.Baudrate == 0 ||
	!(Option.Tab==2 || Option.Tab==4 ||Option.Tab==8) ||
	!(Option.SerialConDisabled==0 || Option.SerialConDisabled==1)
  	  ){
	  // ResetAllFlash();   // init the options if this is the very first startup
	  ResetAllBackupRam();  // We don't need/want to clear Program Memory so just clear saved vars and reset options in the battery Backup Ram
	  _excep_code=0;
  }
  SerialConDisabled=Option.SerialConDisabled;


  RAMBase = (void *)((unsigned int)RAMBASE + (Option.MaxCtrls * sizeof(struct s_ctrl))+ SavedMemoryBufferSize);
  RAMBase = (void *)MRoundUp((unsigned int)RAMBase);
//if(HAS_64PINS)  PinDef = (struct s_PinDef *)PinDef64 ;
PinDef = (struct s_PinDef *)PinDef64 ;
if(HAS_100PINS) PinDef = (struct s_PinDef *)PinDef100 ;
if(HAS_144PINS) PinDef = (struct s_PinDef *)PinDef144 ;

  // setup a pointer to the base of the GUI controls table
  Ctrl = (struct s_ctrl *)RAMBASE;
  for(i = 1; i < Option.MaxCtrls; i++) {
      Ctrl[i].state = Ctrl[i].type = 0;
      Ctrl[i].s = NULL;
  }
  goto skip_init;

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  //MX_GPIO_Init();
 // MX_RTC_Init();
 // MX_USB_DEVICE_Init();
  //MX_ADC1_Init();
  //MX_ADC2_Init();
  //MX_ADC3_Init();
  //MX_DAC_Init();
  //MX_TIM2_Init();
  //MX_TIM3_Init();
 // MX_TIM9_Init();
//  MX_UART4_Init();
//  MX_USART2_UART_Init();
 // MX_TIM4_Init();
//  MX_I2C1_Init();
//  MX_I2C2_Init();
 // MX_USART1_UART_Init();
 // MX_SPI1_Init();
 // MX_SPI2_Init();
  //MX_FSMC_Init();
 // MX_RNG_Init();
 // MX_TIM5_Init();
  //MX_TIM6_Init();
  //MX_TIM7_Init();
 // MX_TIM10_Init();
 // MX_TIM12_Init();
 // MX_USART6_UART_Init();
 // MX_SDIO_SD_Init();
 // MX_FATFS_Init();
//MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  skip_init:
if(HAS_64PINS){
	    MX_GPIO_Init();
	    /* Test for MMBasic Serial Console on PC13 if not feather  */
	 	if (Feather==false)   {
	  		GPIO_InitTypeDef GPIO_InitStruct;
	  		GPIO_InitStruct.Pin = GPIO_PIN_13;
	  		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	  		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	  		HAL_Delay(200);

	  		//if(HAL_GPIO_ReadPin(GPIOC,  GPIO_PIN_13)){
	  		if(HAL_GPIO_ReadPin(GPIOC,  GPIO_PIN_13) && Option.SerialConDisabled){
	  			Option.SerialConDisabled=0;
	  			SaveOptions();
	  			SoftReset();                                                // this will restart the processor

	  		}
	  		GPIO_InitStruct.Pin = GPIO_PIN_13;
	  		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  		GPIO_InitStruct.Pull = GPIO_NOPULL;
	  		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	 	}


	    /* Test for MMBasic Reset on PC01  */
	 	{
	  		GPIO_InitTypeDef GPIO_InitStruct;
	  		GPIO_InitStruct.Pin = GPIO_PIN_1;
	  		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	  		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	  		HAL_Delay(200);

	  		if(HAL_GPIO_ReadPin(GPIOC,  GPIO_PIN_1)){
	  			//FlashWriteInit(PROGRAM_FLASH);    //This is included in ResetAllFlash() so is not required here
	  			ResetAllFlash();
	  		}
	  		GPIO_InitStruct.Pin = GPIO_PIN_1;
	  		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  		GPIO_InitStruct.Pull = GPIO_NOPULL;
	  		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	 	}



	HAL_SRAM_DeInit(&hsram1);
	MX_DAC_Init();
	MX_RNG_Init();
	MX_SPI2_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_TIM5_Init();
    MX_TIM9_Init();
    MX_TIM10_Init();
    MX_TIM12_Init();
	////  	MX_RTC_Init();
	MX_SPI1_Init();
	MX_I2C1_Init();
	if (!Feather)MX_I2C2_Init();   //COM1 is using these Pins so no I2C2 on feather
    // myMX_FSMC_Init();  //Returns immediately
    MX_SDIO_SD_Init();

  initExtIO();

   if(Option.SerialConDisabled==0 && Feather==false){
		SetAndReserve(COM1_TX_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
		SetAndReserve(COM1_RX_PIN, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
		MX_USART1_UART_Init();
		HAL_UART_DeInit(&huart1);
		huart1.Init.BaudRate = Option.Baudrate;
		HAL_UART_Init(&huart1);
		HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);
	}

/*
 	if(Option.SerialConDisabled==0){
 		SetAndReserve(COM1_TX_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
 		SetAndReserve(COM1_RX_PIN, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
 		MX_USART3_UART_Init();
 		HAL_UART_DeInit(&huart3);
 		huart3.Init.BaudRate = Option.Baudrate;
 		HAL_UART_Init(&huart3);
 		HAL_UART_Receive_IT(&huart3, &RxBuffer, 1);
 	}

*/

  HAL_TIM_Base_Start(&htim2);
  initConsole();
  InitDisplaySPI(1);

 // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);
	if(Option.SerialConDisabled){
        //PA11 and PA12 are USB D- and USB D+ i.e. USB CONSOLE
		GPIO_InitTypeDef GPIO_InitStruct;
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);
		GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		HAL_Delay(100);
		pullup=GPIOA->IDR & GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, RESET);
		HAL_Delay(500);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
		MX_USB_DEVICE_Init();
      if(!pullup)USB_DevConnect(USB_OTG_FS);
	}

	//MX_USB_DEVICE_Init();
   // if(!pullup)USB_DevConnect(USB_OTG_FS);
	HAL_Delay(1000);
 // initKeyboard();
 // InitDisplaySSD(1);
  InitBasic();
  //InitTouch();
  InitFlash_CS();    //Initialise the CS for the Windbond flash so it doesn't interfere with SPI1
  InitHeap();
}else{ //VGT100 pin  and ZGT 144 pin
  	MX_GPIO_Init();
 	{
  		GPIO_InitTypeDef GPIO_InitStruct;
  		GPIO_InitStruct.Pin = KEY0_Pin;
  		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  		GPIO_InitStruct.Pull = GPIO_PULLUP;
  		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  		if(!HAL_GPIO_ReadPin(GPIOE,  GPIO_PIN_4) && Option.SerialConDisabled){
  			Option.SerialConDisabled=0;
  			SaveOptions();
  		    SoftReset();                                                // this will restart the processor
  		}
  		GPIO_InitStruct.Pin = KEY0_Pin;
  		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  		GPIO_InitStruct.Pull = GPIO_NOPULL;
  		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
 	}
 	{
  		GPIO_InitTypeDef GPIO_InitStruct;
  		GPIO_InitStruct.Pin = KEY1_Pin;
  		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  		GPIO_InitStruct.Pull = GPIO_PULLUP;
  		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  		if(!HAL_GPIO_ReadPin(GPIOE,  GPIO_PIN_3)){
  			//FlashWriteInit(PROGRAM_FLASH);    //This is included in ResetAllFlash() so is not required here
  			ResetAllFlash();
  		}
  		GPIO_InitStruct.Pin = KEY1_Pin;
  		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  		GPIO_InitStruct.Pull = GPIO_NOPULL;
  		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
 	}
   	HAL_SRAM_DeInit(&hsram1);
 	MX_DAC_Init();
  	MX_RNG_Init();
  	MX_SPI2_Init();
  	MX_TIM1_Init();
  	MX_TIM2_Init();
  	MX_TIM3_Init();
  	MX_TIM4_Init();
  	MX_TIM5_Init();
    MX_TIM9_Init();
    MX_TIM10_Init();
    MX_TIM12_Init();
	//  	MX_RTC_Init();
  	MX_SPI1_Init();
  	MX_I2C1_Init();
  	MX_I2C2_Init();
    //myMX_FSMC_Init();
    //myMX_FSMC_DeInit();
    MX_SDIO_SD_Init();
    MX_CAN1_Init();
    initExtIO();
	if(Option.SerialConDisabled==0){
		SetAndReserve(COM1_TX_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
		SetAndReserve(COM1_RX_PIN, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
		MX_USART1_UART_Init();
		HAL_UART_DeInit(&huart1);
		huart1.Init.BaudRate = Option.Baudrate;
		HAL_UART_Init(&huart1);
		HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);
	}
    HAL_TIM_Base_Start(&htim2);
    initConsole();
    InitDisplaySPI(1);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);
	if(Option.SerialConDisabled){
  		GPIO_InitTypeDef GPIO_InitStruct;
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);
		GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		HAL_Delay(100);
		pullup=GPIOA->IDR & GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, RESET);
		HAL_Delay(500);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
		MX_USB_DEVICE_Init();
        if(!pullup)USB_DevConnect(USB_OTG_FS);
	}
  	HAL_Delay(1000);
    initKeyboard();
    InitDisplaySSD(1);
    InitBasic();
    InitTouch();
    InitFlash_CS();    //Initialise the CS for the Windbond flash so it doesn't interfere with SPI1
    InitHeap();
}
   // InitFileIO();
    BasicRunning = true;
    ErrorInPrompt = false;
    ErrorInPrompt = false;
    /********************** Only print the banner if not one of these events  **************/
    if(!(_excep_code == RESTART_NOAUTORUN || _excep_code == RESET_COMMAND || _excep_code == WATCHDOG_TIMEOUT || _excep_code == SCREWUP_TIMEOUT || _excep_code == RESTART_HEAP)){
  	  if(Option.Autorun==0 ){
  		  MMPrintString(MES_SIGNON);            // print sign on message
  		  if(Feather) MMPrintString(" (RGT6 64 pins [Feather])");
  		  if(HAS_64PINS && !Feather) MMPrintString(" (RGT6 64 pins)");
  		  if(HAS_100PINS) MMPrintString(" (VGT6 100 pins)");
  		  if(HAS_144PINS) MMPrintString(" (ZGT6 144 pins)");
  		 // PInt(Feather);
  		  PIntHC(package);PIntComma(flashsize & 0xFFFF);
  		  MMPrintString(COPYRIGHT);                                   // print copyright message
  		  PRet();
  	  }
    }
    if(_excep_code == RESTART_HEAP) {
		MMPrintString("Error: Heap overrun\r\n");
    }
    if(_excep_code == WATCHDOG_TIMEOUT) {
        WatchdogSet = true;                                 // remember if it was a watchdog timeout
        MMPrintString("\r\n\nWatchdog timeout\r\n");
    }
    if(_excep_code == SCREWUP_TIMEOUT) {
    	   MMPrintString("\r\n\nCommand timeout\r\n");
    }
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2047);
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2047);
    *tknbuf = 0;
    if(HAS_64PINS && Feather){
    	HAL_NVIC_DisableIRQ(USART3_IRQn);
    }else{
	  if(Option.SerialConDisabled){
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	  } else {
		HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);
	  }
    }
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_NVIC_DisableIRQ(UART4_IRQn);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
    HAL_NVIC_DisableIRQ(TIM5_IRQn);
    HAL_NVIC_DisableIRQ(TIM7_IRQn);
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
    if (HAL_TIM_Base_Start_IT(&htim12) != HAL_OK)
    {
      /* Starting Error */
        error("HAL_TIM_Base_Start_IT");
    }
    if(setjmp(mark) != 0) {
        // we got here via a long jump which means an error or CTRL-C or the program wants to exit to the command prompt

    	ScrewUpTimer=0;
    	optionangle=1.0;

        ContinuePoint = nextstmt;                                   // in case the user wants to use the continue command
        *tknbuf = 0;                                                // we do not want to run whatever is in the token buffer
         memset(inpbuf,0,STRINGSIZE);
    } else {
          if(_excep_cause != CAUSE_MMSTARTUP) {
              ClearProgram();
              PrepareProgram(true);
              _excep_cause = CAUSE_MMSTARTUP;
#if defined(TEST_CONFIG)
            CurrentLinePtr = inpbuf;
            strcpy(inpbuf, TEST_CONFIG);
            multi=false;
            tokenise(true);
            ExecuteProgram(tknbuf);
            memset(inpbuf,0,STRINGSIZE);
#endif
              if(FindSubFun("MM.STARTUP", 0) >= 0) ExecuteProgram("MM.STARTUP\0");
              _excep_cause = CAUSE_NOTHING;
              //if(Option.Autorun && *ProgMemory == 0x01 && _excep_code != RESTART_NOAUTORUN) {
              if((Option.Autorun && *ProgMemory == 0x01 && _excep_code != RESTART_NOAUTORUN) || (_excep_code==RESTART_DOAUTORUN && *ProgMemory == 0x01)) {
            	  //Fix from picomite for lockup withe execute command and OPTION AUTORUN ON
            	  //if(Option.ProgFlashSize == PROG_FLASH_SIZE) ExecuteProgram(ProgMemory + Option.ProgFlashSize);       // run anything that might be in the library
                  //ExecuteProgram(ProgMemory);                                                                          // then run the program if autorun is on
                  *tknbuf=GetCommandValue((char *)"RUN");
                  goto autorun;
              }
          }
    }
    SerUSBPutS("\033[?1000l");                         // Tera Term turn off mouse click report in vt200 mode
    CurrentSPISpeed=NONE_SPI_SPEED;
// Diagnostic use
//    MMPrintString((pullup? "Pullup is on\r\n":"pullup is off\r\n"));
//    if(HAS_100PINS)MMPrintString("STM32F407\r\n");
//    if(HAS_64PINS)MMPrintString("STM32F405\r\n");
//
//     PIntH(chipID);PIntHC(package);PIntComma(flashsize & 0xFFFF);PRet();
//
//   ExtCfg(2, EXT_DIG_OUT, 0);
//   PinSetBit(2,LATCLR);
//   while(1){
//  	 LL_GPIO_SetOutputPin(GPIOE,GPIO_PIN_3);
//  	 uSec(10);
//  	 LL_GPIO_ResetOutputPin(GPIOE,GPIO_PIN_3);
//  	 uSec(10);
//   }
//	MX_FATFS_Init();
//    if(f_mount(&FatFs, (TCHAR const*)SDPath, 1) != FR_OK)FATFS_UnLinkDriver(SDPath);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if(Option.DISPLAY_CONSOLE) {
          SetFont(PromptFont);
          gui_fcolour = PromptFC;
          gui_bcolour = PromptBC;
          if(CurrentX != 0) PRet();                   // prompt should be on a new line
      }
      MMAbort = false;
        BreakKey = BREAK_KEY;
        EchoOption = true;
        LocalIndex = 0;                                             // this should not be needed but it ensures that all space will be cleared
        ClearTempMemory();                                           // clear temp string space (might have been used by the prompt)
        CurrentLinePtr = NULL;                                      // do not use the line number in error reporting
        if(MMCharPos > 1) PRet();                    // prompt should be on a new line
          while(Option.PIN && !IgnorePIN) {
            _excep_code = PIN_RESTART;
            if(Option.PIN == 99999999)                              // 99999999 is permanent lockdown
                MMPrintString("Console locked, press enter to restart: ");
            else
                MMPrintString("Enter PIN or 0 to restart: ");
            MMgetline(0, inpbuf);
            if(Option.PIN == 99999999) SoftReset();
            if(*inpbuf != 0) {
                uSec(3000000);
                i = atoi(inpbuf);
                if(i == 0) SoftReset();
                if(i == Option.PIN) {
                    IgnorePIN = true;
                    break;
                }
            }
        }
        _excep_code = 0;
        PrepareProgram(false);
        if(!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
            ErrorInPrompt = true;
            ExecuteProgram("MM.PROMPT\0");
            MMPromptPos=MMCharPos-1;    //Save length of prompt
        } else {
            MMPrintString("> ");                                    // print the prompt
            MMPromptPos=2;    //Save length of prompt
        }
        ErrorInPrompt = false;
        EditInputLine();          //Enter|Recall|Edit the command line. Save to command history
        if(!*inpbuf) continue;                                      // ignore an empty line
        char *p=inpbuf;
        skipspace(p);
        executelocal(p);
        if(*p=='*'){ //shortform RUN command so convert to a normal version
            transform_star_command(inpbuf);
            p = inpbuf;
        }
        multi=false;
        tokenise(true);                                             // turn into executable code
autorun:
        i=0;
        if(*tknbuf==GetCommandValue((char *)"RUN"))i=1;
        if (setjmp(jmprun) != 0) {
            PrepareProgram(false);
            CurrentLinePtr = 0;
        }

        if(*tknbuf == T_LINENBR)                                    // don't let someone use line numbers at the prompt
            tknbuf[0] = tknbuf[1] = tknbuf[2] = ' ';                // convert the line number into spaces
        ExecuteProgram(tknbuf);                                     // execute the line straight away
        if(i){
            cmdline = "";      //Clear the cmdline so that cmd_end() does not complain.
            cmd_end();
        }else {
            memset(inpbuf,0,STRINGSIZE);
       	    longjmp(mark, 1);												// jump back to the input prompt
        }

        /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  //PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_RCC_BKPSRAM_CLK_ENABLE();
  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();
}

void SystemClock_Config12(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  //PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_RCC_BKPSRAM_CLK_ENABLE();
  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();
}
#ifdef XXX
/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

#endif

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */
  /** DAC Initialization 
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config 
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT2 config 
  */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_14TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
	int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
	int calibrate= -Option.RTC_Calibrate;
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
goto skiprtc;
  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
skiprtc:;
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 7;
  hrtc.Init.SynchPrediv = 4095;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
	  Error_Handler();
  }

  if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
	  Error_Handler();
  }
  RtcGetTime();
  if(sDate.Year<18){
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  }
  if(Option.RTC_Calibrate>0){
	  up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
	  calibrate=512-Option.RTC_Calibrate;
  }
  HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 5;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}
static void MX_TIM1_Init(void)
{

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 167;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
	Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
	Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
	{
	Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
	Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM2;
	sConfigOC.Pulse = 980;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
	{
	Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
	Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
		/* PWM Generation Error */
		error("HAL_TIM_PWM_Start");
	}
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 20000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 20000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}
#ifdef xxx
/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 10000;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 10000;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 0;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

#endif

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 0;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 20000;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */
  HAL_TIM_MspPostInit(&htim9);

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 1679;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 50;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}
static void MX_TIM12_Init(void)
{

  /* USER CODE BEGIN TIM12_Init 0 */

  /* USER CODE END TIM12_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 83;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 65535;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */

}
#ifdef xxx
/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

#endif

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
#ifdef XXX
/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
#endif

#ifdef XXX
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

#endif

#ifdef XXX
/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */


static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

#endif

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
if(HAS_64PINS){
	 /* GPIO Ports Clock Enable */
	//  __HAL_RCC_GPIOE_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	 // __HAL_RCC_GPIOD_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	//  HAL_GPIO_WritePin(GPIOB, PWM_1D_LCD_BL_Pin|T_CS_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(Drive_VBUS_FS_GPIO_Port, Drive_VBUS_FS_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pins : IR_Pin KEY1_Pin KEY0_Pin PE1 */
	//  GPIO_InitStruct.Pin = IR_Pin|KEY1_Pin|KEY0_Pin|GPIO_PIN_1;
	//  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	//  GPIO_InitStruct.Pull = GPIO_NOPULL;
	//  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	 //   GPIO_InitStruct.Pin = GPIO_PIN_1;
	 // 	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	 //   GPIO_InitStruct.Pull = GPIO_NOPULL;
	 //   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : SDIO_CD_Pin T_PEN_Pin */
	  GPIO_InitStruct.Pin = SDIO_CD_Pin|T_PEN_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PWM_1D_LCD_BL_Pin T_CS_Pin */
	  GPIO_InitStruct.Pin = /*PWM_1D_LCD_BL_Pin|*/T_CS_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /*Configure GPIO pin : Drive_VBUS_FS_Pin */
	  GPIO_InitStruct.Pin = Drive_VBUS_FS_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(Drive_VBUS_FS_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pins : PA8 KBD_CLK_Pin */
	  GPIO_InitStruct.Pin = GPIO_PIN_8|KBD_CLK_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : KBD_DATA_Pin */
	  GPIO_InitStruct.Pin = KBD_DATA_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(KBD_DATA_GPIO_Port, &GPIO_InitStruct);


}else{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOB, PWM_1D_LCD_BL_Pin|T_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Drive_VBUS_FS_GPIO_Port, Drive_VBUS_FS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : IR_Pin KEY1_Pin KEY0_Pin PE1 */
  GPIO_InitStruct.Pin = IR_Pin|KEY1_Pin|KEY0_Pin|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : SDIO_CD_Pin T_PEN_Pin */
  GPIO_InitStruct.Pin = SDIO_CD_Pin|T_PEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PWM_1D_LCD_BL_Pin T_CS_Pin */
  GPIO_InitStruct.Pin = /*PWM_1D_LCD_BL_Pin|*/T_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Drive_VBUS_FS_Pin */
  GPIO_InitStruct.Pin = Drive_VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Drive_VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 KBD_CLK_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_8|KBD_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : KBD_DATA_Pin */
  GPIO_InitStruct.Pin = KBD_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KBD_DATA_GPIO_Port, &GPIO_InitStruct);

}
}
#ifdef XXX
/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}
#endif

/* USER CODE BEGIN 4 */
/* FSMC initialization function */
//static void myMX_FSMC_Init(void)
void myMX_FSMC_Init(void)
{
 if(HAS_64PINS)return;
 FSMC_NORSRAM_TimingTypeDef WTiming, RTiming;

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  if(Option.DISPLAY_TYPE==ILI9341_16 || Option.DISPLAY_TYPE==IPS_4_16 ){

	  WTiming.AddressSetupTime = 2;
	  WTiming.AddressHoldTime = 12;
	  WTiming.DataSetupTime = 7;
	  WTiming.BusTurnAroundDuration = 0;
	  WTiming.CLKDivision = 2;
	  WTiming.DataLatency = 2;
	  WTiming.AccessMode = FSMC_ACCESS_MODE_A;

	  RTiming.AddressSetupTime = 12;
	  RTiming.AddressHoldTime = 6;
	  if(Option.DISPLAY_TYPE==ILI9341_16){
	    RTiming.DataSetupTime = 36;  //was 36
	  }else{
		RTiming.DataSetupTime = 24;  //was 36
      }
	  RTiming.BusTurnAroundDuration = 1;
	  RTiming.CLKDivision = 2;
	  RTiming.DataLatency = 2;
	  RTiming.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

	  if (HAL_SRAM_Init(&hsram1, &RTiming, &WTiming) != HAL_OK)
	  {
		  Error_Handler( );
	  }

  }  else {
  		  WTiming.AddressSetupTime = 8;
  		  WTiming.AddressHoldTime = 30;
  		  WTiming.DataSetupTime = 14;
  		  WTiming.BusTurnAroundDuration = 1;
  		  WTiming.CLKDivision = 16;
  		  WTiming.DataLatency = 17;
  		  WTiming.AccessMode = FSMC_ACCESS_MODE_A;
  		  if (HAL_SRAM_Init(&hsram1, &WTiming, &WTiming) != HAL_OK)
  		  {
  			  Error_Handler( );
  		  }
  	  }

}
// send a character to the Console serial port
void SerialConsolePutC(int c) {

	if(c == '\b') {
			    	if (MMCharPos==1){
			            //MMCharPos = 80;
			            //SerUSBPutS("\e[1A");SerUSBPutS("\e[80C");
			            //MMCharPos = 80;
			            //return;
			    	}else{
			    		MMCharPos -= 1;
			    	}
	}
	if(Option.SerialConDisabled==0){
		int empty=(huart1.Instance->SR & USART_SR_TC) | !(huart1.Instance->CR1 & USART_CR1_TCIE) ;
		while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE)); //wait if buffer full
		ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
		ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
		if(empty){
			huart1.Instance->CR1 |= USART_CR1_TCIE;
		}
	} else {
		while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE)  ); //wait if buffer full
		HAL_SuspendTick();
		ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
		ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
		HAL_ResumeTick();
	}

}
// get a char from the UART1 serial port (the console)
// will return immediately with -1 if there is no character waiting
int __attribute__ ((optimize("-O2"))) getConsole(void) {
    int c=-1;
    CheckAbort();
	if(Option.SerialConDisabled==0){
		huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
			c = ConsoleRxBuf[ConsoleRxBufTail];
			ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
		}
		huart1.Instance->CR1 |= USART_CR1_RXNEIE;
	} else {
	    if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
	    	c = ConsoleRxBuf[ConsoleRxBufTail];
	    	ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
	    }
	}
    return c;
}
void putConsole(int c) {
    SerialConsolePutC(c);
    DisplayPutC(c);
}
int __attribute__ ((optimize("-O2"))) kbhitConsole(void) {
    int i;
    i = ConsoleRxBufHead - ConsoleRxBufTail;
    if(i < 0) i += CONSOLE_RX_BUF_SIZE;
    return i;
}
// print a char on the Serial and USB consoles only (used in the EDIT command and dp() macro)
void SerUSBPutC(char c) {
    SerialConsolePutC(c);
}
// print a string on the Serial and USB consoles only (used in the EDIT command and dp() macro)
void SerUSBPutS(char *s) {
    while(*s) SerUSBPutC(*s++);
}
void initConsole(void) {
    ConsoleRxBufHead = ConsoleRxBufTail = 0;

}

void SoftReset(void){
	NVIC_SystemReset();
}

void inline __attribute__((always_inline))  CheckAbort(void) {
    if(MMAbort) {
        WDTimer = 0;                                                // turn off the watchdog timer
        //longjmp(mark, 1);                                         // jump back to the input prompt
        cleanend();
    }
}
/*****************************************************************************************
The vt100 escape code sequences
===============================
3 char codes            Arrow Up    esc [ A
                        Arrow Down  esc [ B
                        Arrow Right esc [ C
                        Arrow Left  esc [ D

4 char codes            Home        esc [ 1 ~
                        Insert      esc [ 2 ~
                        Del         esc [ 3 ~
                        End         esc [ 4 ~
                        Page Up     esc [ 5 ~
                        Page Down   esc [ 6 ~

5 char codes            F1          esc [ 1 1 ~
                        F2          esc [ 1 2 ~
                        F3          esc [ 1 3 ~
                        F4          esc [ 1 4 ~
                        F5          esc [ 1 5 ~         note the
                        F6          esc [ 1 7 ~         disconnect
                        F7          esc [ 1 8 ~
                        F8          esc [ 1 9 ~
                        F9          esc [ 2 0 ~
                        F10         esc [ 2 1 ~         note the
                        F11         esc [ 2 3 ~         disconnect
                        F12         esc [ 2 4 ~

                        SHIFT-F3    esc [ 2 5 ~         used in the editor

*****************************************************************************************/

// check if there is a keystroke waiting in the buffer and, if so, return with the char
// returns -1 if no char waiting
// the main work is to check for vt100 escape code sequences and map to Maximite codes
int MMInkey(void) {
    unsigned int c = -1;                                            // default no character
    unsigned int tc = -1;                                           // default no character
    unsigned int ttc = -1;                                          // default no character
    static unsigned int c1 = -1;
    static unsigned int c2 = -1;
    static unsigned int c3 = -1;
    static unsigned int c4 = -1;

    if(c1 != -1) {                                                  // check if there are discarded chars from a previous sequence
        c = c1; c1 = c2; c2 = c3; c3 = c4; c4 = -1;                 // shuffle the queue down
        return c;                                                   // and return the head of the queue
    }

    c = getConsole();                                               // do discarded chars so get the char
    if(c == 0x1b) {
        InkeyTimer = 0;                                             // start the timer
        while((c = getConsole()) == -1 && InkeyTimer < 30);         // get the second char with a delay of 30mS to allow the next char to arrive
        if(c == 'O'){   //support for many linux terminal emulators
            while((c = getConsole()) == -1 && InkeyTimer < 50);        // delay some more to allow the final chars to arrive, even at 1200 baud
            if(c == 'P') return F1;
            if(c == 'Q') return F2;
            if(c == 'R') return F3;
            if(c == 'S') return F4;
            if(c == '2'){
                while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
                if(tc == 'R') return F3 + 0x20;
                c1 = 'O'; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
            }
            c1 = 'O'; c2 = c; return 0x1b;                 // not a valid 4 char code
        }
        if(c != '[') { c1 = c; return 0x1b; }                       // must be a square bracket
        while((c = getConsole()) == -1 && InkeyTimer < 50);         // get the third char with delay
        if(c == 'A') return UP;                                     // the arrow keys are three chars
        if(c == 'B') return DOWN;
        if(c == 'C') return RIGHT;
        if(c == 'D') return LEFT;
        if(c < '1' && c > '6') { c1 = '['; c2 = c; return 0x1b; }   // the 3rd char must be in this range
        while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
        if(tc == '~') {                                             // all 4 char codes must be terminated with ~
            if(c == '1') return HOME;
            if(c == '2') return INSERT;
            if(c == '3') return DEL;
            if(c == '4') return END;
            if(c == '5') return PUP;
            if(c == '6') return PDOWN;
            c1 = '['; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
        }
        while((ttc = getConsole()) == -1 && InkeyTimer < 90);       // get the 5th char with delay
        if(ttc == '~') {                                            // must be a ~
            if(c == '1') {
                if(tc >='1' && tc <= '5') return F1 + (tc - '1');   // F1 to F5
                if(tc >='7' && tc <= '9') return F6 + (tc - '7');   // F6 to F8
            }
            if(c == '2') {
                if(tc =='0' || tc == '1') return F9 + (tc - '0');   // F9 and F10
                if(tc =='3' || tc == '4') return F11 + (tc - '3');  // F11 and F12
                if(tc =='5') return F3 + 0x20;                      // SHIFT-F3
            }
        }
        // nothing worked so bomb out
        c1 = '['; c2 = c; c3 = tc; c4 = ttc;
        return 0x1b;
    }
    return c;
}

#define SetupTime 22
void uSec(unsigned int us) {
  us=us*84-SetupTime;
  WriteCoreTimer(0);
  while (ReadCoreTimer()<us);
}
void shortpause(unsigned int ticks){
	  WriteCoreTimer(0);
	  while (ReadCoreTimer()<ticks);
}
void stripcomment(char *p){
    char *q=p;
    int toggle=0;
    while(*q){
        if(*q=='\'' && toggle==0){
            *q=0;
            break;
        }
        if(*q=='"')toggle^=1;
        q++;
    }
}
/*
void testlocal(char *p, char *command, void (*func)()){
    int len=strlen(command);
    if((strncasecmp(p,command,len)==0) && (strlen(p)==len || p[len]==' ' || p[len]=='\'')){
        p+=len;
        skipspace(p);
        cmdline=GetTempMemory(STRINGSIZE);
        stripcomment(p);
        strcpy(cmdline,p);
        (*func)();
        memset(inpbuf,0,STRINGSIZE);
        longjmp(mark, 1);												// jump back to the input prompt
    }

}
*/

void testlocal(char *p, char *command, void (*func)()){
    int len=strlen(command);
    if((strncasecmp(p,command,len)==0) && (strlen(p)==len || p[len]==' ' || p[len]=='\'')){
        p+=len;
        skipspace(p);
        cmdline=GetTempMemory(STRINGSIZE);
        stripcomment(p);
        strcpy(cmdline,p);
        char *q=cmdline;
        int toggle=0;
        while(*q){
            if(*q++ == '"') {
            toggle ^=1;
            }
        }
        if(toggle)cmdline[strlen(cmdline)]='"';
        (*func)();
        memset(inpbuf,0,STRINGSIZE);
        longjmp(mark, 1);                                                                                                                                                                 // jump back to the input prompt
    }
}



void executelocal(char *p){
    testlocal(p,"FILES",cmd_files);
   // testlocal(p,"UPDATE FIRMWARE",cmd_update);
    testlocal(p,"NEW",cmd_new);
    testlocal(p,"AUTOSAVE",cmd_autosave);
}

// takes a pointer to RAM containing a program (in clear text) and writes it to program flash in tokenised format
void SaveProgramToFlash(char *pm, int msg) {
    char *p, endtoken, fontnbr, prevchar = 0, buf[STRINGSIZE];
    int nbr, i, n, SaveSizeAddr;
    multi=false;
    uint32_t storedupdates[MAXCFUNCTION], updatecount=0, realflashsave;

    memcpy(buf, tknbuf, STRINGSIZE);                                // save the token buffer because we are going to use it
    FlashWriteInit(PROGRAM_FLASH);                                   // initialise for flash and erase any pages

    nbr = 0;
    // this is used to count the number of bytes written to flash
    while(*pm) {
        p = inpbuf;
        while(!(*pm == 0 || *pm == '\r' || (*pm == '\n' && prevchar != '\r'))) {
            if(*pm == TAB) {
                do {*p++ = ' ';
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                } while((p - inpbuf) % 2);
            } else {
                if(IsPrint((uint8_t)*pm)) {
                    *p++ = *pm;
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                }
            }
            prevchar = *pm++;
        }
        if(*pm) prevchar = *pm++;                                   // step over the end of line char but not the terminating zero
        *p = 0;                                                     // terminate the string in inpbuf

        if(*inpbuf == 0 && (*pm == 0 || (!IsPrint((uint8_t)*pm) && pm[1] == 0))) break; // don't save a trailing newline

        tokenise(false);                                            // turn into executable code
        p = tknbuf;
        while(!(p[0] == 0 && p[1] == 0)) {
            FlashWriteByte(*p++); nbr++;

            if((realflashpointer - (uint32_t)ProgMemory) >= Option.ProgFlashSize - 5)
                goto exiterror1;
        }
        FlashWriteByte(0); nbr++;                              // terminate that line in flash
    }
    FlashWriteByte(0);
    FlashWriteAlign();                                            // this will flush the buffer and step the flash write pointer to the next word boundary
    // now we must scan the program looking for CFUNCTION/CSUB/DEFINEFONT statements, extract their data and program it into the flash used by  CFUNCTIONs
     // programs are terminated with two zero bytes and one or more bytes of 0xff.  The CFunction area starts immediately after that.
     // the format of a CFunction/CSub/Font in flash is:
     //   Unsigned Int - Address of the CFunction/CSub in program memory (points to the token representing the "CFunction" keyword) or NULL if it is a font
     //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
     //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  Omitted in a font.
     //   word1..wordN - The CFunction/CSub/Font code
     // The next CFunction/CSub/Font starts immediately following the last word of the previous CFunction/CSub/Font
    int firsthex=1;

    realflashsave= realflashpointer;
    p = (char *)ProgMemory;                                              // start scanning program memory
    while(*p != 0xff) {
    	nbr++;
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program
        if(*p == T_NEWLINE) {
            CurrentLinePtr = p;
            p++;                                                    // skip the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // step over the line number

        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
            if(*p == GetCommandValue("DefineFont")) {
             endtoken = GetCommandValue("End DefineFont");
             p++;                                                // step over the token
             skipspace(p);
             if(*p == '#') p++;
             fontnbr = getint(p, 1, FONT_TABLE_SIZE);
                                                 // font 6 has some special characters, some of which depend on font 1
             if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6 or 7");
             realflashpointer+=4;
             skipelement(p);                                     // go to the end of the command
             p--;
            } else {
            	if(*p == cmdCFUN)
            	  endtoken = GetCommandValue("End CFunction");
            	else
            	  endtoken = GetCommandValue("End CSub");

                realflashpointer+=4;
                fontnbr = 0;
                firsthex=0;
            }
             SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             realflashpointer+=4;
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart((uint8_t)*p))  error("Function name");
                 do { p++; } while(isnamechar((uint8_t)*p));
                 skipspace(p);
                 if(!(IsxDigit((uint8_t)p[0]) && IsxDigit((uint8_t)p[1]) && IsxDigit((uint8_t)p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit((uint8_t)*p)) error("Invalid hex word");
                         //if((int)((char *)realflashpointer - ProgMemory) >= PROG_FLASH_SIZE - 5) error("Not enough memory");
                         if((int)((char *)realflashpointer - ProgMemory) >= Option.ProgFlashSize - 5) error("Not enough memory 7");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }
                     realflashpointer+=4;
                     skipspace(p);
                     if(firsthex){
                    	 firsthex=0;
                    	 if(((n>>16) & 0xff) < 0x20)error("Can't define non-printing characters");
                     }
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                     CurrentLinePtr = p;
                     p++;                                            // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                         // skip over the line number
                 skipspace(p);
             } while(*p != endtoken);
             storedupdates[updatecount++]=realflashpointer - SaveSizeAddr - 4;
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }

    realflashpointer = realflashsave ;
    updatecount=0;
    p = (char *)ProgMemory;                                              // start scanning program memory
     while(*p != 0xff) {
     	nbr++;
         if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
         if(*p == 0) break;                                          // end of the program
         if(*p == T_NEWLINE) {
             CurrentLinePtr = p;
             p++;                                                    // skip the newline token
         }
         if(*p == T_LINENBR) p += 3;                                 // step over the line number

         skipspace(p);
         if(*p == T_LABEL) {
             p += p[1] + 2;                                          // skip over the label
             skipspace(p);                                           // and any following spaces
         }
         if(*p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
         if(*p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
             endtoken = GetCommandValue("End DefineFont");
             p++;                                                // step over the token
             skipspace(p);
             if(*p == '#') p++;
             fontnbr = getint(p, 1, FONT_TABLE_SIZE);
                                                 // font 6 has some special characters, some of which depend on font 1
             if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6, or 7");

             FlashWriteWord(fontnbr - 1);             // a low number (< FONT_TABLE_SIZE) marks the entry as a font
             skipelement(p);                                     // go to the end of the command
             p--;
         } else {
        	 if(*p == cmdCFUN)
        	   	endtoken = GetCommandValue("End CFunction");
        	 else
        	    endtoken = GetCommandValue("End CSub");

        	 FlashWriteWord((unsigned int)p);               // if a CFunction/CSub save a pointer to the declaration
        	 fontnbr = 0;
         }
            SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             FlashWriteWord(storedupdates[updatecount++]);                        // leave this blank so that we can later do the write
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart((uint8_t)*p))  error("Function name");
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)) error("Invalid hex word");
                         if((int)((char *)realflashpointer - ProgMemory) >= Option.ProgFlashSize - 5) error("Not enough memory 7");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }

                     FlashWriteWord(n);
                     skipspace(p);
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;                                        // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 skipspace(p);
             } while(*p != endtoken);
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
     FlashWriteWord(0xffffffff);                                // make sure that the end of the CFunctions is terminated with an erased word
     FlashWriteClose();                                              // this will flush the buffer and step the flash write pointer to the next word boundary
    // AppendLibrary(false);
    if(msg) {                                                       // if requested by the caller, print an informative message
        if(MMCharPos > 1) PRet();                    // message should be on a new line
        MMPrintString("Saved ");
        IntToStr(tknbuf, nbr + 3, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");
    }
    memcpy(tknbuf, buf, STRINGSIZE);                                // restore the token buffer in case there are other commands in it
    initConsole();
    return;

    // we only get here in an error situation while writing the program to flash
    exiterror1:
        FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
        FlashWriteClose();
        error("Not enough memory 7");
}

// get a keystroke from the console.  Will wait forever for input
// if the char is a cr then replace it with a newline (lf)
int MMgetchar(void) {
    int c;
    static char prevchar = 0;

    loopback:
    do {
    	ShowCursor(true);
    	uSec(50);
        CheckSDCard();
        processgps();
        c = MMInkey();
    } while(c == -1);
    if(c == '\n' && prevchar == '\r') {
        prevchar = 0;
        goto loopback;
    }
    ShowCursor(false);
    prevchar = c;
    if(c == '\n') c = '\r';
    return c;
}
// put a character out to the serial console
char MMputchar(char c) {
    putConsole(c);
    if(IsPrint(c)) MMCharPos++;
    if(c == '\r') {
        MMCharPos = 1;
    }

    return c;
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

// insert a string into the start of the lastcmd buffer.
// the buffer is a sequence of strings separated by a zero byte.
// using the up arrow usere can call up the last few commands executed.
void InsertLastcmd( char *s) {
int i, slen;
    if(strcmp(lastcmd, s) == 0) return;                             // don't duplicate
    slen = strlen(s);
    if(slen < 1 || slen > CMD_BUFFER_SIZE - 1) return;
    slen++;
    for(i = CMD_BUFFER_SIZE - 1; i >=  slen ; i--)
        lastcmd[i] = lastcmd[i - slen];                             // shift the contents of the buffer up
    strcpy(lastcmd, s);                                             // and insert the new string in the beginning
    for(i = CMD_BUFFER_SIZE - 1; lastcmd[i]; i--) lastcmd[i] = 0;             // zero the end of the buffer
}

// Debug display onto LCDPANEL
/*
void show(int i,int pos,char *s){

	 char tt[20];
	 int x,y;
	 x=CurrentX;y=CurrentY;
	  IntToStr(tt, i, 10);
      strcat(tt,"      ");
      GUIPrintString(50*pos, 0, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, s);
	  GUIPrintString(50*pos, 20, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, tt);
   CurrentX=x;CurrentY=y;
}
*/
void EditInputLine(void) {
    char *p = NULL;
    char buf[MAXKEYLEN + 3];
    char goend[10];
 //   char linelen[10];
    int lastcmd_idx, lastcmd_edit;
    int insert, /*startline,*/ maxchars;
    int CharIndex, BufEdited;
    int c, i, j;
    int l4,l3,l2;
    maxchars=255; //i.e. 3 lines on Vt100 width 80 -2 characters for prompt.
    if(Option.DISPLAY_CONSOLE && Option.Width<=SCREENWIDTH){     //We will always assume the Vt100 is 80 colums if LCD is the console <=80.
       l2=SCREENWIDTH+1-MMPromptPos;
       l3=2*SCREENWIDTH+2-MMPromptPos;
       l4=3*SCREENWIDTH+3-MMPromptPos;
     }else{                         // otherwise assume the VT100 matches Option.Width
       l2=Option.Width +1-MMPromptPos;
       l3=2*Option.Width+2-MMPromptPos;
       l4=3*Option.Width+3-MMPromptPos;
     }

     // Build "\e[80C" equivalent string for the line length
     strcpy(goend,"\e[");IntToStr(&goend[strlen(goend)],l2+MMPromptPos, 10);strcat(goend, "C");

    MMPrintString(inpbuf);                                                              // display the contents of the input buffer (if any)
    CharIndex = strlen(inpbuf);                                                         // get the current cursor position in the line
    insert = false;

    lastcmd_edit = lastcmd_idx = 0;
    BufEdited = false; //(CharIndex != 0);
    while(1) {
    	c=MMgetchar(); //need this as it checks SDCARD and GPS
        if(c == TAB) {
            strcpy(buf, "        ");
            switch (Option.Tab) {
              case 2:
                buf[2 - (CharIndex % 2)] = 0; break;
              case 3:
                buf[3 - (CharIndex % 3)] = 0; break;
              case 4:
                buf[4 - (CharIndex % 4)] = 0; break;
              case 8:
                buf[8 - (CharIndex % 8)] = 0; break;
            }
        } else {
            buf[0] = c;
            buf[1] = 0;

        }
        do {
            switch(buf[0]) {
                case '\r':
                case '\n':
                            goto saveline;
                            break;

                case '\b':
                	        if(CharIndex > 0) {
                	           BufEdited = true;
                               i = CharIndex - 1;
                               j= CharIndex;
                               for(p = inpbuf + i; *p; p++) *p = *(p + 1);                       // remove the char from inpbuf

                                    // Lets put the cursor at the beginning of where the command is displayed.
                                    // backspace to the beginning of line

                               while(j)  {
                                  // MMputchar('\b');
                                  if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                  j--;
                               }
                               fflush(stdout);
                               MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen
                               j=0;
                               while(j < strlen(inpbuf)) {
                                     MMputchar(inpbuf[j]);
                                     if((j==l4-1 || j==l3-1 || j==l2-1 ) && j == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                                     if((j==l4-1 || j==l3-1 || j==l2-1 ) && j < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[j+1]);SerUSBPutS("\b");}
                                     j++;
                               }
                               fflush(stdout);

                               // return the cursor to the right position
                               for(j = strlen(inpbuf); j > i; j--){
                                  if (j==l4 || j==l3 || j==l2) {DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                               }
                               CharIndex--;
                               fflush(stdout);
                               if(strlen(inpbuf)==0)BufEdited = false;
                            }
                            break;


                /****************************************** <--- arrow *****************************************************/
                case CTRLKEY('S'):
                case LEFT:

                	    BufEdited = true;
                	    insert=false; //left at first char will turn OVR on
                	    if(CharIndex > 0) {
                                if (CharIndex==l4 || CharIndex==l3 || CharIndex==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                                insert=true; //Any left turns on INS
                                CharIndex--;
                         }
                         break;

                /****************************************** --->  arrow *****************************************************/
                case CTRLKEY('D'):
                case RIGHT:

                	  if(CharIndex < strlen(inpbuf)) {
                	   	BufEdited = true;
                	    MMputchar(inpbuf[CharIndex]);
                	    if((CharIndex==l4-1 || CharIndex==l3-1|| CharIndex==l2-1 ) && CharIndex == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                	    if((CharIndex==l4-1 || CharIndex==l3-1|| CharIndex==l2-1 ) && CharIndex < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[CharIndex+1]);SerUSBPutS("\b");}
                        CharIndex++;
                      }

                      insert=false; //right always switches to OVER
                      break;

                /*********************************************DEL ********************************************************/
                case CTRLKEY(']'):
                case DEL:
                	      if(CharIndex < strlen(inpbuf)) {
                	           BufEdited = true;
                	           i = CharIndex;

                	           for(p = inpbuf + i; *p; p++) *p = *(p + 1);                 // remove the char from inpbuf
                	           j = strlen(inpbuf);
                	           // Lets put the cursor at the beginning of where the command is displayed.
                                // backspace to the beginning of line
                                j=CharIndex;
                                while(j)  {
                                  if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                  j--;
                                }
                                fflush(stdout);
                                MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen
                               j=0;
                               while(j < strlen(inpbuf)) {
                                    MMputchar(inpbuf[j]);
                                    if((j==l4-1 || j==l3-1 || j==l2-1 ) && j == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                                    if((j==l4-1 || j==l3-1 || j==l2-1 ) && j < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[j+1]);SerUSBPutS("\b");}
                                    j++;
                               }
                               fflush(stdout);
                               // return the cursor to the right position
                               for(j = strlen(inpbuf); j > i; j--){
                                    if (j==l4 || j==l3 || j==l2) {DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                               }
                               fflush(stdout);
                           }
                	       break;

                /*********************************************INS ********************************************************/
                case CTRLKEY('N'):
                case INSERT:insert = !insert;

                           break;

               /*********************************************HOME ********************************************************/
                case CTRLKEY('U'):
                case HOME:
                	         BufEdited = true;
                	         if(CharIndex > 0) {
                                if(CharIndex == strlen(inpbuf)) {
                                    insert = true;
                                }
                                // backspace to the beginning of line
                                while(CharIndex)  {
                                	 if (CharIndex==l4 || CharIndex==l3 || CharIndex==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                                   	 CharIndex--;
                                }
                                fflush(stdout);
                            }else{                            //HOME @ home turns off edit mode
                            	BufEdited = false;
                            	insert=false;                   //home at first char will turn OVR on
                            }
                            break;
              /*********************************************END ********************************************************/
                case CTRLKEY('K'):
                case END:
                	        BufEdited = true;
                	        while(CharIndex < strlen(inpbuf))
                                MMputchar(inpbuf[CharIndex++]);
                            fflush(stdout);
                            break;

             /****************************************Function Keys ***************************************************/
                case 0x91:      //F1
                    break;
                case 0x92:      //F2
                    strcpy(&buf[1],"RUN\r\n");
                    break;
                case 0x93:       //F3
                    strcpy(&buf[1],"LIST\r\n");
                    break;
                case 0x94:       //F4
                    strcpy(&buf[1],"EDIT\r\n");
                    break;
               case 0x95:

            	    /*** F5 will clear LCDPANEL and the VT100  ***/
            	      SerUSBPutS("\e[2J\e[H");
            	      fflush(stdout);
            	      if(Option.DISPLAY_CONSOLE){MX470Display(DISPLAY_CLS);CurrentX=0;CurrentY=0;}
            	      MMPrintString("> ");
            	      fflush(stdout);
                      break;
               // case 0x96:
               //     if(*Option.F6key)strcpy(&buf[1],Option.F6key);
               //     break;
               // case 0x97:
               //     if(*Option.F7key)strcpy(&buf[1],Option.F7key);
               //     break;
               // case 0x98:
               //     if(*Option.F8key)strcpy(&buf[1],Option.F8key);
               //     break;
               // case 0x99:
               //     if(*Option.F9key)strcpy(&buf[1],Option.F9key);
               //     break;
                case 0x9a:   //F10
                    strcpy(&buf[1],"AUTOSAVE\r\n");
                    break;
                case 0x9b:   //F11
                    strcpy(&buf[1],"XMODEM RECEIVE\r\n");
                    break;
                 case 0x9c:  //F12
                    strcpy(&buf[1],"XMODEM SEND\r\n");
                    break;

                                     /*  ^          */
                                     /* /|\         */
                case CTRLKEY('E'):   /*  | Up arrow */
                case UP:    if(!(BufEdited /*|| autoOn || CurrentLineNbr */)) {

                	            if(lastcmd_edit) {
                                    i = lastcmd_idx + strlen(&lastcmd[lastcmd_idx]) + 1;    // find the next command
                                    if(lastcmd[i] != 0 && i < CMD_BUFFER_SIZE - 1) lastcmd_idx = i;  // and point to it for the next time around
                                } else
                                    lastcmd_edit = true;
                                strcpy(inpbuf, &lastcmd[lastcmd_idx]);                      // get the command into the buffer for editing
                                goto insert_lastcmd;
                            }
                            break;

                                    /*  |               */
                case CTRLKEY('X'):  /* \|/  Down Arrow  */
                case DOWN:  if(!(BufEdited)) {

                                if(lastcmd_idx == 0)
                                    *inpbuf = lastcmd_edit = 0;
                                else {
                                    for(i = lastcmd_idx - 2; i > 0 && lastcmd[i - 1] != 0; i--);// find the start of the previous command
                                    lastcmd_idx = i;                                        // and point to it for the next time around
                                    strcpy(inpbuf, &lastcmd[i]);                            // get the command into the buffer for editing
                                }
                                goto insert_lastcmd;                                        // gotos are bad, I know, I know
                            }
                            break;

                insert_lastcmd:                                                             // goto here if we are just recalling a command from buffer

				        // If NoScroll and its near the bottom then clear screen and write command at top
				          // if(Option.NoScroll && Option.DISPLAY_CONSOLE && (CurrentY + 2*gui_font_height >= VRes)){
				          if(Option.NoScroll && Option.DISPLAY_CONSOLE && (CurrentY + (2 + strlen(inpbuf)/Option.Width)*gui_font_height >= VRes)){
				             ClearScreen(gui_bcolour);CurrentX=0;CurrentY=0;
				             if(FindSubFun(( char *)"MM.PROMPT", 0) >= 0) {
				            	 SerUSBPutS("\r");
				                 ExecuteProgram(( char *)"MM.PROMPT\0");
				             } else{
				            	SerUSBPutS("\r");
				                MMPrintString("> ");                           // print the prompt
				             }

				          }else{

			                  // Lets put the cursor at the beginning of where the command is displayed.
                              // backspace to the beginning of line

				              j=CharIndex;
                              while(j)  {
                                  if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                  j--;
                              }
                              fflush(stdout);
                              MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen
                         }

				         CharIndex = strlen(inpbuf);
                         MMPrintString(inpbuf);                                          // display the line
                         if(CharIndex==l4 || CharIndex==l3 || CharIndex==l2){SerUSBPutS(" ");SerUSBPutS("\b");}
                         fflush(stdout);
                         CharIndex = strlen(inpbuf);                                     // get the current cursor position in the line
                         break;


                /********************************************* Other Keys ********************************************************/
                default:    if(buf[0] >= ' ' && buf[0] < 0x7f) {
                                //BufEdited = true;                                           // this means that something was typed
                                i = CharIndex;
                                j = strlen(inpbuf);
                                if(insert) {
                                    if(strlen(inpbuf) >= maxchars - 1) break;               // sorry, line fulljust ignore
                                    for(p = inpbuf + strlen(inpbuf); j >= CharIndex; p--, j--) *(p + 1) = *p;
                                    inpbuf[CharIndex] = buf[0];                             // insert the char
                                    MMPrintString(&inpbuf[CharIndex]);                      // display new part of the line
                                    CharIndex++;
                                    for(j = strlen(inpbuf); j > CharIndex; j--){
                                        if (j==l4 || j==l3 || j==l2){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                    }
                                    fflush(stdout);                                   // return the cursor to the right position

                                } else {
                                	if(strlen(inpbuf) >= maxchars-1 ) break;               // sorry, line full  just ignore
                                    inpbuf[strlen(inpbuf) + 1] = 0;                         // incase we are adding to the end of the string
                                    inpbuf[CharIndex++] = buf[0];                           // overwrite the char
                                    MMputchar(buf[0]);                                      // display it
                                    if(j==l4-1 || j==l3-1 || j==l2-1){SerUSBPutS(" ");SerUSBPutS("\b");}
                                    fflush(stdout);
                                }
                                i = CharIndex;
                                j = strlen((const char *)inpbuf);

                                // If its going to scroll then clear screen
                                if(Option.NoScroll && Option.DISPLAY_CONSOLE){
                                   if(CurrentY + 2*gui_font_height >= VRes) {
                                      ClearScreen(gui_bcolour);/*CurrentX=0*/;CurrentY=0;
                                      CurrentX = (MMPromptPos-2)*gui_font_width  ;
                                      DisplayPutC('>');
                                      DisplayPutC(' ');
                                      DisplayPutS((char *)inpbuf);                      // display the line

                                    }
                                }

                            }
                            break;
            }
            for(i = 0; i < MAXKEYLEN + 1; i++) buf[i] = buf[i + 1];                             // shuffle down the buffer to get the next char
        } while(*buf);
        if(CharIndex == strlen(inpbuf)) {insert = false;}

    }

    saveline:
    PRet();
    BufEdited = false;
    if(strlen(inpbuf) < maxchars)InsertLastcmd(inpbuf);

}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
