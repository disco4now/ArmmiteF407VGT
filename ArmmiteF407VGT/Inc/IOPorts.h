/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

IOPorts.h

Include file that defines the IOPins for the PIC32 chip in MMBasic.

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

#ifndef IOPORTS_H
#define IOPORTS_H

// these are the valid peek/poke memory ranges for the STM32F407VET6
//#define RAM(a)  ((a >= RAMBASE && a < RAMEND) || (a >= 0xd0000000 && a < 0xd0800000) || (a >= 0x24000000 && a < 0x24080000) || (a >= 0x38000000 && a<  0x38010000) || (a >= 0x38800000 && a<  0x38801000))
// Register access added (a >= 0x40000000 && a < 0x50060fff)
// VARTBLRAMBASE and  VARTBLRAMEND  defines where vartbl is located in the CCRAM
#define RAM(a)  ((a >= VARTBLRAMBASE && a < VARTBLRAMEND) ||(a >= RAMBASE && a < RAMEND) || (a >= 0x40000000 && a < 0x50060fff) || (a >= 0xd0000000 && a < 0xd0800000) || (a >= 0x24000000 && a < 0x24080000) || (a >= 0x38000000 && a<  0x38010000) || (a >= 0x38800000 && a<  0x38801000))
#define ROM(a)  (a >= 0x08000000 && a < 0x08200000)
#define PEEKRANGE(a) (RAM(a) || ROM(a))
#define POKERANGE(a) (RAM(a))

// General defines
#define P_INPUT				1						// for setting the TRIS on I/O bits
#define P_OUTPUT			0
#define P_ON				1
#define P_OFF				0
// Structure that defines the SFR, bit number and mode for each I/O pin
struct s_PinDef {
	GPIO_TypeDef *sfr;
    unsigned int bitnbr;
    unsigned char mode;
    ADC_TypeDef *  ADCn;
    uint32_t ADCchannel;
};
typedef struct s_PinDef PinDefAlias;

// Defines for the various modes that an I/O pin can be set to
#define PUNUSED       1
#define ANALOG_IN    2
#define DIGITAL_IN   4
#define COUNTING     8
#define INTERRUPT    16
#define DIGITAL_OUT  32
#define OC_OUT       64
#define DO_NOT_RESET 128
//#define HAS_64PINS 0
#define NBR_PINS_144CHIP    144					    // number of pins for external i/o on a 144 pin chip
#define NBR_PINS_100CHIP    100					    // number of pins for external i/o on a 100 pin chip
#define NBR_PINS_64CHIP    64					    // number of pins for external i/o on a 64 pin chip

#define NBR_PINS_MAXCHIP    144		//144	        // max number of pins for supported packages on chip
#define MAX_ANALOGUE_PIN_PACKAGE   47 // 47		    // 47 max analogue pin no for supported packages on chip i.e 144 pins



#define package (*(volatile unsigned int *)(PACKAGE_BASE) & 0b11111)
//#define package  0b01010
#define flashsize *(volatile unsigned int *)(FLASHSIZE_BASE)
#define chipID (DBGMCU->IDCODE & 0x00000FFF)
//#define HAS_32PINS          (package==0x8)
//#define HAS_48PINS          (package==0xB)
//#define HAS_64PINS          (package==0x9 || package==0)
//#define HAS_64PINS            (package==0xA || package==0x11)
//#define HAS_64PINS          1
//#define HAS_100PINS          (package==0x10)
#define HAS_100PINS          (package==0x1C)
#define HAS_144PINS          (package==0x06)  //06
// default to 64 pins if package is unknown
#define HAS_64PINS           (package==0xA || package==0x11 || (!HAS_144PINS && !HAS_100PINS ))

#define NBRPINS             (HAS_144PINS ? 144 :(HAS_100PINS ? 100 : 64))

#define NBRINTPINS          10
#if defined(DEFINE_PINDEF_TABLE)
const struct s_PinDef PinDef144[NBR_PINS_144CHIP + 1]={
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 0
		{ GPIOE,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 1 IR
		{ GPIOE,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 2 Count 3/KEY1
		{ GPIOE,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 3 Count 4/KEY0
		{ GPIOE,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 4 PWM-3A
		{ GPIOE,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 5 PWM-3B
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 6 VBAT
		{ GPIOC,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						// pin 7
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 8 OSC32_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 9 OSC32_OUT

		{ GPIOF,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 10
		{ GPIOF,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 11
		{ GPIOF,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 12
		{ GPIOF,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 13
		{ GPIOF,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 14
		{ GPIOF,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 15

	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 16 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 17 VDD

		{ GPIOF,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 18
		{ GPIOF,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 19
		{ GPIOF,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						    // pin 20
		{ GPIOF,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 21
		{ GPIOF,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 22

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 23 OSC8_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 24 OSC8_OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 25 NRST
		{ GPIOC,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_10},    // pin 26
		{ GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_11},    // pin 27
		{ GPIOC,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_12},    // pin 28
		{ GPIOC,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_13},    // pin 29
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 30 VDD
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 31 VSSA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 32 VREF+
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 33 VDDA
		{ GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_0},    	// pin 34 COM3-TX/COUNT-0/WK_UP
	    { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_1},     // pin 35 COM3-RX
	    { GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_2},     // pin 36 COM4-TX
	    { GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_3},     // pin 37 COM4-RX
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 38 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 39 VDD
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 40 DAC-1
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 41 DAC-2
	    { GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_6},     // pin 42 PWM-1A +LED-D2
	    { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_7},     // pin 43 PWM-1B +LED-D3
		{ GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_14},   // pin 44
		{ GPIOC,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_15},   // pin 45 T_IRQ
	    { GPIOB,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_8},     // pin 46 PWM-1C/F_CS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 47 LCD_BL
		{ GPIOB,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						    // pin 48    BOOT1
		{ GPIOF,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 49
		{ GPIOF,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 50
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 51 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 52 VDD
		{ GPIOF,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 53
		{ GPIOF,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 54
		{ GPIOF,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 55
		{ GPIOG,  GPIO_PIN_0 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 56
		{ GPIOG,  GPIO_PIN_1 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 57

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 58 FSMC_D4
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 59 FSMC_D5
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 60 FSMC_D6

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 61 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 62 VDD

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 63 FSMC_D7
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 64 FSMC_D8
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 65 FSMC_D9
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 66 FSMC_D10
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 67 FSMC_D11
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 68 FSMC_D12
	    { GPIOB,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 69 I2C2-SCL
	    { GPIOB,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 70 I2C2-SDA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 71 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 72 VDD
	    { GPIOB,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 73 T_CS
	    { GPIOB,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 74 SPI2-CLK
	    { GPIOB,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 75 SPI2-IN
	    { GPIOB,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 76 SPI2-OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 77 FSMC_D13
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 78 FSMC_D14
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 79 FSMC_D15
		{ GPIOD,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 80 Drive_VBUS_FS
		{ GPIOD,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 81 PWM-2A
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 82 FSMC_A18

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 83 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 84 VDD

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 85 FSMC_D0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 86 FSMC_D1

		{ GPIOG,  GPIO_PIN_2 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 87
		{ GPIOG,  GPIO_PIN_3 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 88
		{ GPIOG,  GPIO_PIN_4 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 89
		{ GPIOG,  GPIO_PIN_5 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 90
		{ GPIOG,  GPIO_PIN_6 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 91
		{ GPIOG,  GPIO_PIN_7 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 92
		{ GPIOG,  GPIO_PIN_8 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 93
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 94 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 95 VDD

		{ GPIOC,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 96 COM2-TX
		{ GPIOC,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 97 COM2-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 98 SDIO_D0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 99 SDIO_D1
	    { GPIOA,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 100
	    { GPIOA,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 101 COM1-TX
	    { GPIOA,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 102 COM1-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 103 USB-DM
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 104 USB-DP
		{ GPIOA,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 105 SWDIO
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 106 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 107 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 108 VDD
		//
	    { GPIOA,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 109 SWCLK
	    { GPIOA,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 110 KBD_CLK
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 111 SDIO_D2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 112 SDIO_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 113 SDIO_CK
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 114 FSMC_D2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 115 FSMC_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 116 SDIO_CMD
	    { GPIOD,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 117 KBD_DATA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 118 FSMC_NOE
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 119 FSMC_NWE

		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 120 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 121 VDD

	    { GPIOD,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 122
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 123 FSMC_NE1

		{ GPIOG,  GPIO_PIN_9 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					    	// pin 124
		{ GPIOG,  GPIO_PIN_10 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					   	// pin 125
		{ GPIOG,  GPIO_PIN_11 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					   	// pin 126
		{ GPIOG,  GPIO_PIN_12 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					   	// pin 127
		{ GPIOG,  GPIO_PIN_13 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    					   	// pin 128
		{ GPIOG,  GPIO_PIN_14 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    				    	// pin 129
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 130 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 131 VDD
		{ GPIOG,  GPIO_PIN_15 , DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    				    	// pin 132

	    { GPIOB,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 133 SPI_CLK
	    { GPIOB,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 134 SPI_IN
	    { GPIOB,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 135 SPI-OUT
	    { GPIOB,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 136 I2C-SCL/NRF_CE
	    { GPIOB,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 137 I2C-SDA/NRF_CS
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 138 BOOT0
		{ GPIOB,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 139 PWM-2B
		{ GPIOB,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 140 PWM-2C
		{ GPIOE,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 141
		{ GPIOE,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 142 COUNT 1
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 143 PDR_ON
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 144 VDD
};


const struct s_PinDef PinDef100[NBR_PINS_100CHIP + 1]={
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 0
		{ GPIOE,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 1 IR
		{ GPIOE,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 2 Count 3/KEY1
		{ GPIOE,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 3 Count 4/KEY0
		{ GPIOE,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 4 PWM-3A
		{ GPIOE,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 5 PWM-3B
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 6 VBAT
		{ GPIOC,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						// pin 7
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 8 OSC32_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 9 OSC32_OUT
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 10 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 11 VDD
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 12 OSC8_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 13 OSC8_OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 14 NRST
		{ GPIOC,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_10},    // pin 15
		{ GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_11},    // pin 16
		{ GPIOC,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_12},    // pin 17
		{ GPIOC,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_13},    // pin 18
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 19 VDD
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 20 VSSA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 21 VREF+
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 22 VDDA
		{ GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_0},    	// pin 23 COM3-TX/COUNT-0/WK_UP
	    { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_1},     // pin 24 COM3-RX
	    { GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_2},     // pin 25 COM4-TX
		//
	    { GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_3},     // pin 26 COM4-RX
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 27 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 28 VDD
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 29 DAC-1
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 30 DAC-2
	    { GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_6},     // pin 31 PWM-1A +LED-D2
	    { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_7},     // pin 32 PWM-1B +LED-D3
		{ GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_14},   // pin 33
		{ GPIOC,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_15},   // pin 34 T_IRQ
	    { GPIOB,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_8},     // pin 35 PWM-1C/F_CS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 36 LCD_BL
		{ GPIOB,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						    // pin 37
		{ GPIOE,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                           	// pin 38 FSMC_D4
		{ GPIOE,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 39 FSMC_D5
		{ GPIOE,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 40 FSMC_D6
		{ GPIOE,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 41 FSMC_D7
		{ GPIOE,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 42 FSMC_D8
		{ GPIOE,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 43 FSMC_D9
		{ GPIOE,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                           	// pin 44 FSMC_D10
		{ GPIOE,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 45 FSMC_D11
		{ GPIOE,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 46 FSMC_D12
	    { GPIOB,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 47 I2C2-SCL
	    { GPIOB,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 48 I2C2-SDA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 49 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 50 VDD
		//
	    { GPIOB,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 51 T_CS
	    { GPIOB,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 52 SPI2-CLK
	    { GPIOB,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 53 SPI2-IN
	    { GPIOB,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 54 SPI2-OUT
		{ GPIOD,  GPIO_PIN_8,   DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 55 FSMC_D13
		{ GPIOD,  GPIO_PIN_9,   DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 56 FSMC_D14
		{ GPIOD,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 57 FSMC_D15
		{ GPIOD,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 58 Drive_VBUS_FS
		{ GPIOD,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 59 PWM-2A
		{ GPIOD,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                         	// pin 60 FSMC_A18
		{ GPIOD,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 61 FSMC_D0
		{ GPIOD,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 62 FSMC_D1
		{ GPIOC,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 63 COM2-TX
		{ GPIOC,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 64 COM2-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 65 SDIO_D0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 66 SDIO_D1
	    { GPIOA,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 67
	    { GPIOA,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 68 COM1-TX
	    { GPIOA,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 69 COM1-RX
//		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 68 CONSOLE-TX
//		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 69 CONSOLE-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 70 USB-DM
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 71 USB-DP
		{ GPIOA,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 72 SWDIO
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 73 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 74 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 75 VDD
		//
	    { GPIOA,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 76 SWCLK
	    { GPIOA,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 77 KBD_CLK
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 78 SDIO_D2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 79 SDIO_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 80 SDIO_CK
		{ GPIOD,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                        	// pin 81 FSMC_D2    PD0   ------> FSMC_D2
		{ GPIOD,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                           	// pin 82 FSMC_D3    PD1   ------> FSMC_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 83 SDIO_CMD
	    { GPIOD,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 84 KBD_DATA
		{ GPIOD,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                           	// pin 85 FSMC_NOE
		{ GPIOD,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                         	// pin 86 FSMC_NWE
	    { GPIOD,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 87
		{ GPIOD,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},                          	// pin 88 FSMC_NE1
	    { GPIOB,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 89 SPI_CLK
	    { GPIOB,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 90 SPI_IN
	    { GPIOB,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 91 SPI-OUT
	    { GPIOB,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 92 I2C-SCL/NRF_CE
	    { GPIOB,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 93 I2C-SDA/NRF_CS
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 94 BOOT0
		{ GPIOB,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 95 PWM-2B
		{ GPIOB,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 96 PWM-2C
		{ GPIOE,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 97
		{ GPIOE,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT	,  NULL, 0},    						// pin 98 COUNT 1
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 99 VSS
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 100 VDD
};

const struct s_PinDef PinDef64[NBR_PINS_64CHIP + 1]={
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 1 VBAT
		{ GPIOC,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    						// pin 2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 3 OSC32_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 4 OSC32_OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 5 OSC8_IN
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 6 OSC8_OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 7 NRST
		{ GPIOC,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_10},    // pin 8
		{ GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_11},    // pin 9
		{ GPIOC,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_12},    // pin 10
		{ GPIOC,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_13},    // pin 11
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 12 VSSA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 13 VDDA
		{ GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_0},    	// pin 14 COM3-TX/COUNT-0/WK_UP
	    { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_1},     // pin 15 COM3-RX
	    { GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_2},     // pin 16 COM4-TX
		{ GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_3},     // pin 17 COM4-RX
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 18 VSS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 19 VDD
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 20 DAC-1
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 21 DAC-2
	    { GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_6},     // pin 22 PWM-1A +LED-D2
	    { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_7},     // pin 23 PWM-1B +LED-D3
		{ GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_14},   // pin 24
		{ GPIOC,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,  ADC2, ADC_CHANNEL_15},   // pin 25 T_IRQ
	    { GPIOB,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_8},     // pin 26 PWM-1C/F_CS
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 27 LCD_BL
		{ GPIOB,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						    // pin 28
		{ GPIOB,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 29 I2C2-SCL
	    { GPIOB,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 30 I2C2-SDA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 31 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 32 VDD
		{ GPIOB,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 33 T_CS
	    { GPIOB,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 34 SPI2-CLK
	    { GPIOB,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 35 SPI2-IN
	    { GPIOB,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 36 SPI2-OUT
		{ GPIOC,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 37 COM2-TX
		{ GPIOC,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT,  NULL, 0},    							// pin 38 COM2-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 39 SDIO_D0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 40 SDIO_D1
		{ GPIOA,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 41
	    { GPIOA,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 42 COM1-TX
	    { GPIOA,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 43 COM1-RX
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 44 USB-DM
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 45 USB-DP
		{ GPIOA,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 46 SWDIO
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 47 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 48 VDD
		{ GPIOA,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 49 SWCLK
	    { GPIOA,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 50 KBD_CLK
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 51 SDIO_D2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 52 SDIO_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 53 SDIO_CK
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 54 SDIO_CMD
	    { GPIOB,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 55 SPI_CLK
	    { GPIOB,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 56 SPI_IN
	    { GPIOB,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 57 SPI-OUT
	    { GPIOB,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 58 I2C-SCL/NRF_CE
	    { GPIOB,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 59 I2C-SDA/NRF_CS
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 60 BOOT0
		{ GPIOB,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 61 PWM-2B
		{ GPIOB,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},      						// pin 62 PWM-2C
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 63 VSS
	    { NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 64 VDD
};
#else
	//const extern struct s_PinDef PinDef64[];
	//const extern struct s_PinDef PinDef100[];
	//const extern struct s_PinDef PinDef144[];
#endif      // DEFINE_PINDEF_TABLE
    extern struct s_PinDef *PinDef;

#ifdef RGT

#define INT1PIN              	41  //PA1,PE1
#define INT2PIN              	41 	//PE3
#define INT3PIN             	41 	//PE4
#define INT4PIN              	41 	//PA8
#define IRPIN                  	41	//PE2


   // I2C pin numbers
#define P_I2C_SCL           	58 	//PB6
#define P_I2C_SDA           	59 	//PB7

#define P_I2C2_SCL           	29 	//PB10
#define P_I2C2_SDA           	30 	//PB11

// COMx: port pin numbers
#define COM1_TX_PIN         	42 	//PA9 USART1
#define COM1_RX_PIN         	43 	//PA10

#define COM2_TX_PIN         	37 	//PC6 USART6
#define COM2_RX_PIN         	38 	//PC7

#define COM3_TX_PIN         	14 	//PA0 UART4
#define COM3_RX_PIN         	15	//PA1

#define COM4_RX_PIN				16 	//PA2 USART2
#define COM4_TX_PIN				17 	//PA3

// SPI pin numbers
#define SPI_CLK_PIN         	55 	//PB3
#define SPI_INP_PIN         	56 	//PB4
#define SPI_OUT_PIN         	57 	//PB5

// SPI2 pin numbers
#define SPI2_CLK_PIN        	34 	//PB13
#define SPI2_INP_PIN        	35 	//PB14
#define SPI2_OUT_PIN        	36 	//PB15

// DAC pin numbers
#define DAC_1_PIN           	20 	//PA4
#define DAC_2_PIN           	21 	//PA5

// PWM pin numbers
#define PWM_CH1_PIN         	22 	//PA6  - PWM1A/TIM3_CH1
#define PWM_CH2_PIN         	23 	//PA7  - PWM1B/TIM3_CH2
#define PWM_CH3_PIN         	26 	//PB0  - PWM1C/TIM3_CH3
// #define PWM_CH5_PIN         	59 	//PD12 - PWM2A/TIM4_CH1
#define PWM_CH6_PIN         	61 	//PB8  - PWM2A/TIM4_CH3
#define PWM_CH7_PIN         	62 	//PB9  - PWM2A/TIM4_CH4
// #define PWM_CH8_PIN         	4	//PE5  - PWM3A/TIM9_CH1
//#define PWM_CH9_PIN         	5	//PE6  - PWM3A/TIM9_CH2

#endif

#ifdef ZGT

#define INT1PIN              	142  //PA1,PE1
#define INT2PIN              	2  	//PE3
#define INT3PIN             	3  	//PE4
#define INT4PIN              	100	//PA8
#define IRPIN                  	1	//PE2



// I2C pin numbers
#define P_I2C_SCL           	136 	//PB6
#define P_I2C_SDA           	137 	//PB7

#define P_I2C2_SCL           	69 	//PB10
#define P_I2C2_SDA           	70 	//PB11

// COMx: port pin numbers
#define COM1_TX_PIN         	101 	//PA9 USART1
#define COM1_RX_PIN         	102 	//PA10

#define COM2_TX_PIN         	96 	//PC6 USART6
#define COM2_RX_PIN         	97 	//PC7

#define COM3_TX_PIN         	34 	//PA0 UART4
#define COM3_RX_PIN         	35 	//PA1

#define COM4_RX_PIN				36 	//PA2 USART2
#define COM4_TX_PIN				37 	//PA3

    // SPI pin numbers
#define SPI_CLK_PIN         	133 	//PB3
#define SPI_INP_PIN         	134 	//PB4
#define SPI_OUT_PIN         	135 	//PB5

// SPI2 pin numbers
#define SPI2_CLK_PIN        	74 	//PB13
#define SPI2_INP_PIN        	75 	//PB14
#define SPI2_OUT_PIN        	76 	//PB15
//
// DAC pin numbers
#define DAC_1_PIN           	40 	//PA4
#define DAC_2_PIN           	41	//PA5
//
// PWM pin numbers
#define PWM_CH1_PIN         	42 	//PA6  - PWM1A/TIM3_CH1
#define PWM_CH2_PIN         	43 	//PA7  - PWM1B/TIM3_CH2
#define PWM_CH3_PIN         	46 	//PB0  - PWM1C/TIM3_CH3
#define PWM_CH5_PIN         	81 	//PD12 - PWM2A/TIM4_CH1
#define PWM_CH6_PIN         	139 //PB8  - PWM2B/TIM4_CH3
#define PWM_CH7_PIN         	140 //PB9  - PWM2C/TIM4_CH4
#define PWM_CH8_PIN         	4	//PE5  - PWM3A/TIM9_CH1
#define PWM_CH9_PIN         	5	//PE6  - PWM3A/TIM9_CH2

#define KEYBOARD_CLOCK			110 	//PA15
#define KEYBOARD_DATA			117 	//PD3

//#define NRF_CE_PIN            	136	//PB6
//#define NRF_CS_PIN            	137 	//PB7
//#define NRF_IRQ_PIN            	139 	//PB8

#define F_CS	                35   //PB0



#endif



#define INT1PIN            	    (HAS_100PINS ? 98 :(HAS_144PINS ? 142 : 9))        //PE1
#define INT2PIN                 (HAS_100PINS ? 2  :(HAS_144PINS ? 2   : 11))	   	//PE3  PC3
#define INT3PIN                 (HAS_100PINS ? 3  :(HAS_144PINS ? 3   : 24))	  	//PE4  PC4
#define INT4PIN                 (HAS_100PINS ? 67 :(HAS_144PINS ? 100 : 41))		//PA8  PA8
#define IRPIN                   (HAS_100PINS ? 1  :(HAS_144PINS ? 1   : 10))		//PE2  PC2


//CAN Pins F4

#define CAN_1A_RX               (HAS_100PINS ? 95  : (HAS_144PINS ? 139 : 61))      //PB8  PB8  Also  PWM2B PWM2A Feather
#define CAN_1A_TX               (HAS_100PINS ? 96  : (HAS_144PINS ? 160 : 62))      //PB9  PB9  Also  PWM2C PWM2B Feather
#define CAN_2A_RX               (HAS_100PINS ? 81  : 114)                           //PD0  PD1  Also SSD1963 D2 Not Available on 64pin
#define CAN_2A_TX               (HAS_100PINS ? 82  : 115)                           //PD1  PB9  Also SSD1963 D3 Not Available on 64pin

// I2C pin numbers
#define P_I2C_SCL               (HAS_100PINS ? 92 :(HAS_144PINS ? 136 : 58))	//PB6
#define P_I2C_SDA               (HAS_100PINS ? 93 :(HAS_144PINS ? 137 : 59))	//PB7

#define P_I2C2_SCL              (HAS_100PINS ? 47 :(HAS_144PINS ? 69 : 0))	 	//PB10
#define P_I2C2_SDA              (HAS_100PINS ? 48 :(HAS_144PINS ? 70 : 0))	 	//PB11

// COMx: port pin numbers
#define COM1_TX_PIN         	(HAS_100PINS ? 68 :(HAS_144PINS ? 101 : 29)) 	//PA9 USART1 3.3v on feather 42
#define COM1_RX_PIN         	(HAS_100PINS ? 69 :(HAS_144PINS ? 102 : 30)) 	//PA10       3.3v on feather 43

#define COM2_TX_PIN         	(HAS_100PINS ? 63 :(HAS_144PINS ? 96 : 37)) 	//PC6 USART6
#define COM2_RX_PIN         	(HAS_100PINS ? 64 :(HAS_144PINS ? 97 : 38)) 	//PC7

#define COM3_TX_PIN         	(HAS_100PINS ? 23 :(HAS_144PINS ? 34 : 14)) 	//PA0 UART4
#define COM3_RX_PIN         	(HAS_100PINS ? 24 :(HAS_144PINS ? 35 : 15))	    //PA1

#define COM4_TX_PIN				(HAS_100PINS ? 25 :(HAS_144PINS ? 36 : 16)) 	    //PA2 USART2
#define COM4_RX_PIN				(HAS_100PINS ? 26 :(HAS_144PINS ? 37 : 17)) 	//PA3 vcc/2 on feather


    // SPI pin numbers
#define SPI_CLK_PIN         	(HAS_100PINS ? 89 :(HAS_144PINS ? 133 : 55))	 //PB3
#define SPI_INP_PIN         	(HAS_100PINS ? 90 :(HAS_144PINS ? 134 : 56)) 	//PB4
#define SPI_OUT_PIN         	(HAS_100PINS ? 91 :(HAS_144PINS ? 135 : 57)) 	//PB5

// SPI2 pin numbers
#define SPI2_CLK_PIN        	(HAS_100PINS ? 52 :(HAS_144PINS ? 74 : 34)) 	//PB13
#define SPI2_INP_PIN        	(HAS_100PINS ? 53 :(HAS_144PINS ? 75 : 35)) 	//PB14
#define SPI2_OUT_PIN        	(HAS_100PINS ? 54 :(HAS_144PINS ? 76 : 36)) 	//PB15
//
// DAC pin numbers
#define DAC_1_PIN           	(HAS_100PINS ? 29 :(HAS_144PINS ? 40 : 20))   	//PA4
#define DAC_2_PIN           	(HAS_100PINS ? 30 :(HAS_144PINS ? 41 : 21))   	//PA5
//
// PWM pin numbers
#define PWM_CH1_PIN         	(HAS_100PINS ? 31 :(HAS_144PINS ? 42 : 22))	    //PA6  - PWM1A/TIM3_CH1
#define PWM_CH2_PIN         	(HAS_100PINS ? 32 :(HAS_144PINS ? 43 : 23)) 	//PA7  - PWM1B/TIM3_CH2
#define PWM_CH3_PIN         	(HAS_100PINS ? 35 :(HAS_144PINS ? 46 : 26)) 	//PB0  - PWM1C/TIM3_CH3
#define PWM_CH5_PIN         	(HAS_100PINS ? 59 :(HAS_144PINS ? 81 : 0)) 	    //PD12 - PWM2A/TIM4_CH1
#define PWM_CH6_PIN         	(HAS_100PINS ? 95 :(HAS_144PINS ? 139 : 61)) 	//PB8  - PWM2B/TIM4_CH3 2A feather
#define PWM_CH7_PIN         	(HAS_100PINS ? 96 :(HAS_144PINS ? 140 : 62))	//PB9  - PWM2C/TIM4_CH4 2B feather
#define PWM_CH8_PIN         	(HAS_100PINS ?  4 :(HAS_144PINS ?   4 : 0))	    //PE5  - PWM3A/TIM9_CH1
#define PWM_CH9_PIN         	(HAS_100PINS ?  5 :(HAS_144PINS ?   5 : 0))  	//PE6  - PWM3A/TIM9_CH2

#define KEYBOARD_CLOCK			(HAS_100PINS ? 77 :(HAS_144PINS ? 110 : 0)) 	//PA15
#define KEYBOARD_DATA			(HAS_100PINS ? 84 :(HAS_144PINS ? 117 : 0)) 	//PD3

#define NRF_CE_PIN            	(HAS_100PINS ? 92 :(HAS_144PINS ? 136 : 30)) 	//PB6
#define NRF_CS_PIN            	(HAS_100PINS ? 93 :(HAS_144PINS ? 137 : 30)) 	//PB7
#define NRF_IRQ_PIN            	(HAS_100PINS ? 95 :(HAS_144PINS ? 139 : 30)) 	//PB8

#define F_CS	                (HAS_100PINS ? 35 :(HAS_144PINS ? 30 : 30))     //PB0

#endif


