/*-*****************************************************************************

ArmmiteF4 MMBasic

memory.c

This module manages all RAM memory allocation for MMBasic .


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
/**************************************************************************************************************************

NOTE:
  In the PIC32 the following variables are set by the linker:

      (unsigned char *)&_stack         This is the virtual address of the top of the stack and unless some RAM functions are
                                       defined it is also the top of the RAM.  In this case its value is 0xA0020000.

      (unsigned char *)&_splim         This is the virtual address of the top of the heap and represents the start of free
                                       and unallocated memory.

      (unsigned char *)&_heap          This is the virtual address of the top of the memory allocated by the compiler (static
                                       variables, etc)

      (unsigned int)&_min_stack_size   This is the number of bytes allocated to the stack.  No run time checking is performed
                                       and this value is only used by the linker to warn if memory is over allocated.


  The RAM memory map looks like this:

  |--------------------|    <<<   0xA0002000  (Top of RAM)
  |  Functions in RAM  |
  |--------------------|    <<<   (unsigned char *)&_stack
  |                    |
  | Stack (grows down) |
  |                    |
  |--------------------|    <<<   (unsigned char *)&_stack - (unsigned int)&_min_stack_size
  |                    |
  |                    |
  |     Free RAM       |
  |                    |
  |                    |
  |--------------------|   <<<   (unsigned char *)&_splim
  |                    |
  | Heap (if allocated)|
  |                    |
  |--------------------|   <<<   (unsigned char *)&_heap
  |                    |
  |                    |
  |                    |
  |  Static RAM Vars   |
  |                    |
  |                    |
  |                    |
  |--------------------|    <<<   0xA0000000


  The variables must be defined to the C Compiler before using them.  Eg:
      extern unsigned int _stack;
      extern unsigned int _splim;
      extern unsigned int _heap;
      extern unsigned int _min_stack_size;


************************************************************************************************************************/



				// the pre Harmony peripheral libraries

#define INCLUDE_FUNCTION_DEFINES

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"



// memory parameters for this chip
// ===============================
// The following settings will allow MMBasic to use all the free memory on the PIC32.  If you need some RAM for
// other purposes you can declare the space needed as a static variable -or- allocate space to the heap (which
// will reduce the memory available to MMBasic) -or- change the definition of RAMEND.
// NOTE: MMBasic does not use the heap.  It has its own heap which is allocated out of its own memory space.

// The virtual address that MMBasic can start using memory.  This must be rounded up to RAMPAGESIZE.
// MMBasic uses just over 5K for static variables so, in the simple case, RAMBASE could be set to 0xA001800.
// However, the PIC32 C compiler provides us with a convenient marker (see diagram above).


volatile char *StrTmp[MAXTEMPSTRINGS];                                       // used to track temporary string space on the heap
volatile char StrTmpLocalIndex[MAXTEMPSTRINGS];                              // used to track the LocalIndex for each temporary string space on the heap
void *RAMBase;                                                      // this is the base of the RAM used for the variable table (which grows up))
unsigned int MBitsGet(void *addr);
void MBitsSet(void *addr, int bits);
void *getheap(int size);
unsigned int UsedHeap(void);
void heapstats(char *m1);
uint32_t SavedMemoryBufferSize;
int TempMemoryIsChanged = false;						            // used to prevent unnecessary scanning of strtmp[]
int StrTmpIndex = 0;                                                // index to the next unallocated slot in strtmp[]
unsigned int mmap[128];




/***********************************************************************************************************************
 MMBasic commands
************************************************************************************************************************/
void MIPS16 cmd_memory(void) {
	char *p,*tp,*pp;
    tp = checkstring(cmdline, (char *)"PACK");
    if(tp){
        getargs(&tp,7,(char *)",");
        if(argc!=7)error("Syntax");
        int i,n=getinteger(argv[4]);
        if(n<=0)return;
        int size=getint(argv[6],1,32);
        if(!(size==1 || size==4 || size==8 || size==16 || size==32))error((char *)"Invalid size");
        int sourcesize,destinationsize;
        void *top=NULL;
        uint64_t *from=NULL;
        if(CheckEmpty((char *)argv[0])){
            sourcesize=parseintegerarray(argv[0],(int64_t **)&from, 1,1,NULL,false);
            if(sourcesize<n)error("Source array too small");
        } else from=(uint64_t *)GetPokeAddr(argv[0]);
        if(CheckEmpty((char *)argv[2])){
            destinationsize=parseintegerarray(argv[2],(int64_t **)&top, 2,1,NULL,true);
            if(destinationsize*64/size<n)error("Destination array too small");
        } else top=(void *)GetPokeAddr(argv[2]);
        if((uint32_t)from % 8)error("Source address not divisible by 8");
        if(size==1){
            uint8_t *to=(uint8_t *)top;
            for(i=0;i<n;i++){
                int s= i % 8;
                if(s==0)*to=0;
                *to |= ((*from++) & 0x1)<<s;
                if(s==7)to++;
           }
        } else if(size==4){
            uint8_t *to=(uint8_t *)top;
            for(i=0;i<n;i++){
                if((i & 1) == 0){
                    *to=(*from++) & 0xF;
                } else {
                    *to |= ((*from++) & 0xF)<<4;
                    to++;
                }
           }
        } else if(size==8){
            uint8_t *to=(uint8_t *)top;
            while(n--){
            *to++=(uint8_t)*from++;
            }
        } else if(size==16){
            uint16_t *to=(uint16_t *)top;
            if((uint32_t)to % 2)error("Destination address not divisible by 2");
            while(n--){
            *to++=(uint16_t)*from++;
            }
        } else if(size==32){
            uint32_t *to=(uint32_t *)top;
            if((uint32_t)to % 4)error("Destination address not divisible by 4");
            while(n--){
            *to++=(uint32_t)*from++;
            }
        }
        return;
    }
    tp = checkstring(cmdline, ( char *)"PRINT");
    if(tp){
        char *fromp=NULL;
        int sourcesize;
        int64_t *aint;
        getargs(&tp,5,( char *)",");
        if(!(argc==5))error("Syntax");
	    if(*argv[0] == '#') argv[0]++;
		int fnbr = getint(argv[0],1,MAXOPENFILES);	// get the number
        int n=getinteger(argv[2]);
        if(CheckEmpty((char *)argv[4])){
            sourcesize=parseintegerarray(argv[4],&aint,3,1,NULL,false);
            if(sourcesize*8<n)error("Source array too small");
            fromp=(char *)aint;
        } else {
            fromp=(char *)GetPeekAddr(argv[4]);
        }
        if (FileTable[fnbr].com > MAXCOMPORTS)
        {
            FilePutStr(n, fromp, fnbr);
        }
        else error("File % not open",fnbr);
        return;
    }
    tp = checkstring(cmdline, (char *)"INPUT");
    if(tp){
        char *fromp=NULL;
        int sourcesize;
        int64_t *aint;
        getargs(&tp,5,(char *)",");
        if(!(argc==5))error("Syntax");
	    if(*argv[0] == '#') argv[0]++;
		int fnbr = getint(argv[0],1,MAXOPENFILES);	// get the number
        int n=getinteger(argv[2]);
        if(CheckEmpty((char *)argv[4])){
            sourcesize=parseintegerarray(argv[4],&aint,3,1,NULL,false);
            if(sourcesize*8<n)error("Source array too small");
            fromp=(char *)aint;
        } else {
            fromp=(char *)GetPokeAddr(argv[4]);
        }
        if (FileTable[fnbr].com > MAXCOMPORTS)
        {
            while(!(MMfeof(fnbr)) && n--) *fromp++=FileGetChar(fnbr);
            if(n)error("End of file");
        }
        else error("File % not open",fnbr);
        return;
    }
    tp = checkstring(cmdline, (char *)"UNPACK");
    if(tp){
        getargs(&tp,7,(char *)",");
        if(argc!=7)error("Syntax");
        int i,n=getinteger(argv[4]);
        if(n<=0)return;
        int size=getint(argv[6],1,32);
        if(!(size==1 || size==4 || size==8 || size==16 || size==32))error((char *)"Invalid size");
        int sourcesize,destinationsize;
        uint64_t *to=NULL;
        void *fromp=NULL;
        if(CheckEmpty((char *)argv[0])){
            sourcesize=parseintegerarray(argv[0],(int64_t **)&fromp, 1,1,NULL,false);
            if(sourcesize*64/size<n)error("Source array too small");
        } else {
            fromp=(void*)GetPokeAddr(argv[0]);
        }
        if(CheckEmpty((char *)argv[2])){
            destinationsize=parseintegerarray(argv[2],(int64_t **)&to, 2,1,NULL,true);
            if(n>destinationsize)error("Destination array too small");
        } else to=(uint64_t *)GetPokeAddr(argv[2]);
        if((uint32_t)to % 8)error("Source address not divisible by 8");
        if(size==1){
            uint8_t *from=(uint8_t *)fromp;
            for(i=0;i<n;i++){
                int s= i % 8;
                *to++ = ((*from & (1<<s)) ? 1 : 0);
                if(s==7)from++;
           }

        } else if(size==4){
            uint8_t *from=(uint8_t *)fromp;
            for(i=0;i<n;i++){
                if((i & 1) == 0){
                    *to++=(*from) & 0xF;
                } else {
                    *to++ = (*from) >> 4;
                    from++;
                }
           }
        } else if(size==8){
            uint8_t *from=(uint8_t *)fromp;
            while(n--){
            *to++=(uint64_t)*from++;
            }
        } else if(size==16){
            uint16_t *from=(uint16_t *)fromp;
            if((uint32_t)from % 2)error("Source address not divisible by 2");
            while(n--){
            *to++=(uint64_t)*from++;
            }
        } else if(size==32){
            uint32_t *from=(uint32_t *)fromp;
            if((uint32_t)from % 4)error("Source address not divisible by 4");
            while(n--){
            *to++=(uint64_t)*from++;
            }
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"COPY");
    if(tp){
    	if((p = checkstring(tp, (char *)"INTEGER"))) {
    		int stepin=1, stepout=1;
        	getargs(&p,9,(char *)",");
        	if(argc<5)error("Syntax");
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
         	uint64_t *from=(uint64_t *)GetPokeAddr(argv[0]);
         	uint64_t *to=(uint64_t *)GetPokeAddr(argv[2]);
        	if((uint32_t)from % 8)error("Address not divisible by 8");
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	if(argc>=7 && *argv[6])stepin=getint(argv[6],0,0xFFFF);
        	if(argc==9)stepout=getint(argv[8],0,0xFFFF);
        	if(stepin==1 && stepout==1)memmove(to, from, n*8);
        	else{
                if(from<to){
                    from+=(n-1)*stepin;
                    to+=(n-1)*stepout;
                    while(n--){
                        *to=*from;
                        to-=stepout;
                        from-=stepin;
                    }
                } else {
                    while(n--){
                        *to=*from;
                        to+=stepout;
                        from+=stepin;
                    }
                }
        	}
    		return;
    	}
    	if((p = checkstring(tp, (char *)"FLOAT"))) {
    		int stepin=1, stepout=1;
        	getargs(&p,9,(char *)","); //assume byte
        	if(argc<5)error("Syntax");
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	MMFLOAT *from=(MMFLOAT *)GetPokeAddr(argv[0]);
        	MMFLOAT *to=(MMFLOAT *)GetPokeAddr(argv[2]);
        	if((uint32_t)from % 8)error("Address not divisible by 8");
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	if(argc>=7 && *argv[6])stepin=getint(argv[6],0,0xFFFF);
        	if(argc==9)stepout=getint(argv[8],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1 && stepout==1)memmove(to, from, n*8);
        	else{
                if(from<to){
                    from+=(n-1)*stepin;
                    to+=(n-1)*stepout;
                    while(n--){
                        *to=*from;
                        to-=stepout;
                        from-=stepin;
                    }
                } else {
                    while(n--){
                        *to=*from;
                        to+=stepout;
                        from+=stepin;
                    }
                }
        	}
    		return;
    	}
        getargs(&tp,9,(char *)","); //assume byte
        if(argc<5)error("Syntax");
        int stepin=1, stepout=1;
    	char *from=(char *)GetPeekAddr(argv[0]);
    	char *to=(char *)GetPokeAddr(argv[2]);
    	int n=getinteger(argv[4]);
        if(argc>=7 && *argv[6])stepin=getint(argv[6],0,0xFFFF);
        if(argc==9)stepout=getint(argv[8],0,0xFFFF);
        if(n<=0)return;
    	if(stepin==1 && stepout==1)memmove(to, from, n);
        else {
            if(from<to){
                from+=(n-1)*stepin;
                to+=(n-1)*stepout;
                while(n--){
                    *to=*from;
                    to-=stepout;
                    from-=stepin;
                }
            } else {
                while(n--){
                    *to=*from;
                    to+=stepout;
                    from+=stepin;
                }
            }
        }
    	return;
    }
    tp = checkstring(cmdline, (char *)"SET");
    if(tp){
    	char *p;
    	if((p = checkstring(tp, (char *)"BYTE"))) {
        	getargs(&p,5,(char *)","); //assume byte
        	if(argc!=5)error("Syntax");
         	char *to=(char *)GetPokeAddr(argv[0]);
         	int val=getint(argv[2],0,255);
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	memset(to, val, n);
    		return;
    	}
    	if((p = checkstring(tp, (char *)"SHORT"))) {
        	getargs(&p,5,(char *)","); //assume byte
        	if(argc!=5)error("Syntax");
         	short *to=(short *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 2)error("Address not divisible by 2");
        	short *q=to;
   		    short data=getint(argv[2],0,65535);
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	while(n>0){
                *q++=data;
                n--;  
        	}
    		return;
    	}
    	if((p = checkstring(tp, (char *)"WORD"))) {
        	getargs(&p,5,(char *)","); //assume byte
        	if(argc!=5)error("Syntax");
         	unsigned int *to=(unsigned int *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 4)error("Address not divisible by 4");
        	unsigned int *q=to;
   		    unsigned int data=getint(argv[2],0,0xFFFFFFFF);
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	while(n>0){
                *q++=data;
                n--;  
        	}
    		return;
     	}
    	if((p = checkstring(tp, (char *)"INTEGER"))) {
    		int stepin=1;
        	getargs(&p,7,(char *)",");
        	if(argc<5)error("Syntax");
         	uint64_t *to=(uint64_t *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	int64_t data;
    		data=getinteger(argv[2]);
        	int n=getinteger(argv[4]);
        	if(argc==7)stepin=getint(argv[6],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1)while(n--)*to++=data;
        	else{
            	while(n--){
            		*to=data;
            		to+=stepin;
            	}
        	}
    		return;
    	}
    	if((p = checkstring(tp, (char *)"FLOAT"))) {
    		int stepin=1;
        	getargs(&p,7,(char *)","); //assume byte
        	if(argc<5)error("Syntax");
        	MMFLOAT *to=(MMFLOAT *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	MMFLOAT data;
    		data=getnumber(argv[2]);
        	int n=getinteger(argv[4]);
           	if(argc==7)stepin=getint(argv[6],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1)while(n--)*to++=data;
        	else{
            	while(n--){
            		*to=data;
            		to+=stepin;
            	}
        	}
    		return;
    	}
    	getargs(&tp,5,(char *)","); //assume byte
    	if(argc!=5)error("Syntax");
     	char *to=(char *)GetPokeAddr(argv[0]);
     	int val=getint(argv[2],0,255);
    	int n=getinteger(argv[4]);
    	if(n<=0)return;
    	memset(to, val, n);
    	return;
    }
    //-----------Output of memory command-------------
	//-------------------------------------------------
    int i, j,k, var, nbr, vsize, VarCnt,pmax;
    int ProgramSize, ProgramPercent, VarSize, VarPercent, GeneralSize, GeneralPercent, SavedVarSize, SavedVarSizeK, SavedVarPercent, SavedVarCnt;
    int CFunctSize, CFunctSizeK, CFunctNbr, CFunctPercent, FontSize, FontSizeK, FontNbr, FontPercent, LibrarySizeK, LibraryPercent;
    unsigned int CurrentRAM, *pint;
    //char *p;
    pmax=SAVEDVARS_FLASH_SIZE;
    CurrentRAM = (unsigned int)RAMEND - (unsigned int)RAMBase;
    // calculate the space allocated to variables on the heap
    //for(i = VarCnt = vsize = var = 0; var < varcnt; var++) {
    for(i = VarCnt = vsize = var = 0; var < MAXVARS; var++) {

        if(vartbl[var].type == T_NOTYPE) continue;
        VarCnt++;  vsize += sizeof(struct s_vartbl);
        if(vartbl[var].val.s == NULL) continue;
        if(vartbl[var].type & T_PTR) continue;
        nbr = vartbl[var].dims[0] + 1 - OptionBase;
        if(vartbl[var].dims[0]) {
            for(j = 1; j < MAXDIM && vartbl[var].dims[j]; j++)
                nbr *= (vartbl[var].dims[j] + 1 - OptionBase);
            if(vartbl[var].type & T_NBR)
                i += MRoundUp(nbr * sizeof(MMFLOAT));
            else if(vartbl[var].type & T_INT)
                i += MRoundUp(nbr * sizeof(long long int));
            else
                i += MRoundUp(nbr * (vartbl[var].size + 1));
        } else
            if(vartbl[var].type & T_STR)
                i += STRINGSIZE;
    }

    VarSize = (vsize + i + 512)/1024;                               // this is the memory allocated to variables
    VarPercent = ((vsize + i) * 100)/CurrentRAM;
    if(VarCnt && VarSize == 0) VarPercent = VarSize = 1;            // adjust if it is zero and we have some variables
   // PIntComma (UsedHeap());PIntComma (i);
	i = UsedHeap() - i;
    if(i < 0) i = 0;
    GeneralSize = (i + 512)/1024; GeneralPercent = (i * 100)/CurrentRAM;

    // count the space used by saved variables (in flash)
    p = (char *)FLASH_SAVED_VAR_ADDR;
    SavedVarCnt = 0;
    while(!(*p == 0 || *p == 0xff)) {
	unsigned char type, array;
   	    pmax--;
        SavedVarCnt++;
        type = *p++;
        array = type & 0x80;  type &= 0x7f;                         // set array to true if it is an array
        if(!(type==T_INT || type==T_STR || type==T_NBR)){
        	ClearSavedVars();
        	SavedVarCnt=0;
        	break;
        }
        p += strlen(p) + 1;
        if(array)
            p += (p[0] | p[1] << 8 | p[2] << 16| p[3] << 24) + 4;
        else {
            if(type &  T_NBR)
                p += sizeof(MMFLOAT);
            else if(type &  T_INT)
                p += sizeof(long long int);
            else
                p += *p + 1;
        }

        if(pmax==0)error("Saved Vars RAM corrupt");
    }
    SavedVarSize = p - (char *)FLASH_SAVED_VAR_ADDR;
    SavedVarSizeK = (SavedVarSize + 512) / 1024;
    SavedVarPercent = (SavedVarSize * 100) / (SAVEDVARS_FLASH_SIZE);
    if(SavedVarCnt && SavedVarSizeK == 0) SavedVarPercent = SavedVarSizeK = 1;        // adjust if it is zero and we have some variables

    // count the space used by CFunctions, CSubs and fonts
    CFunctSize = CFunctNbr = FontSize = FontNbr = 0;
    pint = (unsigned int *)CFunctionFlash;
    while(*pint != 0xffffffff) {
        if(*pint <FONT_TABLE_SIZE) {
            pint++;
            FontNbr++;
            FontSize += *pint + 8;
        } else {
            pint++;
            CFunctNbr++;
            CFunctSize += *pint + 8;
        }
        pint += (*pint + 4) / sizeof(unsigned int);
    }
    CFunctPercent = (CFunctSize * 100) /  (PROG_FLASH_SIZE /* + SAVEDVARS_FLASH_SIZE*/);
    CFunctSizeK = (CFunctSize + 512) / 1024;
    if(CFunctNbr && CFunctSizeK == 0) CFunctPercent = CFunctSizeK = 1;              // adjust if it is zero and we have some functions
    FontPercent = (FontSize * 100) /  (PROG_FLASH_SIZE /* + SAVEDVARS_FLASH_SIZE*/);
    FontSizeK = (FontSize + 512) / 1024;
    if(FontNbr && FontSizeK == 0) FontPercent = FontSizeK = 1;                      // adjust if it is zero and we have some functions

    //Get the library size
    LibrarySizeK = LibraryPercent = 0;
    if(Option.ProgFlashSize == PROG_FLASH_SIZE) {
           k = 0;
           // first count the normal program code residing in the Library
           p = ProgMemory + Option.ProgFlashSize;
           while(!(p[0] == 0 && p[1] == 0)) {
               	p++; k++;
           }
           while(*p == 0){ // the end of the program can have multiple zeros -count them
               p++;k++;
           }
           p++; k++;    //get 0xFF that ends the program and count it
           while((unsigned int)p & 0b11) { //count to the next word boundary
           	p++;k++;
           }

          // now count the CFunction/CSub/Font data
          if(CFunctionLibrary != NULL) {
             pp = (char *)CFunctionLibrary;
             while(*(unsigned int *)pp != 0xffffffff) {
               j = (*(unsigned int *)(pp + 4)) + 8;  // calculate the total size of the CFunction
               while(j--){
              	  pp++;k++;
               }
             }
           }


              LibrarySizeK=(k+512)/1024;
              LibraryPercent = (LibrarySizeK * 100)/128;
              if(LibrarySizeK == 0) LibrarySizeK = 1;              // adjust if it is zero and we have any library
              if(LibraryPercent == 0) LibraryPercent = 1;          // adjust if it is zero and we have any library

     }

    // count the number of lines in the program
    p = ProgMemory;
    i = 0;
    while(*p != 0xff) {                                             // skip if program memory is erased
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program or module
        if(*p == T_NEWLINE) {
            i++;                                                    // count the line
            p++;                                                    // skip over the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
        skipspace(p);
        if(p[0] == T_LABEL) p += p[1] + 2;                          // skip over the label
        while(*p) p++;                                              // look for the zero marking the start of an element
    }
    ProgramSize = ((p - ProgMemory) + 512)/1024;
    ProgramPercent = ((p - ProgMemory) * 100)/(PROG_FLASH_SIZE /*+ SAVEDVARS_FLASH_SIZE*/);
    if(ProgramPercent > 100) ProgramPercent = 100;
    if(i && ProgramSize == 0) ProgramPercent = ProgramSize = 1;                                        // adjust if it is zero and we have some lines

    MMPrintString("Program Flash:\r\n");
    IntToStrPad(inpbuf, ProgramSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), ProgramPercent, ' ', 2, 10); strcat(inpbuf, "%) Program (");
    IntToStr(inpbuf + strlen(inpbuf), i, 10); strcat(inpbuf, " lines)\r\n");
	MMPrintString(inpbuf);

    if(CFunctNbr) {
        IntToStrPad(inpbuf, CFunctSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
        IntToStrPad(inpbuf + strlen(inpbuf), CFunctPercent, ' ', 2, 10); strcat(inpbuf, "%) "); MMPrintString(inpbuf);
        IntToStr(inpbuf, CFunctNbr, 10); strcat(inpbuf, " Embedded C Routine"); strcat(inpbuf, CFunctNbr == 1 ? "\r\n":"s\r\n");
        MMPrintString(inpbuf);
    }

    if(FontNbr) {
        IntToStrPad(inpbuf, FontSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
        IntToStrPad(inpbuf + strlen(inpbuf), FontPercent, ' ', 2, 10); strcat(inpbuf, "%) "); MMPrintString(inpbuf);
        IntToStr(inpbuf, FontNbr, 10); strcat(inpbuf, " Embedded Fonts"); strcat(inpbuf, FontNbr == 1 ? "\r\n":"s\r\n");
        MMPrintString(inpbuf);
    }

    IntToStrPad(inpbuf, ((PROG_FLASH_SIZE /* + SAVEDVARS_FLAH_SIZE*/) + 512)/1024 - ProgramSize - CFunctSizeK - FontSizeK /*- SavedVarSizeK*/ - LibrarySizeK, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), 100 - ProgramPercent - CFunctPercent - FontPercent /*- SavedVarPercent*/ - LibraryPercent, ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");
	MMPrintString(inpbuf);

    /*
      LibrarySizeK = LibraryPercent = 0;
       if(Option.ProgFlashSize == PROG_FLASH_SIZE) {
           LibrarySizeK = PROG_FLASH_SIZE - Option.ProgFlashSize;
           LibraryPercent = (LibrarySizeK * 100)/PROG_FLASH_SIZE;
           LibrarySizeK /= 1024;
           IntToStrPad(inpbuf, LibrarySizeK, ' ', 4, 10); strcat(inpbuf, "K (");
           IntToStrPad(inpbuf + strlen(inpbuf), LibraryPercent, ' ', 2, 10); strcat(inpbuf, "%) "); strcat(inpbuf, "Library\r\n");
           MMPrintString(inpbuf);
       }
     */

  	 //LibrarySizeK = LibraryPercent = 0;
  	 if(Option.ProgFlashSize == PROG_FLASH_SIZE) {
  		   MMPrintString("\r\nLibrary Flash:\r\n");
  	       // LibrarySizeK = PROG_FLASH_SIZEMAX - Option.ProgFlashSize;  //fixme
  		  //  LibrarySizeK=64;
  	       // LibraryPercent = (LibrarySizeK * 100)/128; //fixme
  	        //LibrarySizeK /= 1024;
  	        IntToStrPad(inpbuf, LibrarySizeK, ' ', 4, 10); strcat(inpbuf, "K (");
  	        //IntToStrPad(inpbuf, (128*1024  + 512)/1024  - LibrarySizeK, ' ', 4, 10); strcat(inpbuf, "K (");
  	        IntToStrPad(inpbuf + strlen(inpbuf), LibraryPercent, ' ', 2, 10); strcat(inpbuf, "%) "); strcat(inpbuf, "Library\r\n");

  	        IntToStrPad(inpbuf + strlen(inpbuf), 128-LibrarySizeK, ' ', 4, 10); strcat(inpbuf, "K (");
  	        IntToStrPad(inpbuf + strlen(inpbuf), 100 - LibraryPercent, ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");

  	        MMPrintString(inpbuf);
  	}



	MMPrintString("\r\nSaved Variables FLASH (4K):\r\n");
	if(SavedVarCnt) {
	       IntToStrPad(inpbuf, SavedVarSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
	       IntToStrPad(inpbuf + strlen(inpbuf), SavedVarPercent, ' ', 2, 10); strcat(inpbuf, "%)");
	       IntToStrPad(inpbuf + strlen(inpbuf), SavedVarCnt, ' ', 2, 10); strcat(inpbuf, " Saved Variable"); strcat(inpbuf, SavedVarCnt == 1 ? " (":"s (");
	       IntToStr(inpbuf + strlen(inpbuf), SavedVarSize, 10); strcat(inpbuf, " bytes)\r\n");
	       MMPrintString(inpbuf);
	}
	IntToStrPad(inpbuf, (( SAVEDVARS_FLASH_SIZE) + 512)/1024 - SavedVarSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
	IntToStrPad(inpbuf + strlen(inpbuf), 100 -  SavedVarPercent, ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");
	MMPrintString(inpbuf);


    MMPrintString("\r\nRAM:\r\n");
    IntToStrPad(inpbuf, VarSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), VarPercent, ' ', 2, 10); strcat(inpbuf, "%) ");
    IntToStr(inpbuf + strlen(inpbuf), VarCnt, 10); strcat(inpbuf, " Variable"); strcat(inpbuf, VarCnt == 1 ? "\r\n":"s\r\n");
	MMPrintString(inpbuf);

    IntToStrPad(inpbuf, GeneralSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), GeneralPercent, ' ', 2, 10); strcat(inpbuf, "%) General\r\n");
	MMPrintString(inpbuf);

    IntToStrPad(inpbuf, (CurrentRAM + 512)/1024 - VarSize - GeneralSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), 100 - VarPercent - GeneralPercent, ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");
	MMPrintString(inpbuf);



}

/***********************************************************************************************************************
 Public memory management functions
************************************************************************************************************************/

/* all memory allocation (except for the heap) is made by m_alloc()
   memory layout used by MMBasic:

          |--------------------|    <<<   This is the end of the RAM allocated to MMBasic (defined as RAMEND)
          |                    |
          |    MMBasic Heap    |
          |    (grows down)    |
          |                    |
          |--------------------|   <<<   VarTableTop
          |   Variable Table   |
          |     (grows up)     |
          |--------------------|   <<<   vartbl
                                         This is the start of the RAM allocated to MMBasic (defined as RAMBASE)

   m_alloc(size) is called when the program is running and whenever the variable table needs to be expanded

   Separately calls are made to GetMemory() and FreeMemory() to allocate or free space on the heap (which grows downward
   towards the variable table).  While the program is running an out of memory situation will occur when the space between
   the heap (growing downwards) and the variable table (growing up) reaches zero.

*/


void m_alloc(int size) {
    // every time the variable table is increased this must be called to verify that enough memory is free
/*    vartbl = (struct s_vartbl *)RAMBase;
    VarTableTop = (unsigned char *)vartbl + MRoundUp(size);
    if(MBitsGet(VarTableTop) & PUSED) {
        LocalIndex = 0;
        ClearTempMemory();                                          // hopefully this will give us enough memory to print the prompt
        error("Not enough memory");
    }*/
	memset(vartbl,0,MAXVARS * sizeof(struct s_vartbl));

}



// get some memory from the heap
//void *GetMemory(size_t msize) {
//    TestStackOverflow();                                            // throw an error if we have overflowed the PIC32's stack
//    return getheap(msize);                                          // allocate space
//}


// Get a temporary buffer of any size, returns a pointer to the buffer
// The space only lasts for the length of the command or in the case of a sub/fun until it has exited.
// A pointer to the space is also saved in strtmp[] so that the memory can be automatically freed at the end of the command
// StrTmpLocalIndex[] is used to track the sub/fun nesting level at which it was created
void *GetTempMemory(int NbrBytes) {
    if(StrTmpIndex >= MAXTEMPSTRINGS) error("Not enough memory");
    StrTmpLocalIndex[StrTmpIndex] = LocalIndex;
    StrTmp[StrTmpIndex] = GetMemory(NbrBytes);
    TempMemoryIsChanged = true;
    return (void *)StrTmp[StrTmpIndex++];
}


// get a temporary string buffer
// this is used by many BASIC string functions.  The space only lasts for the length of the command.
void *GetTempStrMemory(void) {
    if(StrTmpIndex >= MAXTEMPSTRINGS) error("Not enough memory");
    StrTmpLocalIndex[StrTmpIndex] = LocalIndex;
    StrTmp[StrTmpIndex] = GetMemory(STRINGSIZE);
    TempMemoryIsChanged = true;
    return (void *)StrTmp[StrTmpIndex++];
//    return GetTempMemory(STRINGSIZE);
}


// clear any temporary string spaces (these last for just the life of a command) and return the memory to the heap
// this will not clear memory allocated with a local index less than LocalIndex, sub/funs will increment LocalIndex
// and this prevents the automatic use of ClearTempMemory from clearing memory allocated before calling the sub/fun
void ClearTempMemory(void) {
    while(StrTmpIndex > 0) {
        if(StrTmpLocalIndex[StrTmpIndex - 1] >= LocalIndex) {
            StrTmpIndex--;
            FreeMemory((char *)StrTmp[StrTmpIndex]);
            StrTmp[StrTmpIndex] = NULL;
            TempMemoryIsChanged = false;
        } else
            break;
    }
}



void ClearSpecificTempMemory(void *addr) {
    int i;
    for(i = 0; i < StrTmpIndex; i++) {
        if(StrTmp[i] == addr) {
            FreeMemory(addr);
            StrTmp[i] = NULL;
            StrTmpIndex--;
            while(i < StrTmpIndex) {
                StrTmp[i] = StrTmp[i + 1];
                StrTmpLocalIndex[i] = StrTmpLocalIndex[i + 1];
                i++;
            }
            return;
        }
    }
}



int MemSize(void *addr){
    int i=0;
    int bits;
    do {
        if(addr < (void *)RAMBase || addr >= (void *)RAMEND) return i;
        bits = MBitsGet(addr);
        addr += RAMPAGESIZE;
        i+=RAMPAGESIZE;
    } while(bits != (PUSED | PLAST));
    return i;
}

void FreeMemory(void *addr) {
    int bits;
    do {
        if(addr < (void *)RAMBase || addr >= (void *)RAMEND) return;
        bits = MBitsGet(addr);
        MBitsSet(addr, 0);
        addr += RAMPAGESIZE;
    } while(bits != (PUSED | PLAST));
}


void FreeMemorySafe(void **addr){
	if(*addr!=NULL){
        FreeMemory(*addr);
        *addr=NULL;
	}
}

// test the stack for overflow
// this will probably be caused by a fault within MMBasic but it could also be
// caused by a very complex BASIC expression
static void inline __attribute__((always_inline)) TestStackOverflow(void) {
	unsigned int currstack=__get_MSP();
	if(currstack < (unsigned int)STACKLIMIT){
		error("Expression is too complex");
	}
}



void InitHeap(void) {
    int i;
    for(i = 0; i < 128; i++) mmap[i] = 0;
    for(i = 0; i < MAXTEMPSTRINGS; i++) StrTmp[i] = NULL;
    MBitsSet((unsigned char *)RAMEND, PUSED | PLAST);
    StrTmpIndex = TempMemoryIsChanged = 0;
}




/***********************************************************************************************************************
 Private memory management functions
************************************************************************************************************************/


unsigned int MBitsGet(void *addr) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)RAMBase);
    p = (void *)&mmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    return (*p >> i) & ((1 << PAGEBITS) -1);
}



void MBitsSet(void *addr, int bits) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)RAMBase);
    p = (void *)&mmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    *p = (bits << i) | (*p & (~(((1 << PAGEBITS) -1) << i)));
}


void *GetMemory(size_t size) {
    TestStackOverflow();
//void *getheap(int size) {
    uint64_t *i;
    unsigned int j, n, k;
    unsigned char *addr;
    k = j = n = (size + RAMPAGESIZE - 1)/RAMPAGESIZE;                         // nbr of pages rounded up
    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBase; addr -= RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            if(--n == 0) {                                          // found a free slot
                j--;
                MBitsSet(addr + (j * RAMPAGESIZE), PUSED | PLAST);     // show that this is used and the last in the chain of pages
                while(j--) MBitsSet(addr + (j * RAMPAGESIZE), PUSED);  // set the other pages to show that they are used
                i=(uint64_t *)addr;
                while(k--){
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                	*i++ = 0;
                }
                return (void *)addr;
            }
        } else
            n = j;                                                  // not enough space here so reset our count
    }
    // out of memory
    LocalIndex = 0;
    ClearTempMemory();                                              // hopefully this will give us enough to print the prompt
    error("Not enough memory");
    return NULL;                                                    // keep the compiler happy
}


int FreeSpaceOnHeap(void) {
    unsigned int nbr;
    unsigned char *addr;
    nbr = 0;
    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBase; addr -= RAMPAGESIZE)
        if(!(MBitsGet(addr) & PUSED)) nbr++;
    return nbr * RAMPAGESIZE;
}



unsigned int UsedHeap(void) {
    unsigned int nbr;
    unsigned char *addr;
    nbr = 0;
    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBase; addr -= RAMPAGESIZE)
        if(MBitsGet(addr) & PUSED) nbr++;
    return nbr * RAMPAGESIZE;
}



