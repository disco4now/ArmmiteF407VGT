/*-*****************************************************************************

MMBasic  for STM32F407VET6 (Armmite F4)

Version.h

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

//#define DEBUGMODE

#define VERSION         "5.07.02b2"
#define MES_SIGNON  "ARMmite VGT6 MMBasic Version " VERSION
#define YEAR		"2011-2023"			    // and the year
#define YEAR2          "2016-2023"
#define COPYRIGHT  "\r\nCopyright " YEAR " Geoff Graham\r\nCopyright " YEAR2 " Peter Mather\r\n"

// Beta 2 SPIClose() added before AppendLibrary incase left open by flash write fail
//        multiline comments
//        fix for potential FILES error from peter.

// These options are compiled conditionally


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // debugging options
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // #define DEBUGMODE                     // enable debugging macros (with reduced program memory for the Micromite)

 // VET6 MINI with ILI9486_16 FLASH_CS =77
 //#define TEST_CONFIG "OPTION LCDPANEL DISABLE : OPTION LCDPANEL ILI9486_16, LANDSCAPE : OPTION TOUCH PB12, PC5 : GUI CALIBRATE 0, 4012, 3900, -1271, -868 : BACKLIGHT 80,S : OPTION FLASH_CS 77 : OPTION SAVE : ? \"ILI9486_16 ON\""

 // VET6 ILI9341 SPI  SERIAL CONSOLE 38400
 // #define TEST_CONFIG "OPTION LCDPANEL SSD1963_5, LANDSCAPE, 48, 6 : OPTION TOUCH 1, 40, 39 : OPTION SDCARD 47 : GUI CALIBRATE 1, 108, 3849, 2050, -1342 : OPTION SAVE : ? \"SSD1963_5 ON\""

