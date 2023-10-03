/*-*****************************************************************************

ArmmiteF4 MMBasic

FileIO.c

Does all the SD Card related file I/O in MMBasic.

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

//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"
#include "fatfs.h"
int OptionFileErrorAbort = true;
extern char SDPath[4]; /* SD card logical drive path */
extern void Display_Refresh(void);
extern BYTE MDD_SDSPI_CardDetectState(void);
extern 	volatile unsigned int SDtimer;
// FatFs definitions
FATFS FatFs;
DIR dj;
FILINFO fno;

char FileGetChar(int fnbr);
char FilePutChar(char c, int fnbr);
int FileEOF(int fnbr);
char *GetCWD(void);
void File_CloseAll(void);
int InitSDCard(void);
char *ChangeToDir(char *p);
void LoadImage(char *p);
void LoadFont(char *p);
int dirflags;
FRESULT FSerror;
volatile BYTE SDCardStat = STA_NOINIT | STA_NODISK;
volatile int diskcheckrate = 0;
extern 	volatile unsigned int checkSD;

extern unsigned int __attribute__((section(".my_section"))) _excep_cause;
extern int as_strcmpi (const char *s1, const char *s2);
#define SDbufferSize 512
static char *SDbuffer[MAXOPENFILES+1]={NULL};
int buffpointer[MAXOPENFILES+1]={0};
static uint32_t lastfptr[MAXOPENFILES+1]={[0 ... MAXOPENFILES ] = -1};
uint32_t fmode[MAXOPENFILES+1]={0};
static unsigned int bw[MAXOPENFILES+1]={[0 ... MAXOPENFILES ] = -1};

extern RTC_HandleTypeDef hrtc;

/*****************************************************************************************
Mapping of errors reported by the file system to MMBasic file errors
*****************************************************************************************/
const int ErrorMap[] = {        0,                                  // 0
                                1,                                  // Assertion failed
                                2,                                  // Low level I/O error
                                3,                                  // No response from SDcard
                                4,                                  // Could not find the file
                                5,                                  // Could not find the path
                                6,                                  // The path name format is invalid
                                7,                                  // Prohibited access or directory full
                                8,                                  // Directory exists or path to it cannot be found
                                9,                                  // The file/directory object is invalid
                               10,                                  // SD card is write protected
                               11,                                  // The logical drive number is invalid
                               12,                                  // The volume has no work area
                               13,                                  // Not a FAT volume
                               14,                                  // Format aborted
                               15,                                  // Could not access volume
                               16,                                  // File sharing policy
                               17,                                  // Buffer could not be allocated
                               18,                                  // Too many open files
                               19,                                  // Parameter is invalid
							   20									// Not present
                            };

/******************************************************************************************
Text for the file related error messages reported by MMBasic
******************************************************************************************/
const char *FErrorMsg[] = {	"Succeeded ",
		"Low level I/O error",
		"Assertion failed",
		"SD Card not found",
		"Could not find the file",
		"Could not find the path",
		"The path name format is invalid",
		"Prohibited access or not empty",
		"Exists or path to it not found",
		"The file/directory is invalid",
		"SD Card is write protected",
		"The drive number is invalid",
		"The volume has no work area",
		"Not A FAT volume",
		"Format aborted",
		"Could not access volume",
		"File sharing policy",
		"Buffer could not be allocated",
		"Too many open files",
		"Parameter is invalid",
		"SD card not present"
};




//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// MMBASIC COMMANDS & FUNCTIONS FOR THE SDCARD /////////////////////////////


void cmd_save(void) {
    int fnbr;
    unsigned int nbr;
    char *p, *pp, *flinebuf;
    int x,y,w,h, filesize;
    int i;
    if(!InitSDCard()) return;
	int maxH=VRes;
    int maxW=HRes;
    fnbr = FindFreeFileNbr();
    if((p = checkstring(cmdline, "IMAGE")) !=NULL){
        unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
        unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
        unsigned char bmppad[3] = {0,0,0};
    	getargs(&p,9,",");
        if(!InitSDCard()) return;
        if((void *)ReadBuffer == (void *)DisplayNotSet) error("SAVE IMAGE not available on this display");
        pp = getFstring(argv[0]);
        if(argc!=1 && argc!=9)error("Syntax");
        if(strchr(pp, '.') == NULL) strcat(pp, ".BMP");
        if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
        if(argc==1){
        	x=0; y=0; h=maxH; w=maxW;
        } else {
        	x=getint(argv[2],0,maxW-1);
        	y=getint(argv[4],0,maxH-1);
        	w=getint(argv[6],1,maxW-x);
        	h=getint(argv[8],1,maxH-y);
        }
        filesize=54 + 3*w*h;
        bmpfileheader[ 2] = (unsigned char)(filesize    );
        bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
        bmpfileheader[ 4] = (unsigned char)(filesize>>16);
        bmpfileheader[ 5] = (unsigned char)(filesize>>24);

        bmpinfoheader[ 4] = (unsigned char)(       w    );
        bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
        bmpinfoheader[ 6] = (unsigned char)(       w>>16);
        bmpinfoheader[ 7] = (unsigned char)(       w>>24);
        bmpinfoheader[ 8] = (unsigned char)(       h    );
        bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
        bmpinfoheader[10] = (unsigned char)(       h>>16);
        bmpinfoheader[11] = (unsigned char)(       h>>24);
		f_write(FileTable[fnbr].fptr, bmpfileheader, 14, &nbr);
		f_write(FileTable[fnbr].fptr, bmpinfoheader, 40, &nbr);
        flinebuf = GetTempMemory(maxW * 4);
        for(i = y+h-1; i >= y; i--){
           ReadBuffer(x, i, x+w-1, i, flinebuf);
           f_write(FileTable[fnbr].fptr, flinebuf, w*3, &nbr);
           if((w*3) % 4 !=0) f_write(FileTable[fnbr].fptr, bmppad, 4-((w*3) % 4) , &nbr);
        }
        FileClose(fnbr);
        return;
    } else if((p = checkstring(cmdline, "DATA")) !=NULL){
		getargs(&p,5,",");
        if(!InitSDCard()) return;
		if(argc!=5)error("Syntax");
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPeekAddr(argv[2]) & 0b11111111111111111111111111111100);
		uint32_t size=getint(argv[4],1,0x7FFFFFFF);
		for(uint32_t i=address;i<address+size;i++)if(!PEEKRANGE(i)) error("Address");
		if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
		f_write(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File write error");
        FileClose(fnbr);
        return;
    } else {
        char b[STRINGSIZE];
        if(!InitSDCard()) return;
        p = getFstring(cmdline);                           // get the file name and change to the directory
        if(strchr(p, '.') == NULL) strcat(p, ".BAS");
        if(!BasicFileOpen(p, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
        p  = ProgMemory;
        while(!(*p == 0 || *p == 0xff)) {                               // this is a safety precaution
            p = llist(b, p);                                            // expand the line
            pp = b;
            while(*pp) FilePutChar(*pp++, fnbr);                        // write the line to the SD card
            FilePutChar('\r', fnbr); FilePutChar('\n', fnbr);           // terminate the line
            if(p[0] == 0 && p[1] == 0) break;                           // end of the listing ?
        }
        FileClose(fnbr);
    }
}

// load a file into program memory
int FileLoadProgram(char *fname) {
    int fnbr;
    char *p, *buf;
    int c;

    if(!InitSDCard()) return false;
    ClearProgram();												    // clear any leftovers from the previous program
    fnbr = FindFreeFileNbr();
    p = getFstring(fname);
    if(strchr(p, '.') == NULL) strcat(p, ".BAS");
    if(!BasicFileOpen(p, fnbr, FA_READ)) return false;
    p = buf = GetTempMemory(EDIT_BUFFER_SIZE - 256*6);          // leave space for the couple of buffers defined and the file handle
    while(!FileEOF(fnbr)) {                                     // while waiting for the end of file
        if((p - buf) >= EDIT_BUFFER_SIZE - 256*6) error("Not enough memory");
        c = FileGetChar(fnbr) & 0x7f;
        if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
            if(c == TAB) c = ' ';
            *p++ = c;                                               // get the input into RAM
        }
    }
    *p = 0;                                                         // terminate the string in RAM
    FileClose(fnbr);
    SaveProgramToFlash(buf, false);
    return true;
}
void cmd_load(void) {
    int autorun = false;
    char *p;

    p = checkstring(cmdline, "IMAGE");
	if(p) {
        LoadImage(p);
        if(Option.Refresh)Display_Refresh();
        return;
    }
	p = checkstring(cmdline, "DATA");
	if(p) {
	    int fnbr;
	    unsigned int nbr;
		static FILINFO fnod;
	    char *pp;
		getargs(&p,3,",");
		if(argc!=3)error("Syntax");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPokeAddr(argv[2]) & 0b11111111111111111111111111111100);
		FSerror = f_stat(pp, &fnod);
		if(FSerror != FR_OK)error((char *)FErrorMsg[4]);
		if((fnod.fattrib & AM_DIR))error((char *)FErrorMsg[4]);
		uint32_t size=fnod.fsize;
		for(uint32_t i=address;i<address+size;i++)if(!POKERANGE(i)) error("Address");
	    fnbr = FindFreeFileNbr();
		if(!BasicFileOpen(pp, fnbr, FA_READ)) return;
		f_read(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File read error");
	    FileClose(fnbr);
	    return;
	}
    getargs(&cmdline, 3, ",");
    if(!(argc & 1) || argc == 0) error("Syntax");
    if(argc == 3) {
        if(toupper(*argv[2]) == 'R')
            autorun = true;
        else
            error("Syntax");
    } else if(CurrentLinePtr != NULL)
        error("Invalid in a program");

    if(!FileLoadProgram(argv[0])) return;

    if(autorun) {
        if(*ProgMemory != 0x01) return;                              // no program to run
        ClearRuntime();
        WatchdogSet = false;
        PrepareProgram(true);
        IgnorePIN = false;
        if(Option.ProgFlashSize != PROG_FLASH_SIZE) ExecuteProgram(ProgMemory + Option.ProgFlashSize);       // run anything that might be in the library
        nextstmt = ProgMemory;
    }
}



// search for a volume label, directory or file
// s$ = DIR$(fspec, DIR|FILE|ALL)       will return the first entry
// s$ = DIR$()                          will return the next
// If s$ is empty then no (more) files found
void fun_dir(void) {
    static DIR djd;
    char *p;
    static FILINFO fnod;
    static char pp[32];
    getargs(&ep, 3, ",");
    if(argc != 0) dirflags = -1;
    if(!(argc <= 3)) error("Syntax");

    if(argc == 3) {
        if(checkstring(argv[2], "DIR"))
            dirflags = AM_DIR;
        else if(checkstring(argv[2], "FILE"))
            dirflags = -1;
        else if(checkstring(argv[2], "ALL"))
            dirflags = 0;
        else
            error("Invalid flag specification");
    }


    if(argc != 0) {
        // this must be the first call eg:  DIR$("*.*", FILE)
        p = getFstring(argv[0]);
        strcpy(pp,p);
        djd.pat = pp;
        if(!InitSDCard()) return;                                   // setup the SD card
        FSerror = f_opendir(&djd, "");
        ErrorCheck(0);
    }
        if(disk_status(0) & STA_NOINIT){
           f_closedir(&djd);
            error("SD card not found");
        }
        if(dirflags == AM_DIR){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && (fnod.fattrib & AM_DIR) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else if(dirflags == -1){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_DIR)&& !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else {
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }

    if (FSerror != FR_OK || !fnod.fname[0])f_closedir(&djd);
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    strcpy(sret, fnod.fname);
    CtoM(sret);                                                     // convert to a MMBasic style string
    targ = T_STR;
}


void cmd_mkdir(void) {
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_mkdir(p);
    ErrorCheck(0);
    SDtimer=1000;
}



void cmd_rmdir(void){
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_unlink(p);
    ErrorCheck(0);
    SDtimer=1000;
}



void cmd_chdir(void){
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_chdir(p);
    ErrorCheck(0);
    SDtimer=1000;
}



void fun_cwd(void) {
	sret=GetTempStrMemory();
	strcpy(sret,"A");
	strcat(sret,GetCWD());
	CtoM(sret);
   // sret = CtoM(GetCWD());
    targ = T_STR;
}



void cmd_kill(void){
    char *p;
    p = getFstring(cmdline);                                        // get the file name
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_unlink(p);
    ErrorCheck(0);
    SDtimer=1000;
}



void cmd_seek(void) {
    int fnbr, idx;
    char *buff;
    getargs(&cmdline, 5, ",");
    if(argc != 3) error("Syntax");
    if(*argv[0] == '#') argv[0]++;
    fnbr = getinteger(argv[0]);
    if(fnbr < 1 || fnbr > MAXOPENFILES || FileTable[fnbr].com <= MAXCOMPORTS) error("File number");
    if(FileTable[fnbr].com == 0) error("File number #% is not open", fnbr);
    if(!InitSDCard()) return;
    idx = getinteger(argv[2]) - 1;
    if(idx < 0) idx = 0;
    if(fmode[fnbr] & FA_WRITE){
        FSerror = f_lseek(FileTable[fnbr].fptr,idx);
        ErrorCheck(fnbr);
    } else {
    	buff=SDbuffer[fnbr];
    	FSerror = f_lseek(FileTable[fnbr].fptr,idx - (idx % 512));
    	ErrorCheck(fnbr);
    	FSerror = f_read(FileTable[fnbr].fptr, buff,SDbufferSize, &bw[fnbr]);
    	ErrorCheck(fnbr);
    	buffpointer[fnbr]=idx % 512;
    	lastfptr[fnbr]=(uint32_t)FileTable[fnbr].fptr;
    }
    SDtimer=1000;
}

void cmd_name(void) {
    char *old, *new, ss[2];
    ss[0] = tokenAS;                                                // this will be used to split up the argument line
    ss[1] = 0;
    {                                                               // start a new block
        getargs(&cmdline, 3, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc != 3) error("Syntax");
        old = getFstring(argv[0]);                                  // get the old name
        if(old[1] == ':') *old = toupper(*old) - 'A' + '0';         // convert a DOS style disk name to FatFs device number
        new = getFstring(argv[2]);                                  // get the new name
        if(new[1] == ':') *new = toupper(*new) - 'A' + '0';         // convert a DOS style disk name to FatFs device number
        if(!InitSDCard()) return;
        FSerror = f_rename(old, new);
        ErrorCheck(0);
        SDtimer=1000;
    }
}


extern int BMP_bDecode(int x, int y, int fnbr);

void LoadImage(char *p) {
	int fnbr;
	int xOrigin, yOrigin;

	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name

    xOrigin = yOrigin = 0;
	if(argc >= 3) xOrigin = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc == 5) yOrigin = getinteger(argv[4]);                    // get the y origin (optional) argument

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".BMP");
	fnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
    BMP_bDecode(xOrigin, yOrigin, fnbr);
    FileClose(fnbr);
}



#define MAXFILES 500
typedef struct ss_flist {
    char fn[_MAX_LFN];
    int fs;
} s_flist;

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}

void cmd_files(void) {
	int i, dirs, ListCnt;
	char *p, *q;
	int fcnt;
	char ts[STRINGSIZE] = "";
	s_flist *flist;
    static DIR djd;
    static FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(CurrentLinePtr) error("Invalid in a program");
//	OptionFileErrorAbort = 0;
    fcnt = 0;
   if(*cmdline)
		p = getFstring(cmdline);
    else
		p = "*";

    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
    flist=GetMemory(sizeof(s_flist)*MAXFILES);
     // print the current directory
    q = GetCWD();
    MMPrintString("A");MMPrintString(q); PRet();

    // search for the first file/dir
    FSerror = f_findfirst(&djd, &fnod, "", p);
    ErrorCheck(0);
    // add the file to the list, search for the next and keep looping until no more files
    while(FSerror == FR_OK && fnod.fname[0]) {
        if(fcnt >= MAXFILES) {
            	FreeMemory(flist);
            	f_closedir(&djd);
                error("Too many files to list");
        }
        if(!(fnod.fattrib & (AM_SYS | AM_HID))){
            // add a prefix to each line so that directories will sort ahead of files
            if(fnod.fattrib & AM_DIR)
                ts[0] = 'D';
            else
                ts[0] = 'F';

            // and concatenate the filename found
            strcpy(&ts[1], fnod.fname);

            // sort the file name into place in the array
            for(i = fcnt; i > 0; i--) {
                if( strcicmp((flist[i - 1].fn), (ts)) > 0)
                    flist[i] = flist[i - 1];
                else
                    break;
            }
            strcpy(flist[i].fn, ts);
            flist[i].fs = fnod.fsize;
            fcnt++;
        }
        FSerror = f_findnext(&djd, &fnod);
   }

    // list the files with a pause every screen full
	ListCnt = 2;
	for(i = dirs = 0; i < fcnt; i++) {
        if(MMAbort) {
            FreeMemory(flist);
            f_closedir(&djd);
            WDTimer = 0;                                                // turn off the watchdog timer
            longjmp(mark, 1);                                           // jump back to the input prompt
        }
		if(flist[i].fn[0] == 'D') {
    		dirs++;
            MMPrintString("   <DIR>  ");
		}
		else {
            IntToStrPad(ts, flist[i].fs, ' ', 10, 10); MMPrintString(ts);
            MMPrintString("  ");
        }
        MMPrintString(flist[i].fn + 1);
		PRet();
		// check if it is more than a screen full
		if(++ListCnt >= Option.Height && i < fcnt) {
			MMPrintString("PRESS ANY KEY ...");
			MMgetchar();
			MMPrintString("\r                 \r");
			ListCnt = 1;
		}
	}
    // display the summary
    IntToStr(ts, dirs, 10); MMPrintString(ts);
    MMPrintString(" director"); MMPrintString(dirs==1?"y, ":"ies, ");
    IntToStr(ts, fcnt - dirs, 10); MMPrintString(ts);
    MMPrintString(" file"); MMPrintString((fcnt-dirs)==1?"":"s");
	PRet();
    FreeMemory(flist);
    f_closedir(&djd);
	longjmp(mark, 1);                                               // jump back to the input prompt
}





//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////


void ErrorThrow(int e) {
    MMerrno = e;
    FSerror = e;
    strcpy(MMErrMsg, (char *)FErrorMsg[e]);
    if(e && OptionFileErrorAbort) error(MMErrMsg);
    return;
}


void ErrorCheck(int fnbr) {                                         //checks for an error, if fnbr is specified frees up the filehandle before sending error
    int e;
    e = (int)FSerror;
    if(fnbr != 0 && e != 0) ForceFileClose(fnbr);
    if(e >= 1 && e <= 19) ErrorThrow(ErrorMap[e]);
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// GENERAL I/O ////////////////////////////////////////////


void FileOpen(char *fname, char *fmode, char *ffnbr) {
    int fnbr;
    BYTE mode = 0;
    if(str_equal(fmode, "OUTPUT"))
        mode = FA_WRITE | FA_CREATE_ALWAYS;
    else if(str_equal(fmode, "APPEND"))
        mode = FA_WRITE | FA_OPEN_APPEND;
    else if(str_equal(fmode, "INPUT"))
        mode = FA_READ;
    else if(str_equal(fmode, "RANDOM"))
        mode = FA_WRITE | FA_OPEN_APPEND | FA_READ;
    else
        error("File access mode");

    if(*ffnbr == '#') ffnbr++;
    fnbr = getinteger(ffnbr);
    BasicFileOpen(fname, fnbr, mode);
}


// this performs the basic duties of opening a file, all file opens in MMBasic should use this
// it will open the file, set the FileTable[] entry and populate the file descriptor
// it returns with true if successful or false if an error
int BasicFileOpen(char *fname, int fnbr, int mode) {
    if(fnbr < 1 || fnbr > MAXOPENFILES) error("File number");
    if(FileTable[fnbr].com != 0) error("File number already open");
    if(!InitSDCard()) return false;
    // if we are writing check the write protect pin (negative pin number means that low = write protect)
	FileTable[fnbr].fptr = GetMemory(sizeof(FIL));              // allocate the file descriptor
	SDbuffer[fnbr]=GetMemory(SDbufferSize);
	if(fname[1] == ':') *fname = toupper(*fname) - 'A' + '0';   // convert a DOS style disk name to FatFs device number
	FSerror = f_open(FileTable[fnbr].fptr, fname, mode);        // open it
	ErrorCheck(fnbr);
	buffpointer[fnbr]=0;
	lastfptr[fnbr]=-1;
	bw[fnbr]=-1;
	fmode[fnbr]=mode;

    if(FSerror) {
        ForceFileClose(fnbr);
        return false;
    } else
        return true;
}


//close the file and free up the file handle
// it will generate an error if needed
void FileClose(int fnbr) {
    ForceFileClose(fnbr);
    ErrorThrow(FSerror);
}


//close the file and free up the file handle
// it will NOT generate an error
void ForceFileClose(int fnbr) {
    if(fnbr && FileTable[fnbr].fptr != NULL){
        FSerror = f_close(FileTable[fnbr].fptr);
        FreeMemory(FileTable[fnbr].fptr);
        FreeMemory(SDbuffer[fnbr]);
        FileTable[fnbr].fptr = NULL;
        buffpointer[fnbr]=0;
        lastfptr[fnbr]=-1;
        bw[fnbr]=-1;
        fmode[fnbr]=0;
    }
}


char FileGetChar(int fnbr) {
    char ch;
    char *buff=SDbuffer[fnbr];
;
    if(!InitSDCard()) return 0;
    if(fmode[fnbr] & FA_WRITE){
        FSerror = f_read(FileTable[fnbr].fptr, &ch,1, &bw[fnbr]);
        ErrorCheck(fnbr);
        SDtimer=1000;
    } else {
    	if(!(lastfptr[fnbr]==(uint32_t)FileTable[fnbr].fptr && buffpointer[fnbr]<SDbufferSize)){
    		FSerror = f_read(FileTable[fnbr].fptr, buff,SDbufferSize, &bw[fnbr]);
    		ErrorCheck(fnbr);
    		buffpointer[fnbr]=0;
    		lastfptr[fnbr]=(uint32_t)FileTable[fnbr].fptr;
    		SDtimer=1000;
    	}
    	ch=buff[buffpointer[fnbr]];
    	buffpointer[fnbr]++;
    }
    return ch;
}


void FilePutStr(int count, char *c, int fnbr){
    unsigned int bw;
    InitSDCard();
    FSerror = f_write(FileTable[fnbr].fptr, c, count, &bw);
    SDtimer=1000;
    ErrorCheck(fnbr);
}


char FilePutChar(char c, int fnbr) {
    static char t;
    unsigned int bw;
    t = c;
    if(!InitSDCard()) return 0;
    FSerror = f_write(FileTable[fnbr].fptr, &t, 1, &bw);
    SDtimer=1000;
    lastfptr[fnbr]=-1; //invalidate the read file buffer
    ErrorCheck(fnbr);
    return t;
}



int FileEOF(int fnbr) {
    int i;
    if(!InitSDCard()) return 0;
    if(buffpointer[fnbr]<=bw[fnbr]-1) i=0;
    else {
    	i = f_eof(FileTable[fnbr].fptr);
    }
    return i;
}



char *GetCWD(void) {
    char *b;
    int i;
    b = GetTempStrMemory();
    if(!InitSDCard()) return b;
    FSerror = f_getcwd(b, STRINGSIZE);
    for(i=1;i<=strlen(b);i++)b[i-1]=b[i];
    ErrorCheck(0);
    return b;
}



int InitSDCard(void) {
    ErrorThrow(0);    // reset mm.errno to zero
    if(!(SDCardStat & STA_NOINIT)) return true;                     // if the card is present and has been initialised we have nothing to do
    MX_FATFS_Init();
    if(f_mount(&FatFs, (TCHAR const*)SDPath, 1) != FR_OK){
    	FATFS_UnLinkDriver(SDPath);
    	ErrorThrow(ErrorMap[3]);
    	return false;
    }
    SDCardStat &= ~(STA_NODISK | STA_NOINIT);
    return true;
}



// finds the first available free file number.  Throws an error if no free file numbers
int FindFreeFileNbr(void) {
    int i;
    for(i = 1; i <= MAXOPENFILES; i++)
        if(FileTable[i].com == 0) return i;
    error("Too many files open");
    return 0;
}


// check the SD card to see if it has been removed.  Also check WAV playback
// this is called from cmd_pause(), the main ExecuteProgram() loop and the console's MMgetchar()
void __attribute__ ((optimize("-O2"))) CheckSDCard(void) {
    if(CurrentlyPlaying == P_WAV || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_MOD)
        checkWAVinput();
    else {
        if(checkSD){
        	checkSD=0;
        	if(!(SDCardStat & STA_NOINIT)){ //the card is supposed to be initialised - lets check
            	if(BSP_SD_GetCardState()!= MSD_OK){
            	//if(BSP_SD_IsDetected()!= MSD_OK){
            		FATFS_UnLinkDriver(SDPath);
                    BYTE s;
                    s = SDCardStat;
                    s |= (STA_NODISK | STA_NOINIT);
                    SDCardStat = s;
            	}
        	}
        }
    }
}

int ExistsFile(char *p){
    int retval=0;
	DIR djd;
	FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(!InitSDCard()) return -1;
	FSerror = f_stat(p, &fnod);
	if(FSerror != FR_OK)iret=0;
	else if(!(fnod.fattrib & AM_DIR))retval=1;
    return retval;
}
int ExistsDir(char *p){
    int ireturn=0;
    if(strcmp(p,"/")==0 || strcmp(p,"/.")==0 || strcmp(p,"./")==0 || strcmp(p,"/..")==0 )return 1;
	DIR djd;
	FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(p[strlen(p)-1]=='/')strcat(p,".");
	if(!InitSDCard()) return -1;
	FSerror = f_stat(p, &fnod);
	if(FSerror != FR_OK)ireturn=0;
	else if((fnod.fattrib & AM_DIR))ireturn=1;
    return ireturn;
}


