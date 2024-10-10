/*-*****************************************************************************

ArmmiteF4 MMBasic

External.c

Handles reading and writing to the digital and analog input/output pins ising the SETPIN and PIN commands

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
#include "main.h"
#include "audio.h"
//#include "stm32f4xx_ll_adc.h"
/* Definition for ADCx clock resources */

#define TRIGGEREDMODE                   ADC_TRIGGEREDMODE_SINGLE_TRIGGER   /* A single trigger for all channel oversampled conversions */
#define OVERSAMPLINGSTOPRESET           ADC_REGOVERSAMPLING_CONTINUED_MODE /* Oversampling buffer maintained during injection sequence */
#define DEFINE_PINDEF_TABLE

int ExtCurrentConfig[NBR_PINS_MAXCHIP + 1];
//unsigned char ADCbits[55];
volatile unsigned char ADCbits[MAX_ANALOGUE_PIN_PACKAGE+1];

volatile int INT1Count, INT1Value, INT1InitTimer, INT1Timer;
volatile int INT2Count, INT2Value, INT2InitTimer, INT2Timer;
volatile int INT3Count, INT3Value, INT3InitTimer, INT3Timer;
volatile int INT4Count, INT4Value, INT4InitTimer, INT4Timer;
//volatile uint64_t INT5Count, INT5Value, INT5InitTimer, INT5Timer;
GPIO_InitTypeDef GPIO_InitDef, IR_InitDef;
int InterruptUsed;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim14;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern void MX_TIM8_Init(void);
extern TIM_HandleTypeDef htim8;
//extern volatile uint64_t Count5High;
extern void dacclose(void);
extern void ADCclose(void);
extern void CNInterrupt(void);
extern int CurrentSPISpeed;
extern void CallCFuncInt1(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt1;                                      // we should call the CFunction Int1 function if this is non zero
extern void CallCFuncInt2(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt2;                                      // we should call the CFunction Int2 function if this is non zero
extern void CallCFuncInt3(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt3;                                      // we should call the CFunction Int3 function if this is non zero
extern void CallCFuncInt4(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt4;                                      // we should call the CFunction Int4 function if this is non zero
extern volatile int CSubComplete;
extern char *KeyInterrupt;
extern volatile int Keycomplete;
extern int keyselect;

extern SPI_HandleTypeDef hspi1;
uint32_t synctime=0;
extern volatile e_CurrentlyPlaying CurrentlyPlaying;
TIM_HandleTypeDef htim14;
//static void MX_TIM14_Init(void)
static void MX_TIM14_Init(int prescale)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
 // htim14.Init.Prescaler = 3;
  htim14.Init.Prescaler = prescale;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 0xFFFF;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}
const MMFLOAT ADCdiv[13]={0,0,0,0,0,0,0,0,255.0,0,1023.0,0,4095.0,};
int ADC_init(int32_t pin)
{
	int ADCinuse=1,resolution;
	resolution=ADC_RESOLUTION_12B;
	if(ADCbits[pin]==10)resolution=ADC_RESOLUTION_10B;
	if(ADCbits[pin]==8)resolution=ADC_RESOLUTION_8B;
//	  memset(&hadc2,0,sizeof(ADC_HandleTypeDef));
	  /*##-1- Configure the ADC peripheral #######################################*/
	  if(pin>0){
		  if(PinDef[pin].ADCn==ADC2) {
			  ADCinuse=2;
		  }
		  else if(PinDef[pin].ADCn==ADC3) {
			  ADCinuse=3;
		  } else {
			  ADCinuse=1;
		  }
	  } else if(pin==0) {
		  ADCinuse=1;
		  hadc1.Init.Resolution  = resolution;
	  }
	  if(ADCinuse==1){
		  hadc1.Instance = ADC1;
		  hadc1.Init.Resolution=resolution;
		  hadc1.Init.ClockPrescaler           = ADC_CLOCK_SYNC_PCLK_DIV2;      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc1.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc1.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc1.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc1.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  if (HAL_ADC_Init(&hadc1) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


	  } else if(ADCinuse==2){
		  hadc2.Instance = ADC2;
		  hadc2.Init.Resolution=resolution;
		  hadc2.Init.ClockPrescaler           = ADC_CLOCK_SYNC_PCLK_DIV2;      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc2.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc2.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc2.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc2.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc2.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  if (HAL_ADC_Init(&hadc2) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


		  /* Run the ADC calibration in single-ended mode */
	  }	else {
		  hadc3.Instance = ADC3;
		  hadc3.Init.Resolution=resolution;
		  hadc3.Init.ClockPrescaler           = ADC_CLOCK_SYNC_PCLK_DIV2;      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc3.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc3.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc3.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc3.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc3.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  if (HAL_ADC_Init(&hadc3) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


	  }
	  if(pin>0){
		  GPIO_InitTypeDef GPIO_InitStruct;
		  /*##-2- Configure peripheral GPIO ##########################################*/
		  /* ADC Channel GPIO pin configuration */
		  GPIO_InitStruct.Pin = PinDef[pin].bitnbr;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitStruct);
	  }
	  return ADCinuse;
}


extern inline void PinSetBit(int pin, unsigned int offset) {
	switch (offset){
	case LATCLR:
		LL_GPIO_ResetOutputPin(PinDef[pin].sfr, PinDef[pin].bitnbr);
		return;
	case LATSET:
		LL_GPIO_SetOutputPin(PinDef[pin].sfr, PinDef[pin].bitnbr);
		return;
	case LATINV:
		LL_GPIO_TogglePin(PinDef[pin].sfr, PinDef[pin].bitnbr);
		return;
	case TRISSET:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	case TRISCLR:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as output with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	case CNPUSET:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_PULLUP; //set as input with pullup
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitDef.Alternate = LL_GPIO_AF_0;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	    return;
	case CNPDSET:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_PULLDOWN; //set as input with no pullup or down
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	    return;
	case CNPUCLR:
	case CNPDCLR:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	case ODCCLR:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	case ODCSET:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	default: error("Unknown PinSetBit command");
	}
}
// return the value of a pin's input
int PinRead(int pin) {
    return  PinDef[pin].sfr->IDR & PinDef[pin].bitnbr? 1: 0;
}
// return a pointer to the pin's sfr register
volatile unsigned int GetPinStatus(int pin) {
    return (PinDef[pin].sfr->ODR>>PinDef[pin].bitnbr) & 1;
}
// return an integer representing the bit number in the sfr corresponding to the pin's bit
int GetPinBit(int pin) {
    return PinDef[pin].bitnbr;
}
void WriteCoreTimer(unsigned long timeset){
	__HAL_TIM_SET_COUNTER(&htim2, timeset);
}
unsigned long ReadCoreTimer(void){
	return __HAL_TIM_GET_COUNTER(&htim2);
}
unsigned long readusclock(void){
	return __HAL_TIM_GET_COUNTER(&htim2)/84;
}
void writeusclock(unsigned long timeset){
	__HAL_TIM_SET_COUNTER(&htim2, timeset*84);
}


/*******************************************************************************************
External I/O related commands in MMBasic
========================================
These are the functions responsible for executing the ext I/O related  commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

Each function is responsible for decoding a command
all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a
basic function) so, if you want to search for the function responsible for the LOCATE command
look for cmd_name

There are 4 items of information that are setup before the command is run.
All these are globals.

int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				y=spi(1,2,3command can do with it is save it or change it to some other location.

char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

The only actions a command can do to change the program flow is to change nextstmt or
execute longjmp(mark, 1) if it wants to abort the program.

********************************************************************************************/

uint8_t aMAP[16]={23,24,25,26,29,30,31,32,67,68,69,70,71,72,76,77};
uint8_t bMAP[16]={35,36,37,89,90,91,92,93,95,96,47,48,51,52,53,54};
uint8_t cMAP[16]={15,16,17,18,33,34,63,64,65,66,78,79,80,7,8,9};
uint8_t dMAP[16]={81,82,83,84,85,86,87,88,55,56,57,58,59,60,61,62};
uint8_t eMAP[16]={97,98,1,2,3,4,5,38,39,40,41,42,43,44,45,46};

uint8_t aMAP144[16]={23,24,25,26,29,30,31,32,67,68,69,70,71,72,76,77};
uint8_t bMAP144[16]={35,36,37,89,90,91,92,93,95,96,47,48,51,52,53,54};
uint8_t cMAP144[16]={15,16,17,18,33,34,63,64,65,66,78,79,80,7,8,9};
uint8_t dMAP144[16]={81,82,83,84,85,86,87,88,55,56,57,58,59,60,61,62};
uint8_t eMAP144[16]={97,98,1,2,3,4,5,38,39,40,41,42,43,44,45,46};
uint8_t fMAP144[16]={81,82,83,84,85,86,87,88,55,56,57,58,59,60,61,62};
uint8_t gMAP144[16]={97,98,1,2,3,4,5,38,39,40,41,42,43,44,45,46};

uint8_t aMAP64[16]={14,15,16,17,20,21,22,23,41,42,43,44,45,46,49,50};
uint8_t bMAP64[16]={26,27,28,55,56,57,58,59,61,62,29,30,33,34,35,36};
uint8_t cMAP64[16]={8,9,10,11,24,25,37,38,39,40,51,52,53,2,3,4};


//if(HAS_100PINS){;}

int codemap(char code, int pin){
	if(HAS_100PINS){
		if(code=='A' || code=='a'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)aMAP[pin];
		} else if(code=='B' || code=='b'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)bMAP[pin];
		} else if(code=='C' || code=='c'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)cMAP[pin];
		} else if(code=='D' || code=='d'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)dMAP[pin];
		} else if(code=='E' || code=='e'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)eMAP[pin];
		}
	}else if(HAS_144PINS){
		if(code=='A' || code=='a'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)aMAP144[pin];
		} else if(code=='B' || code=='b'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)bMAP144[pin];
		} else if(code=='C' || code=='c'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)cMAP144[pin];
		} else if(code=='D' || code=='d'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)dMAP144[pin];
		} else if(code=='E' || code=='e'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)eMAP144[pin];
		} else if(code=='F' || code=='f'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)fMAP144[pin];
		} else if(code=='G' || code=='g'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)gMAP144[pin];
		}

	}else if(HAS_64PINS){
		if(code=='A' || code=='a'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)aMAP64[pin];
		} else if(code=='B' || code=='b'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)bMAP64[pin];
		} else if(code=='C' || code=='c'){
			if(pin>15 || pin<0) error("Invalid pin");
			return (int)cMAP64[pin];
		}
	}

	return 0;
}
int codecheck(char *line){
	char code=0;
	if(*line=='P' || *line=='p'){
		line++;
		if((*line>='A' && *line<='E') || (*line>='a' && *line<='e')){
			code=*line;
			line++;
			if(!IsDigit((uint8_t)*line)){ //check for a normal variable
				code=0;
				line-=2;
			}
		}
	}
	return code;
}

// this is invoked as a command (ie, pin(3) = 1)
// first get the argument then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pin accordingly
	void cmd_pin(void) {
		int pin, value;
		char code;
		if((code=codecheck(cmdline)))cmdline+=2;
		pin = getinteger(cmdline);
		if(code)pin=codemap(code, pin);
	    if(IsInvalidPin(pin)) error("Invalid pin");
		while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
		if(!*cmdline) error("Invalid syntax");
		++cmdline;
		if(!*cmdline) error("Invalid syntax");
		value = getinteger(cmdline);
		ExtSet(pin, value);
	}


// this is invoked as a function (ie, x = pin(3) )
void fun_pin(void) {
	int ADCinuse;
	ADC_ChannelConfTypeDef sConfig;
	memset(&sConfig,0,sizeof(ADC_ChannelConfTypeDef));
	int pin;
	if(checkstring(ep, "BAT")){
		ADC_init(0);
	      sConfig.Channel      = ADC_CHANNEL_VBAT;             /* Sampled channel number */
		  sConfig.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
		  sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;    /* Sampling time (number of clock cycles unit) */
		  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		  {
		    /* Channel Configuration Error */
			    error("HAL_ADC_ConfigChannel");
		  }
	    if (HAL_ADC_Start(&hadc1) != HAL_OK)
	    {
	      /* Start Conversation Error */
	      error("HAL_ADC_Start");
	    }

	    /*##-4- Wait for the end of conversion #####################################*/
	    /*  For simplicity reasons, this example is just waiting till the end of the
	        conversion, but application may perform other tasks while conversion
	        operation is ongoing. */
	    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
	    {
	      /* End Of Conversion flag not set on time */
	        error("HAL_ADC_PollForConversion");
	    }
	    else
	    {
	      /* ADC conversion completed */
	      /*##-5- Get the converted value of regular channel  ########################*/
	      fret = (MMFLOAT)(HAL_ADC_GetValue(&hadc1))/(MMFLOAT)0XFFF * VCC * (MMFLOAT)2.0;
	    }
		if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
		{
		    /* ADC de-initialization Error */
		    error("HAL_ADC_DeInit");
		}


	    targ = T_NBR;
	    return;
	}
	if(checkstring(ep, "TEMP")){
		uint16_t *TS_CAL1=(uint16_t *)0x1FFF7A2C;
		uint16_t *TS_CAL2=(uint16_t *)0x1FFF7A2E;
		ADC_init(0);
	      sConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;             // Sampled channel number
		  sConfig.Rank         = 1;          // Rank of sampled channel number ADCx_CHANNEL
		  sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;    // Sampling time (number of clock cycles unit)
		  sConfig.Offset = 0;                                 // Parameter discarded because offset correction is disabled

		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		  {
		    // Channel Configuration Error
			    error("HAL_ADC_ConfigChannel");
		  }
	    if (HAL_ADC_Start(&hadc1) != HAL_OK)
	    {
	      // Start Conversation Error
	      error("HAL_ADC_Start");
	    }

	    //##-4- Wait for the end of conversion #####################################
	    //  For simplicity reasons, this example is just waiting till the end of the
	    //    conversion, but application may perform other tasks while conversion
	    //    operation is ongoing.
	    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
	    {
	      // End Of Conversion flag not set on time
	        error("HAL_ADC_PollForConversion");
	    }
	    else
	    {
	      // ADC conversion completed
	      //##-5- Get the converted value of regular channel  ########################
	    	int tt=HAL_ADC_GetValue(&hadc1);
	    	fret = ((MMFLOAT)(80.0)) / ((MMFLOAT)(*TS_CAL2-*TS_CAL1)) * (((MMFLOAT)(tt))-((MMFLOAT)*TS_CAL1)) + ((MMFLOAT)30.0) ;
	    }
		if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
		{
		    // ADC de-initialization Error
		    error("HAL_ADC_DeInit");
		}


	    targ = T_NBR;
	    return;
	}

	if(checkstring(ep, "IREF")){
		  ADC_init(0);
	      sConfig.Channel      = ADC_CHANNEL_VREFINT;             /* Sampled channel number */
		  sConfig.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
		  sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;    /* Sampling time (number of clock cycles unit) */
		  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		  {
		    /* Channel Configuration Error */
			    error("HAL_ADC_ConfigChannel");
		  }
	    if (HAL_ADC_Start(&hadc1) != HAL_OK)
	    {
	      /* Start Conversation Error */
	      error("HAL_ADC_Start");
	    }

	    /*##-4- Wait for the end of conversion #####################################*/
	    /*  For simplicity reasons, this example is just waiting till the end of the
	        conversion, but application may perform other tasks while conversion
	        operation is ongoing. */
	    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
	    {
	      /* End Of Conversion flag not set on time */
	        error("HAL_ADC_PollForConversion");
	    }
	    else
	    {
	      /* ADC conversion completed */
	      /*##-5- Get the converted value of regular channel  ########################*/
	      fret = (MMFLOAT)(HAL_ADC_GetValue(&hadc1))/(MMFLOAT)0XFFF * VCC;
	    }

		if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
		{
		    /* ADC de-initialization Error */
		    error("HAL_ADC_DeInit");
		}

	    targ = T_NBR;
	    return;
	}
	if(checkstring(ep,"SREF")){
		uint16_t *VREF=(uint16_t *)0x1FFF7A2A;
		fret=(MMFLOAT)*VREF/(MMFLOAT)0xFFF * (MMFLOAT)3.3;
	    targ = T_NBR;
		return;
	}
	char code;
	if((code=codecheck(ep)))ep+=2;
	pin = getinteger(ep);
	if(code)pin=codemap(code, pin);
    if(pin != 0) {  // pin = 0 when we are reading the internal reference voltage (1.2V) go straight to the analog read
        if(IsInvalidPin(pin)) error("Invalid pin");
        switch(ExtCurrentConfig[pin]) {
            case EXT_DIG_IN:
            case EXT_CNT_IN:
            case EXT_INT_HI:
            case EXT_INT_LO:
            case EXT_INT_BOTH:
            case EXT_DIG_OUT:
            case EXT_OC_OUT:    iret = ExtInp(pin);
                                targ = T_INT;
                                return;
            case EXT_PER_IN:	// if period measurement get the count and average it over the number of cycles
                                if(pin == INT1PIN) fret = (MMFLOAT)ExtInp(pin) / INT1InitTimer;
                                else if(pin == INT2PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT2InitTimer;
                                else if(pin == INT3PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT3InitTimer;
                                else if(pin == INT4PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT4InitTimer;
                                targ = T_NBR;
                                return;
            case EXT_FREQ_IN:	// if frequency measurement get the count and scale the reading
                                if(pin == INT1PIN) fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT1InitTimer;
                                else if(pin == INT2PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT2InitTimer;
                                else if(pin == INT3PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT3InitTimer;
                                else if(pin == INT4PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT4InitTimer;
                                 targ = T_NBR;
                                return;
            case EXT_ANA_IN:    break;
            default:            error("Pin | is not an input",pin);
        }
    } else error("Invalid Pin %", pin);

	  /*##-2- Configure ADC regular channel ######################################*/
    	ADCinuse=ADC_init(pin);
    	if(ADCinuse==1){
    	sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
    	sConfig.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
    	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;    /* Sampling time (number of clock cycles unit) */
    	sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


    	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    	{
    		/* Channel Configuration Error */
		    error("HAL_ADC_ConfigChannel");
    	}
    	if (HAL_ADC_Start(&hadc1) != HAL_OK)
    	{
    		/* Start Conversation Error */
    		error("HAL_ADC_Start");
    	}

    	/*##-4- Wait for the end of conversion #####################################*/
    	/*  For simplicity reasons, this example is just waiting till the end of the
        	conversion, but application may perform other tasks while conversion
        	operation is ongoing. */
    	if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
    	{
    		/* End Of Conversion flag not set on time */
    		error("HAL_ADC_PollForConversion");
    	}
    	else
    	{
    		/* ADC conversion completed */
    		/*##-5- Get the converted value of regular channel  ########################*/
    		//ADCdiv[ADCbits[ADCchannelB]];
    		fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc1))/ADCdiv[ADCbits[pin]] * VCC;
    		//if(ADCbits[pin]==12)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc1))/(MMFLOAT)0XFFF * VCC;
			//else if(ADCbits[pin]==10)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc1))/(MMFLOAT)0X3FF * VCC;
			//else if(ADCbits[pin]==8)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc1))/(MMFLOAT)0XFF * VCC;
    	}
    	if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
    	{
    		/* ADC de-initialization Error */
    		error("HAL_ADC_DeInit");
    	}

    	targ = T_NBR;
    	return;
    } else if(ADCinuse==2){
    	sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
    	sConfig.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
    	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;    /* Sampling time (number of clock cycles unit) */
    	sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


    	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    	{
    		/* Channel Configuration Error */
  		    error("HAL_ADC_ConfigChannel");
    	}
    	if (HAL_ADC_Start(&hadc2) != HAL_OK)
    	{
    		/* Start Conversation Error */
    		error("HAL_ADC_Start");
    	}

    	/*##-4- Wait for the end of conversion #####################################*/
    	/*  For simplicity reasons, this example is just waiting till the end of the
          	  conversion, but application may perform other tasks while conversion
          	  operation is ongoing. */
    	if (HAL_ADC_PollForConversion(&hadc2, 50) != HAL_OK)
    	{
    		/* End Of Conversion flag not set on time */
    		error("HAL_ADC_PollForConversion");
    	}
    	else
    	{
    		/* ADC conversion completed */
    		/*##-5- Get the converted value of regular channel  ########################*/
    		fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc2))/ADCdiv[ADCbits[pin]] * VCC;
    		//if(ADCbits[pin]==12)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc2))/(MMFLOAT)0XFFF * VCC;
			//else if(ADCbits[pin]==10)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc2))/(MMFLOAT)0X3FF * VCC;
			//else if(ADCbits[pin]==8)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc2))/(MMFLOAT)0XFF * VCC;


    	}
    	if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
    	{
    		/* ADC de-initialization Error */
    		error("HAL_ADC_DeInit");
    	}

    	targ = T_NBR;
    	return;
    	} else {
    		sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
    		sConfig.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
    		sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;    /* Sampling time (number of clock cycles unit) */
    		sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


    		if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    		{
    			/* Channel Configuration Error */
    			error("HAL_ADC_ConfigChannel");
    		}
    		if (HAL_ADC_Start(&hadc3) != HAL_OK)
    		{
    			/* Start Conversation Error */
    			error("HAL_ADC_Start");
    		}

    		/*##-4- Wait for the end of conversion #####################################*/
    		/*  For simplicity reasons, this example is just waiting till the end of the
          	  conversion, but application may perform other tasks while conversion
          	  operation is ongoing. */
    		if (HAL_ADC_PollForConversion(&hadc3, 50) != HAL_OK)
    		{
    			/* End Of Conversion flag not set on time */
    			error("HAL_ADC_PollForConversion");
    		}
    		else
    		{
    			/* ADC conversion completed */
    			/*##-5- Get the converted value of regular channel  ########################*/
    			fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc3))/ADCdiv[ADCbits[pin]] * VCC;
    			//if(ADCbits[pin]==12)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc3))/(MMFLOAT)0XFFF * VCC;
    			//else if(ADCbits[pin]==10)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc3))/(MMFLOAT)0X3FF * VCC;
    			//else if(ADCbits[pin]==8)fret=(MMFLOAT)(HAL_ADC_GetValue(&hadc3))/(MMFLOAT)0XFF * VCC;
    		}
    		if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
    		{
    			/* ADC de-initialization Error */
    			error("HAL_ADC_DeInit");
    		}

    		targ = T_NBR;
    		return;
    	}
}

void cmd_sync(void){
	int64_t i;
	getargs(&cmdline,3,",");
	if(synctime && argc==0){
		while(__HAL_TIM_GET_COUNTER(&htim5)<synctime){};
		__HAL_TIM_SET_COUNTER(&htim5, 0);
	} else {
		if(argc==0)error("sync not initialised");
		i=getinteger(argv[0]);
		if(i){
			CurrentlyPlaying = P_SYNC;
			if(argc==3){
				if(checkstring(argv[2],"U")){
					i *= 84;
				} else if(checkstring(argv[2],"M")){
					i *= 84000;
				} else if(checkstring(argv[2],"S")){
					i*=84000000;
				}
				if(i>4294967297)error("Period > 100 seconds");
//				HAL_TIM_Base_DeInit(&htim5);
				htim5.Init.Period = 0xFFFFFFFF;
				htim5.Instance->ARR = 0xFFFFFFFF;
//				HAL_TIM_Base_Init(&htim5);
				HAL_TIM_Base_Start(&htim5);
			}
			synctime=(uint32_t)i;
			__HAL_TIM_SET_COUNTER(&htim5, 0);
		} else {
			HAL_TIM_Base_Stop(&htim5);
			synctime=0;
			CurrentlyPlaying = P_NOTHING;
		}
	}
}

// this is invoked as a command (ie, port(3, 8) = Value)
// first get the arguments then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pins accordingly
void cmd_port(void) {
	int pin, nbr, value, code, pincode;
    int i;
	getargs(&cmdline, NBRPINS * 4, ",");

	if((argc & 0b11) != 0b11) error("Invalid syntax");

    // step over the equals sign and get the value for the assignment
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Invalid syntax");
	++cmdline;
	if(!*cmdline) error("Invalid syntax");
	value = getinteger(cmdline);

    for(i = 0; i < argc; i += 4) {
    	code=0;
    	if((code=codecheck(argv[i])))argv[i]+=2;
    	pincode = getinteger(argv[i]);
        nbr = getinteger(argv[i + 2]);
        if(nbr < 0 || (pincode == 0 && code==0) || (pincode<0)) error("Invalid argument");

        while(nbr) {
        	if(code)pin=codemap(code, pincode);
        	else pin=pincode;
//        	PIntComma(pin);
            if(IsInvalidPin(pin) || !(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Invalid output pin");
            ExtSet(pin, value & 1);
            value >>= 1;
            nbr--;
            pincode++;
        }
    } //PRet();
}



// this is invoked as a function (ie, x = port(10,8) )
void fun_port(void) {
	int pin, nbr, i, value = 0, code, pincode;

	getargs(&ep, NBRPINS * 4, ",");
	if((argc & 0b11) != 0b11) error("Invalid syntax");

    for(i = argc - 3; i >= 0; i -= 4) {
    	code=0;
    	if((code=codecheck(argv[i])))argv[i]+=2;
        pincode = getinteger(argv[i]);
        nbr = getinteger(argv[i + 2]);
        if(nbr < 0 || (pincode == 0 && code==0) || (pincode<0)) error("Invalid argument");
        pincode += nbr - 1;                                             // we start by reading the most significant bit

        while(nbr) {
        	if(code)pin=codemap(code, pincode);
        	else pin=pincode;
            if(IsInvalidPin(pin) || !(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH)) error("Invalid input pin");
            value <<= 1;
            value |= PinRead(pin);
            nbr--;
            pincode--;
        }
    }

    iret = value;
    targ = T_INT;
}


void cmd_setpin(void) {
	int i, pin, value, option = 0;
	getargs(&cmdline, 7, ",");
	if(argc%2 == 0 || argc < 3) error("Argument count");

    if(checkstring(argv[2], "OFF") || checkstring(argv[2], "0"))
        value = EXT_NOT_CONFIG;
    else if(checkstring(argv[2], "AIN"))
        value = EXT_ANA_IN;
    else if(checkstring(argv[2], "DIN"))
        value = EXT_DIG_IN;
    else if(checkstring(argv[2], "FIN"))
        value = EXT_FREQ_IN;
    else if(checkstring(argv[2], "PIN"))
        value = EXT_PER_IN;
    else if(checkstring(argv[2], "CIN"))
        value = EXT_CNT_IN;
    else if(checkstring(argv[2], "INTH"))
        value = EXT_INT_HI;
    else if(checkstring(argv[2], "INTL"))
        value = EXT_INT_LO;
    else if(checkstring(argv[2], "DOUT"))
        value = EXT_DIG_OUT;
    else if(checkstring(argv[2], "OOUT"))
        value = EXT_OC_OUT;
    else if(checkstring(argv[2], "INTB"))
        value = EXT_INT_BOTH;
    else
        value = getint(argv[2], 1, 9);

    // check for any options
    switch(value) {
    	case EXT_ANA_IN:if(argc == 5) {
    						option = getint((argv[4]), 8, 12);
    						if(option & 1)error("Invalid bit count");
        					} else
        					option = 12;
        break;

        case EXT_DIG_IN:    if(argc == 5) {
                                if(checkstring(argv[4], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[4], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_INT_HI:
        case EXT_INT_LO:
        case EXT_INT_BOTH:  if(argc == 7) {
                                if(checkstring(argv[6], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[6], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_FREQ_IN:   if(argc == 5)
                                option = getint((argv[4]), 10, 100000);
                            else
                                option = 1000;
                            break;
        case EXT_PER_IN:   if(argc == 5)
                                option = getint((argv[4]), 1, 10000);
                            else
                                option = 1;
                            break;

        case EXT_CNT_IN:   if(argc == 5)
                               option = getint((argv[4]), 1, 5);
                            else
                                option = 1;
                            break;

        case EXT_DIG_OUT:   if(argc == 5) {
                                if(checkstring(argv[4], "OC"))
                                    value = EXT_OC_OUT;
                            else
                                error("Invalid option");
                            }
                            break;
        default:            if(argc > 3) error("Unexpected text");
    }

	char code;
	if((code=codecheck(argv[0])))argv[0]+=2;
	pin = getinteger(argv[0]);
	if(code)pin=codemap(code, pin);
    {
        CheckPin(pin, CP_IGNORE_INUSE);
        ExtCfg(pin, value, option);
    }

	if(value == EXT_INT_HI || value == EXT_INT_LO || value == EXT_INT_BOTH) {
		// we need to set up a software interrupt
		if(argc < 5) error("Argument count");
        for(i = 0; i < NBRINTERRUPTS; i++) if(inttbl[i].pin == 0) break;
        if(i >= NBRINTERRUPTS) error("Too many interrupts");
        inttbl[i].pin = pin;
		inttbl[i].intp = GetIntAddress(argv[4]);					// get the interrupt routine's location
		inttbl[i].last = ExtInp(pin);								// save the current pin value for the first test
        switch(value) {                                             // and set trigger polarity
            case EXT_INT_HI:    inttbl[i].lohi = T_LOHI; break;
            case EXT_INT_LO:    inttbl[i].lohi = T_HILO; break;
            case EXT_INT_BOTH:  inttbl[i].lohi = T_BOTH; break;
        }
		InterruptUsed = true;
	}
}



void cmd_pulse(void) {
    int pin, i, x, y;
    MMFLOAT f;

	getargs(&cmdline, 3, ",");
	if(argc != 3) error("Invalid syntax");
	char code;
	if((code=codecheck(argv[0])))argv[0]+=2;
	pin = getinteger(argv[0]);
	if(code)pin=codemap(code, pin);
	if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin | is not an output", pin);

    f = getnumber(argv[2]);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    x = f;                                                          // get the integer portion (in mSec)
    y = (int)((f - (MMFLOAT)x) * 1000.0);                             // get the fractional portion (in uSec)

    for(i = 0; i < NBR_PULSE_SLOTS; i++)                            // search looking to see if the pin is in use
        if(PulseCnt[i] != 0 && PulsePin[i] == pin) {
            mT4IntEnable(0);       									// disable the timer interrupt to prevent any conflicts while updating
            PulseCnt[i] = x;                                        // and if the pin is in use, set its time to the new setting or reset if the user wants to terminate
            mT4IntEnable(1);
            if(x == 0) PinSetBit(PulsePin[i], LATINV);
            return;
        }

    if(x == 0 && y == 0) return;                                    // silently ignore a zero pulse width

    if(x < 3) {                                                     // if this is under 3 milliseconds just do it now
        PinSetBit(pin, LATINV);                    // starting edge of the pulse
        uSec(x * 1000 + y);
        PinSetBit(pin, LATINV);                    // finishing edge
        return;
    }

    for(i = 0; i < NBR_PULSE_SLOTS; i++)
        if(PulseCnt[i] == 0) break;                                 // find a spare slot

    if(i >= NBR_PULSE_SLOTS) error("Too many concurrent PULSE commands");

    PinSetBit(pin, LATINV);                        // starting edge of the pulse
    if(x == 1) uSec(500);                                           // prevent too narrow a pulse if there is just one count
    PulsePin[i] = pin;                                              // save the details
    PulseCnt[i] = x;
    PulseActive = true;
}


void fun_pulsin(void) { //allowas timeouts up to 10 seconds
    int pin, polarity;
    unsigned int t1, t2;

	getargs(&ep, 7, ",");
	if((argc &1) != 1 || argc < 3) error("Invalid syntax");
	char code;
	if((code=codecheck(argv[0])))argv[0]+=2;
	pin = getinteger(argv[0]);
	if(code)pin=codemap(code, pin);
    if(IsInvalidPin(pin)) error("Invalid pin");
	if(ExtCurrentConfig[pin] != EXT_DIG_IN) error("Pin | is not an input",pin);
    polarity = getinteger(argv[2]);

    t1 = t2 = 100000;                                               // default timeout is 100mS
    if(argc >= 5) t1 = t2 = getint(argv[4], 5, 10000000);
    if(argc == 7) t2 = getint(argv[6], 5, 10000000);
    iret = -1;                                                      // in anticipation of a timeout
    writeusclock(0);
    if(polarity) {
        while(PinRead(pin)) if(readusclock() > t1) return;
        while(!PinRead(pin)) if(readusclock() > t1) return;
        writeusclock(0);
        while(PinRead(pin)) if(readusclock() > t2) return;
    } else {
        while(!PinRead(pin)) if(readusclock() > t1) return;
        while(PinRead(pin)) if(readusclock() > t1) return;
        writeusclock(0);
        while(!PinRead(pin)) if(readusclock() > t2) return;
    }
    t1 = readusclock();
    iret = t1;
    targ = T_INT;
}



/****************************************************************************************************************************
IR routines
*****************************************************************************************************************************/

void cmd_ir(void) {
    char *p;
    int i, pin, dev, cmd;
    if(checkstring(cmdline, "CLOSE")) {
        IrState = IR_CLOSED;
    	HAL_NVIC_DisableIRQ(EXTI4_IRQn);
        IrInterrupt = NULL;
        ExtCfg(IRPIN, EXT_NOT_CONFIG, 0);
    } else if((p = checkstring(cmdline, "SEND"))) {
        getargs(&p, 5, ",");
        pin = getinteger(argv[0]);
        dev = getint(argv[2], 0, 0b11111);
        cmd = getint(argv[4], 0, 0b1111111);
        if(ExtCurrentConfig[pin] >= EXT_COM_RESERVED)  error("Pin  | is in use",pin);
        ExtCfg(pin, EXT_DIG_OUT, 0);
        cmd = (dev << 7) | cmd;
        IRSendSignal(pin, 186);
        for(i = 0; i < 12; i++) {
            uSec(600);
            if(cmd & 1)
                IRSendSignal(pin, 92);
            else
                IRSendSignal(pin, 46);
            cmd >>= 1;
        }
    } else {
        getargs(&cmdline, 5, ",");
        if(IrState != IR_CLOSED) error("Already open");
        if(argc%2 == 0 || argc == 0) error("Invalid syntax");
        IrVarType = 0;
        IrDev = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b01;
        IrCmd = findvar(argv[2], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b10;
        InterruptUsed = true;
        IrInterrupt = GetIntAddress(argv[4]);							// get the interrupt location
        IrInit();
    }
}


void IrInit(void) {
    writeusclock(0);
    if(ExtCurrentConfig[IRPIN] >= EXT_COM_RESERVED)  error("Pin | is in use",IRPIN);
    ExtCfg(IRPIN, EXT_DIG_IN, 0);
    ExtCfg(IRPIN, EXT_COM_RESERVED, 0);
	IR_InitDef.Pull = GPIO_PULLUP; //set as input with no pullup or down
	IR_InitDef.Pin = PinDef[IRPIN].bitnbr;
	IR_InitDef.Mode = GPIO_MODE_IT_RISING_FALLING;
	IR_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_GPIO_Init(PinDef[IRPIN].sfr, &IR_InitDef);
    IrReset();
}


void IrReset(void) {
	IrState = IR_WAIT_START;
    IrCount = 0;
    writeusclock(0);
}


// this modulates (at about 38KHz) the IR beam for transmit
// half_cycles is the number of half cycles to send.  ie, 186 is about 2.4mSec
void IRSendSignal(int pin, int half_cycles) {
    while(half_cycles--) {
        PinSetBit(pin, LATINV);
        uSec(13);
    }
}




/****************************************************************************************************************************
 The LCD command
*****************************************************************************************************************************/

void LCD_Nibble(int Data, int Flag, int Wait_uSec);
void LCD_Byte(int Data, int Flag, int Wait_uSec);
void LcdPinSet(int pin, int val);
static char lcd_pins[6];

void cmd_lcd(char *lcd)
 {
    char *p;
    int i, j, code;

    if((p = checkstring(lcd, "INIT"))) {
        getargs(&p, 11, ",");
        if(argc != 11) error("Invalid syntax");
        if(*lcd_pins) error("Already open");
        for(i = 0; i < 6; i++) {
        	code=0;
        	if((code=codecheck(argv[i * 2])))argv[i * 2]+=2;
            lcd_pins[i] = getinteger(argv[i * 2]);
        	if(code)lcd_pins[i]=codemap(code, lcd_pins[i]);
            if(ExtCurrentConfig[(int)lcd_pins[i]] >= EXT_COM_RESERVED)  error("Pin | is in use",lcd_pins[i]);
            ExtCfg(lcd_pins[i], EXT_DIG_OUT, 0);
            ExtCfg(lcd_pins[i], EXT_COM_RESERVED, 0);
        }
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0010, 0, 2000);                                // 4 bit mode
        LCD_Byte(0b00101100, 0, 600);                               // 4 bits, 2 lines
        LCD_Byte(0b00001100, 0, 600);                               // display on, no cursor
        LCD_Byte(0b00000110, 0, 600);                               // increment on write
        LCD_Byte(0b00000001, 0, 3000);                              // clear the display
        return;
    }

    if(!*lcd_pins) error("Not open");
    if(checkstring(lcd, "CLOSE")) {
        for(i = 0; i < 6; i++) {
			ExtCfg(lcd_pins[i], EXT_NOT_CONFIG, 0);					// all set to unconfigured
			ExtSet(lcd_pins[i], 0);									// all outputs (when set) default to low
            *lcd_pins = 0;
        }
    } else if((p = checkstring(lcd, "CLEAR"))) {                // clear the display
        LCD_Byte(0b00000001, 0, 3000);
    } else if((p = checkstring(lcd, "CMD")) || (p = checkstring(lcd, "DATA"))) { // send a command or data
        getargs(&p, MAX_ARG_COUNT * 2, ",");
        for(i = 0; i < argc; i += 2) {
            j = getint(argv[i], 0, 255);
            LCD_Byte(j, toupper(*lcd) == 'D', 0);
        }
    } else {
        const char linestart[4] = {0, 64, 20, 84};
        int center, pos;

        getargs(&lcd, 5, ",");
        if(argc != 5) error("Invalid syntax");
        i = getint(argv[0], 1, 4);
        pos = 1;
        if(checkstring(argv[2], "C8"))
            center = 8;
        else if(checkstring(argv[2], "C16"))
            center = 16;
        else if(checkstring(argv[2], "C20"))
            center = 20;
        else if(checkstring(argv[2], "C40"))
            center = 40;
        else {
            center = 0;
            pos = getint(argv[2], 1, 256);
        }
        p = getstring(argv[4]);                                     // returns an MMBasic string
        i = 128 + linestart[i - 1] + (pos - 1);
        LCD_Byte(i, 0, 600);
        for(j = 0; j < (center - *p) / 2; j++) {
            LCD_Byte(' ', 1, 0);
        }
        for(i = 1; i <= *p; i++) {
            LCD_Byte(p[i], 1, 0);
            j++;
        }
        for(; j < center; j++) {
            LCD_Byte(' ', 1, 0);
        }
    }
}



void LCD_Nibble(int Data, int Flag, int Wait_uSec) {
    int i;
    LcdPinSet(lcd_pins[4], Flag);
    for(i = 0; i < 4; i++)
        LcdPinSet(lcd_pins[i], (Data >> i) & 1);
    LcdPinSet(lcd_pins[5], 1); uSec(250); LcdPinSet(lcd_pins[5], 0);
    if(Wait_uSec)
        uSec(Wait_uSec);
    else
        uSec(250);
}


void LCD_Byte(int Data, int Flag, int Wait_uSec) {
    LCD_Nibble(Data/16, Flag, 0);
    LCD_Nibble(Data, Flag, Wait_uSec);
}


void LcdPinSet(int pin, int val) {
    PinSetBit(pin, val ? LATSET : LATCLR);
}
void __attribute__ ((optimize("-O2"))) WS2812e(int pin, int T1H, int T1L, int T0H, int T0L, int nbr, char *p){
    short now;
	HAL_TIM_Base_Start(&htim14);
	for(int i=0;i<nbr;i++){
		__HAL_TIM_SET_COUNTER(&htim14,0);
		for(int j=0;j<8;j++){
			if(*p & 1){
				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim14)+T1H;while(__HAL_TIM_GET_COUNTER(&htim14)<now){};
				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim14)+T1L;while(__HAL_TIM_GET_COUNTER(&htim14)<now){};
			} else {
				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim14)+T0H;while(__HAL_TIM_GET_COUNTER(&htim14)<now){};
				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim14)+T0L;while(__HAL_TIM_GET_COUNTER(&htim14)<now){};
			}
			*p>>=1;
		}
		p++;
	}
}

void WS2812(char *q){
       void *ptr1 = NULL;
        int64_t *dest=NULL;
        uint32_t pin, red , green, blue, white, colour, colours=3;
        short T0H=0,T0L=0,T1H=0,T1L=0;
        char *p;
        int i, j, bit, nbr=0;
    	getargs(&q, 7, ",");
        if(argc != 7)error("Argument count");
    	p=argv[0];
    	if(toupper(*p)=='O'){
    		T1H=13;
    		T1L=8;
    		T0H=6;
    		T0L=13;
    	} else if(toupper(*p)=='B'){
    		T1H=15;
    		T1L=6;
    		T0H=6;
    		T0L=13;
    	} else if(toupper(*p)=='S'){
    		T0H=4;
    		T0L=14;
    		T1H=11;
    		T1L=9 ;
    	} else if(toupper(*p)=='W' ){
            colours=4;
    		T0H=4;
    		T0L=14;
    		T1H=11;
    		T1L=9 ;
    	} else error("Syntax");
        nbr=getint(argv[4],1,256);
        if(nbr>1){
            ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_INT) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                    error("Argument 1 must be integer array");
                } else {
                    if((vartbl[VarIndex].dims[0] - OptionBase + 1)<nbr)error("Array size");
                }
                dest = (long long int *)ptr1;
            } else error("Argument 1 must be integer array");
        } else {
            colour=getinteger(argv[6]);
            dest = (long long int *)&colour;
        }
    	char code;
    	if((code=codecheck(argv[2])))argv[2]+=2;
    	pin = getinteger(argv[2]);
    	if(code)pin=codemap(code, pin);
        if(IsInvalidPin(pin)) error("Invalid pin");
        if(!(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT))  error("Pin | is in use",pin);
        ExtCfg(pin, EXT_DIG_OUT, 0);
		p=GetTempMemory((nbr+1)*colours);
		uSec(80);
    	for(i=0;i<nbr;i++){
    		green=(dest[i]>>8) & 0xFF;
    		red=(dest[i]>>16) & 0xFF;
    		blue=dest[i] & 0xFF;
            if(colours==4)white=(dest[i]>>24) & 0xFF;
			p[0]=0;p[1]=0;p[2]=0;
            if(colours==4){p[3]=0;}
    		for(j=0;j<8;j++){
    			bit=1<<j;
    			if( green &  (1<<(7-j)) )p[0] |= bit;
    			if(red   & (1<<(7-j)))p[1] |= bit;
    			if(blue  & (1<<(7-j)))p[2] |= bit;
                if(colours==4){
    			    if(white  & (1<<(7-j)))p[3] |= bit;
                }
    		}
    		p+=colours;
    	}
    	p-=(nbr*colours);
        __disable_irq();
        MX_TIM14_Init(3);
        WS2812e(pin, T1H, T1L, T0H, T0L, nbr*colours, p);
        HAL_TIM_Base_Stop(&htim14);
        __enable_irq();
}

/****************************************************************************************************************************
 The DISTANCE function
*****************************************************************************************************************************/

void fun_distance(void) {
    int trig, echo,techo;

	getargs(&ep, 3, ",");
	if((argc &1) != 1) error("Invalid syntax");
	char code;
	if((code=codecheck(argv[0])))argv[0]+=2;
	trig = getinteger(argv[0]);
	if(code)trig=codemap(code, trig);
    if(argc == 3){
    	if((code=codecheck(argv[2])))argv[2]+=2;
    	echo = getinteger(argv[2]);
    	if(code)echo=codemap(code, echo);
    }
    else
        echo = trig;                                                // they are the same if it is a 3-pin device
    if(IsInvalidPin(trig) || IsInvalidPin(echo)) error("Invalid pin |",echo);
    if(ExtCurrentConfig[trig] >= EXT_COM_RESERVED || ExtCurrentConfig[echo] >= EXT_COM_RESERVED)  error("Pin | is in use",trig);
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // setup the echo input
    PinSetBit(trig, LATCLR);                                        // trigger output must start low
    ExtCfg(trig, EXT_DIG_OUT, 0);                                   // setup the trigger output
    PinSetBit(trig, LATSET); uSec(20); PinSetBit(trig, LATCLR);     // pulse the trigger
    uSec(50);
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // this is in case the sensor is a 3-pin type
    uSec(50);
    PauseTimer = 0;                                                 // this is our timeout
    while(PinRead(echo)) if(PauseTimer > 50) { fret = -2; return; } // wait for the acknowledgement pulse start
    while(!PinRead(echo)) if(PauseTimer > 100) { fret = -2; return;}// then its end
    PauseTimer = 0;
    WriteCoreTimer(0);
    while(PinRead(echo)) {                                          // now wait for the echo pulse
        if(PauseTimer > 38) {                                       // timeout is 38mS
            fret = -1;
            return;
        }
    }
    techo=ReadCoreTimer();
    // we have the echo, convert the time to centimeters
    fret = ((MMFLOAT)techo)/11600.0;  //200 ticks per us, 58 us per cm
    targ = T_NBR;
}




/****************************************************************************************************************************
 The KEYPAD command
*****************************************************************************************************************************/

static char keypad_pins[8];
MMFLOAT *KeypadVar;
char *KeypadInterrupt = NULL;
void KeypadClose(void);

void cmd_keypad(void) {
    int i, j, code;

    if(checkstring(cmdline, "CLOSE"))
        KeypadClose();
    else {
        getargs(&cmdline, 19, ",");
        if(argc%2 == 0 || argc < 17) error("Invalid syntax");
        if(KeypadInterrupt != NULL) error("Already open");
        KeypadVar = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(!(vartbl[VarIndex].type & T_NBR)) error("Floating point variable required");
        InterruptUsed = true;
        KeypadInterrupt = GetIntAddress(argv[2]);					// get the interrupt location
        for(i = 0; i < 8; i++) {
            if(i == 7 && argc < 19) {
                keypad_pins[i] = 0;
                break;
            }
        	code=0;
        	if((code=codecheck(argv[(i + 2) * 2])))argv[(i + 2) * 2]+=2;
        	j = getinteger(argv[(i + 2) * 2]);
        	if(code)j=codemap(code, j);
            if(ExtCurrentConfig[j] >= EXT_COM_RESERVED)  error("Pin | is in use",j);
            if(i < 4) {
                ExtCfg(j, EXT_DIG_IN, CNPUSET);
            } else {
                ExtCfg(j, EXT_OC_OUT, 0);
                PinSetBit(j, LATSET);
            }
            ExtCfg(j, EXT_COM_RESERVED, 0);
            keypad_pins[i] = j;
        }
    }
}


void KeypadClose(void) {
    int i;
    if(KeypadInterrupt == NULL) return;
    for(i = 0; i < 8; i++) {
        if(keypad_pins[i]) {
            ExtCfg(keypad_pins[i], EXT_NOT_CONFIG, 0);				// all set to unconfigured
        }
    }
    KeypadInterrupt = NULL;
}


int KeypadCheck(void) {
    static unsigned char count = 0, keydown = false;
    int i, j;
    const char PadLookup[16] = { 1, 2, 3, 20, 4, 5, 6, 21, 7, 8, 9, 22, 10, 0, 11, 23 };

    if(count++ % 64) return false;                                  // only check every 64 loops through the interrupt processor

    for(j = 4; j < 8; j++) {                                        // j controls the pull down pins
        if(keypad_pins[j]) {                                        // we might just have 3 pull down pins
            PinSetBit(keypad_pins[j], LATCLR);                      // pull it low
            for(i = 0; i < 4; i++) {                                // i is the row sense inputs
                if(PinRead(keypad_pins[i]) == 0) {                  // if it is low we have found a keypress
                    if(keydown) goto exitcheck;                     // we have already reported this, so just exit
                    uSec(40 * 1000);                                // wait 40mS and check again
                    if(PinRead(keypad_pins[i]) != 0) goto exitcheck;// must be contact bounce if it is now high
                    *KeypadVar = PadLookup[(i << 2) | (j - 4)];     // lookup the key value and set the variable
                    PinSetBit(keypad_pins[j], LATSET);
                    keydown = true;                                 // record that we know that the key is down
                    return true;                                    // and tell the interrupt processor that we are good to go
                }
            }
            PinSetBit(keypad_pins[j], LATSET);                      // wasn't this pin, clear the pulldown
        }
    }
    keydown = false;                                                // no key down, record the fact
    return false;

exitcheck:
    PinSetBit(keypad_pins[j], LATSET);
    return false;
}
int64_t __attribute__ ((optimize("-O2"))) DHmem(int pin){
    int timeout = 400;
    long long int r;
    int i;
    writeusclock(0);
    PinSetBit(pin, CNPUSET);
    PinSetBit(pin, TRISSET);
    uSec(5);
    // wait for the DHT22 to pull the pin low and return it high then take it low again
    while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;
    while(!PinRead(pin)) if(readusclock() > timeout) goto error_exit;
    while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;
//    PInt(readusclock());PRet();

    // now we wait for the pin to go high and measure how long it stays high (> 50uS is a one bit, < 50uS is a zero bit)
    for(r = i = 0; i < 40; i++) {
    	timeout=400;
        while(!PinRead(pin)) if(readusclock() > timeout) goto error_exit;
        timeout=400;writeusclock(0);
        while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;
        r <<= 1;
        r |= (readusclock() > 50);
    }
    return r;
    error_exit:
    return -1;

}
void DHT22(char *p) {
    int pin;
    long long int r;
    int dht22=0;
    MMFLOAT *temp, *humid;

    getargs(&p, 7, ",");
    if(!(argc == 5 || argc == 7)) error("Incorrect number of arguments");

    // get the two variables
	temp = findvar(argv[2], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
	humid = findvar(argv[4], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");

    // get the pin number and set it up
    // get the pin number and set it up
	char code;
	if((code=codecheck(argv[0])))argv[0]+=2;
	pin = getinteger(argv[0]);
	if(code)pin=codemap(code, pin);
    if(IsInvalidPin(pin)) error("Invalid pin");
    if(ExtCurrentConfig[pin] != EXT_NOT_CONFIG)  error("Pin is in use");
    ExtCfg(pin, EXT_OC_OUT, 0);
    if(argc==7){
    	dht22=getint(argv[6],0,1);
    }
    // pulse the pin low for 1mS
    PinSetBit(pin, LATCLR);
    uSec(1000+dht22*18000);
    if((r=DHmem(pin))==-1) goto error_exit;
    // we have all 40 bits
    // first validate against the checksum
    if( ( ( ((r >> 8) & 0xff) + ((r >> 16) & 0xff) + ((r >> 24) & 0xff) + ((r >> 32) & 0xff) ) & 0xff) != (r & 0xff)) goto error_exit;                                           // returning temperature
    if(dht22==0){
		*temp = (MMFLOAT)((r >> 8) &0x7fff) / 10.0;                       // get the temperature
		if((r >> 8) &0x8000) *temp = -*temp;                            // the top bit is the sign
		*humid = (MMFLOAT)(r >> 24) / 10.0;                               // get the humidity
    } else {
		*temp = (MMFLOAT)((signed char)((r>>16) & 0xFF));                       // get the temperature
		*humid = (MMFLOAT)((signed char)(r >> 32));                               // get the humidity
    }
    goto normal_exit;

error_exit:
    *temp = *humid = 1000.0;                                        // an obviously incorrect reading

normal_exit:
    ExtCfg(pin, EXT_NOT_CONFIG, 0);
    PinSetBit(pin, LATCLR);
}





/*******************************************************************************************
********************************************************************************************

Utility routines for the external I/O commands and functions in MMBasic

********************************************************************************************
********************************************************************************************/


void ClearExternalIO(void) {
	int i;

    PWMClose(0);
	PWMClose(1);                                         // close any running PWM output
	if(!HAS_64PINS){
		PWMClose(2);
		PWMClose(3);
	}
    if(Option.SerialConDisabled) SerialClose(1);        // same for serial ports
    SerialClose(2);
    SerialClose(3);
    SerialClose(4);                                // same for serial ports
    SPIClose();
    SPI2Close();
    CurrentSPISpeed=NONE_SPI_SPEED;
    dacclose();
    ADCclose();
    ResetDisplay();
    i2c_disable();                                                  // close I2C
    IrState = IR_CLOSED;
    IrInterrupt = NULL;
    IrGotMsg = false;
    if(!HAS_64PINS)i2c2_disable();                                  // close I2C2
    KeypadInterrupt = NULL;
    *lcd_pins = 0;                                                  // close the LCD
    ds18b20Timer = -1;                                              // turn off the ds18b20 timer
    for(i=0;i<MAXBLITBUF;i++){
    	blitbuffptr[i] = NULL;
    }
	for(i = 1; i < NBRPINS + 1; i++) {
		if(CheckPin(i, CP_NOABORT | CP_IGNORE_INUSE | CP_IGNORE_RESERVED)) {    // don't reset invalid or boot reserved pins
			ExtCfg(i, EXT_NOT_CONFIG, 0);							// all set to unconfigured
		}
	}

	for(i = 0; i < NBRINTERRUPTS; i++) {
		inttbl[i].pin = 0;                                          // disable all interrupts
	}
	InterruptReturn = NULL;
	InterruptUsed = false;
    OnKeyGOSUB = NULL;
    KeyInterrupt=NULL;
    keyselect=0;

    for(i = 0; i < NBRSETTICKS; i++) TickInt[i] = NULL;
    for(i = 0; i < NBRSETTICKS; i++) TickActive[i] = 1;

	for(i = 0; i < NBR_PULSE_SLOTS; i++) PulseCnt[i] = 0;           // disable any pending pulse commands
    PulseActive = false;
}



/****************************************************************************************************************************
Initialise the I/O pins
*****************************************************************************************************************************/
void initExtIO(void) {
	int i;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	if(HAS_144PINS){
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	//__HAL_RCC_GPIOH_CLK_ENABLE();
	}

	for(i = 1; i < NBRPINS + 1; i++) {
        if(CheckPin(i, CP_NOABORT | CP_IGNORE_INUSE | CP_IGNORE_RESERVED | CP_IGNORE_BOOTRES)){
            ExtCfg(i, EXT_NOT_CONFIG, 0);							// all set to unconfigured
        }
	}
	oc1=0;oc2=0;oc3=0;oc4=0;oc5=0;oc6=0;oc7=0,oc8=0,oc9=0; //set all PWM to off
}

//int getinttblpos(int tbl[],int pin){
//    int i=0;
//    while (tbl[i]!=pin){
//        i++;
//    }
//    return i;
//}

/****************************************************************************************************************************
Configure an I/O pin
*****************************************************************************************************************************/
void ExtCfg(int pin, int cfg, int option) {
	int i,edge,pull;

    if(IsInvalidPin(pin)) error("Invalid pin");

    CheckPin(pin, CP_IGNORE_INUSE | CP_IGNORE_RESERVED);

    if(cfg >= EXT_COM_RESERVED) {
        ExtCurrentConfig[pin] = cfg;                                // don't do anything except set the config type
        return;
    }

	// make sure that interrupts are disabled in case we are changing from an interrupt input
    if(pin == INT1PIN) HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    if(pin == INT2PIN) HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    if(pin == INT3PIN) HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    if(pin == INT4PIN) HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    if(pin == IRPIN) HAL_NVIC_DisableIRQ(EXTI2_IRQn);

    // make sure any pullups/pulldowns are removed in case we are changing from a digital input
    PinSetBit(pin, CNPUCLR);  PinSetBit(pin, CNPDCLR);

	for(i = 0; i < NBRINTERRUPTS; i++)
        if(inttbl[i].pin == pin)
            inttbl[i].pin = 0;                                           // start off by disable a software interrupt (if set) on this pin

	switch(cfg) {
		case EXT_NOT_CONFIG:
								GPIO_InitDef.Pull = GPIO_PULLDOWN; //set as input with no pullup or down
								GPIO_InitDef.Pin = PinDef[pin].bitnbr;
								GPIO_InitDef.Mode = GPIO_MODE_INPUT;
								GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
								HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;


		case EXT_ANA_IN:        if(!(PinDef[pin].mode & ANALOG_IN)) error("Invalid configuration");
								ADCbits[pin]=option;
								break;

		case EXT_PER_IN:		// same as counting, so fall through unless high speed
   		case EXT_FREQ_IN:		// same as counting, so fall through
		case EXT_CNT_IN:
			                   //Set the options for CIN i.e.
                               edge = GPIO_MODE_IT_RISING;
                               pull = GPIO_PULLDOWN;
                               if(cfg==EXT_CNT_IN && option==2)edge = GPIO_MODE_IT_FALLING;
                               if(cfg==EXT_CNT_IN && option>=3)edge = GPIO_MODE_IT_RISING | GPIO_MODE_IT_FALLING;
                               //if(option==1 || option==4)pull = GPIO_PULLDOWN;  //Rising and Pull Down or both and pulldown
                               if(option==2 || option==5)pull = GPIO_PULLUP;    //Falling and pullup or both and pullup
                               if(option==3 )pull = GPIO_NOPULL;    //Falling and pullup or both and pullup

			                   if(pin == INT1PIN) {
									GPIO_InitDef.Pull = pull; //set as input with pull down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI1_IRQn);
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									INT1Count = INT1Value = 0;
                                    INT1Timer = INT1InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT2PIN) {
									GPIO_InitDef.Pull = pull; //set as input with pull down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI3_IRQn);
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									INT2Count = INT2Value = 0;
                                    INT2Timer = INT2InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT3PIN) {
									GPIO_InitDef.Pull = pull; //set as input with no pull down
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI4_IRQn);
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									INT3Count = INT3Value = 0;
                                    INT3Timer = INT3InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT4PIN) {
									GPIO_InitDef.Pull = pull; //set as input with no pull down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									INT4Count = INT4Value = 0;
                                    INT4Timer = INT4InitTimer = option;  // only used for frequency and period measurement
									break;
								}
				                error("Invalid configuration");		// not an interrupt enabled pin
								return;

		case EXT_INT_LO:											// same as digital input, so fall through
		case EXT_INT_HI:											// same as digital input, so fall through
		case EXT_INT_BOTH:											// same as digital input, so fall through
		case EXT_DIG_IN:		if(!(PinDef[pin].mode & DIGITAL_IN)) error("Invalid configuration");
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									if(option==CNPUSET)GPIO_InitDef.Pull = GPIO_PULLUP;
									if(option==CNPDSET)GPIO_InitDef.Pull = GPIO_PULLDOWN;
									GPIO_InitDef.Mode = GPIO_MODE_INPUT;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									break;

		case EXT_DIG_OUT:		if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;

		case EXT_OC_OUT:		if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_OD;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;

		default:				error("Invalid configuration");
		                        return;
	}
	ExtCurrentConfig[pin] = cfg;
	if(cfg == EXT_NOT_CONFIG) ExtSet(pin, 0);						// set the default output to low

}

// round a float to an integer
unsigned short FloatToUint16(MMFLOAT x) {
    if(x<0 || x > 3120 )
        error("Number range");
    return (x >= 0 ? (unsigned short)(x + 0.5) : (unsigned short)(x - 0.5)) ;
}
void __attribute__ ((optimize("-O2"))) bitstream(int pin, unsigned short *data, int num){
	unsigned short now;
	for(int i=0;i<num;i++){
 		__HAL_TIM_SET_COUNTER(&htim14,0);
   		PinDef[pin].sfr->ODR ^= PinDef[pin].bitnbr;
 		now=data[i];
		while(__HAL_TIM_GET_COUNTER(&htim14)<now){};
	}

}
void cmd_bitbang(void){
	char *tp;
	tp = checkstring(cmdline, "WS2812");
	if(tp) {
		WS2812(tp);
		return;
	}
	tp = checkstring(cmdline, "LCD");
	if(tp) {
		cmd_lcd(tp);
		return;
	}
	tp = checkstring(cmdline, "HUMID");
	if(tp) {
		DHT22(tp);
		return;
	}
	tp = checkstring(cmdline, "BITSTREAM");
	if(tp) {
		int i,num,size;
		uint32_t pin;
		MMFLOAT *a1float=NULL;
		int64_t *a1int=NULL;
		unsigned short *data;
		getargs(&tp, 5,",");
		if(!(argc == 5)) error("Argument count");
		num=getint(argv[2],1,10000);
    	char code;
    	if((code=codecheck(argv[0])))argv[0]+=2;
    	pin = getinteger(argv[0]);
    	if(code)pin=codemap(code, pin);
        if(IsInvalidPin(pin)) error("Invalid pin");
        if(!(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT))  error("Pin | is in use",pin);
        ExtCfg(pin, EXT_DIG_OUT, 0);
        size=parsenumberarray(argv[4],&a1float, &a1int, 3, 1, NULL, false);
        if(size < num)error("Array too small");
        data=GetTempMemory(num * sizeof(unsigned short));
        if(a1float!=NULL){
        	if(*a1float <0 || *a1float>3120)error("Allowed number range (0-3120");
            for(i=0; i< num;i++)data[i]= FloatToUint16(*a1float++);
        } else {
        	if(*a1int <0 || *a1int>3120)error("Allowed number range (0-3120)");
            for(i=0; i< num;i++){data[i]= *a1int++; }
        }
        for(i=0; i< num;i++){
             data[i]*=21;
        }
        // if (data[0]>21) data[0]-=5;
         data[0]-=2;
         //MX_TIM14_Init(83); //1MHz i.e. 84/84
         MX_TIM14_Init(3);  //21MHz  i.e. 84/4
        __disable_irq();
       	HAL_TIM_Base_Start(&htim14);
    	bitstream(pin,data,num);
        HAL_TIM_Base_Stop(&htim14);
        __enable_irq();
		return;
	}
	error("Syntax");
}

/****************************************************************************************************************************
Set the output of a digital I/O pin or set the current count for an interrupt pin
*****************************************************************************************************************************/
void ExtSet(int pin, int val){

    if(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT) {
        PinSetBit(pin, val ? LATSET : LATCLR);
//        INTEnableInterrupts();
    }
    else if(ExtCurrentConfig[pin] == EXT_CNT_IN){ //allow the user to zero the count
        if(pin == INT1PIN) INT1Count=val;
        if(pin == INT2PIN) INT2Count=val;
        if(pin == INT3PIN) INT3Count=val;
        if(pin == INT4PIN) INT4Count=val;

    }
    else
        error("Pin | is not an output",pin);
}



/****************************************************************************************************************************
Get the value of an I/O pin and returns it
For digital returns 0 if low or 1 if high
For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
*****************************************************************************************************************************/
int64_t ExtInp(int pin){
	// read from a digital input
	if(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH || ExtCurrentConfig[pin] == EXT_OC_OUT || ExtCurrentConfig[pin] == EXT_DIG_OUT) {
        return  PinRead(pin);
	}

	// read from a frequency/period input
	if(ExtCurrentConfig[pin] == EXT_FREQ_IN || ExtCurrentConfig[pin] == EXT_PER_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Value;
        if(pin == INT2PIN) return INT2Value;
        if(pin == INT3PIN) return INT3Value;
        if(pin == INT4PIN) return INT4Value;
	}

	// read from a counter input
	if(ExtCurrentConfig[pin] == EXT_CNT_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Count;
        if(pin == INT2PIN) return INT2Count;
        if(pin == INT3PIN) return INT3Count;
        if(pin == INT4PIN) return INT4Count;
	}
	return 0;
}



/****************************************************************************************************************************
New, more portable, method of manipulating an I/O pin
*****************************************************************************************************************************/













void *IrDev, *IrCmd;
volatile char IrVarType;
volatile char IrState, IrGotMsg;
int IrBits, IrCount;
char *IrInterrupt;

void NRFInterrupt(void){

}
/****************************************************************************************************************************
Interrupt service routines for the counting functions (eg, frequency, period)
*****************************************************************************************************************************/


// perform the counting functions for INT0
// this interrupt is also used by the IR command and also to wake us from sleep


// perform the counting functions for INT1
void __attribute__ ((optimize("-O2"))) TM_EXTI_Handler_1(void) {
	if(ExtCurrentConfig[INT1PIN] == EXT_PER_IN) {
        if(--INT1Timer <= 0) {
            INT1Value = INT1Count;
            INT1Timer = INT1InitTimer;
            INT1Count = 0;
        }
	}
    else {
        if(CFuncInt1)
            CallCFuncInt1();                                        // Hardware interrupt 1 for a CFunction (see CFunction.c)
        else
            INT1Count++;
    }

}



// perform the counting functions for INT2
void __attribute__ ((optimize("-O2"))) TM_EXTI_Handler_2(void) {
    if(ExtCurrentConfig[INT2PIN] == EXT_PER_IN) {
        if(--INT2Timer <= 0) {
            INT2Value = INT2Count;
            INT2Timer = INT2InitTimer;
            INT2Count = 0;
        }
    }
    else {
        if(CFuncInt2)
            CallCFuncInt2();                                        // Hardware interrupt 2 for a CFunction (see CFunction.c)
        else
            INT2Count++;
    }
}




// perform the counting functions for INT3
void __attribute__ ((optimize("-O2"))) TM_EXTI_Handler_3(void) {
	if(ExtCurrentConfig[INT3PIN] == EXT_PER_IN) {
        if(--INT3Timer <= 0) {
            INT3Value = INT3Count;
            INT3Timer = INT3InitTimer;
            INT3Count = 0;
        }
	}
	else {
		if(CFuncInt3)
		  CallCFuncInt3();                                        // Hardware interrupt 3 for a CFunction (see CFunction.c)
		else
		  INT3Count++;
	}
}




// perform the counting functions for INT4
void __attribute__ ((optimize("-O2"))) TM_EXTI_Handler_4(void) {
	if(ExtCurrentConfig[INT4PIN] == EXT_PER_IN) {
        if(--INT4Timer <= 0) {
            INT4Value = INT4Count;
            INT4Timer = INT4InitTimer;
            INT4Count = 0;
        }
	}
	else {
		if(CFuncInt4)
		  CallCFuncInt4();                                        // Hardware interrupt 4 for a CFunction (see CFunction.c)
		else
		  INT4Count++;
	}
}

void __attribute__ ((optimize("-O2"))) TM_EXTI_Handler_5(void){
    int ElapsedMicroSec;
    static unsigned int LastIrBits;
        ElapsedMicroSec = readusclock();
        switch(IrState) {
            case IR_WAIT_START:
                writeusclock(0);                                           // reset the timer
                IrState = IR_WAIT_START_END;                        // wait for the end of the start bit
                break;
            case IR_WAIT_START_END:
                if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2800)
                    IrState = SONY_WAIT_BIT_START;                  // probably a Sony remote, now wait for the first data bit
                else if(ElapsedMicroSec > 8000 && ElapsedMicroSec < 10000)
                    IrState = NEC_WAIT_FIRST_BIT_START;             // probably an NEC remote, now wait for the first data bit
                else {
                    IrReset();                                      // the start bit was not valid
                    break;
                }
                IrCount = 0;                                        // count the bits in the message
                IrBits = 0;                                         // reset the bit accumulator
                writeusclock(0);                                           // reset the timer
                break;
            case SONY_WAIT_BIT_START:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 900) { IrReset(); break; }
                writeusclock(0);                                           // reset the timer
                IrState = SONY_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case SONY_WAIT_BIT_END:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 1500 || IrCount > 20) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 900) << IrCount;       // get the data bit
                IrCount++;                                          // and increment our count
                writeusclock(0);                                           // reset the timer
                IrState = SONY_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
            case NEC_WAIT_FIRST_BIT_START:
            	if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2500) {
                    IrBits = LastIrBits;                            // key is held down so just repeat the last code
                    IrCount = 32;                                   // and signal that we are finished
                    IrState = NEC_WAIT_BIT_END;
                    break;
                }
                else if(ElapsedMicroSec > 4000 && ElapsedMicroSec < 5000)
                    IrState = NEC_WAIT_BIT_END;                     // wait for the end of this data bit
                else {
                    IrReset();                                      // the start bit was not valid
                    break;
                }
                writeusclock(0);                                           // reset the timer
                break;
            case NEC_WAIT_BIT_START:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 1800) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 840) << (31 - IrCount);// get the data bit
                LastIrBits = IrBits;
                IrCount++;                                          // and increment our count
                writeusclock(0);                                           // reset the timer
                IrState = NEC_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case NEC_WAIT_BIT_END:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 700) { IrReset(); break; }
                if(IrCount == 32) break;
                writeusclock(0);                                           // reset the timer
                IrState = NEC_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
        }
    }

void __attribute__ ((optimize("-O2"))) HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_1)
  {
	  TM_EXTI_Handler_1();
  }
  if (GPIO_Pin == GPIO_PIN_2)
  {
	  TM_EXTI_Handler_5();
  }
  if (GPIO_Pin == GPIO_PIN_3)
  {
	  TM_EXTI_Handler_2();
  }
  if (GPIO_Pin == GPIO_PIN_4)
  {
	  TM_EXTI_Handler_3();
  }
  if (GPIO_Pin == GPIO_PIN_8)
  {
	  TM_EXTI_Handler_4();
  }
//#ifndef RGT
  //if(HAS_100PINS){
    if (GPIO_Pin == GPIO_PIN_15)
    {
	  CNInterrupt();
    }
  //}
//#endif
}
int IsInvalidPin(int pin) {
    if(pin < 1 || pin > NBRPINS) return true;
    if(PinDef[pin].mode & PUNUSED) return true;
    return false;
}

int MIPS16 CheckPin(int pin, int action) {

    if(pin < 1 || pin > NBRPINS || (PinDef[pin].mode & PUNUSED)) {
    if(!(action & CP_NOABORT)) error("Pin | is invalid", pin);
    return false;
}

if(!(action & CP_IGNORE_INUSE) && ExtCurrentConfig[pin] > EXT_NOT_CONFIG && ExtCurrentConfig[pin] < EXT_COM_RESERVED) {
    if(!(action & CP_NOABORT)) error("Pin | is in use", pin);
    return false;
}

if(!(action & CP_IGNORE_BOOTRES) && ExtCurrentConfig[pin] == EXT_BOOT_RESERVED) {
    if(!(action & CP_NOABORT)) {
        error("Pin | is reserved on startup", pin);
        uSec(1000000);
    }
    return false;
}
if(!(action & CP_IGNORE_RESERVED) && ExtCurrentConfig[pin] >= EXT_COM_RESERVED) {
    if(!(action & CP_NOABORT)) error("Pin | is in use", pin);
    return false;
}

    return true;
}
