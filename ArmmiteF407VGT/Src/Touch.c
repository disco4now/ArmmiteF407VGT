/*-*****************************************************************************

ArmmiteF4 MMBasic

Touch.c

Does all the touch screen related I/O in MMBasic.


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
#define GenSPI hspi2
int GetTouchAxis2046(int cmd);
int (*GetTouchAxis)(int a) = (int (*)(int ))GetTouchAxis2046;
int GetTouchValue(int cmd);
void TDelay(void);

// these are defined so that the state of the touch PEN IRQ can be determined with the minimum of CPU cycles
extern SPI_HandleTypeDef GenSPI;

#define TOUCH_SPI_SPEED     3                                  // we run at 200KHz to minimise noise
int TOUCH_GETIRQTRIS=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// configure the touch parameters (chip select pin and the IRQ pin)
// this is called by the OPTION TOUCH command
void MIPS16 ConfigTouch(char *p) {
	int p1,p2,p3;
	char code;
	getargs(&p, 5, ",");
    if(!(argc == 3 || argc == 5)) error("Argument count");

	if((code=codecheck(argv[0])))argv[0]+=2;
	p1 = getinteger(argv[0]);
	if(code)p1=codemap(code, p1);
	if((code=codecheck(argv[2])))argv[2]+=2;
	p2 = getinteger(argv[2]);
	if(code)p2=codemap(code, p2);
    CheckPin(p1, CP_IGNORE_INUSE);
    CheckPin(p2, CP_IGNORE_INUSE);
    
    if(argc == 5) {
    	if((code=codecheck(argv[4])))argv[4]+=2;
    	p3 = getinteger(argv[4]);
    	if(code)p3=codemap(code, p3);
        Option.TOUCH_Click = p3;
    }

    Option.TOUCH_CS = p1;
    Option.TOUCH_IRQ = p2;

    Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup touch based on the settings saved in flash
void MIPS16 InitTouch(void) {
    if(!Option.TOUCH_CS) return;
    SetAndReserve(Option.TOUCH_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);    // config CS as an output
    SetAndReserve(Option.TOUCH_IRQ, P_INPUT, 0, EXT_BOOT_RESERVED);    // config IRQ as an input
    PinSetBit(Option.TOUCH_IRQ, CNPUSET);
    SetAndReserve(Option.TOUCH_Click, P_OUTPUT, 0, EXT_BOOT_RESERVED); // config the click pin as an output
    OpenSpiChannel();

    GetTouchValue(CMD_PENIRQ_ON);                                      // send the controller the command to turn on PenIRQ
    TOUCH_GETIRQTRIS = 1;
    
    GetTouchAxis(CMD_MEASURE_X);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this function is only used in calibration
// it draws the target, waits for the touch to stabilise and returns the x and y in raw touch controller numbers (ie, not scaled)
void MIPS16 GetCalibration(int x, int y, int *xval, int *yval) {
    int i, j, k;
    #define TCAL_FONT    0x02

	if(HRes == 0) error("Display not configured");
    ClearScreen(BLACK);
    GUIPrintString(HRes/2, VRes/2 - GetFontHeight(TCAL_FONT)/2, TCAL_FONT, JUSTIFY_CENTER, JUSTIFY_MIDDLE, 0, WHITE, BLACK, "Touch Target");
    GUIPrintString(HRes/2, VRes/2 + GetFontHeight(TCAL_FONT)/2, TCAL_FONT, JUSTIFY_CENTER, JUSTIFY_MIDDLE, 0, WHITE, BLACK, "and Hold");
    DrawLine(x - (TARGET_OFFSET * 3)/4, y, x + (TARGET_OFFSET * 3)/4, y, 1, WHITE);
    DrawLine(x, y - (TARGET_OFFSET * 3)/4, x, y + (TARGET_OFFSET * 3)/4, 1, WHITE);
    DrawCircle(x, y, TARGET_OFFSET/2, 1, WHITE, -1, 1);
myloop:
    while(!TOUCH_DOWN) CheckAbort();                                // wait for the touch
    for(i = j = 0; i < 50; i++) {                                   // throw away the first 50 reads as rubbish
        GetTouchAxis(CMD_MEASURE_X); GetTouchAxis(CMD_MEASURE_Y);
        if(!TOUCH_DOWN)goto myloop;
        uSec(2000);
    }

    // make a lot of readings and average them
    for(i = j = k = 0; i < 20; i++){
    	if(!TOUCH_DOWN)goto myloop;
    	k += GetTouchAxis(CMD_MEASURE_Y);
    	uSec(2000);
    	if(!TOUCH_DOWN)goto myloop;
    	j += GetTouchAxis(CMD_MEASURE_X);
    	uSec(2000);
    }
    *xval = j/20;
    *yval = k/20;

    ClearScreen(BLACK);
    if(Option.Refresh)Display_Refresh();
    while(TOUCH_DOWN) CheckAbort();                                 // wait for the touch to be lifted
    uSec(25000);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this is the main function to call to get a touch reading
// if y is true the y reading will be returned, otherwise the x reading
// this function does noise reduction and scales the reading to pixels
// a return of TOUCH_ERROR means that the pen is not down
int GetTouch(int y) {
    int i;
    static int lastx, lasty;
    
    if(Option.TOUCH_CS == 0) error("Touch option not set");
    if(Option.TOUCH_XZERO == TOUCH_NOT_CALIBRATED) error("Touch not calibrated");
    do {
        if(PinRead(Option.TOUCH_IRQ)) return TOUCH_ERROR;
        uSec(50);
        if(PinRead(Option.TOUCH_IRQ)) return TOUCH_ERROR;
        if(y) {
            i = ((MMFLOAT)(GetTouchAxis(Option.TOUCH_SWAPXY? CMD_MEASURE_X:CMD_MEASURE_Y) - Option.TOUCH_YZERO) * Option.TOUCH_YSCALE);
            if(i < lasty - CAL_ERROR_MARGIN || i > lasty + CAL_ERROR_MARGIN) { lasty = i; i = -1; }
        } else {
            i = ((MMFLOAT)(GetTouchAxis(Option.TOUCH_SWAPXY? CMD_MEASURE_Y:CMD_MEASURE_X) - Option.TOUCH_XZERO) * Option.TOUCH_XSCALE);
            if(i < lastx - CAL_ERROR_MARGIN || i > lastx + CAL_ERROR_MARGIN) { lastx = i; i = -1; }
        }
    } while(i < 0 || i >= (y ? VRes : HRes));
    return i;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this will get a reading from a single axis
// the returned value is not scaled, it is the raw number produced by the touch controller
// it takes multiple readings, discards the outliers and returns the average of the medium values
int GetTouchAxis2046(int cmd) {
    int i, j, t, b[TOUCH_SAMPLES];

    TOUCH_GETIRQTRIS=0;
    PinSetBit(Option.TOUCH_IRQ, CNPDSET);                           // Set the PenIRQ to an output
//    HAL_GPIO_WritePin(PinDef[(uint8_t)Option.TOUCH_IRQ].sfr, PinDef[(uint8_t)Option.TOUCH_IRQ].bitnbr, GPIO_PIN_RESET);// Drive the PenIRQ low so the diode is not forward biased
//    TOUCH_GETIRQTRIS=0;
    GetTouchValue(cmd);
    // we take TOUCH_SAMPLES readings and sort them into descending order in buffer b[].
    for(i = 0; i < TOUCH_SAMPLES; i++) {
        b[i] = GetTouchValue(cmd);									// get the value
        for(j = i; j > 0; j--) {							        // and sort into position
            if(b[j - 1] < b[j]) {
                t = b[j - 1];
                b[j - 1] = b[j];
                b[j] = t;
            }
            else
                break;
        }
    }

    // we then discard the top TOUCH_DISCARD samples and the bottom TOUCH_DISCARD samples and add up the remainder
    for(j = 0, i = TOUCH_DISCARD; i < TOUCH_SAMPLES - TOUCH_DISCARD; i++) j += b[i];

    // and return the average
    i = j / (TOUCH_SAMPLES - (TOUCH_DISCARD * 2));
    GetTouchValue(CMD_PENIRQ_ON);                                   // send the command to turn PenIRQ on
    PinSetBit(Option.TOUCH_IRQ, CNPUSET);                           // Set the PenIRQ to an input
    TOUCH_GETIRQTRIS=1;
    return i;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this will get a single reading from the touch controller
//
// it assumes that PenIRQ line has been pulled low and that the SPI baudrate is correct
// this takes 260uS at 120MHz
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetTouchValue(int cmd) {
    int val;
    unsigned int lb, hb;
    SpiCsLow(Option.TOUCH_CS, TOUCH_SPI_SPEED);
    TDelay();
    HAL_SPI_TransmitReceive(&GenSPI,(uint8_t *)&cmd,(uint8_t *)&val,1,500);    //    SpiChnPutC(TOUCH_SPI_CHANNEL, cmd);                             // send the read command (also selects the axis)
    HAL_SPI_TransmitReceive(&GenSPI,(unsigned char[]){0},(uint8_t *)&hb,1,500);//    SpiChnPutC(TOUCH_SPI_CHANNEL, 0x00);                            // Send a dummy byte in order to clock the data out
	val = (hb & 0b1111111) << 5;         // the top 7 bits
    HAL_SPI_TransmitReceive(&GenSPI,(unsigned char[]){0},(uint8_t *)&lb,1,500);//    SpiChnPutC(TOUCH_SPI_CHANNEL, 0x00);                            // Send a dummy byte in order to clock the data out
    val |= (lb >> 3) & 0b11111;          // the bottom 5 bits
    SpiCsHigh(Option.TOUCH_CS);                                     // Deassert the CS line
    return val;
}


void TDelay(void)		// provides a small (~200ns) delay for the touch screen controller.
{
	shortpause(50);
}


// the MMBasic TOUCH() function
void fun_touch(void) {
    if(checkstring(ep, "X"))
        iret = GetTouch(GET_X_AXIS);
    else if(checkstring(ep, "Y"))
        iret = GetTouch(GET_Y_AXIS);
    else if(checkstring(ep, "REF"))
        iret = CurrentRef;
    else if(checkstring(ep, "LASTREF"))
        iret = LastRef;
    else if(checkstring(ep, "LASTX"))
        iret = LastX;
    else if(checkstring(ep, "LASTY"))
        iret = LastY;
    else if(checkstring(ep, "DOWN"))
        iret = TOUCH_DOWN;
    else if(checkstring(ep, "UP"))
        iret = !TOUCH_DOWN;
    else
        error("Invalid argument");

    targ = T_INT;
}

