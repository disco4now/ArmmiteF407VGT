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
#define RAM(a)  ((a >= RAMBASE && a < RAMEND) || (a >= 0x40000000 && a < 0x50060fff) || (a >= 0xd0000000 && a < 0xd0800000) || (a >= 0x24000000 && a < 0x24080000) || (a >= 0x38000000 && a<  0x38010000) || (a >= 0x38800000 && a<  0x38801000))
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

#define NBR_PINS_MAXCHIP    100				    // max number of pins for supported packages on chip
#define MAX_ANALOGUE_PIN_PACKAGE    55		       // max analogue pin no for supported packages on chip



#define package (*(volatile unsigned int *)(PACKAGE_BASE) & 0b11111)
#define flashsize *(volatile unsigned int *)(FLASHSIZE_BASE)
#define chipID (DBGMCU->IDCODE & 0x00000FFF)
//#define HAS_32PINS          (package==0x8)
//#define HAS_48PINS          (package==0xB)
//#define HAS_64PINS          (package==0x9 || package==0)
#define HAS_64PINS          (package==0xA )
//#define HAS_100PINS          (package==0x10)
#define HAS_100PINS          (package==0x1C)
//#define HAS_144PINS          (package==0x03)
#define NBRPINS             (HAS_100PINS ? 100 : 64)

#define NBRINTPINS          10
#if defined(DEFINE_PINDEF_TABLE)
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
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 38 FSMC_D4
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 39 FSMC_D5
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 40 FSMC_D6
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 41 FSMC_D7
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 42 FSMC_D8
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 43 FSMC_D9
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 44 FSMC_D10
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 45 FSMC_D11
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 46 FSMC_D12
	    { GPIOB,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 47 I2C2-SCL
	    { GPIOB,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 48 I2C2-SDA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 49 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 50 VDD
		//
	    { GPIOB,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 51 T_CS
	    { GPIOB,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 52 SPI2-CLK
	    { GPIOB,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 53 SPI2-IN
	    { GPIOB,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    						// pin 54 SPI2-OUT
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 55 FSMC_D13
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 56 FSMC_D14
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 57 FSMC_D15
		{ GPIOD,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 58 Drive_VBUS_FS
		{ GPIOD,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT ,  NULL, 0},    						// pin 59 PWM-2A
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 60 FSMC_A18
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 61 FSMC_D0
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 62 FSMC_D1
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
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 81 FSMC_D2
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 82 FSMC_D3
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 83 SDIO_CMD
	    { GPIOD,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 84 KBD_DATA
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 85 FSMC_NOE
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 86 FSMC_NWE
	    { GPIOD,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},    							// pin 87
		{ NULL,  0, PUNUSED , NULL, 0},                                                       	// pin 88 FSMC_NE1
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
	const extern struct s_PinDef PinDef144[];
#endif      // DEFINE_PINDEF_TABLE
    extern struct s_PinDef *PinDef;
#ifdef RGT
#define INT1PIN              	41  //PA1,PE1
#define INT2PIN              	41  	//PE3
#define INT3PIN             	41  	//PE4
#define INT4PIN              	41 	//PA8
#define IRPIN                  	41	//PE2

   // I2C pin numbers
#define P_I2C_SCL           	58 	//PB6
#define P_I2C_SDA           	59 	//PB7

#define P_I2C2_SCL           	29 	//PB10
#define P_I2C2_SDA           	30 	//PB11


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

    // COMx: port pin numbers
    #define COM1_TX_PIN         	42 	//PA9 USART1
    #define COM1_RX_PIN         	43 	//PA10

    #define COM2_TX_PIN         	37 	//PC6 USART6
    #define COM2_RX_PIN         	38 	//PC7

    #define COM3_TX_PIN         	14 	//PA0 UART4
    #define COM3_RX_PIN         	15	//PA1

    #define COM4_RX_PIN				16 	//PA2 USART2
    #define COM4_TX_PIN				17 	//PA3

    // PWM pin numbers
    #define PWM_CH1_PIN         	22 	//PA6  - PWM1A/TIM3_CH1
    #define PWM_CH2_PIN         	23 	//PA7  - PWM1B/TIM3_CH2
    #define PWM_CH3_PIN         	26 	//PB0  - PWM1C/TIM3_CH3
   // #define PWM_CH5_PIN         	59 	//PD12 - PWM2A/TIM4_CH1
    #define PWM_CH6_PIN         	61 	//PB8  - PWM2A/TIM4_CH3
    #define PWM_CH7_PIN         	62 	//PB9  - PWM2A/TIM4_CH4
   // #define PWM_CH8_PIN         	4	//PE5  - PWM3A/TIM9_CH1
   //#define PWM_CH9_PIN         	5	//PE6  - PWM3A/TIM9_CH2



#else

#define INT1PIN              	98  //PA1,PE1
#define INT2PIN              	2  	//PE3
#define INT3PIN             	3  	//PE4
#define INT4PIN              	67 	//PA8
#define IRPIN                  	1	//PE2
// I2C pin numbers
#define P_I2C_SCL           	92 	//PB6
#define P_I2C_SDA           	93 	//PB7

#define P_I2C2_SCL           	47 	//PB10
#define P_I2C2_SDA           	48 	//PB11

// COMx: port pin numbers
#define COM1_TX_PIN         	68 	//PA9 USART1
#define COM1_RX_PIN         	69 	//PA10

#define COM2_TX_PIN         	63 	//PC6 USART6
#define COM2_RX_PIN         	64 	//PC7

#define COM3_TX_PIN         	23 	//PA0 UART4
#define COM3_RX_PIN         	24 	//PA1

#define COM4_RX_PIN				25 	//PA2 USART2
#define COM4_TX_PIN				26 	//PA3

    // SPI pin numbers
#define SPI_CLK_PIN         	89 	//PB3
#define SPI_INP_PIN         	90 	//PB4
#define SPI_OUT_PIN         	91 	//PB5

// SPI2 pin numbers
#define SPI2_CLK_PIN        	52 	//PB13
#define SPI2_INP_PIN        	53 	//PB14
#define SPI2_OUT_PIN        	54 	//PB15
//
// DAC pin numbers
#define DAC_1_PIN           	29 	//PA4
#define DAC_2_PIN           	30 	//PA5
//
// PWM pin numbers
#define PWM_CH1_PIN         	31 	//PA6  - PWM1A/TIM3_CH1
#define PWM_CH2_PIN         	32 	//PA7  - PWM1B/TIM3_CH2
#define PWM_CH3_PIN         	35 	//PB0  - PWM1C/TIM3_CH3
#define PWM_CH5_PIN         	59 	//PD12 - PWM2A/TIM4_CH1
#define PWM_CH6_PIN         	95 	//PB8  - PWM2A/TIM4_CH3
#define PWM_CH7_PIN         	96 	//PB9  - PWM2A/TIM4_CH4
#define PWM_CH8_PIN         	4	//PE5  - PWM3A/TIM9_CH1
#define PWM_CH9_PIN         	5	//PE6  - PWM3A/TIM9_CH2

#define KEYBOARD_CLOCK			77 	//PA15
#define KEYBOARD_DATA			84 	//PD3

#define NRF_CE_PIN            	92 	//PB6
#define NRF_CS_PIN            	93 	//PB7
#define NRF_IRQ_PIN            	95 	//PB8

#define F_CS	                35   //PB0
#endif

#endif


