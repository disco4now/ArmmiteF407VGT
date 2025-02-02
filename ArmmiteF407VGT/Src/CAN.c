/*-*****************************************************************************
MMBasic for STM32F407 [VET6] (Armmite F4)

CAN.c

Handles the CAN command.

Copyright 2011-2024 Geoff Graham and  Peter Mather.
Copyright 2024      Gerry Allardice.

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
/********** CAN implementation MMBasic on Armmite F407 *********************************
 *  One CAN interface is supported. The pins are allocated as below.
 *  Device                   CAN-H        CAN-L      Shared with
 *
 *
 *  The CAN clock is 42Mhz derived i.e. 168/4

 *
 * Commands to interface the CAN
 * CAN OPEN index,speed,mode[,prescaler,seg1,seg2,sjw]
 * CAN CLOSE
 * CAN START
 * CAN STOP
 * CAN FILTER index,eid,type,config,id1,id2
 * CAN SEND id,eid,rtr,dlc,msg,ret
 * CAN READ fifo,id,eid,rtr,dlc,msg,fmi,ret

 * **************************************************************
 * canopen   HAS_64PINS Pin allocations  CANx     Shares pins with
 *           HAS_100PINS
 *           HAS_144PINS
 *           CANRX        CANTX           Used
 * -------   ---------- ----------------  -------- --------------
 * 1         PB8/95      PB9/96           CAN1     PWM2B,2C  (PWM2A,2B on Feather)
 * 2         PD0/81      PD1/82           CAN1     FSMC D2,D3
 *
 *
 *
 CAN_1A_RX               (HAS_100PINS ? 95  : (HAS_144PINS ? 139 : 61))      //PB8  PB8  Also  PWM2B PWM2A Feather
 CAN_1A_TX               (HAS_100PINS ? 96  : (HAS_144PINS ? 140 : 62))      //PB9  PB9  Also  PWM2C PWM2B Feather
 CAN_2A_RX               (HAS_100PINS ? 81  : 114)                           //PD0  PD1  Also SSD1963 D2 Not Available on 64pin
 CAN_2A_TX               (HAS_100PINS ? 82  : 115)                           //PD1  PB9  Also SSD1963 D3 Not Available on 64pin
 */


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "CAN.h"


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan;
//FDCAN_ClkCalUnitTypeDef sCcuConfig;
CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;



/*

  CAN_ID_STD                  (0x00000000U)   < Standard Id
  CAN_ID_EXT                  (0x00000004U)   < Extended Id

 CAN_FILTERMODE_IDMASK       (0x00000000U)    < Identifier mask mode
 CAN_FILTERMODE_IDLIST       (0x00000001U)    < Identifier list mode

 CAN_RTR_DATA                (0x00000000U)   < Data frame
 CAN_RTR_REMOTE              (0x00000002U)   < Remote frame

 CAN_RX_FIFO0                (0x00000000U)   < CAN receive FIFO 0
 CAN_RX_FIFO1                (0x00000001U)   < CAN receive FIFO 1

 CAN_TX_MAILBOX0             (0x00000001U)  Tx Mailbox 0
 CAN_TX_MAILBOX1             (0x00000002U)  Tx Mailbox 1
 CAN_TX_MAILBOX2             (0x00000004U)  Tx Mailbox 2

 CAN_FILTER_DISABLE          (0x00000000U)   !< Disable filter
 CAN_FILTER_ENABLE           (0x00000001U)   !< Enable filter

 CAN_MODE_NORMAL             (0x00000000U)                               < Normal mode
 CAN_MODE_LOOPBACK           ((uint32_t)CAN_BTR_LBKM)                    < Loopback mode
 CAN_MODE_SILENT             ((uint32_t)CAN_BTR_SILM)                    < Silent mode
 CAN_MODE_SILENT_LOOPBACK    ((uint32_t)(CAN_BTR_LBKM | CAN_BTR_SILM))   < Loopback combined with silent mode

 CAN_INITSTATUS_FAILED       (0x00000000U) < CAN initialization failed
 CAN_INITSTATUS_SUCCESS      (0x00000001U) < CAN initialization OK

*/
char canmode=0;	  //CAN mode not set.
void cmd_can(void) {
	int speed,i,cansave;
    char *p;

    if(checkstring(cmdline, "CLOSE")) {

        /* Stop the CAN module */
        HAL_CAN_Stop(&hcan);
    	HAL_CAN_DeInit(&hcan);

      	if (canopen==1 ) {
      		canopen=0;
    		if(ExtCurrentConfig[CAN_1A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
    		if(ExtCurrentConfig[CAN_1A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_TX, EXT_NOT_CONFIG, 0);
    	}
      	if (canopen==2 ) {
      	    canopen=0;
      	   if(ExtCurrentConfig[CAN_2A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
      	   if(ExtCurrentConfig[CAN_2A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_TX, EXT_NOT_CONFIG, 0);
      	}

      	canopen=0;
        return;
    }

    if(checkstring(cmdline, "START")) {
    	if (!canopen) error("CAN not open");
        /* Start the CAN module */
        HAL_CAN_Start(&hcan);
        return;
    }

    if(checkstring(cmdline, "STOP")) {
    	if (!canopen) error("CAN not open");
        /* Stop the CAN module */
        HAL_CAN_Stop(&hcan);
        return;
    }

    /* CAN SEND id,eid,rtr,dlc,msg,ret
     * Adds one message to the Fifo TXBuffer.
     * Returns 0 if  message is added.
     */
    if((p = checkstring(cmdline, "SEND")) != NULL) {
    	 if (!canopen) error("CAN not open");
    	 uint8_t TxData[8];
    	 uint32_t TxMailbox;
    	 int i;
    	 int *ret,eid,rtr,dlc;
    	 uint32_t id;
    	 void *ptr1 = NULL;
    	 uint8_t *msg=NULL;
    	// union car
    	// {
    	// 	uint64_t iTxBuffer;
    	// 	uint8_t cTxBuffer[8];
    	// }mybuff;

         getargs(&p, 11, ",");
    	 if(!(argc == 11 )) error("Incorrect number of arguments");
    	 eid=getint(argv[2],0,1);
    	 if(eid){
    	     id=getint(argv[0],0,0x1FFFFFFF);
    	 }else{
    		 id=getint(argv[0],0,0x7FF);
    		 //MMPrintString("ID=");PIntHC(id);PRet();
    	 }
    	 rtr=getint(argv[4],0,1);
       	 dlc=getint(argv[6],0,8);
    	// mybuff.iTxBuffer=getinteger(argv[8]);
    	 ptr1 = findvar(argv[8], V_FIND );
    	 msg = (uint8_t *)ptr1;


    	 ret = findvar(argv[10], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for ret");

    	 if((HAL_CAN_GetTxMailboxesFreeLevel(&hcan))>0){
    		// PIntH(id);MMPrintString(" txBuffer FREE \r\n");
    	  if (eid){
    		  TxHeader.IDE = CAN_ID_EXT;
    		  TxHeader.ExtId = id;
    	  }else{
    		  TxHeader.IDE = CAN_ID_STD;
    		  TxHeader.StdId = id;
    	  }
    	  if (rtr){
    	     TxHeader.RTR = CAN_RTR_REMOTE;
    	  }else{
    		 TxHeader.RTR = CAN_RTR_DATA;
    	  }
    	   TxHeader.DLC =dlc;
    	   TxHeader.TransmitGlobalTime = DISABLE;

    	  // for (i=0;i<8;i++){TxData[7-i]=mybuff.cTxBuffer[i];}
    	   for (i=0;i<8;i++){TxData[7-i]=msg[i];}

    	   if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK){
    		   *ret=1;
    	   }else{
    		   *ret=0;
    	   }
    	  }else{
    	    *ret=1;
    	     PIntH(id);MMPrintString(" txBuffer busy \r\n");
    	  }
    	   return;

    }


    /* CAN READ fifo,id,eid,rtr,dlc,msg,fmi,ret
     * Reads one message from the nominated Fifo RXBuffer.
     * Returns 0 if no message is available, else returns the number of messages.
     */
    if((p = checkstring(cmdline, "READ")) != NULL) {
    	 if (!canopen) error("CAN not open");
    	 uint8_t RxData[8];
    	 int i,fifo;
    	 int *ret,*eid,*dlc,*rtr,*fmi;
    	 uint32_t *id;
    	 uint8_t *msg;

         getargs(&p, 15, ",");
    	 if(!(argc == 15 )) error("Incorrect number of arguments");
    	 fifo=getint(argv[0],0,1);
    	 // get the  variables
    	 id = findvar(argv[2], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for id");
    	 eid = findvar(argv[4], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for eid");
    	 rtr = findvar(argv[6], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for rtr");
    	 dlc = findvar(argv[8], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for dl");
    	 msg = findvar(argv[10], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for msg");
    	 fmi = findvar(argv[12], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for fmi");
    	 ret = findvar(argv[14], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for ret");

    	 if (fifo==0){
    	     *ret = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
    	     if (*ret) {
    	         HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);
    	         // PIntH(RxHeader.Identifier);
    	         if (RxHeader.IDE == CAN_ID_STD) {
    	        	 *eid=0;
    	        	 *id=RxHeader.StdId;
    	         }else{
    	        	 *eid=1;
    	        	 *id=RxHeader.ExtId;
    	         }
    	         *dlc=(RxHeader.DLC);
    	         if (RxHeader.RTR==CAN_RTR_REMOTE){
    	        	 *rtr=1;
    	        	 *dlc=0;
    	         }else{
    	        	 *rtr=0;
    	         }

    	         *fmi=RxHeader.FilterMatchIndex;
    	        // PInt(*filterno);
    	       	 // for (i=0;i<8;i++){PIntH(RxData[i]);};PRet();
    	    	 for (i=0;i<8;i++){msg[7-i]=RxData[i];}
    	        // *dlc=(RxHeader.DLC>>16);

    	         for (i=*dlc;i<8;i++){msg[7-i]=0;}
    	        // PInt(*ret);MMPrintString("rec0 \r\n");
    	     }

    	  }else{
    		  /* Retrieve next message from Rx FIFO 1 */
     	     *ret = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO1);
     	     if (*ret) {
     	         HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO1, &RxHeader, RxData);
     	         // PIntH(RxHeader.Identifier);
    	         if (RxHeader.IDE == CAN_ID_STD) {
    	        	 *eid=0;
    	        	 *id=RxHeader.StdId;
    	         }else{
    	        	 *eid=1;
    	        	 *id=RxHeader.ExtId;
    	         }
    	         *dlc=(RxHeader.DLC);
       	         if (RxHeader.RTR==CAN_RTR_REMOTE){
        	     	*rtr=1;
        	     	*dlc=0;
        	     }else{
        	       *rtr=0;
        	     }
    	         *fmi=RxHeader.FilterMatchIndex;
    	         //PInt(*filterno);
     	    	 // for (i=0;i<8;i++){PIntH(RxData[i]);};PRet();
     	    	 for (i=0;i<8;i++){msg[7-i]=RxData[i];}
     	         for (i=*dlc;i<8;i++){msg[7-i]=0;}
     	        // PInt(*ret);MMPrintString("rec1 \r\n");
     	     }
    	 }
        return;
    }


    /* CAN FILTER index,idtype,type,config,id1,id2
     * index: Filter number 0-27  -Filter are evaluated in order.
     * idtype: 0 for STDID, 1 for EXTID
     * type:    0-2 0=Classic Mask 1=Range with EIDM applied  2=Range filter with no EIDM applied
     * config:  0-2 0=Disable 1=Accept to FIFO0 2=Accept to FIFO1
     * id1:    The CAN message ID associated with the filter
     * id2:    The MASK or second ID associated with the filter
     *
     * type values
     * -----------
       0 = CAN_FILTERMODE_IDMASK  0       (0x00000000U)  Classic filter: FilterID1 = filter, FilterID2 = mask  (EIDM mask IS applied)
       1 = CAN_FILTERMODE_IDLIST  1       (0x00000001U)  Two Ids: FilterID1 or FilterID2                       (EIDM mask IS applied)
       2 = CAN_FILTERMODE_IDMASK_NO_EIDM                 Classic filter: FilterID1 = filter, FilterID2 = mask  (EIDM mask NOT applied)

     * Config values
     * ------------------
     * 0 = DISABLE FILTER
     * 1 = CAN_FILTER_FIFO0            (0x00000000U)          Filter FIFO 0 assignment for filter x
     * 2 = CAN_FILTER_FIFO1            (0x00000001U)          Filter FIFO 1 assignment for filter x



      CAN_FILTERSCALE_16BIT       (0x00000000U)   < Two 16-bit filters
      CAN_FILTERSCALE_32BIT       (0x00000001U)   < One 32-bit filter
      CAN_FILTER_DISABLE          (0x00000000U)   < Disable filter
      CAN_FILTER_ENABLE           (0x00000001U)   < Enable filter


     CAN FILTER index,idtype,type,config,id1,id2
     */

    if((p = checkstring(cmdline, "FILTER")) != NULL) {
    	if (!canopen) error("CAN not open");
    	uint32_t idtype,index,type,config,id1,id2;
       	getargs(&p, 11, ",");
    	if(!(argc == 11 )) error("Incorrect number of arguments");
    	index=getint(argv[0],0,27);
    	idtype=getint(argv[2],0,1);
    	type=getint(argv[4],0,2);
    	config=getint(argv[6],0,2);
       	if(idtype){
    		id1=getint(argv[8],0,0x1FFFFFFF);  //ID
    		id2=getint(argv[10],0,0x1FFFFFFF); //ID or MASK
     	}else{
    		id1=getint(argv[8],0,0x7FF);
    		id2=getint(argv[10],0,0x7FF);

    	}
       	/* Configure filter */

    	  sFilterConfig.FilterBank = index;   // 0 to 27
    	  if (type==0 || type==2 )sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;            //MASK
    	  if (type==1 )sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;  //Two IDS
    	  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //CAN_FILTERSCALE_16BIT

    	  if(idtype){  //extid

    	      sFilterConfig.FilterIdHigh = (id1>>13 ) & 0xFFFF;
    	      sFilterConfig.FilterIdLow = ((id1<<3) | 4 )  & 0xFFFF;  //Set extid
    	      sFilterConfig.FilterMaskIdHigh = id2>>13 & 0xFFFF ;
    	      if (type==2 ){
    	    	 sFilterConfig.FilterMaskIdLow = ((id2<<3) ) & 0xFFFF;  //DONT Set extid in mask
    	      }else{
    	    	 sFilterConfig.FilterMaskIdLow = ((id2<<3) | 4 ) & 0xFFFF;  //Set extid in mask
    	      }

    	 }else{

    		 sFilterConfig.FilterIdHigh = (id1<<5 ) & 0xFFE0;
    		 sFilterConfig.FilterIdLow = 0;//((id1<<3) & 0 )  & 0xFFFF;    //  DONT Set extid
    		// sFilterConfig.FilterIdLow = 4;//((id1<<3) & 0 )  & 0xFFFF;  //Set extid
    		 sFilterConfig.FilterMaskIdHigh = (id2<<5) & 0xFFE0 ;
   	         if (type==2 || type==1){
   	        	sFilterConfig.FilterMaskIdLow = 0 ;//((id2<<3) | 0 ) & 0xFFFF;  //Dont Set extid mask if its an ID or EIDM not applied
   	         }else{
   	        	sFilterConfig.FilterMaskIdLow = 4 ;//((id2<<3) | 0 ) & 0xFFFF;  //Set extid  mask
   	         }

    	 }
    	 if(config==0){
    		 sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  //CAN_FILTER_FIFO0
    		 sFilterConfig.FilterActivation = DISABLE;
    	 }
    	 if(config==1){
    	     sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  //CAN_FILTER_FIFO0
    	     sFilterConfig.FilterActivation = ENABLE;
    	 }
    	 if(config==2){
    		 sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;  //CAN_FILTER_FIFO1
    		 sFilterConfig.FilterActivation = ENABLE;
    	 }

    	 sFilterConfig.SlaveStartFilterBank = 27;
         if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");

         return;
     }

/*                         5                       13
	CAN OPEN index,speed,mode[,prescale,seg1,seg2,sjw]

 */
    if((p = checkstring(cmdline, "OPEN")) != NULL) {
       int prescale,seg1,seg2,sjw;
       if (canopen) error("Already open");

    	getargs(&p, 13, ",");
    	//getargs(&p, 5, ",");
        if(argc != 5 && argc !=13) error("Incorrect argument count");

        cansave=canopen;canopen=0;
        speed = getinteger(argv[2]);
        if (!(speed==0||speed==125000||speed==250000||speed==500000||speed==1000000))error("Valid speeds are 0,125000,250000,500000,1000000");
        if (speed==0 && argc != 13)error(" prescaler,seg1,seg2 and sjw required if speed is 0");
        if (speed==0){
        	prescale = getinteger(argv[6]);
        	seg1 = getint(argv[8],1,16);
        	seg2 = getint(argv[10],1,8);
        	sjw = getint(argv[12],1,4);
        }
        canopen=getint(argv[0],1,2);
        if (canopen==2  && HAS_64PINS)error("Only index 1 available on 64 Pin chip");
        canmode = getint(argv[4],0,2);
        //canopen=cansave;

        if (canopen==1) {
        	cansave=canopen;canopen=0;
        	CheckPin(CAN_1A_RX, CP_CHECKALL);  //Shared with PWM2A
            CheckPin(CAN_1A_TX, CP_CHECKALL);  //Shared with PWM2B
            ExtCurrentConfig[CAN_1A_RX] = EXT_COM_RESERVED;
        	ExtCurrentConfig[CAN_1A_TX] = EXT_COM_RESERVED;
        	canopen=cansave;
        }
        if (canopen==2) {
           cansave=canopen;canopen=0;
           CheckPin(CAN_2A_RX, CP_CHECKALL);  //SHARED with Parallel LCD
           CheckPin(CAN_2A_TX, CP_CHECKALL);  //SHARED with Parallel LCD
           ExtCurrentConfig[CAN_2A_RX] = EXT_COM_RESERVED;
           ExtCurrentConfig[CAN_2A_TX] = EXT_COM_RESERVED;
           canopen=cansave;
        }

        /* Initializes the CAN peripheral */
        hcan.Instance = CAN1;
        if(canmode==1){
        	hcan.Init.Mode = CAN_MODE_SILENT_LOOPBACK;
        }else if(canmode==2){
            hcan.Init.Mode = CAN_MODE_LOOPBACK;
        }else{
            hcan.Init.Mode = CAN_MODE_NORMAL;
        }

         hcan.Init.TimeTriggeredMode = DISABLE;
         hcan.Init.AutoBusOff = DISABLE;
         hcan.Init.AutoWakeUp = DISABLE;
         hcan.Init.AutoRetransmission = ENABLE;
         hcan.Init.ReceiveFifoLocked = DISABLE;
         hcan.Init.TransmitFifoPriority = DISABLE;


/*   The Armmite F4 sets clock rate at 42MHz.  i.e.168/4  so use 42Mhz in table below.
 *   see http://www.bittiming.can-wiki.info/ for calculating values for various speeds and
 *   sample points.
 */

/**** Clock parameters at 71.4% sample point ***************************************
       Speed    Pre    Max    Seg1   Seg2  JSW  Sample
		KHz     Scale  tq                       Point
        125	 	16    21  	 14		6     1    71.4
        250	 	 8    21	 14	    6     1    71.4
		500	 	 4 	  21	 14	    6     1    71.4
		1000 	 2	  21     14	    6	  1    71.4
***********************************************************************************/
/**** Clock parameters at 85.75 to 87.5% sample point ******************************
      Speed    Pre    Max    Seg1   Seg2  JSW  Sample
    	KHz    Scale  tq                       Point
        125	  	21    16  	 13		2     1    87.5
        250	  	12    14	 11	    2     1    85.7
     	500	  	 6 	  14	 11	    2     1    85.7
      	1000  	 3	  14     11	    2	  1    85.7
 ***********************************************************************************/
      if (speed==125000){
	   //samples set at at 87.5%
    	 hcan.Init.Prescaler = 21;
	     hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	     hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
	     hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
       // MMPrintString("Speed 125000 \r\n");
      }else if(speed==250000){
         hcan.Init.Prescaler = 12;
    	 hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
         hcan.Init.TimeSeg1 = CAN_BS1_11TQ;
         hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
      }else if(speed==500000){
         hcan.Init.Prescaler = 6;
       	 hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
         hcan.Init.TimeSeg1 = CAN_BS1_11TQ;
         hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
      }else if(speed==1000000){
         hcan.Init.Prescaler = 3;
       	 hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
         hcan.Init.TimeSeg1 = CAN_BS1_11TQ;
         hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
      }else if(speed==0){
         hcan.Init.Prescaler = prescale;
         hcan.Init.TimeSeg1 = (seg1-1)<<16;
         hcan.Init.TimeSeg2 = (seg2-1)<<20;
         hcan.Init.SyncJumpWidth = (sjw-1)<<24;
      }

       HAL_CAN_DeInit(&hcan);
       if (HAL_CAN_Init(&hcan) != HAL_OK)error("Failed to initial CAN");
       //Disable all filters.
       for(i=0;i<28;i++){
         sFilterConfig.FilterBank = i;   // 0 to 27
         sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
         sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
         sFilterConfig.FilterActivation = DISABLE;
         if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");
       }
       //Now set a default filter to accept all messages into FIFO0
         sFilterConfig.FilterBank = 0;   // 0 to 27
         sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;            //MASK
         sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //CAN_FILTERSCALE_16BIT
         sFilterConfig.FilterIdHigh = 0;
         sFilterConfig.FilterIdLow =  0;  //Dont Set extid
         sFilterConfig.FilterMaskIdHigh = 0 ;
         sFilterConfig.FilterMaskIdLow = 0;  //DONT Set extid in mask
         sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  //CAN_FILTER_FIFO0
         sFilterConfig.FilterActivation = ENABLE;
         sFilterConfig.SlaveStartFilterBank = 27;
         if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");
         HAL_Delay(400);

       return;
     }
     error("Invalid syntax");
 }
