/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "configuration.h"
#include "serial.h"
#include "gps.h"
#include "stm32f4xx_ll_gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern DAC_HandleTypeDef hdac;
extern SD_HandleTypeDef hsd;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim10;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;
/* USER CODE BEGIN EV */
extern RTC_HandleTypeDef hrtc;
extern void Timer1msHandler(void);
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern void Audio_Interrupt(void);
extern volatile uint64_t Count5High;
extern int64_t *d1point, *d2point;
extern int d1max, d2max;
extern volatile int d1pos, d2pos;
extern int64_t *a1point, *a2point, *a3point;
extern int ADCmax;
extern volatile int ADCpos;
extern volatile int ADCchannelA;
extern volatile int ADCchannelB;
extern volatile int ADCchannelC;
extern int ADCtriggervalue;
extern int ADCtriggerchannel;
extern int ADCnegativeslope;
extern volatile int ADCcomplete;
extern void MIPS16 error(char *msg, ...);
extern volatile int periodstarted;
extern volatile int ConsoleRxBufHead;
extern volatile int ConsoleRxBufTail;
extern char ConsoleTxBuf[];
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern char ConsoleRxBuf[];
extern char BreakKey;
extern char *KeyInterrupt;
extern volatile int Keycomplete;
extern int keyselect;
extern volatile int MMAbort;
extern volatile struct option_s Option, *SOption;
extern char SerialConDisabled;
uint64_t TIM12count=0;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  Timer1msHandler();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
  */
void TIM1_UP_TIM10_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */
//	LL_GPIO_SetOutputPin(PWM_1D_LCD_BL_GPIO_Port, PWM_1D_LCD_BL_Pin);
	HAL_TIM_Base_Stop_IT(&htim10);
    TIM10->SR=0;
	return;
  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  HAL_TIM_IRQHandler(&htim10);
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	if(SerialConDisabled){
		uint32_t isrflags   = READ_REG(huart1.Instance->SR);
		if ((isrflags & USART_SR_RXNE) != RESET){
			char cc = huart1.Instance->DR;
			if(GPSchannel==1){
				*gpsbuf=cc;
				gpsbuf++;
				gpscount++;
				if((char)cc==10 || gpscount==128){
					if(gpscurrent){
						*gpsbuf=0;
						gpscurrent=0;
						gpscount=0;
						gpsbuf=gpsbuf1;
						gpsready=gpsbuf2;
					} else {
						*gpsbuf=0;
						gpscurrent=1;
						gpscount=0;
						gpsbuf=gpsbuf2;
						gpsready=gpsbuf1;

					}
				}
			} else {
				com1Rx_buf[com1Rx_head]  = cc;   // store the byte in the ring buffer
				com1Rx_head = (com1Rx_head + 1) % com1_buf_size;     // advance the head of the queue
				if(com1Rx_head == com1Rx_tail) {                           // if the buffer has overflowed
					com1Rx_tail = (com1Rx_tail + 1) % com1_buf_size; // throw away the oldest char
				}
			}
		}
		if ((isrflags & USART_SR_TC) != RESET){
			if(com1Tx_head != com1Tx_tail) {
				huart1.Instance->DR = com1Tx_buf[com1Tx_tail];
				com1Tx_tail = (com1Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
			} else {
				huart1.Instance->CR1 &= ~USART_CR1_TCIE;
			}
		}
	} else {
	  	uint32_t isrflags   = READ_REG(huart1.Instance->SR);
	  	if ((isrflags & USART_SR_RXNE) != RESET){
	  		ConsoleRxBuf[ConsoleRxBufHead]  = huart1.Instance->DR;   // store the byte in the ring buffer
	  		if(BreakKey && ConsoleRxBuf[ConsoleRxBufHead] == BreakKey) {// if the user wants to stop the progran
	  			MMAbort = true;                                        // set the flag for the interpreter to see
	  			ConsoleRxBufHead = ConsoleRxBufTail;                    // empty the buffer

	  		} else if(ConsoleRxBuf[ConsoleRxBufHead] ==keyselect && KeyInterrupt!=NULL){
	  					Keycomplete=1;
	  		} else {
	  			ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;     // advance the head of the queue
	  			if(ConsoleRxBufHead == ConsoleRxBufTail) {                           // if the buffer has overflowed
	  				ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE; // throw away the oldest char
	  			}
	  		}
	  	}
	  	if ((isrflags & USART_SR_TC) != RESET){
	  		if(ConsoleTxBufTail != ConsoleTxBufHead) {
	  			huart1.Instance->DR = ConsoleTxBuf[ConsoleTxBufTail];
	  			ConsoleTxBufTail = (ConsoleTxBufTail + 1) % CONSOLE_TX_BUF_SIZE; // advance the tail of the queue
	  		} else {
	  	        huart1.Instance->CR1 &= ~USART_CR1_TCIE;
	  		}
	  	}
	}
  	return;
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	uint32_t isrflags   = READ_REG(huart2.Instance->SR);
	if ((isrflags & USART_SR_RXNE) != RESET){
	  char cc = huart2.Instance->DR;
      if(GPSchannel==4){
          *gpsbuf=cc;
          gpsbuf++;
          gpscount++;
          if((char)cc==10 || gpscount==128){
             if(gpscurrent){
                 *gpsbuf=0;
                 gpscurrent=0;
                 gpscount=0;
                 gpsbuf=gpsbuf1;
                 gpsready=gpsbuf2;
             } else {
                 *gpsbuf=0;
                 gpscurrent=1;
                 gpscount=0;
                 gpsbuf=gpsbuf2;
                 gpsready=gpsbuf1;

             }
          }
      } else {
      	com4Rx_buf[com4Rx_head]  = cc;   // store the byte in the ring buffer
      	com4Rx_head = (com4Rx_head + 1) % com4_buf_size;     // advance the head of the queue
      	if(com4Rx_head == com4Rx_tail) {                           // if the buffer has overflowed
      		com4Rx_tail = (com4Rx_tail + 1) % com4_buf_size; // throw away the oldest char
      	}
      }
	}
	if ((isrflags & USART_SR_TC) != RESET){
		if(com4Tx_head != com4Tx_tail) {
			huart2.Instance->DR = com4Tx_buf[com4Tx_tail];
		    com4Tx_tail = (com4Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
		} else {
	        huart2.Instance->CR1 &= ~USART_CR1_TCIE;
		}
	}
	return;
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles SDIO global interrupt.
  */
void SDIO_IRQHandler(void)
{
  /* USER CODE BEGIN SDIO_IRQn 0 */

  /* USER CODE END SDIO_IRQn 0 */
  HAL_SD_IRQHandler(&hsd);
  /* USER CODE BEGIN SDIO_IRQn 1 */

  /* USER CODE END SDIO_IRQn 1 */
}

/**
  * @brief This function handles TIM5 global interrupt.
  */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM5_IRQn 0 */
   	Audio_Interrupt();
   	TIM5->SR=0;
   	return;

  /* USER CODE END TIM5_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM5_IRQn 1 */

  /* USER CODE END TIM5_IRQn 1 */
}
void TIM8_BRK_TIM12_IRQHandler(void)
{
  /* USER CODE BEGIN TIM8_BRK_TIM12_IRQn 0 */
   	TIM12->SR=0;
   	TIM12count++;
   	return;
  /* USER CODE END TIM8_BRK_TIM12_IRQn 0 */
//  HAL_TIM_IRQHandler(&htim12);
  /* USER CODE BEGIN TIM8_BRK_TIM12_IRQn 1 */

  /* USER CODE END TIM8_BRK_TIM12_IRQn 1 */
}
/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */
  goto skip4;
  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */
  skip4:;
	uint32_t isrflags   = READ_REG(huart4.Instance->SR);
	if ((isrflags & USART_SR_RXNE) != RESET){
		char cc = huart4.Instance->DR;
        if(GPSchannel==3){
            *gpsbuf=cc;
            gpsbuf++;
            gpscount++;
            if((char)cc==10 || gpscount==128){
               if(gpscurrent){
                   *gpsbuf=0;
                   gpscurrent=0;
                   gpscount=0;
                   gpsbuf=gpsbuf1;
                   gpsready=gpsbuf2;
               } else {
                   *gpsbuf=0;
                   gpscurrent=1;
                   gpscount=0;
                   gpsbuf=gpsbuf2;
                   gpsready=gpsbuf1;

               }
            }
        } else {
        	com3Rx_buf[com3Rx_head]  = cc;   // store the byte in the ring buffer
        	com3Rx_head = (com3Rx_head + 1) % com3_buf_size;     // advance the head of the queue
        	if(com3Rx_head == com3Rx_tail) {                           // if the buffer has overflowed
        		com3Rx_tail = (com3Rx_tail + 1) % com3_buf_size; // throw away the oldest char
        	}
        }
	}
	if ((isrflags & USART_SR_TC) != RESET){
		if(com3Tx_head != com3Tx_tail) {
			huart4.Instance->DR = com3Tx_buf[com3Tx_tail];
		    com3Tx_tail = (com3Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
		} else {
	        huart4.Instance->CR1 &= ~USART_CR1_TCIE;
		}
	}
  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
	if(d1max){
		if(!d2max){
			HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1, DAC_ALIGN_12B_R, d1point[d1pos]);
			d1pos++;
			if(d1pos>d1max)d1pos=0;
		} else {
			HAL_DACEx_DualSetValue(&hdac, DAC_ALIGN_12B_R, d1point[d1pos], d2point[d2pos]);
			d1pos++;
			if(d1pos>d1max)d1pos=0;
			d2pos++;
			if(d2pos>d2max)d2pos=0;
		}
	}
	TIM6->SR=0;
	return;
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_DAC_IRQHandler(&hdac);
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
	static int lastread, ADCtriggerfound;
	int c, c1=0, c2=0, c3=0;
//	a=10000; while (HAL_IS_BIT_CLR(hadc1.Instance->SR, EOC_SINGLE_CONV) && a--);
	HAL_ADC_PollForConversion(&hadc1, 10);
	c1=HAL_ADC_GetValue(&hadc1);
	if(ADCpos < ADCmax)HAL_ADC_Start(&hadc1);
	if(ADCchannelB){
//		a=10000; while (HAL_IS_BIT_CLR(hadc3.Instance->SR, EOC_SINGLE_CONV) && a--);
		HAL_ADC_PollForConversion(&hadc3, 10);
		c2=HAL_ADC_GetValue(&hadc3);
		if(ADCpos < ADCmax)HAL_ADC_Start(&hadc3);
	}
	if(ADCchannelC){
//		a=10000; while (HAL_IS_BIT_CLR(hadc2.Instance->SR, EOC_SINGLE_CONV) && a--);
		HAL_ADC_PollForConversion(&hadc2, 10);
		c3=HAL_ADC_GetValue(&hadc2);
		if(ADCpos < ADCmax)HAL_ADC_Start(&hadc2);
	}
	a1point[ADCpos]=c1;
	if(ADCchannelB)a2point[ADCpos]=c2;
	if(ADCchannelC)a3point[ADCpos]=c3;
	if(ADCpos==ADCmax){
		HAL_TIM_Base_Stop(&htim7);
		HAL_TIM_Base_DeInit(&htim7);
		ADCcomplete=true;
		TIM7->SR=0;
		return;
	}
	if(ADCtriggerchannel){
		if(ADCtriggerchannel==1)c=c1;
		else if(ADCtriggerchannel==2)c=c2;
		else c=c3;
		if(ADCpos==0){
			ADCtriggerfound=0;
			lastread=c;
		} else if(!ADCtriggerfound){
			if(ADCnegativeslope){ //if looking for down slope
				if(lastread>=ADCtriggervalue && c<ADCtriggervalue){
					ADCtriggerfound=1;
				} else {
					lastread=c;
					ADCpos--;
					a1point[ADCpos]=c1;
					if(ADCchannelB)a2point[ADCpos]=c2;
					if(ADCchannelC)a3point[ADCpos]=c3;
				}
			} else {
				if(lastread<=ADCtriggervalue && c>ADCtriggervalue){
					ADCtriggerfound=1;
				} else {
					lastread=c;
					ADCpos--;
					a1point[ADCpos]=c1;
					if(ADCchannelB)a2point[ADCpos]=c2;
					if(ADCchannelC)a3point[ADCpos]=c3;
				}
			}
		}
	}
	ADCpos++;
	TIM7->SR=0;
	return;
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */
  goto skip6;
  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */
  skip6:;
	uint32_t isrflags   = READ_REG(huart6.Instance->SR);
	if ((isrflags & USART_SR_RXNE) != RESET){
		char cc = huart6.Instance->DR;
        if(GPSchannel==2){
            *gpsbuf=cc;
            gpsbuf++;
            gpscount++;
            if((char)cc==10 || gpscount==128){
               if(gpscurrent){
                   *gpsbuf=0;
                   gpscurrent=0;
                   gpscount=0;
                   gpsbuf=gpsbuf1;
                   gpsready=gpsbuf2;
               } else {
                   *gpsbuf=0;
                   gpscurrent=1;
                   gpscount=0;
                   gpsbuf=gpsbuf2;
                   gpsready=gpsbuf1;

               }
            }
        } else {
        	com2Rx_buf[com2Rx_head]  = cc;   // store the byte in the ring buffer
        	com2Rx_head = (com2Rx_head + 1) % com2_buf_size;     // advance the head of the queue
        	if(com2Rx_head == com2Rx_tail) {                           // if the buffer has overflowed
        		com2Rx_tail = (com2Rx_tail + 1) % com2_buf_size; // throw away the oldest char
        	}
        }
	}
	if ((isrflags & USART_SR_TC) != RESET){
		if(com2Tx_head != com2Tx_tail) {
			huart6.Instance->DR = com2Tx_buf[com2Tx_tail];
		    com2Tx_tail = (com2Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
		} else {
	        huart6.Instance->CR1 &= ~USART_CR1_TCIE;
		}
	}
  /* USER CODE END USART6_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void RTC_WKUP_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}
void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR & EXTI_PR_PR8_Msk) HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
	if(EXTI->PR & EXTI_PR_PR6_Msk) HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
