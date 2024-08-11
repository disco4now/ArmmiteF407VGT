/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

CAN.c

Handles the SPI command.

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
 *  144 Pin H743             B7(91)
 *  100 Pin H743
 *
 *  The CAN clock is 84Mhz derived i.e. 168/2

 *
 * Commands to interface the CAN
 * CAN OPEN canopen,speed
 * CAN CLOSE
 * CAN START
 * CAN STOP
 * CAN FILTER index,idtype,type,config,id1,id2
 * CAN SEND id,eid,dlc,msg,ret
 * CAN READ fifo,id,eid,rtr,dlc,msg,fmi,ret

 * **************************************************************
 * canopen   HAS_100PINS Pin allocations  CANx     Shares pins with
 *           CANRX        CANTX           Used
 * -------   ---------- ----------------  -------- --------------
 * 1         PB8/95      PB9/96           CAN1     PWM2B,2C
 * 2         PD0/81      PD1/82           CAN1     FSMC
 * 3         n/a
 * 4         n/a
 * 5         n/a
 * 6         n/a
 * 7         LOOPBACK                     CAN1
 * 8         LOOPBACK                     CAN1
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

/3. Start the FDCAN module using HAL_FDCAN_Start function. At this level the node is active on the bus: it can
send and receive messages.
4. The following Tx control functions can only be called when the CAN module is started:
– HAL_CAN_AddMessageToTxFifoQ
– HAL_CAN_EnableTxBufferRequest
– HAL_CAN_AbortTxRequest
5. After having submitted a Tx request in Tx Fifo or Queue, it is possible to get Tx buffer location used to place
the Tx request thanks to HAL_FDCAN_GetLatestTxFifoQRequestBuffer API. It is then possible to abort later
on the corresponding Tx Request using HAL_FDCAN_AbortTxRequest API.
6. When a message is received into the FDCAN message RAM, it can be retrieved using the
HAL_FDCAN_GetRxMessage function.
7. Calling the HAL_FDCAN_Stop function stops the FDCAN module by entering it to initialization mode and
re-enabling access to configuration registers through the configuration functions listed here above.
8. All other control functions can be called any time after initialization phase, no matter if the FDCAN module is
started or stopped.
*/

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
void cmd_can(void) {
	int speed,i,cansave;
    char *p;

    if(checkstring(cmdline, "CLOSE")) {

        /* Stop the FDCAN module */
        HAL_CAN_Stop(&hcan);
    	HAL_CAN_DeInit(&hcan);

      	if (canopen==1 || canopen==4) {
      		canopen=0;
    		if(ExtCurrentConfig[CAN_1A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
    		if(ExtCurrentConfig[CAN_1A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_TX, EXT_NOT_CONFIG, 0);
    	}
      	if (canopen==2 || canopen==5) {
      	    canopen=0;
      	   if(ExtCurrentConfig[CAN_2A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
      	   if(ExtCurrentConfig[CAN_2A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_TX, EXT_NOT_CONFIG, 0);
      	}
     // 	if (canopen==3) {
      //	   if(ExtCurrentConfig[CAN_3A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_3A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
     // 	   if(ExtCurrentConfig[CAN_3A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_3A_TX, EXT_NOT_CONFIG, 0);
     // 	}

      	canopen=0;
        return;
    }

    if(checkstring(cmdline, "START")) {

        /* Start the FDCAN module */
        HAL_CAN_Start(&hcan);
        return;
    }

    if(checkstring(cmdline, "STOP")) {

        /* Stop the FDCAN module */
        HAL_CAN_Stop(&hcan);
        return;
    }

    /* CAN SEND id,eid,rtr,dlc,msg,ret
     * Adds one message to the Fifo TXBuffer.
     * Returns 0 if  message is added.
     */
    if((p = checkstring(cmdline, "SEND")) != NULL) {

    	 uint8_t TxData[8];
    	 uint32_t TxMailbox;
    	 int i;
    	 int *ret,eid,rtr,dlc;
    	 uint32_t id;
    	 union car
    	 {
    	 	uint64_t iTxBuffer;
    	 	uint8_t cTxBuffer[8];
    	 }mybuff;

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
    	 mybuff.iTxBuffer=getinteger(argv[8]);
    	 ret = findvar(argv[10], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for ret");

    	 /*##-4- Start the Transmission process #####################################*/
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

    	   for (i=0;i<8;i++){TxData[7-i]=mybuff.cTxBuffer[i];}
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


    	 /*##-5- Start the Reception process ########################################*/

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
      	//if(idtype){
       	//     sFilterConfig.IdType = FDCAN_EXTENDED_ID;
      	//}else{
        //	sFilterConfig.IdType = FDCAN_STANDARD_ID;
       	//}
       	//sFilterConfig.FilterIndex = index;
       	//sFilterConfig.FilterType = type;
       //	sFilterConfig.FilterConfig = config;
       // sFilterConfig.FilterID1 = id1;
       //	sFilterConfig.FilterID2 = id2;
       //	HAL_FDCAN_ConfigFilter(&hfdcan, &sFilterConfig);


          /*##-2- Configure the CAN Filter ###########################################*/

    	  sFilterConfig.FilterBank = index;   // 0 to 27
    	  if (type==0 || type==2 )sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;            //MASK
    	  if (type==1 )sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;  //Two IDS
    	  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //CAN_FILTERSCALE_16BIT

    	  if(idtype){  //extid
    		  //sFilterConfig.FilterIdHigh = (id1>>13 ) & 0xFFFF;
    		 // sFilterConfig.FilterIdLow = ((id1<<3) | 4 )  & 0xFFFF;  //Set extid
    		 // sFilterConfig.FilterMaskIdHigh = id2>>13 & 0xFFFF ;
    		 // sFilterConfig.FilterMaskIdLow = ((id2<<3) | 4 ) & 0xFFFF;  //Set extid

    	      sFilterConfig.FilterIdHigh = (id1>>13 ) & 0xFFFF;
    	      sFilterConfig.FilterIdLow = ((id1<<3) | 4 )  & 0xFFFF;  //Set extid
    	      sFilterConfig.FilterMaskIdHigh = id2>>13 & 0xFFFF ;
    	      if (type==2 ){

    	    	 sFilterConfig.FilterMaskIdLow = ((id2<<3) ) & 0xFFFF;  //DONT Set extid in mask

    	      }else{

    	    	 sFilterConfig.FilterMaskIdLow = ((id2<<3) | 4 ) & 0xFFFF;  //Set extid in mask
    	      }



    	    //  PIntHC(sFilterConfig.FilterIdHigh);
    	    //  PIntHC(sFilterConfig.FilterIdLow);
    	    //  PIntHC(sFilterConfig.FilterMaskIdHigh);
    	    //  PIntHC(sFilterConfig.FilterMaskIdLow);PRet();
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

    		    	   //sFilterConfig.FilterIdHigh = ((id1<<5) & 0xFFE0) | 8;     //  ID High
    		    	  // sFilterConfig.FilterIdLow = (id2<<5) & 0xFFE0;     //(id1<<5) & 0x0000;      // ID Low
    		    	  // sFilterConfig.FilterMaskIdHigh = ((id1<<5) & 0xFFE0) | 8;   // Mask Hi
    		    	   //sFilterConfig.FilterMaskIdLow = (id2<<5) & 0xFFE0;     //(id1<<5) & 0x0000;      // ID Low
    		    	   //sFilterConfig.FilterMaskIdHigh = 0x0;    //MASK
    		    	   //sFilterConfig.FilterMaskIdLow = 0x4;     //MASK

    		 // PIntHC(sFilterConfig.FilterIdHigh);
    		 // PIntHC(sFilterConfig.FilterIdLow);
    		//  PIntHC(sFilterConfig.FilterMaskIdHigh);
    		//  PIntHC(sFilterConfig.FilterMaskIdLow);PRet();
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


    	  // sFilterConfig.FilterBank = index;   // 0 to 13
    	  // sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  //CAN_FILTERMODE_IDLIST
    	  // sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //CAN_FILTERSCALE_16BIT

    	  // sFilterConfig.FilterIdHigh = (id1<<5 ) & 0xFFE0;
    	 //  sFilterConfig.FilterIdLow = 0;//((id1<<3) & 0 )  & 0xFFFF;  //Set extid
    	  // sFilterConfig.FilterMaskIdHigh = (id2<<5) & 0xFFE0 ;
    	  // sFilterConfig.FilterMaskIdLow = 4 ;//((id2<<3) | 0 ) & 0xFFFF;  //Set extid

    	   //sFilterConfig.FilterIdHigh = ((id1<<5) & 0xFFE0) | 8;     //  ID High
    	  // sFilterConfig.FilterIdLow = (id2<<5) & 0xFFE0;     //(id1<<5) & 0x0000;      // ID Low
    	  // sFilterConfig.FilterMaskIdHigh = ((id1<<5) & 0xFFE0) | 8;   // Mask Hi
    	   //sFilterConfig.FilterMaskIdLow = (id2<<5) & 0xFFE0;     //(id1<<5) & 0x0000;      // ID Low
    	   //sFilterConfig.FilterMaskIdHigh = 0x0;    //MASK
    	   //sFilterConfig.FilterMaskIdLow = 0x4;     //MASK

    	 //  PIntHC(sFilterConfig.FilterIdHigh);
    	 //  PIntHC(sFilterConfig.FilterIdLow);
    	  // PIntHC(sFilterConfig.FilterMaskIdHigh);
    	  // PIntHC(sFilterConfig.FilterMaskIdLow);PRet();

    	   //PIntHC(sFilterConfig.FilterIdLow);  //id
    	   //PIntHC(sFilterConfig.FilterIdHigh); //mask
    	   //PIntHC(sFilterConfig.FilterMaskIdLow);  //id
    	   //PIntHC(sFilterConfig.FilterMaskIdHigh);PRet();  //mask


    	  // sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;  //CAN_FILTER_FIFO1
    	  // sFilterConfig.FilterActivation = ENABLE;
    	  // sFilterConfig.SlaveStartFilterBank = 14;
    	  // if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");


       // sFilterConfig.FilterBank = 0;   // 0 to 13
       // sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  //CAN_FILTERMODE_IDLIST
       // sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT; //CAN_FILTERSCALE_16BIT
       // sFilterConfig.FilterIdHigh = 0x0000;
       // sFilterConfig.FilterIdLow = 0x0000;
       // sFilterConfig.FilterMaskIdHigh = 0x0111;    //MASK
       // sFilterConfig.FilterMaskIdLow = 0x0555;     //ID
       // sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;  //CAN_FILTER_FIFO1
       // sFilterConfig.FilterActivation = ENABLE;
       // sFilterConfig.SlaveStartFilterBank = 14;

       // if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");

       // sFilterConfig.FilterBank = 1;   // 0 to 13
       // sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  //CAN_FILTERMODE_IDLIST
       // sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //CAN_FILTERSCALE_16BIT
       // sFilterConfig.FilterIdHigh = 0x0333;
       // sFilterConfig.FilterIdLow = 0x3333;
       // sFilterConfig.FilterMaskIdHigh = 0x1FFF;
       // sFilterConfig.FilterMaskIdLow = 0xFFFF;
       // sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  //CAN_FILTER_FIFO1
       // sFilterConfig.FilterActivation = ENABLE;
       // sFilterConfig.SlaveStartFilterBank = 14;

       // if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");

    	//MMPrintString("added ... \r\n");
    	//MMPrintString("IdType:");PIntH(sFilterConfig.IdType);PRet();
    	//MMPrintString("FilterIndex:");PIntH(sFilterConfig.FilterIndex);PRet();
    	//MMPrintString("FilterType:");PIntH(sFilterConfig.FilterType);PRet();
    	//MMPrintString("FilterConfig:");PIntH(sFilterConfig.FilterConfig);PRet();
    	//MMPrintString("FilterID1:");PIntH(sFilterConfig.FilterID1);PRet();
    	//MMPrintString("FilterID2:");PIntH(sFilterConfig.FilterID2);PRet();

    	//HAL_FDCAN_ConfigGlobalFilter(&hfdcan, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    	//return;


   // }

    /*************************************************************************
     * CAN GLOBAL id,eid,idr,eidr

       Values for id and eid                values for idr and eidr
       ---------------------                -----------------------
       0=FDCAN_ACCEPT_IN_RX_FIFO0           0=FDCAN_FILTER_REMOTE
       1=FDCAN_ACCEPT_IN_RX_FIFO1		    1=FDCAN_REJECT_REMOTE
       2=FDCAN_REJECT

    *************************************************************************
    if((p = checkstring(cmdline, "GLOBAL")) != NULL) {

       	uint32_t id,eid,idr,eidr;
       	getargs(&p, 7, ",");
       	if(!(argc == 7 )) error("Incorrect number of arguments");
       	id=getint(argv[0],0,2);
       	eid=getint(argv[2],0,2);
       	idr=getint(argv[4],0,1);
       	eidr=getint(argv[6],0,1);
  //     	HAL_FDCAN_ConfigGlobalFilter(&hfdcan, id, eid, idr, eidr);
        return;
    }
    */


/*
	 0=FDCAN_FRAME_CLASSIC
	 1=FDCAN_FRAME_FD_BRS
*/

    if((p = checkstring(cmdline, "OPEN")) != NULL) {

      if (canopen) error("Already open");

    	getargs(&p, 5, ",");
        if(argc < 3) error("Incorrect argument count");
        canopen=getint(argv[0],1,9);
        //canopen=getinteger(argv[0]);
        //speed=getint(argv[2],125000,1000000);
        speed = getinteger(argv[2]);
        if (!(speed==125000||speed==250000||speed==500000||speed==1000000))error("Valid speeds are 125000,250000,5000000,1000000");


        if (canopen==1 || canopen==1) {
        	cansave=canopen;canopen=0;
        	CheckPin(CAN_1A_RX, CP_CHECKALL);  //Shared with PWM2A
            CheckPin(CAN_1A_TX, CP_CHECKALL);  //Shared with PWM2B
            ExtCurrentConfig[CAN_1A_RX] = EXT_COM_RESERVED;
        	ExtCurrentConfig[CAN_1A_TX] = EXT_COM_RESERVED;
        	canopen=cansave;
        }
        if (canopen==2 || canopen==2) {
           cansave=canopen;canopen=0;
           CheckPin(CAN_2A_RX, CP_CHECKALL);  //SHARED with Parallel LCD
           CheckPin(CAN_2A_TX, CP_CHECKALL);  //SHARED with Parallel LCD
           ExtCurrentConfig[CAN_2A_RX] = EXT_COM_RESERVED;
           ExtCurrentConfig[CAN_2A_TX] = EXT_COM_RESERVED;
           canopen=cansave;
        }
       // if (canopen==3) {
       //    usecan2=1;
       //    ExtCfg(CAN_3A_RX, EXT_COM_RESERVED, 0);
       //    ExtCfg(CAN_3A_TX, EXT_COM_RESERVED, 0);
       // }




        /* Initializes the CAN peripheral */
        hcan.Instance = CAN1;
        //if(canopen==4 || canopen==5){
       //       hcan.Init.Mode = CAN_MODE_SILENT; //CAN_MODE_SILENT_LOOPBACK;  //CAN_MODE_LOOPBACK;
        //     MMPrintString("Open Silent_MODE \r\n");
        if(canopen==9 || canopen==9){
        	 hcan.Init.Mode = CAN_MODE_LOOPBACK; //CAN_MODE_SILENT_LOOPBACK;  //CAN_MODE_LOOPBACK;
             //MMPrintString("Open Loopback \r\n");
        }else{
          hcan.Init.Mode = CAN_MODE_NORMAL;
        }

         hcan.Init.TimeTriggeredMode = DISABLE;
         hcan.Init.AutoBusOff = DISABLE;
         hcan.Init.AutoWakeUp = DISABLE;
         hcan.Init.AutoRetransmission = ENABLE;
         hcan.Init.ReceiveFifoLocked = DISABLE;
         hcan.Init.TransmitFifoPriority = DISABLE;

       //hfdcan.Init.NominalPrescaler = 0x1; /* tq = NominalPrescaler x (1/fdcan_ker_ck)
/*   The Armmite F4 sets clock rate at 42MHz.  i.e.168/4  so use 42Mhz in table below.
 *   see http://www.bittiming.can-wiki.info/ for calculating values for various speeds and
 *   sample points.
 */

/**** Clock parameters at 71.4% sample point ***************************************
       Speed       Pre    Max    Seg1   Seg2  JSW  Sample
		KHz        Scale  tq                       Point
        125	 0.0000	16    21  	 14		6     1    71.4
        250	 0.0000	 8    21	 14	    6     1    71.4
		500	 0.0000	 4 	  21	 14	    6     1    71.4
		1000 0.0000	 2	  21     14	    6	  1    71.4
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
	    hcan.Init.Prescaler = 16;
	    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	    hcan.Init.TimeSeg1 = CAN_BS1_14TQ;
	    hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
       // MMPrintString("Speed 125000 \r\n");
      }else if(speed==250000){
        hcan.Init.Prescaler = 8;
    	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
        hcan.Init.TimeSeg1 = CAN_BS1_14TQ;
        hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
      }else if(speed==500000){
        hcan.Init.Prescaler = 4;
       	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
        hcan.Init.TimeSeg1 = CAN_BS1_14TQ;
        hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
      }else if(speed==1000000){
        hcan.Init.Prescaler = 2;
       	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
        hcan.Init.TimeSeg1 = CAN_BS1_14TQ;
        hcan.Init.TimeSeg2 = CAN_BS2_6TQ;

      }


       HAL_CAN_DeInit(&hcan);
       if (HAL_CAN_Init(&hcan) != HAL_OK)error("Failed to initial CAN");
       //Reset all filters to count as 1 in filterIndex
       for(i=0;i<28;i++){
         sFilterConfig.FilterBank = i;   // 0 to 27
         sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
         sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
         sFilterConfig.FilterActivation = DISABLE;
         if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)error ("Filter configuration failed");
       }

       /*** Initially set the global filter to accept all messages to RX_FIFO1 **/
     //  HAL_FDCAN_ConfigGlobalFilter(&hfdcan, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
       return;
     }
     error("Invalid syntax");
 }






#ifdef LOOPBACK
/**
  * @brief  Configures the CAN, transmit and receive by polling
  * @param  None
  * @retval PASSED if the reception is well done, FAILED in other case
  */
HAL_StatusTypeDef CAN_Polling(void)
{
  CAN_FilterTypeDef  sFilterConfig;

  /*##-1- Configure the CAN peripheral #######################################*/
  CanHandle.Instance = CANx;

  CanHandle.Init.TimeTriggeredMode = DISABLE;
  CanHandle.Init.AutoBusOff = DISABLE;
  CanHandle.Init.AutoWakeUp = DISABLE;
  CanHandle.Init.AutoRetransmission = ENABLE;
  CanHandle.Init.ReceiveFifoLocked = DISABLE;
  CanHandle.Init.TransmitFifoPriority = DISABLE;
  CanHandle.Init.Mode = CAN_MODE_LOOPBACK;
  CanHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;
  CanHandle.Init.TimeSeg1 = CAN_BS1_4TQ;
  CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
  CanHandle.Init.Prescaler = 6;

  if(HAL_CAN_Init(&CanHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Configure the CAN Filter ###########################################*/
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if(HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
  {
    /* Filter configuration Error */
    Error_Handler();
  }

  /*##-3- Start the CAN peripheral ###########################################*/
  if (HAL_CAN_Start(&CanHandle) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }

  /*##-4- Start the Transmission process #####################################*/
  TxHeader.StdId = 0x11;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.DLC = 2;
  TxHeader.TransmitGlobalTime = DISABLE;
  TxData[0] = 0xCA;
  TxData[1] = 0xFE;

  /* Request transmission */
  if(HAL_CAN_AddTxMessage(&CanHandle, &TxHeader, TxData, &TxMailbox) != HAL_OK)
  {
    /* Transmission request Error */
    Error_Handler();
  }

  /* Wait transmission complete */
  while(HAL_CAN_GetTxMailboxesFreeLevel(&CanHandle) != 3) {}

  /*##-5- Start the Reception process ########################################*/
  if(HAL_CAN_GetRxFifoFillLevel(&CanHandle, CAN_RX_FIFO0) != 1)
  {
    /* Reception Missing */
    Error_Handler();
  }

  if(HAL_CAN_GetRxMessage(&CanHandle, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {
    /* Reception Error */
    Error_Handler();
  }

  if((RxHeader.StdId != 0x11)                     ||
     (RxHeader.RTR != CAN_RTR_DATA)               ||
     (RxHeader.IDE != CAN_ID_STD)                 ||
     (RxHeader.DLC != 2)                          ||
     ((RxData[0]<<8 | RxData[1]) != 0xCAFE))
  {
    /* Rx message Error */
    return HAL_ERROR;
  }

  return HAL_OK; /* Test Passed */
}

#endif

#ifdef POLLING


#endif
