/*-*****************************************************************************

ArmmiteF4 MMBasic

MM_Custom.c

Handles all the custom commands and functions in the Armmite F4 implementation of MMBasic.
These are commands and functions that are not normally part of the MMBasic.
This is a good place to insert your own customised commands.


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

#include <stdio.h>

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include <math.h>
#include <complex.h>
MMFLOAT PI;
typedef MMFLOAT complex cplx;
#define max(x, y) (((x) > (y)) ? (x) : (y))
    #define hdac1 hdac


/*************************************************************************************************************************
**************************************************************************************************************************
IMPORTANT:
This module is empty and should be used for your special functions and commands.  In the standard distribution this file
will never be changed, so your code should be safe here.  You should avoid placing commands and functions in other files as
they may be changed and you would then need to re insert your changes in a new release of the source.

**************************************************************************************************************************
**************************************************************************************************************************/

/********************************************************************************************************************************************
 custom commands and functions
 each function is responsible for decoding a command
 all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a basic function) so, if you want to search for the
 function responsible for the NAME command look for cmd_name

 There are 4 items of information that are setup before the command is run.
 All these are globals.

 int cmdtoken   This is the token number of the command (some commands can handle multiple
                statement types and this helps them differentiate)

 char *cmdline  This is the command line terminated with a zero char and trimmed of leading
                spaces.  It may exist anywhere in memory (or even ROM).

 char *nextstmt This is a pointer to the next statement to be executed.  The only thing a
                command can do with it is save it or change it to some other location.

 char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

 The only actions a command can do to change the program flow is to change nextstmt or
 execute longjmp(mark, 1) if it wants to abort the program.

 ********************************************************************************************************************************************/
/**
  * @brief  Sets the TIM Autoreload Register value on runtime without calling
  *         another time any Init function.
  * @param  __HANDLE__: TIM handle.
  * @param  __AUTORELOAD__: specifies the Counter register new value.
  * @retval None
  */
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
int64_t *d1point=NULL, *d2point=NULL;
int d1max, d2max;
volatile int d1pos, d2pos;
int64_t *a1point=NULL, *a2point=NULL, *a3point=NULL;
MMFLOAT *a1float=NULL, *a2float=NULL, *a3float=NULL;
int ADCmax=0;
volatile int ADCpos=0;
extern int ADC_init(int32_t pin);
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
    extern unsigned char ADCbits[55];
volatile int ADCchannelA=0;
volatile int ADCchannelB=0;
volatile int ADCchannelC=0;
int ADCtriggervalue=0;
int ADCtriggerchannel=0;
int ADCnegativeslope=0;
char *ADCInterrupt;
int ADCNumchannels=0;
volatile int ADCcomplete = false;
static void MX_TIM6_Init(int prescale, int period)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = prescale;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = period;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    error("HAL_TIM_Base_Init");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    error("HAL_TIMEx_MasterConfigSynchronization");
  }
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    /* Initialization Error */
    error("HAL_TIM_Base_Init");
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
    /* Starting Error */
      error("HAL_TIM_Base_Start_IT");
  }
}
static void MX_TIM7_Init(int prescale, int period)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = prescale;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = period;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    error("HAL_TIM_Base_Init");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    error("HAL_TIMEx_MasterConfigSynchronization");
  }
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    /* Initialization Error */
    error("HAL_TIM_Base_Init");
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
  {
    /* Starting Error */
      error("HAL_TIM_Base_Start_IT");
  }
}

void dacclose(void){
    CurrentlyPlaying=P_NOTHING;
    if(d1max){
        HAL_TIM_Base_Stop(&htim6);
        HAL_TIM_Base_DeInit(&htim6);
    }
    d1max=0;
    d2max=0;
}
void ADCclose(void){
    if(ADCchannelA){
        HAL_TIM_Base_Stop(&htim7);
        HAL_TIM_Base_DeInit(&htim7);
        if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
        {
            // ADC de-initialization Error /
            error("HAL_ADC_DeInitA");
        }
        ExtCfg(ADCchannelA, EXT_NOT_CONFIG, 0);
        }
    if(ADCchannelB){
        if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
        {
            //* ADC de-initialization Error/
            error("HAL_ADC_DeInitB");
        }
        ExtCfg(ADCchannelB, EXT_NOT_CONFIG, 0);
    }
    if(ADCchannelC){
        if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
        {
            //* ADC de-initialization Error/
            error("HAL_ADC_DeInitC");
        }
        ExtCfg(ADCchannelC, EXT_NOT_CONFIG, 0);
    }
    ADCchannelA = ADCchannelB = ADCchannelC = ADCtriggerchannel = ADCNumchannels = ADCpos = ADCmax = 0;
    ADCInterrupt=NULL;
    ADCcomplete = false;
}
void cmd_ADC(void){
    char *tp;
    static int  prescale, period;
    ADC_ChannelConfTypeDef sConfigA, sConfigB, sConfigC;
    int a,b,c;
    MMFLOAT freq=0.0;
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    void *ptr3 = NULL;
    tp = checkstring(cmdline, "OPEN");
    if(tp) {
        getargs(&tp, 9, ",");
        if(argc<3)error("Syntax");
        if(ADCchannelA)error("ADC already open");
        memset(&sConfigA,0,sizeof(ADC_ChannelConfTypeDef));
        memset(&sConfigB,0,sizeof(ADC_ChannelConfTypeDef));
        ADCchannelA = ADCchannelB = ADCchannelC = 0;
        if(argc == 9) {
            InterruptUsed = true;
            ADCInterrupt = GetIntAddress(argv[8]);                          // get the interrupt location
        } else
            ADCInterrupt = NULL;
        freq=getnumber(argv[0]);
        freq*=2;
        prescale=(int)((MMFLOAT)(SystemCoreClock*2)/(freq*50000.0L));
        period=(int)(((MMFLOAT)SystemCoreClock/(MMFLOAT)(prescale+1))/freq);
        char code;
        if((code=codecheck(argv[2])))argv[2]+=2;
        ADCchannelA = getinteger(argv[2]);
        if(code)ADCchannelA=codemap(code, ADCchannelA);
        ADCbits[ADCchannelA]=12;
        if(freq>160000.0)ADCbits[ADCchannelA]=10;
        if(freq>320000.0)ADCbits[ADCchannelA]=8;

        if(argc>3 && *argv[4]){
        char code;
        if((code=codecheck(argv[4])))argv[4]+=2;
        ADCchannelB = getinteger(argv[4]);
        if(code)ADCchannelB=codemap(code, ADCchannelB);
        }

        if(argc>5 && *argv[6]){
            char code;
            if((code=codecheck(argv[6])))argv[6]+=2;
            ADCchannelC = getinteger(argv[6]);
            if(code)ADCchannelC=codemap(code, ADCchannelC);
        }

        if(PinDef[ADCchannelA].ADCn!=ADC1){
            ADCchannelA = ADCchannelB = ADCchannelC = 0;
            error("First channel must use ANALOG_A pin");
        }
       // CheckPin(ADCchannelA, CP_IGNORE_INUSE);
       	a=ADCchannelA;ADCchannelA=0;
    	CheckPin(a, CP_IGNORE_INUSE);
    	ADCchannelA=a;

       // ExtCfg(ADCchannelA, EXT_ANA_IN, ADCbits[ADCchannelA]);
       // ExtCfg(ADCchannelA, EXT_COM_RESERVED, 0);
        ADCNumchannels=1;

        if(ADCchannelB){
            if(PinDef[ADCchannelB].ADCn!=ADC3){
                ADCchannelA = ADCchannelB = ADCchannelC = 0;
                error("Second channel must use ANALOG_C pin");
            }
           // CheckPin(ADCchannelB, CP_IGNORE_INUSE);
            a=ADCchannelA;ADCchannelA=0;
            b=ADCchannelB;ADCchannelB=0;
            CheckPin(b, CP_IGNORE_INUSE);
            ADCchannelA=a;ADCchannelB=b;

            ADCbits[ADCchannelB]=ADCbits[ADCchannelA];
            //ExtCfg(ADCchannelB, EXT_ANA_IN, ADCbits[ADCchannelB]);
            //ExtCfg(ADCchannelB, EXT_COM_RESERVED, 0);
            ADCNumchannels++;
        }

        if(ADCchannelC){
            if(PinDef[ADCchannelC].ADCn!=ADC2){
                ADCchannelA = ADCchannelB = ADCchannelC = 0;
                error("Third channel must use ANALOG_B pin");
            }
            //CheckPin(ADCchannelC, CP_IGNORE_INUSE);
            a=ADCchannelA;ADCchannelA=0;
            b=ADCchannelB;ADCchannelB=0;
            c=ADCchannelC;ADCchannelC=0;
            CheckPin(c, CP_IGNORE_INUSE);
            ADCchannelA=a;ADCchannelB=b;ADCchannelC=c;

            ADCbits[ADCchannelC]=ADCbits[ADCchannelA];
            //ExtCfg(ADCchannelC, EXT_ANA_IN, ADCbits[ADCchannelC]);
            //ExtCfg(ADCchannelC, EXT_COM_RESERVED, 0);
            ADCNumchannels++;
        }
        ExtCfg(ADCchannelA, EXT_ANA_IN, ADCbits[ADCchannelA]);
        ExtCfg(ADCchannelA, EXT_COM_RESERVED, 0);
        ADC_init(ADCchannelA);
        sConfigA.Channel      = PinDef[ADCchannelA].ADCchannel;                /* Sampled channel number */
        sConfigA.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
        sConfigA.SamplingTime = ADC_SAMPLETIME_3CYCLES;    /* Sampling time (number of clock cycles unit) */
        sConfigA.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


        if (HAL_ADC_ConfigChannel(&hadc1, &sConfigA) != HAL_OK)
        {
            /* Channel Configuration Error */
            ADCchannelA = ADCchannelB = ADCchannelC = 0;
            error("HAL_ADC_ConfigChannelA");
        }

        if(ADCchannelB){
        	ExtCfg(ADCchannelB, EXT_ANA_IN, ADCbits[ADCchannelB]);
        	ExtCfg(ADCchannelB, EXT_COM_RESERVED, 0);
            ADC_init(ADCchannelB);
            sConfigB.Channel      = PinDef[ADCchannelB].ADCchannel;                /* Sampled channel number */
            sConfigB.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
            sConfigB.SamplingTime = ADC_SAMPLETIME_3CYCLES;    /* Sampling time (number of clock cycles unit) */
            sConfigB.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


            if (HAL_ADC_ConfigChannel(&hadc3, &sConfigB) != HAL_OK)
            {
                /* Channel Configuration Error */
                ADCchannelA = ADCchannelB = ADCchannelC = 0;
                error("HAL_ADC_ConfigChannelB");
            }
        }
        if(ADCchannelC){
        	ExtCfg(ADCchannelC, EXT_ANA_IN, ADCbits[ADCchannelC]);
        	ExtCfg(ADCchannelC, EXT_COM_RESERVED, 0);
            ADC_init(ADCchannelC);
            sConfigC.Channel      = PinDef[ADCchannelC].ADCchannel;                /* Sampled channel number */
            sConfigC.Rank         = 1;          /* Rank of sampled channel number ADCx_CHANNEL */
            sConfigC.SamplingTime = ADC_SAMPLETIME_3CYCLES;    /* Sampling time (number of clock cycles unit) */
            sConfigC.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


            if (HAL_ADC_ConfigChannel(&hadc2, &sConfigC) != HAL_OK)
            {
                /* Channel Configuration Error */
                ADCchannelA = ADCchannelB = ADCchannelC = 0;
                error("HAL_ADC_ConfigChannelC");
            }
        }
        return;
    }
    tp = checkstring(cmdline, "START");
    if(tp) {
        getargs(&tp, 5, ",");
        if(!ADCchannelA)error("ADC not open");
        if(!(argc >= 1))error("Argument count");
        a1point=NULL; a2point=NULL; a3point=NULL;
        ADCmax=0;
        ADCpos=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_NBR) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be float array");
            }
            a1point = (int64_t *)ptr1;
            a1float = (MMFLOAT *)ptr1;
        } else error("Argument 1 must be float array");
        ADCmax=(vartbl[VarIndex].dims[0] - OptionBase);
        if(argc>=3){
           if(!ADCchannelB)error("Second channel not open");
           ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                    error("Argument 2 must be float array");
                }
                a2point = (int64_t *)ptr2;
                a2float = (MMFLOAT *)ptr2;
            } else error("Argument 2 must be float array");
            if((vartbl[VarIndex].dims[0] - OptionBase) !=ADCmax)error("Arrays should be the same size");
        }
        if(argc>=5){
           if(!ADCchannelC)error("Third channel not open");
           ptr3 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
            if(vartbl[VarIndex].type & T_NBR) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                    error("Argument 3 must be float array");
                }
                a3point = (int64_t *)ptr3;
                a3float = (MMFLOAT *)ptr3;
            } else error("Argument 3 must be float array");
            if((vartbl[VarIndex].dims[0] - OptionBase) !=ADCmax)error("Arrays should be the same size");
        }
        if (HAL_ADC_Start(&hadc1) != HAL_OK)
        {
            /* Start Conversation Error */
            error("HAL_ADC_StartA");
        }
        if(ADCchannelB){
            if (HAL_ADC_Start(&hadc3) != HAL_OK)
            {
                /* Start Conversation Error */
                error("HAL_ADC_StartB");
            }
        }
        if(ADCchannelC){
            if (HAL_ADC_Start(&hadc2) != HAL_OK)
            {
                /* Start Conversation Error */
                error("HAL_ADC_StartC");
            }
        }
        MX_TIM7_Init(prescale,period);
        if(ADCInterrupt==NULL){
            int i;
            while(!ADCcomplete){
                CheckAbort();
                CheckSDCard();
            }
        for(i=0;i<=ADCmax;i++){
            if(ADCbits[ADCchannelA]==12)a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0XFFF * VCC);
            else if(ADCbits[ADCchannelA]==10)a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0X3FF * VCC);
            else if(ADCbits[ADCchannelA])a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0XFF * VCC);
            if(ADCchannelB){
                if(ADCbits[ADCchannelB]==12)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0XFFF * VCC);
                else if(ADCbits[ADCchannelB]==10)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0X3FF * VCC);
                else if(ADCbits[ADCchannelB]==8)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0XFF * VCC);
            }
            if(ADCchannelC){
                if(ADCbits[ADCchannelC]==12)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0XFFF * VCC);
                else if(ADCbits[ADCchannelC]==10)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0X3FF * VCC);
                else if(ADCbits[ADCchannelC]==8)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0XFF * VCC);
            }
        }
        }
        return;
    }
    tp = checkstring(cmdline, "CLOSE");
    if(tp) {
        if(!ADCchannelA)error("ADC not open");
        ADCclose();
        return;
    }
    tp = checkstring(cmdline, "TRIGGER");
    if(tp) {
        MMFLOAT voltage;
        getargs(&tp, 3, ",");
        if(argc!=3)error("Syntax");
        ADCtriggerchannel=getint(argv[0],1,ADCNumchannels);
        voltage = getnumber(argv[2]);
        if(voltage<=-VCC || voltage >=VCC) error("Invalid Voltage");
        ADCnegativeslope=0;
        if(voltage<0.0){
            ADCnegativeslope=1;
            voltage=-voltage;
        }
        if(ADCbits[ADCchannelA]==12)ADCtriggervalue=(int)(voltage/VCC*(MMFLOAT)0XFFF);
        else if(ADCbits[ADCchannelA]==10)ADCtriggervalue=(int)(voltage/VCC*(MMFLOAT)0X3FF);
        else if(ADCbits[ADCchannelA]==8)ADCtriggervalue=(int)(voltage/VCC*(MMFLOAT)0XFF);
        return;
    }
    tp = checkstring(cmdline, "FREQUENCY");
    if(tp) {
        int newbits;
        getargs(&tp, 1, ",");
        if(!ADCchannelA)error("ADC not open");
        freq=getnumber(argv[0]);
        newbits=16;
        if(freq>500000.0)error("Frequency greater than 500KHz");
        if(freq>40000.0)newbits=14;
        if(freq>80000.0)newbits=12;
        if(freq>160000.0)newbits=10;
        if(freq>320000.0)newbits=8;
        if(ADCbits[ADCchannelA]!=newbits)error("Invalid frequency change - use CLOSE then OPEN");
        prescale=(int)((MMFLOAT)(SystemCoreClock*2)/(freq*50000.0L));
        period=(int)(((MMFLOAT)SystemCoreClock/(MMFLOAT)(prescale+1))/freq);
        return;
    }
}
//#endif
void cmd_DAC(void){
    char *tp;
    int channel;
    uint16_t dacvalue;
    float voltage;
    MMFLOAT freq;
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int  prescale, period;
    tp = checkstring(cmdline, "STOP");
    if(tp){
        dacclose();
        return;
    }
    tp = checkstring(cmdline, "START");
    if(tp){
        getargs(&tp, 5, ",");
        if(CurrentlyPlaying != P_NOTHING) error("DAC in use");
        if(!(argc == 5 || argc == 3))error("Argument count");
        freq=getnumber(argv[0]);
        if(freq>700000) error("Frequency > 700KHz");
        freq*=2;
        d1max=0;
        d2max=0;
        d1pos=0;
        d2pos=0;
        ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 2 must be integer array");
            }
            d1point = (int64_t *)ptr1;
        } else error("Argument 2 must be integer array");
        d1max=(vartbl[VarIndex].dims[0] - OptionBase);
        if(argc==5){
            ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK);
            if(vartbl[VarIndex].type & T_INT) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                    error("Argument 3 must be integer array");
                }
                d2point = (int64_t *)ptr2;
            } else error("Argument 3 must be integer array");
            d2max=(vartbl[VarIndex].dims[0] - OptionBase);
        }
        CurrentlyPlaying=P_DAC;
        prescale=(int)((MMFLOAT)(SystemCoreClock*2)/(freq*50000.0L));
        period=(int)(((MMFLOAT)SystemCoreClock/(MMFLOAT)(prescale+1))/freq);
        MX_TIM6_Init(prescale,period);
        return;
    }
    getargs(&cmdline, 9, ",");
    if((argc & 0x01) == 0 || argc < 3) error("Invalid syntax");

    channel = getint(argv[0], 1, 2);
    voltage = getnumber(argv[2]);
    if(voltage<0 || voltage >=VCC) error("Invalid Voltage");
    dacvalue=(uint16_t)(voltage/VCC*4096.0);
    switch(channel) {
        case 1:  {
            HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacvalue);
            break;
        }
        case 2:  {
            HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, dacvalue);
            break;
        default: error("No such DAC");
        }
    }
}
