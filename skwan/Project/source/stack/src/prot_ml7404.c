/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: RF packet TX/RX driver for ML7404
    
    Copyrights(C)2016-2017 Skyley Networks, Inc. All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the Institute nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
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

#include	"skyley_stack.h"
#include	"rf_interface.h"


// -------------------------------------------------
//   Debug options
// -------------------------------------------------
//#define DEBUG_CCA
//#define DEBUG_RX
#define DEBUG_INT
//#define DEBUG_STATUS


// -------------------------------------------------
//   Global PIBs
// -------------------------------------------------
SK_UB			gnPHY_LowerLayer;
SK_UB 			gnPHY_UpperLayer;
SK_UB  			gnPHY_CurrentChannel;
SK_UB			gnPHY_CCAThreshold;
SK_UB			gnPHY_CurrentTRX;

//for tx test mode
SK_UB			gnPHY_TestMode;


// -------------------------------------------------
//   Function proto types
// -------------------------------------------------
static void PostDataConfirm(SK_UB layer, SK_UB result);
static SK_BOOL TransmitPacket(void);
static void SK_PHY_ReceivePacket(void);
static SK_BOOL SK_PHY_ExecCSMACA(void);


// -------------------------------------------------
//   Private and working variables
// -------------------------------------------------
//TX buffer rolling
static SK_UB* gSendingData;
static SK_UH gTxTotalLen; //送信データ総バイト数

static SK_UH gRxTotalLen; //受信データ総バイト数
static SK_BOOL gIsCRCError; //CRCエラー発生？

static SK_BOOL gCCAResult;

//RF interrupt flags
static volatile SK_UB gnPHY_TxCompleted;
static volatile SK_UB gnPHY_TxDataRequest;
static volatile SK_UB gnPHY_CCACompleted;


//ML7414 DSSSS consts
#define DSSS_PSDU_LEN 	32
#define DSSS_CRC_LEN 	2
#define FIFO_TH 		(DSSS_PSDU_LEN - DSSS_CRC_LEN)
static SK_UB tx_pad[32];
static SK_UB rx_fifo[64];


// -------------------------------------------------
//   ARIB T108 送信時間制限 処理
// -------------------------------------------------
#define	SEND_LIMIT_PERIOD			(3600000 + 180000) //1 hour + 3 minutes
#define SEND_PERMIT_TIME			(360000 - 10000) //360 - 10sec

static void StartTxTime(void);
static void StopTxTime(void);
static void CheckSendLimit(void);
static void UpdateSendLimit(void);
void ResetTxTimeContext(void);

//送信制限時間中=TRUE
SK_BOOL 	gUnderSendLimit;

//1パケットの送信開始のタイムスタンプ
SK_UW  		gTxTimeStart;

//送信総和時間(ms)
SK_UW  		gTotalTxTime;

//送信総和時間の積算開始のタイムスタンプ
SK_UW  		gTotalTxTimeStart;

//1時間経過チェックの頻度調整
SK_UW		gUpdateCheckInterval;


// -------------------------------------------------
//   State machines
// -------------------------------------------------
SK_STATESTART(PHY);	
SK_STATESTART(CCA);	

typedef enum {
	eIdle,	
	eCCA, 
	eWaitTxComp,
	eCCABusy, 
	eFinalize,
	
	eWaitCCAComp,
	eExecCCA
} SendState;


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
	
	gTxTotalLen = 0;
	
	ResetTxTimeContext();

	SSMac_Wakeup();

	gnSK_Time_CCA++;
	
	SK_INITSTATE(PHY);
	SK_INITSTATE(CCA);
}


// -------------------------------------------------
//   PHY Main Task
// -------------------------------------------------

void SK_PHY_Task(void) {
	static SK_UB nCmd,nResID,*pPkt;

	//SK_STATEADD(PHY,1);	
	SK_STATEADD(PHY,2);
	
	// -------------------------------------------------
	if (SK_GetMessage(SK_LAYER_PHY,&nResID,&nCmd,(SK_VP *)&pPkt)==SK_E_OK) {
		// メッセージを受信した
		switch(nCmd) {

		case SK_MCPS_DATA_REQUEST_CMD:
		case SK_PD_DATA_REQUEST_CMD:{
			SK_UH i;
			SK_BOOL ans;
			SK_UB* data_buf;
			SK_UH data_len;
			
#ifdef DEBUG_PHY
SK_print("SK_PD_DATA_REQUEST_CMD\r\n");
#endif

			if( nCmd == SK_PD_DATA_REQUEST_CMD ){
				SK_PD_DATA_REQUEST *PdReq;
				PdReq = (SK_PD_DATA_REQUEST *)pPkt;
				data_buf = PdReq->m_Psdu;
				data_len = PdReq->m_PsduLength;
			} else {
				SK_MCPS_DATA_REQUEST *MdReq;
				MdReq = (SK_MCPS_DATA_REQUEST *)pPkt;
				data_buf = MdReq->m_Msdu;
				data_len = MdReq->m_MsduLength;
			}
		
			if( gUnderSendLimit == TRUE ){
			  	SS_STATS(phy.send_drop);
				PostDataConfirm(nResID, SK_PHY_UNDER_SEND_LIMIT); 
				break;
			}
			
			if( data_len + SS_CRC_LEN > PSDU ){
			  	SS_STATS(phy.send_drop);
				break;
			}
			
			//alloc buffer to load FIFO data
			if (SK_AllocDataMemoryWith((SK_VP *)&gSendingData, 1) != SK_E_OK) {	
			  	SS_STATS(phy.send_drop);
				break;
			}

			SSMac_Wakeup();
			
			ml7404_trx_off();

			//copy original data to tranmit buffer
			for( i = 0; i < data_len; i++ ){
				gSendingData[i] = data_buf[i];
			}
			
			//setup tx context
			gTxTotalLen = data_len;

			//move to CCA state
			SK_SETSTATE(eCCA, CCA);
			
			//clear interrupt flags
			gIsCRCError = FALSE;
			gCCAResult = FALSE;
			gnPHY_CCACompleted = FALSE;
			gnPHY_TxCompleted = FALSE;
			gnPHY_TxDataRequest = FALSE;

			//wait for TX done
			SK_WAITFOR(1000, (SK_PHY_ExecCSMACA() != 0),PHY, 2);

			//free working buffer
			SK_FreeMemory(gSendingData);
			gTxTotalLen = 0;

			ans = SK_PHY_SUCCESS;
			
			if( gCCAResult == FALSE ){
				if( SK_GETSTATE(CCA) != eIdle ){ //TX complete lost case
					#ifdef DEBUG_STATUS
					SK_print("err:tx or cca time out. reset.");
					SK_print_hex( SK_CountFreeDataMemory(), 4 );
					SK_print("\r\n");
					SK_print_hex( SK_GETSTATE(CCA), 2 );
					SK_print("\r\n");
					#endif
					
					SS_STATS(phy.err);
					ans = SK_PHY_NO_RESPONSE;
					rf_reboot();
				} else {
				  	SS_STATS(phy.busy);
					ans = SK_PHY_BUSY;
				}			
			} else {
			  	SS_STATS(phy.send);
			}

			PostDataConfirm(nResID, ans);

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
	
	UpdateSendLimit();
}


// -------------------------------------------------
//   Carrier sense and packet transmission
// -------------------------------------------------

SK_BOOL SK_PHY_ExecCSMACA(void){
	static SK_UB reg;
	static SendState state;
	SK_UB cca_exit = 0;

	state = (SendState)SK_GETSTATE(CCA);

	if( state == eCCA ){
		
		SK_SETSTATE(eExecCCA, CCA);
	
	} else if( state == eExecCCA ){
		#ifdef DEBUG_CCA
		SK_print("now start to send\r\n");
		#endif

		rf_phy_reset();
		ml7404_start_ecca();
		
		//debug
		//gnPHY_CCACompleted = TRUE;

		SK_SETSTATE(eWaitCCAComp, CCA);
		
	} else if( state == eWaitCCAComp ){
		//Detect CCA completed
		if( gnPHY_CCACompleted == TRUE ){
			#ifdef DEBUG_CCA
			SK_print("cca done\r\n");
			#endif

			//Check CCA status 
			reg = ml7404_get_cca_status() & 0x03;
			//reg = 0; //debug always success
			ml7404_clear_interrupt_source(INT_CCA_COMPLETE);

			//CCA OK -> Start Tx 
			if( reg == 0 ){
				#ifdef DEBUG_CCA
				SK_print("start wait for tx comp\r\n");
				#endif
			
				//move to TX state
				#ifdef DEBUG_GPIO_RFTX
				DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
				#endif
				
				StartTxTime();

				TransmitPacket();

				//move to TX state
				SK_SETSTATE(eWaitTxComp, CCA);
			} else {
				#ifdef DEBUG_CCA
				SK_print("cca fail\r\n");
				#endif
			
				gnPHY_CCACompleted = FALSE;
				ml7404_clear_interrupt_source(INT_CCA_COMPLETE); 
				ml7404_trx_off();

				SK_SETSTATE(eCCABusy, CCA);	//CCA busy case
			}
		} 
	} else if( state == eWaitTxComp ){		
		//wait for TX complete
		if( gnPHY_TxCompleted == TRUE ){
			#ifdef DEBUG_GPIO_RFTX
			DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
			#endif

			#ifdef DEBUG_CCA
			SK_print("tx ok\r\n");
			#endif

			StopTxTime();
			
			SK_SETSTATE(eFinalize, CCA);
		} 
		
	} else if( state == eFinalize || state == eCCABusy ){
		
		#ifdef DEBUG_CCA
		SK_print("wait interval\r\n");
		#endif
		
		gTxTotalLen = 0;

		ml7404_go_rx_mode();

		SK_SETSTATE(eIdle, CCA);
		cca_exit = 1;
		
		if( state == eFinalize ){
			gCCAResult = TRUE;
		} else {
			gCCAResult = FALSE;
		}

		#ifdef DEBUG_CCA
		SK_print("Idle\r\n");
		#endif
	} 

	return cca_exit;
}


static	SK_BOOL	TransmitPacket(void)
{
	ml7404_clear_tx_fifo();
	ml7404_clear_interrupt_source(INT_STATUS_GRP3);
	
	//program packet length
	ml7404_reg_write8(b0_TX_PKT_LEN_H, 0);
	ml7404_reg_write8(b0_TX_PKT_LEN_L, DSSS_PSDU_LEN);

	//write tx data to TX FIFO
	ml7404_fifo_write_block(gSendingData, gTxTotalLen);
	
	//pad to 32 bytes
	memset(tx_pad, 0, 32);
	if( gTxTotalLen < FIFO_TH ){
		SK_UB more = FIFO_TH - (gTxTotalLen);
		ml7404_fifo_write_block(tx_pad, more);
	}

	//wait for data ready interupt event
	RF_LOCK();
	ml7404_wait_for_int_event(INT_TX_COMPLETE_REQUEST, 20);
	RF_UNLOCK();

	//start tx
	ml7404_go_tx_mode();

	return TRUE;
}


static void SK_PHY_ReceivePacket(void)
{
  	SK_UB layer;
	SK_MCPS_DATA_INDICATION *MdInd;
	SS_MHR phr;
	
	ml7404_fifo_read_block(rx_fifo, 64);

	phr.Raw[0] = rx_fifo[0];
	
	if( gIsCRCError == TRUE ){
		SS_STATS(phy.recv_drop);
	  	return;
	}

	if( gnPHY_TestMode == 0 ){
		gRxTotalLen = phr.Field.m_Length + SS_MHR_LEN + SS_CRC_LEN;
	
		if( phr.Field.m_Length == 0 ) {
		  	SS_STATS(phy.recv_drop);
			return;
		} else if( gRxTotalLen > SS_MAX_FRAME_LEN ){
		  	SS_STATS(phy.recv_drop);
			return;
		}
	} else {
	  	gRxTotalLen = PSDU;
	}
	
	#ifdef DEBUG_RX
	SK_print_hex(gRxTotalLen, 2); SK_print(" ");
	#endif
	
	//buffer alloc OK?
	if( SK_AllocDataMemoryWith((SK_VP *)&MdInd, 2) == SK_E_OK ){
		memset((SK_UB*)MdInd, 0, sizeof(MdInd));
		
		MdInd->m_Rssi		= SK_PHY_ReadRSSIValue();
		MdInd->m_TimeStamp 	= SK_GetLongTick();
		MdInd->m_RecvSlot 	= SSMac_GetCurrentSlot();
		MdInd->m_MsduLength = gRxTotalLen - SS_CRC_LEN;
		
		memcpy(MdInd->m_Msdu, rx_fifo, MdInd->m_MsduLength);

		#ifdef DEBUG_RX
		{
			SK_UB i;
			for( i = 0; i < MdInd->m_MsduLength; i++ ){
				SK_print_hex(MdInd->m_Msdu[i], 2); 
			}
			SK_print("\r\n");
		}
		#endif

		SS_STATS(phy.recv);
		
		if( gnPHY_TestMode == 0 ){
			layer = gnPHY_UpperLayer;
		} else if( gnPHY_TestMode == 1 ){
			layer = SK_LAYER_APL;
		}
	
		if(SK_PostMessage(layer, SK_LAYER_PHY, SK_MCPS_DATA_INDICATION_CMD, (SK_VP)MdInd) != SK_E_OK){
			SK_FreeMemory(MdInd);
		}
	} else {
		//
		//stack memory full
		//
	}
	
	gRxTotalLen = 0;
}


// -------------------------------------------------
//   Process RF interrupts
// -------------------------------------------------

void SK_PHY_Interrupt(void) {
	SK_UW	source;
	SK_UB	bank;
	
	bank = rf_reg_rd(BANK_SEL);

	source = ml7404_get_interrupt_source();

	#ifdef _DEBUG_INT
	SK_print("INT:"); SK_print_hex(source, 8);
	SK_print("\r\n");
	#endif

	if( (source & INT_CCA_COMPLETE) != 0 ){
		gnPHY_CCACompleted = 1;
		ml7404_trx_off();
	}
	
	if( (source & INT_TX_COMPLETE) != 0 ){
		gnPHY_TxCompleted = 1;
	}
	
	if( (source & INT_TX_COMPLETE_REQUEST) != 0 ){
		gnPHY_TxDataRequest = 1;
	}
	
	if( (source & INT_CRC_ERROR) != 0 ){
		gIsCRCError = TRUE;
	}
	
	if( (source & INT_RX_COMPLETE) != 0 ){
		SK_PHY_ReceivePacket();
	}
	
	if( (source & INT_SYNC_ERROR) != 0 ){
		#ifdef DEBUG_INT
		SK_print("INT:"); SK_print_hex(source, 8);
		SK_print("...sync error\r\n");
		#endif
	}
	
	if( (source & INT_STATUS_GRP3) != 0 ){
	  	//->ここでクリアするとCCA結果もクリアされるので注意
	  	if( (source & INT_CCA_COMPLETE) != 0 ){
		  	ml7404_clear_interrupt_source( (source & ~(INT_CCA_COMPLETE)) & INT_STATUS_GRP3);
		} else {
			ml7404_clear_interrupt_source(source & INT_STATUS_GRP3);
		}
	}
	
	if( (source & INT_STATUS_GRP2) != 0 ){
		ml7404_clear_interrupt_source(source & INT_STATUS_GRP2);
	}
	
	if( (source & INT_STATUS_GRP1) != 0 ){
		ml7404_clear_interrupt_source(source & INT_STATUS_GRP1);
	}
	
	rf_reg_wr(BANK_SEL, bank);
}


// -------------------------------------------------
//   RSSI mesurement
// -------------------------------------------------

SK_UB SK_PHY_ReadRSSIValue(void) {
	SK_UB rssi;
	rssi = ml7404_get_rssi();
	return rssi;
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
//   Change channel
// -------------------------------------------------
/**
ch24 -> 920.7MHz
...
...
ch60 -> 927.9MHz
*/
SK_BOOL SK_PHY_ChangeChannel(SK_UB ch) {
  	if( ch < 24 || ch > 60 ){
		return FALSE;
  	}
  
	if( gnPHY_CurrentTRX == SK_PHY_TRX_OFF ){
		SK_PHY_Wakeup();
	}
	
	gnPHY_CurrentChannel = ch;
	
	ml7404_change_channel(ch - 24);
	
	return TRUE;
}


// -------------------------------------------------
//   Post PD_DATA.confirm to upper layer
// -------------------------------------------------
static void PostDataConfirm(SK_UB layer, SK_UB result){
	SK_MCPS_DATA_CONFIRM 	*lMdCon;
	
	if (SK_AllocCommandMemory((SK_VP *)&lMdCon) != SK_E_OK) return;
	
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


//start TX time measurement
void StartTxTime(void){
  	gTxTimeStart = SK_GetLongTick();
}


//stop TX time measurement and update total time
void StopTxTime(void){
  	SK_UW now = SK_GetLongTick();
	SK_UW diff;
	
	diff = (SK_UW)(now - gTxTimeStart);

	gTotalTxTime += diff + 10;
	
	CheckSendLimit();
}


void CheckSendLimit(void){
	if( gTotalTxTime > SEND_PERMIT_TIME ){
		gUnderSendLimit = TRUE;
	}
}


void UpdateSendLimit(void){
  	SK_UW now = SK_GetLongTick();

	if( ((SK_UW)(now - gUpdateCheckInterval)) < 50 ){
	  	return;
	}
		  
	gUpdateCheckInterval = now;

	if( ((SK_UW)(now - gTotalTxTimeStart)) > SEND_LIMIT_PERIOD ){
		gTotalTxTimeStart 	= now;
		gTotalTxTime 		= 0;
		gUnderSendLimit 	= FALSE;
	}
}


void ResetTxTimeContext(void){
	gUnderSendLimit 		= FALSE;	
	gTxTimeStart 			= 0;
	gTotalTxTime 			= 0;
	gTotalTxTimeStart 		= SK_GetLongTick();
	gUpdateCheckInterval 	= 0;	
}
