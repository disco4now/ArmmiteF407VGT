/*******************************************************************************************
 *
 *  Definitions used when calling MMBasic Interpreter API Functions from CFunctions
 *  For Armmite F4 MMBasic V5.07.02
 *
 *  This file is public domain and may be used without license.
 *
 *	V1.5
 *	V1.6  Added parenthesis around (BaseAddress+0xx) expressions to force correct evaluation
 *	      Option Macro now -->  #define Option (*(struct option_s *)(unsigned int)Vector_Option)
 *        in struct_s changed PIN and Baudrate to unsigned int -->  unsigned int PIN;
 *                                                                  unsigned int Baudrate;
 *        Added #define NOP()  __asm volatile ("nop")
 *        USERLCDPANEL    19 Added
 *  V1.7  unsigned char FLASH_CS;  added to unsigned char FLASH_CS;  char dummy[2]; reduced from char dummy[3];
 *  V1.8  added      #define Interrupt 	(*(unsigned int *) Vector_CSubComplete) and #define Vector_CSubComplete	 (*(unsigned int *)(BaseAddress+0xA8))       // CSubComplete
 *        USER Panel now 10. i.e. #define USERLCDPANEL            10
 *  v1.9  Added Function Keys for Armmite F407xGT
 ******************************************************************************************/
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXDIM              5                       // maximum nbr of dimensions to an array
#define MMFLOAT double


//Addresses in the API Table for the pointers to each function
#define BaseAddress   0x8000390 
#define Vector_uSec               (*(int*)(BaseAddress+0x00))       // void uSec(unsigned int us)
#define Vector_putConsole         (*(int*)(BaseAddress+0x04))       // void putConsole(int c)
#define Vector_getConsole         (*(int*)(BaseAddress+0x08))       // int getConsole(void)
#define Vector_ExtCfg             (*(int*)(BaseAddress+0x0c))       // void ExtCfg(int pin, int cfg, int option)
#define Vector_ExtSet             (*(int*)(BaseAddress+0x10))       // void ExtSet(int pin, int val)
#define Vector_ExtInp             (*(int*)(BaseAddress+0x14))      // int ExtInp(int pin)
#define Vector_PinSetBit          (*(int*)(BaseAddress+0x18))      // void PinSetBit(int pin, unsigned int offset)
#define Vector_PinRead            (*(int*)(BaseAddress+0x1c))      // int PinRead(int pin)
#define Vector_MMPrintString      (*(int*)(BaseAddress+0x20))      // void MMPrintString(char* s)
#define Vector_IntToStr           (*(int*)(BaseAddress+0x24))      // void IntToStr(char *strr, long long int nbr, unsigned int base)
#define Vector_CheckAbort         (*(int*)(BaseAddress+0x28))       // void CheckAbort(void)
#define Vector_GetMemory          (*(int*)(BaseAddress+0x2c))       // void *GetMemory(size_t msize);
#define Vector_GetTempMemory      (*(int*)(BaseAddress+0x30))      // void *GetTempMemory(int NbrBytes)
#define Vector_FreeMemory         (*(int*)(BaseAddress+0x34))      // void FreeMemory(void *addr)
#define Vector_DrawRectangle      *(unsigned int *)(int*)(BaseAddress+0x38)          // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_DrawBitmap         *(unsigned int *)(int*)(BaseAddress+0x3c)         // void DrawBitmap(int x1, int y1, int width, int height, int scale, int fg, int bg, unsigned char *bitmap )
#define Vector_DrawLine           (*(int*)(BaseAddress+0x40))      // void DrawLine(int x1, int y1, int x2, int y2, int w, int c)
#define Vector_FontTable          (*(int*)(BaseAddress+0x44))      // const unsigned char *FontTable[FONT_NBR]
#define Vector_ExtCurrentConfig   (*(int*)(BaseAddress+0x48))       // int ExtCurrentConfig[NBRPINS + 1];
#define Vector_HRes               (*(int*)(BaseAddress+0x4c))       // HRes
#define Vector_VRes               (*(int*)(BaseAddress+0x50))     // VRes
#define Vector_SoftReset          (*(int*)(BaseAddress+0x54))      // void SoftReset(void)
#define Vector_error              (*(int*)(BaseAddress+0x58))       // void error(char *msg)
#define Vector_ProgFlash          (*(int*)(BaseAddress+0x5c))      // ProgFlash
#define Vector_vartbl             (*(int*)(BaseAddress+0x60))       // vartbl
#define Vector_varcnt             (*(int*)(BaseAddress+0x64))      // varcnt
#define Vector_DrawBuffer         *(unsigned int *)(int*)(BaseAddress+0x68)       // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_ReadBuffer         *(unsigned int *)(int*)(BaseAddress+0x6c)          // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_FloatToStr         (*(int*)(BaseAddress+0x70))     // convert a float to a string including scientific notation if necessary
#define Vector_ExecuteProgram     (*(int*)(BaseAddress+0x74))       // void ExecuteProgram(char *fname)
#define Vector_CFuncmSec          (*(int*)(BaseAddress+0x78))       // CFuncmSec
#define Vector_CFuncRam           (*(int*)(BaseAddress+0x7c))       // StartOfCFuncRam
#define Vector_ScrollLCD          *(unsigned int *)(int*)(BaseAddress+0x80)        // void scrollLCD(int lines, int blank)
#define Vector_IntToFloat         (*(int*)(BaseAddress+0x84))       	// MMFLOAT IntToFloat(long long int a)
#define Vector_FloatToInt         (*(int*)(BaseAddress+0x88))       	// long long int FloatToInt64(MMFLOAT x)
#define Vector_Option             (*(int*)(BaseAddress+0x8c))       	// Option
#define Vector_CFuncInt1          (*(int*)(BaseAddress+0x90))       // CFuncInt1
#define Vector_CFuncInt2          (*(int*)(BaseAddress+0x94))        // CFuncInt2
#define Vector_Sine               (*(int*)(BaseAddress+0x98))      	// MMFLOAT sin(MMFLOAT)
#define Vector_DrawCircle         (*(int*)(BaseAddress+0x9c))      	// DrawCircle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect)
#define Vector_DrawTriangle       (*(int*)(BaseAddress+0xa0))     	// DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill)
#define Vector_Timer   			  (*(int*)(BaseAddress+0xa4))      	// uint64_t timer(void)
#define Vector_CSubComplete		  (*(unsigned int *)(BaseAddress+0xa8))       // CSubComplete
#define Vector_CFuncInt3          (*(int*)(BaseAddress+0xAC))       // CFuncInt1
#define Vector_CFuncInt4          (*(int*)(BaseAddress+0xB0))        // CFuncInt2
//Macros to call each function.

#define uSec(a)                         ((void (*)(unsigned int )) Vector_uSec) (a)
#define putConsole(a)                   ((void (*)(char)) Vector_putConsole) (a)
#define getConsole()                    ((int (*)(void)) Vector_getConsole) ()
#define ExtCfg(a,b,c)                   ((void (*)(int, int, int)) Vector_ExtCfg) (a,b,c)
#define ExtSet(a,b)                     ((void(*)(int, int)) Vector_ExtSet) (a,b)
#define ExtInp(a)                       ((int(*)(int)) Vector_ExtInp) (a)
#define PinSetBit(a,b)                  ((void(*)(int, int)) Vector_PinSetBit) (a,b)
#define PinRead(a)                      ((int(*)(int)) Vector_PinRead) (a)
#define MMPrintString(a)                ((void (*)(char*)) Vector_MMPrintString) (a)
#define IntToStr(a,b,c)                 ((void (*)(char *, long long int, unsigned int)) Vector_IntToStr) (a,b,c)
#define CheckAbort()                    ((void (*)(void)) Vector_CheckAbort) ()
#define GetMemory(a)                    ((void* (*)(int)) Vector_GetMemory) (a)
#define GetTempMemory(a)                ((void* (*)(int)) Vector_GetTempMemory) (a)
#define FreeMemory(a)                   ((void (*)(void *)) Vector_FreeMemory) (a)
#define DrawRectangle(a,b,c,d,e)        ((void (*)(int,int,int,int,int)) (*(unsigned int *)Vector_DrawRectangle)) (a,b,c,d,e)
#define DrawRectangleVector             (*(unsigned int *)Vector_DrawRectangle)
#define DrawBitmap(a,b,c,d,e,f,g,h)     ((void (*)(int,int,int,int,int,int,int, char*)) (*(unsigned int *)Vector_DrawBitmap)) (a,b,c,d,e,f,g,h)
#define DrawBitmapVector                (*(unsigned int *)Vector_DrawBitmap)
#define DrawLine(a,b,c,d,e,f)           ((void (*)(int,int,int,int,int,int)) Vector_DrawLine) (a,b,c,d,e,f)
#define FontTable                       (void*)((int*)(Vector_FontTable))
#define ExtCurrentConfig                ((int *) Vector_ExtCurrentConfig)
#define HRes                            (*(unsigned int *) Vector_HRes)
#define VRes                            (*(unsigned int *) Vector_VRes)
#define SoftReset()                     ((void (*)(void)) Vector_SoftReset) ()
#define error(a)                        ((void (*)(char *)) Vector_error) (a)
#define ProgFlash                       ((int *) Vector_ProgFlash)
#define vartbl                          (*(struct s_vartbl *) Vector_vartbl)
#define varcnt                          (*(unsigned int *) Vector_varcnt)
#define DrawBuffer(a,b,c,d,e)           ((void (*)(int,int,int,int,char *)) (*(unsigned int *)Vector_DrawBuffer)) (a,b,c,d,e)
#define DrawBufferVector                (*(unsigned int *)Vector_DrawBuffer)
#define ReadBuffer(a,b,c,d,e)           ((void (*)(int,int,int,int,char *)) (*(unsigned int *)Vector_ReadBuffer)) (a,b,c,d,e)
#define ReadBufferVector                (*(unsigned int *)Vector_ReadBuffer)
#define FloatToStr(a,b,c,d,e)           ((void (*)(char *, MMFLOAT, int, int, char)) Vector_FloatToStr) (a,b,c,d,e)
// NOTE:  The argument to RunBasicSub is a string specifying the name of the BASIC subroutine to be executed.
//        It MUST be terminated with TWO null chars.
#define RunBasicSub(a)                  ((void (*)(char *)) Vector_ExecuteProgram) (a)
#define CFuncmSec                       (*(unsigned int *) Vector_CFuncmSec)
#define CFuncRam                        ((int *) Vector_CFuncRam)
#define ScrollLCD(a,b)                  ((void (*)(int, int)) (*(unsigned int *)Vector_ScrollLCD)) (a, b)

#define DrawPixel(a,b,c)                ((void(*)(int, int, int)) Vector_DrawPixel) (a,b,c)
#define IntToFloat(a)                   ((MMFLOAT (*)(long long)) Vector_IntToFloat) (a)
#define FloatToInt(a)                   ((long long (*)(MMFLOAT)) Vector_FloatToInt) (a)
//#define Option                          ({struct option_s *optionstructurepointer; optionstructurepointer=(void *)(unsigned int)Vector_Option;})
#define Option                          (*(struct option_s *)(unsigned int)Vector_Option)
//#define vartbl                          (*(struct s_vartbl *) Vector_vartbl)
#define uSecTimer                       ((unsigned long long (*)(void)) Vector_Timer)
#define CFuncInt1                       (*(unsigned int *) Vector_CFuncInt1)
#define CFuncInt2                       (*(unsigned int *) Vector_CFuncInt2)
#define Interrupt                    	(*(unsigned int *) Vector_CSubComplete)
#define CFuncInt3                       (*(unsigned int *) Vector_CFuncInt3)
#define CFuncInt4                       (*(unsigned int *) Vector_CFuncInt4)

#define FastTimer                       ((unsigned long long  (*)(void)) Vector_FastTimer)

#define Sine(a)                         ((MMFLOAT (*)(MMFLOAT)) Vector_Sine) (a)
#define DrawCircle(a,b,c,d,e,f,g)       ((void (*)(int,int,int,int,int,int,MMFLOAT)) Vector_DrawCircle) (a,b,c,d,e,f,g)
#define DrawTriangle(a,b,c,d,e,f,g,h)   ((void (*)(int,int,int,int,int,int,int,int)) Vector_DrawTriangle) (a,b,c,d,e,f,g,h)


// the structure of the variable table, passed to the CFunction as a pointer Vector_vartbl which is #defined as vartbl
struct s_vartbl {                               // structure of the variable table
    char name[MAXVARLEN];                       // variable's name
    char type;                                  // its type (T_NUM, T_INT or T_STR)
    char level;                                 // its subroutine or function level (used to track local variables)
    unsigned char size;                         // the number of chars to allocate for each element in a string array
    char dummy;
    int __attribute__ ((aligned (4))) dims[MAXDIM];                     // the dimensions. it is an array if the first dimension is NOT zero
    union u_val{
        MMFLOAT f;                              // the value if it is a float
        long long int i;                        // the value if it is an integer
        MMFLOAT *fa;                            // pointer to the allocated memory if it is an array of floats
        long long int *ia;                      // pointer to the allocated memory if it is an array of integers
        char *s;                                // pointer to the allocated memory if it is a string
    }  __attribute__ ((aligned (8))) val;
} __attribute__ ((aligned (8))) val;

//  Useful macros


// Types used to define a variable in the variable table (vartbl).   Often they are ORed together.
// Also used in tokens and arguments to functions
#define T_NOTYPE       0                            // type not set or discovered
#define T_NBR       0x01                            // number (or float) type
#define T_STR       0x02                            // string type
#define T_INT       0x04                            // 64 bit integer type
#define T_PTR       0x08                            // the variable points to another variable's data
#define T_IMPLIED   0x10                            // the variables type does not have to be specified with a suffix
#define T_CONST     0x20                            // the contents of this variable cannot be changed


//***************************************************************************************************
// Constants and definitions copied from the Micromite MkII and Micromite Plus source
//***************************************************************************************************

//The Option structure
struct option_s {
    char Autorun;              //0
    char Tab;
    char Restart;
    char Listcase;
    char Height;

    char Width;                //5
    char  ColourCode;          //6
    char DISPLAY_TYPE;
    char DISPLAY_ORIENTATION;
    unsigned char TOUCH_CS;
    unsigned char TOUCH_IRQ;   //10
    char TOUCH_SWAPXY;         //11
    //for the SPI LCDs
    char LCD_CD;               //12
    char LCD_CS;               //13
    char LCD_Reset;            //14
    char SerialConDisabled;    //15
    char SSDspeed;             //16
    char DISPLAY_CONSOLE;      //17
    char DefaultFont;          //18
    char KeyboardConfig;       //19
    unsigned char TOUCH_Click; //20
    char DefaultBrightness;    //21 default backlight brightness
    char SerialPullup;         //22
    char fulltime;             //23
    char Refresh;              //24
    unsigned char FLASH_CS;    //25
    unsigned char NoScroll;    //26
    char dummy;                //27
    short MaxCtrls;            // 28-29 maximum number of controls allowed
    short RTC_Calibrate;       //30-31
    int DISPLAY_WIDTH;         //32-35
    int DISPLAY_HEIGHT;        //36-39
    unsigned int PIN;          //40-43
    unsigned int Baudrate;     //44-47
    MMFLOAT TOUCH_XSCALE;      //48-55 8 Bytes
    MMFLOAT TOUCH_YSCALE;      //56-63 8 bytes
    unsigned int ProgFlashSize;//64-67 used to store the size of the program flash (also start of the LIBRARY code)
    int DefaultFC, DefaultBC;  //68-71 the default colours
    short  TOUCH_XZERO;        //72-73
    short  TOUCH_YZERO;        //74-75
    unsigned int spare;        // 76-79  i.e 4 bytes
#ifdef VGT
    unsigned char F1key[MAXKEYLEN]; //80+64 =144
    unsigned char F5key[MAXKEYLEN]; //144+64=208
    unsigned char F6key[MAXKEYLEN]; //206+64=270
    unsigned char F7key[MAXKEYLEN]; //270+64=334
    unsigned char F8key[MAXKEYLEN]; //334+64=398
    unsigned char F9key[MAXKEYLEN]; //398+64=462
#endif
};



// Define the offsets from the PORT address
// these are used by GetPortAddr(a,b)
#define ANSEL               -8
#define ANSELCLR            -7
#define ANSELSET            -6
#define ANSELINV            -5
#define TRIS                -4
#define TRISCLR             -3
#define TRISSET             -2
#define TRISINV             -1
#define PORT                0
#define PORTCLR             1
#define PORTSET             2
#define PORTINV             3
#define LAT                 4
#define LATCLR              5
#define LATSET              6
#define LATINV              7
#define ODC                 8
#define ODCCLR              9
#define ODCSET              10
#define ODCINV              11
#define CNPU                12
#define CNPUCLR             13
#define CNPUSET             14
#define CNPUINV             15
#define CNPD                16
#define CNPDCLR             17
#define CNPDSET             18
#define CNPDINV             19
#define CNCON               20
#define CNCONCLR            21
#define CNCONSET            22
#define CNCONINV            23
#define CNEN                24
#define CNENCLR             25
#define CNENSET             26
#define CNENINV             27
#define CNSTAT              28
#define CNSTATCLR           29
#define CNSTATSET           30
#define CNSTATINV           31

// configurations for an I/O pin
// these are used by ExtCfg(a,b,c)
#define EXT_NOT_CONFIG          0
#define EXT_ANA_IN				1
#define EXT_DIG_IN				2
#define EXT_FREQ_IN				3
#define EXT_PER_IN				4
#define EXT_CNT_IN				5
#define EXT_INT_HI				6
#define EXT_INT_LO				7
#define EXT_DIG_OUT				8
#define EXT_OC_OUT				9
#define EXT_INT_BOTH			10
#define EXT_COM_RESERVED        100                 // this pin is reserved and SETPIN and PIN cannot be used
#define EXT_BOOT_RESERVED       101                 // this pin is reserved at bootup and cannot be used
#define EXT_DS18B20_RESERVED    102                 // this pin is reserved for DS18B20 and cannot be used


#define NOP()                   __asm volatile ("nop")
#define USERLCDPANEL            10
