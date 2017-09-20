/**
    Author:       Skyley Networks, Inc.
    Version:
    Description:  Scalable-Slotted MAC (aka SSMAC) for SkWAN
    
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
#include	<string.h>
#include	<stdlib.h>

#include	"skyley_stack.h"
#include	"sha2.h"
#include	"rijndael-alg-fst.h"


// -------------------------------------------------
//   Debug options
// -------------------------------------------------
//#define DEBUG_PRINT
//#define DEBUG_DATA
//#define DEBUG_SLOT
//#define DEBUG_SLOT_PRINT
//#define DEBUG_ACK
//#define DEBUG_PEND
//#define DEBUG_JOIN
//#define DEBUG_SEC
//#define DEBUG_SLEEP
//#define DEBUG_MCU_SLEEP
//#define DEBUG_FH
//#define DEBUG_GPIO_TRX

#define MCU_SLEEP

#ifndef DEBUG_PRINT
#define SK_print(X) 
#define SK_print_hex(X, Y) 
#endif

#ifdef USE_MEMCPY_F
#define memcpy memcpy_f
#define strncmp strncmp_f
#endif


// -------------------------------------------------
//   Consts
// -------------------------------------------------
#ifdef ENDDEVICE
static const char* gsSSMac_VerStr = "1.0.0rc4-ED";
#else
static const char* gsSSMac_VerStr = "1.0.0rc4-STA";
#endif

static const SK_UB gnDefaultHashKey[ SS_SLOT_HASHKEY_LEN ] = 
{
	0x11, 0x22, 0x33, 0x44
};
 

// -------------------------------------------------
//   Slot-Address database (Station only)
// -------------------------------------------------
SS_SLOT_ADDR_DB_ITEM gaSSMac_SlotAddr_DB[ SS_DB_SIZE ];


// -------------------------------------------------
//   Slot In-use bitmap (Station only)
// 32bit * 32 = 1024 bit 
// -------------------------------------------------
#define SLOT_BITMAP_SIZE 32
SK_UW gaSSMac_SlotBitmap[ SLOT_BITMAP_SIZE ];


// -------------------------------------------------
//   Buffer for downlink data transmission 
// -------------------------------------------------
SS_PENDING_INFO gaSSMac_PendingBuf[ SS_PENDINGBUF_SIZE ];


// -------------------------------------------------
//   Downlink slot info
// -------------------------------------------------
//スロットモードによらず静的に最大数確保しておく
//Todo: bitmap方式に変更する
SK_UB gaSSMac_DownSlot[ SS_MAX_DOWN_SLOT_NUM ];


// -------------------------------------------------
//   Dev info DB for meta beacon
// -------------------------------------------------
SS_DEV_INFO gaSSMac_DevInfo[ SS_MAX_NIC_NUM ];


// -------------------------------------------------
//   Channel Table
// -------------------------------------------------
SK_UB gaSSMac_ChannelTable[ SS_MAX_HOPPING_CH_NUM ];

static const SK_UB gaSSMac_HoppingTable[4][4] = {
	{ 0, 0, 0, 0 },
	{ 1, 0, 1, 0 },
	{ 1, 0, 2, 0 },
	{ 1, 2, 3, 0 },
};


// -------------------------------------------------
//   MAC PIBs
// -------------------------------------------------
SK_UB 			gnSSMac_LowerLayer = SK_LAYER_PHY;
SK_UB 			gnSSMac_UpperLayer = SK_LAYER_APL;

//My 64bit Ext address
SK_ADDRESS		gnSSMac_Address; 

SK_UW			gnSSMac_StationID;
SK_UB			gnSSMac_BSN;
SK_UB			gnSSMac_CurrentCapacity;
SK_UB			gnSSMac_SlotMode;
SK_UB			gnSSMac_CurrentHoppingTable;
SK_BOOL			gnSSMac_HoppingEnable;
SK_UB			gnSSMac_SeqNo;
SK_UB			gnSSMac_BaseChannel;

SK_UH			gnSSMac_CurrentSlotUnit;
SK_UH			gnSSMac_CurrentSlot;
SK_UH			gnSSMac_MySlot;
SK_UB			gnSSMac_InSlotIdx;

SK_UB			gnSSMac_DeviceType;
SK_BOOL			gnSSMac_RxOnWhenIdle;

SK_BOOL			gnSSMac_IsFullManage;

SK_UB			gnSSMac_SlotCalib = 0;
SK_UW			gnSSMac_FineCalib = 0;

//Frame Counter
//EDのみ有効
//ED端末が発する暗号フレームのoutgoing frame counter(24bit)
SK_UW			gnSSMac_FrameCounter; 

//EDのみ有効
//STAのincoming frame counter
SK_UW			gnSSMac_FrameCounterSTA;

//how many times beacon lost for sync loss
SK_UB			gnSSMac_SyncLossThreshold; 
SK_UW			gnSSMac_PendingExpireTime;

SK_UB			gaSSMac_SlotHashKey[ SS_SLOT_HASHKEY_LEN ];

SK_UW			gnSSMac_RandS;

//Device only
SK_UB			gaSSMac_AESKey[ SS_AES_KEY_LEN ];
SK_UB			gaSSMac_PSK[ SS_AES_KEY_LEN ];


// -------------------------------------------------
//   MAC Stats
// -------------------------------------------------
SS_STATISTICS gaSSMac_Stats;


// -------------------------------------------------
//   Private working functions
// -------------------------------------------------
static void PostSlotChanged(SK_UH slot);
static void PostSlotPreChange(SK_UH slot);
static void PostSlotChangedExec(SK_UH slot);
static void PostSlotPreChangeExec(SK_UH slot);
static void PostBeaconNotifyInd(SS_BCN_FRAMECONTROL* fctrl, SS_SLOT_CONFIG* slot, SK_UB bsn, SK_UW sta_id, SK_UB rssi);
static void PostMetaBeaconNotifyInd(SK_UW sta_id, SS_DEV_INFO info[], SK_UB size);
static void PostDataConf(SK_UB status, SS_DATA_REQUEST* SdReq);
static void PostSyncInd(SK_UB status);
static void PostCommStatInd(SK_UB status);
static void PostJoinConf(SK_UB status, SK_UH slot, SK_UB idx);
static void PostAckInd(SS_DOWN_FRAMECONTROL* down_fctrl, SK_UB pend_slot);

static SK_UH GetUpSlotNum(SK_UB mode);
	   SK_UH GetAllSlotNum(SK_UB mode);
static SK_UB GetDownSlotNum(SK_UB mode);
static SK_UB FindFreeDownSlot(void);
void ClearAllDownSlot(void);
static void ClearDownSlot(SK_UB slot);
static SK_UH CalcSlotFor(SK_UB* hash, SK_ADDRESS* addr, SK_UW sta_id);
SK_UH CalcMySlot(SK_UB* key);

static void StopSync(void);
static SK_BOOL NeedWakeup(SK_UH slot);
static SK_BOOL CanSleep(SK_UH slot);

static void CheckSyncLoss(void);
static void CheckPendExpire(void);
static SK_BOOL CheckMacCmdAccept(SK_UB cmd);
static SK_BOOL CheckSelectorAccept(SK_UB selector);

static SK_BOOL AutoRegisterDevSlot(SK_ADDRESS* addr, SK_UH slot, SK_UH* res_slot, SK_UB* idx);
static SK_BOOL RegisterDevWithSlotIdx(SK_ADDRESS* addr, SK_UH slot, SK_UB idx);
static SK_ADDRESS* GetDBAddressFor(SK_UH slot, SK_UB idx);
static SS_SLOT_ADDR_DB_ITEM* FindDBItemFor(SK_UH slot);
static SS_SLOT_ADDR_DB_ITEM* FindDBItemOf(SK_ADDRESS* addr);
static SK_UB GetInSlotIdxOf(SK_ADDRESS* addr);
static SK_UB* GetEncKeyOf(SK_ADDRESS* addr);
static SK_UW GetFrameCounterFor(SK_ADDRESS* addr);

static SK_BOOL AddToPendingBuf(SS_DATA_REQUEST* SdReq);
static SS_PENDING_INFO* FindPendingBufTo(SK_ADDRESS* dst);
static SS_PENDING_INFO* FindPendingBufFor(SK_UH slot);
static void FreePendingData(SS_PENDING_INFO* PendInf);
static SS_PENDING_INFO* FindPendingBufOfSlot(SK_UB downslot);
static SK_BOOL PutAddrToItem(SK_ADDRESS* addr, SS_SLOT_ADDR_DB_ITEM* item, SK_UB* idx);
static SK_BOOL PutAddrToItemWithIdx(SK_ADDRESS* addr, SS_SLOT_ADDR_DB_ITEM* item, SK_UB idx);

static SK_UW __ssmac_rand(void);
static SK_UH GetSeed(void);

static SK_UW GetPower2(SK_UB power);
static void GenerateEncKey(SK_UB* key, SK_UB key_len);
static void IncSeqNumAndFrameCnt(SS_DATA_REQUEST* SdReq);

static void StartSlotBitmap(void);
static void SetSlotBitmap(SK_UH slot);
static void ClearSlotBitmap(SK_UH slot);
static SK_BOOL IsSetBitmap(SK_UH slot);

SK_BOOL EncryptData(SK_UB* buf, SK_UB offset, SK_UW frm_cnt, SS_DATA_REQUEST* SdReq);
SK_BOOL EncryptJoin(SK_UB* buf, SK_UB offset, SS_DATA_REQUEST* SdReq);
SK_BOOL EncryptJoinRes(SK_UB* buf, SK_UB offset, SS_DATA_REQUEST* SdReq);
SK_BOOL DecryptData(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_UW frame_cnt, SK_MCPS_DATA_INDICATION* MdInd);
SK_BOOL DecryptJoin(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_MCPS_DATA_INDICATION* MdInd);
SK_BOOL DecryptJoinRes(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_MCPS_DATA_INDICATION* MdInd);
static SK_BOOL Encrypt(SS_ENC_REQUEST* in, SK_UB* key, SS_CCMSTAR_NONCE* nonce, SS_ENC_REQUEST* out);
static SK_BOOL Decrypt(SS_ENC_REQUEST* in, SK_UB* key, SS_CCMSTAR_NONCE* nonce, SS_ENC_REQUEST* out);
static SK_UB *CreateAuthEnc(SS_ENC_REQUEST* in, SK_UB* key,SS_CCMSTAR_NONCE* nonce);
static SK_UB* CreateAuthDec(SS_ENC_REQUEST* in,  SK_UB header_len, SK_UB* key, SS_CCMSTAR_NONCE* nonce);
static void GetNonce(SS_CCMSTAR_NONCE* nonce, SK_ADDRESS* addr, SK_UW frame_cnt);
static void GetNonceForJoin(SS_CCMSTAR_NONCE* nonce, SK_UH slot);
static void GetNonceForJoinRes(SS_CCMSTAR_NONCE* nonce, SK_ADDRESS* addr);

static void SSMac_InitChannelTable(SK_UB table[], SK_UB size);
static void ResetChannelTable(SK_UB ch);


// -------------------------------------------------
//   Private working variables
// -------------------------------------------------
SK_UH	gnTickSlotCount;

//自端末宛て下りデータのスロット番号
//データがない場合は0xFFFF
static SK_UH gnPendingSlot;

//Sync lossチェック
static SK_UW gnLastSyncLossCheck;
static SK_UW gnLastBeaconRecvTime;

static SK_UW gnLastPendCheck;

static SK_UH gSlotChanged;
static SK_UH gSlotPreChanged;


// -------------------------------------------------
//   MAC Initialize
// -------------------------------------------------
SK_STATESTART(SSMAC);	//init state machines


void SSMac_Init(SK_UW macadr1, SK_UW macadr2) {	
	gnSSMac_LowerLayer = SK_LAYER_PHY;
	gnSSMac_UpperLayer = SK_LAYER_APL;

	gnSSMac_Address.Body.Extended.m_Long[0] = macadr1;
	gnSSMac_Address.Body.Extended.m_Long[1] = macadr2;
	gnSSMac_Address.m_AddrMode = SK_ADDRMODE_EXTENDED;
	
	gnSSMac_StationID = 0xFFFFFFFFU;
	gnSSMac_BSN = 0;
	gnSSMac_CurrentCapacity = 0;
	gnSSMac_SlotMode = SS_SLOTMODE_32;
	gnSSMac_CurrentHoppingTable = SS_HOPPING_TABLE1;
	gnSSMac_HoppingEnable = TRUE;
	gnSSMac_SeqNo = 0;
	gnSSMac_FrameCounter = 0;
	gnSSMac_FrameCounterSTA = 0;
	gnSSMac_BaseChannel = SS_INITIAL_BASE_CHANNEL;
	
	memcpy(gaSSMac_SlotHashKey, gnDefaultHashKey, sizeof(gaSSMac_SlotHashKey));
	
	srand( GetSeed() );
	GenerateEncKey(gaSSMac_AESKey, SS_AES_KEY_LEN);
	GenerateEncKey(gaSSMac_PSK, SS_AES_KEY_LEN);
	
	gnSSMac_RandS = SSMac_GetRand32();

	gnTickSlotCount = 0;
	gnSSMac_CurrentSlot = 0;
	gnSSMac_CurrentSlotUnit = 0;
	gnSSMac_MySlot = 0xFFFF; //slot=0xFFFF means not sync to beacon
	gnSSMac_InSlotIdx = 0xFF;
	
	gnSSMac_DeviceType = SS_TYPE_IDLING;
	gnSSMac_RxOnWhenIdle = FALSE;

	gnSSMac_SyncLossThreshold = SS_SYNC_LOSS_THRESHOLD;
	gnSSMac_PendingExpireTime = (10L * 60L * 1000L); //10 minutes default

	gnSSMac_IsFullManage = FALSE;
	
	//gnSSMac_SlotCalib = 0;
	//gnSSMac_FineCalib = 0;
	
	gnLastBeaconRecvTime = 0;
	gnLastSyncLossCheck = 0;
	gnLastPendCheck = 0;
	
	gnPendingSlot = 0xFFFF;
	
	SSMac_InitDB(gaSSMac_SlotAddr_DB, SS_DB_SIZE);
	
	memset(gaSSMac_SlotBitmap, 0, sizeof(gaSSMac_SlotBitmap));
	
	ClearAllDownSlot();
	
	SSMac_InitPendingBuf(gaSSMac_PendingBuf, SS_PENDINGBUF_SIZE);
	
	StartSlotBitmap();
	//test_bitmap();
	
	SSMac_InitDevInfo(gaSSMac_DevInfo, SS_MAX_NIC_NUM);

	SSMac_InitChannelTable(gaSSMac_ChannelTable, SS_MAX_HOPPING_CH_NUM);
	
	SSMac_InitStats();
	
	SK_INITSTATE(SSMAC);
}


// -------------------------------------------------
//   Upper/Lower layer setting
// -------------------------------------------------

void SSMac_SetLowerLayer(SK_UB layer) {
	gnSSMac_LowerLayer = layer;
}

void SSMac_SetUpperLayer(SK_UB layer) {
	gnSSMac_UpperLayer = layer;
}

SK_UB SSMac_GetLowerLayer(void) {
	return gnSSMac_LowerLayer;
}

SK_UB SSMac_GetUpperLayer(void) {
	return gnSSMac_UpperLayer;
}


// -------------------------------------------------
//   MAC main task
// -------------------------------------------------

void SSMac_Task(void) {
	static SK_UB nCmd,nResID,*pPkt,*pResPkt;
	static SK_UB src_layer;
	
	SK_STATEADD(SSMAC,1);
	SK_STATEADD(SSMAC,2);
	
	// -------------------------------------------------
	//   Message processing
	// -------------------------------------------------
	if (SK_GetMessage(SK_LAYER_SS_MAC,&nResID,&nCmd,(SK_VP *)&pPkt)==SK_E_OK) {
	
		switch(nCmd) {
		case SS_DATA_REQUEST_CMD:{
			static SK_UB pdconf_result;
			static SS_DATA_REQUEST* SdReq;
			
			#ifdef DEBUG_DATA
			SK_print("SS_DATA_REQUEST_CMD:");
			SK_print_hex(SSMac_GetCurrentSlot(), 4);
			SK_print("\r\n");
			#endif
			
			SdReq = (SS_DATA_REQUEST *)pPkt;
			pdconf_result = SS_MAC_SUCCESS;
			
			if( SSMac_GetDeviceType() == SS_TYPE_IDLING || SSMac_GetDeviceType() == SS_TYPE_METABEACON ){
				PostDataConf(SS_MAC_INVALID_REQUEST, SdReq);
				break;
			}
			
			//エンドデバイスは非同期時の送信要求はエラー
			if( SSMac_GetDeviceType() == SS_TYPE_DEVICE && SSMac_GetSlotNum() == 0xFFFF ){
				PostDataConf(SS_MAC_INVALID_REQUEST, SdReq);
				break;
			}
			
			//Data transmission from station to device must be indirect mode
			if( (SdReq->m_TxOptions & SS_TXOPTIONS_INDIRECT) != 0 ){
				SK_BOOL ans;
				
				//add to pending buf and wait for uplink data
				ans = AddToPendingBuf(SdReq);
				if( ans == TRUE ){
					PostDataConf(SS_MAC_FRAME_PENDING, SdReq);
				} else {
					//post SS_DATA_CONFIRM as pending overflow here
					PostDataConf(SS_MAC_INDIRECT_OVERFLOW, SdReq);
				}
			} else {
				SK_BOOL ans;
				
				//data transmission from device
				ans = SSMac_TransmitData(SdReq);
				if( ans == FALSE ){
					PostDataConf(SS_MAC_INVALID_REQUEST, SdReq);
					break;
				}
				
				//wait for data transmission complete
				SK_WAITFOR(10000, SK_GetMessageByCmd(SK_LAYER_SS_MAC, &src_layer, SK_MCPS_DATA_CONFIRM_CMD,(SK_VP *)&pResPkt)== SK_E_OK, SSMAC, 1);
				
				//notify result of data transmission
				if (pResPkt != NULL){ 
					pdconf_result = ((SK_MCPS_DATA_CONFIRM *)pResPkt)->m_Status;
					//pdconf_result = SS_MAC_SUCCESS;
					SK_FreeMemory(pResPkt);
				} else {
					pdconf_result = SS_MAC_EVENT_NO_RESP;
				}
				PostDataConf(pdconf_result, SdReq);
			}
			break;
		}
		
		//Transmit pending data by down slot
		case SS_PEND_DATA_REQUEST_CMD:{
			static SS_PENDING_INFO* pend_inf;
			static SK_UH numslot;
			static SK_UH current;
			static SK_UB downslot;
			static SS_DATA_REQUEST* SdReq;
			SK_BOOL ans;
			
			SdReq = (SS_DATA_REQUEST *)pPkt;
			current = SSMac_GetCurrentSlot();
			numslot = GetUpSlotNum(gnSSMac_SlotMode);
			downslot = 0xFF;
			
			#ifdef DEBUG_DATA
			SK_print("SS_PEND_DATA_REQUEST_CMD:");
			SK_print_hex(SSMac_GetCurrentSlot(), 4); SK_print("\r\n");
			#endif

			if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
				break;	
			}
			
			//Transmit data now
			ans = SSMac_TransmitData(SdReq);
			//PostしないままSKWAITFORに入らないように戻り値はチェックする
			if( ans == FALSE ){
				PostDataConf(SS_MAC_INVALID_REQUEST, SdReq);
				goto clear_down_slot;
				//break;
			}
				
			//wait for data transmission complete
			SK_WAITFOR(10000, SK_GetMessageByCmd(SK_LAYER_SS_MAC, &src_layer, SK_MCPS_DATA_CONFIRM_CMD,(SK_VP *)&pResPkt)== SK_E_OK, SSMAC, 2);
			if (pResPkt != NULL){ 
				SK_UB result;
				result = ((SK_MCPS_DATA_CONFIRM *)pResPkt)->m_Status;
				if( result == SK_MAC_SUCCESS ){
					PostDataConf(SS_MAC_SUCCESS, SdReq);
				} else {
					PostDataConf(result, SdReq);
				}
				SK_FreeMemory(pResPkt);
			}

		clear_down_slot:
			if( current > numslot ){
				downslot =  (SK_UB)(current - numslot - 1);
			}				

			if( downslot != 0xFF ){
				pend_inf = FindPendingBufOfSlot(downslot);
				if( pend_inf != NULL ){
					FreePendingData(pend_inf);
				}
				ClearDownSlot(downslot);
			}
			
			break;
		}

		case SK_MCPS_DATA_INDICATION_CMD:{
			SK_MCPS_DATA_INDICATION *MdInd = (SK_MCPS_DATA_INDICATION *)pPkt;
			SK_UB pos, offset;
			SS_MHR phr;

			//Meta Bcnモードは一切の受信は不要
			if( SSMac_GetDeviceType() == SS_TYPE_IDLING ||
				SSMac_GetDeviceType() == SS_TYPE_METABEACON ){
				break;	
			}
			
			//同期済みDeviceの場合、フレーム受信後はすぐRFスリープさせて良い
			if( (SSMac_GetDeviceType() == SS_TYPE_DEVICE) && (gnSSMac_MySlot != 0xFFFF) ){
				SSMac_Sleep(FALSE);
			}
				
			//frame is too big
			if( SS_MAX_FRAME_LEN < MdInd->m_MsduLength ){
			  	SS_STATS(mac.recv_drop);
				break;
			}

			//at least PHR header
			if( MdInd->m_MsduLength < 1 ){
			  	SS_STATS(mac.recv_drop);
				break;	
			}

			offset = 0;
			phr.Raw[0] = MdInd->m_Msdu[offset];

			offset++;

			//check length consistency
			if( phr.Field.m_Length != MdInd->m_MsduLength - 1 ){
			  	SS_STATS(mac.recv_drop);
				break;
			}
			
			switch(phr.Field.m_Type){
			case SS_FRAME_DATA:{
				SS_DATA_INDICATION* SdInd;
				SK_ADDRESS* src_addr = NULL;
				SS_PENDING_INFO* PendInf;
				SS_UP_FRAMECONTROL up_fctrl;
				SS_DOWN_FRAMECONTROL down_fctrl;
				SK_UB recv_seq_no;
				SK_UW recv_frame_cnt;
				SK_UB drift, pend_slot;
				SK_UB* ss_msdu;
				SK_UH ss_msdu_len;

				pos = SSMac_AnalyzeDataHdr(	MdInd->m_Msdu + offset,
											(SK_UB)phr.Field.m_Length,
											&up_fctrl, 
											&recv_seq_no,
											&recv_frame_cnt);

				//ヘッダ期待値より受信データが少ないので破棄
				if( (pos == 0) || (phr.Field.m_Length < pos) ) {
				  	SS_STATS(mac.recv_drop);
					break;	
				}

				//phr.Field.m_Lengthが示すペイロード長が大きすぎる
				if( (SS_PAYLOAD_LEN + SS_MIC_LEN) < (phr.Field.m_Length - pos) ) {
				  	SS_STATS(mac.recv_drop);
					break;	
				}
				
				/*
				verify this data is acceptable or not here
				*/
				//verify 1
				//エンドデバイスはPendingSlot以外にDataは来ない
				//only Ack will be received while my slot.
				if( SSMac_GetDeviceType() == SS_TYPE_DEVICE ){
					/*
					if( SSMac_GetSlotNum() == MdInd->m_RecvSlot ){
						#ifdef DEBUG_DATA
						SK_print("device: receiving data in my slot...discard.\r\n");
						#endif
						
						SS_STATS(mac.recv_drop);
						break;	
					}
					*/
					if( SSMac_GetCurrentSlot() != gnPendingSlot ){
						#ifdef DEBUG_DATA
						SK_print("device: receiving data in other slot...discard.\r\n");
						#endif
						
						SS_STATS(mac.recv_drop);
						break;	
					}

					//スロット内インデックス不一致
					//条件に注意
					//1. join_res受信時はスロット内インデックスが確定していない
					if( (up_fctrl.Field.m_Selector != SS_SELECTOR_JOIN_RES_CMD) && (gnSSMac_InSlotIdx != up_fctrl.Field.m_InSlotIdx) && gnSSMac_InSlotIdx != 0xFF ){
						#ifdef DEBUG_DATA
						SK_print("device: receiving data is not my slot idx...discard.\r\n");
						#endif
						
						SS_STATS(mac.recv_drop);
						break;	
					}
				}
				
				//verify 2
				//selector condition
				if( CheckSelectorAccept(up_fctrl.Field.m_Selector) == FALSE ){
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				//
				//and may be more verification necessary
				
				//
				//
				
				if (SK_AllocDataMemory((SK_VP *)&SdInd) != SK_E_OK) {
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
					src_addr = GetDBAddressFor(MdInd->m_RecvSlot, up_fctrl.Field.m_InSlotIdx);
					
					//join cmdの場合up_fctrl.Field.m_InSlotIdx=0で来るが実際はidx未確定なのでsrc_addrが正しく取得されない
					// -> cmdは除外する
					if( src_addr != NULL && up_fctrl.Field.m_Selector != SS_SELECTOR_JOIN_CMD ){
						SSMac_CopyAddress(&(SdInd->m_SrcAddress), src_addr);
					} else {
						SSMac_InitAddr(&(SdInd->m_SrcAddress));
					}
				} else {
					SSMac_InitAddr(&(SdInd->m_SrcAddress));
				}
				
				SdInd->m_Rssi = MdInd->m_Rssi;
				SdInd->m_RecvSlot = MdInd->m_RecvSlot;
				SdInd->m_RecvInSlotIdx = up_fctrl.Field.m_InSlotIdx;
				SdInd->m_Selector = up_fctrl.Field.m_Selector;
				SdInd->m_Channel = gnPHY_CurrentChannel;
				
				//
				//decrypt here
				//
				if( up_fctrl.Field.m_Security == 1 ){
					SK_BOOL dec_result;

					if( up_fctrl.Field.m_Selector == SS_SELECTOR_JOIN_RES_CMD ){
						dec_result = DecryptJoinRes(SdInd, offset + pos, MdInd);

					} else if( up_fctrl.Field.m_Selector == SS_SELECTOR_JOIN_CMD ){
						dec_result = DecryptJoin(SdInd, offset + pos, MdInd);

					} else {
						dec_result = DecryptData(SdInd, offset + pos, recv_frame_cnt, MdInd);
					}
					
					if( dec_result == FALSE ){
						#ifdef DEBUG_SEC
						SK_print("Decrypt failure\r\n");
						#endif
						
						//
						//notify security error indication to upper layer
						PostCommStatInd(SS_MAC_SECURITY_FAILURE);
						//
						SK_FreeMemory(SdInd);
						
						SS_STATS(mac.recv_drop);
						break;	
					}
					
					SdInd->m_MsduLength = phr.Field.m_Length - pos - (SK_UB)SS_MIC_LEN;
					//overwrite encrypted data to decrypted data for subsequent mac cmd analyze
					memcpy(MdInd->m_Msdu + offset + pos, SdInd->m_Msdu, SdInd->m_MsduLength);
				} else {
					SdInd->m_MsduLength = phr.Field.m_Length - pos; //最大でもSS_PAYLOAD_LEN + MIC_LEN
					memcpy(SdInd->m_Msdu, MdInd->m_Msdu + offset + pos, SdInd->m_MsduLength);
				}
				
				ss_msdu = MdInd->m_Msdu + offset + pos; //offset = PHR, pos = up fctrl and seq no
				ss_msdu_len = SdInd->m_MsduLength; //note: ss_msdu_len can be 0

				if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_DATA_INDICATION_CMD,(SK_VP)SdInd) != SK_E_OK){
					SK_FreeMemory(SdInd);
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				SS_STATS(mac.recv_data);
				
				//if ss_msdu_len is zero, no more procedure is necessary
				if( ss_msdu_len == 0 ){
					break;
				}

				//Setup Ack 
				down_fctrl.Raw[0] = 0;
				down_fctrl.Field.m_Selector = up_fctrl.Field.m_Selector;
				down_fctrl.Field.m_FramePending = 0;
				down_fctrl.Field.m_InSlotIdx = up_fctrl.Field.m_InSlotIdx;
				drift = 0;
				pend_slot = 0xFF; //must indicates downslot=0xFF when no pending data 
				
				switch(up_fctrl.Field.m_Selector){
				// --MAC command process
				case SS_SELECTOR_MAC_CMD:{
					//check cmd sub type
					switch(ss_msdu[0]){
					
					//case XXXXX:
					//no mac cmd at this moment
					//break;
					
					default: 
						break;
					} //end of switch(ss_msdu[0])
					
					break; 
				} //end of case SS_SELECTOR_MAC_CMD
				
				// --Join cmd
				case SS_SELECTOR_JOIN_CMD:{
					SK_UB cmd_offset = 0;
					SK_UB res_idx;
					SK_UB join_status = SS_STAT_JOIN_OK; //success
					SK_UH src_slot;
					SK_UH reg_slot; //join後の最終確定スロット
					static SK_ADDRESS join_src;
					SK_BOOL reg_ans;
					SS_SLOT_ADDR_DB_ITEM* item;
					SS_DATA_REQUEST* SdReq;
					
					if( ss_msdu_len < SS_JOIN_CMD_LEN ){
						break;
					}

					src_slot = (SK_UH)(up_fctrl.Field.m_SlotH) << 8;
					src_slot |= (SK_UH)(up_fctrl.Field.m_SlotL & 0x00FF);
					
					SSMac_GetExtAddress(&join_src, ss_msdu);
					
					#ifdef DEBUG_JOIN
					SK_print("SS_MAC_CMD_JOIN:");
					SK_print_hex(src_slot, 4); SK_print("\r\n");
					#endif

					join_status = SS_STAT_JOIN_OK;
					
					//スロットDBにデバイスを登録
					//すでにスロットが別デバイスで埋まっている時は
					//代替スロットをアサインして応答する
					reg_ans = AutoRegisterDevSlot(&join_src, src_slot, &reg_slot, &res_idx);
					if( reg_ans == FALSE ){
						join_status = SS_STAT_DEVICE_FULL;
						reg_slot = 0xFFFF;
					}
					
					//size of down_fctrl.Field.m_InSlotIdx is 1 bit at this moment,
					//so idx must be 0 or 1
					if( res_idx != 0xFF ){
						down_fctrl.Field.m_InSlotIdx = res_idx;
					} else {
						down_fctrl.Field.m_InSlotIdx = 0;
						join_status = SS_STAT_DEVICE_FULL;
						reg_slot = 0xFFFF;
					}
					
					src_addr = &join_src;
					
					//
					// now joinning deivce is success
					//
					cmd_offset = 0;
					
					if (SK_AllocDataMemory((SK_VP *)&SdReq) != SK_E_OK) { 
						break; 
					}
					
					SSMac_CopyAddress(&(SdReq->m_DstAddress), &join_src);
					
					SdReq->m_Selector = SS_SELECTOR_JOIN_RES_CMD;
					SdReq->m_MsduLength = SS_JOIN_RES_CMD_LEN;
					
					#if(BASE_PHY_TYPE == SS_PHY_15_4K)
					
					#else
					SdReq->m_Msdu[cmd_offset] = join_status;
					cmd_offset++;
					
					MAC_SET_LONG(SdReq->m_Msdu + cmd_offset, SdReq->m_DstAddress.Body.Extended.m_Long[0]);
					cmd_offset+=4;
					MAC_SET_LONG(SdReq->m_Msdu + cmd_offset, SdReq->m_DstAddress.Body.Extended.m_Long[1]);
					cmd_offset+=4;
					#endif
					
					item = FindDBItemOf(&SdReq->m_DstAddress);
					if( item != NULL ){
						memcpy(SdReq->m_Msdu + cmd_offset, item->m_Key[res_idx], SS_AES_KEY_LEN);
						cmd_offset+=SS_AES_KEY_LEN;
					} else {
						//there must be an corresponding DB item for the dst addr
						//item == NULL is abnormal case...abort
						SK_FreeMemory(SdReq);
						break;
					}
					
					MAC_SET_WORD(SdReq->m_Msdu + cmd_offset, reg_slot);
					cmd_offset+=2;
					
					#if(BASE_PHY_TYPE == SS_PHY_15_4K)
					
					#else
					SdReq->m_Msdu[cmd_offset] = res_idx;
					cmd_offset++;
					#endif
					
					SdReq->m_TxOptions = 0;
					SdReq->m_TxOptions |= SS_TXOPTIONS_SECURITY;
					SdReq->m_TxOptions |= SS_TXOPTIONS_INDIRECT;
					
					SdReq->m_Handle = (SK_UH)SSMac_GetRand32();
					
					reg_ans = AddToPendingBuf(SdReq);
					
					if( reg_ans == TRUE ){
						PostJoinConf(join_status, reg_slot, res_idx);
					} else {
						PostJoinConf(SS_STAT_INDIRECT_OVERFLOW, reg_slot, res_idx);
					}
					
					SK_FreeMemory(SdReq);
					
					//if (SK_PostMessage(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_DATA_REQUEST_CMD, (SK_VP)SdReq) != SK_E_OK) {
					//	SK_FreeMemory(SdReq);
					//}

					break; 
				} //end of case SS_SELECTOR_JOIN_CMD
				
				// --Join_Res cmd
				case SS_SELECTOR_JOIN_RES_CMD:{
					SK_UB cmd_offset = 0;
					SK_UB status;
					#if(BASE_PHY_TYPE != SS_PHY_15_4K)
					SK_UW lower_addr, upper_addr;
					#endif
					
					if( ss_msdu_len < SS_JOIN_RES_CMD_LEN ){
						break;
					}

					//change ss_msdu pointer to head of decrypted join_res cmd
					//Because join_res is encrypted, ss_msdu must points decrypted payload body
					//SdInd->m_Msdu is already decrypted
					ss_msdu = SdInd->m_Msdu;
					
					#if(BASE_PHY_TYPE == SS_PHY_15_4K)
					status = SS_STAT_JOIN_OK;
					#else
					status = ss_msdu[cmd_offset];
					cmd_offset++;
					
					upper_addr = MAC_GET_LONG(ss_msdu + cmd_offset);
					cmd_offset+=4;
					lower_addr = MAC_GET_LONG(ss_msdu + cmd_offset);
					cmd_offset+=4;
					
					//this must be ignored
					if( (gnSSMac_Address.Body.Extended.m_Long[0] != upper_addr) ||
						(gnSSMac_Address.Body.Extended.m_Long[1] != lower_addr) ){
						#ifdef DEBUG_JOIN
						SK_print("JOIN_RES is not belongs to me.\r\n");
						#endif
						
						PostJoinConf(SS_STAT_JOIN_CONFLICT, 0xFFFF, 0xFF);
						
						StopSync();
						PostSyncInd(SS_MAC_SYNC_LOSS);
						break;
					}
	
					if( status != SS_STAT_JOIN_OK ){
						PostJoinConf(status, 0xFFFF, 0xFF);

						StopSync();
						PostSyncInd(SS_MAC_SYNC_LOSS);
						break;
					}
					#endif
					
					memcpy(gaSSMac_AESKey, ss_msdu + cmd_offset, SS_AES_KEY_LEN);
					cmd_offset+=SS_AES_KEY_LEN;

					gnSSMac_MySlot = MAC_GET_WORD(ss_msdu + cmd_offset);
					cmd_offset+= 2;

					#if(BASE_PHY_TYPE == SS_PHY_15_4K)
					gnSSMac_InSlotIdx = 0;
					#else
					gnSSMac_InSlotIdx = ss_msdu[cmd_offset];
					cmd_offset++;
					#endif
					
					//reset incoming frame cnt
					gnSSMac_FrameCounterSTA = 0;
					
					//reset my outgoing frame cnt
					gnSSMac_FrameCounter = 0;
					
					#ifdef DEBUG_JOIN
					SK_print("JOIN_RES:\r\n"); 
					SK_print(" status:"); SK_print_hex(status, 2); SK_print("\r\n");
					SK_print(" slot:"); SK_print_hex(gnSSMac_MySlot, 4); SK_print("\r\n");
					SK_print(" idx:"); SK_print_hex(gnSSMac_InSlotIdx, 2); SK_print("\r\n");
					#endif
					
					PostJoinConf(status, gnSSMac_MySlot, gnSSMac_InSlotIdx);

					break;
				} //end of case SS_SELECTOR_JOIN_RES_CMD
				
				default:  
					break;
				} //end of switch(up_fctrl.Field.m_Selector)
				
				//
				// --- End of selector procedure
				//
				
				//
				// Down link 
				//
				//Check if I have pending data for the src device
				//note: src_addr might be NULL
				PendInf = FindPendingBufTo(src_addr);
				if( PendInf != NULL ){
					SK_UB down_slot;
					
					//any free down slot?
					down_slot = FindFreeDownSlot();
					
					#ifdef DEBUG_PEND
					SK_print("Pending down slot:");SK_print_hex(down_slot, 2); SK_print("\r\n");
					#endif
					
					if( down_slot != 0xFF ){
						SK_UH abs_slot;
						//convert down slot -> absolute slot num
						abs_slot = GetUpSlotNum(gnSSMac_SlotMode) + down_slot + 1;
						
						PendInf->m_Packet->m_TxOptions &= ~SS_TXOPTIONS_INDIRECT; //Clear indirect opt
						PendInf->m_DownSlot = down_slot;
						
						#ifdef DEBUG_PEND
						SK_print("Post pendingbuf slot->");SK_print_hex(abs_slot, 4); SK_print("\r\n");
						#endif
						
						//Post data req at specified abs slot
						//if (SK_PostMessageS(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_DATA_REQUEST_CMD, (SK_VP)PendInf->m_Packet, abs_slot) == SK_E_OK) {
						if (SK_PostMessageS(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_PEND_DATA_REQUEST_CMD, (SK_VP)PendInf->m_Packet, abs_slot) == SK_E_OK) { 
							//ready header flags in ack
						  	pend_slot = down_slot;
							down_fctrl.Field.m_FramePending = 1;
							
							//20170116 fix
							//PostMessageが成功したならPendInf->m_PacketはPost先で開放されるので
							//こちらもクリアしなくてはならない
							//SK_print("c:");SK_print_hex((SK_UW)PendInf->m_Packet, 8); SK_print("\r\n");
							PendInf->m_Packet = NULL;
							FreePendingData(PendInf);
							//
						}  
					} else {
						//here,	slot for down link is full
						//20161004 join_res cmd must not to be kept till next time slot cycle
						if( PendInf->m_Packet->m_Selector == SS_SELECTOR_JOIN_RES_CMD ){
							FreePendingData(PendInf);
						}
					}
				}
				
				// Ack response
				if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
					#ifdef DEBUG_ACK
					SK_print("Sending Ack:"); 
					SK_print_hex(down_fctrl.Field.m_InSlotIdx, 1); SK_print(" ");
					SK_print_hex(seq_no, 2); SK_print(" "); 
					SK_print_hex(drift, 2);  SK_print(" "); 
					SK_print_hex(pend_slot, 2);  SK_print("\r\n");
					#endif
					
					SKMac_TransmitAck(&down_fctrl, recv_seq_no, drift, pend_slot);
				}
				
				break;
			}
			
			case SS_FRAME_ACK:{
				SS_DOWN_FRAMECONTROL down_fctrl;
				SK_UB seq_no;
				SK_UB drift, pend_slot;
				
				//自スロットでないAckは破棄
				if( SSMac_GetDeviceType() == SS_TYPE_DEVICE ){
					if( SSMac_GetSlotNum() != MdInd->m_RecvSlot ){
						SS_STATS(mac.recv_drop);
						break;	
					}
				} else {
					//Enddevice以外はAck処理は不要
					SS_STATS(mac.recv_drop);
					break;
				}
				
				pos = SSMac_AnalyzeAck(	MdInd->m_Msdu + offset,
										(SK_UB)phr.Field.m_Length,
										&down_fctrl, 
										&seq_no,
										&drift,
										&pend_slot);
										
				#ifdef DEBUG_ACK
				SK_print("ACK:");
				SK_print_hex(down_fctrl.Field.m_InSlotIdx, 1); SK_print(" ");
				SK_print_hex(down_fctrl.Field.m_FramePending, 1); SK_print(" ");
				SK_print_hex(down_fctrl.Field.m_Selector, 2); SK_print(" ");
				SK_print_hex(seq_no, 2); SK_print(" ");
				SK_print_hex(drift, 2); SK_print(" ");
				SK_print_hex(pend_slot, 2); SK_print("\r\n");
				#endif

				if( (pos == 0) || (phr.Field.m_Length < pos) ) {
				  	SS_STATS(mac.recv_drop);
					break;	
				}
				
				#if 1
				//スロット内indexが違う = 自分宛てでない
				if( SSMac_GetDeviceType() == SS_TYPE_DEVICE ){
					if( gnSSMac_InSlotIdx != 0xFF && gnSSMac_InSlotIdx != down_fctrl.Field.m_InSlotIdx ){
						#ifdef DEBUG_DATA
						SK_print("device: receiving ack different idx...discard.\r\n");
						SK_print_hex(gnSSMac_InSlotIdx, 2); SK_print(" ");
						SK_print_hex(down_fctrl.Field.m_InSlotIdx, 2); SK_print("\r\n");
						#endif
						
						SS_STATS(mac.recv_drop);
						break;
					}
				}
				#endif
				
				if( pend_slot != 0xFF ){
					gnPendingSlot = GetUpSlotNum(gnSSMac_SlotMode) + pend_slot + 1;
				}
				
				SS_STATS(mac.recv_ack);
				PostAckInd(&down_fctrl, pend_slot);

				//DebugPort_Set(0, ((~DebugPort_Get(0)) & 0x01));

				break;
			}
				
			case SS_FRAME_BEACON:{
				SS_BCN_FRAMECONTROL bcn_fctrl;
				SK_UB bsn;
				SS_SLOT_CONFIG slot_cfg;
				SK_UW sta_id;
				//SK_UB hashkey[SS_SLOT_HASHKEY_LEN];
				SK_UW rand_s;
				
				//エンドデバイス以外はビーコンを処理しない
				if( SSMac_GetDeviceType() != SS_TYPE_DEVICE ){
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				pos = SSMac_AnalyzeBeacon(	MdInd->m_Msdu + offset,
											(SK_UB)phr.Field.m_Length,
											&bcn_fctrl, 
											&bsn, 
											&slot_cfg, 
											&sta_id, 
											&rand_s);

				if( (pos == 0) || (phr.Field.m_Length < pos) ) {
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				//Station Id未設定
				// =どのStationに接続するか未確定
				// =上位にビーコン受信通知を行い選択を促す
				if( SSMac_GetStationId() == 0xFFFFFFFFU ){
					goto bcn_notify;
				}
				
				//Station Idが一致しないビーコンは破棄
				if( SSMac_GetStationId() != sta_id ){
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				gnLastBeaconRecvTime = SK_GetLongTick();
				
				//ビーコン受信による時刻補正
				SSMac_AdjustSlot(bsn, &slot_cfg, MdInd->m_TimeStamp);
				
				//memcpy(gaSSMac_SlotHashKey, hashkey, SS_SLOT_HASHKEY_LEN);
				gnSSMac_RandS = rand_s;
				
				gnSSMac_SlotMode = slot_cfg.Field.m_SlotMode;
				
				gnSSMac_CurrentHoppingTable = slot_cfg.Field.m_HoppingTable;

				//joinの結果、slot移動している可能性があるため
				//0xFFFF 未設定の場合のみ、ハッシュから計算割当する
				if( gnSSMac_MySlot == 0xFFFF ){
					gnSSMac_MySlot = CalcMySlot(gaSSMac_SlotHashKey);
				}
				
				//end deviceはgnSSMac_BSNを使ってステーションの現在BSNを保存
				// ->周波数ホッピング計算に使う
				gnSSMac_BSN = bsn;
				
				PostSyncInd(SS_MAC_SYNC_START);
				
				bcn_notify:
				SS_STATS(mac.recv_bcn);
				PostBeaconNotifyInd(&bcn_fctrl, &slot_cfg, bsn, sta_id, MdInd->m_Rssi);
				
				break;
			}
			
			case SS_FRAME_META_BEACON:{
				SK_UW sta_id;

				//エンドデバイス以外はメタビーコンを処理しない
				if( SSMac_GetDeviceType() != SS_TYPE_DEVICE ){
				  	SS_STATS(mac.recv_drop);
					break;
				}
				
				pos = SSMac_AnalyzeMetaBeacon(	MdInd->m_Msdu + offset,
											(SK_UB)phr.Field.m_Length,
											&sta_id, 
											gaSSMac_DevInfo);

				if( (pos == 0) || (phr.Field.m_Length < pos) ) {
				  	SS_STATS(mac.recv_drop);
					break;	
				}
				
				SS_STATS(mac.recv_metabcn);
				PostMetaBeaconNotifyInd(sta_id, gaSSMac_DevInfo, SS_MAX_NIC_NUM);
				break;
			}
				
			default:
				break;
			} // end of switch(phr.Field.m_Type){
			
			break; //end of case SK_MCPS_DATA_INDICATION_CMD
		}
		
		case SK_MCPS_DATA_CONFIRM_CMD:{
			#ifdef DEBUG_DATA
			SK_print("SK_MCPS_DATA_CONFIRM_CMD:\r\n");
			#endif
			
			if( ((SK_MCPS_DATA_CONFIRM *)pResPkt)->m_Status == SK_PHY_NO_RESPONSE ){
				PostCommStatInd(SK_PHY_NO_RESPONSE);
			}

			break;
		}
		
		case SS_JOIN_REQUEST_CMD:{
			SS_DATA_REQUEST* SdReq;
			
			#ifdef DEBUG_JOIN
			{
			  	SS_JOIN_REQUEST* SjReq = (SS_JOIN_REQUEST *)pPkt;
				SK_print("SS_JOIN_REQUEST_CMD:");
				SK_print_hex(SjReq->m_StationID, 8);
				SK_print("\r\n");
			}
			#endif

			if( SSMac_GetSlotNum() == 0xFFFF ){
				PostCommStatInd(SS_MAC_INVALID_REQUEST);
				break;
			}
			
			if (SK_AllocDataMemory((SK_VP *)&SdReq) != SK_E_OK) { 
				break; 
			}

			SdReq->m_Selector = SS_SELECTOR_JOIN_CMD;
			SdReq->m_MsduLength = SS_JOIN_CMD_LEN; 

			SdReq->m_TxOptions = 0; 
			SdReq->m_TxOptions |= SS_TXOPTIONS_SECURITY;
			
			SdReq->m_DstAddress.Body.Extended.m_Long[0] = 0;
			SdReq->m_DstAddress.Body.Extended.m_Long[1] = 0;
			SdReq->m_Handle = (SK_UH)SSMac_GetRand32();
			
			//Join cmd body
			SSMac_SetAddress(SdReq->m_Msdu, &gnSSMac_Address);
			
			if (SK_PostMessageS(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_DATA_REQUEST_CMD, (SK_VP)SdReq, SSMac_GetSlotNum()) != SK_E_OK) {
				SK_FreeMemory(SdReq);
			}

			break;
		} 
		
		case SS_SLOT_CHANGED_INDICATION_CMD:{
			SK_UH slot;
			SS_SLOT_CHANGED_INDICATION* SlotInd = (SS_SLOT_CHANGED_INDICATION *)pPkt;
			
			#ifdef DEBUG_SLOT
			if( SlotInd->m_SlotNum % 10 == 0 ){
				SK_print("SLOT-CHANGED:");
				SK_print_hex(SlotInd->m_SlotNum, 4);
				SK_print("\r\n");
			}
			#endif
			
			slot = SlotInd->m_SlotNum;
			
			/*
			if( SSMac_GetDeviceType() == SS_TYPE_DEVICE && SSMac_GetSlotNum() == 0xFFFF ){
				//ビーコン非同期のエンドデバイスはまだホッピング制御してはいけない
			} else {
				SSMac_ChannelHopping(slot, gnSSMac_BSCN);
			}
			*/
			
			#ifdef DEBUG_FH
			//if( SlotInd->m_SlotNum % 10 == 0 ){
				SK_print("slot:");
				SK_print_hex(SlotInd->m_SlotNum, 2); 
				SK_print(" channel:");
				SK_print_hex(gnPHY_CurrentChannel, 2);
				SK_print("\r\n");
			//}
			#endif
			
			if( (slot == 0) && (SSMac_GetDeviceType() == SS_TYPE_STATION) ){
				SSMac_TransmitBeacon();	
			}
			
			if( (slot == 0) && (SSMac_GetDeviceType() == SS_TYPE_DEVICE) ){
				//ペンディングスロットは一度だけ有効
				gnPendingSlot = 0xFFFF;	
			}

			if( CanSleep(slot) == TRUE ){
				#ifdef DEBUG_SLEEP
				SK_print("RF Sleep\r\n");
				#endif

				SSMac_Sleep(FALSE);
				
				#ifdef MCU_SLEEP
				if( slot == 1 ){
					//送信データが１つも無いならば自スロットを飛ばして次のビーコンスロットまでスリープ
					if( SK_CountFreeDataMemory() == DEF_DATA_MEMBLOCK_NUM ){
						SleepTimer_Go(GetAllSlotNum(gnSSMac_SlotMode) - 1);
					} else {
						SleepTimer_Go(gnSSMac_MySlot - 1);
					}
				} else if( slot == gnSSMac_MySlot + 1 ){
					if( gnPendingSlot == 0xFFFF ){
						SleepTimer_Go(GetAllSlotNum(gnSSMac_SlotMode) - 1);
					} else {
						SleepTimer_Go(gnPendingSlot - 1);
					}
				} else if( slot == gnPendingSlot + 1 ){
					SleepTimer_Go(GetAllSlotNum(gnSSMac_SlotMode) - 1);
					
				} else {
				  	if( slot + 1 == gnSSMac_MySlot ){
					  	//keep wakeup
					} else if( slot + 2 <= gnSSMac_MySlot ){
						SleepTimer_Go(gnSSMac_MySlot - 1);	
					} else if( gnPendingSlot != 0xFFFF && (slot + 1 == gnPendingSlot) ){
					  	//keep wakeup
					} else if( gnPendingSlot != 0xFFFF && (slot + 2 <= gnPendingSlot) ){
						SleepTimer_Go(gnPendingSlot - 1);
					} else if( slot + 2 <= GetAllSlotNum(gnSSMac_SlotMode) ){
						SleepTimer_Go(GetAllSlotNum(gnSSMac_SlotMode) - 1);
					}
				}
				#endif
			}
			
			//ペンディングスロットは一度だけ有効
			//ここで0xFFFFにすると if( slot == gnPendingSlot + 1 ) の条件で
			//MCUスリープできない
			//if( gnPendingSlot == slot ){
			//	gnPendingSlot = 0xFFFF;	
			//}

			break;
		} 
		
 		case SS_SLOT_PRE_CHANGE_INDICATION_CMD:{
			SS_SLOT_PRE_CHANGE_INDICATION* SlotInd = (SS_SLOT_PRE_CHANGE_INDICATION *)pPkt;
			SK_UH next_slot;
			SK_BOOL need_wake = FALSE;
			
			/*
			#ifdef DEBUG_SLOT
			SK_print("SLOT-PRE-CHANGE:");
			SK_print_hex(SlotInd->m_SlotNum, 4);
			SK_print("\r\n");
			#endif
			*/
			next_slot = SlotInd->m_SlotNum + 1;
			if( NeedWakeup(next_slot) == TRUE ){
				#ifdef DEBUG_SLEEP
				SK_print("RF Wake\r\n");
				#endif

				need_wake = TRUE;
				SSMac_Wakeup();
			}

			if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
				//Idlingではホッピング制御してはいけない
			} else if( SSMac_GetDeviceType() == SS_TYPE_DEVICE && SSMac_GetSlotNum() == 0xFFFF ){
				//ビーコン非同期のエンドデバイスはまだホッピング制御してはいけない
			} else {
				if( GetAllSlotNum(gnSSMac_SlotMode) == next_slot ){
					next_slot = 0;
				}
				
				//次のスロットに備えてここで周波数ホッピングする
				if( SSMac_GetDeviceType() == SS_TYPE_DEVICE ){
					//Enddeviceは次が起床すべきときだけホッピング
					//RFスリープ状態でチャンネル変更を実行すると起床状態になるため
					//自スロットでは　need_wake==FALSEになるので注意
					if( need_wake == TRUE ){
						SSMac_ChannelHopping(next_slot, gnSSMac_BSN);
					}
				} else {
					//Station
					SSMac_ChannelHopping(next_slot, gnSSMac_BSN);
				}
			}
	
			break;
		} 
			
		default:
			break;
		}
		// -------------------------------------------
		// dispose memory block
		if (pPkt  != NULL) SK_FreeMemory(pPkt);
	}
	// -------------------------------------------------

	SK_STATEEND(SSMAC);
	
	CheckSyncLoss();
	
	CheckPendExpire();
	
	if( gSlotChanged != 0xFFFF ){
		PostSlotChangedExec(gSlotChanged);
	  	gSlotChanged = 0xFFFF;
	}
	
	if( gSlotPreChanged != 0xFFFF ){
	  	PostSlotPreChangeExec(gSlotPreChanged);
	  	gSlotPreChanged = 0xFFFF;
	}
}


// -------------------------------------------------
//   下り保留データの期限切れチェック
// -------------------------------------------------
static void CheckPendExpire(void){
	SK_UH i;
	
	//ループパフォーマンスを上げるため50msec間隔でチェック
	if( ((SK_UW)(SK_GetLongTick() - gnLastPendCheck)) < 50 ){
		return;	
	}
	
	gnLastPendCheck = SK_GetLongTick();
	
	for( i = 0; i < SS_PENDINGBUF_SIZE; i++ ){
		if( gaSSMac_PendingBuf[i].m_Packet == NULL ) continue;
		
		if( ((SK_UW)(SK_GetLongTick() - gaSSMac_PendingBuf[i].m_TimeStamp)) > gnSSMac_PendingExpireTime ){
			PostDataConf(SS_MAC_INDIRECT_EXPIRED, gaSSMac_PendingBuf[i].m_Packet);
			FreePendingData( &(gaSSMac_PendingBuf[i]) );
		}
	}
}


// -------------------------------------------------
//   同期ロストチェック
// -------------------------------------------------
static void CheckSyncLoss(void){
	SK_UW threshold;
	
	if( gnSSMac_DeviceType != SS_TYPE_DEVICE ) {
		return;
	}

	if( gnLastBeaconRecvTime == 0 ) {
		return;
	}
	
	//ループパフォーマンスを上げるため50msec間隔でチェック
	if( ((SK_UW)(SK_GetLongTick() - gnLastSyncLossCheck)) < 50 ){
		return;	
	}
	
	gnLastSyncLossCheck = SK_GetLongTick();
	
	//ビーコンが未受信でも良い最長時間をミリ秒で計算
	//同期ロスト閾値 * １スロットの時間 * 1周期のスロット数
	threshold = ((SK_UW)gnSSMac_SyncLossThreshold) * SS_SLOT_DURATION * (SK_UW)(GetAllSlotNum(gnSSMac_SlotMode) + 1);
	
	if( ((SK_UW)(SK_GetLongTick() - gnLastBeaconRecvTime)) > threshold ){
		StopSync();
		PostSyncInd(SS_MAC_SYNC_LOSS);
	}
}


SK_BOOL CheckMacCmdAccept(SK_UB cmd){
	switch(cmd){
	
	//no mac cmd at this moment
	
	default:
		return TRUE;
	}
}


// -------------------------------------------------
//   Selector毎の受け入れ条件チェック
// TRUE:受け入れ, FALSE:拒否
// -------------------------------------------------
SK_BOOL CheckSelectorAccept(SK_UB selector){
	switch(selector){
	case SS_SELECTOR_MAC_CMD:
		return TRUE;
		
	case SS_SELECTOR_JOIN_CMD:
		if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
			return TRUE;
		} else {
			return FALSE;
		}
		
	case SS_SELECTOR_JOIN_RES_CMD:
		if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
			return FALSE;
		} else {
			return TRUE;
		}
		
	case SS_SELECTOR_APP_DATA:
		return TRUE;
	
	default:
		return FALSE;
	}
}



//スロット移動の直前に呼ばれる
//引数slotは現在スロットの次のスロットが渡される
SK_BOOL NeedWakeup(SK_UH slot){
	if( SSMac_GetDeviceType() == SS_TYPE_STATION ) {
		return TRUE;
	}
	
	//slot 0の１個手前に来た
	//1024構成の場合、1023番が最終スロット番号
	//slot==1024ということは、次は0ということ
	if( GetAllSlotNum(gnSSMac_SlotMode) == slot ){
		#ifdef DEBUG_SLEEP
		SK_print("Wake for beacon\r\n");
		#endif

		return TRUE;
			
	//自スロットの１個手前に来た
	// 送信データがない場合があるので、
	// wakeupはphyで自動起床させ自スロットでは起床しない
	/*} else if( SSMac_GetMySlot() == slot ){
		#ifdef DEBUG_SLEEP
		SK_print("Wake for my slot\r\n");
		#endif
			
		return TRUE;
	*/	
	//自分宛てデータ用スロットの１個手前に来た
	} else if( (gnPendingSlot != 0xFFFF) && (slot == gnPendingSlot) ){
		#ifdef DEBUG_SLEEP
		SK_print("Wake for down\r\n");
		#endif
			
		return TRUE;		
	}
	
	return FALSE;
}


//スロット移動の直後に呼ばれる
SK_BOOL CanSleep(SK_UH slot){
	if( SSMac_GetDeviceType() == SS_TYPE_STATION ) {
		return FALSE;
	}
	
	//ビーコン未受信
	if( gnSSMac_MySlot == 0xFFFF ){
		return FALSE;	
	}
	
	if( gnSSMac_RxOnWhenIdle == TRUE ){
		return FALSE;
	}
	
	//slot 0は寝れない
	if( slot == 0 ){
		
		return FALSE;
			
	//自スロットは寝れない
	//（送信データがなくスリープしている場合はスリープを維持）
	} else if( SSMac_GetSlotNum() == slot ){
		
		return FALSE;

	//自分宛てデータ用スロットの１個手前に来た
	} else if( (gnPendingSlot != 0xFFFF) && (slot == gnPendingSlot) ){
	
		return FALSE;		

	}

	return TRUE;
}


// -------------------------------------------------
//   Init Slot-Addr database
// -------------------------------------------------
void SSMac_InitDB(SS_SLOT_ADDR_DB_ITEM db[], SK_UH size){
	SK_UH i;
	SK_UB j;
	
	for( i = 0; i < size; i++ ){
		db[i].m_SlotNum = 0xFFFF;
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			SSMac_InitAddr(&(db[i].m_Addr[j]));
			memset(db[i].m_Key[j], 0, SS_AES_KEY_LEN);
			db[i].m_FrameCounter[j] = SS_MAX_FRAME_COUNTER;
			db[i].m_OutgoingFrameCounter[j] = 0;
		}
	}
}


// -------------------------------------------------
//   Get Slot-Addr item correspinding to specified slot
// -------------------------------------------------
static SS_SLOT_ADDR_DB_ITEM* FindDBItemFor(SK_UH slot){
	SK_UH i;
	
	if( slot > GetUpSlotNum(gnSSMac_SlotMode) ) {
		return NULL;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		if( gaSSMac_SlotAddr_DB[i].m_SlotNum == slot ){
			return &(gaSSMac_SlotAddr_DB[i]);
		}
	}
	return NULL;
}


SS_SLOT_ADDR_DB_ITEM* SSMac_GetDBItemFor(SK_UH slot){
	return FindDBItemFor(slot);
}


// -------------------------------------------------
//   Get Slot-Addr item correspinding to specified addr
// -------------------------------------------------
static SS_SLOT_ADDR_DB_ITEM* FindDBItemOf(SK_ADDRESS* addr){
	SK_UH i;
	SK_UB j;

	if( addr->Body.Extended.m_Long[0] == 0 && addr->Body.Extended.m_Long[1] == 0 ){
	  	return NULL;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( SSMac_Equals( &(gaSSMac_SlotAddr_DB[i].m_Addr[j]), addr ) == TRUE ){
				return &(gaSSMac_SlotAddr_DB[i]);
			}
		}
	}
	
	return NULL;
}


// -------------------------------------------------
//   Get free Slot-Addr item
// -------------------------------------------------
static SS_SLOT_ADDR_DB_ITEM* GetFreeDBItem(void){
	SK_UH i;
	SK_UH max;
	
	max = GetUpSlotNum(gnSSMac_SlotMode);
	
	//最大スロット数とSS_DB_SIZEが一致してなくても処理できるよう修正
	// ->例：64 slot周期をSS_DB_SIZE=16設定でも処理できるように
	//if( max > SS_DB_SIZE ) return NULL;
	if( max > SS_DB_SIZE ) {
		max = SS_DB_SIZE;
	}
	
	for( i = 0; i < max; i++ ){
		if( gaSSMac_SlotAddr_DB[i].m_SlotNum == 0xFFFF ){
			return &(gaSSMac_SlotAddr_DB[i]);
		}
	}
	return NULL;
}


// -------------------------------------------------
//   Get address correspinding to the index in the slot 
// -------------------------------------------------
static SK_ADDRESS* GetDBAddressFor(SK_UH slot, SK_UB idx){
	SK_UH i;
	
	if( slot > GetUpSlotNum(gnSSMac_SlotMode) ) {
		return NULL;
	}
	
	if( idx >= SS_MAX_SHARED_DEVICES ) {
		return NULL;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		if( gaSSMac_SlotAddr_DB[i].m_SlotNum == slot ){
			return &(gaSSMac_SlotAddr_DB[i].m_Addr[idx]);
		}
	}
	
	return NULL;
}


// -------------------------------------------------
//   Get in slot index for the addr 
// return 0xFF if slot idx for addr is not found
// -------------------------------------------------
static SK_UB GetInSlotIdxOf(SK_ADDRESS* addr){
	SK_UH i;
	SK_UB j;

	if( addr->Body.Extended.m_Long[0] == 0 && addr->Body.Extended.m_Long[1] == 0 ){
	  	return 0xFF;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( SSMac_Equals( &(gaSSMac_SlotAddr_DB[i].m_Addr[j]), addr ) ){
				return j;
			}
		}
	}
	
	//indicates not found
	return 0xFF;
}


// -------------------------------------------------
//   Get encrypt key for the addr
// -------------------------------------------------
static SK_UB* GetEncKeyOf(SK_ADDRESS* addr){
	SK_UH i;
	SK_UB j;

	if( addr->Body.Extended.m_Long[0] == 0 && addr->Body.Extended.m_Long[1] == 0 ){
	  	return NULL;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( SSMac_Equals( &(gaSSMac_SlotAddr_DB[i].m_Addr[j]), addr ) ){
				return &(gaSSMac_SlotAddr_DB[i].m_Key[j][0]);
			}
		}
	}
	return NULL;
}


// -------------------------------------------------
//   Set encrypt key for the addr
// -------------------------------------------------
SK_BOOL SetEncKeyOf(SK_ADDRESS* addr, SK_UB* key, SK_UB key_len){
	SK_UH i;
	SK_UB j;
	
	if( addr->Body.Extended.m_Long[0] == 0 && addr->Body.Extended.m_Long[1] == 0 ){
	  	return FALSE;
	}
	
	if( key_len != SS_AES_KEY_LEN ) return FALSE;

	for( i = 0; i < SS_DB_SIZE; i++ ){
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( SSMac_Equals( &(gaSSMac_SlotAddr_DB[i].m_Addr[j]), addr ) ){
				memcpy(&(gaSSMac_SlotAddr_DB[i].m_Key[j][0]), key, SS_AES_KEY_LEN);
				return TRUE;
			}
		}
	}
	return FALSE;
}


// -------------------------------------------------
//   Get outgoing frame counter for the addr
// -------------------------------------------------
SK_UW GetFrameCounterFor(SK_ADDRESS* addr){
	SK_UH i;
	SK_UB j;

	if( addr->Body.Extended.m_Long[0] == 0 && addr->Body.Extended.m_Long[1] == 0 ){
	  	return 0xFFFFFFFFU;
	}
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( SSMac_Equals( &(gaSSMac_SlotAddr_DB[i].m_Addr[j]), addr ) ){
				return gaSSMac_SlotAddr_DB[i].m_OutgoingFrameCounter[j];
			}
		}
	}
	
	//indicates not found
	return 0xFFFFFFFFU;
}


// -------------------------------------------------
//   Init pending buf
// -------------------------------------------------
void SSMac_InitPendingBuf(SS_PENDING_INFO buf[], SK_UH size){
	SK_UH i;

	for( i = 0; i < size; i++ ){
		buf[i].m_Packet = NULL;
		buf[i].m_TimeStamp = 0;
		buf[i].m_DownSlot =0xFF;
	}
}


// -------------------------------------------------
//   Add data request to pending buf for down link
// -------------------------------------------------
static SK_BOOL AddToPendingBuf(SS_DATA_REQUEST* SdReq){
	SK_UH i, j;
	SS_DATA_REQUEST* SdCpy;
								
	for( i = 0; i < SS_PENDINGBUF_SIZE; i++ ){
		if( gaSSMac_PendingBuf[i].m_Packet == NULL ){
			if (SK_AllocDataMemory((SK_VP *)&SdCpy) != SK_E_OK) {
				return FALSE;
			}
	
			for( j = 0; j < SK_DATA_MEMBLOCK_SIZE; j++ ){
				((SK_UB*)SdCpy)[j] = ((SK_UB*)SdReq)[j];
			}
	
			gaSSMac_PendingBuf[i].m_Packet = SdCpy;
			gaSSMac_PendingBuf[i].m_TimeStamp = SK_GetLongTick();
			return TRUE;
		}
	}
	
	return FALSE;
}


// -------------------------------------------------
//   Free pending buf item
// -------------------------------------------------
static void FreePendingData(SS_PENDING_INFO* PendInf){
	PendInf->m_TimeStamp = 0;
	if( PendInf->m_Packet != NULL ){
		SK_FreeMemory(PendInf->m_Packet);
	}
	PendInf->m_Packet = NULL;
	PendInf->m_DownSlot = 0xFF;
}


// -------------------------------------------------
//   Find pending data of the down slot
// -------------------------------------------------
static SS_PENDING_INFO* FindPendingBufOfSlot(SK_UB downslot){
	SK_UH i;
	
	for( i = 0; i < SS_PENDINGBUF_SIZE; i++ ){
		if( gaSSMac_PendingBuf[i].m_Packet == NULL ) continue;
		
		if( gaSSMac_PendingBuf[i].m_DownSlot == downslot ){
			return &(gaSSMac_PendingBuf[i]);
		}	
	}
	
	return NULL;
}


// -------------------------------------------------
//   Find pending data sent to the dst
// -------------------------------------------------
static SS_PENDING_INFO* FindPendingBufTo(SK_ADDRESS* dst){
	SK_UH i;
	
	if( dst == NULL ) {
		return NULL;	
	}
	
	for( i = 0; i < SS_PENDINGBUF_SIZE; i++ ){
		if( gaSSMac_PendingBuf[i].m_Packet == NULL ) continue;
		
		if( SSMac_Equals(&(gaSSMac_PendingBuf[i].m_Packet->m_DstAddress), dst) == TRUE ){
			return &(gaSSMac_PendingBuf[i]);
		}	
	}
	
	return NULL;
}

	
// -------------------------------------------------
//   Find pending data for the slot
// -------------------------------------------------
static SS_PENDING_INFO* FindPendingBufFor(SK_UH slot){
	SK_UH i;
	SK_UB j;
	SS_PENDING_INFO* info;
	
	for( i = 0; i < SS_DB_SIZE; i++ ){
		if( gaSSMac_SlotAddr_DB[i].m_SlotNum != slot ) continue;
		
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			info = FindPendingBufTo(&(gaSSMac_SlotAddr_DB[i].m_Addr[j]));
			if( info != NULL ){
				return info;	
			}
		}
		
		//slot値が一致した最初のエントリーのチェックのみで十分
		return NULL;
	}
	
	return NULL;
}
	
	
// -------------------------------------------------
//   Init SK_ADDRESS
// -------------------------------------------------
void SSMac_InitAddr(SK_ADDRESS* addr){
	addr->m_AddrMode = SK_ADDRMODE_EXTENDED;
	addr->Body.Extended.m_Long[0] = 0L;
	addr->Body.Extended.m_Long[1] = 0L;
}


// -------------------------------------------------
//   Register specified address to slot data base
// IN:addr, item, out:idx
// -------------------------------------------------
static SK_BOOL PutAddrToItem(SK_ADDRESS* addr, SS_SLOT_ADDR_DB_ITEM* item, SK_UB* idx){
	SK_UB i;
	for( i = 0; i < SS_MAX_SHARED_DEVICES; i++ ){
		if( (item->m_Addr[i].Body.Extended.m_Long[0] == addr->Body.Extended.m_Long[0]) && 
			(item->m_Addr[i].Body.Extended.m_Long[1] == addr->Body.Extended.m_Long[1]) ){
			*idx = i; 
			return TRUE;
		}

		if( (item->m_Addr[i].Body.Extended.m_Long[0] == 0) && 
			(item->m_Addr[i].Body.Extended.m_Long[1] == 0) ){	
			SSMac_CopyAddress(&(item->m_Addr[i]), addr);
			GenerateEncKey( &(item->m_Key[i][0]), SS_AES_KEY_LEN );
			item->m_FrameCounter[i] = 0; //reset incoming frame counter
			item->m_OutgoingFrameCounter[i] = 0; //reset outgoing frame counter
			*idx = i; 
			return TRUE;
		}
	}
	*idx = 0xFF;
	return FALSE;
}


static SK_BOOL PutAddrToItemWithIdx(SK_ADDRESS* addr, SS_SLOT_ADDR_DB_ITEM* item, SK_UB idx){
	
	if( idx >= SS_MAX_SHARED_DEVICES ) return FALSE;

	//already exit entry
	if( (item->m_Addr[idx].Body.Extended.m_Long[0] == addr->Body.Extended.m_Long[0]) && 
		(item->m_Addr[idx].Body.Extended.m_Long[1] == addr->Body.Extended.m_Long[1]) ){
		return TRUE;
	}
	
	//register new item
	if( (item->m_Addr[idx].Body.Extended.m_Long[0] == 0) && 
		(item->m_Addr[idx].Body.Extended.m_Long[1] == 0) ){	
		SSMac_CopyAddress(&(item->m_Addr[idx]), addr);
		GenerateEncKey( &(item->m_Key[idx][0]), SS_AES_KEY_LEN );
		item->m_FrameCounter[idx] = 0; //reset incoming frame counter
		item->m_OutgoingFrameCounter[idx] = 0; //reset outgoing frame counter
		return TRUE;
	}

	return FALSE;
}


// -------------------------------------------------
//   Slot bitmap operation
// -------------------------------------------------
static void StartSlotBitmap(void){
	memset(gaSSMac_SlotBitmap, 0, sizeof(gaSSMac_SlotBitmap));
	SetSlotBitmap(0);
}


static void SetSlotBitmap(SK_UH slot){
	SK_UH offset;
	SK_UH mod;

	offset = slot / 32;
	if( offset > 31 ) return;
	
	mod = slot % 32;
	gaSSMac_SlotBitmap[offset] |= (1UL << mod);
}


static void ClearSlotBitmap(SK_UH slot){
	SK_UH offset;
	SK_UH mod;

	offset = slot / 32;
	if( offset > 31 ) return;
	
	mod = slot % 32;
	gaSSMac_SlotBitmap[offset] &= ~(1UL << mod);	
}


static SK_BOOL IsSetBitmap(SK_UH slot){
	SK_UH offset;
	SK_UH mod;

	offset = slot / 32;
	if( offset > 31 ) return FALSE;
	
	mod = slot % 32;
	
	if( ((gaSSMac_SlotBitmap[offset] >> mod) & 1L) != 0 ){
		return TRUE;	
	} else {
		return FALSE;	
	}
}


// -------------------------------------------------
//   Build and transmit a beacon
// -------------------------------------------------
void SSMac_TransmitBeacon(void) {
	SK_MCPS_DATA_REQUEST *MdReq;
	SK_UB len;
	
	if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
		return;	
	}
	
	//Sta Id未設定では送信しない
	if( SSMac_GetStationId() == 0xFFFFFFFFU ){
	  	SS_STATS(mac.send_drop);
		return;	
	}

	if (SK_AllocDataMemory((SK_VP *)&MdReq) != SK_E_OK) {
	  	SS_STATS(mac.send_drop);
		return;
	}

	MdReq->m_SrcPanID	= 0xFFFF;

	SSMac_CopyAddress(&MdReq->m_SrcAddr, &gnSSMac_Address);
	MdReq->m_SrcAddr.m_AddrMode = SK_ADDRMODE_EXTENDED;

	MdReq->m_DstPanID	= 0xFFFF;
	MdReq->m_DstAddr.m_AddrMode = SK_ADDRMODE_SHORT;
	MdReq->m_DstAddr.Body.Short.m_Word = 0xFFFF;
	
	MdReq->m_MsduHandle	= 0;
	MdReq->m_TxOptions	= (SK_UB)0;
	
	// ---Encoding Beacon
	len = SSMac_EncodeBeacon(MdReq->m_Msdu, SK_AMAXMACPAYLOADSIZE);
	if( len == 0 ){
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}
	
	MdReq->m_MsduLength = len;

	if (SK_PostMessage(gnSSMac_LowerLayer,SK_LAYER_SS_MAC,SK_MCPS_DATA_REQUEST_CMD,(SK_VP)MdReq) != SK_E_OK) {
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}
	
	SS_STATS(mac.send_bcn);
}


// -------------------------------------------------
//   Build and transmit a meta beacon
// -------------------------------------------------
void SSMac_TransmitMetaBeacon(void) {
	SK_MCPS_DATA_REQUEST *MdReq;
	SK_UB len;

	if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
		return;	
	}
	
	//Sta Id未設定では送信しない
	if( SSMac_GetStationId() == 0xFFFFFFFF ){
	  	SS_STATS(mac.send_drop);
		return;	
	}

	if (SK_AllocDataMemory((SK_VP *)&MdReq) != SK_E_OK) {
	  	SS_STATS(mac.send_drop);
		return;
	}

	MdReq->m_SrcPanID	= 0xFFFF;

	SSMac_CopyAddress(&MdReq->m_SrcAddr, &gnSSMac_Address);
	MdReq->m_SrcAddr.m_AddrMode = SK_ADDRMODE_EXTENDED;

	MdReq->m_DstPanID	= 0xFFFF;
	MdReq->m_DstAddr.m_AddrMode = SK_ADDRMODE_SHORT;
	MdReq->m_DstAddr.Body.Short.m_Word = 0xFFFF;
	
	MdReq->m_MsduHandle	= 0;
	MdReq->m_TxOptions	= (SK_UB)0;
	
	// ---Encoding Beacon
	len = SSMac_EncodeMetaBeacon(MdReq->m_Msdu, SK_AMAXMACPAYLOADSIZE);
	if( len == 0 ){
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}
	
	MdReq->m_MsduLength = len;

	if (SK_PostMessage(gnSSMac_LowerLayer,SK_LAYER_SS_MAC,SK_MCPS_DATA_REQUEST_CMD,(SK_VP)MdReq) != SK_E_OK) {
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}

	SS_STATS(mac.send_metabcn);
}


// -------------------------------------------------
//   Build and transmit a data
// -------------------------------------------------
SK_BOOL SSMac_TransmitData(SS_DATA_REQUEST* SdReq) {
	SK_MCPS_DATA_REQUEST* MdReq;
	SK_UB len;

	if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
		return FALSE;
	}
	
	if (SK_AllocDataMemory((SK_VP *)&MdReq) != SK_E_OK) {
	  	SS_STATS(mac.send_drop);
		return FALSE;
	}

	MdReq->m_SrcPanID	= 0xFFFF;

	SSMac_CopyAddress(&MdReq->m_SrcAddr, &gnSSMac_Address);
	MdReq->m_SrcAddr.m_AddrMode = SK_ADDRMODE_EXTENDED;

	MdReq->m_DstPanID	= 0xFFFF;
	MdReq->m_DstAddr.m_AddrMode = SK_ADDRMODE_SHORT;
	MdReq->m_DstAddr.Body.Short.m_Word = 0xFFFF;
	
	MdReq->m_MsduHandle	= 0;
	MdReq->m_TxOptions	= (SK_UB)0;
	
	// ---Encoding a data
	len = SSMac_EncodeDataReq(SdReq, MdReq->m_Msdu, SK_AMAXMACPAYLOADSIZE);
	if( len == 0 ){
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return FALSE;
	}
	
	MdReq->m_MsduLength = len;

	//Enddeviceは送信手前でChホッピングを実行
	if( SSMac_GetDeviceType() == SS_TYPE_DEVICE ){
		SK_BOOL ans;
		
		//20170404 チャンネル切り替え失敗ならば送信自体をキャンセル
		ans = SSMac_ChannelHopping(gnSSMac_CurrentSlot, gnSSMac_BSN);
		if( ans == FALSE ){
			SK_FreeMemory(MdReq);
			SS_STATS(mac.send_drop);
			return FALSE;
		}
	}
	
	if (SK_PostMessage(gnSSMac_LowerLayer,SK_LAYER_SS_MAC,SK_MCPS_DATA_REQUEST_CMD,(SK_VP)MdReq) != SK_E_OK) {
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return FALSE;

	}

	SS_STATS(mac.send_data);
	
	//gnSSMac_SeqNo++;
	IncSeqNumAndFrameCnt(SdReq);
	
	return TRUE;
}


// -------------------------------------------------
//   Build and transmit an ack
// -------------------------------------------------
void SKMac_TransmitAck(SS_DOWN_FRAMECONTROL* fctrl, SK_UB seq, SK_UB drift, SK_UB pend_slot) {
	SK_MCPS_DATA_REQUEST *MdReq;
	SK_UB len;
	
	if( SSMac_GetDeviceType() == SS_TYPE_IDLING ){
		return;
	}

	if (SK_AllocDataMemory((SK_VP *)&MdReq) != SK_E_OK) {
	  	SS_STATS(mac.send_drop);
		return;
	}

	MdReq->m_SrcPanID	= 0xFFFF;

	SSMac_CopyAddress(&MdReq->m_SrcAddr, &gnSSMac_Address);
	MdReq->m_SrcAddr.m_AddrMode = SK_ADDRMODE_EXTENDED;

	MdReq->m_DstPanID	= 0xFFFF;
	MdReq->m_DstAddr.m_AddrMode = SK_ADDRMODE_SHORT;
	MdReq->m_DstAddr.Body.Short.m_Word = 0xFFFF;
	
	MdReq->m_MsduHandle	= 0;
	MdReq->m_TxOptions	= (SK_UB)0;
	
	// ---Encoding Beacon
	len = SSMac_EncodeAck(fctrl, seq, drift, pend_slot, MdReq->m_Msdu, SK_AMAXMACPAYLOADSIZE);
	if( len == 0 ){
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}
	
	MdReq->m_MsduLength = len;

	if (SK_PostMessage(gnSSMac_LowerLayer,SK_LAYER_SS_MAC,SK_MCPS_DATA_REQUEST_CMD,(SK_VP)MdReq) != SK_E_OK) {
		SK_FreeMemory(MdReq);
		SS_STATS(mac.send_drop);
		return;
	}
	
	SS_STATS(mac.send_ack);
}


// -------------------------------------------------
//   Encode beacon frame
// -------------------------------------------------
SK_UB SSMac_EncodeBeacon(SK_UB* buf, SK_UB len){
	SS_SLOT_CONFIG slot_cfg;
	SS_BCN_FRAMECONTROL bcn_fctrl;
	SS_MHR phr;
	SK_UB offset = 0;
	SK_UB total_len = SS_BEACON_LEN;
	
	if( len < (total_len + SS_MHR_LEN) ) {
		return 0;	
	}
	
	//PHR
	phr.Raw[0] = 0;
	phr.Field.m_Type = SS_FRAME_BEACON;
	phr.Field.m_Length = total_len;
	buf[offset] = phr.Raw[0];
	offset++;
	
	//Beacon framecontrol
	bcn_fctrl.Raw[0] = 0;
	bcn_fctrl.Field.m_Version = SS_PROTOCOL_VERSION;
	bcn_fctrl.Field.m_Capacity = gnSSMac_CurrentCapacity; //@@ need to reflect Capacity status
	buf[offset] = bcn_fctrl.Raw[0];
	offset ++;
	
	//送信前にインクリメント
	//送信後だとFHのチャンネル計算がエンドデバイスと１ずれる
	gnSSMac_BSN++;
	
	//Beacon seq
	buf[offset] = gnSSMac_BSN;
	offset ++;	
	
	//Slot config
	slot_cfg.Raw[0] = 0;
	slot_cfg.Field.m_SlotMode = gnSSMac_SlotMode;
	slot_cfg.Field.m_HoppingTable = gnSSMac_CurrentHoppingTable;
	buf[offset] = slot_cfg.Raw[0];
	offset ++;

	//Station ID
	MAC_SET_LONG(buf + offset, gnSSMac_StationID);
	offset += 4;
	
	//Slot hashkey
	//memcpy(buf + offset, gaSSMac_SlotHashKey, SS_SLOT_HASHKEY_LEN);
	//offset += SS_SLOT_HASHKEY_LEN;
	
	gnSSMac_RandS = SSMac_GetRand32();
	MAC_SET_LONG(buf + offset, gnSSMac_RandS);
	offset += 4;
	
	//to notify beacon transmit to upper layer
	PostBeaconNotifyInd(&bcn_fctrl, &slot_cfg, gnSSMac_BSN, gnSSMac_StationID, 0);

	return offset;
}


// -------------------------------------------------
//   Encode meta beacon frame
// -------------------------------------------------
SK_UB SSMac_EncodeMetaBeacon(SK_UB* buf, SK_UB len){
	SK_UB i;
	SS_NIC_CONFIG nic;
	SS_MHR phr;
	SK_UB offset = 0;
	SK_UB total_len = SS_META_BEACON_LEN;
	
	if( len < (total_len + SS_MHR_LEN) ) {
		return 0;	
	}
	
	//PHR
	phr.Raw[0] = 0;
	phr.Field.m_Type = SS_FRAME_META_BEACON;
	phr.Field.m_Length = total_len;
	buf[offset] = phr.Raw[0];
	offset++;
	
	//Station ID
	MAC_SET_LONG(buf + offset, gnSSMac_StationID);
	offset += 4;
	
	for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
		nic.Raw[0] = 0;
		nic.Raw[1] = 0;
		nic.Raw[2] = 0;

	 	nic.Field.m_SlotH = (SK_UB)((gaSSMac_DevInfo[i].m_CurrentSlot >> 8) & 0x03);
		nic.Field.m_SlotL = (SK_UB)(gaSSMac_DevInfo[i].m_CurrentSlot & 0x00FF);
	 	nic.Field.m_BaseChannel = gaSSMac_DevInfo[i].m_BaseChannel;
		nic.Field.m_SlotMode = gaSSMac_DevInfo[i].m_SlotMode;
		
		buf[offset  ] = nic.Raw[0];
		buf[offset+1] = nic.Raw[1];
		buf[offset+2] = nic.Raw[2];
		offset +=3;
	}
	
	return offset;
}


// -------------------------------------------------
//   Encode data frame
// -------------------------------------------------
SK_UB SSMac_EncodeDataReq(SS_DATA_REQUEST* SdReq, SK_UB* buf, SK_UB len){
	SS_UP_FRAMECONTROL up_fctrl;
	SS_MHR phr;
	SK_UB offset = 0;
	SK_UW frm_cnt;
	SK_UB total_len = SdReq->m_MsduLength + (SK_UB)SS_UPFCTRL_LEN + (SK_UB)SS_SEQ_NO_LEN + (SK_UB)SS_FRAME_CNT_LEN; 
	
	if( (SdReq->m_TxOptions & SS_TXOPTIONS_SECURITY) != 0 ){
		total_len+= SS_MIC_LEN;
	}
	
	if( len < (total_len + SS_MHR_LEN) ) return 0;
	if( SS_MAX_FRAME_LEN < (total_len + SS_MHR_LEN) ) return 0;
	
	//PHR
	phr.Raw[0] = 0;
	phr.Field.m_Type = SS_FRAME_DATA;
	phr.Field.m_Length = total_len;
	buf[offset] = phr.Raw[0];
	offset++;

	//UP Fctrl
	up_fctrl.Raw[0] = 0;
	up_fctrl.Raw[1] = 0;
	
	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		SK_UB idx;
		
		idx = GetInSlotIdxOf(&(SdReq->m_DstAddress));
		if( idx == 0xFF ){
			return 0;	
		}
		
		up_fctrl.Field.m_InSlotIdx = idx;
	} else {
		if( gnSSMac_InSlotIdx == 0xFF ){ 
			//joinが完了してないときは、join_cmdのためidx=0で送信
			up_fctrl.Field.m_InSlotIdx = 0;
		} else {
			up_fctrl.Field.m_InSlotIdx = gnSSMac_InSlotIdx;
		}
	}
	
	if( (SdReq->m_TxOptions & SS_TXOPTIONS_SECURITY) != 0 ){
		up_fctrl.Field.m_Security=1;
	}
	
	up_fctrl.Field.m_Selector = SdReq->m_Selector;
	up_fctrl.Field.m_SlotH = (SK_UB)((SSMac_GetCurrentSlot() >> 8) & 0x03);
	up_fctrl.Field.m_SlotL = (SK_UB)(SSMac_GetCurrentSlot() & 0x00FF);

	buf[offset  ] = up_fctrl.Raw[0];
	buf[offset+1] = up_fctrl.Raw[1];
	offset+=2;
	
	//Seq no
	buf[offset] = gnSSMac_SeqNo;
	offset++;
	
	//frame cnt
	if( SdReq->m_Selector == SS_SELECTOR_JOIN_RES_CMD || SdReq->m_Selector == SS_SELECTOR_JOIN_CMD ){
		frm_cnt = 0;
	} else {
		if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
			frm_cnt = GetFrameCounterFor(&(SdReq->m_DstAddress));
			//対応するフレームカウンタエントリーがない
			if( frm_cnt == 0xFFFFFFFFU ){
				return 0;
			}
		} else {
			frm_cnt = SSMac_GetFrameCounter();
		}
	}
	
	buf[offset    ] = (SK_UB)(frm_cnt & 0x000000FF);
	buf[offset + 1] = (SK_UB)((frm_cnt >> 8) & 0x000000FF);
	buf[offset + 2] = (SK_UB)((frm_cnt >> 16) & 0x000000FF);
	offset+= 3;
	
	//
	// encrypt data here
	//
	if( (SdReq->m_TxOptions & SS_TXOPTIONS_SECURITY) != 0 ){
		SK_BOOL enc_result;
		
		//encrypt SdReq->m_Msdu and copy the enc data to buf
		if( SdReq->m_Selector == SS_SELECTOR_JOIN_RES_CMD ){
			enc_result = EncryptJoinRes(buf, offset, SdReq);
			
		} else if( SdReq->m_Selector == SS_SELECTOR_JOIN_CMD ){
			enc_result = EncryptJoin(buf, offset, SdReq);
		
		} else {
			enc_result = EncryptData(buf, offset, frm_cnt, SdReq);
		}
		
		if( enc_result == FALSE ){
			return 0;	
		}
		offset+= SdReq->m_MsduLength + (SK_UB)SS_MIC_LEN;
	} else {
		//just copy plain data
		memcpy(buf + offset, SdReq->m_Msdu, SdReq->m_MsduLength);
		offset+= SdReq->m_MsduLength;
	}
	
	return offset;
}


// -------------------------------------------------
//   Encode data frame
// -------------------------------------------------
SK_UB SSMac_EncodeAck(
	SS_DOWN_FRAMECONTROL* fctrl, 
	SK_UB seq, SK_UB drift, 
	SK_UB pend_slot, 
	SK_UB* buf, 
	SK_UB len)
{
	SS_MHR phr;
	SK_UB offset = 0;
	SK_UB total_len = SS_ACK_LEN;
	
	if( len < (total_len + SS_MHR_LEN) ) return 0;
	if( SS_MAX_FRAME_LEN < (total_len + SS_MHR_LEN) ) return 0;
	
	phr.Raw[0] = 0;
	phr.Field.m_Type = SS_FRAME_ACK;
	phr.Field.m_Length = total_len;
	buf[offset] = phr.Raw[0];
	offset++;

	buf[offset] = fctrl->Raw[0];
	offset++;

	buf[offset] = seq;
	offset++;
	
	buf[offset] = drift;
	offset++;
	
	buf[offset] = pend_slot;
	offset++;	

	return offset;
}


// -------------------------------------------------
//   Parse beacon frame
// -------------------------------------------------
SK_UB SSMac_AnalyzeBeacon(
	SK_UB* buf, 
	SK_UB len, 
	SS_BCN_FRAMECONTROL* bcn_fctrl,
	SK_UB* bscn,
	SS_SLOT_CONFIG* slot_cfg,
	SK_UW* sta_id,
	SK_UW* rand_s)
{
	SK_UB offset = 0;
	
	if( len < SS_BEACON_LEN ) {
		return 0;	
	}

	//Beacon framecontrol
	bcn_fctrl->Raw[0] = buf[offset];
	offset ++;
	
	//Beacon seq
	*bscn = buf[offset];
	offset ++;	
	
	//Slot config
	slot_cfg->Raw[0] = buf[offset];
	offset ++;

	//Station ID
	*sta_id = MAC_GET_LONG(buf + offset);
	offset += 4;
	
	//Slot hashkey
	//memcpy(hash, buf + offset, SS_SLOT_HASHKEY_LEN);
	//offset += SS_SLOT_HASHKEY_LEN;
	
	*rand_s = MAC_GET_LONG(buf + offset);
	offset += 4;
	
	return offset;
}


// -------------------------------------------------
//   Parse meta beacon frame
// -------------------------------------------------
SK_UB SSMac_AnalyzeMetaBeacon(
	SK_UB* buf, 
	SK_UB len, 
	SK_UW* sta_id,
	SS_DEV_INFO inf[])
{
	SK_UB i;
	SS_NIC_CONFIG nic;
	SK_UB offset = 0;
	
	if( len < SS_META_BEACON_LEN ) {
		return 0;	
	}

	//Station ID
	*sta_id = MAC_GET_LONG(buf + offset);
	offset += 4;
	
	for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
		nic.Raw[0] = buf[offset    ];
		nic.Raw[1] = buf[offset + 1];
		nic.Raw[2] = buf[offset + 2];
		offset +=3;
		
		inf[i].m_CurrentSlot = (SK_UH)(nic.Field.m_SlotH) << 8;
		inf[i].m_CurrentSlot |= (SK_UH)(nic.Field.m_SlotL & 0x00FF);
		inf[i].m_BaseChannel = nic.Field.m_BaseChannel;
		inf[i].m_SlotMode = nic.Field.m_SlotMode;
	}
	
	return offset;
}


// -------------------------------------------------
//   Parse Data frame
// -------------------------------------------------
SK_UB SSMac_AnalyzeDataHdr(
	SK_UB* buf, 
	SK_UB len, 
	SS_UP_FRAMECONTROL* up_fctrl,
	SK_UB* seq_no,
	SK_UW* frame_cnt)
{
	SK_UB offset = 0;
	
	if( len < (SS_UPFCTRL_LEN + SS_SEQ_NO_LEN + SS_FRAME_CNT_LEN) ) {
		return 0;	
	}

	//uplink framecontrol
	up_fctrl->Raw[0] = buf[offset    ];
	up_fctrl->Raw[1] = buf[offset + 1];
	offset+=2;

	//seq no
	*seq_no = buf[offset];
	offset++;	
	
	*frame_cnt = (((SK_UW)buf[offset + 2]) << 16) | (((SK_UW)buf[offset + 1]) << 8) | ((SK_UW)buf[offset]);
	offset+=3;
	
	return offset;
}


// -------------------------------------------------
//   Parse Ack frame
// -------------------------------------------------
SK_UB SSMac_AnalyzeAck(
		SK_UB* buf, 
		SK_UB len, 
		SS_DOWN_FRAMECONTROL* down_fctrl, 
		SK_UB* seq_no, 
		SK_UB* drift, 
		SK_UB* pend_slot)
{
	SK_UB offset = 0;
	
	if( len < SS_ACK_LEN ) {
		return 0;	
	}

	//downslot frame control
	down_fctrl->Raw[0] = buf[offset];
	offset++;
	
	//Seq no
	*seq_no = buf[offset];
	offset++;	
	
	//time drift
	*drift = buf[offset];
	offset++;

	//pending slot
	*pend_slot = buf[offset];
	offset++;

	return offset;
}


// -------------------------------------------------
//   Cipher functions
// -------------------------------------------------

//buf: points head of this frame
//offset: indicates PHR + MHR offset
//SdReq: holds plain data to be encrypted
SK_BOOL EncryptData(SK_UB* buf, SK_UB offset, SK_UW frame_cnt, SS_DATA_REQUEST* SdReq){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;
	
	if( frame_cnt >= SS_MAX_FRAME_COUNTER ){
		return FALSE;	
	}

	//copy plain data fisrt
	memcpy(buf + offset, SdReq->m_Msdu, SdReq->m_MsduLength);
	
	EncReq.m_PsduLength = offset + SdReq->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = buf;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		GetNonce(&nonce, &SdReq->m_DstAddress, frame_cnt);
		
		key = GetEncKeyOf(&SdReq->m_DstAddress);
		if( key == NULL ){
			return FALSE;	
		}
	} else {
		GetNonce(&nonce, SSMac_GetAddress(), frame_cnt);
		
		key = gaSSMac_AESKey;
	}

	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
	result = Encrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		return FALSE;
	}
	
	//here, work->m_MsduLendth includes MIC len
	memcpy(buf + offset, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


//buf: points head of this frame
//offset: indicates PHR + MHR offset
//SdReq: holds plain data to be encrypted
SK_BOOL EncryptJoin(SK_UB* buf, SK_UB offset, SS_DATA_REQUEST* SdReq){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;

	//copy plain data fisrt
	memcpy(buf + offset, SdReq->m_Msdu, SdReq->m_MsduLength);
	
	EncReq.m_PsduLength = offset + SdReq->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = buf;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		return FALSE;	
	} else {
		GetNonceForJoin(&nonce, gnSSMac_MySlot);
	}

	//Join uses PSK for Enc/Decryption
	key = gaSSMac_PSK;
	
	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
	result = Encrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		return FALSE;
	}
	
	//here, work->m_MsduLendth includes MIC len
	memcpy(buf + offset, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


//buf: points head of this frame
//offset: indicates PHR + MHR offset
//SdReq: holds plain data to be encrypted
SK_BOOL EncryptJoinRes(SK_UB* buf, SK_UB offset, SS_DATA_REQUEST* SdReq){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;

	if( SdReq->m_MsduLength != SS_JOIN_RES_CMD_LEN ) return FALSE;
	
	//copy plain data fisrt
	memcpy(buf + offset, SdReq->m_Msdu, SdReq->m_MsduLength);

	EncReq.m_PsduLength = offset + SdReq->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = buf;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		GetNonceForJoinRes(&nonce, &SdReq->m_DstAddress);
	} else {
		return FALSE;
	}
	
	//Join res uses PSK for Enc/Decryption
	key = gaSSMac_PSK;

	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
	result = Encrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		return FALSE;
	}
	
	//here, work->m_MsduLendth includes MIC len
	memcpy(buf + offset, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


//buf: points head of decoded data
//offset: indicates PHR + MHR offset
//MdInd: holds encrypted data to be decoded
SK_BOOL DecryptData(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_UW frame_cnt, SK_MCPS_DATA_INDICATION* MdInd){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SS_SLOT_ADDR_DB_ITEM* item = NULL;
	SK_UB idx = 0;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;

	if( frame_cnt >= SS_MAX_FRAME_COUNTER ){
	  	SS_STATS(sec.counter_err);
		return FALSE;
	}
	
	EncReq.m_PsduLength = (SK_UB)MdInd->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = MdInd->m_Msdu;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		
		item = FindDBItemOf(&SdInd->m_SrcAddress);
		if( item == NULL ) {
		  	SS_STATS(sec.decode_fail);
			return FALSE;	
		}
		
		idx = GetInSlotIdxOf(&SdInd->m_SrcAddress);
		if( idx == 0xFF ){
		  	SS_STATS(sec.decode_fail);
			return FALSE;	
		} else if( idx >= SS_MAX_SHARED_DEVICES ){
		  	return FALSE;
		}
		
		//frame counter check
		if( frame_cnt < item->m_FrameCounter[idx] ){
		  	SS_STATS(sec.counter_err);
			return FALSE;
		}
		
		GetNonce(&nonce, &SdInd->m_SrcAddress, frame_cnt);
		
		key = GetEncKeyOf(&SdInd->m_SrcAddress);
		if( key == NULL ){
		  	SS_STATS(sec.decode_fail);
			return FALSE;	
		}
	} else {
		//frame counter check
		if( frame_cnt < gnSSMac_FrameCounterSTA ){
		  	SS_STATS(sec.counter_err);
			return FALSE;
		}

		GetNonce(&nonce, SSMac_GetAddress(), frame_cnt);
		
		key = gaSSMac_AESKey;
	}

	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
	  	SS_STATS(sec.decode_fail);
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
#ifdef DEBUG_SEC
	{
		SK_UH i;
		SK_print("Dec:\r\n");
		for( i = 0; i < EncReq.m_PsduLength; i++ ){
			SK_print_hex(EncReq.m_Psdu[i], 2);	
		}
		SK_print("\r\n");
		for( i = 0; i < 13; i++ ){
			SK_print_hex(nonce.m_Bytes[i], 2); 
		}
		SK_print("\r\n");
	}
#endif
	
	result = Decrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	//if( work.m_PsduLength < (offset + (SK_UB)SS_MIC_LEN) ) {
	//20170301 fix m_PsduLengthにMIC長は含まれないで戻る
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		SS_STATS(sec.decode_fail);
		return FALSE;
	}
	
	if( result == TRUE ){
		if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
			if( frame_cnt < SS_MAX_FRAME_COUNTER ){
				item->m_FrameCounter[idx] = frame_cnt + 1;
			}
		} else {
			if( frame_cnt < SS_MAX_FRAME_COUNTER ){
				gnSSMac_FrameCounterSTA = frame_cnt + 1;
			}
		}
	} else {
	  	SS_STATS(sec.decode_fail);
	}
	
	//here, work->m_MsduLendth excludes MIC len
	memcpy(SdInd->m_Msdu, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


//buf: points head of decoded data
//offset: indicates PHR + MHR offset
//MdInd: holds encrypted data to be decoded
SK_BOOL DecryptJoin(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_MCPS_DATA_INDICATION* MdInd){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;

	if( MdInd->m_MsduLength < (SS_JOIN_CMD_LEN + SS_MIC_LEN) ) {
	  	SS_STATS(sec.decode_fail);
		return FALSE;
	}
	
	EncReq.m_PsduLength = (SK_UB)MdInd->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = MdInd->m_Msdu;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		GetNonceForJoin(&nonce, MdInd->m_RecvSlot);
		key = gaSSMac_PSK;
	} else {
		return FALSE;
	}

	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
#ifdef DEBUG_SEC
	{
		SK_UH i;
		SK_print("Dec:\r\n");
		for( i = 0; i < EncReq.m_PsduLength; i++ ){
			SK_print_hex(EncReq.m_Psdu[i], 2);	
		}
		SK_print("\r\n");
		for( i = 0; i < 13; i++ ){
			SK_print_hex(nonce.m_Bytes[i], 2); 
		}
		SK_print("\r\n");
	}
#endif
	
	result = Decrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	//if( work.m_PsduLength < (offset + (SK_UB)SS_MIC_LEN) ) {
	//20170301 fix
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		SS_STATS(sec.decode_fail);
		return FALSE;
	}
	
	//here, work->m_MsduLendth excludes MIC len
	memcpy(SdInd->m_Msdu, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


SK_BOOL DecryptJoinRes(SS_DATA_INDICATION* SdInd, SK_UB offset, SK_MCPS_DATA_INDICATION* MdInd){
	SS_ENC_REQUEST EncReq;
	SS_ENC_REQUEST work;
	SK_UB* tmpbuf;
	SS_CCMSTAR_NONCE nonce;
	SK_UB* key;
	SK_BOOL result;

	if( MdInd->m_MsduLength < (SS_JOIN_RES_CMD_LEN + SS_MIC_LEN) ) {
		SS_STATS(sec.decode_fail);
		return FALSE;
	}
	
	EncReq.m_PsduLength = (SK_UB)MdInd->m_MsduLength;
	EncReq.m_HeaderLength = offset;
	EncReq.m_Psdu = MdInd->m_Msdu;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
	  	SS_STATS(sec.decode_fail);
		return FALSE;
	} else {
		GetNonceForJoinRes(&nonce, SSMac_GetAddress());
	}
	
	key = gaSSMac_PSK;

	if (SK_AllocDataMemory((SK_VP *)&tmpbuf ) != SK_E_OK) {
	  	SS_STATS(sec.decode_fail);
		return FALSE;
	}

	work.m_Psdu = tmpbuf;
	
#ifdef DEBUG_SEC
	SK_print("Decrypt...\r\n"); SK_print_hex(offset, 2); SK_print("\r\n");
#endif
	result = Decrypt( &EncReq, key, &nonce, &work );
	
	//abnormal result
	//if( work.m_PsduLength < (offset + (SK_UB)SS_MIC_LEN) ) {
	//20170301 fix
	if( work.m_PsduLength < offset ) {
		SK_FreeMemory(tmpbuf);
		SS_STATS(sec.decode_fail);
		return FALSE;
	}
	
	//here, work->m_MsduLendth excludes MIC len
	memcpy(SdInd->m_Msdu, work.m_Psdu + offset, (SK_UB)(work.m_PsduLength - offset));
	
	SK_FreeMemory(tmpbuf);
	
	return result;
}


static void GetNonce(SS_CCMSTAR_NONCE* nonce, SK_ADDRESS* addr, SK_UW frame_cnt){
	SK_UB offset = 0;
	
	memset(nonce->m_Bytes, 0, sizeof(nonce->m_Bytes));

	MAC_SET_LONG(nonce->m_Bytes + offset, addr->Body.Extended.m_Long[0]);
	offset+=4;
	
	MAC_SET_LONG(nonce->m_Bytes + offset, addr->Body.Extended.m_Long[1]);
	offset+=4;
	
	MAC_SET_LONG(nonce->m_Bytes + offset, frame_cnt);
	offset+= 4;

	//last byte
	nonce->m_Bytes[ SS_CCMSTAR_NONCE_LEN-1 ] = SS_CCMSTAR_SEC_LEVEL;
}


//NONCE is fixed 13 bytes
//encode LSB first
static void GetNonceForJoin(SS_CCMSTAR_NONCE* nonce, SK_UH slot){
	SK_UB offset = 0;
	
	memset(nonce->m_Bytes, 0, sizeof(nonce->m_Bytes));

	MAC_SET_LONG(nonce->m_Bytes + offset, gnSSMac_StationID);
	offset+= 4;

	//memcpy(nonce->m_Bytes + offset, gaSSMac_SlotHashKey, SS_SLOT_HASHKEY_LEN);
	//offset+= SS_SLOT_HASHKEY_LEN;
	
	MAC_SET_LONG(nonce->m_Bytes + offset, gnSSMac_RandS);
	offset+= 4;
	
	MAC_SET_WORD(nonce->m_Bytes + offset, slot);
	offset+= 2;
	
	//in slot idx = 0
	nonce->m_Bytes[offset] = 0;
	offset++;
	
	//pad
	nonce->m_Bytes[offset] = 0;
	offset++;
	
	//last byte
	nonce->m_Bytes[ SS_CCMSTAR_NONCE_LEN-1 ] = SS_CCMSTAR_SEC_LEVEL;
}


//NONCE is fixed 13 bytes
//encode LSB first
static void GetNonceForJoinRes(SS_CCMSTAR_NONCE* nonce, SK_ADDRESS* addr){
	SK_UB offset = 0;
	
	memset(nonce->m_Bytes, 0, sizeof(nonce->m_Bytes));

	//MAC_SET_LONG(nonce->m_Bytes + offset, gnSSMac_StationID);
	MAC_SET_LONG(nonce->m_Bytes + offset, gnSSMac_RandS);
	offset+= 4;

	MAC_SET_LONG(nonce->m_Bytes + offset, addr->Body.Extended.m_Long[0]);
	offset+=4;
	
	MAC_SET_LONG(nonce->m_Bytes + offset, addr->Body.Extended.m_Long[1]);
	offset+=4;

	//last byte
	nonce->m_Bytes[ SS_CCMSTAR_NONCE_LEN-1 ] = SS_CCMSTAR_SEC_LEVEL;
}


// -------------------------------------------------
//   AES-CCM* Encription
// -------------------------------------------------
#define BLOCK_SIZE 								16
#define HEADERFIELDSIZE 						0

static SK_UB An[BLOCK_SIZE];
static SK_UB En[BLOCK_SIZE];
static SK_UB Mn[BLOCK_SIZE];
static SK_UB S0[BLOCK_SIZE];
static SK_UW rk[4*(MAXNR + 1)];

SK_BOOL Encrypt(SS_ENC_REQUEST* in, SK_UB* key, SS_CCMSTAR_NONCE* nonce, SS_ENC_REQUEST* out){
	SS_ENC_REQUEST *Plain;
	SS_ENC_REQUEST *Cipher;
	SK_UW Nr;
	SK_UB MICLength;
	SK_UW i, j;
	SK_UH lm;
	SK_UB Flag = 0;
	SK_UB L = 2;

	SK_UB *pT;

	if((in == NULL) || (key == NULL) || (nonce == NULL) || (out == NULL)){
		return FALSE;
	}

	Plain = in;
	Cipher = out;

	pT = CreateAuthEnc(Plain, key, nonce);

	// Copy Header Data
	memcpy((&Cipher->m_Psdu[0]), &(Plain->m_Psdu[0]), Plain->m_HeaderLength);

	// Flags = Reserved || Reserved || 0 || L 
	Flag = L -1;

	lm = Plain->m_PsduLength - Plain->m_HeaderLength;

	// Ai = Flags || Nonce N || Counter
	An[0] = Flag;

	memcpy(&(An[1]), &(nonce->m_Bytes[0]), SS_CCMSTAR_NONCE_LEN);

	Nr = rijndaelKeySetupEnc(rk, key, 128);

	for(i = 0; i < lm; i += BLOCK_SIZE){
		An[14] = (SK_UB)(i >> 8);
		An[15] = (SK_UB)(((i / BLOCK_SIZE) + 1) & 0xFF);

		rijndaelEncrypt(rk, Nr, &(An[0]), (&En[0]));
		memcpy(&(Mn[0]), &(Plain->m_Psdu[Plain->m_HeaderLength + i]), BLOCK_SIZE);
		for(j = 0; j < BLOCK_SIZE; j++){
			Cipher->m_Psdu[Plain->m_HeaderLength + i + j] = En[j] ^ Mn[j];
		}
	}

	MICLength = SS_MIC_LEN;

#ifdef DEBUG_SEC
	SK_print("Enc: MIC\r\n");
#endif

	An[14] = 0;
	An[15] = 0;
	rijndaelEncrypt(rk, Nr, &(An[0]), &(S0[0]));
	for(j = 0; j < MICLength; j++){
		Cipher->m_Psdu[Plain->m_PsduLength + j] = pT[j] ^ S0[j];
		
#ifdef DEBUG_SEC
		SK_print_hex(Cipher->m_Psdu[Plain->m_PsduLength + j], 2);
#endif
	}
	
#ifdef DEBUG_SEC
	SK_print("\r\n");
#endif

	//Update m_PsduLength
	Cipher->m_PsduLength = Plain->m_PsduLength + MICLength;

	return TRUE;
}


// -------------------------------------------------
//   AES-CCM* Decryption
// -------------------------------------------------
static SK_UB T[BLOCK_SIZE];

static SK_BOOL Decrypt(SS_ENC_REQUEST *in, SK_UB *key, SS_CCMSTAR_NONCE *nonce, SS_ENC_REQUEST *out){
	SS_ENC_REQUEST *Cipher;
	SS_ENC_REQUEST *Decipher;
	SK_UW Nr;
	SK_UB MICLength;
	SK_UW i, j;
	SK_UH lm;
	SK_UB Flag = 0;
	SK_UB L = 2;
	SK_UB *CheckT;

	//null check
	if((in == NULL) || (key == NULL) || (nonce == NULL) || (out == NULL)){
		#ifdef DEBUG_SEC
		SK_print("invalid param...NULL\r\n");
		#endif
		return FALSE;
	}

	Cipher = in;
	Decipher = out;

	// Copy Header Data
	memcpy((&Decipher->m_Psdu[0]), &(Cipher->m_Psdu[0]), Cipher->m_HeaderLength);

	// Flags = Reserved || Reserved || 0 || L 
	Flag = L - 1;

	lm = Cipher->m_PsduLength - Cipher->m_HeaderLength;
	
	// B0 = Flags || Nonce N || l(m)
	An[0] = Flag;
	memcpy(&(An[1]), &(nonce->m_Bytes[0]), SS_CCMSTAR_NONCE_LEN);

	Nr = rijndaelKeySetupEnc(rk, key, 128);

	for(i = 0; i < lm; i += BLOCK_SIZE){
		An[14] = (SK_UB)(i >> 8);
		An[15] = (SK_UB)(((i / BLOCK_SIZE) + 1) & 0xFF);

		rijndaelEncrypt(rk, Nr, &(An[0]), (&En[0]));
		memcpy(&(Mn[0]), &(Cipher->m_Psdu[Cipher->m_HeaderLength + i]), BLOCK_SIZE);
		for(j = 0; j < BLOCK_SIZE; j++){
			Decipher->m_Psdu[Cipher->m_HeaderLength + i + j] = En[j] ^ Mn[j];
		}
	}

	MICLength = SS_MIC_LEN;
	Decipher->m_PsduLength = Cipher->m_PsduLength - MICLength;

	An[14] = 0;
	An[15] = 0;
	rijndaelEncrypt(rk, Nr, &(An[0]), (&S0[0]));
	for(j = 0; j < MICLength; j++){
		T[j] = Cipher->m_Psdu[Cipher->m_PsduLength - MICLength + j] ^ S0[j];
	}

	CheckT = CreateAuthDec(Decipher, Cipher->m_HeaderLength, key, nonce);

#ifdef DEBUG_SEC
	{
		SK_UB i;
		SK_print("Dec: T\r\n");
		for( i = 0; i < MICLength; i++ ){
			SK_print_hex(T[i], 2); 	
		}
		SK_print("\r\n");

		SK_print("Dec: CheckT\r\n");
		for( i = 0; i < MICLength; i++ ){
			SK_print_hex(CheckT[i], 2); 	
		}
		SK_print("\r\n");
	}
#endif

	if(strncmp((const char *)&(T[0]), (const char *)CheckT, MICLength) != 0){
		return FALSE;
	}

	return TRUE;
}


// -------------------------------------------------
//   AES-CCM* MIC Calculation for Encryption
// -------------------------------------------------
static SK_UB AddAuthData[ SS_MAX_FRAME_LEN + 16 ];
static SK_UB AuthData[ SS_MAX_FRAME_LEN + 16 ];
static SK_UW rk2[4*(MAXNR + 1)];
static SK_UB B0[BLOCK_SIZE];
static SK_UB Bn[BLOCK_SIZE];
static SK_UB Xn1[BLOCK_SIZE];
static SK_UB Xn[BLOCK_SIZE];
	
static SK_UB *CreateAuthEnc(SS_ENC_REQUEST *in, SK_UB* key, SS_CCMSTAR_NONCE* nonce){
	SK_UW Nr;	
	SK_UB Flag = 0;
	SK_UB Adata = 0;
	SK_UB M = 0;
	SK_UB L = 0;
	SK_UH lm;
	SK_UW i, j;
	SK_UH La;

	SK_UH AddAuthDataPaddingSize;
	SK_UH AddAuthDataLength;
	SK_UH AuthDataLength;
	SK_UH AuthDataPaddingSize;

	memset(AddAuthData, 0x00, SS_MAX_FRAME_LEN + 16);
	memset(AuthData, 0x00, SS_MAX_FRAME_LEN + 16);

	La = in->m_HeaderLength;

	AddAuthData[0] = (SK_UB)(La >> 8) & 0xFF;
	AddAuthData[1] = (SK_UB)(La & 0xFF);

	memcpy(&(AddAuthData[HEADERFIELDSIZE]), &(in->m_Psdu[0]), in->m_HeaderLength);

	if((HEADERFIELDSIZE + in->m_HeaderLength) % BLOCK_SIZE){
		AddAuthDataPaddingSize = BLOCK_SIZE - ((HEADERFIELDSIZE + in->m_HeaderLength) % BLOCK_SIZE);
	}else{
		AddAuthDataPaddingSize = 0;
	}
	AddAuthDataLength = HEADERFIELDSIZE + in->m_HeaderLength + AddAuthDataPaddingSize;

	lm = in->m_PsduLength - in->m_HeaderLength;

	if(lm % BLOCK_SIZE){
		AuthDataPaddingSize = BLOCK_SIZE - (SK_UB)(lm % BLOCK_SIZE);
	}else{
		AuthDataPaddingSize = 0;
	}
	AuthDataLength = AddAuthDataLength + (SK_UB)lm + AuthDataPaddingSize;

	memcpy(&(AuthData[0]), &(AddAuthData[0]), AddAuthDataLength);
	memcpy(&(AuthData[AddAuthDataLength]), &(in->m_Psdu[in->m_HeaderLength]), lm);

	// Flags = Reserved || Adata || M || L
	// Check Adata
	if(in->m_HeaderLength > 0){
		Adata = (1 << 6);
	}else{
		Adata = 0;
	}
	// Get Auth Length
	M = SS_MIC_LEN;
	M = ((M - 2)/2) << 3;

	// Set L
	L = 15 - SS_CCMSTAR_NONCE_LEN - 1;

	Flag = Adata | M | L;

	// B0 = Flags || Nonce N || l(m)
	B0[0] = Flag;
	memcpy(&(B0[1]), &(nonce->m_Bytes[0]), SS_CCMSTAR_NONCE_LEN);
	B0[14] = (SK_UB)(lm >> 8);
	B0[15] = (SK_UB)(lm & 0xFF);

	memset(&(Xn[0]), 0x00, BLOCK_SIZE);
	memset(&(Xn1[0]), 0x00, BLOCK_SIZE);

	Nr = rijndaelKeySetupEnc(rk2, key, 128);

	for(i = 0; i < (AuthDataLength + BLOCK_SIZE); i += BLOCK_SIZE){
		if(i == 0){
			for(j = 0; j < BLOCK_SIZE; j++){
				Xn[j] = Xn[j] ^ B0[j];
			}
			rijndaelEncrypt(rk2, Nr, &(Xn[0]), &(Xn1[0]));
		}else{
			memcpy(Bn, &(AuthData[i - BLOCK_SIZE]), BLOCK_SIZE);
			for(j = 0; j < BLOCK_SIZE; j++){
				Xn[j] = Xn[j] ^ Bn[j];
			}
			rijndaelEncrypt(rk2, Nr, &(Xn[0]), &(Xn1[0]));
		}
		memcpy(&(Xn[0]), &(Xn1[0]), BLOCK_SIZE);
	}

	return &(Xn[0]);
}


// -------------------------------------------------
//   AES-CCM* MIC Calculation for Decryption
// -------------------------------------------------
static SK_UB* CreateAuthDec(SS_ENC_REQUEST *in,  SK_UB header_len, SK_UB *key, SS_CCMSTAR_NONCE *nonce){
	SK_UW Nr;
	SK_UB Flag = 0;
	SK_UB Adata = 0;
	SK_UB M = 0;
	SK_UB L = 0;
	SK_UH lm;
	SK_UW i, j;
	SK_UH La;
	SK_UH AddAuthDataPaddingSize;
	SK_UH AddAuthDataLength;
	SK_UH AuthDataLength;
	SK_UH AuthDataPaddingSize;

	memset(AddAuthData, 0x00, SS_MAX_FRAME_LEN + 16);
	memset(AuthData, 0x00, SS_MAX_FRAME_LEN + 16);

	La = header_len;

	AddAuthData[0] = (SK_UB)((La >> 8) & 0xFF);
	AddAuthData[1] = (SK_UB)(La & 0xFF);

	memcpy(&(AddAuthData[HEADERFIELDSIZE]), &(in->m_Psdu[0]), header_len);

	if((HEADERFIELDSIZE + header_len) % BLOCK_SIZE){
		AddAuthDataPaddingSize = BLOCK_SIZE - ((HEADERFIELDSIZE + header_len) % BLOCK_SIZE);
	}else{
		AddAuthDataPaddingSize = 0;
	}
	AddAuthDataLength = HEADERFIELDSIZE + header_len + AddAuthDataPaddingSize;

	lm = in->m_PsduLength - header_len;

	if(lm % BLOCK_SIZE){
		AuthDataPaddingSize = BLOCK_SIZE - (SK_UB)(lm % BLOCK_SIZE);
	}else{
		AuthDataPaddingSize = 0;
	}
	AuthDataLength = AddAuthDataLength + (SK_UB)lm + AuthDataPaddingSize;

	memcpy(&(AuthData[0]), &(AddAuthData[0]), AddAuthDataLength);
	memcpy(&(AuthData[AddAuthDataLength]), &(in->m_Psdu[header_len]), lm);

	// Flags = Reserved || Adata || M || L
	// Check Adata
	if(header_len > 0){
		Adata = (1 << 6);
	}else{
		Adata = 0;
	}
	// Get Auth Length
	M = SS_MIC_LEN;
	M = ((M - 2)/2) << 3;

	// Set L
	L = 15 - SS_CCMSTAR_NONCE_LEN - 1;

	Flag = Adata | M | L;

	// B0 = Flags || Nonce N || l(m)
	B0[0] = Flag;
	memcpy(&(B0[1]), &(nonce->m_Bytes[0]), SS_CCMSTAR_NONCE_LEN);
	B0[14] = (SK_UB)(lm >> 8);
	B0[15] = (SK_UB)(lm & 0xFF);

	memset(&(Xn[0]), 0x00, BLOCK_SIZE);
	memset(&(Xn1[0]), 0x00, BLOCK_SIZE);

	Nr = rijndaelKeySetupEnc(rk2, key, 128);

	for(i = 0; i < (AuthDataLength + BLOCK_SIZE); i += BLOCK_SIZE){
		if(i == 0){
			for(j = 0; j < BLOCK_SIZE; j++){
				Xn[j] = Xn[j] ^ B0[j];
			}
			rijndaelEncrypt(rk2, Nr, &(Xn[0]), &(Xn1[0]));
		}else{
			memcpy(Bn, &(AuthData[i - BLOCK_SIZE]), BLOCK_SIZE);
			for(j = 0; j < BLOCK_SIZE; j++){
				Xn[j] = Xn[j] ^ Bn[j];
			}
			rijndaelEncrypt(rk2, Nr, &(Xn[0]), &(Xn1[0]));
		}
		memcpy(&(Xn[0]), &(Xn1[0]), BLOCK_SIZE);
	}

	return &(Xn[0]);
}


// -------------------------------------------------
//   Post SLOT-CHANGED event to this layer
// -------------------------------------------------
static void PostSlotChanged(SK_UH slot){
	gSlotChanged = slot;  
}

static void PostSlotChangedExec(SK_UH slot){
	SS_SLOT_CHANGED_INDICATION *SlotInd;
	
	if (SK_AllocCommandMemory((SK_VP *)&SlotInd) != SK_E_OK) return;
	
	SlotInd->m_SlotNum = slot;

	if (SK_PostMessage(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_SLOT_CHANGED_INDICATION_CMD, (SK_VP)SlotInd) != SK_E_OK) {
		SK_FreeMemory(SlotInd);
	}
}


// -------------------------------------------------
//   Post SLOT-PRE-CHANGE event to this layer
// -------------------------------------------------
static void PostSlotPreChange(SK_UH slot){
  	gSlotPreChanged = slot;
}

static void PostSlotPreChangeExec(SK_UH slot){
	SS_SLOT_PRE_CHANGE_INDICATION *SlotInd;
	
	if (SK_AllocCommandMemory((SK_VP *)&SlotInd) != SK_E_OK) return;
	
	SlotInd->m_SlotNum = slot;

	if (SK_PostMessage(SK_LAYER_SS_MAC, SK_LAYER_SS_MAC, SS_SLOT_PRE_CHANGE_INDICATION_CMD, (SK_VP)SlotInd) != SK_E_OK) {
		SK_FreeMemory(SlotInd);
	}
}


// -------------------------------------------------
//   Post BEACON-NOTIFY.indication to upper layer
// -------------------------------------------------
static void PostBeaconNotifyInd(SS_BCN_FRAMECONTROL* fctrl, SS_SLOT_CONFIG* slot, SK_UB bsn, SK_UW sta_id, SK_UB rssi){
	SS_BEACON_NOTIFY_INDICATION *BcnInd;
	
	if (SK_AllocDataMemory((SK_VP *)&BcnInd) != SK_E_OK) return;
	
	BcnInd->m_Version = fctrl->Field.m_Version;
	BcnInd->m_BSN = bsn;
   	BcnInd->m_Capacity = fctrl->Field.m_Capacity;
    BcnInd->m_SlotMode = slot->Field.m_SlotMode;
    BcnInd->m_HoppingTable = slot->Field.m_HoppingTable;
	BcnInd->m_StationId = sta_id;
	
	BcnInd->m_Rssi = rssi;

	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
	  	//Stationはbeacon送信処理の前にイベント文字表示が発生しないようディレイを入れて上位に通知する
		if (SK_PostMessageL(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_BEACON_NOTIFY_INDICATION_CMD, (SK_VP)BcnInd, 50, 0) != SK_E_OK) {
			SK_FreeMemory(BcnInd);
		}	  
	} else {
		if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_BEACON_NOTIFY_INDICATION_CMD, (SK_VP)BcnInd) != SK_E_OK) {
			SK_FreeMemory(BcnInd);
		}
	}
}


// -------------------------------------------------
//   Post META-BEACON-NOTIFY.indication to upper layer
// -------------------------------------------------
static void PostMetaBeaconNotifyInd(SK_UW sta_id, SS_DEV_INFO info[], SK_UB size){
	SK_UB i;
	SS_META_BEACON_NOTIFY_INDICATION *MetaBcnInd;
	
	if( size > SS_MAX_NIC_NUM ) return;
	
	if (SK_AllocDataMemory((SK_VP *)&MetaBcnInd) != SK_E_OK) return;
	
	MetaBcnInd->m_StationId = sta_id;
	
	for( i = 0; i < size; i++ ){
		MetaBcnInd->m_DevInfo[i].m_CurrentSlot = info[i].m_CurrentSlot;
		MetaBcnInd->m_DevInfo[i].m_BaseChannel = info[i].m_BaseChannel;
		MetaBcnInd->m_DevInfo[i].m_SlotMode = info[i].m_SlotMode;
	}

	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_META_BEACON_NOTIFY_INDICATION_CMD, (SK_VP)MetaBcnInd) != SK_E_OK) {
		SK_FreeMemory(MetaBcnInd);
	}
}


// -------------------------------------------------
//   Post SS-DATA.confirm event to upper layer
// -------------------------------------------------
static void PostDataConf(SK_UB status, SS_DATA_REQUEST* SdReq){
	SS_DATA_CONFIRM *DataConf;
	
	if (SK_AllocCommandMemory((SK_VP *)&DataConf) != SK_E_OK) return;
	
	DataConf->m_Status = status;
	DataConf->m_Handle = SdReq->m_Handle;
	SSMac_CopyAddress(&(DataConf->m_DstAddress), &(SdReq->m_DstAddress));

	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_DATA_CONFIRM_CMD, (SK_VP)DataConf) != SK_E_OK) {
		SK_FreeMemory(DataConf);
	}
}


// -------------------------------------------------
//   Post SYNC.indication event to upper layer
// -------------------------------------------------
static void PostSyncInd(SK_UB status){
	SS_SYNC_INDICATION *SyncInd;
	
	if (SK_AllocCommandMemory((SK_VP *)&SyncInd) != SK_E_OK) return;
	
	SyncInd->m_Status = status;

	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_SYNC_INDICATION_CMD, (SK_VP)SyncInd) != SK_E_OK) {
		SK_FreeMemory(SyncInd);
	}
}


// -------------------------------------------------
//   Post COMM-STAT.indication event to upper layer
// -------------------------------------------------
static void PostCommStatInd(SK_UB status){
	SS_COMM_STAT_INDICATION *CommInd;
	
	if (SK_AllocCommandMemory((SK_VP *)&CommInd) != SK_E_OK) return;
	
	CommInd->m_Status = status;

	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_COMM_STAT_INDICATION_CMD, (SK_VP)CommInd) != SK_E_OK) {
		SK_FreeMemory(CommInd);
	}
}


// -------------------------------------------------
//   Post JOIN.confirm event to upper layer
// -------------------------------------------------
static void PostJoinConf(SK_UB status, SK_UH slot, SK_UB idx){
	SS_JOIN_CONFIRM *JoinConf;
	
	if (SK_AllocCommandMemory((SK_VP *)&JoinConf) != SK_E_OK) return;
	
	JoinConf->m_Status = status;
	JoinConf->m_SlotNum = slot;
	JoinConf->m_InSlotIdx = idx;
	
	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_JOIN_CONFIRM_CMD, (SK_VP)JoinConf) != SK_E_OK) {
		SK_FreeMemory(JoinConf);
	}
}


// -------------------------------------------------
//   Notify Ack receive to upper layer
// -------------------------------------------------
static void PostAckInd(SS_DOWN_FRAMECONTROL* down_fctrl, SK_UB pend_slot){
	SS_ACK_INDICATION *AckInd;

	if (SK_AllocCommandMemory((SK_VP *)&AckInd) != SK_E_OK) return;

	AckInd->m_FramePending = down_fctrl->Field.m_FramePending;

	if (SK_PostMessage(gnSSMac_UpperLayer, SK_LAYER_SS_MAC, SS_ACK_INDICATION_CMD, (SK_VP)AckInd) != SK_E_OK) {
		SK_FreeMemory(AckInd);
	}
}


// -------------------------------------------------
//   Direct register the device of the addr with slot and idx
// -------------------------------------------------
SK_BOOL SSMac_DirectJoin(SK_ADDRESS* addr, SK_UH slot, SK_UB idx){
	SK_BOOL ans;

	ans = RegisterDevWithSlotIdx(addr, slot, idx);
	
	return ans;
}


// -------------------------------------------------
//   Direct register the device of the addr with auto register
// -------------------------------------------------
SK_BOOL SSMac_AutoJoin(SK_ADDRESS* addr){
	SK_BOOL ans;
	SK_UB res_idx;
	SK_UH res_slot;
	SK_UH slot;

	slot = CalcSlotFor(gaSSMac_SlotHashKey, addr, gnSSMac_StationID);	
	ans = AutoRegisterDevSlot(addr, slot, &res_slot, &res_idx);

	return ans;
}


// -------------------------------------------------
// Register addr/slot touple to slot data base
// return FALSE if data base is already full
// out:res_slot conclusive registered slot
// out:res_idx conclusive offset in the slot
// -------------------------------------------------
SK_BOOL AutoRegisterDevSlot(SK_ADDRESS* addr, SK_UH slot, SK_UH* res_slot, SK_UB* res_idx){
	SK_UH i, j;	
	SK_BOOL ans;
	SK_UH size;
	SS_SLOT_ADDR_DB_ITEM* item;
	
	//DBにアドレス登録があるならその情報を応答する
	item = FindDBItemOf(addr);
	if( item != NULL ){
		SK_UB idx;
		idx = GetInSlotIdxOf(addr);
		if( idx == 0xFF ){
			//InSlotIdxが未設定->状態がおかしい
			return FALSE;
		} else if( idx >= SS_MAX_SHARED_DEVICES ){
		  	return FALSE;
		} else {
			*res_slot = item->m_SlotNum;
			*res_idx = idx;
			
			//join済みデバイスの再joinはkeyを再生成
			GenerateEncKey( &(item->m_Key[idx][0]), SS_AES_KEY_LEN );
			
			//clear frame counter
			item->m_FrameCounter[idx] = 0;
			item->m_OutgoingFrameCounter[idx] = 0;
			//
			
			return TRUE;
		}
	} else {
		//Full manage modeの場合、mac addrで登録がなければ接続拒否する
		if( gnSSMac_IsFullManage == TRUE ){
			return FALSE;	
		}
	}
	
	//スロットがDBに登録されているか
	item = FindDBItemFor(slot);
	if( item == NULL ){
		//空きアイテムを探す
		item = GetFreeDBItem();
		if( item == NULL ) {
			//DB is full, can not register joined address
			return FALSE;
		}
	}

	//この時点でitemはNULLでない
	
	//アドレスを登録してidxを割り当てる
	ans = PutAddrToItem(addr, item, res_idx);
	if( ans == TRUE ){
		//Success registration
		item->m_SlotNum = slot;
		*res_slot = slot;
		return TRUE;
	} 

	//
	//スロットがすべて埋まっているので代替スロットを割り当てる
	//
	StartSlotBitmap();
	
	//利用中スロットをすべて列挙してBitmap Set
	for( i = 0; i < SS_DB_SIZE; i++ ){
		if( gaSSMac_SlotAddr_DB[i].m_SlotNum != 0xFFFF ){
			SetSlotBitmap(gaSSMac_SlotAddr_DB[i].m_SlotNum);
		}
	}
	
	size = SLOT_BITMAP_SIZE;
	
	for( i = 0; i < size; i++ ){
		//空きがないなら次の32bit
		if( gaSSMac_SlotBitmap[i] == 0xFFFFFFFFU ) continue;
		
		//0の場所を探す
		for( j = 0; j < 32; j++ ){
			if( ((gaSSMac_SlotBitmap[i] >> j) & 1L) == 0 ){
				SK_UH target;
				
				//bitmapからスロット番号へ変換
				target = i * 32 + j;
				
				//この時点で最大番号を超えていたらスロットは一杯
				if( target > GetUpSlotNum(gnSSMac_SlotMode) ){
					goto last;
				} else {
					ans = RegisterDevWithSlotIdx(addr, target, 0);
					if( ans == FALSE ) return FALSE;
					*res_slot = target;
					*res_idx = 0;
					return TRUE;
				}
			}
		}
	}
	
	
	last:
	if( SS_MAX_SHARED_DEVICES == 1 ) return FALSE;
	
	//
	//スロットがすべて埋まっているのでインデックス=1が空いているスロットを探す
	//
	StartSlotBitmap();
	
	//idx=0, 1が埋まっているスロットをすべて列挙してBitmap Set
	for( i = 0; i < SS_DB_SIZE; i++ ){
		SK_BOOL is_full = TRUE;
		for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
			if( gaSSMac_SlotAddr_DB[i].m_Addr[j].Body.Extended.m_Long[0] == 0 &&
				gaSSMac_SlotAddr_DB[i].m_Addr[j].Body.Extended.m_Long[1] == 0 ){
				is_full = FALSE;
				break;
			}
		}
		if( is_full == TRUE ){
			SetSlotBitmap(gaSSMac_SlotAddr_DB[i].m_SlotNum);
		}
	}
	
	size = SLOT_BITMAP_SIZE;
	
	for( i = 0; i < size; i++ ){
		//空きがないなら次の32bit
		if( gaSSMac_SlotBitmap[i] == 0xFFFFFFFF ) continue;
		
		//0の場所を探す
		for( j = 0; j < 32; j++ ){
			if( ((gaSSMac_SlotBitmap[i] >> j) & 1L) == 0 ){
				SK_UH target;
				
				//bitmapからスロット番号へ変換
				target = i * 32 + j;
				
				//この時点で最大番号を超えていたらスロットは一杯
				if( target > GetUpSlotNum(gnSSMac_SlotMode) ){
					return FALSE;
				} else {
					//idx=1しか空いていないはず
					ans = RegisterDevWithSlotIdx(addr, target, 1);
					if( ans == FALSE ) return FALSE;
					*res_slot = target;
					*res_idx = 1; //idx=1をアサインする
					return TRUE;
				}
			}
		}
	}
	
	//すべてが埋まっている
	return FALSE;
}


// -------------------------------------------------
// アドレスをslot, idx指定で登録
// slot, idxが利用済みの場合はFALSE
// -------------------------------------------------
SK_BOOL RegisterDevWithSlotIdx(SK_ADDRESS* addr, SK_UH slot, SK_UB idx){
	SK_BOOL ans = TRUE;
	SS_SLOT_ADDR_DB_ITEM* item;
	
	item = FindDBItemFor(slot);
	if( item == NULL ){
		item = GetFreeDBItem();
		if( item == NULL ) {
			//DB is full, can not register joined address
			ans = FALSE;
		} 
	}
	
	if( item != NULL ){
		SK_BOOL res;
		res = PutAddrToItemWithIdx(addr, item, idx);
		if( res == TRUE ){
			//Success registration
			item->m_SlotNum = slot;
			ans = TRUE;	
		} else {
			ans = FALSE;
		}
	}
	
	return ans;
}


// -------------------------------------------------
//   Time slot calculation
// *) This function is called from timer interrupt
// -------------------------------------------------
void SK_IncrementSlot(SK_UH tick){
	SK_UB i;
	
	if( gnSSMac_DeviceType == SS_TYPE_METABEACON ){
		for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
			if( gaSSMac_DevInfo[i].m_TickSlotCount == 0xFFFF ) continue;
			
			gaSSMac_DevInfo[i].m_TickSlotCount++;
			if( gaSSMac_DevInfo[i].m_TickSlotCount >= SS_TICKS_PER_UNIT ){
				gaSSMac_DevInfo[i].m_CurrentSlotUnit++;
				if( gaSSMac_DevInfo[i].m_CurrentSlotUnit >= SS_UNITS_PER_SLOT ){
					gaSSMac_DevInfo[i].m_CurrentSlot++;
					
					if( gaSSMac_DevInfo[i].m_CurrentSlot >= GetAllSlotNum(gaSSMac_DevInfo[i].m_SlotMode) ){
						gaSSMac_DevInfo[i].m_CurrentSlot = 0; 
					}
					gaSSMac_DevInfo[i].m_CurrentSlotUnit = 0;
				}
				gaSSMac_DevInfo[i].m_TickSlotCount = 0;
			}
		}
	} else {
		gnTickSlotCount++;
		
		if( gnTickSlotCount >= SS_TICKS_PER_UNIT ){
			gnSSMac_CurrentSlotUnit++;
			if( gnSSMac_CurrentSlotUnit >= SS_UNITS_PER_SLOT ){
				gnSSMac_CurrentSlot++;
				
				if( gnSSMac_CurrentSlot >= GetAllSlotNum(gnSSMac_SlotMode) ){
					gnSSMac_CurrentSlot = 0; 
				}

				#ifdef DEBUG_GPIO_SLOT				
				DebugPort_Set( 0, gnSSMac_CurrentSlot % 2 );
				DebugPort_Set( 1, (gnSSMac_CurrentSlot==1) );
				#endif
				
				PostSlotChanged(gnSSMac_CurrentSlot);
				gnSSMac_CurrentSlotUnit = 0;
			} else if( (gnSSMac_CurrentSlotUnit + 1) == SS_UNITS_PER_SLOT ){
			  
				PostSlotPreChange(gnSSMac_CurrentSlot);

			} else {
			  	//do nothing
			}
			gnTickSlotCount = 0;
		} else if( ((gnTickSlotCount + 2) >= SS_TICKS_PER_UNIT) ){
			//PostSlotPreChange(gnSSMac_CurrentSlot);
		}
	}

	#ifdef DEBUG_SLOT_TICK
	SK_print_hex(gnTickSlotCount, 8); SK_print(" ");
	SK_print_hex(gnSSMac_CurrentSlotUnit, 8); SK_print(" "); 
	SK_print_hex(gnSSMac_CurrentSlot, 8); SK_print("\r\n");
	#endif
}


void SSMac_IncCurrentSlot(SK_BOOL event){
	gnSSMac_CurrentSlot++;	
	
	if( event == TRUE ){
		PostSlotChanged(gnSSMac_CurrentSlot);
	}
}


void SSMac_SetSlotUnitTick(SK_UH unit, SK_UH tick){
  	gnTickSlotCount = tick;
	gnSSMac_CurrentSlotUnit = unit;
}


//
//ビーコン受信によるタイムスロット補正処理
//
void SSMac_AdjustSlot(SK_UB bscn, SS_SLOT_CONFIG* slot_cfg, SK_UW timestamp){
	SK_UW diff; //us

	#ifdef DEBUG_GPIO_TRX
	DebugPort_Set( 0, TRUE );
	#endif

	//タイマ割込み禁止
	TIMER_LOCK();

	gnTickSlotCount = 0;
	gnSSMac_CurrentSlotUnit = 0;
	gnSSMac_CurrentSlot = 0;
	
	//ビーコン送信時間 + 経過時間をusに換算
	diff = (SS_BEACON_TRANS_TIME_ABS);
	
	//時間誤差が1スロット以上だったら現在スロット番号を補正
	if( diff > ((SK_UW)SS_SLOT_DURATION * 1000) ){
		//gnTickSlotCount = (SK_UH)(diff / SS_UNIT_DURATION);
		//SK_UW slot_d = ((SK_UW)SS_SLOT_DURATION * 1000L);
		gnSSMac_CurrentSlot = (SK_UH)(diff / ((SK_UW)SS_SLOT_DURATION * 1000L));
		diff = diff - (gnSSMac_CurrentSlot * (SK_UW)SS_SLOT_DURATION * 1000L);
	}
	
 	if( diff > ((SK_UW)SS_UNIT_DURATION * 1000) ){
		//gnTickSlotCount = (SK_UH)(diff / SS_UNIT_DURATION);
		gnSSMac_CurrentSlotUnit = (SK_UH)(diff / ((SK_UW)SS_UNIT_DURATION * 1000L));
		diff = diff - (gnSSMac_CurrentSlotUnit * (SK_UW)SS_UNIT_DURATION * 1000L);
	}

	//スロットtickを補正
	gnTickSlotCount = (SK_UH)(diff / ((SK_UW)SS_SYSTICK * 1000L));
	diff = diff - (gnTickSlotCount * ((SK_UW)SS_SYSTICK * 1000L));

	//微修正
	#if 1
	{
	  	SK_UB cnt;
		SK_UW wcnt;
		
		RF_LOCK();
		for( cnt = 0; cnt < gnSSMac_SlotCalib; cnt++ ){
			SK_IncrementSlot(0);
	  	}
		for( wcnt = 0; wcnt < gnSSMac_FineCalib; wcnt++ );
		RF_UNLOCK();
	}
	#endif
	
	//tickタイマリスタート
	Timer_Initialize();

	//タイマ割込み禁止解除
	TIMER_UNLOCK();

	#ifdef DEBUG_SLOT_PRINT
	SK_print("A:");
	SK_print_hex(SS_BEACON_TRANS_TIME, 8); SK_print(" ");
	SK_print_hex(log[0], 8); SK_print(" ");
	SK_print_hex(log[1], 8); SK_print(" ");
	SK_print_hex(log[2], 8); SK_print(" ");
	SK_print_hex(log[3], 8); SK_print(" ");
	SK_print_hex(log[4], 8); SK_print(" ");
	SK_print("->");
	SK_print_hex(gnTickSlotCount, 4); SK_print(" ");
	SK_print_hex(gnSSMac_CurrentSlotUnit, 4); SK_print(" ");
	SK_print_hex(gnSSMac_CurrentSlot, 4); SK_print("\r\n");
	#endif
}


// -------------------------------------------------
//   Set/Get current slot
// -------------------------------------------------
SK_UH SSMac_GetCurrentSlot(void){
	return gnSSMac_CurrentSlot;	
}


void SSMac_SetCurrentSlot(SK_UH slot){
	gnSSMac_CurrentSlot = slot;
}


// -------------------------------------------------
//   Set/Get device type
//	SS_TYPE_DEVICE
//	SS_TYPE_STAION
//  SS_TYPE_METABEACON
//  SS_TYPE_IDLING
// -------------------------------------------------
SK_BOOL SSMac_SetDeviceType(SK_UB type){
	if( type > SS_TYPE_IDLING ) return FALSE;
	gnSSMac_DeviceType = type;	
	return TRUE;
}


SK_UB SSMac_GetDeviceType(void){
	return gnSSMac_DeviceType;	
}


// -------------------------------------------------
//   Set/Get slot mode
// -------------------------------------------------
SK_BOOL SSMac_SetSlotMode(SK_UB mode){
	if( mode > SS_SLOTMODE_1024 ) return FALSE;
	gnSSMac_SlotMode = mode;
	
	//スロットモード変更に連動して保留データ期限切れ時間を設定
	//スロット周期 * 3回分
	gnSSMac_PendingExpireTime = (SK_UW)GetAllSlotNum(gnSSMac_SlotMode) * SS_SLOT_DURATION * (SK_UW)SS_SYNC_LOSS_THRESHOLD;
	
	return TRUE;
}


SK_UB SSMac_GetSlotMode(void){
	return gnSSMac_SlotMode;
}


// -------------------------------------------------
//   Set/Get Rx on whe idle
// -------------------------------------------------
SK_BOOL SSMac_SetRxOnWhenIdle(SK_BOOL flag){
	gnSSMac_RxOnWhenIdle = flag;	
	return TRUE;
}


SK_BOOL SSMac_GetRxOnWhenIdle(void){
	return gnSSMac_RxOnWhenIdle;	
}


// -------------------------------------------------
//   Set/Get Station Id
// -------------------------------------------------
SK_BOOL SSMac_SetStationId(SK_UW id){
	gnSSMac_StationID = id;	
	return TRUE;
}


SK_UW SSMac_GetStationId(void){
	return gnSSMac_StationID;	
}


// -------------------------------------------------
//   Set/Get pending data expire time
// -------------------------------------------------
SK_BOOL SSMac_SetPendExpTime(SK_UW time){
	gnSSMac_PendingExpireTime = time;	
	return TRUE;
}


SK_UW SSMac_GetPendExpTime(void){
	return gnSSMac_PendingExpireTime;	
}


// -------------------------------------------------
//   Get slot hash key
// -------------------------------------------------
SK_UB* SSMac_GetSlotHashKey(void){
	return gaSSMac_SlotHashKey;	
}


// -------------------------------------------------
//   Get/Set AES key
// For device only
// -------------------------------------------------
SK_UB* SSMac_GetAESKey(void){
	return gaSSMac_AESKey;	
}


SK_BOOL SSMac_SetAESKey(SK_UB* key, SK_UB key_len){
	if( key_len != SS_AES_KEY_LEN ) return FALSE;
	memcpy(gaSSMac_AESKey, key, SS_AES_KEY_LEN);
	return TRUE;
}


// -------------------------------------------------
//   Get/Set PSK 
// -------------------------------------------------
SK_UB* SSMac_GetPSK(void){
	return gaSSMac_PSK;	
}


SK_BOOL SSMac_SetPSK(SK_UB* key, SK_UB key_len){
	if( key_len != SS_AES_KEY_LEN ) return FALSE;
	memcpy(gaSSMac_PSK, key, SS_AES_KEY_LEN);
	return TRUE;
}


// -------------------------------------------------
//   Get 64bit addr of this device
// -------------------------------------------------
SK_ADDRESS* SSMac_GetAddress(void){
	return &gnSSMac_Address;	
}


// -------------------------------------------------
//   Get/Set my slot number
// 0xFFFF indicates this device not sync
// -------------------------------------------------
SK_UH SSMac_GetSlotNum(void){
	return gnSSMac_MySlot;	
}


SK_BOOL SSMac_SetSlotNum(SK_UH slot){
	gnSSMac_MySlot = slot;
	return TRUE;
}


// -------------------------------------------------
//   Get in slot index
//  0xFF indicates this device not sync
// -------------------------------------------------
SK_UB SSMac_GetInSlotIdx(void){
	return gnSSMac_InSlotIdx;	
}


SK_BOOL SSMac_SetInSlotIdx(SK_UB idx){
	gnSSMac_InSlotIdx = idx;
	return TRUE;
}


// -------------------------------------------------
//   Return a pointer to version str
// -------------------------------------------------
const char* SSMac_GetVerStr(void){
	return gsSSMac_VerStr;	
}


// -------------------------------------------------
//   Set/Get hopping table this device  use
// -------------------------------------------------
SK_BOOL SSMac_SetHoppingTable(SK_UB table){
	if( table > SS_HOPPING_TABLE4 ) return FALSE;
	gnSSMac_CurrentHoppingTable = table;
	return TRUE;
}


SK_UB SSMac_GetHoppingTable(void){
	return gnSSMac_CurrentHoppingTable;
}


SK_UB SSMac_SetHoppingEnable(SK_BOOL flag){
	gnSSMac_HoppingEnable = flag;
	return TRUE;
}


SK_BOOL SSMac_GetHoppingEnable(void){
	return gnSSMac_HoppingEnable;
}


// -------------------------------------------------
//   Frame counter access
// -------------------------------------------------
SK_UW SSMac_GetFrameCounter(void){
	return gnSSMac_FrameCounter;
}


SK_UW SSMac_GetFrameCounterSTA(void){
	return gnSSMac_FrameCounterSTA;
}


// -------------------------------------------------
//   Set/Get base channel
// -------------------------------------------------
SK_BOOL SSMac_SetBaseChannel(SK_UB ch){
	SK_BOOL ans;
	
	ans = SK_PHY_ChangeChannel(ch);
	if( ans == FALSE ) return FALSE;
	
	gnSSMac_BaseChannel = ch;
	
	ResetChannelTable(ch);
	
	return TRUE;
}


SK_UB SSMac_GetBaseChannel(void){
	return gnSSMac_BaseChannel;
}


// -------------------------------------------------
//   Set/Get Full manage mode
// -------------------------------------------------
SK_BOOL SSMac_SetFullManage(SK_BOOL flag){
  	gnSSMac_IsFullManage = flag;
	return TRUE;
}


SK_BOOL SSMac_IsFullManage(void){
  	return gnSSMac_IsFullManage;
}


// -------------------------------------------------
//   Fine/Coarse calibration for time slot sync
// -------------------------------------------------
SK_BOOL SSMac_SetSlotCalib(SK_UB val){
  	gnSSMac_SlotCalib = val;
	return TRUE;
}


SK_UB SSMac_GetSlotCalib(void){
  	return gnSSMac_SlotCalib;
}


SK_BOOL SSMac_SetFineCalib(SK_UW val){
 	gnSSMac_FineCalib = val;
	return TRUE;
}


SK_UW SSMac_GetFineCalib(void){
  	return gnSSMac_FineCalib;
}


// -------------------------------------------------
//   Return true if the slot is just on the current slot
// -------------------------------------------------
SK_BOOL SSMac_IsJustOn(SK_UH slot){
	// 同一スロットで複数データを送信しないためのガード
	if( gnSSMac_CurrentSlot == slot && gnSSMac_CurrentSlotUnit == 0 && gnTickSlotCount == 0 ){
		return TRUE;
	} else {
		return FALSE;
	}
}


// -------------------------------------------------
//   Encode address to byte array
// -------------------------------------------------
SK_UB SSMac_SetAddress(SK_UB *ptr, SK_ADDRESS* adr) {
	switch(adr->m_AddrMode) {
	case SK_ADDRMODE_SHORT:
		MAC_SET_WORD(&(ptr[0]),adr->Body.Short.m_Word);
		return 2;
	case SK_ADDRMODE_EXTENDED:
		MAC_SET_LONG(&(ptr[0]),adr->Body.Extended.m_Long[1]);
		MAC_SET_LONG(&(ptr[4]),adr->Body.Extended.m_Long[0]);
		return 8;
	}
	return 0;
}


// -------------------------------------------------
//   Decode EXT address from byte array
// -------------------------------------------------
void SSMac_GetExtAddress(SK_ADDRESS* dstaddr, SK_UB *ptr) {
	dstaddr->Body.m_Raw[0] = ptr[4];
	dstaddr->Body.m_Raw[1] = ptr[5];
	dstaddr->Body.m_Raw[2] = ptr[6];
	dstaddr->Body.m_Raw[3] = ptr[7];
			
	dstaddr->Body.m_Raw[4] = ptr[0];
	dstaddr->Body.m_Raw[5] = ptr[1];
	dstaddr->Body.m_Raw[6] = ptr[2];
	dstaddr->Body.m_Raw[7] = ptr[3];
	
	dstaddr->m_AddrMode = SK_ADDRMODE_EXTENDED;
}


// -------------------------------------------------
//   Copy SK_ADDRESS
// -------------------------------------------------
void SSMac_CopyAddress(SK_ADDRESS* dst, SK_ADDRESS* src) {
	SK_UB i;
	
	dst->m_AddrMode = src->m_AddrMode;
	for( i = 0; i < 8; i++ ){
		dst->Body.m_Raw[i] = src->Body.m_Raw[i];
	}
}


// -------------------------------------------------
//   Address matching
// -------------------------------------------------
SK_BOOL SSMac_Equals(SK_ADDRESS* dst, SK_ADDRESS* src) {
	if( dst->m_AddrMode != src->m_AddrMode ) return FALSE;
	
	if( dst->m_AddrMode == SK_ADDRMODE_EXTENDED ){
		if( dst->Body.Extended.m_Long[0] == src->Body.Extended.m_Long[0] &&
			dst->Body.Extended.m_Long[1] == src->Body.Extended.m_Long[1] ){
			return TRUE;
		} else {
			return FALSE;	
		}
	} else if( dst->m_AddrMode == SK_ADDRMODE_SHORT ){
		if( dst->Body.Short.m_Word == src->Body.Short.m_Word ){
			return TRUE;
		} else {
			return FALSE;	
		}		
	} else {
		return FALSE;	
	}
}


// -------------------------------------------------
//   Calc my slot num from slot hash key
// -------------------------------------------------
SK_UH GetAllSlotNum(SK_UB mode){
	switch(mode){
		case SS_SLOTMODE_16: //16 slots
			return 16;
		case SS_SLOTMODE_32: //32 slots
			return 32;
		case SS_SLOTMODE_64: //64 slots
			return 64;
		case SS_SLOTMODE_128: //128 slots
			return 128;
		case SS_SLOTMODE_256: //256 slots
			return 256;
		case SS_SLOTMODE_512: //512 slots
			return 512;
		case SS_SLOTMODE_1024: //1024 slots
			return 1024;
		default:
			return 32;
	}
}

#if 0
static SK_UH GetUpSlotNum(SK_UB mode){
	switch(mode){
		case SS_SLOTMODE_16: //16 slots
			return 12;
		case SS_SLOTMODE_32: //32 slots
			return 25;
		case SS_SLOTMODE_64: //64 slots
			return 51;
		case SS_SLOTMODE_128: //128 slots
			return 102;
		case SS_SLOTMODE_256: //256 slots
			return 204;
		case SS_SLOTMODE_512: //512 slots
			return 409;
		case SS_SLOTMODE_1024: //1024 slots
			return 819;
		default:
			return 25;
	}
}
#endif

//最終UPスロットをGAPスロットとする仕様変更につき、UP slot数を1減らす
static SK_UH GetUpSlotNum(SK_UB mode){
	switch(mode){
		case SS_SLOTMODE_16: //16 slots
			return 11;
		case SS_SLOTMODE_32: //32 slots
			return 24;
		case SS_SLOTMODE_64: //64 slots
			return 50;
		case SS_SLOTMODE_128: //128 slots
			return 101;
		case SS_SLOTMODE_256: //256 slots
			return 203;
		case SS_SLOTMODE_512: //512 slots
			return 408;
		case SS_SLOTMODE_1024: //1024 slots
			return 818;
		default:
			return 24;
	}
}


static SK_UB GetDownSlotNum(SK_UB mode){
  	SK_UB ans = 0;
  
	switch(mode){
		case SS_SLOTMODE_16: //16 slots
			ans = 3;
			break;
		case SS_SLOTMODE_32: //32 slots
			ans = 6;
			break;
		case SS_SLOTMODE_64: //64 slots
			ans = 12;
			break;
		case SS_SLOTMODE_128: //128 slots
			ans = 25;
			break;
		case SS_SLOTMODE_256: //256 slots
			ans = 51;
			break;
		case SS_SLOTMODE_512: //512 slots
			ans = 102;
			break;
		case SS_SLOTMODE_1024: //1024 slots
			ans = 204;
			break;
		default:
			ans = 6;
			break;
	}
	
	if( ans > SS_MAX_DOWN_SLOT_NUM ) {
	  	return SS_MAX_DOWN_SLOT_NUM;
	} else {
	  	return ans;
	}
}


static SK_UH CalcMySlot(SK_UB* key){
	return CalcSlotFor(key, &gnSSMac_Address, gnSSMac_StationID);
}


//SHA2 
SK_UH CalcSlotFor(SK_UB* key, SK_ADDRESS* addr, SK_UW sta_id){
	SK_UB output[SHA256_DIGEST_SIZE]; //32 bytes
	SK_UB seed[16];
	SK_UW slot_seed = 0;
	SK_UH i;
	SK_UH slot;
	
	memcpy(seed, key, 4); //Hash key sizeは4(以上)を想定
	MAC_SET_LONG_B(seed + 4, 	addr->Body.Extended.m_Long[0]);
	MAC_SET_LONG_B(seed + 8, 	addr->Body.Extended.m_Long[1]);	 
	MAC_SET_LONG_B(seed + 12, sta_id);
	
	sha256((const unsigned char*)seed, sizeof(seed), output);

	/*
	for( i = 0; i < (SHA256_DIGEST_SIZE); i+=4 ){
		slot_seed += MAC_GET_LONG_B( output + i );
	}
	*/
 	for( i = 0; i < (SHA256_DIGEST_SIZE); i++ ){
		slot_seed += output[i];
	}

	slot = (SK_UH)(slot_seed % GetUpSlotNum(gnSSMac_SlotMode) + 1);
	
	return slot;
}


static SK_UB FindFreeDownSlot(void){
	SK_UB i;
	SK_UB max;
	
	max = GetDownSlotNum(gnSSMac_SlotMode);
	
	//20170303 fix
	//if( max > SS_MAX_DOWN_SLOT_NUM ) return 0xFF;

	for( i = 0; i < max; i++ ){
		if( gaSSMac_DownSlot[i] == 0 ){
			gaSSMac_DownSlot[i] = 1;
			return i;
		}
	}
	
	return 0xFF;
}


void ClearAllDownSlot(void){
  	SK_UB i;
	
	for( i = 0; i < SS_MAX_DOWN_SLOT_NUM; i++ ){
		gaSSMac_DownSlot[i] = 0;
	} 
}


void ClearDownSlot(SK_UB slot){
	if( slot >= SS_MAX_DOWN_SLOT_NUM ) return;
	gaSSMac_DownSlot[slot] = 0;	
}


static void StopSync(void){
	gnSSMac_MySlot = 0xFFFF;
	gnSSMac_InSlotIdx = 0xFF;
	gnLastBeaconRecvTime = 0;
	
	//20170211 hoppingでchが遷移している可能性あり
	//sync loss時はbase channelへ戻す
	SK_PHY_ChangeChannel(gnSSMac_BaseChannel);
}


// -------------------------------------------------
// Increment seq number and outgoing frame counter
// -------------------------------------------------
static void IncSeqNumAndFrameCnt(SS_DATA_REQUEST* SdReq){
	SS_SLOT_ADDR_DB_ITEM* item = NULL;
	SK_UB idx;
	
	gnSSMac_SeqNo++;
	
	if( SdReq->m_Selector == SS_SELECTOR_JOIN_RES_CMD || SdReq->m_Selector == SS_SELECTOR_JOIN_CMD ){
		return;	
	}
	
	//frame cnt = 24bit
	if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
		item = FindDBItemOf(&SdReq->m_DstAddress);
		if( item == NULL ) {
			return;
		}
		
		idx = GetInSlotIdxOf(&SdReq->m_DstAddress);
		if( idx == 0xFF ){
			return;	
		} else if( idx >= SS_MAX_SHARED_DEVICES ){
		  	return;
		}
		
		if( item->m_OutgoingFrameCounter[idx] >= SS_MAX_FRAME_COUNTER ){
			//don't rotate frame counter
			//keep max value			
		} else {
			item->m_OutgoingFrameCounter[idx]++;
		}
	} else {
		if( gnSSMac_FrameCounter >= SS_MAX_FRAME_COUNTER ){
			//don't rotate frame counter
			//keep max value
		} else {
			gnSSMac_FrameCounter++;
		}
	}
}


// -------------------------------------------------
//   Init device info database for meta beacon
// -------------------------------------------------
void SSMac_InitDevInfo(SS_DEV_INFO info[], SK_UH size){
	SK_UH i;
	
	if( size > SS_MAX_NIC_NUM ) return;

	for( i = 0; i < size; i++ ){
		info[i].m_TickSlotCount = 0xFFFF;
		info[i].m_CurrentSlotUnit = 0;
		
		info[i].m_CurrentSlot = 0;
		info[i].m_BaseChannel = 0;
		info[i].m_SlotMode = 7; //slotmode is must be in 0-6, so 7 indicates this entry is not using
	}
}


// -------------------------------------------------
//   Set device info to dev database
// -------------------------------------------------
SK_BOOL SSMac_SetDevInfo(SK_UB index, SK_UH slot, SK_UB channel, SK_UB mode){
	if( index >= SS_MAX_NIC_NUM ) return FALSE;
	if( mode > SS_SLOTMODE_1024 ) return FALSE;
	if( slot > GetAllSlotNum(mode) ) return FALSE;
	
	gaSSMac_DevInfo[ index ].m_TickSlotCount = 0;
	gaSSMac_DevInfo[ index ].m_CurrentSlotUnit = 0;
	
	gaSSMac_DevInfo[ index ].m_CurrentSlot = slot;
	gaSSMac_DevInfo[ index ].m_BaseChannel = channel;
	gaSSMac_DevInfo[ index ].m_SlotMode = mode;
	
	return TRUE;
}


// -------------------------------------------------
//   Init channel table for freq hopping
// -------------------------------------------------
static void SSMac_InitChannelTable(SK_UB table[], SK_UB size){
	SK_UH i;
	
	if( size > SS_MAX_HOPPING_CH_NUM ) return;

	for( i = 0; i < size; i++ ){
		table[i] = gnSSMac_BaseChannel; 
	}
}


SK_BOOL SSMac_SetChannelTable(SK_UB index, SK_UB ch){
	if( index >= SS_MAX_HOPPING_CH_NUM ) return FALSE;
	
	gaSSMac_ChannelTable[index] = ch;
	return TRUE;
}


//Ch tableをbase, base+1, base+2, base+3に初期化
void ResetChannelTable(SK_UB ch){
	SK_UB i;
	for( i = 0; i < SS_MAX_HOPPING_CH_NUM; i++ ){
		gaSSMac_ChannelTable[i] = ch + i;
	}	
}


// -------------------------------------------------
//   Exec channel hopping according to ch table
// -------------------------------------------------
SK_BOOL SSMac_ChannelHopping(SK_UH slot, SK_UB bscn){
	SK_UB table_index;
	SK_UB channel_index;
	
	if( gnSSMac_HoppingEnable == FALSE ) return TRUE;
	
	if( gnSSMac_CurrentHoppingTable <= SS_HOPPING_TABLE1 ) return TRUE;
	
	if( gnSSMac_CurrentHoppingTable > SS_HOPPING_TABLE4 ) return FALSE;
	
	if( slot == 0 ){
		SK_PHY_ChangeChannel(gnSSMac_BaseChannel);
		return TRUE;
	}
	
	table_index = (SK_UB)(((slot-1)%4 + (bscn%4)) %4);
	if( table_index > 3 ) return FALSE;
	
	channel_index = gaSSMac_HoppingTable[ gnSSMac_CurrentHoppingTable ][ table_index ];
	if( channel_index > (SS_MAX_HOPPING_CH_NUM-1) ) return FALSE;
	
	return SK_PHY_ChangeChannel( gaSSMac_ChannelTable[ channel_index ] );
}


//フレーム先頭ビットの受信時点でタイムスロットの一定範囲に入っていない
//ならば、そのフレームは同期してないので受信破棄する
//条件設定が難しいのでTBD
SK_BOOL SSMac_IsSlotAcceptable(void){
	//
	//
	//
	return TRUE;
}


// -------------------------------------------------
//   Stats
// -------------------------------------------------
void SSMac_InitStats(void){
	memset(&gaSSMac_Stats, 0, sizeof(gaSSMac_Stats));
}


SS_STATISTICS* SSMac_GetStats(void){
	return &gaSSMac_Stats;
}


// -------------------------------------------------
//   Random calculation
// !! should be replaced with more preceise preudo-random function !!
// -------------------------------------------------
static SK_UW GetPower2(SK_UB power) {
	SK_UW returnvalue = 1;
	if (power > 32) {
		power = 32;
	}
	for ( ; power > 0; power--) {
		returnvalue *= 2;
	}
	return returnvalue;
}


// returns value between 0 (inclusive) and 32768 (exclusive)
static SK_UH GetSeed(void) { 
	SK_UW sum = 0, seed;
	SK_UB i;

	for (i=0; i<8; i++) {
		sum += (gnSSMac_Address.Body.m_Raw[i] * GetPower2(i%4));
	}
	seed = (sum + 1) * (SK_GetLongTick() + 1);
    seed = seed * 1103515245L + 12345;
    return (SK_UH) ((seed / 65536L) % 32768U);
}


static SK_UW __ssmac_rand(void){
	return rand();	
}


SK_UW SSMac_GetRand32(void){
	SK_UW val1, val2;
	/*
	double r = __ssmac_rand();
	r = r / ((double)RAND_MAX + (double)1.0);
	r = r * 0xFFFF;
	*/
	val1 = (SK_UW) (((double)__ssmac_rand() / ((double)RAND_MAX + (double)1.0)) * 0xFFFF);
	val2 = (SK_UW) (((double)__ssmac_rand() / ((double)RAND_MAX + (double)1.0)) * 0xFFFF);
	return val1 << 16 | val2;
}


//Generate random AES Key
//Todo: must use much better random func
static void GenerateEncKey(SK_UB* key, SK_UB key_len){
	SK_UB i;
	
	if( key_len != SS_AES_KEY_LEN ) return;
	
	for( i = 0; i < key_len; i++ ){
		key[i] = (SK_UB)__ssmac_rand();
	}
}


// -------------------------------------------------
//   Sleep RF and state management
// -------------------------------------------------
SK_BOOL SSMac_Sleep(SK_BOOL force){
	if( force == TRUE || gnPHY_CurrentTRX != SK_PHY_TRX_OFF ) {
		#ifdef DEBUG_SLEEP
		SK_print("sleep...");
		#endif
		
		SK_PHY_Sleep();
		return TRUE;
	}

	return FALSE;
}


// -------------------------------------------------
//   Wakeup RF
// -------------------------------------------------
SK_BOOL SSMac_Wakeup(void){
	if( gnPHY_CurrentTRX != SK_PHY_RX_ON ) {
		#ifdef DEBUG_SLEEP
		SK_print("wakeup...");
		#endif
		
		SK_PHY_Wakeup();
		return TRUE;
	}
	
	return FALSE;
}



// -------------------------------------------------
//   some test codes
// -------------------------------------------------

void test_bitmap(void){
	SK_BOOL ans;
	
	memset(gaSSMac_SlotBitmap, 1, sizeof(gaSSMac_SlotBitmap));
	memset(gaSSMac_SlotBitmap, 0, sizeof(gaSSMac_SlotBitmap));
	
	SetSlotBitmap(0);
	SetSlotBitmap(31);
	SetSlotBitmap(32);
	SetSlotBitmap(1023);
	SetSlotBitmap(1024);

	ans = IsSetBitmap(0); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(31); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(32); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(1023); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(1024); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");

	ClearSlotBitmap(0);
	ClearSlotBitmap(31);
	ClearSlotBitmap(32);
	ClearSlotBitmap(1023);
	ClearSlotBitmap(1024);

	ans = IsSetBitmap(0); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(31); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(32); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(1023); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
	
	ans = IsSetBitmap(1024); SK_print_hex((SK_UW)ans, 1);
	SK_print_hex(ans, 1); SK_print("\r\n");
}
