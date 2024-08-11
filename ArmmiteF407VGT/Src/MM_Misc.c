/*-*****************************************************************************

ArmmiteF4 MMBasic

MM_Misc.c

Handles all the miscelaneous commands and functions in MMBasic.
These are commands and functions that do not comfortably fit anywhere else.

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
#include <time.h>
#include "cJSON.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

struct s_inttbl inttbl[NBRINTERRUPTS];
extern char *InterruptReturn;

int TickPeriod[NBRSETTICKS];
volatile int TickTimer[NBRSETTICKS];
char *TickInt[NBRSETTICKS];
volatile unsigned char TickActive[NBRSETTICKS];

char *OnKeyGOSUB = NULL;
const char *daystrings[] = {"dummy","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};

char EchoOption = true;
unsigned long long int __attribute__((section(".my_section"))) saved_variable;  //  __attribute__ ((persistent));  // and this is the address
extern const char *KBrdList[];

unsigned int CurrentCpuSpeed;
unsigned int PeripheralBusSpeed;
extern RTC_HandleTypeDef hrtc;
extern char *ADCInterrupt;
extern volatile int ADCcomplete;
extern int64_t *a1point, *a2point, *a3point;
extern unsigned char ADCbits[];
extern MMFLOAT *a1float, *a2float, *a3float;
extern int ADCmax;
extern volatile int ADCchannelA;
extern volatile int ADCchannelB;
extern volatile int ADCchannelC;
extern MMFLOAT ADCscale[3], ADCbottom[3];
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern char *LCDList[];
extern char LCDAttrib;
extern char Feather;
extern volatile BYTE SDCardStat;
extern volatile int keyboardseen;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint64_t TIM12count;
extern TIM_HandleTypeDef htim12;
MMFLOAT optionangle=1.0;

extern void WriteSSD1963Command(int cmd);
extern void WriteDataSSD1963(int data);


void  setterminal(int height,int width);
extern int terminal_width,terminal_height;


char *CSubInterrupt;
volatile int CSubComplete=0;

extern char *KeyInterrupt;
extern volatile int Keycomplete;
//extern int keyselect;
int SaveOptionErrorSkip=0;
int SaveMMerrno;           // save the error number
char SaveMMErrMsg[MAXERRMSG];  // save the error message



void integersort(int64_t *iarray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] < iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] > iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}
void floatsort(MMFLOAT *farray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] < farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] > farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}

void stringsort(unsigned char *sarray, int n, int offset, long long *index, int flags, int startpoint){
	int ii,i, s = 1,isave;
	int k;
	unsigned char *s1,*s2,*p1,*p2;
	unsigned char temp;
	int reverse= 1-((flags & 1)<<1);
    while (s){
      s=0;
      for(i=1;i<n;i++){
        s2=i*offset+sarray;
        s1=(i-1)*offset+sarray;
        ii = *s1 < *s2 ? *s1 : *s2; //get the smaller  length
        p1 = s1 + 1; p2 = s2 + 1;
        k=0; //assume the strings match
        while((ii--) && (k==0)) {
          if(flags & 2){
			  if(toupper(*p1) > toupper(*p2)){
				k=reverse; //earlier in the array is bigger
			  }
			  if(toupper(*p1) < toupper(*p2)){
				 k=-reverse; //later in the array is bigger
			  }
          } else {
			  if(*p1 > *p2){
				k=reverse; //earlier in the array is bigger
			  }
			  if(*p1 < *p2){
				 k=-reverse; //later in the array is bigger
			  }
          }
          p1++; p2++;
        }
      // if up to this point the strings match
      // make the decision based on which one is shorter
      if(k==0){
        if(*s1 > *s2) k=reverse;
        if(*s1 < *s2) k=-reverse;
      }
      if (k==1){ // if earlier is bigger swap them round
        ii = *s1 > *s2 ? *s1 : *s2; //get the bigger length
        ii++;
        p1=s1;p2=s2;
        while(ii--){
          temp=*p1;
          *p1=*p2;
          *p2=temp;
          p1++; p2++;
        }
        s=1;
        if(index!=NULL){
        	isave=index[i-1+startpoint];
        	index[i-1+startpoint]=index[i+startpoint];
        	index[i+startpoint]=isave;
        }
      }
    }
  }
}
void cmd_sort(void){
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL,*a4int=NULL;
    unsigned char *a3str=NULL;
    int i, size=0, truesize,flags=0, maxsize=0, startpoint=0;
	getargs(&cmdline,9,(char *)",");
    size=parseany(argv[0],&a3float,&a3int,&a3str,&maxsize,true)-1;
    truesize=size;
    if(argc>=3 && *argv[2]){
        int card=parseintegerarray(argv[2],&a4int,2,1,NULL,true)-1;
    	if(card !=size)error("Array size mismatch");
    }
    if(argc>=5 && *argv[4])flags=getint(argv[4],0,3);
    if(argc>=7 && *argv[6])startpoint=getint(argv[6],OptionBase,size+OptionBase);
    size-=startpoint;
    if(argc==9)size=getint(argv[8],1,size+1+OptionBase)-1;
    if(startpoint)startpoint-=OptionBase;
    if(a3float!=NULL){
    	a3float+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	floatsort(a3float, size+1, a4int, flags, startpoint);
    } else if(a3int!=NULL){
    	a3int+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	integersort(a3int,  size+1, a4int, flags, startpoint);
    } else if(a3str!=NULL){
    	a3str+=((startpoint)*(maxsize+1));
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	stringsort(a3str,  size+1,maxsize+1, a4int, flags, startpoint);
    }
}
/*
void fun_format(void) {
	char *p, *fmt;
	int inspec;
	getargs(&ep, 3, ",");
	if(argc%2 == 0) error("Invalid syntax");
	if(argc == 3)
		fmt = getCstring(argv[2]);
	else
		fmt = "%g";

	// check the format string for errors that might crash the CPU
	for(inspec = 0, p = fmt; *p; p++) {
		if(*p == '%') {
			inspec++;
			if(inspec > 1) error("Only one format specifier (%) allowed");
			continue;
		}

		if(inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' || *p == 'E'|| *p == 'l'))
			inspec++;


		if(inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' || *p == '.' || *p == ' '))
			error("Illegal character in format specification");
	}
	if(inspec != 2) error("Format specification not found");
	sret = GetTempMemory(STRINGSIZE);									// this will last for the life of the command
	sprintf(sret, fmt, getnumber(argv[0]));
	CtoM(sret);
	targ=T_STR;
}
*/

// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
void cmd_timer(void) {
	int64_t fasttimer;
    while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
    if(!*cmdline) error("Syntax");
    fasttimer = (uint64_t)(getnumber(++cmdline)*1000.0);
    if(fasttimer<0.0)error("Syntax");
    __HAL_TIM_DISABLE(&htim12);
    TIM12count=fasttimer>>16;
    __HAL_TIM_SET_COUNTER(&htim12,fasttimer & 0xFFFF);
    __HAL_TIM_ENABLE(&htim12);

}

// this is invoked as a function
void fun_timer(void) {
    __HAL_TIM_DISABLE(&htim12);
    uint64_t fasttimer=(TIM12count<<16) | __HAL_TIM_GET_COUNTER(&htim12);
    __HAL_TIM_ENABLE(&htim12);
    fret=((MMFLOAT)fasttimer/1000);
    targ = T_NBR;
}
void fun_datetime(void){
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    if(checkstring(ep, "NOW")){
        RtcGetTime();                                   // disable the timer interrupt to prevent any conflicts while updating
        IntToStrPad(sret, day, '0', 2, 10);
        sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
        sret[5] = '-'; IntToStr(sret + 6, year, 10);
        sret[10] = ' ';
        IntToStrPad(sret+11, hour, '0', 2, 10);
        sret[13] = ':'; IntToStrPad(sret + 14, minute, '0', 2, 10);
        sret[16] = ':'; IntToStrPad(sret + 17, second, '0', 2, 10);
    } else {
        struct tm  *tm;
        struct tm tma;
        tm=&tma;
        time_t timestamp = getinteger(ep);                       /* See README.md if your system lacks timegm(). */
        //time_t timestamp = getint(ep, 0x80000000, 0x7FFFFFFF); /* See README.md if your system lacks timegm(). */
        tm=gmtime(&timestamp);
        IntToStrPad(sret, tm->tm_mday, '0', 2, 10);
        sret[2] = '-'; IntToStrPad(sret + 3, tm->tm_mon+1, '0', 2, 10);
        sret[5] = '-'; IntToStr(sret + 6, tm->tm_year+1900, 10);
        sret[10] = ' ';
        IntToStrPad(sret+11, tm->tm_hour, '0', 2, 10);
        sret[13] = ':'; IntToStrPad(sret + 14, tm->tm_min, '0', 2, 10);
        sret[16] = ':'; IntToStrPad(sret + 17, tm->tm_sec, '0', 2, 10);
    }
    CtoM(sret);
    targ = T_STR;
}

void fun_epoch(void){
    char *arg;
    struct tm  *tm;
    struct tm tma;
    tm=&tma;
    int d, m, y, h, min, s;
    if(!checkstring(ep, "NOW"))
    {
        arg = getCstring(ep);
        getargs(&arg, 11, "-/ :");                                      // this is a macro and must be the first executable stmt in a block
        if(!(argc == 11)) error("Syntax");
            d = atoi(argv[0]);
            m = atoi(argv[2]);
            y = atoi(argv[4]);
			if(d>1000){
				int tmp=d;
				d=y;
				y=tmp;
			}
            if(y >= 0 && y < 100) y += 2000;
            if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
            h = atoi(argv[6]);
            min  = atoi(argv[8]);
            s = atoi(argv[10]);
            if(h < 0 || h > 23 || min < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
            day = d;
            month = m;
            year = y;
            tm->tm_year = y - 1900;
            tm->tm_mon = m - 1;
            tm->tm_mday = d;
            tm->tm_hour = h;
            tm->tm_min = min;
            tm->tm_sec = s;
    } else {
        RtcGetTime();                                   // disable the timer interrupt to prevent any conflicts while updating
        tm->tm_year = year - 1900;
        tm->tm_mon = month - 1;
        tm->tm_mday = day;
        tm->tm_hour = hour;
        tm->tm_min = minute;
        tm->tm_sec = second;
    }
        time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
        iret=timestamp;
        targ = T_INT;
}

void cmd_pause(void) {
    static int interrupted = false;
    MMFLOAT f;

    f = getnumber(cmdline);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    if(f < 0.05) return;

    if(f < 1.5) {
        uSec(f * 1000);                                             // if less than 1.5mS do the pause right now
        return;                                                     // and exit straight away
    }

    if(InterruptReturn == NULL) {
        // we are running pause in a normal program
        // first check if we have reentered (from an interrupt) and only zero the timer if we have NOT been interrupted.
        // This means an interrupted pause will resume from where it was when interrupted
        if(!interrupted) PauseTimer = 0;
        interrupted = false;

        while(PauseTimer < FloatToInt32(f)) {
            CheckAbort();
            if(check_interrupt()) {
                // if there is an interrupt fake the return point to the start of this stmt
                // and return immediately to the program processor so that it can send us off
                // to the interrupt routine.  When the interrupt routine finishes we should reexecute
                // this stmt and because the variable interrupted is static we can see that we need to
                // resume pausing rather than start a new pause time.
                while(*cmdline && *cmdline != cmdtoken) cmdline--;  // step back to find the command token
                InterruptReturn = cmdline;                          // point to it
                interrupted = true;                                 // show that this stmt was interrupted
                return;                                             // and let the interrupt run
            }
        }
        interrupted = false;
    }
    else {
        // we are running pause in an interrupt, this is much simpler but note that
        // we use a different timer from the main pause code (above)
        IntPauseTimer = 0;
        while(IntPauseTimer < FloatToInt32(f)) CheckAbort();
    }
}

void MIPS16 cmd_longString(void){
    char *tp;
    tp = checkstring(cmdline, (char *)"SETBYTE");
    if(tp){
        int64_t *dest=NULL;
        int p=0;
        uint8_t *q=NULL;
        int nbr;
        int j=0;
    	getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true)-1)*8-1;
        q=(uint8_t *)&dest[1];
        p = getint(argv[2],OptionBase,j-OptionBase);
        nbr=getint(argv[4],0,255);
        q[p-OptionBase]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"APPEND");
    if(tp){
        int64_t *dest=NULL;
        char *p= NULL;
        char *q= NULL;
        int i,j,nbr;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        q+=dest[0];
        p=(char *)getstring(argv[2]);
        nbr = i = *p++;
        if(j*8 < dest[0]+i)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"TRIM");
    if(tp){
        int64_t *dest=NULL;
        uint32_t trim;
        char *p, *q=NULL;
        int i;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        trim=getint(argv[2],1,dest[0]);
        i = dest[0]-trim;
        p=q+trim;
        while(i--)*q++=*p++;
        dest[0]-=trim;
        return;
    }
    tp = checkstring(cmdline, (char *)"REPLACE");
    if(tp){
        int64_t *dest=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        p=(char *)getstring(argv[2]);
        nbr=getint(argv[4],1,dest[0]-*p+1);
        q+=nbr-1;
        i = *p++;
        while(i--)*q++=*p++;
        return;
    }
    tp = checkstring(cmdline, (char *)"LOAD");
    if(tp){
        int64_t *dest=NULL;
        char *p;
        char *q=NULL;
        int i,j;
        getargs(&tp, 5, ( char *)",");
        if(argc != 5)error("Argument count");
        int64_t nbr=getinteger(argv[2]);
        i=nbr;
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        dest[0]=0;
        p=(char *)getstring(argv[4]);
        if(nbr> *p)nbr=*p;
        p++;
        if(j*8 < dest[0]+nbr)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"LEFT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0])nbr=i=src[0];
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"RIGHT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0]){
            nbr=i=src[0];
        } else p+=(src[0]-nbr);
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"MID");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr,start;
        getargs(&tp, 7,(char *)",");
        if(argc < 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        start=getint(argv[4],1,src[0]);
        if(argc==7)nbr=getinteger(argv[6]);
        else nbr=src[0];
        p+=start-1;
        if(nbr+start>src[0]){
            nbr=src[0]-start+1;
        }
        i=nbr;
        if(j*8 < nbr)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"CLEAR");
    if(tp){
        int64_t *dest=NULL;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        dest[0]=0;
        return;
    }
    tp = checkstring(cmdline, (char *)"RESIZE");
    if(tp){
        int64_t *dest=NULL;
        int j=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true)-1)*8;
        dest[0] = getint(argv[2], 0, j);
        return;
    }
    tp = checkstring(cmdline, (char *)"UCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
        if(*q >= 'a' && *q <= 'z')
            *q -= 0x20;
        q++;
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"PRINT");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int j, fnbr=0;
        getargs(&tp, 3, (char *)",");
        if(argc == 3){
            if(*argv[0] == '#')argv[0]++;                                 // check if the first arg is a file number
            fnbr = getinteger(argv[0]);                                 // get the number
            parseintegerarray(argv[2],&dest,2,1,NULL,true);
        } else {
            parseintegerarray(argv[0],&dest,1,1,NULL,true);
        }
        q=(char *)&dest[1];
        j=dest[0];
        while(j--){
            MMfputc(*q++, fnbr);
        }
        MMfputs((char *)"\2\r\n", fnbr);
        return;
    }
    tp = checkstring(cmdline, (char *)"LCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
            if(*q >= 'A' && *q <= 'Z')
                *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"COPY");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        dest[0]=0;
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        if((j-i)*8 < src[0])error("Destination array too small");
        i=src[0];
        while(i--)*q++=*p++;
        dest[0]=src[0];
        return;
    }
    tp = checkstring(cmdline, (char *)"CONCAT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j,d=0,s=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        d=dest[0];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        i = s = src[0];
        if(j*8 < (d+s))error("Destination array too small");
        q+=d;
        while(i--)*q++=*p++;
        dest[0]+=src[0];
        return;
    }
    error("Invalid option");
}
void fun_LGetStr(void){
        char *p;
        char *s=NULL;
        int64_t *src=NULL;
        int start,nbr,j;
        getargs(&ep, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1)*8;
        start = getint(argv[2],1,j);
        nbr = getinteger(argv[4]);
        if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
        if(start+nbr>src[0])nbr=src[0]-start+1;
        sret = GetTempMemory(STRINGSIZE);                                       // this will last for the life of the command
        s=(char *)&src[1];
        s+=(start-1);
        p=(char *)sret+1;
        *sret=nbr;
        while(nbr--)*p++=*s++;
        *p=0;
        targ = T_STR;
}

void fun_LGetByte(void){
        uint8_t *s=NULL;
        int64_t *src=NULL;
        int start,j;
    	getargs(&ep, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1)*8;
        s=(uint8_t *)&src[1];
        start = getint(argv[2],OptionBase,j-OptionBase);
        iret=s[start-OptionBase];
        targ = T_INT;
}


void MIPS16 fun_LInstr(void){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 5, ",");
        if(argc <3  || argc > 5)error("Argument count");
        int64_t start;
        if(argc==5)start=getinteger(argv[4])-1;
        else start=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        slen=*srch;
        iret=0;
        if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
        if(!found){
            n=dest[0]- slen - start;

            for(i = start; i <= n + start; i++) {
                if(str[i + 8] == srch[1]) {
                    for(j = 0; j < slen; j++)
                        if(str[j + i + 8] != srch[j + 1])
                            break;
                    if(j == slen) {iret= i + 1; break;}
                }
            }
        }
        targ = T_INT;
}

void fun_LCompare(void){
    int64_t *dest, *src;
    char *p=NULL;
    char *q=NULL;
    int d=0,s=0,found=0;
    getargs(&ep, 3, (char *)",");
    if(argc != 3)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false);
    q=(char *)&dest[1];
    d=dest[0];
    parseintegerarray(argv[2],&src,1,1,NULL,false);
    p=(char *)&src[1];
    s=src[0];
    while(!found) {
        if(d == 0 && s == 0) {found=1;iret=0;}
        if(d == 0 && !found) {found=1;iret=-1;}
        if(s == 0 && !found) {found=1;iret=1;}
        if(*q < *p && !found) {found=1;iret=-1;}
        if(*q > *p && !found) {found=1;iret=1;}
        q++;  p++;  d--; s--;
    }
    targ = T_INT;
}

void fun_LLen(void) {
    int64_t *dest=NULL;
    getargs(&ep, 1, (char *)",");
    if(argc != 1)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false);
    iret=dest[0];
    targ = T_INT;
}




void MIPS16 update_clock(void){
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        error("RTC hardware error");
    }
    sDate.WeekDay = day_of_week;
    sDate.Month = month;
    sDate.Date = day;
    sDate.Year = year-2000;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        error("RTC hardware error");
    }
}


// this is invoked as a command (ie, date$ = "6/7/2010")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void MIPS16 cmd_date(void) {
	char *arg;
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	int dd, mm, yy;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	arg = getCstring(cmdline);
	{
		getargs(&arg, 5, "-/");										// this is a macro and must be the first executable stmt in a block
		if(argc != 5) error("Syntax");
		dd = atoi(argv[0]);
		mm = atoi(argv[2]);
		yy = atoi(argv[4]);
		if(yy >= 0 && yy < 100) yy += 2000;
	    //check year
	    if(yy>=1900 && yy<=9999)
	    {
	        //check month
	        if(mm>=1 && mm<=12)
	        {
	            //check days
	            if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12))
	                {}
	            else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11))
	                {}
	            else if((dd>=1 && dd<=28) && (mm==2))
	                {}
	            else if(dd==29 && mm==2 && (yy%400==0 ||(yy%4==0 && yy%100!=0)))
	                {}
	            else
	                error("Day is invalid");
	        }
	        else
	        {
	            error("Month is not valid");
	        }
	    }
	    else
	    {
	        error("Year is not valid");
	    }

		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		day = dd;
		month = mm;
		year = yy;
	    tm->tm_year = year - 1900;
	    tm->tm_mon = month - 1;
	    tm->tm_mday = day;
	    tm->tm_hour = hour;
	    tm->tm_min = minute;
	    tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    tm=gmtime(&timestamp);
	    day_of_week=tm->tm_wday;
	    if(day_of_week==0)day_of_week=7;
		update_clock();
		mT4IntEnable(1);       										// enable interrupt
	}
}

// this is invoked as a function
void fun_date(void) {
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    RtcGetTime();                                   // disable the timer interrupt to prevent any conflicts while updating
    IntToStrPad(sret, day, '0', 2, 10);
    sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
    sret[5] = '-'; IntToStr(sret + 6, year, 10);
    CtoM(sret);
    targ = T_STR;
}

// this is invoked as a function
void MIPS16 fun_day(void) {
    char *arg;
    struct tm  *tm;
    struct tm tma;
    tm=&tma;
    time_t time_of_day;
    int i;
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    int d, m, y;
    if(!checkstring(ep, "NOW"))
    {
        arg = getCstring(ep);
        getargs(&arg, 5, "-/");                                     // this is a macro and must be the first executable stmt in a block
        if(!(argc == 5))error("Syntax");
        d = atoi(argv[0]);
        m = atoi(argv[2]);
        y = atoi(argv[4]);
		if(d>1000){
			int tmp=d;
			d=y;
			y=tmp;
		}
        if(y >= 0 && y < 100) y += 2000;
        if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
        tm->tm_year = y - 1900;
        tm->tm_mon = m - 1;
        tm->tm_mday = d;
        tm->tm_hour = 0;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        time_of_day = timegm(tm);
        tm=gmtime(&time_of_day);
        i=tm->tm_wday;
        if(i==0)i=7;
        strcpy(sret,daystrings[i]);
    } else {
        RtcGetTime();                                   // disable the timer interrupt to prevent any conflicts while updating
        strcpy(sret,daystrings[day_of_week]);
    }
    CtoM(sret);
    targ = T_STR;
}

// this is invoked as a command (ie, time$ = "6:10:45")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void MIPS16 cmd_time(void) {
	char *arg;
	int h = 0;
	int m = 0;
	int s = 0;
    MMFLOAT f;
    long long int i64;
    char *ss;
    int t=0;
    int offset;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
    evaluate(cmdline, &f, &i64, &ss, &t, false);
	if(t==T_STR){
	arg = getCstring(cmdline);
	{
		getargs(&arg, 5, ":");								// this is a macro and must be the first executable stmt in a block
		if(argc%2 == 0) error("Syntax");
		h = atoi(argv[0]);
		if(argc >= 3) m = atoi(argv[2]);
		if(argc == 5) s = atoi(argv[4]);
		if(h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		hour = h;
		minute = m;
		second = s;
		SecondsTimer = 0;
		update_clock();
    	mT4IntEnable(1);       										// enable interrupt
    }
	} else {
		struct tm  *tm;
		struct tm tma;
		tm=&tma;
		offset=getinteger(cmdline);
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		tm->tm_year = year - 1900;
		tm->tm_mon = month - 1;
		tm->tm_mday = day;
		tm->tm_hour = hour;
		tm->tm_min = minute;
		tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    timestamp+=offset;
	    tm=gmtime(&timestamp);
		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		hour = tm->tm_hour;
		minute = tm->tm_min;
		second = tm->tm_sec;
		SecondsTimer = 0;
		update_clock();
    	mT4IntEnable(1);       										// enable interrupt
	}
}




// this is invoked as a function
void fun_time(void) {
    sret = GetTempStrMemory();                                  // this will last for the life of the command
    RtcGetTime();                                   // disable the timer interrupt to prevent any conflicts while updating
    IntToStrPad(sret, hour, '0', 2, 10);
    sret[2] = ':'; IntToStrPad(sret + 3, minute, '0', 2, 10);
    sret[5] = ':'; IntToStrPad(sret + 6, second, '0', 2, 10);
    if(Option.fulltime){
        sret[8] = '.'; IntToStrPad(sret + 9, milliseconds, '0', 3, 10);
    }
    CtoM(sret);
    targ = T_STR;
}



void cmd_ireturn(void){
  if(InterruptReturn == NULL) error("Not in interrupt");
  checkend(cmdline);
  nextstmt = InterruptReturn;
  InterruptReturn = NULL;
  if(LocalIndex)    ClearVars(LocalIndex--);                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
    *CurrentInterruptName = 0;                                        // for static vars we are not in an interrupt
    if(DelayedDrawKeyboard) {
        DelayedDrawKeyboard = false;
        DrawKeyboard(1);                                            // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(DelayedDrawFmtBox) {
        DelayedDrawFmtBox = false;
        DrawFmtBox(1);                                              // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(SaveOptionErrorSkip>0)OptionErrorSkip=SaveOptionErrorSkip+1; //fix for error skip if interrupted
    strcpy( MMErrMsg, SaveMMErrMsg);   //restore saved error messages
    MMerrno=SaveMMerrno;              //restore saved MMerrno

}


// set up the tick interrupt
void cmd_settick(void){
	int period;
	int irq=0;;
    getargs(&cmdline, 5, ",");
    if(!(argc == 3 || argc == 5)) error("Argument count");
    period = getint(argv[0], 0, INT_MAX);
    if(argc == 5) irq = getint(argv[4], 1, NBRSETTICKS) - 1;
    if(strcasecmp(argv[0],"PAUSE")==0){
            TickActive[irq]=0;
            return;
        } else if(strcasecmp(argv[0],"RESUME")==0){
            TickActive[irq]=1;
            return;
        } else period = getint(argv[0], -1, INT_MAX);
        if(period == 0) {
            TickInt[irq] = NULL;                                        // turn off the interrupt
        TickPeriod[irq] = 0;
        TickTimer[irq] = 0;                                         // set the timer running
        TickActive[irq]=0;
        } else {
            TickPeriod[irq] = period;
            TickInt[irq] = GetIntAddress(argv[2]);                      // get a pointer to the interrupt routine
            TickTimer[irq] = 0;                                         // set the timer running
            InterruptUsed = true;
            TickActive[irq]=1;
        }
}


/*
void setterminal(void){

	// if(Option.DISPLAY_CONSOLE)return;
	  char sp[20]={0};
	  strcpy(sp,"\033[8;");
	  IntToStr(&sp[strlen(sp)],Option.Height,10);
	  strcat(sp,";");
	  IntToStr(&sp[strlen(sp)],Option.Width+1,10);
	  strcat(sp,"t");
	  //SSPrintString(sp);						//
	  SerUSBPutS(sp);
}
*/

void  setterminal(int height,int width){
	  char sp[20]={0};
	  strcpy(sp,"\033[8;");
	  IntToStr(&sp[strlen(sp)],height,10);
	  strcat(sp,";");
	  IntToStr(&sp[strlen(sp)],width+1,10);
	  strcat(sp,"t");
	  SerUSBPutS(sp);					//
}





void cmd_option(void) {
    char *tp;

    tp = checkstring(cmdline, "BASE");
    if(tp) {
        if(DimUsed) error("Must be before DIM or LOCAL");
        OptionBase = getint(tp, 0, 1);
        return;
    }

    tp = checkstring(cmdline, "ESCAPE");
    	    if(tp) {
    	        OptionEscape = true;
    	        return;
   }

    tp = checkstring(cmdline, "EXPLICIT");
    if(tp) {
//        if(varcnt != 0) error("Variables already defined");
        OptionExplicit = true;
        return;
    }
	tp = checkstring(cmdline, "ANGLE");
	if(tp) {
		if(checkstring(tp, "DEGREES"))	{ optionangle=RADCONV; return; }
		if(checkstring(tp, "RADIANS"))	{ optionangle=1.0; return; }
	}

    tp = checkstring(cmdline, "DEFAULT");
    if(tp) {
        if(checkstring(tp, "INTEGER"))  { DefaultType = T_INT;  return; }
        if(checkstring(tp, "FLOAT"))    { DefaultType = T_NBR;  return; }
        if(checkstring(tp, "STRING"))   { DefaultType = T_STR;  return; }
        if(checkstring(tp, "NONE"))     { DefaultType = T_NOTYPE;   return; }
    }

    tp = checkstring(cmdline, "BREAK");
    if(tp) {
        BreakKey = getinteger(tp);
        return;
    }
    tp = checkstring(cmdline, "MILLISECONDS");
    if(tp) {
        if(checkstring(tp, "ON"))       { Option.fulltime = true; return; }
        if(checkstring(tp, "OFF"))      { Option.fulltime = false; return;  }
    }

    tp = checkstring(cmdline, "AUTORUN");
    if(tp) {
        if(checkstring(tp, "ON"))       { Option.Autorun = true; SaveOptions(); return; }
        if(checkstring(tp, "OFF"))      { Option.Autorun = false; SaveOptions(); return;  }
    }

    tp = checkstring(cmdline, "CASE");
    if(tp) {
        if(checkstring(tp, "LOWER"))    { Option.Listcase = CONFIG_LOWER; SaveOptions(); return; }
        if(checkstring(tp, "UPPER"))    { Option.Listcase = CONFIG_UPPER; SaveOptions(); return; }
        if(checkstring(tp, "TITLE"))    { Option.Listcase = CONFIG_TITLE; SaveOptions(); return; }
    }

    tp = checkstring(cmdline, "TAB");
    if(tp) {
        if(checkstring(tp, "2"))        { Option.Tab = 2; SaveOptions(); return; }
		if(checkstring(tp, "3"))		{ Option.Tab = 3; SaveOptions(); return; }
        if(checkstring(tp, "4"))        { Option.Tab = 4; SaveOptions(); return; }
        if(checkstring(tp, "8"))        { Option.Tab = 8; SaveOptions(); return; }
    }
    tp = checkstring(cmdline, "BAUDRATE");
    if(tp) {
        if(Option.SerialConDisabled!=0)error("Invalid with USB console");
        //if(CurrentLinePtr) error("Invalid in a program");                    //G.A.
        int i;
        i = getinteger(tp);
        if(i > PeripheralBusSpeed/17) error("Baud rate too high");
        if(i < 100) error("Number out of bounds");
        Option.Baudrate = i;
        if(!CurrentLinePtr)SaveOptions();    // G.A.
        if(!CurrentLinePtr)MMPrintString("Restart to activate");                // set the console baud rate G.A.
        return;
    }
    tp = checkstring(cmdline, "VCC");
    if(tp) {
        MMFLOAT f;
        f = getnumber(tp);
        if(f > 3.6) error("VCC too high");
        if(f < 1.8) error("VCC too low");
        VCC=f;
        return;
    }

    tp = checkstring(cmdline, "PIN");
    if(tp) {
        int i;
        i = getint(tp, 0, 99999999);
        Option.PIN = i;
        SaveOptions();
        return;
    }

    tp = checkstring(cmdline, "DISPLAY");
    if(tp) {
            getargs(&tp, 3, (char *)",");
            if(Option.DISPLAY_CONSOLE && argc>1 ) error("Cannot change LCD console");
            if(argc >= 1) Option.Height = getint(argv[0], 5, 100);
            if(argc == 3) Option.Width = getint(argv[2], 37, 240);
            if (Option.DISPLAY_CONSOLE) {
               setterminal((Option.Height > SCREENHEIGHT)?Option.Height:SCREENHEIGHT,(Option.Width > SCREENWIDTH)?Option.Width:SCREENWIDTH);                                                    // or height is > 24
            }else{
              setterminal(Option.Height,Option.Width);
            }
            if(argc >= 1 )SaveOptions();  //Only save if necessary
            return;
    }
    /*
    if(tp) {
        getargs(&tp, 3, ",");
        if(Option.DISPLAY_CONSOLE) error("Cannot change LCD console");
        Option.Height = getint(argv[0], 5, 100);

        if(argc == 3) Option.Width = getint(argv[2], 37, 240);


       // terminal_height=Option.Height;
       // terminal_width=Option.Width;
        setterminal();


        SaveOptions();
        return;
    }
    */

    tp = checkstring(cmdline, "LCDPANEL");
    if(tp) {
        getargs(&tp, 13, ",");
        if(str_equal(argv[0], "USER")) {
            if(Option.DISPLAY_TYPE) error("Display already configured");
            if(argc != 5) error("Argument count");
            HRes = DisplayHRes = getint(argv[2], 1, 10000);
            VRes = DisplayVRes = getint(argv[4], 1, 10000);
            Option.DISPLAY_TYPE = USER;
            // setup the pointers to the drawing primitives
            DrawRectangle = DrawRectangleUser;
            DrawBitmap = DrawBitmapUser;
            return;
        }
    }
    tp = checkstring(cmdline, "COLOURCODE");
    if(tp == NULL) tp = checkstring(cmdline, "COLORCODE");
    if(tp) {
        if(checkstring(tp, "ON"))       { Option.ColourCode = true; SaveOptions(); return; }
        if(checkstring(tp, "OFF"))      { Option.ColourCode = false; SaveOptions(); return;  }
    }



    tp = checkstring(cmdline, "SAVE");
    if(tp) {
        if(!CurrentLinePtr) return;

        //if(SaveOptions()) {
        if(Option.Restart==false){
        	 Option.Restart=true;
        	 SaveOptions();
            // Restart on first pass
            _excep_code = RESTART_DOAUTORUN;
            SoftReset();                            // this will restart the processor
        }
        return;
    }

    OtherOptions();
}



// function (which looks like a pre defined variable) to return the type of platform
void fun_device(void){
    sret = GetTempStrMemory();                                  // this will last for the life of the command
    strcpy(sret, "ARMmite F407xGT");
    CtoM(sret);
    targ = T_STR;
}

void fun_info(void){
        char *tp;
        sret = GetTempStrMemory();                                  // this will last for the life of the command
        tp=checkstring(ep, "FONT POINTER");
    	if(tp){
    		iret=(int64_t)((uint32_t)&FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
    		targ=T_INT;
    		return;
    	}
        tp=checkstring(ep, "FONT ADDRESS");
    		if(tp){
    		iret=(int64_t)((uint32_t)FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
    		targ=T_INT;
    		return;
    	}
        //********** OPTION option ****************************
    	tp=checkstring(ep, "OPTION");
    	if(tp){
            if(checkstring(tp, "AUTORUN")){
            	 if(Option.Autorun == false)strcpy(sret,"Off");
            	 else strcpy(sret,"On");

            }else if(checkstring(tp, "EXPLICIT")){
    			if(OptionExplicit == false)strcpy(sret,"Off");
    			else strcpy(sret,"On");

    		}else if(checkstring(tp, "DEFAULT")){
    			if(DefaultType == T_INT)strcpy(sret,"Integer");
    			else if(DefaultType == T_NBR)strcpy(sret,"Float");
    			else if(DefaultType == T_STR)strcpy(sret,"String");
    			else strcpy(sret,"None");

    		 }else if(checkstring(tp, "BASE")){
    			if(OptionBase==1)iret=1;
    			else iret=0;
    			targ=T_INT;
    			return;
    		 }else if(checkstring(tp, "BREAK")){
    			iret=BreakKey;
    			targ=T_INT;
    			return;
			} else if(checkstring(tp, "ANGLE")){
				if(optionangle==1.0)strcpy(sret,"RADIANS");
				else strcpy(sret,"DEGREES");
	            CtoM(sret);
	            targ=T_STR;
	            return;
    		 } else if(checkstring(tp, "TOUCH_IRQ")){
    		 	iret=Option.TOUCH_IRQ;
    		 	targ=T_INT;
    		 	return;

    		 } else if(checkstring(tp, "FLASH_CS")){
    		 	 iret=Option.FLASH_CS;
    		 	 targ=T_INT;
    		 	 return;

    		 } else if(checkstring(tp, "VCC")){
    		     fret=VCC;
    		     targ=T_NBR;
    		     return;

    		 } else error("Syntax");
    		 CtoM(sret);
    	     targ=T_STR;
    		 return;
        }


     tp=checkstring(ep, "NBRPINS");
     if(tp){
     if(HAS_64PINS)iret=64;
     if(HAS_100PINS)iret=100;
     if(HAS_144PINS)iret=144;
    	targ=T_INT;
    	return;
     }

     tp=checkstring(ep, "PIN");
      if(tp){

        int pin;
        char code;
        if((code=codecheck(tp)))tp+=2;
        pin = getinteger(tp);
        if(code)pin=codemap(code, pin);
        if(IsInvalidPin(pin))strcpy(sret,"Invalid");
        else if(ExtCurrentConfig[pin] & CP_IGNORE_RESERVED)strcpy(sret,"Reserved");
        else if(ExtCurrentConfig[pin]) strcpy(sret,"In Use");
        else strcpy(sret,"Unused");

         CtoM(sret);
         targ=T_STR;
         return;
      }

      tp=checkstring(ep, "PINNO");
         if(tp){
             int pin;
             char code;
             if((code=codecheck(tp)))tp+=2;
            // if(!(code=codecheck(tp)))tp+=2;
            // else ("Syntax");
             pin = getinteger(tp);
             if(code)pin=codemap(code,pin);
             if(IsInvalidPin(pin))error("Invalid pin");
             iret=pin;
             targ=T_INT;
             return;
      }


     if(checkstring(ep, "CPUSPEED")){
            IntToStr(sret,SystemCoreClock,10);

     }else 	if (checkstring(ep, "LINE")) {
          if (!CurrentLinePtr) {
              strcpy(sret, "UNKNOWN");
          } else if(CurrentLinePtr < ProgMemory + Option.ProgFlashSize) {
        	 sprintf(sret, "%d", CountLines(CurrentLinePtr));
   	      } else {
   	     	strcpy(sret, "LIBRARY");
   	      }
 	      //CtoM(sret);
          // targ=T_STR;
   		  //  return;

     } else if((tp=checkstring(ep, "EXISTS DIR"))){
		char *p = getFstring(tp);
		targ=T_INT;
		iret=ExistsDir(p);
		return;
	} else if((tp=checkstring(ep, "EXISTS FILE"))){
		char *p = getFstring(tp);
		iret=ExistsFile(p);
		if(iret==0){  //return -2 if its a directory
		  iret=ExistsDir(p);
		  if (iret==1)iret=-2;
		}
		targ=T_INT;
		return;
     } else if(checkstring(ep, "AUTORUN")){
            if(Option.Autorun == false)strcpy(sret,"Off");
            else strcpy(sret,"On");


     } else if(checkstring(ep, "LCDPANEL")){
            strcpy(sret,LCDList[(int)Option.DISPLAY_TYPE]);
           // CtoM(sret);
           // targ=T_STR;

     } else if(checkstring(ep, "LCDPANELID")){
        	//iret= Option.SSDspeed;
        	iret=LCDAttrib;
        	targ=T_INT;
        	 return;
     } else if(checkstring(ep, "RESTART")){
             iret= Option.Restart;
             targ=T_INT;
             return;
    // } else if(checkstring(ep, "CURSORPOS")){
    //        iret= MMCharPos;
    //        targ=T_INT;
    //        return;
     } else if(checkstring(ep, "HEIGHT")){
        	iret= Option.Height;
        	targ=T_INT;
        	return;
     } else if(checkstring(ep, "WIDTH")){
        	iret= (int64_t)Option.Width;
        	targ=T_INT;
            return;
     } else if(checkstring(ep, "TOUCH")){
            if(Option.TOUCH_CS == false)strcpy(sret,"Disabled");
            else if(Option.TOUCH_XZERO == TOUCH_NOT_CALIBRATED)strcpy(sret,"Not calibrated");
            else strcpy(sret,"Ready");

     } else if(checkstring(ep, "CONSOLE")){
           	if(Option.DISPLAY_CONSOLE==false)strcpy(sret,"NOCONSOLE");
           	else strcpy(sret,"CONSOLE");


     } else if(checkstring(ep, "SDCARD")){
            int i=OptionFileErrorAbort;
            OptionFileErrorAbort=0;
            if(!InitSDCard())strcpy(sret,"Not present");
            else  strcpy(sret,"Ready");
            OptionFileErrorAbort=i;

     } else if(checkstring(ep, "FONTWIDTH")){
                iret = FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
                targ=T_INT;
                return;
     } else if(checkstring(ep, "FONTHEIGHT")){
                iret = FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
                targ=T_INT;
                return;
     } else if(checkstring(ep, "HPOS")){
                iret = CurrentX;
                targ=T_INT;
                return;
     } else if(checkstring(ep, "HEAP")){
                iret=FreeSpaceOnHeap();
                targ=T_INT;
                return;
     } else if(checkstring(ep, "VPOS")){
                iret = CurrentY;
                targ=T_INT;
                return;

     } else if(checkstring(ep, "ERRNO")){
                iret = MMerrno;
                targ=T_INT;
                return;
     } else if(checkstring(ep, "ERRMSG")){
                strcpy(sret, MMErrMsg);

     } else if(checkstring(ep, "FCOLOUR") || checkstring(ep, "FCOLOR") ){
                iret=gui_fcolour;
                targ=T_INT;
                return;
     } else if(checkstring(ep, "BCOLOUR") || checkstring(ep, "BCOLOR")){
                iret=gui_bcolour;
                targ=T_INT;
                return;
     } else if(checkstring(ep, "FONT")){
                iret=(gui_font >> 4)+1;
                targ=T_INT;
                return;
     } else if(checkstring(ep, "DEVICE")){
                fun_device();
                return;
     } else if(checkstring(ep, "DEVICETYPE")){
    	  if(Feather)strcpy(sret, "Feather");
    	    else strcpy(sret, "");
     } else if(checkstring(ep, "VERSION")){
                fun_version();
                return;
      } else if(checkstring(ep, "VARCNT")){
                iret=(int64_t)((uint32_t)varcnt);
                targ=T_INT;
                return;


      } else error("Syntax");

      CtoM(sret);
      targ=T_STR;
      return;

}

void cmd_watchdog(void) {
    int i;

    if(checkstring(cmdline, "OFF") != NULL) {
        WDTimer = 0;
    } else {
        i = getinteger(cmdline);
        if(i < 1) error("Invalid argument");
        WDTimer = i;
    }
}


void fun_restart(void) {
    iret = WatchdogSet;
    targ = T_INT;
}

static void SYSCLKConfig_STOP(void)
{
      RCC_ClkInitTypeDef RCC_ClkInitStruct;
      RCC_OscInitTypeDef RCC_OscInitStruct;

      /* Enable Power Control clock */
      __HAL_RCC_PWR_CLK_ENABLE();

      /* The voltage scaling allows optimizing the power consumption when the device is
         clocked below the maximum system frequency, to update the voltage scaling value
         regarding system frequency refer to product datasheet.  */
      __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
      uint32_t pFLatency = 0;

      /* Get the Oscillators configuration according to the internal RCC registers */
      HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

      /* After wake-up from STOP reconfigure the system clock: Enable HSE and PLL */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
      RCC_OscInitStruct.HSIState = RCC_HSI_ON;
      RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
      if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
        Error_Handler();
      }
      /* After wake-up from STOP reconfigure the system clock: Enable HSE and PLL */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
      RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
      RCC_OscInitStruct.HSEState = RCC_HSE_ON;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
      if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
        Error_Handler();
      }

      /* Get the Clocks configuration according to the internal RCC registers */
      HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

      /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
         clocks dividers */
      RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
      RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
      if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
      {
        Error_Handler();
      }
}
//void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
//{
//}

void MIPS16 cmd_cpu(void) {
    char *p;

//    while(!UARTTransmissionHasCompleted(UART1));                    // wait for the console UART to send whatever is in its buffer

    if((p = checkstring(cmdline, "RESTART"))) {
        PRet();
        _excep_code = RESET_COMMAND;
        while(ConsoleTxBufTail != ConsoleTxBufHead);
        uSec(10000);
        SoftReset();                                                // this will restart the processor ? only works when not in debug
    } else if((p = checkstring(cmdline, "SLEEP"))) {
        if(!(*p == 0 || *p =='\'')) {
        	int pullup=0;
        	GPIO_InitTypeDef GPIO_InitStruct = {0};
            int timetosleep;
            MMFLOAT totalseconds;
            getargs(&p, 3, ",");
            totalseconds=getnumber(p)-0.01;
            if(totalseconds<=(1.0))error("Invalid period");
            while(ConsoleTxBufTail != ConsoleTxBufHead);
            HAL_Delay(10);
            if(Option.SerialConDisabled){
				USBD_Stop(&hUsbDeviceFS);
				USBD_DeInit(&hUsbDeviceFS);
				USB_DevDisconnect(USB_OTG_FS);
				HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);
				GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
				GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
				HAL_Delay(100);
				pullup=GPIOA->IDR & GPIO_PIN_12;
				GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, RESET);
				HAL_Delay(400);
				HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
				totalseconds-=0.9;
            }
            while(totalseconds>0){
                if(totalseconds>30.0){
                    timetosleep=30*2048;
                    totalseconds-=30.0;
                } else {
                    timetosleep= (int)(totalseconds*2048.0);
                    totalseconds=0.0;
                }
                HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0F, 0);
                HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
                HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, timetosleep, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
                HAL_SuspendTick();
                HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
                HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
                HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);
            }
             SYSCLKConfig_STOP();
             HAL_ResumeTick();
             if(Option.SerialConDisabled){
				 MX_USB_DEVICE_Init();
				 if(!pullup)USB_DevConnect(USB_OTG_FS);
				 HAL_Delay(400);
             }
//             MMPrintString(">\r\n");
         } else {
             GPIO_InitTypeDef  GPIO_InitStruct;
             int pullup=0;
             while(ConsoleTxBufTail != ConsoleTxBufHead);
             uSec(10000);
             if(Option.SerialConDisabled){
 				USBD_Stop(&hUsbDeviceFS);
 				USBD_DeInit(&hUsbDeviceFS);
 				USB_DevDisconnect(USB_OTG_FS);
 				HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);
 				GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
 				GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
 				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
 				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 				HAL_Delay(100);
 				pullup=GPIOA->IDR & GPIO_PIN_12;
 				GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, RESET);
 				HAL_Delay(400);
 				HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
             }
               GPIO_InitStruct.Pin = GPIO_PIN_0;
             GPIO_InitStruct.Pull = GPIO_PULLDOWN;
             GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
             HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
             HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
             HAL_NVIC_EnableIRQ(EXTI0_IRQn);
             HAL_SuspendTick();
             HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
             SYSCLKConfig_STOP();
             HAL_ResumeTick();
             if(Option.SerialConDisabled){
				 MX_USB_DEVICE_Init();
				 if(!pullup)USB_DevConnect(USB_OTG_FS);
				 HAL_Delay(400);
             }
//             MMPrintString(">\r\n");
         }
    } else error("Syntax");
}

void cmd_csubinterrupt(void){
    getargs(&cmdline,1,",");
    if(argc != 0){
        if(checkstring(argv[0],"0")){
            CSubInterrupt = NULL;
            CSubComplete=0;
        } else {
            CSubInterrupt = GetIntAddress(argv[0]);
            CSubComplete=0;
            InterruptUsed = true;
        }
    } else CSubComplete=1;
}

void cmd_cfunction(void) {
    char *p, EndToken;
    if(cmdtoken == cmdCFUN)
            EndToken = GetCommandValue("End CFunction");                // this terminates a CFUNCTION
    else
      if(cmdtoken == cmdCSUB)
             EndToken = GetCommandValue("End CSub");                 // this terminates a CSUB
      else
             EndToken = GetCommandValue("End DefineFont");           // this terminates a DefineFont
    p = cmdline;
    while(*p != 0xff) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) error("Missing END declaration");               // end of the program
        if(*p == T_NEWLINE) p++;                                    // skip over the newline token
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == EndToken) {                                        // found an END token
            nextstmt = p;
            skipelement(nextstmt);
            return;
        }
        p++;
    }
}




// utility function used by cmd_poke() to validate an address
unsigned int GetPokeAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    if(!POKERANGE(i)) error("Address");
    return i;
}



void cmd_poke(void) {
    char *p;
    void *pp;


    char *q;
    if((p = checkstring(cmdline, "DISPLAY"))){
        if(!Option.DISPLAY_TYPE)error("Display not configured");
        if((q=checkstring(p,"HRES"))){
            HRes=getint(q,0,1920);
        } else if((q=checkstring(p,"VRES"))){
            VRes=getint(q,0,1200);

        } else {
            getargs(&p,(MAX_ARG_COUNT * 2) - 3,",");
            if(!argc)return;
            if((Option.DISPLAY_TYPE >= SSD_PANEL_START && Option.DISPLAY_TYPE <= SSD_PANEL_END) || (Option.DISPLAY_TYPE >= P16_PANEL_START &&  Option.DISPLAY_TYPE <= P16_PANEL_END )){

            	WriteSSD1963Command(getinteger(argv[0]));
                for(int i = 2; i < argc; i += 2) {
                   	WriteDataSSD1963(getinteger(argv[i]));
                }
                return;
            } else if(Option.DISPLAY_TYPE>=SPI_PANEL_START && Option.DISPLAY_TYPE<=SPI_PANEL_END){
                spi_write_command(getinteger(argv[0]));
                for(int i = 2; i < argc; i += 2) {
                    spi_write_data(getinteger(argv[i]));
                }
                return;
           // } else if(Option.DISPLAY_TYPE<=I2C_PANEL){
           //     if(argc>1)error("UNsupported command");
           //     I2C_Send_Command(getinteger(argv[0]));
            } else
            error("Display not supported");

        } //error("Syntax");
    } else {


    getargs(&cmdline, 5, ",");
    if((p = checkstring(argv[0], "BYTE"))) {
        if(argc != 3) error("Argument count");
        uint32_t a=GetPokeAddr(p);
        uint8_t *padd=(uint8_t *)(a);
        *padd = getinteger(argv[2]);
//        padd = (uint8_t *)((uint32_t)padd & 0xFFFFFFE0);
//        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }
    if((p = checkstring(argv[0], "SHORT"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 2)error("Address not divisible by 2");
    	uint16_t *padd=(uint16_t *)(a);
        *padd = getinteger(argv[2]);
 //       padd = (uint16_t *)((uint32_t)padd & 0xFFFFFFE0);
 //       SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "WORD"))) {
        if(argc != 3) error("Argument count");
        uint32_t a=GetPokeAddr(p);
        if(a % 4)error("Address not divisible by 4");
        uint32_t *padd=(uint32_t *)(a);
        *padd = getinteger(argv[2]);
//        padd = (uint32_t *)((uint32_t)padd & 0xFFFFFFE0);
//        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }


    if((p = checkstring(argv[0], "INTEGER"))) {
        if(argc != 3) error("Argument count");
        uint32_t a=GetPokeAddr(p);
        if(a % 8)error("Address not divisible by 8");
        uint64_t *padd=(uint64_t *)(a);
        *padd = getinteger(argv[2]);
//        padd = (uint64_t *)((uint32_t)padd & 0xFFFFFFE0);
//        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }



    if((p = checkstring(argv[0], "FLOAT"))) {
        if(argc != 3) error("Argument count");
        uint32_t a=GetPokeAddr(p);
        if(a % 8)error("Address not divisible by 8");
        MMFLOAT *padd=(MMFLOAT *)(a);
        *padd = getnumber(argv[2]);
//        padd = (MMFLOAT *)((uint32_t)padd & 0xFFFFFFE0);
//        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if(argc != 5) error("Argument count");

    if(checkstring(argv[0], "VARTBL")) {
        *((char *)vartbl + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    if((p = checkstring(argv[0], "VAR"))) {
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        *((char *)pp + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    // the default is the old syntax of:   POKE hiaddr, loaddr, byte
    *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2])) = getinteger(argv[4]);
}

}

// function to find a CFunction
// only used by fun_peek() below
unsigned int GetCFunAddr(int *ip, int i) {
    while(*ip != 0xffffffff) {
        if(*ip++ == (unsigned int)subfun[i]) {                      // if we have a match
            ip++;                                                   // step over the size word
            i = *ip++;                                              // get the offset
            return (unsigned int)(ip + i);                          // return the entry point
        }
        ip += (*ip + 4) / sizeof(unsigned int);
    }
    return 0;
}




// utility function used by fun_peek() to validate an address
unsigned int GetPeekAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    //if(!PEEKRANGE(i)) error("Address");
    return i;
}


// Will return a byte within the PIC32 virtual memory space.
void fun_peek(void) {
    char *p;
    void *pp;
    getargs(&ep, 3, ",");

    if((p = checkstring(argv[0], "BYTE"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned char *)GetPeekAddr(p);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "VARADDR"))){
        if(argc != 1) error("Syntax");
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = (unsigned int)pp;
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "CFUNADDR"))){
    	int i,j;
        if(argc != 1) error("Syntax");
        i = FindSubFun(p, true);                                    // search for a function first
        if(i == -1) i = FindSubFun(p, false);                       // and if not found try for a subroutine
        //if(i == -1 || !(*subfun[i] == cmdCSUB)) error("Invalid argument");
        if(i == -1 || !(*subfun[i] == cmdCFUN || *subfun[i] == cmdCSUB)) error("Invalid argument");
        // search through program flash and the library looking for a match to the function being called
        j = GetCFunAddr((int *)CFunctionFlash, i);
        //if(!j) error("Internal fault 6(sorry)");
        if(!j) j = GetCFunAddr((int *)CFunctionLibrary, i);    //Check the library
        if(!j) error("Internal fault 6(sorry)");
        iret = (unsigned int)j;                                     // return the entry point
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "WORD"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned int *)(GetPeekAddr(p) & 0b11111111111111111111111111111100);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "SHORT"))){
        if(argc != 1) error("Syntax");
       // iret = *(unsigned int *)(GetPeekAddr(p) & 0b11111111111111111111111111111110);
        iret = *(unsigned short *)(GetPeekAddr(p) & 0b11111111111111111111111111111110);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "INTEGER"))){
        if(argc != 1) error("Syntax");
        iret = *(uint64_t *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "FLOAT"))){
        if(argc != 1) error("Syntax");
        fret = *(MMFLOAT *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_NBR;
        return;
        }




    if(argc != 3) error("Syntax");

    if((checkstring(argv[0], "PROGMEM"))){
        iret = *((char *)ProgMemory + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((checkstring(argv[0], "VARTBL"))){
        iret = *((char *)vartbl + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "VAR"))){
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = *((char *)pp + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }


/*  not required. Just peek the registers directly.
    if((p = checkstring(argv[0], "OPTION"))){
    	       int i;
    	       i=getint(argv[2],0, 79);
    	       //j=i>>2;
               iret=HAL_RTCEx_BKUPRead (&hrtc,i>>2);
               i=i&3;
               iret=iret>>8*i;
               iret=iret&0xFF;
               targ = T_INT;
               return;
               }
*/
    // default action is the old syntax of  b = PEEK(hiaddr, loaddr)
    iret = *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2]));
    targ = T_INT;
}

// remove unnecessary text
void MIPS16 CrunchData(char **p, int c) {
    static char inquotes, lastch, incomment;

    if(c == '\n') c = '\r';                                         // CR is the end of line terminator
    if(c == 0  || c == '\r' ) {
        inquotes = false; incomment = false;                        // newline so reset our flags
        if(c) {
            if(lastch == '\r') return;                              // remove two newlines in a row (ie, empty lines)
            *((*p)++) = '\r';
        }
        lastch = '\r';
        return;
    }

    if(incomment) return;                                           // discard comments
    if(c == ' ' && lastch == '\r') return;                          // trim all spaces at the start of the line
    if(c == '"') inquotes = !inquotes;
    if(inquotes) {
        *((*p)++) = c;                                              // copy everything within quotes
        return;
    }
    if(c == '\'') {                                                 // skip everything following a comment
        incomment = true;
        return;
    }
    if(c == ' ' && (lastch == ' ' || lastch == ',')) {
        lastch = ' ';
        return;                                                     // remove more than one space or a space after a comma
    }
    *((*p)++) = lastch = c;
}




void MIPS16 cmd_autosave(void) {
    char *buf, *p;
    int c, prevc = 0, crunch = false;
   // int count = 0;
   // uint64_t timeout;

    if(CurrentLinePtr) error("Invalid in a program");
    char *tp=(char *)checkstring(cmdline,( char *)"APPEND");
    if(tp){
        // ClearRuntime()  Only want to do some of this
    	ClearVars(0);
        CloseAudio();
        CloseAllFiles();
        ClearExternalIO();                                              // this MUST come before InitHeap()

        p = buf = GetMemory(EDIT_BUFFER_SIZE);
        char * fromp  = (char *)ProgMemory;
        p = buf;
        while(*fromp != 0xff) {
            if(*fromp == T_NEWLINE) {
                fromp = ( char *)llist(( char *)p, ( char *)fromp);                                // otherwise expand the line
                p += strlen((char *)p);
                *p++ = '\n'; *p = 0;
            }
            // finally, is it the end of the program?
            if(fromp[0] == 0 || fromp[0] == 0xff) break;
        }
        goto readin;
    }
    if(*cmdline) {
        if(toupper(*cmdline) == 'C')
            crunch = true;
        else
            error("Unknown command");
    }

    ClearProgram();                                                 // clear any leftovers from the previous program
    p = buf = GetTempMemory(EDIT_BUFFER_SIZE);
    CrunchData(&p, 0);                                              // initialise the crunch data subroutine
    //while((c = (getConsole() & 0x7f)) != 0x1a) {                    // while waiting for the end of text char
readin:
    while((c = MMInkey()) != 0x1a && c!=F1 && c!=F2) {                    // while waiting for the end of text char
    	//if (c == -1 && count && time_us_64() - timeout > 100000)
    	//{
       //   fflush(stdout);
        //  count = 0;
       // }
       	if(p == buf && c == '\n') continue;                         // throw away an initial line feed which can follow the command
        if((p - buf) >= EDIT_BUFFER_SIZE) error("Not enough memory");
        if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
            if(c == TAB) c = ' ';
            if(crunch)
                CrunchData(&p, c);                                  // insert into RAM after throwing away comments. etc
            else
                *p++ = c;                                           // insert the input into RAM
            {
                if(!(c == '\n' && prevc == '\r')) MMputchar(c);     // and echo it
                if(c == '\r') MMputchar('\n');
            }
            prevc = c;
        }
    }
    *p = 0;                                                         // terminate the string in RAM
    while(getConsole() != -1);                                      // clear any rubbish in the input
//    ClearSavedVars();                                               // clear any saved variables
    SaveProgramToFlash(buf, true);
    if(c==F2){
        ClearVars(0);
        strcpy(inpbuf,"RUN\r\n");
        multi=false;
        tokenise(true);                                             // turn into executable code
        ExecuteProgram(tknbuf);                                     // execute the line straight away
    }
}




/***********************************************************************************************
interrupt check

The priority of interrupts (highest to low) is:
Touch (MM+ only)
CFunction Interrupt
ON KEY
I2C Slave Rx
I2C Slave Tx
COM1
COM2
COM3 (MM+ only)
COM4 (MM+ only)
GUI Int Down (MM+ only)
GUI Int Up (MM+ only)
WAV Finished (MM+ only)
IR Receive
Interrupt command/CSub Interrupt
I/O Pin Interrupts in order of definition
Tick Interrupts (1 to 4 in that order)

************************************************************************************************/

// check if an interrupt has occured and if so, set the next command to the interrupt routine
// will return true if interrupt detected or false if not
int __attribute__ ((optimize("-O2"))) check_interrupt(void) {
    int i, v;
    char *intaddr;
    static char rti[2];
    ProcessTouch();
    CheckSDCard();
    processgps();
    if(CheckGuiFlag) CheckGui();                                    // This implements a LED flash

//  if(CFuncInt) CallCFuncInt();                                    // check if the CFunction wants to do anything (see CFunction.c)
    if(!InterruptUsed) return 0;                                    // quick exit if there are no interrupts set
    if(InterruptReturn != NULL || CurrentLinePtr == NULL) return 0; // skip if we are in an interrupt or in immediate mode

    // check for an  ON KEY loc  interrupt
    if(OnKeyGOSUB && kbhitConsole()) {
        intaddr = OnKeyGOSUB;                                       // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }

    // check for an  ON KEY key ,loc  interrupt
    if(KeyInterrupt != NULL && Keycomplete) {
   		Keycomplete=false;
   		intaddr = KeyInterrupt;									    // set the next stmt to the interrupt location
   		goto GotAnInterrupt;
   	}


#ifdef INCLUDE_I2C_SLAVE

    if ((I2C_Status & I2C_Status_Slave_Receive_Rdy)) {
        I2C_Status &= ~I2C_Status_Slave_Receive_Rdy;                // clear completed flag
        intaddr = I2C_Slave_Receive_IntLine;                        // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }
    if ((I2C_Status & I2C_Status_Slave_Send_Rdy)) {
        I2C_Status &= ~I2C_Status_Slave_Send_Rdy;                   // clear completed flag
        intaddr = I2C_Slave_Send_IntLine;                           // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }
#endif


  // interrupt routines for the serial ports
  if(com1 || com2 || com3 || com4){
    if(com1_interrupt != NULL && SerialRxStatus(1) >= com1_ilevel) {// do we need to interrupt?
        intaddr = com1_interrupt;                                   // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }
    if(com1_TX_interrupt && com1_TX_complete){
        intaddr=com1_TX_interrupt;
        com1_TX_complete=false;
        goto GotAnInterrupt;
    }
    if(com2_interrupt != NULL && SerialRxStatus(2) >= com2_ilevel) {// do we need to interrupt?
        intaddr = com2_interrupt;                                   // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }
    if(com2_TX_interrupt && com2_TX_complete){
        intaddr=com2_TX_interrupt;
        com2_TX_complete=false;
        goto GotAnInterrupt;
    }

    if(com3_interrupt != NULL && SerialRxStatus(3) >= com3_ilevel) {// do we need to interrupt?
        intaddr = com3_interrupt;                                   // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }

    if(com3_TX_interrupt && com3_TX_complete){
        intaddr=com3_TX_interrupt;
        com3_TX_complete=false;
        goto GotAnInterrupt;
    }
    if(com4_interrupt != NULL && SerialRxStatus(4) >= com4_ilevel) {// do we need to interrupt?
        intaddr = com4_interrupt;                                   // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }

    if(com4_TX_interrupt && com4_TX_complete){
        intaddr=com4_TX_interrupt;
        com4_TX_complete=false;
        goto GotAnInterrupt;
    }
  }

    if(gui_int_down && GuiIntDownVector) {                          // interrupt on pen down
        intaddr = GuiIntDownVector;                                 // get a pointer to the interrupt routine
        gui_int_down = false;
        goto GotAnInterrupt;
    }

    if(gui_int_up && GuiIntUpVector) {
        intaddr = GuiIntUpVector;                                   // get a pointer to the interrupt routine
        gui_int_up = false;
        goto GotAnInterrupt;
    }

    if(WAVInterrupt != NULL && WAVcomplete) {
        WAVcomplete=false;
        intaddr = WAVInterrupt;                                     // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }
    if(ADCInterrupt != NULL && ADCcomplete){
        ADCcomplete=false;
        intaddr = ADCInterrupt;                                     // set the next stmt to the interrupt location
        /*
        for(i=0;i<=ADCmax;i++){
            if(ADCbits[ADCchannelA]==12)a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0XFFF * VCC);
            else if(ADCbits[ADCchannelA]==10)a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0X3FF * VCC);
            else if(ADCbits[ADCchannelA])a1float[i]=((MMFLOAT)(a1point[i])/(MMFLOAT)0XFF * VCC);
            if(ADCchannelB){
                if(ADCbits[ADCchannelB]==12)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0XFFF * VCC);
                else if(ADCbits[ADCchannelB]==10)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0X3FF * VCC);
                else if(ADCbits[ADCchannelB]==8)a2float[i]=((MMFLOAT)(a2point[i])/(MMFLOAT)0XFF * VCC);
            }
            if(ADCchannelC){
                if(ADCbits[ADCchannelC]==12)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0XFFF * VCC);
                else if(ADCbits[ADCchannelC]==10)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0X3FF * VCC);
                else if(ADCbits[ADCchannelC]==8)a3float[i]=((MMFLOAT)(a3point[i])/(MMFLOAT)0XFF * VCC);
            }
        }
        */
        for(i=0;i<=ADCmax;i++){
                      a1float[i]=((MMFLOAT)(ADCscale[0]*a1point[i]) + ADCbottom[0] );
                      if(ADCchannelB)a2float[i]=((MMFLOAT)(ADCscale[1]*a2point[i]) + ADCbottom[1] );
                      if(ADCchannelC)a3float[i]=((MMFLOAT)(ADCscale[2]*a3point[i]) + ADCbottom[2] );
       }

        goto GotAnInterrupt;
    }
    if(IrGotMsg && IrInterrupt != NULL) {
        IrGotMsg = false;
        intaddr = IrInterrupt;                                      // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }

    if(KeypadInterrupt != NULL && KeypadCheck()) {
        intaddr = KeypadInterrupt;                                  // set the next stmt to the interrupt location
        goto GotAnInterrupt;
    }

    if(CSubInterrupt != NULL && CSubComplete) {
       	intaddr = CSubInterrupt;                                  // set the next stmt to the interrupt location
    	CSubComplete=0;
    	goto GotAnInterrupt;
    }


    for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
        if(inttbl[i].pin != 0) {                                    // if this entry has an interrupt pin set
            v = ExtInp(inttbl[i].pin);                              // get the current value of the pin
            // check if interrupt occured
            if((inttbl[i].lohi == T_HILO && v < inttbl[i].last) || (inttbl[i].lohi == T_LOHI && v > inttbl[i].last) || (inttbl[i].lohi == T_BOTH && v != inttbl[i].last)) {
                intaddr = inttbl[i].intp;                           // set the next stmt to the interrupt location
                inttbl[i].last = v;                                 // save the new pin value
                goto GotAnInterrupt;
            } else
                inttbl[i].last = v;                                 // no interrupt, just update the pin value
        }
    }

    // check if one of the tick interrupts is enabled and if it has occured
    for(i = 0; i < NBRSETTICKS; i++) {
        if(TickInt[i] != NULL && TickTimer[i] > TickPeriod[i]) {
            // reset for the next tick but skip any ticks completely missed
            while(TickTimer[i] > TickPeriod[i]) TickTimer[i] -= TickPeriod[i];
            intaddr = TickInt[i];
            goto GotAnInterrupt;
        }
    }

    // if no interrupt was found then return having done nothing
    return 0;

    // an interrupt was found if we jumped to here
GotAnInterrupt:
    LocalIndex++;                                                   // IRETURN will decrement this
    if(OptionErrorSkip>0)SaveOptionErrorSkip=OptionErrorSkip;
    else SaveOptionErrorSkip = 0;
    OptionErrorSkip=0;
    strcpy( SaveMMErrMsg, MMErrMsg);   //save error message and clear
    *MMErrMsg=0;
    SaveMMerrno=MMerrno;              // saved MMerrno and clear
    MMerrno=0;
    InterruptReturn = nextstmt;                                     // for when IRETURN is executed
    // if the interrupt is pointing to a SUB token we need to call a subroutine
    if(*intaddr == cmdSUB) {
    	strncpy(CurrentInterruptName, intaddr + 1, MAXVARLEN);
    	rti[0] = cmdIRET;                                           // setup a dummy IRETURN command
        rti[1] = 0;
        if(gosubindex >= MAXGOSUB) error("Too many SUBs for interrupt");
        errorstack[gosubindex] = CurrentLinePtr;
        gosubstack[gosubindex++] = rti;                             // return from the subroutine to the dummy IRETURN command
        LocalIndex++;                                               // return from the subroutine will decrement LocalIndex
        skipelement(intaddr);                                       // point to the body of the subroutine
    }

    nextstmt = intaddr;                                             // the next command will be in the interrupt routine
    return 1;
}



// get the address for a MMBasic interrupt
// this will handle a line number, a label or a subroutine
// all areas of MMBasic that can generate an interrupt use this function
char *GetIntAddress(char *p) {
    int i;
    if(isnamestart((uint8_t)*p)) {                                  // if it starts with a valid name char
        i = FindSubFun(p, 0);                                       // try to find a matching subroutine
        if(i == -1)
            return findlabel(p);                                    // if a subroutine was NOT found it must be a label
        else
            return subfun[i];                                       // if a subroutine was found, return the address of the sub
    }

    return findline(getinteger(p), true);                           // otherwise try for a line number
}
void *mymalloc(size_t size){
	static int32_t heaptop=0;
	unsigned int *a = malloc(size);
	if((uint32_t)a+size>heaptop)heaptop=(uint32_t)a+size;
	if(heaptop+0x100 > __get_MSP()) {
	    _excep_code = RESTART_HEAP;                            // otherwise do an automatic reset
		uSec(10000);
	    SoftReset();                                                // this will restart the processor
	}
	return a;
}
void MIPS16 fun_json(void){
    char *json_string=NULL;
    const cJSON *root = NULL;
    void *ptr1 = NULL;
    char *p;
    sret=GetTempMemory(STRINGSIZE);
	int64_t *dest=NULL;
    MMFLOAT tempd;
    int i,j,k,mode,index;
    char field[32],num[6];
    getargs(&ep, 3, ",");
    char *a=GetTempStrMemory();
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
        error("Argument 1 must be integer array");
    }
    dest = (long long int *)ptr1;
    json_string=(char *)&dest[1];
    } else error("Argument 1 must be integer array");
    cJSON_InitHooks(NULL);
    cJSON * parse = cJSON_Parse(json_string);
    if(parse==NULL)error("Invalid JSON data");
    root=parse;
    p=getCstring(argv[2]);
    int len = strlen(p);
    memset(field,0,32);
    memset(num,0,6);
    i=0;j=0;k=0;mode=0;
    while(i<len){
        if(p[i]=='['){ //start of index
            mode=1;
            field[j]=0;
            root = cJSON_GetObjectItemCaseSensitive(root, field);
            memset(field,0,32);
            j=0;
        }
        if(p[i]==']'){
            num[k]=0;
            index=atoi(num);
            root = cJSON_GetArrayItem(root, index);
            memset(num,0,6);
            k=0;
        }
        if(p[i]=='.'){ //new field separator
            if(mode==0){
                field[j]=0;
                root = cJSON_GetObjectItemCaseSensitive(root, field);
             memset(field,0,32);
                j=0;
            } else { //step past the dot after a close bracket
                mode=0;
            }
        } else  {
            if(mode==0)field[j++]=p[i];
            else if(p[i]!='[')num[k++]=p[i];
        }
        i++;
    }
    root = cJSON_GetObjectItem(root, field);

    if (cJSON_IsObject(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsInvalid(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsNumber(root))
    {
        tempd = root->valuedouble;

        if((MMFLOAT)((int64_t)tempd)==tempd) IntToStr(a,(int64_t)tempd,10);
        else FloatToStr(a, tempd, 0, STR_AUTO_PRECISION, ' ');   // set the string value to be saved
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsBool(root)){
        int64_t tempint;
        tempint=root->valueint;
        cJSON_Delete(parse);
        if(tempint)strcpy(sret,"true");
        else strcpy(sret,"false");
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsString(root)){
        strcpy(a,root->valuestring);
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    cJSON_Delete(parse);
    targ=T_STR;
    sret=a;
}
