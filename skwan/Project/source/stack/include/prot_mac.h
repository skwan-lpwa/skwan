/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.
    
    All rights reserved. This program and the accompanying materials
    are made available under the terms of the Eclipse Public License v1.0
    which accompanies this distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html
	
    Contributors:
        Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __prot_mac_h__
#define __prot_mac_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include    "skyley_stack.h"


// -------------------------------------------------
//   Consts
// -------------------------------------------------
#define		SK_AMAXMPDUOVERHEAD					0
#define		SK_AMAXMACPAYLOADSIZE				( SK_AMAXPHYPACKETSIZE - SK_AMAXMPDUOVERHEAD )
#define		SK_ABASESLOTDURATION				60 /* The number of symbols forming a superframe slot when the superframe order is equal to 0. */
#define		SK_ANUMSUPERFRAMESLOTS				16 /* The number of slots contained in any superframe. */
#define		SK_ABASESUPERFRAMEDURATION			( SK_ABASESLOTDURATION * SK_ANUMSUPERFRAMESLOTS )	/* The number of symbols forming a superframe when the superframe order is equal to 0. */
#define     SK_AMAXBEACONOVERHEAD				0//75   /* The maximum number of octets added by the MAC sublayer to the payload of its beacon frame. */
#define     SK_AMAXBEACONPAYLOADLENGTH			( 32 )    /* The maximum size, in octets, of a beacon payload. */
#define		SK_AUNITBACKOFFPERIOD				(SK_ATURNAROUNDTIME + SK_ACCATIME)
#define		SK_ARESPONSEWAITTIME				( 32 * SK_ABASESUPERFRAMEDURATION )	/* The maximum number of symbols a device shall wait for a response command to be available following a request command. */
#define		SK_AMINLIFSPERIOD					40		/* The minimum number of symbols forming a long interframe spacing (LIFS) period. */
#define		SK_AMINSIFSPERIOD					12		/* The minimum number of symbols forming a SIFS period. */
#define		SK_AMAXBE							5		/* The maximum value of the backoff exponent in the CSMA-CA algorithm. */
#define		SK_AMAXFRAMERESPONSETIME			1220	/* The maximum number of CAP symbols in a beaconenabled PAN, or symbols in a nonbeacon-enabled PAN, to wait for a frame intended as a response to a data request frame. */
#define		SK_AMAXFRAMERETRIES					3		/* The maximum number of retries allowed after a transmission failure. */
#define 	SK_AMAXEBIELENGTH					32

#define		SK_MAC_ACK_WAIT_DURATION			(SK_AUNITBACKOFFPERIOD + SK_ATURNAROUNDTIME + SK_PHY_SHR_DURATION + (9*SK_PHY_SYMBOLS_PER_OCTET))

// Possible values of the destination and source addressing mode subfields
#define		SK_ADDRMODE_NOTPRESENT				0		// PAN identifier and address field are not present.
#define		SK_ADDRMODE_SIMPLE					1		// 802.15.4e Address field contains an 8-bit simple address.
#define		SK_ADDRMODE_SHORT					2		// Address field contains a 16 bit short address.
#define		SK_ADDRMODE_EXTENDED				3		// Address field contains a 64 bit extended address.

// Values of the frame type subfield
#define		SK_FRAMETYPE_BEACON					0		// Beacon
#define		SK_FRAMETYPE_DATA					1		// Data
#define		SK_FRAMETYPE_ACK					2		// Acknowledgment
#define		SK_FRAMETYPE_MACCMD					3		// MAC command
#define		SK_FRAMETYPE_LOW_LATENCY			4		// Low Latency 802.15.4e
#define		SK_FRAMETYPE_MULTI					5		// Multipurpose 802.15.4e

// tx options
#define		SK_TXOPTIONS_ACK					0x01	// Acknowledged transmission
#define		SK_TXOPTIONS_GTS					0x02	// GTS transmission
#define		SK_TXOPTIONS_INDIRECT				0x04	// Indirect transmission
#define		SK_TXOPTIONS_SECURITY				0x08	// Security enabled transmission

//Frame version
#define		SK_FRAME_VERSION_2003				0		//Frame compatible with IEEE Std 802.15.4-2003
#define		SK_FRAME_VERSION_2006				1		//Frames introduced with IEEE Std 802.15.4-2006
#define		SK_FRAME_VERSION_ENH				2		//Frame compatible with IEEE Std 802.15.4

// MAC IBs ID
#define		SK_MACRITPERIOD						0x8D
#define		SK_MACRITDATAWAITDURATION			0x8E
#define		SK_MACRITTXWAITDURATION				0x8F

// MAC Command Identifier
#define		SK_ID_ASSOCIATION_REQUEST			0x01
#define		SK_ID_ASSOCIATION_RESPONSE			0x02
#define		SK_ID_DISASSOCIATION_NOTIFICATION	0x03
#define		SK_ID_DATA_REQUEST					0x04
#define		SK_ID_PAN_ID_CONFLICT_NOTIFICATION	0x05
#define		SK_ID_ORPHAN_NOTIFICATION			0x06
#define		SK_ID_BEACON_REQUEST				0x07
#define		SK_ID_COORDINATOR_REALIGNMENT		0x08
#define		SK_ID_GTS_REQUEST					0x09
#define 	SK_ID_LERIT_DATA_REQUEST			0x20	//802.15.4e

// PIBs
#define		SK_MACACKWAITDURATION				0x40
#define		SK_MACASSOCIATIONPERMIT				0x41
#define		SK_MACAUTOREQUEST					0x42
#define		SK_MACBATTLIFEEXT					0x43
#define		SK_MACBATTLIFEEXTPERIODS			0x44
#define		SK_MACBEACONPAYLOAD					0x45
#define		SK_MACBEACONPAYLOADLENGTH			0x46
#define		SK_MACBEACONORDER					0x47
#define		SK_MACBEACONTXTIME					0x48
#define		SK_MACBSN							0x49
#define		SK_MACCOORDEXTENDEDADDRESS			0x4A
#define		SK_MACCOORDSHORTADDRESS				0x4B
#define		SK_MACDSN							0x4C
#define		SK_MACGTSPERMIT						0x4D
#define		SK_MACMAXCSMABACKOFFS				0x4E
#define		SK_MACMINBE							0x4F
#define		SK_MACPANID							0x50
#define		SK_MACPROMISCUOUSMODE				0x51
#define		SK_MACRXONWHENIDLE					0x52
#define		SK_MACSHORTADDRESS					0x53
#define		SK_MACSUPERFRAMEORDER				0x54
#define		SK_MACTRANSACTIONPERSISTENCETIME	0x55

// results
#define		SK_MAC_SUCCESS						0x00 /* The requested operation was completed successfully. For a transmission request, this value indicates a successful transmission. */
#define		SK_MAC_BEACON_LOSS					0xE0 /* The beacon was lost following a synchronization request. */
#define		SK_MAC_CHANNEL_ACCESS_FAILURE		0xE1 /* A transmission could not take place due to activity on the channel, i.e., the CSMA-CA mechanism has failed. */
#define		SK_MAC_DENIED						0xE2 /* The GTS request has been denied by the PAN coordinator. */
#define		SK_MAC_DISABLE_TRX_FAILURE			0xE3 /* The attempt to disable the transceiver has failed. */
#define		SK_MAC_FAILED_SECURITY_CHECK		0xE4 /* The received frame induces a failed security check according to the security suite. */
#define		SK_MAC_FRAME_TOO_LONG				0xE5 /* The frame resulting from secure processing has a length that is greater than aMACMaxFrameSize. */
#define		SK_MAC_INVALID_GTS					0xE6 /* The requested GTS transmission failed because the specified GTS either did not have a transmit GTS direction or was not defined. */
#define		SK_MAC_INVALID_HANDLE				0xE7 /* A request to purge an MSDU from the transaction queue was made using an MSDU handle that was not found in the transaction table. */
#define		SK_MAC_INVALID_PARAMETER			0xE8 /* A parameter in the primitive is out of the valid range. */
#define		SK_MAC_NO_ACK						0xE9 /* No acknowledgment was received after aMaxFrameRetries. */
#define		SK_MAC_NO_BEACON					0xEA /* A scan operation failed to find any network beacons. */
#define		SK_MAC_NO_DATA						0xEB /* No response data were available following a request. */
#define		SK_MAC_NO_SHORT_ADDRESS				0xEC /* The operation failed because a short address was not allocated. */
#define		SK_MAC_OUT_OF_CAP					0xED /* A receiver enable request was unsuccessful because it could not be completed within the CAP. */
#define		SK_MAC_PAN_ID_CONFLICT				0xEE /* A PAN identifier conflict has been detected and communicated to the PAN coordinator. */
#define		SK_MAC_REALIGNMENT					0xEF /* A coordinator realignment command has been received. */
#define		SK_MAC_TRANSACTION_EXPIRED			0xF0 /* The transaction has expired and its information discarded. */
#define		SK_MAC_TRANSACTION_OVERFLOW			0xF1 /* There is no capacity to store the transaction. */
#define		SK_MAC_TX_ACTIVE					0xF2 /* The transceiver was in the transmitter enabled state when the receiver was requested to be enabled. */
#define		SK_MAC_UNAVAILABLE_KEY				0xF3 /* The appropriate key is not available in the ACL. */
#define		SK_MAC_UNSUPPORTED_ATTRIBUTE		0xF4 /* A SET/GET request was issued with the identifier of a PIB attribute that is not supported. */
#define		SK_MAC_FRAME_PENDING				0xFE

#define		SK_MAC_LIMIT_REACHED				0xFA
#define		SK_MAC_SCAN_IN_PROGRESS				0xFC

#define		SK_MAC_RIT_TERMINATED				0xB2
#define		SK_MAC_RIT_TRANSMIT					0xB3

#define		SK_SCAN_ED							0x00	/* ED scan */
#define		SK_SCAN_ACTIVE						0x01	/* Active scan */
#define		SK_SCAN_PASSIVE						0x02	/* Passive scan */
#define		SK_SCAN_ORPHAN						0x03	/* Orphan scan */
#define		SK_SCAN_ACTIVE_ENH					0x07	/* Enhanced active scan 802.15.4e */

#define		SK_MAC_ASSOCSTAT_SUCCESS			0x00	/* Association successful */
#define		SK_MAC_ASSOCSTAT_PAN_CAPACITY		0x01	/* PAN at capacity */
#define		SK_MAC_ASSOCSTAT_PAN_DENIED			0x02	/* PAN access denied */

#define		SK_MAC_COORD_WISHES_DEVICE_TO_LEAVE 0x01
#define		SK_MAC_DEVICE_WISHES_TO_LEAVE		0x02

#define 	SK_MAC_NOT_ALLOCATED_SHORT_ADDR		0xfffe

//convertion duration <-> msec
//note: MSECPERSYMBOL is depending on frequency
//this should be changed in Sub GHz band, refer to 802.15.4d, g
//#define		MSECPERSYMBOL					(0.016)

//for 920/950MHz 100K bps. symbol rate = bit rate
//802.15.4g Table 0 MR-FSK symbol duration used for MAC and PHY timing parameters
#define		MSECPERSYMBOL						(0.010)
#define		SYMBOLSPERSEC						(100000)

#define		DUR_TO_MSEC(X) 						( (SK_UW)((SK_UW)X * (SK_UW)SK_ABASESUPERFRAMEDURATION * MSECPERSYMBOL) )
#define		MSEC_TO_DUR(X) 						( (SK_UW)((SK_UW)X / (SK_UW)((SK_UW)SK_ABASESUPERFRAMEDURATION * MSECPERSYMBOL)) )
#define		IS_BROADCAST(X) 					( X.m_AddrMode == SK_ADDRMODE_SHORT && X.Body.Short.m_Word == 0xFFFF )
#define		SK_TICKPERSEC						1000
#define 	SK_TICKMSEC							1

#define		SK_SCAN_CHANNEL_MIN					1
#define		SK_SCAN_CHANNEL_MAX					28
#define		SK_SCAN_CHANNEL_OFFSET				32
#define		SK_CHANNEL_NUM 						(SK_SCAN_CHANNEL_MAX-SK_SCAN_CHANNEL_MIN+1)

// -------------------------------------------------
//   Table capacity constants
// -------------------------------------------------
//Pending buffer size
#define		SK_PENDINGBUFSIZE					6
#define 	SK_MAC_MAX_PAN_DESC 				1

#define		SK_MAX_IE_SIZE						2


// -------------------------------------------------
//   MPDU Parameters
// -------------------------------------------------
typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_FrameType:3;
		SK_UB			m_SecurityEnabled:1;
		SK_UB			m_FramePending:1;
		SK_UB			m_AckRequest:1;
		SK_UB			m_PanIDCompression:1;
		SK_UB			reserved1:1;
		SK_UB			m_SeqNumSuppression:1;  //802.15.4e
		SK_UB			m_IEListPresent:1; 		//802.15.4e
		SK_UB			m_DstAddrMode:2;
		SK_UB			m_FrameVersion:2;
		SK_UB			m_SrcAddrMode:2;
	} 				Field;
} SK_MPDU_FRAMECONTROL;


typedef union {
    SK_UB           Raw[2];
    struct {
        SK_UB           m_BeaconOrder:4;
        SK_UB           m_SuperframeOrder:4;
        SK_UB           m_FinalCapSlot:4;
        SK_UB           m_BatteryLifeExt:1;
        SK_UB           reserved1:1;
        SK_UB           m_PanCoordinator:1;
        SK_UB           m_AssociationPermit:1;
    }               Field;
} SK_MPDU_SUPERFRAMESPEC;

typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_GTSStartingSlot:4;
        SK_UB           m_GTSLength:4;
    } Field;
} GTS_PARAM;
    
typedef struct {
    SK_UH           m_DeviceShortAddress;
    GTS_PARAM       m_GTSParam;
    SK_UB           m_Direction;
    SK_UH           __m_ExpireFrameCounter;
} SK_GTS_DESCRIPTOR;

typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_GtsDescriptorCount:3;
        SK_UB           reserved1:4;
        SK_UB           m_GtsPermit:1;
    }               Field;
} SK_MPDU_GTSSPEC;

typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_GtsDirectionsMask:7;
        SK_UB           reserved1:1;
    }               Field;
} SK_MPDU_GTSDIRECTIONS;

typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_GTSLength:4;
        SK_UB           m_GTSDirection:1;
        SK_UB           m_CharacteristicsType:1;
        SK_UB           reserved:2;
    }               Field;
} SK_GTS_CHARACTERISTICS;

typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_NumOfShortAddrPending:3;
        SK_UB           reserved1:1;
        SK_UB           m_NumOfExtendedAddrPending:3;
        SK_UB           reserved2:1;
    }               Field;
} SK_MPDU_PENDINGSPEC;


// Capability Informations
typedef union {
    SK_UB           Raw[1];
    struct {
        SK_UB           m_AlternatePanCoordinator:1;    /* shall be set to 1 if the device is capable of becoming a PAN coordinator.*/
        SK_UB           m_DeviceType:1;                 /* shall be set to 1 if the device is an FFD. */
        SK_UB           m_PowerSource:1;                /* shall be set to 1 if the device is receiving power from the alternating current mains. */
        SK_UB           m_ReceiverOnWhenIdle:1;         /* shall be set to 1 if the device does not disable its receiver to conserve power during idle periods. */
        SK_UB           reserved:2;
        SK_UB           m_SecurityCapability:1;         /* shall be set to 1 if the device is capable of sending and receiving MAC frames secured using the security suite */
        SK_UB           m_AllocateAddress:1;
        /* shall be set to 1 if the device wishes the coordinator to allocate a short address as
        a result of the association procedure. If this subfield is set to 0, the special short
        address of 0 x fffe shall be allocated to the device and returned through the association response command.
        In this case, the device shall communicate on the PAN using only its 64 bit extended address. */
    }               Field;
} SK_CAPABILITYINFO;

/*
0x3F
111111
*/

/*
Element ID = 0x7E (0b01111110)
         0000000
 01111110
0

*/
typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_Length:7;
		SK_UB			m_ElementID1:1;
		SK_UB			m_ElementID2:7;
		SK_UB			m_Type:1;
	} Field;
} SK_HEADER_IE;


/*
Length: 10 bits (00000001010b)
Group ID: MLME (0001b)
Type: payload (1b)

        00001010
     000 
 0001
1
*/
typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_Length1:8; //lower 8bit
		SK_UB			m_Length2:3; //higher 3bit
		SK_UB			m_GroupID:4;
		SK_UB			m_Type:1;
	} Field;
} SK_PAYLOAD_IE;

typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_Length:8;
		SK_UB			m_SubID:7;
		SK_UB			m_Type:1;
	} Field;
} SK_MLME_SUB_IE;



typedef struct {
	union {
		SK_UB			Raw[1];
		struct {
			SK_UB			m_PermitJoiningOn:1;
			SK_UB			m_IncludeLinkQualityFilter:1;
			SK_UB			m_IncludePercentFilter:1;
			SK_UB			m_NumOfPIBIdList:2;
			SK_UB			reserved:3;
		} Field;
	} Byte;
	SK_UB			m_LinkQuality;
	SK_UB			m_PercentFilter;
} SK_EB_FILTER;


typedef struct {
	union {
		SK_UB			m_Raw[3];
		struct {
			SK_UB			m_BeaconOrder:4;
			SK_UB			m_SuperframeOrder:4;
			SK_UB			m_FinalCapSlot:4;
			SK_UB			m_EnhancedBeaconOrder:4;
			SK_UB			m_OffsetTimeSlot:4;
			SK_UB			m_CapBackoffOffset:4;
		} Field;
	} Byte;

	SK_UW m_ChannelPage;
	SK_UH m_NBPANEnhancedBeaconOrder;
} SK_COEXIST_SPEC_IE;

// -------------------------------------------------
//   MCPS-DATA
// -------------------------------------------------

//MCPS-DATA.request
typedef struct {
    SK_UH               m_SrcPanID;
    SK_ADDRESS    		m_SrcAddr;
    SK_UH               m_DstPanID;
    SK_ADDRESS    		m_DstAddr;
    SK_UH               m_MsduLength;
    SK_UB               m_Msdu[SK_AMAXMACPAYLOADSIZE];
    SK_UB               m_MsduHandle;
    SK_UB               m_TxOptions; 
} SK_MCPS_DATA_REQUEST;

//MCPS-DATA.confirm
typedef struct {
    SK_UB               m_MsduHandle;  
    SK_UB               m_Status;
} SK_MCPS_DATA_CONFIRM;

//MCPS-DATA.indication
#if 0
typedef struct {
    SK_UB               m_FrameType; 
    SK_UH               m_SrcPanID; 
    SK_ADDRESS			m_SrcAddr;
    SK_UH               m_DstPanID;
    SK_ADDRESS			m_DstAddr; 
    SK_UH               m_MsduLength; 
    SK_UB               m_Msdu[SK_AMAXMACPAYLOADSIZE];
    SK_UB               m_MpduLinkQuality; 
    SK_BOOL             m_SecurityUse; 
    SK_UB               m_AclEntry;
    SK_UB               __m_RxOptions;
    SK_UB               __m_MacDSN;
    SK_UB               m_Rssi;
    SK_UB				__m_SecurityLevel;
    SK_UB				__m_KeyIndex;
    SK_UB				__m_KeyIdMode;
    SK_UW				m_TimeStamp;
    SK_UH				m_RecvSlot;
    SK_UB				m_HeaderIELength;
    SK_UB				m_HeaderIE[SK_MAX_IE_SIZE];
    SK_UH				m_PayloadIELength;
    SK_UB				m_PayloadIE[SK_MAX_IE_SIZE];
} SK_MCPS_DATA_INDICATION;
#endif
typedef struct {
    SK_UB               m_FrameType; 
    SK_UH               m_SrcPanID; 
    SK_ADDRESS			m_SrcAddr;
    SK_UH               m_DstPanID;
    SK_ADDRESS			m_DstAddr; 
    SK_UH               m_MsduLength; 
    SK_UB               m_Msdu[SK_AMAXMACPAYLOADSIZE];
    SK_UB               m_Rssi;
    SK_UW				m_TimeStamp;
    SK_UH				m_RecvSlot;
} SK_MCPS_DATA_INDICATION;


#define BEACON_PAYLOAD_BUFF_SIZE 15
//PAN DESCRIPTOR
typedef struct {
    SK_UH               m_CoordPanID; 
    SK_ADDRESS			m_CoordAddress;
    SK_UB               m_LogicalChannel;
    SK_UB				m_ChannelPage;
    SK_UH               m_SuperFrameSpec;
    SK_BOOL             m_GTSPermit;
    SK_UB               m_LinkQuality;
    SK_BOOL             m_SecurityUse;
    SK_UB				m_AclEntry; 
   	SK_BOOL				m_SecurityFailure;
    SK_UB               m_BeaconPayload[ BEACON_PAYLOAD_BUFF_SIZE ];
} SK_PAN_DESCRIPTOR;


// -------------------------------------------------
//   MLME-BEACON-NOTIFY
// -------------------------------------------------

//MLME-BEACON-NOTIFY.indication
typedef struct {
    SK_UB               m_BSN; 
    SK_PAN_DESCRIPTOR   m_PanDescriptor;
    SK_UB               m_PendAddrSpec;
    SK_UB               m_SduLength;
    SK_UB				__m_IsEnhanced;
    SK_ADDRESS			m_pAddrList[7];
    SK_UB               m_pSdu[SK_AMAXBEACONPAYLOADLENGTH];
} SK_MLME_BEACON_NOTIFY_INDICATION;


// -------------------------------------------------
//   MLME-SCAN
// -------------------------------------------------

//MLME-SCAN.request
typedef struct {
	SK_UB               m_ScanType;
	SK_UW               m_ScanChannels;
	SK_UB               m_ScanDuration;
	SK_UB				m_ChannelPage;
	SK_UB				m_HeaderIE[ SK_AMAXEBIELENGTH ];
	SK_UB				m_HeaderIELength;
	SK_UB				m_PayloadIE[ SK_AMAXEBIELENGTH ];
	SK_UB				m_PayloadIELength;
} SK_MLME_SCAN_REQUEST;

//MLME-SCAN.confirm
typedef struct {
	SK_UB               m_Status;
	SK_UB               m_ScanType;
	SK_UB				m_ChanngelPage;
	SK_UW               m_UnscannedChannels;
	SK_UB               m_ResultListSize; 
	SK_UB               *m_EnergyDetecList;
	SK_PAN_DESCRIPTOR   *m_PanDescriptorList;
} SK_MLME_SCAN_CONFIRM;



// -------------------------------------------------
//   MLME-COMM-STATUS
// -------------------------------------------------

//MLME-COMM-STATUS.indication
typedef struct {
    SK_UH               m_PanID;
    SK_ADDRESS			m_SrcAddr;
    SK_ADDRESS			m_DstAddr;
    SK_UB               m_Status;
} SK_MLME_COMM_STATUS_INDICATION;


// -------------------------------------------------
//   MLME-SYNC-LOSS
// -------------------------------------------------

//MLME-SYNC-LOSS.indication
typedef struct {
	SK_UB				m_LossReason;
	SK_UH				m_PANId;
	SK_UB				m_LogicalChannel;
	SK_UB				m_ChannelPage;
} SK_MLME_SYNC_LOSS_INDICATION;


// -------------------------------------------------
//   MLME-POLL
// -------------------------------------------------

//MLME-POLL.request
typedef struct {
    SK_UH               m_CoordPanID;
    SK_ADDRESS			m_CoordAddress;
} SK_MLME_POLL_REQUEST;

//MLME-POLL.confirm
typedef struct {
    SK_UB               m_Status; 
} SK_MLME_POLL_CONFIRM;


typedef struct {
    SK_ADDRESS				m_DeviceAddress;
    SK_PD_DATA_REQUEST   	*m_Packet;
    SK_UW                   m_TimeStamp;
	SK_UB					m_Handle;
	SK_UB					m_CommandType;
} SK_MAC_PENDING_INFO;


// -------------------------------------------------
//   MLME-START
// -------------------------------------------------

//MLME-START.request
typedef struct {
	SK_UH				m_PanID;				
	SK_UB				m_LogicalChannel;		
	SK_UB				m_BeaconOrder;			
	SK_UB				m_SuperframeOrder;		
	SK_BOOL				m_PanCoordinator;		
	SK_BOOL				m_BatteryLifeExtension;	
	SK_BOOL				m_CoordRealignment;		
	SK_BOOL				m_SecurityEnable;		
} SK_MLME_START_REQUEST;

//MLME-START.confirm
typedef struct {
	SK_UB				m_Status;			
} SK_MLME_START_CONFIRM;


// -------------------------------------------------
//   MLME-ASSOCIATE
// -------------------------------------------------

//MLME-ASSOCIATE.request
typedef struct {
	SK_UB				m_LogicalChannel;		
	SK_CAPABILITYINFO	m_CapabilityInformation;
	SK_BOOL				m_SecurityEnable;		
	SK_UH				m_CoordPanId;			
	SK_UB				m_CoordAddrMode;		
	SK_ADDRESS			m_CoordAddress;		 
} SK_MLME_ASSOCIATE_REQUEST;

//MLME-ASSOCIATE.indication
typedef struct {
	SK_ADDRESS			m_DeviceAddress;			/* IEEE 64bit address */
	SK_UB				m_CapabilityInfomation;	
	SK_BOOL				m_SecurityUse;			
	SK_UB				m_AclEntry;			
} SK_MLME_ASSOCIATE_INDICATION;

//MLME-ASSOCIATE.response
typedef struct {
	SK_ADDRESS			m_DeviceAddress;			/* IEEE 64bit address */
	SK_UH				m_AssocShortAddress;	
	SK_UB				m_Status;				
	SK_BOOL				m_SecurityEnable;		
} SK_MLME_ASSOCIATE_RESPONSE;

//MLME-ASSOCIATE.confirm
typedef struct {
	SK_UH				m_AssocShortAddress;	
	SK_UB				m_Status;				
} SK_MLME_ASSOCIATE_CONFIRM;



// -------------------------------------------------
//   MLME-DISASSOCIATE
// -------------------------------------------------

//MLME-DISASSOCIATE.request
typedef struct {		
	SK_ADDRESS			m_DeviceAddress;			/* IEEE 64bit address */
	SK_UB				m_DisassociateReason;		/* (COORD_WISHES_DEVICE_TO_LEAVE | DEVICE_WISHES_TO_LEAVE) */
	SK_BOOL				m_SecurityEnable;
	SK_UH				m_DevicePANId; 				//802.15.4-2006
	SK_BOOL				__m_Pending;				//aka TxIndirect in 802.15.4-2006
} SK_MLME_DISASSOCIATE_REQUEST;

//MLME-DISASSOCIATE.indication
typedef struct {
	SK_ADDRESS			m_DeviceAddress	;			/* IEEE 64bit address */
	SK_UB				m_DisassociateReason;		/* (COORD_WISHES_DEVICE_TO_LEAVE | DEVICE_WISHES_TO_LEAVE) */
	SK_BOOL				m_SecurityUse;			
	SK_UB				m_AclEntry;			
} SK_MLME_DISASSOCIATE_INDICATION;

//MLME-DISASSOCIATE.confirm
typedef struct {
	SK_UB				m_Status;
	SK_ADDRESS			m_DeviceAddress;			//802.15.4-2006
	SK_UB				m_DeviceAddressMode;		//802.15.4-2006
	SK_UH				m_DevicePANId;				//802.15.4-2006
} SK_MLME_DISASSOCIATE_CONFIRM;


// -------------------------------------------------
//   MLME-ORPHAN
// -------------------------------------------------

//MLME-ORPHAN.indication
typedef struct {
	SK_ADDRESS			m_OrphanAddress;		
	SK_BOOL				m_SecurityUse;			
	SK_UB				m_AclEntry;				
} SK_MLME_ORPHAN_INDICATION;

//MLME-ORPHAN.response
typedef struct {
	SK_ADDRESS			m_OrphanAddress;		
	SK_UH				m_ShortAddress;			
	SK_BOOL				m_AssociatedMember;		
	SK_BOOL				m_SecurityEnable;		
} SK_MLME_ORPHAN_RESPONSE;


// -------------------------------------------------
//   Public variables
// -------------------------------------------------
extern SK_UB			gnMAC_LowerLayer;
extern SK_UB			gnMAC_UpperLayer;

extern SK_ADDRESS 		gnMAC_LongAddress;

extern SK_BOOL			gnMAC_AssociationPermit;
extern SK_BOOL			gnMAC_AutoRequest;
extern SK_BOOL			gnMAC_BattLifeExt;
extern SK_UB			gnMAC_BattLifeExtPeriods;	
extern SK_UB*			gnMAC_BeaconPayload; 	
extern SK_UB			gnMAC_BeaconPayloadLength;
extern SK_UB			gnMAC_BeaconOrder;
extern SK_UW			gnMAC_BeaconTxTime;
extern SK_UB			gnMAC_BSN;
extern SK_ADDRESS		gnMAC_CoordExtendedAddress;
extern SK_UH			gnMAC_CoordShortAddress;
extern SK_UB			gnMAC_CoordAddressMode;
extern SK_UB			gnMAC_DSN;
extern SK_BOOL			gnMAC_GTSPermit;
extern SK_UB			gnMAC_MaxCSMABackoffs;
extern SK_UB			gnMAC_MinBE;
extern SK_UB			gnMAC_MaxBE;
extern SK_UH			gnMAC_PANId;	
extern SK_UH			gnMAC_CoordPANId;
extern SK_BOOL			gnMAC_PromiscuousMode;
extern SK_BOOL			gnMAC_RxOnWhenIdle;
extern SK_UH			gnMAC_ShortAddress;
extern SK_UB			gnMAC_SuperframeOrder;
extern SK_UH			gnMAC_TransactionPersistenceTime;
extern SK_UH			gnMAC_MaxFrameTotalWaitTime;
extern SK_UB			gnMAC_MaxFrameRetries;
extern SK_UW			gnMAC_AckWaitDuration;
extern SK_UW			gnMAC_ResponseWaitTime;
extern SK_BOOL 			gnMAC_SecurityEnabled;
extern SK_UB			gnMAC_MinLIFSPeriod;
extern SK_UB			gnMAC_MinSIFSPeriod;
extern SK_UH			gnMAC_SyncSymbolOffset;
extern SK_BOOL			gnMAC_TimestampSupported;
extern SK_BOOL 			gnMAC_AssociatedPANCoord;

extern SK_UB			gnSK_SEC_CurrentSecurityLevel;
extern SK_UB 			gnSK_SEC_CurrentKeyIdMode;
extern SK_UB 			gnSK_SEC_CurrentKeySource[];
extern SK_UB 			gnSK_SEC_CurrentKeyIndex;

extern SK_UB			gnMAC_EnhancedBeaconOrder;
extern SK_BOOL			gnMAC_MPMIE;
extern SK_UH			gnMAC_NBPANENhancedBeaconOrder;
extern SK_UB			gnMAC_OffsetTimeSlot;

extern SK_BOOL			gnMAC_MetricsEnabled;
extern SK_UB			gnMAC_EBSN;
extern SK_BOOL			gnMAC_EBRPermitJoining;
extern SK_UB			gnMAC_EBRFilters;
extern SK_UB			gnMAC_EBRLinkQuality;
extern SK_UB			gnMAC_EBRPercentFilter;
extern SK_UB			gnMAC_EBRAttributeList;
extern SK_BOOL			gnMAC_BeaconAutoRespond;
extern SK_BOOL			gnMAC_UseEnhancedBeacon;
extern SK_UB			gnMAC_EBIEList;
extern SK_UB			gnMAC_EBIEListMode;
extern SK_BOOL			gnMAC_EBFilteringEnabled;
extern SK_UB			gnMAC_EBAutoSA;
extern SK_UB			gnMAC_EAckIElist;

extern SK_UB			gnMAC_SecureBroadcast;
extern SK_UB			gnMAC_StartedCoord;


// -------------------------------------------------
//   MAC Public functions
// -------------------------------------------------

void SK_MAC_Init(SK_UW macadr1, SK_UW macadr2);
void SK_MAC_Task(void);
void SK_MAC_MHR(
			SK_PD_DATA_REQUEST *PdReq,
			SK_UB frametype,
			SK_UB txoptions,
			SK_ADDRESS *srcaddr,
			SK_UH srcpan,
			SK_ADDRESS *dstaddr,
			SK_UH dstpan);
SK_UB SK_MAC_AnalyzeMHR(
			SK_UB *Psdu,
			SK_UB *frametype,
			SK_UB *txoptions,
			SK_ADDRESS *srcaddr,
			SK_UH *srcpan,
			SK_ADDRESS *dstaddr,
			SK_UH *dstpan
			);
void SK_MAC_MHR_Enh(
			SK_PD_DATA_REQUEST *PdReq,
			SK_UB frametype,
			SK_UB txoptions,
			SK_ADDRESS *srcaddr,
			SK_UH srcpan,
			SK_ADDRESS *dstaddr,
			SK_UH dstpan,
			SK_UB header_ie_len,
			SK_UB payload_ie_len);
SK_UB SK_MAC_AnalyzeMHR_Enh(
			SK_UB *Psdu,
			SK_UH PsduLen,
			SK_UB *frametype,
			SK_UB *txoptions,
			SK_ADDRESS *srcaddr,
			SK_UH *srcpan,
			SK_ADDRESS *dstaddr,
			SK_UH *dstpan,
			SK_UB *header_ie_len,
			SK_UB *header_ie_list,
			SK_UH *payload_ie_len,
			SK_UB *payload_ie_list
			);
void SK_MACLayer_Beacon(SK_PD_DATA_REQUEST *PdReq);
void SK_MAC_BeaconRequest(SK_PD_DATA_REQUEST *PdReq, SK_ADDRESS *Addr);
void SK_MACLayer_EnhancedBeacon(SK_PD_DATA_REQUEST *PdReq, SK_MCPS_DATA_INDICATION *MdInd, SK_UB sub);
SK_BOOL SK_MAC_IsAccept(SK_MPDU_FRAMECONTROL* fctrl, SK_MCPS_DATA_INDICATION *MdInd);
SK_BOOL MAC_AddPendingBuffer(SK_PD_DATA_REQUEST *pkt, SK_ADDRESS *addr,SK_UB handle);
void SK_MAC_BeaconRequest(SK_PD_DATA_REQUEST *PdReq, SK_ADDRESS *Addr);
void SK_MAC_EnhancedBeaconRequest(SK_PD_DATA_REQUEST *PdReq, SK_ADDRESS *Addr);
void SK_MAC_StartCoordinator(SK_UB pan_coord);
void SK_MAC_CopyAddress(SK_ADDRESS* dst, SK_ADDRESS* src);
SK_UB SK_MAC_SetAddress(SK_UB *ptr, SK_ADDRESS* adr);
void SK_MAC_GetExtAddress(SK_ADDRESS* dstaddr, SK_UB *ptr);
SK_UB SK_MAC_EncodeNetworkIdIE(SK_UB offset, SK_UB* buffer, SK_UB term);
SK_UB SK_MAC_EncodeEBFilterIE(SK_UB offset, SK_UB* buffer);
SK_UB SK_MAC_EncodeCoexistSpecIE(SK_UB offset, SK_UB* buffer);

void SK_MAC_SetLowerLayer(SK_UB layer);
void SK_MAC_SetUpperLayer(SK_UB layer);
SK_UB SK_MAC_GetLowerLayer(void);
SK_UB SK_MAC_GetUpperLayer(void);
SK_BOOL SK_MAC_Sleep(SK_BOOL force);
SK_BOOL SK_MAC_Wakeup(void);

// Setter function for PIBs
SK_BOOL SK_MAC_SetAckWaitDuration(SK_UW val);
SK_BOOL SK_MAC_SetAssociationPermit(SK_BOOL val);
SK_BOOL SK_MAC_SetAutoRequest(SK_BOOL val);
SK_BOOL SK_MAC_SetBattLifeExt(SK_BOOL val);
SK_BOOL SK_MAC_SetBattLifeExtPeriods(SK_UB val);
SK_BOOL SK_MAC_SetBeaconPayload(SK_UB *dat);
SK_BOOL SK_MAC_SetBeaconPayloadLength(SK_UB val);
SK_BOOL SK_MAC_SetDSN(SK_UB val);
SK_BOOL SK_MAC_SetPanID(SK_UH pan_id);
SK_BOOL SK_MAC_SetShortAddress(SK_UH addr);
SK_BOOL SK_MAC_SetRxOnWhenIdle(SK_BOOL val);
SK_BOOL SK_MAC_SetTransactionPersistenceTime(SK_UH val);
SK_BOOL SK_MAC_SetMaxFrameTotalWaitTime(void);
SK_BOOL SK_MAC_SetMaxFrameRetries(SK_UB num);
SK_BOOL SK_MAC_SetResponseWaitTime(SK_UB val);
SK_BOOL SK_MAC_SetAssociatedPANCoord(SK_BOOL flag);
SK_BOOL SK_MAC_SetMinBE(SK_UB val);
SK_BOOL SK_MAC_SetMaxBE(SK_UB val);
SK_BOOL SK_MAC_SetMaxCSMABackoffs(SK_UB val);
SK_BOOL SK_MAC_SetNetworkId(SK_UB id[]);
SK_BOOL SK_MAC_IsEDScanning(void);
SK_BOOL SK_MAC_IsScanning(void);

void SK_MAC_SetRepeater(SK_UB flag);
#ifndef NO_REPEATER
typedef struct {
    SK_UH               m_SrcAddress;   
    SK_UB               m_SeqNum; 
    SK_UH               m_TimeStamp;
	SK_UB				m_Using;
} SK_MAC_BTT;

void SK_MAC_ClearBTT(SK_MAC_BTT *btt);
void SK_MAC_AddToBTT(SK_UH Src, SK_UB Seq);
SK_BOOL SK_MAC_IsExistBTT(SK_UH Src, SK_UB Seq);
#endif

SK_BOOL SK_MAC_SetCurrentSecurityLevel(SK_UB level);
SK_BOOL SK_MAC_SetCurrentKeyIdMode(SK_UB key_id_mode);
SK_BOOL SK_MAC_SetCurrentKeySource(SK_UB index, SK_UB *key_source, SK_UB len);
SK_BOOL SK_MAC_SetCurrentKeyIndex(SK_UB key_index);

#ifdef __cplusplus
}
#endif

#endif