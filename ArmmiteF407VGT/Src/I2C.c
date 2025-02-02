/************************************************************************************************************************
Micromite

I2C.c

Routines to handle I2C access.

Copyright 2011 Gerard Sexton
This file is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

************************************************************************************************************************/


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
extern I2C_HandleTypeDef hi2c1, hi2c2;


// Declare functions
void i2cEnable(char *p);
void i2cDisable(char *p);
void i2cSend(char *p);
void i2cReceive(char *p);
void i2c_disable(void);
void i2c_enable(int bps);
void i2c_masterCommand(int timer);
void i2cCheck(char *p);

void i2c2Enable(char *p);
void i2c2Disable(char *p);
void i2c2Send(char *p);
void i2c2Receive(char *p);
void i2c2_disable(void);
void i2c2_enable(int bps);
void i2c2_masterCommand(int timer);
void i2c2Check(char *p);

#ifndef INCLUDE_I2C_SLAVE
void i2cSlaveEnable(char *p,int channel);
void i2cSlaveDisable(char *p,int channel);
//void i2cSlaveSend(char *p);
void i2cSendSlave(char *p);
void i2c2SendSlave(char *p);

//void i2cSlaveReceive(char *p);
void i2cReceiveSlave(char *p);
void i2c2ReceiveSlave(char *p);

void i2c1_slave_disable(void);
void i2c2_slave_disable(void);
//void i2c_slave_enable(int options);
void i2c1_slave_enable();
void i2c2_slave_enable();
//void i2c_slave_send(int sendlen);
//void i2c_slave_receive(int rcvlen, long long int *IntPtr, MMFLOAT *FloatPtr, char *CharPtr, MMFLOAT *rcvdlenFloat, long long int *rcvdlenInt);
#endif


static MMFLOAT *I2C_Rcvbuf_Float;									// pointer to the master receive buffer for a MMFLOAT
static long long int *I2C_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
static char *I2C_Rcvbuf_String;										// pointer to the master receive buffer for a string
static unsigned int I2C_Addr;										// I2C device address
static volatile unsigned int I2C_Sendlen;							// length of the master send buffer
static volatile unsigned int I2C_Rcvlen;							// length of the master receive buffer
static unsigned char I2C_Send_Buffer[256];                          // I2C send buffer
static unsigned int I2C_enabled;									// I2C enable marker
static unsigned int I2C_Timeout;									// master timeout value
volatile unsigned int I2C_Status;										// status flags
static int mmI2Cvalue;
	// value of MM.I2C
static MMFLOAT *I2C2_Rcvbuf_Float;									// pointer to the master receive buffer for a MMFLOAT
static long long int *I2C2_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
static char *I2C2_Rcvbuf_String;									// pointer to the master receive buffer for a string
static unsigned int I2C2_Addr;										// I2C device address
static volatile unsigned int I2C2_Sendlen;							// length of the master send buffer
static volatile unsigned int I2C2_Rcvlen;							// length of the master receive buffer
static unsigned char I2C2_Send_Buffer[256];                         // I2C send buffer
static unsigned int I2C2_enabled;									// I2C enable marker
static unsigned int I2C2_Timeout;									// master timeout value
volatile unsigned int I2C2_Status;									// status flags

extern void fastwrite480(int fnbr);
extern RTC_HandleTypeDef hrtc;
extern void SaveToBuffer(void);
extern void CompareToBuffer(void);


//static unsigned int I2C_Slave_Mask;                                 // slave address mask
static unsigned int I2C_Slave_Addr;                                 // slave address
static unsigned int I2C2_Slave_Addr;                                 // slave address
//static volatile unsigned int I2C_Slave_Sendlen;                     // length of the slave send buffer

//#ifdef INCLUDE_I2C_SLAVE
    static unsigned char I2C_Rcv_Buffer[256];                                // I2C receive buffer
    char *I2C_Slave_Send_IntLine;                                   // pointer to the slave send interrupt line number
    char *I2C_Slave_Receive_IntLine;                                // pointer to the slave receive interrupt line number

    static unsigned char I2C2_Rcv_Buffer[256];                                // I2C receive buffer
    char *I2C2_Slave_Send_IntLine;                                   // pointer to the slave send interrupt line number
    char *I2C2_Slave_Receive_IntLine;                                // pointer to the slave receive interrupt line number
//#else
//    void i2c_slave_disable(void) {}
//#endif

 //   extern void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c);
/*******************************************************************************************
							  I2C related commands in MMBasic
                              ===============================
These are the functions responsible for executing the I2C related commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

********************************************************************************************/

void cmd_i2c(void) {
    char *p, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2cEnable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2cDisable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2cSend(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2cReceive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2cCheck(p);
    else if((pp = checkstring(cmdline, "SLAVE")) != NULL) {
        if((p = checkstring(pp, "OPEN")) != NULL)
            i2cSlaveEnable(p,0);
        else if((p = checkstring(pp, "CLOSE")) != NULL)
            i2cSlaveDisable(p,0);
        else if((p = checkstring(pp, "WRITE")) != NULL)
        	i2cSendSlave(p);
        else if((p = checkstring(pp, "READ")) != NULL)
            i2cReceiveSlave(p);   // i2cReceiveSlave(p,0);
        else
            error("Unknown command");
    }
    else
        error("Unknown command");
}
void cmd_i2c2(void) {
    char *p, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2c2Enable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2c2Disable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2c2Send(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2c2Receive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2c2Check(p);
    else if((pp = checkstring(cmdline, "SLAVE")) != NULL) {
        if((p = checkstring(pp, "OPEN")) != NULL)
            i2cSlaveEnable(p,1);
        else if((p = checkstring(pp, "CLOSE")) != NULL)
            i2cSlaveDisable(p,1);
        else if((p = checkstring(pp, "WRITE")) != NULL)
        	i2c2SendSlave(p);
        else if((p = checkstring(pp, "READ")) != NULL)
            i2c2ReceiveSlave(p);   // i2cReceiveSlave(p,0);
        else
            error("Unknown command");
    }
    else
        error("Unknown command");
}
/*
 * *Initialize RTC and set the Time and Date
*/
void RtcGetTime(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	int localsecs;
	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	localsecs=sTime.Seconds;
	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
	if(sTime.SubSeconds<5){
		do {
			if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
		} while(localsecs==sTime.Seconds);
	}
	second = sTime.Seconds;
	minute = sTime.Minutes;
	hour = sTime.Hours;
	day = sDate.Date;
	month = sDate.Month;
	year = sDate.Year+2000;
	day_of_week=sDate.WeekDay;
}

/**************************************************************************************************
I2C HAL functions to support - slave mode
***************************************************************************************************/

uint8_t rxcount=0;
uint8_t txcount=0;
uint8_t bytesRrecvd = 0;
uint8_t bytesTransd = 0;

extern void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

extern void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{


	if(TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		if (hi2c->Instance == I2C1)
		    HAL_I2C_Slave_Seq_Receive_IT(hi2c, I2C_Rcv_Buffer+rxcount, 1, I2C_LAST_FRAME); // Initiate Reading of data
		else
			HAL_I2C_Slave_Seq_Receive_IT(hi2c, I2C2_Rcv_Buffer+rxcount, 1, I2C_LAST_FRAME); // Initiate Reading of data

		//MMPrintString(" Master Address Match Tx \r\n");
	}
	else if(TransferDirection == I2C_DIRECTION_RECEIVE)  // master requesting the data is not supported yet
	{
		txcount = 0;
		//HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C_REGISTERS+startPosition+txcount, 1, I2C_FIRST_FRAME);
		//MMPrintString("Master Address Match Rx \r\n");
		if (hi2c->Instance == I2C1)
		   I2C_Status |= I2C_Status_Slave_Send_Rdy;
		else
		   I2C2_Status |= I2C_Status_Slave_Send_Rdy;
	}
}
/* RECEIVE FROM MASTER  */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rxcount++;
	if (rxcount < 256)
	{
		if (rxcount == 256-1)
		//if (rxcount == I2C_Rcvbuf_Int-1)
		{

			if (hi2c->Instance == I2C1){
			  HAL_I2C_Slave_Seq_Receive_IT(hi2c, I2C_Rcv_Buffer+rxcount, 1, I2C_LAST_FRAME);
			  I2C_Status |= I2C_Status_Slave_Receive_Rdy;
			}else{
			  HAL_I2C_Slave_Seq_Receive_IT(hi2c, I2C2_Rcv_Buffer+rxcount, 1, I2C_LAST_FRAME);
			  I2C2_Status |= I2C_Status_Slave_Receive_Rdy;
			}
		}
		else
		{
			if (hi2c->Instance == I2C1)
			   HAL_I2C_Slave_Seq_Receive_IT(hi2c,I2C_Rcv_Buffer+rxcount, 1, I2C_NEXT_FRAME);
			else
			   HAL_I2C_Slave_Seq_Receive_IT(hi2c,I2C2_Rcv_Buffer+rxcount, 1, I2C_NEXT_FRAME);
		}
	}

}

/* TRANSMIT TO MASTER */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	//MMPrintString("TxCallback called \r\n");
	txcount++;
	if (hi2c->Instance == I2C1)
	   HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C_Send_Buffer+txcount, 1, I2C_NEXT_FRAME);
	else
	   HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C2_Send_Buffer+txcount, 1, I2C_NEXT_FRAME);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t errorcode = HAL_I2C_GetError(hi2c);

	if (errorcode == 4)  // AF error
	{
		if (txcount == 0)  // error is while slave is receiving
		{
			bytesRrecvd = rxcount;  // the first byte is the register address
			rxcount = 0;  // Reset the rxcount for the next operation
			//PInt(bytesRrecvd);MMPrintString(" bytes received \r\n");
			//Call MMBasic Interrupt to call  I2C SLAVE READ
			if (hi2c->Instance == I2C1)
			   I2C_Status |= I2C_Status_Slave_Receive_Rdy;
			else
			   I2C2_Status |= I2C_Status_Slave_Receive_Rdy;

			//process_data();
		}
		else // error while slave is transmitting
		{
			bytesTransd = txcount-1;  // the txcount is 1 higher than the actual data transmitted
			txcount = 0;  // Reset the txcount for the next operation
			//PInt(bytesTransd);MMPrintString(" bytes transmitted \r\n");
		}
	}

	/* BERR Error commonly occurs during the Direction switch
	 * Here we the software reset bit is set by the HAL error handler
	 * Before resetting this bit, we make sure the I2C lines are released and the bus is free
	 * I am simply reinitializing the I2C to do so
	 */
	else if (errorcode == 1)  // BERR Error
	{
		HAL_I2C_DeInit(hi2c);
		HAL_I2C_Init(hi2c);
	//	memset(RxData,'\0',RxSIZE);  // reset the Rx buffer
		rxcount =0;  // reset the count
	}
	HAL_I2C_EnableListen_IT(hi2c);
	//I2C_Status |= I2C_Status_Slave_Receive_Rdy;
}



/**************************************************************************************************
Enable the I2C1 module - slave mode  -Process commandline  I2C SLAVE OPEN addr,send_int,rec_int
***************************************************************************************************/
void i2cSlaveEnable(char *p,int channel) {
	// int options, addr, mask;
	   int addr;
	    getargs(&p, 5, ",");
	    if(argc != 5) error("Argument count");
	    addr = getinteger(argv[0]);
	    //mask = getinteger(argv[2]);
	   // options = getint(argv[4], 0, 3);
	   // if(options & 0x02) {    // 10 bit addressing
	   //     if(addr < 0x0000 || addr > 0x03ff) error("Invalid address");
	   //     if(mask < 0x0000 || mask > 0x03ff) error("Invalid mask");
	   // } else {    // 7 bit addressing
	       if(addr < 0x00 || (addr > 0x00 && addr > 0x77) /*|| (addr == 0x00 && !(options & 0x01))*/)
	          error("Device address");
	    //    if(mask < 0x00 || mask > 0x77) error("Invalid mask");
	   // }
	    ;

	   // I2C_Slave_Mask = mask;
	    //I2C_Slave_Send_IntLine = GetIntAddress(argv[2]);                // get the interrupt routine's location
	    //I2C_Slave_Receive_IntLine = GetIntAddress(argv[4]);             // get the interrupt routine's location

	    //i2c_slave_enable(options);
	    if (channel==0){
	    	I2C_Slave_Addr = addr;
	    	if(I2C_Status & I2C_Status_Slave) error("I2C Slave already OPEN");
	    	if(I2C_Status & I2C_Status_Enabled) error("I2C already OPEN");
	    	InterruptUsed = true;
	        I2C_Slave_Send_IntLine = GetIntAddress(argv[2]);                // get the interrupt routine's location
	    	I2C_Slave_Receive_IntLine = GetIntAddress(argv[4]);             // get the interrupt routine's location
	        i2c1_slave_enable();
	    }else{
	    	I2C2_Slave_Addr = addr;
	    	if(I2C2_Status & I2C_Status_Slave) error("I2C Slave already OPEN");
	    	if(I2C2_Status & I2C_Status_Enabled) error("I2C already OPEN");
	    	InterruptUsed = true;
	    	I2C2_Slave_Send_IntLine = GetIntAddress(argv[2]);                // get the interrupt routine's location
	    	I2C2_Slave_Receive_IntLine = GetIntAddress(argv[4]);             // get the interrupt routine's location
	        i2c2_slave_enable();
	    }
}

/**************************************************************************************************
Disable the I2C1 module - slave mode -Process commandline
***************************************************************************************************/
void i2cSlaveDisable(char *p,int channel) {
    checkend(p);
    if (channel==0)
      i2c1_slave_disable();
    else
      i2c2_slave_disable();
}

/**************************************************************************************************
Enable the I2C1 module - slave mode -Enable hardware
***************************************************************************************************/
void i2c1_slave_enable() {
	    HAL_I2C_DeInit(&hi2c1);
	    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
	    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
	    PinSetBit(P_I2C_SDA, TRISSET);
	    if(PinRead(P_I2C_SDA) == 0) {
	    	int i;
	        // it appears as if something is holding SLA low
	        // try pulsing the clock to get rid of it
	        PinSetBit(P_I2C_SCL, TRISCLR);
	        PinSetBit(P_I2C_SCL, LATCLR);
	        for(i = 0; i < 20; i++) {
	            uSec(25);
	            PinSetBit(P_I2C_SCL, LATINV);
	            if(PinRead(P_I2C_SDA) == 0) break;
	        }
	        PinSetBit(P_I2C_SCL, TRISSET);
	    }
	    if(!(I2C_Status & I2C_Status_Slave)) {
	        //mI2C1SClearIntFlag();                                       // ensure slave flag is clear
	       // mI2C1SIntEnable(1);                                         // enable slave interrupt
	        I2C_Status |= I2C_Status_Slave;
	    }
	    if(!(I2C_Status & I2C_Status_Enabled)) {
	       I2C_Status |= I2C_Status_Enabled;
	       ExtCfg(P_I2C_SDA, EXT_COM_RESERVED, 0);
	       ExtCfg(P_I2C_SCL, EXT_COM_RESERVED, 0);
	       hi2c1.Init.OwnAddress1 = I2C_Slave_Addr<<1;
	      // PIntH(I2C_Slave_Addr<<1);MMPrintString("Read Address \r\n");
	       HAL_I2C_Init(&hi2c1);
	       HAL_I2C_EnableListen_IT(&hi2c1);


		   //  I2C_enabled=1;
	    }



}

/**************************************************************************************************
Enable the I2C2 module - slave mode -Enable hardware
***************************************************************************************************/
//void i2c_slave_enable(int options) {
void i2c2_slave_enable() {
	    HAL_I2C_DeInit(&hi2c2);
	    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
	    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
	    PinSetBit(P_I2C2_SDA, TRISSET);
	    if(PinRead(P_I2C2_SDA) == 0) {
	    	int i;
	        // it appears as if something is holding SLA low
	        // try pulsing the clock to get rid of it
	        PinSetBit(P_I2C2_SCL, TRISCLR);
	        PinSetBit(P_I2C2_SCL, LATCLR);
	        for(i = 0; i < 20; i++) {
	            uSec(25);
	            PinSetBit(P_I2C2_SCL, LATINV);
	            if(PinRead(P_I2C2_SDA) == 0) break;
	        }
	        PinSetBit(P_I2C2_SCL, TRISSET);
	    }
	    if(!(I2C2_Status & I2C_Status_Slave)) {
	        //mI2C1SClearIntFlag();                                       // ensure slave flag is clear
	       // mI2C1SIntEnable(1);                                         // enable slave interrupt
	        I2C2_Status |= I2C_Status_Slave;
	    }
	    if(!(I2C2_Status & I2C_Status_Enabled)) {
	       I2C2_Status |= I2C_Status_Enabled;
	       ExtCfg(P_I2C2_SDA, EXT_COM_RESERVED, 0);
	       ExtCfg(P_I2C2_SCL, EXT_COM_RESERVED, 0);

	       hi2c2.Init.OwnAddress1 = I2C2_Slave_Addr<<1;
	       //PIntH(I2C2_Slave_Addr<<1);MMPrintString("Read Address \r\n");
	       HAL_I2C_Init(&hi2c2);
	       HAL_I2C_EnableListen_IT(&hi2c2);


		   //  I2C_enabled=1;
	    }



}



/**************************************************************************************************
Disable the I2C1 module - slave mode -Disable hardware
***************************************************************************************************/
void i2c1_slave_disable() {
   // I2C_Status = I2C_Status_Disable;
	if(!(I2C_Status & I2C_Status_Master)) {
	  I2C_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
      I2C_Rcvbuf_Float = NULL;
      I2C_Rcvbuf_Int = NULL;
	  I2C_Sendlen = 0;												// length of the master send buffer
	  I2C_Rcvlen = 0;													// length of the master receive buffer
	  I2C_Addr = 0;													// I2C device address
	 I2C_Timeout = 0;												// master timeout value

	//I2C_enabled = 0;


		HAL_I2C_DeInit(&hi2c1);
	    I2C_Status = 0;                                             // clear status flags
	    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
	    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
	}

}

/**************************************************************************************************
Disable the I2C2 module - slave mode -Disable hardware
***************************************************************************************************/
void i2c2_slave_disable() {
   // I2C_Status = I2C_Status_Disable;
	if(!(I2C2_Status & I2C_Status_Master)) {
	  I2C2_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
      I2C2_Rcvbuf_Float = NULL;
      I2C2_Rcvbuf_Int = NULL;
	  I2C2_Sendlen = 0;												// length of the master send buffer
	  I2C2_Rcvlen = 0;													// length of the master receive buffer
	  I2C2_Addr = 0;													// I2C device address
	  I2C2_Timeout = 0;												// master timeout value

	//I2C_enabled = 0;

	   	HAL_I2C_DeInit(&hi2c2);
	    I2C2_Status = 0;                                             // clear status flags
	    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
	    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
	}

}

//
/* send data to I2C master - Slave Tx  mode
    I2C SLAVE WRITE sendlen,senddata[,senddata]
  This is called in the send_int interrupt when when the slave address is matched
  and a read request is received from the master by STM32 I2C slave  function.
  We need to verify the command syntax and send the data by calling the STM32 I2C slave
  function to send the data.
*/
void i2cSendSlave( char *p) {
	int sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;
	getargs(&p, 99, ( char *)",");
	if(!(argc >=3)) error("Invalid syntax");
	if(!(I2C_Status & I2C_Status_Slave ))error("I2C slave not open");
	sendlen = getinteger(argv[0]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 3) {		// numeric expressions for data
		if(sendlen != ((argc - 1) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 2]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[2], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	//MMPrintString("Slave Send Starting \r\n");
	HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, I2C_Send_Buffer+txcount, 1, I2C_NEXT_FRAME);


}

void i2c2SendSlave( char *p) {
	int sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;
	getargs(&p, 99, ( char *)",");
	if(!(argc >=3)) error("Invalid syntax");
	if(!(I2C2_Status & I2C_Status_Slave ))error("I2C slave not open");
	sendlen = getinteger(argv[0]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 3) {		// numeric expressions for data
		if(sendlen != ((argc - 1) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C2_Send_Buffer[i] = getinteger(argv[i + i + 2]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[2], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C2_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	//MMPrintString("Slave Send Starting \r\n");
	HAL_I2C_Slave_Seq_Transmit_IT(&hi2c2, I2C_Send_Buffer+txcount, 1, I2C_NEXT_FRAME);


}



/* receive data from master - slave Rx mode
    I2C SLAVE READ rcvlen,rcvbuf, rcvd
  The STM32 I2C slave functions have already received all data from the
  master before the rcv_int interrupt holding this   command is called.
  We just need to verify the command syntax  and return the data in the
  MMBasic environment.
*/

void i2cReceiveSlave(char *p) {
	int rcvlen;
	void *ptr = NULL;
    MMFLOAT *rcvdlenFloat=NULL;
	long long int *rcvdlenInt=NULL;
    I2C_Rcvbuf_Float = NULL;
    I2C_Rcvbuf_Int = NULL;
    I2C_Rcvbuf_String = NULL;
 	getargs(&p, 5, ( char *)",");
	if(argc != 5) error("Invalid syntax");
	if(!(I2C_Status & I2C_Status_Slave ))error("I2C slave not open");
	rcvlen = getinteger(argv[0]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");
	ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Int = (long long int *)ptr;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C_Rcvbuf_String = (char *)ptr + 1;
    } else error("Invalid variable");
    ptr = findvar(argv[4], V_FIND);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if(vartbl[VarIndex].type & T_NBR)  rcvdlenFloat = (MMFLOAT *)ptr;
    else if(vartbl[VarIndex].type & T_INT) rcvdlenInt = (long long int *)ptr;
	else error("Invalid variable");

	  for(int i=0;i<rcvlen;i++){
		if(I2C_Rcvbuf_String!=NULL){
			*I2C_Rcvbuf_String=I2C_Rcv_Buffer[i];
			I2C_Rcvbuf_String++;
		}
		if(I2C_Rcvbuf_Float!=NULL){
			*I2C_Rcvbuf_Float=I2C_Rcv_Buffer[i];
			I2C_Rcvbuf_Float++;
		}
		if(I2C_Rcvbuf_Int!=NULL){
			*I2C_Rcvbuf_Int=I2C_Rcv_Buffer[i];
			I2C_Rcvbuf_Int++;
		}
	  }

	if(!(rcvdlenFloat == NULL))
          *rcvdlenFloat = (MMFLOAT)bytesRrecvd;
      else
          *rcvdlenInt = (long long int)bytesRrecvd;

}

void i2c2ReceiveSlave(char *p) {
	int rcvlen;
	void *ptr = NULL;
    MMFLOAT *rcvdlenFloat=NULL;
	long long int *rcvdlenInt=NULL;
    I2C2_Rcvbuf_Float = NULL;
    I2C2_Rcvbuf_Int = NULL;
    I2C2_Rcvbuf_String = NULL;
	getargs(&p, 5, ( char *)",");
	if(argc != 5) error("Invalid syntax");
	if(!(I2C2_Status & I2C_Status_Slave ))error("I2C slave not open");
	rcvlen = getinteger(argv[0]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");
	ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Int = (long long int *)ptr;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C2_Rcvbuf_String = (char *)ptr + 1;
    } else error("Invalid variable");
    ptr = findvar(argv[4], V_FIND);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if(vartbl[VarIndex].type & T_NBR)  rcvdlenFloat = (MMFLOAT *)ptr;
    else if(vartbl[VarIndex].type & T_INT) rcvdlenInt = (long long int *)ptr;
	else error("Invalid variable");

  	  for(int i=0;i<rcvlen;i++){
  		if(I2C2_Rcvbuf_String!=NULL){
  			*I2C2_Rcvbuf_String=I2C2_Rcv_Buffer[i];
  			I2C2_Rcvbuf_String++;
  		}
  		if(I2C2_Rcvbuf_Float!=NULL){
  			*I2C2_Rcvbuf_Float=I2C2_Rcv_Buffer[i];
  			I2C2_Rcvbuf_Float++;
  		}
  		if(I2C2_Rcvbuf_Int!=NULL){
  			*I2C2_Rcvbuf_Int=I2C2_Rcv_Buffer[i];
  			I2C2_Rcvbuf_Int++;
  		}
  	  }


	if(!(rcvdlenFloat == NULL))
          *rcvdlenFloat = (MMFLOAT)bytesRrecvd;
      else
          *rcvdlenInt = (long long int)bytesRrecvd;

}



#ifdef INCLUDE_I2C_SLAVE

// enable the I2C1 module - slave mode
void i2cSlaveEnable(char *p) {
    int options, addr, mask;
    getargs(&p, 9, ",");
    if(argc != 9) error("Argument count");
    addr = getinteger(argv[0]);
    mask = getinteger(argv[2]);
    options = getint(argv[4], 0, 3);
    if(options & 0x02) {    // 10 bit addressing
        if(addr < 0x0000 || addr > 0x03ff) error("Invalid address");
        if(mask < 0x0000 || mask > 0x03ff) error("Invalid mask");
    } else {    // 7 bit addressing
        if(addr < 0x00 || (addr > 0x00 && addr > 0x77) || (addr == 0x00 && !(options & 0x01)))
            error("Device address");
        if(mask < 0x00 || mask > 0x77) error("Invalid mask");
    }
    if(I2C_Status & I2C_Status_Slave) error("Slave enabled");
    I2C_Slave_Addr = addr;
    I2C_Slave_Mask = mask;
    I2C_Slave_Send_IntLine = GetIntAddress(argv[6]);                // get the interrupt routine's location
    I2C_Slave_Receive_IntLine = GetIntAddress(argv[8]);             // get the interrupt routine's location
    InterruptUsed = true;
    i2c_slave_enable(options);
}


// disable the I2C1 module - slave mode
void i2cSlaveDisable(char *p) {
    checkend(p);
    i2c_slave_disable();
}


// send data to the I2C master - slave mode
void i2cSlaveSend(char *p) {
    int sendlen, i;
    void *ptr;
    unsigned char *cptr = NULL;

    getargs(&p, 99, ",");
    if(!(argc & 0x01) || argc < 3) error("Argument count");
    if(!(I2C_Status & I2C_Status_Slave)) error("Slave not enabled");
    if(!(I2C_Status & I2C_Status_Slave_Send)) error("Not in slave send state");
    sendlen = getint(argv[0], 1, 255);
    if(sendlen == 1 || argc > 3) {      // numeric expressions for data
        if(sendlen != ((argc - 1) >> 1)) error("Argument count");
        for (i = 0; i < sendlen; i++) {
            I2C_Send_Buffer[i] = getinteger(argv[i + i + 2]);
        }
    } else {        // an array of MMFLOAT or a string
        ptr = findvar(argv[2], V_EMPTY_OK | V_NOFIND_ERR);
        if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {      // string
            cptr = (unsigned char *)ptr;
            cptr++;                                                                 // skip the length byte in a MMBasic string
            for (i = 0; i < sendlen; i++) {
                I2C_Send_Buffer[i] = (int)(*(cptr + i));
            }
        } else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {   // numeric array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
                error("Insufficient data");
            } else {
                for (i = 0; i < sendlen; i++) {
                    I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
                }
            }
        } else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {   // integer array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
                error("Insufficient data");
            } else {
                for (i = 0; i < sendlen; i++) {
                    I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
                }
            }
        } else error("Invalid variable");
    }
    i2c_slave_send(sendlen);
}


// receive data from the I2C master - slave mode
void i2cSlaveReceive(char *p) {
    int rcvlen;
    void *floatptr, *intptr, *charptr, *rcvdlenFloat, *rcvdlenInt;
    getargs(&p, 5, ",");
    if(argc != 5) error("Argument count");
    if(!(I2C_Status & I2C_Status_Slave)) error("Slave not enabled");
    if(!(I2C_Status & I2C_Status_Slave_Receive)) error("Not in slave read state");
    rcvlen = getint(argv[0], 1, 255);
    charptr = floatptr = intptr = findvar(argv[2], V_EMPTY_OK | V_NOFIND_ERR);
      if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if(vartbl[VarIndex].type & T_NBR) {
          if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
          if(vartbl[VarIndex].dims[0] == 0) {                         // Not an array
              if(rcvlen != 1) error("Invalid variable");
          } else {      // An array
              if( (((MMFLOAT *)floatptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                  error("Invalid variable");
          }
          intptr = NULL;
      } else if(vartbl[VarIndex].type & T_INT) {
          if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
          if(vartbl[VarIndex].dims[0] == 0) {                         // Not an array
              if(rcvlen != 1) error("Invalid variable");
          } else {      // An array
              if( (((long long int *)floatptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                  error("Invalid variable");
          }
      } else if(vartbl[VarIndex].type & T_STR) {
          if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
          floatptr = intptr = NULL;
      } else
          error("Invalid variable");
    rcvdlenFloat = rcvdlenInt = findvar(argv[4], V_FIND | V_NOFIND_ERR);
      if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if(!(vartbl[VarIndex].type & T_NBR)) {
          rcvdlenFloat = NULL;
          if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable");
      }
    i2c_slave_receive(rcvlen, intptr, floatptr, charptr, rcvdlenFloat, rcvdlenInt);
}

#endif
#ifdef INCLUDE_I2C_SLAVE


/**************************************************************************************************
Enable the I2C1 module - slave mode
***************************************************************************************************/
void i2c_slave_enable(int options) {
    int i;
    if(I2C_Send_Buffer == NULL) I2C_Send_Buffer = GetMemory(255);
    if(options & 0x01) {                                            // respond to general call
        I2C1CONSET = _I2C1CON_GCEN_MASK;
    } else {
        I2C1CONCLR = _I2C1CON_GCEN_MASK;
    }
    if(options & 0x02) {                                            // 10 bit addressing
        I2C1CONSET = _I2C1CON_A10M_MASK;
    } else {
        I2C1CONCLR = _I2C1CON_A10M_MASK;
    }
    I2C1CONSET = _I2C1CON_STREN_MASK;                               // clock stretching enabled
    I2C_Slave_Sendlen = 0;                                          // set length of the slave send buffer to 0
    I2C1ADD = I2C_Slave_Addr;
    I2C1MSK = I2C_Slave_Mask;
    I2C_Rcv_Tail = I2C_Rcv_Head = 0;                                // reset receive buffer pointers
    I2C_Send_Index = 0;                                             // current index into I2C_Send_Buffer
    if(!(I2C_Status & I2C_Status_Slave)) {
        mI2C1SClearIntFlag();                                       // ensure slave flag is clear
        mI2C1SIntEnable(1);                                         // enable slave interrupt
        I2C_Status |= I2C_Status_Slave;
    }
    if(!(I2C_Status & I2C_Status_Enabled)) {
        I2C_Status |= I2C_Status_Enabled;
        ExtCfg(P_I2C_SDA, EXT_COM_RESERVED, 0);                     // clear BASIC interrupts and disable PIN and SETPIN
        ExtCfg(P_I2C_SCL, EXT_COM_RESERVED, 0);
        mI2C1SetIntPriority(5);                                     // priority 5
        I2C1CONSET = _I2C1CON_DISSLW_MASK;
        I2C1CONSET = _I2C1CON_I2CEN_MASK | _I2C1CON_STRICT_MASK;    // enable I2C1 module & strict addressing enforced
        mmI2Cvalue = 0;
    }
    for(i = 0; i < 256; i++) I2C_Rcv_Buffer[i] = 0;
}


/**************************************************************************************************
Disable the I2C1 module - slave mode
***************************************************************************************************/
void i2c_slave_disable() {
    FreeMemory(I2C_Send_Buffer);
    I2C_Send_Buffer = NULL;
    I2C1CONCLR = _I2C1CON_GCEN_MASK | _I2C1CON_A10M_MASK;
    I2C1ADD = 0;
    I2C1MSK = 0;
    I2C_Status &= 0xff00ffff;                                       // clear slave status flags
    I2C_Slave_Send_IntLine = NULL;                                  // clear send interrupt location
    I2C_Slave_Receive_IntLine = NULL;                               // clear receive interrupt location
    I2C_Slave_Sendlen = 0;                                          // set length of the slave send buffer to 0
    I2C_Slave_Addr = 0;                                             // slave address
    I2C_Slave_Mask = 0;                                             // slave address mask
    I2C_Rcv_Tail = I2C_Rcv_Head = 0;                                // reset receive buffer pointers
    I2C_Send_Index = 0;                                             // current index into I2C_Send_Buffer
    mI2C1SIntEnable(0);                                             // disable slave interrupt
    mI2C1SClearIntFlag();                                           // ensure slave flag is clear
    I2C1CONSET = _I2C1CON_SCLREL_MASK;                              // release the clock line
    if(!(I2C_Status & I2C_Status_Master)) {
        I2C1CONCLR = _I2C1CON_I2CEN_MASK;                           // disable I2C1 module
        I2C_Status = 0;                                             // clear status flags
    }
      if(ExtCurrentConfig[P_I2C_SDA] != EXT_COM_RESERVED) return;   // not open in the first place
    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);                           // set pins to unconfigured
    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
}


/**************************************************************************************************
Send data - slave mode
***************************************************************************************************/
void i2c_slave_send(int sendlen) {
    I2C1TRN = I2C_Send_Buffer[0];
    I2C_Status &= ~I2C_Status_Slave_Receive;
    I2C_Send_Index = 1;
    I2C_Slave_Sendlen = --sendlen;
    I2C1CONSET = _I2C1CON_SCLREL_MASK;
}


/**************************************************************************************************
Receive data - slave mode
***************************************************************************************************/
void i2c_slave_receive(int rcvlen, long long int *IntPtr, MMFLOAT *FloatPtr, char *CharPtr, MMFLOAT *rcvdlenFloat, long long int *rcvdlenInt) {
    unsigned int i, tail, tail_save;
    char *tp;
    tp = CharPtr++;
    tail = tail_save = I2C_Rcv_Tail;
    for (i = 0; i < rcvlen; i++) {
        if(tail != I2C_Rcv_Head) {
            if(FloatPtr != NULL)
                  FloatPtr[i] = I2C_Rcv_Buffer[tail];
              else if(IntPtr != NULL)
                  IntPtr[i] = I2C_Rcv_Buffer[tail];
              else
                  CharPtr[i] = I2C_Rcv_Buffer[tail];
            tail = (tail + 1) & 0x000000ff;
        } else {
            break;
        }
    }
    if(!(rcvdlenFloat == NULL))
          *rcvdlenFloat = i;
      else
          *rcvdlenInt = i;
      if(FloatPtr == NULL && IntPtr == NULL) *tp = i;
    if(i != 0) {
        mI2C1SIntEnable(0);                                         // disable slave interrupt
        if(tail_save == I2C_Rcv_Tail) {
            I2C_Rcv_Tail = tail;
            if(I2C_Status & I2C_Status_Slave_Receive_Full) {
                I2C_Status &= ~I2C_Status_Slave_Receive_Full;
                mI2C1SSetIntFlag();
            }
        }
        mI2C1SIntEnable(1);                                         // enable slave interrupt
    }
}

#endif

// enable the I2C1 module - master mode
void i2cEnable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400)) error("Valid speeds 100, 400");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C_Status & I2C_Status_Slave) error("I2C Slave already OPEN");
	if(I2C_Status & I2C_Status_Enabled)error("I2C already OPEN");
	//if(I2C_enabled) error("I2C already OPEN");
	I2C_Timeout = timeout;
	i2c_enable(speed);

}
// enable the I2C1 module - master mode
void i2c2Enable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400)) error("Valid speeds 100, 400");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C2_Status & I2C_Status_Slave) error("I2C Slave already OPEN");
	if(I2C2_Status & I2C_Status_Enabled)error("I2C already OPEN");
	//if(I2C2_enabled) error("I2C already OPEN");
	I2C2_Timeout = timeout;
	i2c2_enable(speed);

}


// disable the I2C1 module - master mode
void i2cDisable(char *p) {
	i2c_disable();
}

// disable the I2C1 module - master mode
void i2c2Disable(char *p) {
	i2c2_disable();
}

// send data to an I2C slave - master mode
void i2cSend(char *p) {
	int addr, i2c_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 3) error("Number out of bounds");
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C_Sendlen = sendlen;
	I2C_Rcvlen = 0;

	i2c_masterCommand(1);
}
// send data to an I2C slave - master mode
void i2c2Send(char *p) {
	int addr, i2c2_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getinteger(argv[2]);
	if(i2c2_options < 0 || i2c2_options > 3) error("Number out of bounds");
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C2_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C2_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C2_Sendlen = sendlen;
	I2C2_Rcvlen = 0;

	i2c2_masterCommand(1);
}

void i2cCheck(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)addr, 2, 10);
}
void i2c2Check(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)addr, 2, 10);
}
// receive data from an I2C slave - master mode
void i2cReceive(char *p) {
	int addr, i2c_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 3) error("Number out of bounds");
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Int = (long long int *)ptr;
        I2C_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C_Rcvbuf_String = (char *)ptr + 1;
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C_Rcvlen = rcvlen;

	I2C_Sendlen = 0;

	i2c_masterCommand(1);
}
// receive data from an I2C slave - master mode
void i2c2Receive(char *p) {
	int addr, i2c2_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getinteger(argv[2]);
	if(i2c2_options < 0 || i2c2_options > 3) error("Number out of bounds");
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Int = (long long int *)ptr;
        I2C2_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C2_Rcvbuf_String = (char *)ptr + 1;
        I2C2_Rcvbuf_Float = NULL;
        I2C2_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C2_Rcvlen = rcvlen;

	I2C2_Sendlen = 0;

	i2c2_masterCommand(1);
}

/**************************************************************************************************
Enable the I2C1 module - master mode
***************************************************************************************************/
void i2c_enable(int bps) {
    HAL_I2C_DeInit(&hi2c1);
    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C_SDA, TRISSET);
    if(PinRead(P_I2C_SDA) == 0) {
    	int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C_SCL, TRISCLR);
        PinSetBit(P_I2C_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
            uSec(25);
            PinSetBit(P_I2C_SCL, LATINV);
            if(PinRead(P_I2C_SDA) == 0) break;
        }
        PinSetBit(P_I2C_SCL, TRISSET);
    }
    ExtCfg(P_I2C_SDA, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C_SCL, EXT_COM_RESERVED, 0);
    if(bps==100) hi2c1.Init.ClockSpeed = 100000;
    if(bps==400) hi2c1.Init.ClockSpeed = 400000;

    HAL_I2C_Init(&hi2c1);
	I2C_enabled=1;
	I2C_Status |= I2C_Status_Enabled;
	I2C_Status |= I2C_Status_Master;
}
void i2c2_enable(int bps) {
    HAL_I2C_DeInit(&hi2c2);
    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C2_SDA, TRISSET);
    if(PinRead(P_I2C2_SDA) == 0) {
        int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C2_SCL, TRISCLR);
        PinSetBit(P_I2C2_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
           uSec(25);
           PinSetBit(P_I2C2_SCL, LATINV);
           if(PinRead(P_I2C2_SDA) == 0) break;
        }
        PinSetBit(P_I2C2_SCL, TRISSET);
    }
    ExtCfg(P_I2C2_SDA, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C2_SCL, EXT_COM_RESERVED, 0);
    if(bps==100) hi2c2.Init.ClockSpeed = 100000;
    if(bps==400) hi2c2.Init.ClockSpeed = 400000;
    HAL_I2C_Init(&hi2c2);
	I2C2_enabled=1;
	I2C2_Status |= I2C_Status_Enabled;
	I2C2_Status |= I2C_Status_Master;
}

/**************************************************************************************************
Disable the I2C1 module - master mode
***************************************************************************************************/
void i2c_disable() {
    if(!(I2C_Status & I2C_Status_Slave)) {
      //I2C_Status = I2C_Status_Disable;
	  I2C_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
      I2C_Rcvbuf_Float = NULL;
      I2C_Rcvbuf_Int = NULL;
	  I2C_Sendlen = 0;												// length of the master send buffer
	  I2C_Rcvlen = 0;													// length of the master receive buffer
	  I2C_Addr = 0;													// I2C device address
	  I2C_Timeout = 0;												// master timeout value
	  HAL_I2C_DeInit(&hi2c1);
	  I2C_enabled = 0;
	  I2C_Status = 0;
	  //I2C_Status &= ~I2C_Status_Enabled;
	  //I2C_Status &= ~I2C_Status_Master;
	  ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
      ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
	}
}
void i2c2_disable() {
  if(!(I2C2_Status & I2C_Status_Slave)) {
	  //I2C2_Status = I2C_Status_Disable;
	 I2C2_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
     I2C2_Rcvbuf_Float = NULL;
     I2C2_Rcvbuf_Int = NULL;
	 I2C2_Sendlen = 0;												// length of the master send buffer
	 I2C2_Rcvlen = 0;													// length of the master receive buffer
	 I2C2_Addr = 0;													// I2C device address
	 I2C2_Timeout = 0;												// master timeout value
	 HAL_I2C_DeInit(&hi2c2);
	 I2C2_enabled = 0;
	 I2C2_Status = 0;
	// I2C2_Status &= ~I2C_Status_Enabled;
	// I2C2_Status &= ~I2C_Status_Master;
     ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
     ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
  }
}
/**************************************************************************************************
Send and/or Receive data - master mode
***************************************************************************************************/
void i2c_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2caddr=I2C_Addr<<1,I2C_Rcv_Buffer[256];
	if(I2C_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)i2caddr, I2C_Send_Buffer, I2C_Sendlen, I2C_Timeout);
	}
	if(I2C_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, (uint16_t)i2caddr, (uint8_t *)I2C_Rcv_Buffer, I2C_Rcvlen, I2C_Timeout);
				for(i=0;i<I2C_Rcvlen;i++){
					if(I2C_Rcvbuf_String!=NULL){
						*I2C_Rcvbuf_String=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_String++;
					}
					if(I2C_Rcvbuf_Float!=NULL){
						*I2C_Rcvbuf_Float=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_Float++;
					}
					if(I2C_Rcvbuf_Int!=NULL){
						*I2C_Rcvbuf_Int=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_Int++;
					}
				}
	}
}

void i2c2_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2c2addr=I2C2_Addr<<1,I2C2_Rcv_Buffer[256];
	if(I2C2_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)i2c2addr, I2C2_Send_Buffer, I2C2_Sendlen, I2C2_Timeout);
	}
	if(I2C2_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, (uint16_t)i2c2addr, (uint8_t *)I2C2_Rcv_Buffer, I2C2_Rcvlen, I2C2_Timeout);
				for(i=0;i<I2C2_Rcvlen;i++){
					if(I2C2_Rcvbuf_String!=NULL){
						*I2C2_Rcvbuf_String=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_String++;
					}
					if(I2C2_Rcvbuf_Float!=NULL){
						*I2C2_Rcvbuf_Float=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_Float++;
					}
					if(I2C2_Rcvbuf_Int!=NULL){
						*I2C2_Rcvbuf_Int=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_Int++;
					}
				}
	}
}

void fun_mmi2c(void) {
	iret = mmI2Cvalue;
    targ = T_INT;
}

