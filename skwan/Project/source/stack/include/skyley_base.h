/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __skyley_base_h__
#define __skyley_base_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include	"compiler_options.h"
#include	"skyley_type.h"

// -------------------------------------------------
//   定数
// -------------------------------------------------

//Note: ML7416は4の倍数を指定すること
#define		SK_MBX_NUM					20			// total number of message box slots
#define		SK_CMD_MEMBLOCK_SIZE		24			// Block size for command memory pool
#define		SK_CMD_MEMBLOCK_NUM			12			// Total number of command memory pool block
#define		SK_DATA_MEMBLOCK_SIZE		(PSDU+40)	// Block size for data memory pool
#define		SK_DATA_MEMBLOCK_NUM		DEF_DATA_MEMBLOCK_NUM		// Total number of data memory pool block

// Layer ID
#define		SK_LAYER_PHY				0			// Phy Layer
#define		SK_LAYER_MAC				1			// MAC Layer
#define		SK_LAYER_SS_MAC				3
#define		SK_LAYER_APL				250

// timeout
#define 	SK_MSG_TIMEOUT				5000		// Internal timeout


// -------------------------------------------------
//   コマンド番号
// -------------------------------------------------

enum	{
	// ---- PHY
	SK_PD_DATA_REQUEST_CMD,
	SK_PD_DATA_CONFIRM_CMD,
	SK_PD_DATA_INDICATION_CMD,

	// ---- MAC
	SK_MCPS_DATA_REQUEST_CMD,
	SK_MCPS_DATA_CONFIRM_CMD,
	SK_MCPS_DATA_INDICATION_CMD,
	SK_MLME_BEACON_NOTIFY_INDICATION_CMD,
	SK_MLME_SCAN_REQUEST_CMD,
	SK_MLME_SCAN_CONFIRM_CMD,
	SK_MLME_COMM_STATUS_INDICATION_CMD,
	SK_MLME_POLL_REQUEST_CMD,
	SK_MLME_POLL_CONFIRM_CMD,
	SK_MLME_SYNC_LOSS_INDICATION_CMD,
	SK_MLME_START_REQUEST_CMD,
	SK_MLME_START_CONFIRM_CMD,

	SK_MLME_ASSOCIATE_REQUEST_CMD,
	SK_MLME_ASSOCIATE_INDICATION_CMD,
	SK_MLME_ASSOCIATE_RESPONSE_CMD,
	SK_MLME_ASSOCIATE_CONFIRM_CMD,
	SK_MLME_DISASSOCIATE_REQUEST_CMD,
	SK_MLME_DISASSOCIATE_INDICATION_CMD,
	SK_MLME_DISASSOCIATE_CONFIRM_CMD,
	SK_MLME_ORPHAN_INDICATION_CMD,
	SK_MLME_ORPHAN_RESPONSE_CMD,
	
	// ---- SS_MAC
	SS_SLOT_CHANGED_INDICATION_CMD,
	SS_SLOT_PRE_CHANGE_INDICATION_CMD,
	SS_BEACON_NOTIFY_INDICATION_CMD,
	SS_META_BEACON_NOTIFY_INDICATION_CMD,
	SS_SYNC_INDICATION_CMD,
	SS_COMM_STAT_INDICATION_CMD,
	SS_DATA_REQUEST_CMD,
	SS_PEND_DATA_REQUEST_CMD,
	SS_DATA_INDICATION_CMD,
	SS_ACK_INDICATION_CMD,
	SS_DATA_CONFIRM_CMD,
	SS_JOIN_REQUEST_CMD,
	SS_JOIN_CONFIRM_CMD
};


// -------------------------------------------------
//   typedef
// -------------------------------------------------

struct SK_MailboxList {
	struct SK_MailboxList*		m_pNext;			// Pointer to next message
	struct SK_MailboxList*		m_pPrev;			// Pointet to previous message
	SK_UB						m_nMBXID;			// ID of message box
	SK_UB						m_nCommand;			// Command ID
	SK_UB						m_nResMBXID;		// ID of receiver message box of response
	SK_VP						m_pItem;			// Pointer to contents
	
	SK_UW						m_nTimeStamp;		// Last time stamp
	SK_UH						m_nLatency;			// Delay time for continuous execution
	SK_UB						m_nRepeat;			// Number of repeat for continuous execution
	SK_UH						m_nSlot;				// Target time slot for this event
};
typedef struct SK_MailboxList T_SK_MailboxList;


#ifndef SK_STATESTART
// -------------------------------------------------
//   Macros for state machine procedure
// -------------------------------------------------
// 1.Describe SK_STATESTART(***) in global part. *** is a label, ex. "MAC", "NWK"
// 2.Define SK_STATEADD(***,Number) at beginning of a function
// 3.Define SK_STATEEND(***) at end of a function
// 4.Then macro can be SK_SLEEP(time,***,Number) for instance, here *** is a name of label defined in part 1

	
#define SK_STATESTART(label)									\
	static SK_UB 	gnSK_State_##label		= 0;				\
	static SK_UW 	gnSK_Time_##label		= 0


#define SK_STATEADD(label,num)			if (gnSK_State_##label == num) { goto StateJump_##label##num; }


#define SK_STATEEND(label)				StateEnd_##label:


#define SK_SLEEP(time,label,num)								\
{																\
StateJump_##label##num:											\
	if (gnSK_State_##label == num) {							\
		if ((SK_UW)((SK_UW)gnSK_SystemTick - (SK_UW)gnSK_Time_##label)<time) {	\
			goto StateEnd_##label;								\
		} else {												\
			gnSK_State_##label = 0;								\
		}														\
	} else {													\
		gnSK_Time_##label = gnSK_SystemTick;					\
		gnSK_State_##label = num;								\
		goto StateEnd_##label;									\
	}															\
}


#define SK_WAITFOR(time,flg,label,num)							\
{																\
StateJump_##label##num:											\
	if (gnSK_State_##label == num) {							\
		if ( !(flg) && ((SK_UW)((SK_UW)gnSK_SystemTick - (SK_UW)gnSK_Time_##label)<time) ) {	\
			goto StateEnd_##label;								\
		} else {												\
			gnSK_State_##label = 0;								\
		}														\
	} else {													\
		gnSK_Time_##label = gnSK_SystemTick;					\
		gnSK_State_##label = num;								\
		goto StateEnd_##label;									\
	}															\
}

#define SK_INITSTATE(label)						\
{												\
	gnSK_State_##label		= 0;				\
	gnSK_Time_##label		= 0;				\
}		

#define SK_RETURN_STATE(label)									\
{																\
	if(gnSK_State_##label == 0){								\
		return TRUE;											\
	} else { 													\
		return FALSE;											\
	}															\
}
#endif


#define SK_GETSTATE(label) 			(gnSK_State_##label)
#define SK_SETSTATE(X, label) 		{ gnSK_State_##label = X; }
#define SK_PUSHSTATE(X, Y, label) 	{ gnSK_State_##label = X; gnSK_State_Next_##label = Y; }
#define SK_NEXTSTATE(label) 		{ gnSK_State_##label = gnSK_State_Next_##label; }


// -------------------------------------------------
//   Endian abstraction
// -------------------------------------------------
// LSB -> MSB
#define MAC_SET_WORD(ptr, wrd)			{ ((SK_UB *)(ptr))[0] = (SK_UB)((SK_UH)(wrd) & 0xFF); ((SK_UB *)(ptr))[1] = (SK_UB)(((SK_UH)(wrd) >> 8) & 0xFF); };
#define MAC_SET_LONG(ptr, dwrd)			{ ((SK_UB *)(ptr))[0] = (SK_UB)((SK_UW)(dwrd) & 0xFF); ((SK_UB *)(ptr))[1] = (SK_UB)(((SK_UW)(dwrd) / 0x100L) & 0xFF); ((SK_UB *)(ptr))[2] = (SK_UB)(((SK_UW)(dwrd) / 0x10000L) & 0xFF); ((SK_UB *)(ptr))[3] = (SK_UB)(((SK_UW)(dwrd) / 0x1000000L) & 0xFF); };
#define MAC_GET_WORD(ptr)				((SK_UH)(((SK_UB *)(ptr))[0]) + (SK_UH)(((SK_UB *)(ptr))[1] * 0x100))
#define MAC_GET_LONG(ptr)				((SK_UW)((SK_UW)(((SK_UB *)(ptr))[0])) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[1]) * 0x100) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[2]) * 0x10000L) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[3]) * 0x1000000L))

#define MAC_SET_WORD_B(ptr, wrd)		{ ((SK_UB *)(ptr))[1] = (SK_UB)((SK_UH)(wrd) & 0xFF); ((SK_UB *)(ptr))[0] = (SK_UB)(((SK_UH)(wrd) >> 8) & 0xFF); };
#define MAC_SET_LONG_B(ptr, dwrd)		{ ((SK_UB *)(ptr))[3] = (SK_UB)((SK_UW)(dwrd) & 0xFF); ((SK_UB *)(ptr))[2] = (SK_UB)(((SK_UW)(dwrd) / 0x100L) & 0xFF); ((SK_UB *)(ptr))[1] = (SK_UB)(((SK_UW)(dwrd) / 0x10000L) & 0xFF); ((SK_UB *)(ptr))[0] = (SK_UB)(((SK_UW)(dwrd) / 0x1000000L) & 0xFF); };
#define MAC_GET_WORD_B(ptr)				((SK_UH)(((SK_UB *)(ptr))[1]) + (SK_UH)(((SK_UB *)(ptr))[0] * 0x100))
#define MAC_GET_LONG_B(ptr)				((SK_UW)((SK_UW)(((SK_UB *)(ptr))[3])) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[2]) * 0x100) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[1]) * 0x10000L) + (SK_UW)((SK_UW)(((SK_UB *)(ptr))[0]) * 0x1000000L))


// -------------------------------------------------
//   Global variables
// -------------------------------------------------
// System timer
extern SK_UW	gnSK_SystemTick;
extern SK_UH	gnSK_SlotUnit;
extern SK_UH	gnSK_Slot;


// -------------------------------------------------
//   Utility functions
// -------------------------------------------------

void SK_Initialize(void);
void SK_IncrementTimeTick(SK_UH tick);
void SK_IncrementEventTick(SK_UH tick);
SK_UW SK_GetLongTick(void);


// -------------------------------------------------
//   Message boxes
// -------------------------------------------------

SK_ER SK_PostMessage(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item);
SK_ER SK_PostMessageS(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item,SK_UH slot);
SK_ER SK_PostMessageL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item,SK_UH latency,SK_UB repeat);
SK_ER SK_PostMessageSL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item,SK_UH latency,SK_UB repeat,SK_UH slot);
SK_ER SK_GetMessage(SK_UB id,SK_UB *resid,SK_UB *cmd,SK_VP *item);
SK_ER SK_GetMessageByCmd(SK_UB id,SK_UB *resid,SK_UB cmd,SK_VP *item);
SK_ER SK_StopMessage(SK_VP item);


// -------------------------------------------------
//   Maitain memory block
// -------------------------------------------------

SK_ER SK_AllocCommandMemory(SK_VP *item);
SK_ER SK_AllocDataMemory(SK_VP *item);
SK_ER SK_AllocDataMemoryWith(SK_VP *item, SK_UB tag);
SK_ER SK_FreeMemory(SK_VP item);
SK_ER SK_FreeMemoryL(SK_VP item, SK_BOOL force);
SK_ER SK_FreeMemoryTag(SK_UB tag);
SK_ER SK_FreeMemoryTagNoLock(SK_UB tag);


// -------------------------------------------------
//   Count number of free memory area
// -------------------------------------------------
SK_UB SK_CountFreeDataMemory(void);
SK_UB SK_CountFreeCommandMemory(void);
SK_UB SK_CountFreeMemoryTag(SK_UB tag);


// -------------------------------------------------
//   Set/Get condition flag of memory pool
// -------------------------------------------------
#define FLAG_GET 0
#define FLAG_SET 1

SK_UB SK_MemoryFlag(SK_VP item, SK_UB flag, SK_UB action);


// -------------------------------------------------
//   Stack init and main loop
// -------------------------------------------------
void SK_Base_Init(SK_UW macadr1, SK_UW macadr2);
void SK_Base_Main(void);


#ifdef __cplusplus
}
#endif

#endif
