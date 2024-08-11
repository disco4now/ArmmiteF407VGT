/*-*****************************************************************************

ArmmiteF4 MMBasic

Serial.c

Handles the serial I/O  commands and functions in MMBasic..

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
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart3;
extern char Feather;

// variables for com1
int com1 = 0;														// true if COM1 is enabled
int com1_buf_size;													// size of the buffer used to receive chars
int com1_baud = 0;													// determines the baud rate
char *com1_interrupt, *com1_TX_interrupt;												// pointer to the interrupt routine
int com1_ilevel;													// number nbr of chars in the buffer for an interrupt
int com1_TX_complete = false;
unsigned char *com1Rx_buf;											// pointer to the buffer for received characters
volatile int com1Rx_head, com1Rx_tail;								// head and tail of the ring buffer for com1
unsigned char *com1Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com1Tx_head, com1Tx_tail;								// head and tail of the ring buffer for com1
volatile int com1complete=1;
uint16_t Rx1Buffer;


// variables for com2
int com2 = 0;														// true if COM2 is enabled
int com2_buf_size;													// size of the buffer used to receive chars
int com2_baud = 0;													// determines the baud rate
char *com2_interrupt, *com2_TX_interrupt;												// pointer to the interrupt routine
int com2_ilevel;													// number nbr of chars in the buffer for an interrupt
int com2_TX_complete = false;
unsigned char *com2Rx_buf;											// pointer to the buffer for received characters
volatile int com2Rx_head, com2Rx_tail;								// head and tail of the ring buffer for com2 Rx
unsigned char *com2Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com2Tx_head, com2Tx_tail;								// head and tail of the ring buffer for com2 Tx
volatile int com2complete=1;
uint16_t Rx2Buffer;
char com2_mode;                                                     // keeps track of the settings for com4
unsigned char com2_bit9 = 0;                                        // used to track the 9th bit


// variables for com3
int com3 = 0;														// true if COM2 is enabled
int com3_buf_size;													// size of the buffer used to receive chars
int com3_baud = 0;													// determines the baud rate
char *com3_interrupt, *com3_TX_interrupt;												// pointer to the interrupt routine
int com3_ilevel;													// number nbr of chars in the buffer for an interrupt
int com3_TX_complete = false;
unsigned char *com3Rx_buf;											// pointer to the buffer for received characters
volatile int com3Rx_head, com3Rx_tail;								// head and tail of the ring buffer for com2 Rx
unsigned char *com3Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com3Tx_head, com3Tx_tail;								// head and tail of the ring buffer for com2 Tx
volatile int com3complete=1;
uint16_t Rx3Buffer;
char com3_mode;                                                     // keeps track of the settings for com4
unsigned char com3_bit9 = 0;                                        // used to track the 9th bit

// variables for com4
int com4 = 0;														// true if COM2 is enabled
int com4_buf_size;													// size of the buffer used to receive chars
int com4_baud = 0;													// determines the baud rate
char *com4_interrupt, *com4_TX_interrupt;												// pointer to the interrupt routine
int com4_ilevel;													// number nbr of chars in the buffer for an interrupt
int com4_TX_complete = false;
unsigned char *com4Rx_buf;											// pointer to the buffer for received characters
volatile int com4Rx_head, com4Rx_tail;								// head and tail of the ring buffer for com2 Rx
unsigned char *com4Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com4Tx_head, com4Tx_tail;								// head and tail of the ring buffer for com2 Tx
volatile int com4complete=1;
uint16_t Rx4Buffer;
char com4_mode;                                                     // keeps track of the settings for com4
unsigned char com4_bit9 = 0;                                        // used to track the 9th bit


int get_baudrate(int comport, int timeout){
    float rates[13]={110,300,500,1200,2800,4800,9600,14400,19200,38400,57600,115200,230400};
    int *t=GetMemory(1024),i,j,k,m;
    float a, b,c;
    int pin=0;
    switch(comport){
        case 1:
            if(com1) error("Already open");
            pin=COM1_RX_PIN;
            break;
        case 2:
            if(com2) error("Already open");
            pin=COM2_RX_PIN;
            break;
       case 3:
          if(com3) error("Already open");
           pin=COM3_RX_PIN;
           break;
        case 4:
           if(com4) error("Already open");
            pin=COM4_RX_PIN;
          break;
    }
    WriteCoreTimer(0); j=0;t[j]=0;
    k=PinRead(pin);
    while((ReadCoreTimer() < 1000000 * timeout * 100) && (j<256)){
        m=PinRead(pin);
        if(m!=k){
            t[j]=ReadCoreTimer();
            j++;
            k=m;
        }
    }
    if(j>=2){
        c=10000000;
        k=0;
        m=10000000;
        for(i=2;i<j;i++){ //the first pulse may be spurious so ignore
            if(t[i]-t[i-1]<m)m=t[i]-t[i-1];
        }
        a=1000000.0/((float)m / (float)(100));
        for(i=0;i<13;i++){
            b=a/rates[i];
            if(b<1)b=rates[i]/a;
            if(b<c){c=b;k=i;}
        }
        a=rates[k];
    } else a=0;
    FreeMemory(t);
    return a;
}
void fun_baudrate(void){
    getargs(&ep, 3,",");
    int i= getint(argv[0] ,1 ,4), timeout;
    if(argc==3)timeout=getint(argv[2],1,10);
    else timeout=1;
    iret = get_baudrate(i,timeout);
    targ = T_INT;
}
/***************************************************************************************************
Initialise the serial function including the timer and interrupts.
****************************************************************************************************/

void setupuart(UART_HandleTypeDef * huartx, USART_TypeDef  * USART_ID, int de, int inv,int s2,int parity, int b7, int baud){
	  huartx->Instance = USART_ID;
	  huartx->Init.BaudRate = baud;
	  huartx->Init.StopBits = (s2 ? UART_STOPBITS_2 :UART_STOPBITS_1);
	  if(parity){
		  huartx->Init.WordLength = (b7 ? UART_WORDLENGTH_8B :UART_WORDLENGTH_9B);
		  parity--;
		  huartx->Init.Parity = (parity ? UART_PARITY_ODD : UART_PARITY_EVEN ) ;
	  } else {
		  huartx->Init.WordLength = UART_WORDLENGTH_8B;
		  huartx->Init.Parity = UART_PARITY_NONE;
	  }
	  huartx->Init.Mode = UART_MODE_TX_RX;
	  huartx->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huartx->Init.OverSampling = UART_OVERSAMPLING_16;
	  if (HAL_UART_Init(huartx) != HAL_OK)
  	  	  {
	  	  	  error("Uart Init");
  	  	  }
}
/***************************************************************************************************
Initialise the serial function including the timer and interrupts.
****************************************************************************************************/
void SerialOpen(char *spec) {
	int baud, i, inv, oc, s2, de, parity, b7, bufsize, ilevel;
	char *interrupt, *TXinterrupt;
	GPIO_InitTypeDef GPIO_InitStruct;

	getargs(&spec, 21, ":,");										// this is a macro and must be the first executable stmt
	if(argc != 2 && (argc & 0x01) == 0) error("COM specification");

    b7 = de = parity = inv = oc = s2 = false;
    for(i = 0; i < 6; i++) {
    	if(str_equal(argv[argc - 1], "OC")) { oc = true; argc -= 2; }	// get the open collector option
    	if(str_equal(argv[argc - 1], "EVEN")) {
    		if(parity)error("Syntax");
    		else {parity = 1; argc -= 2; }	// set even parity
    	}
    	if(str_equal(argv[argc - 1], "ODD")) {
    		if(parity)error("Syntax");
    		else {parity = 2; argc -= 2; }	// set even parity
    	}
    	if(str_equal(argv[argc - 1], "S2")) { s2 = true; argc -= 2; }	// get the two stop bit option
    	if(str_equal(argv[argc - 1], "7BIT")) { b7 = true; argc -= 2; }	// set the 7 bit byte option
    }

	if(argc < 1 || argc > 13) error("COM specification");

	if(argc >= 3 && *argv[2]) {
		baud = getinteger(argv[2]);									// get the baud rate as a number
		if(baud<2400)error("2400 baud is minimum supported");
	} else
		baud = COM_DEFAULT_BAUD_RATE;

	if(argc >= 5 && *argv[4])
		bufsize = getinteger(argv[4]);								// get the buffer size as a number
	else
		bufsize = COM_DEFAULT_BUF_SIZE;

	if(argc >= 7) {
    	InterruptUsed = true;
		interrupt = GetIntAddress(argv[6]);							// get the interrupt location
	} else
		interrupt = NULL;

	if(argc >= 9) {
		ilevel = getinteger(argv[8]);								// get the buffer level for interrupt as a number
		if(ilevel < 1 || ilevel > bufsize) error("COM specification");
	} else
		ilevel = 1;

	if(argc >= 11) {
    	InterruptUsed = true;
		TXinterrupt = GetIntAddress(argv[10]);							// get the interrupt location
	} else
		TXinterrupt = NULL;


	if(spec[3] == '1') {
	///////////////////////////////// this is COM1 ////////////////////////////////////

		if(com1) error("Already open");
        CheckPin(COM1_RX_PIN, CP_CHECKALL);
        CheckPin(COM1_TX_PIN, CP_CHECKALL);

 		com1_buf_size = bufsize;									// extracted from the comspec above
		com1_interrupt = interrupt;
		com1_ilevel	= ilevel;
		com1_TX_interrupt = TXinterrupt;
		com1_TX_complete = false;

		// setup for receive
		com1Rx_buf = GetMemory(com1_buf_size);						// setup the buffer
		com1Rx_head = com1Rx_tail = 0;
		ExtCfg(COM1_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com1Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com1Tx_head = com1Tx_tail = 0;
		ExtCfg(COM1_TX_PIN, EXT_COM_RESERVED, 0);
        if(de) error("COM specification");
      if (HAS_64PINS && Feather){
          /**USART3 GPIO Configuration
          PB10     ------> USART3_TX
          PB11     ------> USART3_RX
          */
          setupuart(&huart3, USART3, de, inv, s2, parity, b7, baud);
          if(Option.SerialPullup){
              GPIO_InitStruct.Pin = GPIO_PIN_11;
              GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
              GPIO_InitStruct.Pull = GPIO_PULLUP;
              GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
              GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
              HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
          }
          if(oc){
          	GPIO_InitStruct.Pin = GPIO_PIN_10;
          	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
          	GPIO_InitStruct.Pull = GPIO_NOPULL;
          	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
          	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
          	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
          }
          HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
          HAL_NVIC_EnableIRQ(USART3_IRQn);
          huart3.Instance->CR1 |= USART_CR1_RXNEIE;


      }else{

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        setupuart(&huart1, USART1, de, inv, s2, parity, b7, baud);
        if(Option.SerialPullup){
            GPIO_InitStruct.Pin = RXD1_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        if(oc){
        	GPIO_InitStruct.Pin = TXD1_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        huart1.Instance->CR1 |= USART_CR1_RXNEIE;
      }
        com1 = true;
	}
    if (spec[3] == '2') {
	///////////////////////////////// this is COM2 ////////////////////////////////////

		if(com2) error("Already open");
        CheckPin(COM2_RX_PIN, CP_CHECKALL);
        CheckPin(COM2_TX_PIN, CP_CHECKALL);

 		com2_buf_size = bufsize;									// extracted from the comspec above
		com2_interrupt = interrupt;
		com2_ilevel	= ilevel;
		com2_TX_interrupt = TXinterrupt;
		com2_TX_complete = false;

		// setup for receive
		com2Rx_buf = GetMemory(com2_buf_size);						// setup the buffer
		com2Rx_head = com2Rx_tail = 0;
		ExtCfg(COM2_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com2Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com2Tx_head = com2Tx_tail = 0;
		ExtCfg(COM2_TX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use
		com2_bit9 = com2_mode = 0;
        if(parity)  com2_mode |= COM2_9B;
        setupuart(&huart6, USART6, de, inv, s2, parity, b7, baud);
        if(Option.SerialPullup){
            /**USART6 GPIO Configuration
            PC6     ------> USART6_TX
            PC7     ------> USART6_RX
            */
            GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        }
        if(oc){
        	GPIO_InitStruct.Pin = GPIO_PIN_6;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
        huart6.Instance->CR1 |= USART_CR1_RXNEIE;
        com2 = true;
	}
    if (spec[3] == '3') {
	///////////////////////////////// this is COM3 ////////////////////////////////////

		if(com3) error("Already open");
        CheckPin(COM3_RX_PIN, CP_CHECKALL);
        CheckPin(COM3_TX_PIN, CP_CHECKALL);

 		com3_buf_size = bufsize;									// extracted from the comspec above
		com3_interrupt = interrupt;
		com3_ilevel	= ilevel;
		com3_TX_interrupt = TXinterrupt;
		com3_TX_complete = false;

		// setup for receive
		com3Rx_buf = GetMemory(com3_buf_size);						// setup the buffer
		com3Rx_head = com3Rx_tail = 0;
		ExtCfg(COM3_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com3Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com3Tx_head = com3Tx_tail = 0;
		ExtCfg(COM3_TX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use
		com3_bit9 = com3_mode = 0;
        if(parity)  com3_mode |= COM2_9B;
        setupuart(&huart4, UART4, de, inv, s2, parity, b7, baud);
        if(Option.SerialPullup){
            /**UART4 GPIO Configuration
            PA0-WKUP     ------> UART4_TX
            PA1     ------> UART4_RX
            */
            GPIO_InitStruct.Pin = GPIO_PIN_1;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        if(oc){
        	GPIO_InitStruct.Pin = GPIO_PIN_0;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
        huart4.Instance->CR1 |= USART_CR1_RXNEIE;
        com3 = true;
	}

    if (spec[3] == '4') {
	///////////////////////////////// this is COM4 ////////////////////////////////////

		if(com4) error("Already open");
        CheckPin(COM4_RX_PIN, CP_CHECKALL);
        CheckPin(COM4_TX_PIN, CP_CHECKALL);

 		com4_buf_size = bufsize;									// extracted from the comspec above
		com4_interrupt = interrupt;
		com4_ilevel	= ilevel;
		com4_TX_interrupt = TXinterrupt;
		com4_TX_complete = false;

		// setup for receive
		com4Rx_buf = GetMemory(com4_buf_size);						// setup the buffer
		com4Rx_head = com4Rx_tail = 0;
		ExtCfg(COM4_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com4Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com4Tx_head = com4Tx_tail = 0;
		ExtCfg(COM4_TX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use
		com4_bit9 = com4_mode = 0;
        if(parity)  com4_mode |= COM2_9B;
        setupuart(&huart2, USART2, de, inv, s2, parity, b7, baud);
        if(Option.SerialPullup){
            /**UART2 GPIO Configuration
            PA2     ------> UART2_TX
            PA3     ------> UART2_RX
            */
            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        if(oc){
        	GPIO_InitStruct.Pin = GPIO_PIN_2;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        huart2.Instance->CR1 |= USART_CR1_RXNEIE;
        com4 = true;
	}

}




/***************************************************************************************************
Close a serial port.
****************************************************************************************************/
void SerialClose(int comnbr) {

	if(comnbr == 1 && com1) {
		if (HAS_64PINS){
		  HAL_UART_DeInit(&huart3);
		}else{
		  HAL_UART_DeInit(&huart1);
		}
		com1 = false;
		com1_interrupt = NULL;
        PinSetBit(COM1_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM1_RX_PIN, CNPDCLR);
		ExtCfg(COM1_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM1_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemory(com1Rx_buf);
		FreeMemory(com1Tx_buf);
	}

	else if(comnbr == 2 && com2) {
		HAL_UART_DeInit(&huart6);
		com2 = false;
		com2_interrupt = NULL;
        PinSetBit(COM2_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM2_RX_PIN, CNPDCLR);
		ExtCfg(COM2_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM2_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemory(com2Rx_buf);
		FreeMemory(com2Tx_buf);
	}
	else if(comnbr == 3 && com3) {
		HAL_UART_DeInit(&huart4);
		com3 = false;
		com3_interrupt = NULL;
        PinSetBit(COM3_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM3_RX_PIN, CNPDCLR);
		ExtCfg(COM3_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM3_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemory(com3Rx_buf);
		FreeMemory(com3Tx_buf);
	}
	else if(comnbr == 4 && com4) {
		HAL_UART_DeInit(&huart2);
		com4 = false;
		com4_interrupt = NULL;
        PinSetBit(COM4_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM4_RX_PIN, CNPDCLR);
		ExtCfg(COM4_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM4_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemory(com4Rx_buf);
		FreeMemory(com4Tx_buf);
	}

}



/***************************************************************************************************
Add a character to the serial output buffer.
****************************************************************************************************/
unsigned char SerialPutchar(int comnbr, unsigned char c) {
	if(comnbr == 1) {
		if (HAS_64PINS){
        int empty=(huart3.Instance->SR & USART_SR_TC) | !(huart3.Instance->CR1 & USART_CR1_TCIE) ;
		while(com1Tx_tail == ((com1Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com1Tx_buf[com1Tx_head] = c;							// add the char
		com1Tx_head = (com1Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart3.Instance->CR1 |= USART_CR1_TCIE;
		}
		}else{
        int empty=(huart1.Instance->SR & USART_SR_TC) | !(huart1.Instance->CR1 & USART_CR1_TCIE) ;
		while(com1Tx_tail == ((com1Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com1Tx_buf[com1Tx_head] = c;							// add the char
		com1Tx_head = (com1Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart1.Instance->CR1 |= USART_CR1_TCIE;
		}
		}

	}
	else if(comnbr == 2) {
        int empty=(huart6.Instance->SR & USART_SR_TC) | !(huart6.Instance->CR1 & USART_CR1_TCIE) ;
		while(com2Tx_tail == ((com2Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com2Tx_buf[com2Tx_head] = c;							// add the char
		com2Tx_head = (com2Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart6.Instance->CR1 |= USART_CR1_TCIE;
		}
	}
	else if(comnbr == 3) {
        int empty=(huart4.Instance->SR & USART_SR_TC) | !(huart4.Instance->CR1 & USART_CR1_TCIE) ;
		while(com3Tx_tail == ((com3Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com3Tx_buf[com3Tx_head] = c;							// add the char
		com3Tx_head = (com3Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart4.Instance->CR1 |= USART_CR1_TCIE;
		}
	}

	else if(comnbr == 4) {
        int empty=(huart2.Instance->SR & USART_SR_TC) | !(huart2.Instance->CR1 & USART_CR1_TCIE) ;
		while(com4Tx_tail == ((com4Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com4Tx_buf[com4Tx_head] = c;							// add the char
		com4Tx_head = (com4Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart2.Instance->CR1 |= USART_CR1_TCIE;
		}
	}

	return c;
}



/***************************************************************************************************
Get the status the serial receive buffer.
Returns the number of characters waiting in the buffer
****************************************************************************************************/
int SerialRxStatus(int comnbr) {
	int i = 0;
	if(comnbr == 1) {
		if (HAS_64PINS){
	    huart3.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com1Rx_head - com1Rx_tail;
	    huart3.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com1_buf_size;
		}else{
		huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com1Rx_head - com1Rx_tail;
	    huart1.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com1_buf_size;
		}
	}
	else if(comnbr == 2) {
	    huart6.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com2Rx_head - com2Rx_tail;
	    huart6.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com2_buf_size;
	}
	else if(comnbr == 3) {
	    huart4.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com3Rx_head - com3Rx_tail;
	    huart4.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com3_buf_size;
	}
	else if(comnbr == 4) {
	    huart2.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com4Rx_head - com4Rx_tail;
	    huart2.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com4_buf_size;
	}

	return i;
}


/***************************************************************************************************
Get the status the serial transmit buffer.
Returns the number of characters waiting in the buffer
****************************************************************************************************/
int SerialTxStatus(int comnbr) {
	int i = 0;
	if(comnbr == 1) {
		i = com1Tx_head - com1Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	else if(comnbr == 2) {
		i = com2Tx_head - com2Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	else if(comnbr == 3) {
		i = com3Tx_head - com3Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	else if(comnbr == 4) {
		i = com4Tx_head - com4Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	return i;
}



/***************************************************************************************************
Get a character from the serial receive buffer.
Note that this is returned as an integer and -1 means that there are no characters available
****************************************************************************************************/
int SerialGetchar(int comnbr) {
	int c;
    c = -1;                                                         // -1 is no data
	if(comnbr == 1) {
	  if(HAS_64PINS){
	    huart3.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com1Rx_head != com1Rx_tail) {                            // if the queue has something in it
			c = com1Rx_buf[com1Rx_tail];                            // get the char
 			com1Rx_tail = (com1Rx_tail + 1) % com1_buf_size;        // and remove from the buffer
		}
		huart3.Instance->CR1 |= USART_CR1_RXNEIE;
	  }else{
	    huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com1Rx_head != com1Rx_tail) {                            // if the queue has something in it
			c = com1Rx_buf[com1Rx_tail];                            // get the char
 			com1Rx_tail = (com1Rx_tail + 1) % com1_buf_size;        // and remove from the buffer
		}
	    huart1.Instance->CR1 |= USART_CR1_RXNEIE;
	  }
	}
	else if(comnbr == 2) {

	    huart6.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com2Rx_head != com2Rx_tail) {                            // if the queue has something in it
			c = com2Rx_buf[com2Rx_tail];                            // get the char
 			com2Rx_tail = (com2Rx_tail + 1) % com2_buf_size;        // and remove from the buffer
		}
	    huart6.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	else if(comnbr == 3) {

	    huart4.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com3Rx_head != com3Rx_tail) {                            // if the queue has something in it
			c = com3Rx_buf[com3Rx_tail];                            // get the char
 			com3Rx_tail = (com3Rx_tail + 1) % com3_buf_size;        // and remove from the buffer
		}
	    huart4.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	else if(comnbr == 4) {

	    huart2.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com4Rx_head != com4Rx_tail) {                            // if the queue has something in it
			c = com4Rx_buf[com4Rx_tail];                            // get the char
 			com4Rx_tail = (com4Rx_tail + 1) % com4_buf_size;        // and remove from the buffer
		}
	    huart2.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	return c;
}

