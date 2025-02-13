/************************************************************************************************************************
Micromite

I2C.h

Routines to handle I2C access.

Copyright 2011 Gerard Sexton
This file is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

************************************************************************************************************************/

//#define INCLUDE_I2C_SLAVE                                           // uncomment this to include i2c slave functions

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)


void cmd_i2c(void);
void cmd_i2c2(void);
//void cmd_num2byte(void);
//void fun_byte2num(void);

void fun_mmi2c(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "I2C",	T_CMD,		0, cmd_i2c              },
	{ "I2C2",	T_CMD,		0, cmd_i2c2              },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

	{ "MM.I2C",	T_FNA | T_INT,	0, fun_mmi2c		},
#endif



#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#ifndef I2C_HEADER
#define I2C_HEADER

#define mI2C1BSetIntFlag()	(IFS1SET = _IFS1_I2C1BIF_MASK)      // Macro to set the I2C1 bus collision interrupt flag
#define mI2C1MSetIntFlag()	(IFS1SET = _IFS1_I2C1MIF_MASK)      // macro to set the I2C1 master interrupt flag
#define mI2C1SSetIntFlag()	(IFS1SET = _IFS1_I2C1SIF_MASK)      // macro to set the I2C1 slave interrupt flag

// states of the master state machine
#define I2C_State_Idle     0					    // Bus Idle
#define I2C_State_Start    1					    // Sending Start or Repeated Start
#define I2C_State_10Bit    2					    // Sending a 10 bit address
#define I2C_State_10BitRcv 3					    // 10 bit address receive
#define I2C_State_RcvAddr  4					    // Receive address
#define I2C_State_Send     5					    // Sending Data
#define I2C_State_Receive  6					    // Receiving data
#define I2C_State_Ack      7					    // Sending Acknowledgement
#define I2C_State_Stop     8					    // Sending Stop

// defines for I2C_Status
#define I2C_Status_Enabled			  0x00000001
#define I2C_Status_MasterCmd		  0x00000002
#define I2C_Status_NoAck			  0x00000010
#define I2C_Status_Timeout			  0x00000020
#define I2C_Status_InProgress		  0x00000040
#define I2C_Status_Completed		  0x00000080
#define I2C_Status_Interrupt		  0x00000100
#define I2C_Status_BusHold			  0x00000200
#define I2C_Status_10BitAddr		  0x00000400
#define I2C_Status_BusOwned			  0x00000800
#define I2C_Status_Send				  0x00001000
#define I2C_Status_Receive		      0x00002000
#define I2C_Status_Disable			  0x00004000
#define I2C_Status_Master			  0x00008000
#define I2C_Status_Slave			  0x00010000
#define I2C_Status_Slave_Send		  0x00020000
#define I2C_Status_Slave_Receive	  0x00040000
#define I2C_Status_Slave_Send_Rdy	  0x00080000
#define I2C_Status_Slave_Receive_Rdy  0x00100000
#define I2C_Status_Slave_Receive_Full 0x00200000

// Global variables provided by I2C.c
extern unsigned int I2C_Timer;                                      // master timeout counter
extern char *I2C_IntLine;                                           // pointer to the master interrupt line number
extern void i2c_disable(void);
extern void i2c2_disable(void);
extern void i2c1_slave_disable(void);
extern void i2c2_slave_disable(void);

extern volatile unsigned int I2C_Status;                            // status flags
extern volatile unsigned int I2C2_Status;                            // status flags
extern char *I2C_Slave_Send_IntLine;                                // pointer to the slave send interrupt line number
extern char *I2C_Slave_Receive_IntLine;                             // pointer to the slave receive interrupt line number
extern char *I2C2_Slave_Send_IntLine;                                // pointer to the slave send interrupt line number
extern char *I2C2_Slave_Receive_IntLine;                             // pointer to the slave receive interrupt line number

extern void RtcGetTime(void);
extern int cameraopen;
extern void CloseCamera(void);
#define REG_GAIN                    0x00         // Gain lower 8 bits (rest in vref
    #define REG_BLUE                    0x01         // blue gain
    #define REG_RED                     0x02         // red gain
    #define REG_VREF                    0x03         // Pieces of GAIN, VSTART, VSTOP
    #define REG_COM1                    0x04         // Control 1
    #define   COM1_CCIR656                  0x40     //   CCIR656 enable
    #define REG_BAVE                    0x05         // U/B Average level
    #define REG_GbAVE                   0x06         // Y/Gb Average level
    #define REG_AECHH                   0x07         // AEC MS 5 bits
    #define REG_RAVE                    0x08         // V/R Average level
    #define REG_COM2                    0x09         // Control 2
    #define   COM2_SSLEEP                   0x10     //   Soft sleep mode
    #define REG_PID                     0x0a         // Product ID MSB
    #define REG_VER                     0x0b         // Product ID LSB
    #define REG_COM3                    0x0c         // Control 3
    #define   COM3_SWAP                     0x40     //   Byte swap
    #define   COM3_SCALEEN                  0x08     //   Enable scaling
    #define   COM3_DCWEN                    0x04     //   Enable downsamp/crop/window
    #define REG_COM4                    0x0d         // Control 4
    #define REG_COM5                    0x0e         // All "reserved"
    #define REG_COM6                    0x0f         // Control 6
    #define REG_AECH                    0x10         // More bits of AEC value
    #define REG_CLKRC                   0x11         // Clocl control
    #define   CLK_EXT                       0x40     //   Use external clock directly
    #define   CLK_SCALE                     0x3f     //   Mask for internal clock scale
    #define   REG_COM7                    0x12         // Control 7
    #define   COM7_RESET                    0x80     //   Register reset
    #define   COM7_FMT_MASK                 0x38     //
    #define   COM7_FMT_VGA                  0x00     //   VGA format
    #define   COM7_FMT_CIF                  0x20     //   CIF format
    #define   COM7_FMT_QVGA                 0x10     //   QVGA format
    #define   COM7_FMT_QCIF                 0x08     //   QCIF format
    #define   COM7_RGB                      0x04     //   bits 0 and 2 - RGB format
    #define   COM7_YUV                      0x00     //   YUV
    #define   COM7_BAYER                    0x01     //   Bayer format
    #define   COM7_PBAYER                   0x05     //   "Processed bayer"
    #define   COM7_COLOR_BAR                0x02     //   Enable Color Bar
    #define REG_COM8                    0x13         // Control 8
    #define   COM8_FASTAEC                  0x80     //   Enable fast AGC/AEC
    #define   COM8_AECSTEP                  0x40     //   Unlimited AEC step size
    #define   COM8_BFILT                    0x20     //   Band filter enable
    #define   COM8_AGC                      0x04     //   Auto gain enable
    #define   COM8_AWB                      0x02     //   White balance enable
    #define   COM8_AEC                      0x01     //   Auto exposure enable
    #define REG_COM9                    0x14         // Control 9  - gain ceiling
    #define REG_COM10                   0x15         // Control 10
    #define   COM10_HSYNC                   0x40     //   HSYNC instead of HREF
    #define   COM10_PCLK_HB                 0x20     //   Suppress PCLK on horiz blank
    #define   COM10_HREF_REV                0x08     //   Reverse HREF
    #define   COM10_VS_LEAD                 0x04     //   VSYNC on clock leading edge
    #define   COM10_VS_NEG                  0x02     //   VSYNC negative
    #define   COM10_HS_NEG                  0x01     //   HSYNC negative
    #define REG_HSTART                  0x17         // Horiz start high bits
    #define REG_HSTOP                   0x18         // Horiz stop high bits
    #define REG_VSTART                  0x19         // Vert start high bits
    #define REG_VSTOP                   0x1a         // Vert stop high bits
    #define REG_PSHFT                   0x1b         // Pixel delay after HREF
    #define REG_MIDH                    0x1c         // Manuf. ID high
    #define REG_MIDL                    0x1d         // Manuf. ID low
    #define REG_MVFP                    0x1e         // Mirror / vflip
    #define   MVFP_MIRROR                   0x20     //   Mirror image
    #define   MVFP_FLIP                     0x10     //   Vertical flip
    #define REG_AEW                     0x24         // AGC upper limit
    #define REG_AEB                     0x25         // AGC lower limit
    #define REG_VPT                     0x26         // AGC/AEC fast mode op region
    #define REG_HSYST                   0x30         // HSYNC rising edge delay
    #define REG_HSYEN                   0x31         // HSYNC falling edge delay
    #define REG_HREF                    0x32         // HREF pieces
    #define REG_TSLB                    0x3a         // lots of stuff
    #define   TSLB_YLAST                        0x04     //   UYVY or VYUY - see com13
    #define   TSLB_UV                              0x10     //   enable special effects
    #define   TSLB_NEGATIVE                 0x20     //   enable special effects
    #define REG_COM11                   0x3b         // Control 11
    #define   COM11_NIGHT                   0x80     //   Night mode enable
    #define   COM11_NIGHT_FR2               0x20     //   Night mode 1/2 of normal framerate
    #define   COM11_NIGHT_FR4               0x40     //   Night mode 1/4 of normal framerate
    #define   COM11_NIGHT_FR8               0x60     //   Night mode 1/8 of normal framerate
    #define   COM11_HZAUTO                  0x10     //   Auto detect 50/60 Hz
    #define   COM11_50HZ                    0x08     //   Manual 50Hz select
    #define   COM11_EXP                     0x02     //       Exposure timing can be less than limit
    #define REG_COM12                   0x3c         // Control 12
    #define   COM12_HREF                    0x80     //   HREF always
    #define REG_COM13                   0x3d         // Control 13
    #define   COM13_GAMMA                   0x80     //   Gamma enable
    #define   COM13_UVSAT                   0x40     //   UV saturation auto adjustment
    #define   COM13_UVSWAP                  0x01     //   V before U - w/TSLB
    #define REG_COM14                   0x3e         // Control 14
    #define   COM14_DCWEN                   0x10     //   DCW/PCLK-scale enable
    #define   COM14_MAN_SCAL                0x08     //   Manual scaling enable
    #define   COM14_PCLK_DIV1               0x00     //   PCLK divided by 1
    #define   COM14_PCLK_DIV2               0x01     //   PCLK divided by 2
    #define   COM14_PCLK_DIV4               0x02     //   PCLK divided by 4
    #define   COM14_PCLK_DIV8               0x03     //   PCLK divided by 8
    #define   COM14_PCLK_DIV16              0x04         //   PCLK divided by 16
    #define REG_EDGE                    0x3f         // Edge enhancement factor
    #define REG_COM15                   0x40         // Control 15
    #define   COM15_R10F0                   0x00     //   Data range 10 to F0
    #define   COM15_R01FE                   0x80     //              01 to FE
    #define   COM15_R00FF                   0xc0     //              00 to FF
    #define   COM15_RGB565                  0x10     //   RGB565 output
    #define   COM15_RGB555                  0x30     //   RGB555 output
    #define      COM15_RGB444                     0x10         //       RGB444 output
    #define REG_COM16                   0x41         // Control 16
    #define   COM16_AWBGAIN                 0x08     //   AWB gain enable
    #define   COM16_DENOISE                 0x10     //   Enable de-noise auto adjustment
    #define   COM16_EDGE                          0x20     //   Enable edge enhancement
    #define REG_COM17                   0x42         // Control 17
    #define   COM17_AECWIN                  0xc0     //   AEC window - must match COM4
    #define   COM17_CBAR                    0x08     //   DSP Color bar
    #define REG_DENOISE_STRENGTH                    0x4c                   // De-noise strength
    #define REG_SCALING_XSC             0x70
    #define REG_SCALING_YSC             0x71
    #define REG_SCALING_DCWCTR          0x72
    #define REG_SCALING_PCLK_DIV        0x73
    #define REG_SCALING_PCLK_DELAY      0xa2
    // QQVGA setting
    #define HSTART_QQVGA                0x16
    #define HSTOP_QQVGA                 0x04
    #define HREF_QQVGA                  0xa4  //0x24? 0xa4?
    #define VSTART_QQVGA                0x02
    #define VSTOP_QQVGA                 0x7a
    #define VREF_QQVGA                  0x0a
    #define COM3_QQVGA                  0x04
    #define COM14_QQVGA                 0x1A
    #define SCALING_XSC_QQVGA           0x3a
    #define SCALING_YSC_QQVGA           0x35
    #define SCALING_DCWCTR_QQVGA        0x22
    #define SCALING_PCLK_DIV_QQVGA      0xf2
    #define SCALING_PCLK_DELAY_QQVGA    0x02
    #define REG_CMATRIX_BASE            0x4f
    #define REG_CMATRIX_1            0x4f
    #define REG_CMATRIX_2            0x50
    #define REG_CMATRIX_3            0x51
    #define REG_CMATRIX_4            0x52
    #define REG_CMATRIX_5            0x53
    #define REG_CMATRIX_6            0x54
    #define CMATRIX_LEN                    6
    #define REG_CMATRIX_SIGN            0x58
    //
    #define REG_BRIGHT                  0x55         // Brightness
    #define REG_CONTRAST                 0x56         // Contrast control
    #define REG_GFIX                    0x69         // Fix gain control
    #define REG_GGAIN                   0x6a         // G channel AWB gain
    #define REG_DBLV                        0x6b         // PLL control
    #define REG_REG76                   0x76         // OV//s name
    #define R76_BLKPCOR                   0x80     //   Black pixel correction enable
    #define R76_WHTPCOR                   0x40     //   White pixel correction enable
    #define REG_RGB444                  0x8c         // RGB 444 control
    #define     R444_ENABLE                   0x02     //   Turn on RGB444, overrides 5x5
    #define     R444_RGBX                     0x01     //   Empty nibble at end
    #define R444_XBGR                            0x00
    #define REG_HAECC1                  0x9f         // Hist AEC/AGC control 1
    #define REG_HAECC2                  0xa0         // Hist AEC/AGC control 2
    #define REG_BD50MAX                 0xa5         // 50hz banding step limit
    #define REG_HAECC3                  0xa6         // Hist AEC/AGC control 3
    #define REG_HAECC4                  0xa7         // Hist AEC/AGC control 4
    #define REG_HAECC5                  0xa8         // Hist AEC/AGC control 5
    #define REG_HAECC6                  0xa9         // Hist AEC/AGC control 6
    #define REG_HAECC7                  0xaa         // Hist AEC/AGC control 7
    #define REG_BD60MAX                 0xab         // 60hz banding step limit
#endif
#endif
