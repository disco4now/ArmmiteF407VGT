/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

CAN.h

Include file that contains the globals and defines for SPI in MMBasic.

Copyright 2011-2023 Geoff Graham and  Peter Mather.
Copyright 2024  Gerry Allardice.

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

void cmd_can(void);


#endif


/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
	{ "CAN",	T_CMD,				0, cmd_can	},
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
	//{ "CAN(",	T_FUN | T_INT,		0, fun_can,	},
#endif



/***********************************************************************************
 Function prototypes
***********************************************************************************/

//#ifdef INCLUDE_FUNCTION_DEFINES
#ifndef CAN_H
#define CAN_H

void CanInit(void);

#endif /* CAN_H */
//#endif /* INCLUDE_FUNCTION_DEFINES */

	/* Exported constants --------------------------------------------------------*/
	/* User can use this section to tailor FDCANx instance used and associated
	   resources */
	/* Definition for FDCANx clock resources */
//	#define FDCANx                      FDCAN1
//	#define FDCANx_CLK_ENABLE()         __HAL_RCC_FDCAN_CLK_ENABLE()
//	#define FDCANx_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
//	#define FDCANx_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

//	#define FDCANx_FORCE_RESET()   __HAL_RCC_FDCAN_FORCE_RESET()
//	#define FDCANx_RELEASE_RESET() __HAL_RCC_FDCAN_RELEASE_RESET()

	/* Definition for FDCANx Pins */
//	#define FDCANx_TX_PIN       GPIO_PIN_12
//	#define FDCANx_TX_GPIO_PORT GPIOA
//	#define FDCANx_TX_AF        GPIO_AF9_FDCAN1
//	#define FDCANx_RX_PIN       GPIO_PIN_11
//	#define FDCANx_RX_GPIO_PORT GPIOA
//	#define FDCANx_RX_AF        GPIO_AF9_FDCAN1

	/* Definition for FDCANx's NVIC IRQ and IRQ Handlers */
//	#define FDCANx_IT0_IRQn   FDCAN1_IT0_IRQn
//	#define FDCANx_IT1_IRQn   FDCAN1_IT1_IRQn
//	#define FDCANx_IRQHandler FDCAN1_IRQHandler

	// global variables


/************************************************* end of CAN.h *******************/
