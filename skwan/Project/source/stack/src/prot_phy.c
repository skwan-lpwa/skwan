/**
    Author:       Skyley Networks, Inc.
    Version:
    Description: 
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

// -------------------------------------------------
//   Compiler Options
// -------------------------------------------------
#include "compiler_options.h"

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include 	<string.h>
#include 	<stdlib.h>

#include 	"radio.h"
#include 	"sx1276.h"
#include 	"sx1276-Hal.h"
#include 	"sx1276-LoRa.h"
#include 	"sx1276-LoRaMisc.h"
#include	"smac.h"

#include	"skyley_stack.h"

//#define DEBUG_MEMORY
//#define DEBUG_SEND
//#define DEBUG_RECV

#ifdef USE_MEMCPY_F
#define memcpy memcpy_f
#endif

// -------------------------------------------------
//   Global PIBs
// -------------------------------------------------
SK_UB			gnPHY_LowerLayer;
SK_UB 			gnPHY_UpperLayer;
SK_UB  			gnPHY_CurrentChannel;
SK_UB			gnPHY_CurrentTRX;

//Interrupt flags
volatile SK_UB 	gnPHY_TxCompleted;
volatile SK_UB 	gnPHY_CCABusy;
volatile SK_BOOL 	gnPHY_SymbolTimerCompleted;

//for debug flag
SK_UB			gnPHY_TestMode;


// -------------------------------------------------
//   Function proto types
// -------------------------------------------------
static void PostDataConfirm(SK_UB layer, SK_UB status);
void PostDataInd(SK_UB* data, SK_UH len, SK_UB rssi);


// -------------------------------------------------
//   ES920LR Smac dependant func
// -------------------------------------------------
//From SMAC
extern void SkTransmitData(uint8_t* buf, uint8_t length);
void SK_PHY_StartRx(void);


// -------------------------------------------------
//   Working variables
// -------------------------------------------------
rxPacket_t *gnRxPacket;
SK_UB gnPHY_RecvData[128];
SK_UB gnPHY_SndData[128];

extern SK_UH gnTickSlotCount;


// -------------------------------------------------
//   State machine
// -------------------------------------------------
SK_STATESTART(PHY);	


// -------------------------------------------------
//   Upper/Lower layer setting
// -------------------------------------------------

void SK_PHY_SetLowerLayer(SK_UB layer) {
	gnPHY_LowerLayer = layer;
}

void SK_PHY_SetUpperLayer(SK_UB layer) {
	gnPHY_UpperLayer = layer;
}

SK_UB SK_PHY_GetLowerLayer(void) {
	return gnPHY_LowerLayer;
}

SK_UB SK_PHY_GetUpperLayer(void) {
	return gnPHY_UpperLayer;
}


// -------------------------------------------------
//   Initialize PHY layer
// -------------------------------------------------


void SK_PHY_Init(void) {
	// Layer work area default values
	gnPHY_LowerLayer		= 0;
	gnPHY_UpperLayer		= SK_LAYER_SS_MAC;
	
	gnPHY_CurrentChannel 	= 24;
	gnPHY_CurrentTRX		= SK_PHY_RX_ON;
	
	gnPHY_TestMode 			= 0;

	SSMac_Wakeup();
	
	SK_PHY_StartRx();

	SK_INITSTATE(PHY);
}


// -------------------------------------------------
//   PHY Main Task
// -------------------------------------------------

void SK_PHY_Task(void) {
	static SK_UB nCmd,nResID,*pPkt;
	static SK_PD_DATA_REQUEST *PdReq;
	static SK_MCPS_DATA_REQUEST *MdReq;
	
	//SK_STATEADD(PHY,1);	
	SK_STATEADD(PHY,2);

	// -------------------------------------------------
	if (SK_GetMessage(SK_LAYER_PHY,&nResID,&nCmd,(SK_VP *)&pPkt)==SK_E_OK) {
		// メッセージを受信した
		switch(nCmd) {
		case SK_MCPS_DATA_REQUEST_CMD:
		case SK_PD_DATA_REQUEST_CMD:{
			SK_UB* data_buf;
			SK_UH data_len;
			
			if( gnPHY_CurrentTRX == SK_PHY_TRX_OFF ){
				SK_PHY_Wakeup();
			}
	
			if( nCmd == SK_PD_DATA_REQUEST_CMD ){
				PdReq = (SK_PD_DATA_REQUEST *)pPkt;
				data_buf = PdReq->m_Psdu;
				data_len = PdReq->m_PsduLength;
			} else {
				MdReq = (SK_MCPS_DATA_REQUEST *)pPkt;
				data_buf = MdReq->m_Msdu;
				data_len = MdReq->m_MsduLength;
			}
			
			memcpy(gnPHY_SndData, data_buf, data_len);
			gnPHY_TxCompleted = 0;
			gnPHY_CCABusy = 0;
			
		  	SkTransmitData(gnPHY_SndData, data_len);
			//SendPacket(gnPHY_SndData, data_len);
			  
			SK_WAITFOR(3000, (gnPHY_TxCompleted==1 || gnPHY_CCABusy == 1), PHY, 2);
			
			if( gnPHY_TxCompleted == 0 ){
			  	if( gnPHY_CCABusy == 1 ){
					//driver status cca busy
				  	SS_STATS(phy.busy);
					PostDataConfirm(nResID, SK_PHY_BUSY);				
			  	} else {
					//driver status error
				  	SS_STATS(phy.err);
					PostDataConfirm(nResID, SK_PHY_TRX_OFF);
				}
			} else {
			  	SS_STATS(phy.send);
				PostDataConfirm(nResID, SK_PHY_SUCCESS);
			}
			
			break;
		}
		
		default:{
			break;
		}
		}
		
		// -------------------------------------------
		// Free memory space
		if (pPkt  != NULL) SK_FreeMemory(pPkt);
	}
	// -------------------------------------------------

	SK_STATEEND(PHY);

#ifdef DEBUG_MEMORY
	{
		static SK_UW lasttime = 0;
		static SK_UW lasttime2 = 0;
		static SK_UB cnt = 0;
		
		if( ((SK_UW)(SK_GetLongTick() - lasttime)) > 1000 ){
			SK_print_hex( SK_CountFreeDataMemory(), 4); SK_print("\r\n");
			lasttime = SK_GetLongTick();
		}
		
		if( SK_CountFreeDataMemory() < 0x0A ){
			if( ((SK_UW)(SK_GetLongTick() - lasttime2)) > 3000 ){
				SK_print_hex(SK_CountFreeDataMemory(), 4); SK_print("\r\n");
				cnt++;
				lasttime2 = SK_GetLongTick();
			}
			if( cnt > 10 ){
				SK_print("--------------------------------\r\n");
				SK_print_hex(SK_CountFreeDataMemory(), 4); SK_print("\r\n");
				cnt = 0;
			}
		}
	}
#endif
}



// -------------------------------------------------
//   Process RF interrupts
// -------------------------------------------------
void SK_PHY_Interrupt() {

}


// -------------------------------------------------
//   Make RF deep sleep mode 1
// -------------------------------------------------
void SK_PHY_Sleep( void ) {
	gnPHY_CurrentTRX = SK_PHY_TRX_OFF;
	RF_Sleep();
}


// -------------------------------------------------
//   Wakeup RF
// -------------------------------------------------
void SK_PHY_Wakeup( void ){
	gnPHY_CurrentTRX = SK_PHY_RX_ON;
	RF_Wakeup();
}


// -------------------------------------------------
//   Restart RF RX
// -------------------------------------------------

void SK_PHY_StartRx(void){
	//note: rxPacket_tは先頭にヘッダ領域があるがSkWANでは使わない
	gnRxPacket = (rxPacket_t*)gnPHY_RecvData;
	SMAC_RxStart( gnRxPacket );
}


// -------------------------------------------------
//   Change channel
// -------------------------------------------------
//
// SkWANの論理チャンネル番号はARIBの番号と同じで24-38の15個
// これを実際の実装系の周波数にマップする
// 0 <- ARIB T108 ch24 (920.6)
// 1 <- ch25 (920.8)
// ...
// 14 <- ch38 (923.4)
// 50msec休止時間を採用しているため、Ch39以降は使わない

SK_BOOL SK_PHY_ChangeChannel(SK_UB ch) {
  	SK_UB target;
	
  	if( ch < 24 ) return FALSE;
	
	target = ch - 24;
	
	if( target > 14 ) return FALSE;
	
	if( gnPHY_CurrentTRX == SK_PHY_TRX_OFF ){
		SK_PHY_Wakeup();
	}
	
	gnPHY_CurrentChannel = ch;
	
	SMAC_SetChannel(target, BANDWIDTH125);
	
	return TRUE;
}


// -------------------------------------------------
//   Symbol timer
// -------------------------------------------------

void SK_PHY_StartSymbolTimer(SK_UW symbols) {
	gnPHY_SymbolTimerCompleted = FALSE;
	SymbolTimer_Go(symbols);
}


void SK_PHY_StopSymbolTimer() {
	SymbolTimer_Go(0);
}


// -------------------------------------------------
//   Post PD_DATA.confirm to upper layer
// -------------------------------------------------
static void PostDataConfirm(SK_UB layer, SK_UB result){
	 SK_MCPS_DATA_CONFIRM 	*lMdCon;
	
	if( SK_AllocCommandMemory((SK_VP *)&lMdCon) != SK_E_OK) return;
	
	lMdCon->m_MsduHandle	=  0;
	
	// Status conversion 
	// PHY -> MAC status
	switch ( result ) {
		case SK_PHY_SUCCESS:
			lMdCon->m_Status = SK_MAC_SUCCESS;	
			break;
		
		case SK_PHY_NO_ACK:
			lMdCon->m_Status = SK_MAC_NO_ACK;	
			break;
		
		case SK_PHY_TRX_OFF:
			lMdCon->m_Status = SK_PHY_TRX_OFF;
			break;
		
		case SK_PHY_BUSY:
			lMdCon->m_Status = SK_MAC_CHANNEL_ACCESS_FAILURE;
			break;

		default:
			lMdCon->m_Status = result;
			break;
	}

	if( SK_PostMessage(layer, SK_LAYER_PHY, SK_MCPS_DATA_CONFIRM_CMD, (SK_VP)lMdCon) != SK_E_OK) {
		SK_FreeMemory(lMdCon);
	}
}


// -------------------------------------------------
//   Post MCPS_DATA.ind to upper layer
// -------------------------------------------------
#define OFFSET 	0
#define CRC_LEN 0
			
void PostDataInd(SK_UB* data, SK_UH len, SK_UB rssi){
	SK_UH i;
	SK_UB layer;
	SK_MCPS_DATA_INDICATION *MdInd;

	if( len <= (OFFSET + CRC_LEN) ) {
	  	SS_STATS(phy.recv_drop);
	  	return;
	}
	
	if( (len - OFFSET - CRC_LEN) > SK_AMAXMACPAYLOADSIZE ){
		SS_STATS(phy.recv_drop);
		return;
	}
	
	if (SK_AllocDataMemory((SK_VP *)&MdInd) != SK_E_OK) {
	  	SS_STATS(phy.recv_drop);
	  	return;
	}
	
	memset((SK_UB*)MdInd, 0, sizeof(MdInd));
	
	MdInd->m_Rssi		= rssi;
	MdInd->m_TimeStamp 	= SK_GetLongTick();
	MdInd->m_RecvSlot 	= SSMac_GetCurrentSlot();
	MdInd->m_MsduLength = len - OFFSET - CRC_LEN;
	
	for(i=0;i<MdInd->m_MsduLength;i++) {
		MdInd->m_Msdu[i] = data[i + OFFSET];
	}

	if( gnPHY_TestMode == 0 ){
		layer = gnPHY_UpperLayer;
	} else if( gnPHY_TestMode == 1 ){
		layer = SK_LAYER_APL;
	}
	
	if(SK_PostMessage(layer, SK_LAYER_PHY, SK_MCPS_DATA_INDICATION_CMD, (SK_VP)MdInd) != SK_E_OK){
	//if(SK_PostMessage(SK_LAYER_APL, SK_LAYER_PHY, SK_MCPS_DATA_INDICATION_CMD, (SK_VP)MdInd) != SK_E_OK){
		SK_FreeMemory(MdInd);
		SS_STATS(phy.recv_drop);
		return;
	}
	
	SS_STATS(phy.recv);
}
