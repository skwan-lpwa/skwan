/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __prot_ss_mac_h__
#define __prot_ss_mac_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include	"compiler_options.h"
#include    "skyley_stack.h"


// -------------------------------------------------
//   Consts
// -------------------------------------------------
#define SS_PROTOCOL_VERSION				(1)

#define SS_PHY_LORA						(0)
#define SS_PHY_15_4G					(1)
#define SS_PHY_15_4K					(2)
#define SS_PHY_UNB_TYPE1				(3)
#define SS_PHY_SCANNER					(250)

//Time slot config
//system tick interval (in msec)
//!!Changing systick duration is not recommended 
#define	SS_SYSTICK						(10UL) 

//number of ticks per 1 slot unit
#define	SS_TICKS_PER_UNIT				(6UL) 

//total 1 slot unit duratin (in msec)
#define	SS_UNIT_DURATION				(SS_TICKS_PER_UNIT *  SS_SYSTICK) 

//nuumber of units per 1 slot
#define	SS_UNITS_PER_SLOT				(16UL)

//total 1 time slot duration (in msec)
#define SS_SLOT_DURATION				(SS_UNITS_PER_SLOT * SS_UNIT_DURATION) 

//Header field length
#if (BASE_PHY_TYPE == SS_PHY_15_4G) //15.4g
	#define SS_PREAMBLE_LEN				(15UL)
	#define SS_MHR_LEN					(2UL) //length extention
	#define SS_LOWER_LAYER_OVERHEAD		(2UL) //15.4g PHY Hdr
#else
	#define SS_PREAMBLE_LEN				(5UL)
	#define SS_MHR_LEN					(1UL)
	#define SS_LOWER_LAYER_OVERHEAD		(0UL)
#endif

#define SS_SYNC_LEN						(2UL)
#define SS_UPFCTRL_LEN					(2UL)
#define SS_SEQ_NO_LEN					(1UL)
#define SS_FRAME_CNT_LEN				(3UL)
#define SS_PAYLOAD_LEN					(DEF_PAYLOAD_LEN)
#define SS_MIC_LEN						(4UL)
#define SS_CRC_LEN						(2UL)

#define SS_MAX_FRAME_LEN				(SS_MHR_LEN + SS_UPFCTRL_LEN + SS_SEQ_NO_LEN + SS_FRAME_CNT_LEN + SS_PAYLOAD_LEN + SS_MIC_LEN + SS_CRC_LEN)
#define SS_SLOT_HASHKEY_LEN				(4)

#ifdef USE_STATION_DB
#define SS_MAX_SHARED_DEVICES			(2)
#else
#define SS_MAX_SHARED_DEVICES			(1)
#endif

#define SS_MAX_NIC_NUM					(4)
#define SS_SYNC_LOSS_THRESHOLD			(3)
#define SS_MAX_DOWN_SLOT_NUM			(140)
#define SS_MAX_HOPPING_CH_NUM			(4)
#define SS_INITIAL_BASE_CHANNEL			(33)

//Security component
#define SS_AES_KEY_LEN					(16)
#define	SS_CCMSTAR_SEC_LEVEL			(5)
#define SS_CCMSTAR_NONCE_LEN			(13)
#define SS_MAX_FRAME_COUNTER			(0x00FFFFFF)

//Result code
#define SS_MAC_SUCCESS					0x00
#define SS_MAC_SYNC_START				0xE0
#define SS_MAC_SYNC_LOSS				0xE1
#define SS_MAC_INDIRECT_EXPIRED			0xE2
#define SS_MAC_INDIRECT_OVERFLOW		0xE3
#define SS_MAC_FRAME_PENDING			0xE4
#define SS_MAC_INVALID_REQUEST			0xE5
#define SS_MAC_SECURITY_FAILURE			0xE6
#define SS_MAC_EVENT_NO_RESP			0xF0

//Slot mode (3bit)
#define SS_SLOTMODE_16					0
#define SS_SLOTMODE_32					1
#define SS_SLOTMODE_64					2
#define SS_SLOTMODE_128					3
#define SS_SLOTMODE_256					4
#define SS_SLOTMODE_512					5
#define SS_SLOTMODE_1024				6

//hopping table
#define SS_HOPPING_TABLE1				0
#define SS_HOPPING_TABLE2				1
#define SS_HOPPING_TABLE3				2
#define SS_HOPPING_TABLE4				3

//Frame type (2bit)
#define SS_FRAME_BEACON					0
#define SS_FRAME_META_BEACON			1
#define SS_FRAME_DATA					2
#define SS_FRAME_ACK					3

//Tx options for DATA.request
#define SS_TXOPTIONS_INDIRECT			1
#define SS_TXOPTIONS_SECURITY			2

//Selectors
#define SS_SELECTOR_MAC_CMD				0
#define SS_SELECTOR_JOIN_CMD			1
#define SS_SELECTOR_JOIN_RES_CMD		2
// selector 3-14 reserved
// selector 15 application specific
#define SS_SELECTOR_APP_DATA			15

//Frame length
//Length includes CMD byte itself
#define SS_BEACON_LEN					(11)
#define SS_META_BEACON_LEN				(16)
#define SS_ACK_LEN						(4)
//Data frame is variable length

//MAC Cmd length
#define SS_JOIN_CMD_LEN					(8)
#if (BASE_PHY_TYPE == SS_PHY_15_4K)
	#define SS_JOIN_RES_CMD_LEN			(19)
#else
	#define SS_JOIN_RES_CMD_LEN			(28)
#endif

//Join cmd status code
#define SS_STAT_JOIN_OK					(0)
#define SS_STAT_DEVICE_FULL				(1)
#define SS_STAT_JOIN_CONFLICT			(2)
#define SS_STAT_INDIRECT_OVERFLOW		(3)
#define SS_STAT_NO_RES_FROM_STATION		(4)

//Device type
#define SS_TYPE_STATION					0
#define	SS_TYPE_DEVICE					1
#define SS_TYPE_METABEACON				2
#define SS_TYPE_IDLING					3

#define SS_BEACON_TRANS_TIME_ABS		(DEF_BEACON_TRANS_TIME)

//Slot-Addr database entry size
//Station only
#define SS_DB_SIZE						(DEF_MAX_DEVICE_SIZE)

//Pending buffer size
//Station only
#define	 SS_PENDINGBUF_SIZE				(DEF_PENDING_BUF_SIZE)

//in usec
#define SS_TRIM_COUNT_UNIT				(250L) 
//in units SS_TRIM_COUNT_DURATION * SS_TRIM_COUNT_UNIT is usec
#define SS_TRIM_COUNT_DURATION			(0L)

//10sec
#define SS_META_BEACON_DURATION			(10000)

#if (BASE_PHY_TYPE == SS_PHY_15_4G)  //15.4g
	#define SS_GET_MHR_LEN(MHR) 			((SK_UH)(MHR.Field.m_Length) | ((SK_UH)(MHR.Field.m_LengthExt)<<6))
	#define SS_SET_MHR_LEN(MHR, len) 		{ MHR.Field.m_Length = len & 0x3F; MHR.Field.m_LengthExt = (SK_UB)(len >> 6); }
	#define SS_ENCODE_MHR(MHR, buf, offset)	{ buf[offset] = MHR.Raw[0]; offset++; buf[offset] = MHR.Raw[1]; offset++; }
#else
	#define SS_GET_MHR_LEN(MHR) 			((SK_UH)(MHR.Field.m_Length))
	#define SS_SET_MHR_LEN(MHR, len) 		{ MHR.Field.m_Length = len; }
	#define SS_ENCODE_MHR(MHR, buf, offset)	{ buf[offset] = MHR.Raw[0]; offset++; }
#endif

// -------------------------------------------------
//   Frame format
// -------------------------------------------------
#if (BASE_PHY_TYPE == SS_PHY_15_4G)  //15.4g
typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_Length:6;
		SK_UB			m_Type:2;
		SK_UB			m_LengthExt;
	} 				Field;
} SS_MHR;
#else
typedef union {
	SK_UB			Raw[1];
	struct {
		SK_UB			m_Length:6;
		SK_UB			m_Type:2;
	} 				Field;
} SS_MHR;
#endif

typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_SlotL:8;
		SK_UB			m_SlotH:2;
		SK_UB			m_InSlotIdx:1;
		SK_UB			m_Selector:4;
		SK_UB			m_Security:1;
	} 				Field;
} SS_UP_FRAMECONTROL;

typedef union {
	SK_UB			Raw[1];
	struct {
		SK_UB			m_Selector:4;
		SK_UB			m_FramePending:1;
		SK_UB			m_InSlotIdx:1;
		SK_UB			reserved:2;
	} 				Field;
} SS_DOWN_FRAMECONTROL;

typedef union {
	SK_UB			Raw[1];
	struct {
		SK_UB			m_Version:2;
		SK_UB			m_Capacity:2;
		SK_UB			reserved:4;
	} 				Field;
} SS_BCN_FRAMECONTROL;

typedef union {
	SK_UB			Raw[1];
	struct {
		SK_UB			m_SlotMode:3;
		SK_UB			m_HoppingTable:2;
		SK_UB			reserved:3;
	} 				Field;
} SS_SLOT_CONFIG;

typedef union {
	SK_UB			Raw[3];
	struct {
		SK_UB			m_SlotL:8;
		SK_UB			m_SlotH:2;
		SK_UB			m_BaseChannel:6;
		SK_UB			m_SlotMode:3;
		SK_UB			reserved:5;
	} 				Field;
} SS_NIC_CONFIG;

#if (BASE_PHY_TYPE == SS_PHY_15_4K)
#pragma pack(1)
#endif
typedef struct {
	SK_UH m_SlotNum;
	//1 slotに何デバイス共有するか
	SK_ADDRESS m_Addr[ SS_MAX_SHARED_DEVICES ];
	SK_UB m_Key[ SS_MAX_SHARED_DEVICES ][ SS_AES_KEY_LEN ];
	SK_UW m_FrameCounter[ SS_MAX_SHARED_DEVICES ];
	SK_UW m_OutgoingFrameCounter[ SS_MAX_SHARED_DEVICES ];
} SS_SLOT_ADDR_DB_ITEM;
#if (BASE_PHY_TYPE == SS_PHY_15_4K)
#pragma pack()
#endif

typedef struct {
	SK_UH	m_TickSlotCount;
	SK_UH	m_CurrentSlotUnit;
	
	SK_UH	m_CurrentSlot;
	SK_UB	m_BaseChannel;
	SK_UB	m_SlotMode;
} SS_DEV_INFO;


// -------------------------------------------------
//   SLOT-CHANGED
// -------------------------------------------------
//SLOT-CHANGED.indication
typedef struct {
    SK_UH               m_SlotNum;
} SS_SLOT_CHANGED_INDICATION;


// -------------------------------------------------
//   SLOT-PRE-CHANGE
// -------------------------------------------------
//SLOT-PRE-CHANGE.indication
typedef struct {
    SK_UH               m_SlotNum;
} SS_SLOT_PRE_CHANGE_INDICATION;


// -------------------------------------------------
//   BEACON-NOTIFY
// -------------------------------------------------
//BEACON-NOTIFY.indication
typedef struct {
    SK_UB			m_Version;
    SK_UB			m_BSN;
    SK_UB			m_Capacity;
    SK_UB			m_SlotMode;
    SK_UB			m_HoppingTable;
    SK_UW			m_StationId;
    SK_UB			m_Rssi;
} SS_BEACON_NOTIFY_INDICATION;


// -------------------------------------------------
//   META-BEACON-NOTIFY
// -------------------------------------------------
//META-BEACON-NOTIFY.indication
typedef struct {
	SK_UW			m_StationId;
    SS_DEV_INFO		m_DevInfo[ SS_MAX_NIC_NUM ];
} SS_META_BEACON_NOTIFY_INDICATION;


// -------------------------------------------------
//   SYNC
// -------------------------------------------------
//SYNC.indication
typedef struct {
    SK_UB               m_Status;
} SS_SYNC_INDICATION;


// -------------------------------------------------
//   COMM-STAT
// -------------------------------------------------
//COMM-STAT.indication
typedef struct {
    SK_UB               m_Status;
} SS_COMM_STAT_INDICATION;


// -------------------------------------------------
//   DATA-REQUEST
// -------------------------------------------------
//SS-DATA.request
typedef struct {
	SK_ADDRESS			m_DstAddress;
	SK_UB				m_Selector;
	SK_UB				m_TxOptions;
	SK_UH				m_Handle;
	SK_UB               m_MsduLength;
	SK_UB				m_Msdu[ SS_PAYLOAD_LEN + SS_MIC_LEN ];
} SS_DATA_REQUEST;


//SS-DATA.indication
typedef struct {
	SK_ADDRESS			m_SrcAddress;
	SK_UH				m_RecvSlot;
	SK_UB				m_RecvInSlotIdx;
	SK_UB				m_Selector;
	SK_UB				m_Rssi;
	SK_UB				m_Channel;
	SK_UB               m_MsduLength;
	SK_UB				m_Msdu[ SS_PAYLOAD_LEN + SS_MIC_LEN ];
} SS_DATA_INDICATION;


//SS-DATA.confirm
typedef struct {
    SK_UB               m_Status;
    SK_ADDRESS			m_DstAddress;
    SK_UH				m_Handle;
} SS_DATA_CONFIRM;


//SS-ACK.indication
typedef struct {
	SK_UB				m_FramePending;
} SS_ACK_INDICATION;


// -------------------------------------------------
//   JOIN-REQUEST
// -------------------------------------------------
//SS-JOIN.request
typedef struct {
	SK_UW				m_StationID;
} SS_JOIN_REQUEST;


//SS-JOIN.confirm
typedef struct {
	SK_UB				m_Status;
	SK_UH				m_SlotNum;
	SK_UB				m_InSlotIdx;
} SS_JOIN_CONFIRM;


// -------------------------------------------------
//   Pending buffer item
// -------------------------------------------------
typedef struct {
	SS_DATA_REQUEST*		m_Packet;
	SK_UW					m_TimeStamp;
	SK_UB					m_DownSlot;
} SS_PENDING_INFO;


// -------------------------------------------------
//   ENC-REQUEST
// -------------------------------------------------
typedef struct {
	SK_UB				m_PsduLength;
	SK_UB*				m_Psdu;
	SK_UB				m_HeaderLength;
} SS_ENC_REQUEST;


typedef struct {
	SK_UB				m_Bytes[ SS_CCMSTAR_NONCE_LEN ]; 
} SS_CCMSTAR_NONCE;


// *************************************************
//
//   For External DB Mode
//
// *************************************************
// -------------------------------------------------
//   REGISTER-DEVICE.request/response
// -------------------------------------------------
typedef struct {
    SK_ADDRESS		m_Address;
    SK_UH			m_SrcSlot;
} SS_REGISTER_DEVICE_REQUEST;


typedef struct {
	SK_UH		m_Slot;
	SK_UB		m_InSlotIdx;
	SK_UB		m_Key[SS_AES_KEY_LEN];
} SS_REGISTER_DEVICE_RESPONSE;


// -------------------------------------------------
//   DEVICE-DB.request/response
// -------------------------------------------------
typedef struct {
	SK_UH			m_Slot;
	SK_UB			m_InSlotIdx;
	SK_ADDRESS		m_Address;
} SS_DEVICE_DB_REQUEST;


typedef struct {
	SK_UH 			m_Slot;
	SK_UB 			m_InSlotIdx;
	SK_UB 			m_Key[SS_AES_KEY_LEN];
	SK_UW 			m_FrameCounter;
	SK_UW 			m_OutgoingFrameCounter;
	SK_ADDRESS		m_Address;
} SS_DEVICE_DB_RESPONSE;


// -------------------------------------------------
//   FRMCNT-UPDATE.request
// -------------------------------------------------
typedef struct {
	SK_ADDRESS		m_Address;
	SK_UW			m_IncomingFrameCounter;
	SK_UW			m_OutgoingFrameCounter;
} SS_FRMCNT_UPDATE_REQUEST;


// -------------------------------------------------
//   Direct table access
// -------------------------------------------------
extern SS_SLOT_ADDR_DB_ITEM gaSSMac_SlotAddr_DB[];
extern SS_PENDING_INFO gaSSMac_PendingBuf[];
extern SS_DEV_INFO gaSSMac_DevInfo[];
extern SK_UB gaSSMac_ChannelTable[];


// -------------------------------------------------
//   MAC Public functions
// -------------------------------------------------
void SSMac_Init(SK_UW macadr1, SK_UW macadr2);
void SSMac_Task(void);

//Timer slot management
void IncrementSlot(SK_UH tick);
void SSMac_IncCurrentSlot(SK_BOOL event);
SK_UH SSMac_GetCurrentSlot(void);
void SSMac_SetCurrentSlot(SK_UH slot);
SK_BOOL SSMac_IsJustOn(SK_UH slot);
void SSMac_AdjustSlot(SK_UB bscn, SS_SLOT_CONFIG* slot_cfg, SK_UW timestamp);
void SSMac_SetSlotUnitTick(SK_UH unit, SK_UH tick);

void SSMac_SetLowerLayer(SK_UB layer);
void SSMac_SetUpperLayer(SK_UB layer);
SK_UB _SSMac_GetLowerLayer(void);
SK_UB SSMac_GetUpperLayer(void);

//SK_ADDRESS calculation
SK_UB SSMac_SetAddress(SK_UB *ptr, SK_ADDRESS* adr);
void SSMac_CopyAddress(SK_ADDRESS* dst, SK_ADDRESS* src);
void SSMac_GetExtAddress(SK_ADDRESS* dstaddr, SK_UB *ptr);
SK_BOOL SSMac_Equals(SK_ADDRESS* dst, SK_ADDRESS* src);
SK_ADDRESS* SSMac_GetAddress(void);

//Packet formatting
void SSMac_TransmitBeacon(void);
SK_UB SSMac_EncodeBeacon(SK_UB* buf, SK_UB len);
SK_UB SSMac_AnalyzeBeacon(SK_UB* buf, SK_UB len, SS_BCN_FRAMECONTROL* bcn_fctrl, SK_UB* bscn, SS_SLOT_CONFIG* slot_cfg, SK_UW* sta_id, SK_UW* rands);
SK_BOOL SSMac_TransmitData(SS_DATA_REQUEST* SdReq);
SK_UB SSMac_EncodeDataReq(SS_DATA_REQUEST* SdReq, SK_UB* buf, SK_UB len);
SK_UB SSMac_AnalyzeDataHdr(SK_UB* buf, SK_UB len, SS_UP_FRAMECONTROL* up_fctrl, SK_UB* seq_no, SK_UW* frame_cnt);
void SKMac_TransmitAck(SS_DOWN_FRAMECONTROL* fctrl, SK_UB seq, SK_UB drift, SK_UB pend_slot);
SK_UB SSMac_EncodeAck(SS_DOWN_FRAMECONTROL* fctrl, SK_UB seq, SK_UB drift, SK_UB pend_slot, SK_UB* buf, SK_UB len);
SK_UB SSMac_AnalyzeAck(SK_UB* buf, SK_UB len, SS_DOWN_FRAMECONTROL* down_fctrl, SK_UB* seq_no, SK_UB* drift, SK_UB* pend_slot);
void SSMac_TransmitMetaBeacon(void);
SK_UB SSMac_EncodeMetaBeacon(SK_UB* buf, SK_UB len);
SK_UB SSMac_AnalyzeMetaBeacon(SK_UB* buf, SK_UB len, SK_UW* sta_id, SS_DEV_INFO inf[]);
SK_BOOL SSMac_IsSlotAcceptable(void);

//Set/Get PIBs
SK_BOOL SSMac_SetDeviceType(SK_UB type);
SK_UB SSMac_GetDeviceType(void);
SK_BOOL SSMac_SetSlotMode(SK_UB mode);
SK_UB SSMac_GetSlotMode(void);
SK_BOOL SSMac_SetRxOnWhenIdle(SK_BOOL flag);
SK_BOOL SSMac_GetRxOnWhenIdle(void);
SK_UB* SSMac_GetSlotHashKey(void);
SK_UB SSMac_GetInSlotIdx(void);
SK_BOOL SSMac_SetInSlotIdx(SK_UB idx);
SK_BOOL SSMac_SetStationId(SK_UW id);
SK_UW SSMac_GetStationId(void);
SK_BOOL SSMac_SetPendExpTime(SK_UW time);
SK_UW SSMac_GetPendExpTime(void);
SK_UB* SSMac_GetAESKey(void);
SK_BOOL SSMac_SetAESKey(SK_UB* key, SK_UB key_len);
SK_UB* SSMac_GetPSK(void);
SK_BOOL SSMac_SetPSK(SK_UB* key, SK_UB key_len);
SK_BOOL SSMac_SetKey(SK_UB* key, SK_UB key_len);
const char* SSMac_GetVerStr(void);
const char* SSMac_FuncTypeStr(void);
SK_UH SSMac_GetSlotNum(void);
SK_BOOL SSMac_SetSlotNum(SK_UH slot);
SK_BOOL SSMac_SetHoppingTable(SK_UB table);
SK_UB SSMac_GetHoppingTable(void);
SK_BOOL SSMac_SetBaseChannel(SK_UB ch);
SK_UB SSMac_GetBaseChannel(void);
SK_UB SSMac_SetHoppingEnable(SK_BOOL flag);
SK_BOOL SSMac_GetHoppingEnable(void);
SK_BOOL SSMac_SetFullManage(SK_BOOL flag);
SK_BOOL SSMac_IsFullManage(void);
SK_UW SSMac_GetFrameCounter(void);
SK_UW SSMac_GetFrameCounterSTA(void);
SK_BOOL SSMac_SetSyncLossThreshold(SK_UB val);
SK_UB SSMac_GetSyncLossThreshold(void);

SK_BOOL SSMac_SetSlotCalib(SK_UB val);
SK_UB SSMac_GetSlotCalib(void);
SK_BOOL SSMac_SetFineCalib(SK_UW val);
SK_UW SSMac_GetFineCalib(void);

//Slot database
void SSMac_InitDB(SS_SLOT_ADDR_DB_ITEM db[], SK_UH size);
void SSMac_InitAddr(SK_ADDRESS* addr);
SS_SLOT_ADDR_DB_ITEM* SSMac_GetDBItemFor(SK_UH slot);

//Pending buffer
void SSMac_InitPendingBuf(SS_PENDING_INFO buf[], SK_UH size);

//Slot-Addr DB management
SK_BOOL SSMac_DirectJoin(SK_ADDRESS* addr,SK_UH slot, SK_UB idx);
SK_BOOL SSMac_AutoJoin(SK_ADDRESS* addr);
SK_BOOL SSMac_RemoveDev(SK_ADDRESS* addr);

//Device DB management
void SSMac_InitDevInfo(SS_DEV_INFO db[], SK_UH size);
SK_BOOL SSMac_SetDevInfo(SK_UB index, SK_UH slot, SK_UB channel, SK_UB mode);

//Freq hopping
SK_BOOL SSMac_SetChannelTable(SK_UB index, SK_UB ch);
SK_BOOL SSMac_ChannelHopping(SK_UH slot, SK_UB bscn);

SK_UW SSMac_GetRand32(void);

//power saving control
SK_BOOL SSMac_Sleep(SK_BOOL force);
SK_BOOL SSMac_Wakeup(void);

void SSMac_SlotUnitCalibCallback(void);

#ifdef __cplusplus
}
#endif

#endif
