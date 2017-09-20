/**
    Author:       Skyley Networks, Inc.
    Version:
    Description:  Protocol stack framework 
    
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
#include	"skyley_stack.h"

// -------------------------------------------------
//   Prototypes
// -------------------------------------------------
extern void SK_IncrementSlot(SK_UH tick);
extern SK_BOOL SSMac_IsJustOn(SK_UH slot);

static T_SK_MailboxList* SK_AllocLinkedList(void);
static void SK_ReleaseLinkedList(T_SK_MailboxList* list);
static void SK_RemoveFromList(T_SK_MailboxList *item);
static void SK_AddToTail(T_SK_MailboxList *item);


// -------------------------------------------------
//   •Ï”
// -------------------------------------------------
SK_UW						gnSK_SystemTick;

static SK_UB				gaSK_CmdMemoryFlag		[ SK_CMD_MEMBLOCK_NUM									];
static SK_UB				gaSK_DataMemoryFlag		[ SK_DATA_MEMBLOCK_NUM									];
static SK_UB				gaSK_DataMemoryTag		[ SK_DATA_MEMBLOCK_NUM									];

static SK_UW 				pad;

static SK_UB				gaSK_CmdMemory			[ (SK_CMD_MEMBLOCK_SIZE * SK_CMD_MEMBLOCK_NUM)			];
static SK_UB				gaSK_DataMemory			[ (SK_DATA_MEMBLOCK_SIZE * SK_DATA_MEMBLOCK_NUM)		];

static T_SK_MailboxList		gaSK_MessageList		[ SK_MBX_NUM											];
static T_SK_MailboxList		*gpstSK_MessageStart;
static T_SK_MailboxList		*gpstSK_MessageEnd;


SK_UB SK_MemoryFlag(SK_VP item, SK_UB flag, SK_UB action) {
	SK_UW		i;

	if (item == NULL) return 0xff;
		
	i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_CmdMemory))) / SK_CMD_MEMBLOCK_SIZE);
	if ( (item >= (SK_VP)(gaSK_CmdMemory)) && (i < SK_CMD_MEMBLOCK_NUM) ) {
		if( action == 1 ){
			gaSK_CmdMemoryFlag[i] = flag; 
		}
		return gaSK_CmdMemoryFlag[i];
	} else {
		i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_DataMemory))) / SK_DATA_MEMBLOCK_SIZE);
		if ( (item >= (SK_VP)(gaSK_DataMemory)) && (i < SK_DATA_MEMBLOCK_NUM) ) {
			if( action == 1 ){
				gaSK_DataMemoryFlag[i] = flag; 
			}
			return gaSK_DataMemoryFlag[i];
		} 
	}
	
	return 0xff;
}


void SK_RemoveFromList(T_SK_MailboxList *list){
	if ( list->m_pNext != NULL ) { list->m_pNext->m_pPrev = list->m_pPrev; }
	if ( list->m_pPrev != NULL ) { list->m_pPrev->m_pNext = list->m_pNext; }
	if ( gpstSK_MessageStart == list ) { gpstSK_MessageStart = list->m_pNext; }
	if ( gpstSK_MessageEnd   == list ) { gpstSK_MessageEnd   = list->m_pPrev; }
}


void SK_AddToTail(T_SK_MailboxList *list){
	if ( gpstSK_MessageEnd != NULL ) {
		gpstSK_MessageEnd->m_pNext	= list;
		list->m_pPrev				= gpstSK_MessageEnd;
		list->m_pNext				= NULL;
		gpstSK_MessageEnd			= list;
	} else {
		list->m_pPrev				= NULL;
		list->m_pNext				= NULL;
		gpstSK_MessageStart			= list;
		gpstSK_MessageEnd			= list;
	}
}


// -------------------------------------------------
//   Initialize stack framework
// -------------------------------------------------

void SK_Initialize(void) {
	SK_UB		i;

	
	RF_LOCK();
	
	// init system time counter
	gnSK_SystemTick = 0;

	// clear message box
	gpstSK_MessageStart = NULL;
	gpstSK_MessageEnd   = NULL;
	for(i=0;i<SK_MBX_NUM;i++) {
		gaSK_MessageList[i].m_pItem = NULL;
		gaSK_MessageList[i].m_pPrev = NULL;
		gaSK_MessageList[i].m_pNext = NULL;
		gaSK_MessageList[i].m_nTimeStamp = 0;
		gaSK_MessageList[i].m_nLatency = 0;
		gaSK_MessageList[i].m_nRepeat = 0;
	}
	
	// init memory blocks
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		gaSK_CmdMemoryFlag[i] = 0;
	}
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		gaSK_DataMemoryFlag[i] = 0;
		gaSK_DataMemoryTag[i] = 0;
	}

	
	RF_UNLOCK();
}


// -------------------------------------------------
//   Count up system timer
// -------------------------------------------------

void SK_IncrementTimeTick(SK_UH tick) {
	gnSK_SystemTick += tick;
	SK_IncrementSlot(tick);
}


void SK_IncrementEventTick(SK_UH tick) {
	gnSK_SystemTick += tick;
}


// -------------------------------------------------
//   Get system time counter
// -------------------------------------------------

SK_UW SK_GetLongTick(){
	return gnSK_SystemTick;
}


// -------------------------------------------------
//   Post a message to message box
// -------------------------------------------------
SK_ER SK_PostMessage(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item) {
	return SK_PostMessageSL(id, resid, cmd, item, 0, 0, 0xFFFF);
}

SK_ER SK_PostMessageL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item,SK_UH latency,SK_UB repeat) {
	return SK_PostMessageSL(id, resid, cmd, item, latency, repeat, 0xFFFF);
}

SK_ER SK_PostMessageS(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item,SK_UH slot) {
	return SK_PostMessageSL(id, resid, cmd, item, 0, 0, slot);
}


// -------------------------------------------------
//   Message post with specified repeat times and target slot
// -------------------------------------------------
SK_ER SK_PostMessageSL(SK_UB id,SK_UB resid,SK_UB cmd,SK_VP item, SK_UH latency, SK_UB repeat, SK_UH slot) {
	T_SK_MailboxList *list;
	if (item == NULL) {
        return SK_E_ER;
    }

	RF_LOCK();

	list = SK_AllocLinkedList();
	if (list == NULL) {
		RF_UNLOCK();
		return SK_E_ER;
	}
	
	list->m_pItem		= item;
	list->m_nMBXID		= id;
	list->m_nResMBXID	= resid;
	list->m_nCommand	= cmd;
	list->m_nTimeStamp 	= gnSK_SystemTick;
	list->m_nLatency 	= latency;
	list->m_nRepeat		= repeat;
	list->m_nSlot 		= slot;
	
	if( latency != 0 ){
		SK_MemoryFlag(item, 0x03, FLAG_SET);
	}

	SK_AddToTail(list);

	RF_UNLOCK();

	return SK_E_OK;
}


// -------------------------------------------------
//   Retrieve a message
// -------------------------------------------------

SK_ER SK_GetMessage(SK_UB id,SK_UB *resid,SK_UB *cmd,SK_VP *item) {
	T_SK_MailboxList *list;

	RF_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
	
		if (list->m_nMBXID == id) {
			SK_UW diff;
			diff = (SK_UW)(gnSK_SystemTick - list->m_nTimeStamp);

			*cmd	= list->m_nCommand;
			*item	= list->m_pItem;
			*resid	= list->m_nResMBXID;
			
			if( list->m_nSlot == 0xFFFF ){
				if( list->m_nLatency == 0 ){
					SK_RemoveFromList(list);
					SK_ReleaseLinkedList(list);
		
					RF_UNLOCK();
					return SK_E_OK;
				} else if( diff > list->m_nLatency ){
					if( list->m_nRepeat <= 1 ){
						SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
						SK_RemoveFromList(list);
						SK_ReleaseLinkedList(list);
					} else {
						SK_RemoveFromList(list);
						SK_AddToTail(list);
						list->m_nRepeat--;
						list->m_nTimeStamp = gnSK_SystemTick;
					}
					
					RF_UNLOCK();
					return SK_E_OK;
				}
			} else {
				//if( list->m_nSlot == SSMac_GetCurrentSlot() ){
				if( SSMac_IsJustOn(list->m_nSlot) == TRUE ){
					if( list->m_nRepeat <= 1 ){
						SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
						SK_RemoveFromList(list);
						SK_ReleaseLinkedList(list);
					} else {
						SK_RemoveFromList(list);
						SK_AddToTail(list);
						list->m_nRepeat--;
						list->m_nTimeStamp = gnSK_SystemTick;
					}		
					
					RF_UNLOCK();
					return SK_E_OK;
				}
			}
		}
		list = list->m_pNext;
	}

	*cmd	= 0;
	*item	= NULL;
	*resid	= 0;

	RF_UNLOCK();
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//   Retrieve a message with cmd id
// -------------------------------------------------

SK_ER SK_GetMessageByCmd(SK_UB id,SK_UB *resid,SK_UB cmd,SK_VP *item) {
	T_SK_MailboxList *list;

	RF_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
		if ((list->m_nMBXID == id) && (list->m_nCommand == cmd)) {
			SK_UW diff;
			diff = (SK_UW)(gnSK_SystemTick - list->m_nTimeStamp);
		
			*item	= list->m_pItem;
			*resid	= list->m_nResMBXID;
			
			if( list->m_nSlot == 0xFFFF ){
				if( list->m_nLatency == 0 ){
					SK_RemoveFromList(list);
					SK_ReleaseLinkedList(list);
		
					RF_UNLOCK();
					return SK_E_OK;
				} else if( diff > list->m_nLatency ){
					if( list->m_nRepeat <= 1 ){
						SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
						SK_RemoveFromList(list);
						SK_ReleaseLinkedList(list);
					} else {
						SK_RemoveFromList(list);
						SK_AddToTail(list);
						list->m_nRepeat--;
						list->m_nTimeStamp = gnSK_SystemTick;
					}
					
					RF_UNLOCK();
					return SK_E_OK;
				}
			} else {
				//if( list->m_nSlot == SSMac_GetCurrentSlot() ){
				if( SSMac_IsJustOn(list->m_nSlot) == TRUE ){
					if( list->m_nRepeat <= 1 ){
						SK_MemoryFlag(list->m_pItem, 0x02, FLAG_SET);
						SK_RemoveFromList(list);
						SK_ReleaseLinkedList(list);
					} else {
						SK_RemoveFromList(list);
						SK_AddToTail(list);
						list->m_nRepeat--;
						list->m_nTimeStamp = gnSK_SystemTick;
					}		
					
					RF_UNLOCK();
					return SK_E_OK;
				}				
			}
		}
		list = list->m_pNext;
	}

	*item	= NULL;
	
	RF_UNLOCK();
			
	return SK_E_TMOUT;
}



// -------------------------------------------------
//   Free linked list for memory block
// -------------------------------------------------

T_SK_MailboxList* SK_AllocLinkedList() {
	SK_UB		i;

	for(i=0;i<SK_MBX_NUM;i++) {
		if (gaSK_MessageList[i].m_pItem == NULL) {
			return &gaSK_MessageList[i];
		}
	}
	return NULL;
}


void SK_ReleaseLinkedList(T_SK_MailboxList* list) {
	list->m_pItem = NULL;
	list->m_nLatency = 0;
	list->m_nRepeat = 0;
}


// -------------------------------------------------
//   Allocate small memory block
// -------------------------------------------------

SK_ER SK_AllocCommandMemory(SK_VP *item) {
	SK_UB		i;

	RF_LOCK();
	
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		if (gaSK_CmdMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_CmdMemory[(SK_UH)SK_CMD_MEMBLOCK_SIZE*(SK_UH)i]);
			gaSK_CmdMemoryFlag[i] = 1;
			
			RF_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;
	
	SS_STATS(mem.err);
	
	RF_UNLOCK();
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//    Allocate large memory block
// -------------------------------------------------

SK_ER SK_AllocDataMemory(SK_VP *item) {
	SK_UB		i;

	RF_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_DataMemory[(SK_UH)SK_DATA_MEMBLOCK_SIZE*(SK_UH)i]);
			gaSK_DataMemoryFlag[i] = 1;	
			gaSK_DataMemoryTag[i] = 255;
			
			RF_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;

	SS_STATS(mem.err);
	
	RF_UNLOCK();	
	
	return SK_E_TMOUT;
}


SK_ER SK_AllocDataMemoryWith(SK_VP *item, SK_UB tag) {
	SK_UB		i;

	RF_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			*item = (SK_VP)(&gaSK_DataMemory[(SK_UH)SK_DATA_MEMBLOCK_SIZE*(SK_UH)i]);
			gaSK_DataMemoryFlag[i] = 1;	
			gaSK_DataMemoryTag[i] = tag;
			
			RF_UNLOCK();
			return SK_E_OK;
		}
	}
	*item = NULL;

	SS_STATS(mem.err);
	
	RF_UNLOCK();	
	
	return SK_E_TMOUT;
}


// -------------------------------------------------
//   Free meomry block (large, small, big)
// -------------------------------------------------

SK_ER SK_FreeMemory(SK_VP item) {
	SK_UW		i;
	
	RF_LOCK();

	if (item == NULL) {
		RF_UNLOCK();
		return SK_E_ER;
	}
	
	i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_CmdMemory))) / SK_CMD_MEMBLOCK_SIZE);
	if ( (item >= (SK_VP)(gaSK_CmdMemory)) && (i < SK_CMD_MEMBLOCK_NUM) ) {
		if( gaSK_CmdMemoryFlag[i] <= 2 ){
			gaSK_CmdMemoryFlag[i] = 0;
			RF_UNLOCK();
			return SK_E_OK;
		}
	} else {
		i = (SK_UW)((SK_UW)(((SK_VP_I)item - (SK_VP_I)(gaSK_DataMemory))) / SK_DATA_MEMBLOCK_SIZE);
		if ( (item >= (SK_VP)(gaSK_DataMemory)) && (i < SK_DATA_MEMBLOCK_NUM) ) {
			if( gaSK_DataMemoryFlag[i] <= 2 ){
				gaSK_DataMemoryFlag[i] = 0;
				gaSK_DataMemoryTag[i] = 0; //20130521 add
				RF_UNLOCK();
				return SK_E_OK;
			}
		} 
	}
	
	RF_UNLOCK();
	
	return SK_E_ER;	
}


SK_ER SK_FreeMemoryTag(SK_UB tag) {
	SK_ER err;
	
	RF_LOCK();
	
	err = SK_FreeMemoryTagNoLock(tag);

	RF_UNLOCK();
	
	return err;
}


SK_ER SK_FreeMemoryTagNoLock(SK_UB tag) {
	SK_UB		i;

	for( i = 0; i < SK_DATA_MEMBLOCK_NUM; i++ ){
		//20130521 mod
		if( gaSK_DataMemoryTag[i] == tag && gaSK_DataMemoryFlag[i] != 0 ){
			gaSK_DataMemoryTag[i] = 0;
			gaSK_DataMemoryFlag[i] = 0; //20130521 add

			return SK_E_OK;
		}
	}

	return SK_E_ER;
}


SK_UB SK_CountFreeDataMemory(){
	SK_UB		i, cnt;
	cnt = 0;
	
	RF_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryFlag[i]==0) {
			cnt++;
		}
	}
	
	RF_UNLOCK();
	
	return cnt;	
}


SK_UB SK_CountFreeMemoryTag(SK_UB tag){
	SK_UB		i, cnt;
	cnt = 0;
	
	RF_LOCK();
	
	for(i=0;i<SK_DATA_MEMBLOCK_NUM;i++) {
		if (gaSK_DataMemoryTag[i]==tag) {
			cnt++;
		}
	}
	
	RF_UNLOCK();
	
	return cnt;	
}


SK_UB SK_CountFreeCommandMemory(){
	SK_UB		i, cnt;
	cnt = 0;
	
	RF_LOCK();
	
	for(i=0;i<SK_CMD_MEMBLOCK_NUM;i++) {
		if (gaSK_CmdMemoryFlag[i]==0) {
			cnt++;
		}
	}
	
	RF_UNLOCK();
	
	return cnt;	
}


// -------------------------------------------------
//   Stop delayed execution of a post message
// -------------------------------------------------

SK_ER SK_StopMessage(SK_VP item){
	T_SK_MailboxList *list;

	RF_LOCK();
	
	list = gpstSK_MessageStart;
	while(list != NULL) {
		if (list->m_pItem == item) {
			SK_MemoryFlag(item, 0, FLAG_SET); //2nd bit and 1st bit should be clear

			SK_RemoveFromList(list);
			SK_ReleaseLinkedList(list);
			
			RF_UNLOCK();
			return SK_E_OK;
			
		}
		list = list->m_pNext;
	}
	
	RF_UNLOCK();
	
	return SK_E_TMOUT;
}


