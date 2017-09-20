/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __prot_phy_h__
#define __prot_phy_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include 	"compiler_options.h"
#include	"skyley_stack.h"


// -------------------------------------------------
//   Consts
// -------------------------------------------------
#define		SK_AMAXPHYPACKETSIZE		PSDU

#define		SK_AMRFSKPHRLENGTH			2
#define 	SK_AMRFSKSFDLENGTH			2
#define		SK_ATURNAROUNDTIME			100 //SUN PHY, 1msec (100 symbols)
#define		SK_ACCATIME					13

#define		SK_PHY_SYMBOLS_PER_OCTET	8.0
#define		SK_PHY_FSK_PERAMBLE_LEN		10

#define		_cast float
#define		SK_PHY_SHR_DURATION			((_cast)SK_PHY_SYMBOLS_PER_OCTET * ((_cast)SK_PHY_FSK_PERAMBLE_LEN + (_cast)SK_AMRFSKSFDLENGTH))
#define		SK_PHY_MAX_FRAME_DURATION 	((_cast)SK_PHY_SHR_DURATION + ((_cast)SK_AMRFSKPHRLENGTH + (_cast)SK_AMAXPHYPACKETSIZE)*(_cast)SK_PHY_SYMBOLS_PER_OCTET)

// Results
#define		SK_PHY_BUSY					0x00	/* The CCA attempt has detected a busy channel. */
#define		SK_PHY_RX_ON				0x06	/* The transceiver is in or is to be configured into the receiver enabled state. */
#define		SK_PHY_SUCCESS				0x07	/* A SET/GET, an ED operation, or a transceiver state change was successful. */
#define		SK_PHY_TRX_OFF				0x08	/* The transceiver is in or is to be configured into the transceiver disabled state. */
#define		SK_PHY_NO_ACK				0x0B	/* NO Acknoridge */

#define		SK_PHY_RX_GOING				0x10
#define		SK_PHY_NO_RESPONSE			0x11
#define		SK_PHY_UNDER_SEND_LIMIT		0x12
#define		SK_PHY_ACK_WAITING			0x13

#define		SK_PHY_CHANNEL_PAGE			0x3A400002

// -------------------------------------------------
//   PD-DATA services
// -------------------------------------------------

typedef union {
	SK_UB			Raw[2];
	struct {
		SK_UB			m_Length1:3;
		SK_UB			m_Whitening:1;
		SK_UB			m_FCS:1;
		SK_UB			m_reserved:2;
		SK_UB			m_ModeSwitch:1;
		SK_UB			m_Length2:8;
	} 				Field;
} PHR_15d4g;

//PD-DATA.request
typedef struct {
	SK_UH				m_PsduLength;					/* Payload length */
	SK_UB				m_TxOptions;					/* TX option */
	SK_BOOL				m_UseCSMA;
	SK_UB				m_Retry;
	SK_UB				m_Psdu[SK_AMAXPHYPACKETSIZE];	/* Payload  */
	SK_UH				m_HeaderLength;
	SK_UB 				m_SecurityLevel;
	SK_UB 				m_KeyIdMode;
	SK_UB 				m_KeySource[8];
	SK_UB 				m_KeyIndex;	
	//SK_UW				m_FrameCounter;
} SK_PD_DATA_REQUEST;

//PD-DATA.confirm
typedef struct {
	SK_UB				m_Status;
} SK_PD_DATA_CONFIRM;

//PD-DATA.indication
typedef struct {
	SK_UH					m_PsduLength;					/* Payload length */
	SK_UB					m_PsduRssi;						/* RSSI of receive data */
	SK_UW					m_TimeStamp;
	SK_UH					m_RecvSlot;
	SK_UB					m_Psdu[SK_AMAXPHYPACKETSIZE];	/* Payload */
} SK_PD_DATA_INDICATION;


// -------------------------------------------------
//   PHY Layer public functions
// -------------------------------------------------

void SK_PHY_Init(void);
void SK_PHY_Task(void);
void SK_PHY_Interrupt(void);
SK_BOOL SK_PHY_ChangeChannel(SK_UB nChannel);
SK_UB SK_PHY_ReadRSSIValue(void);
void SK_PHY_Sleep(void);
void SK_PHY_Wakeup(void);
void SK_PHY_SetLowerLayer(SK_UB layer);
void SK_PHY_SetUpperLayer(SK_UB layer);
SK_UB SK_PHY_GetLowerLayer(void);
SK_UB SK_PHY_GetUpperLayer(void);


// -------------------------------------------------
//   Public variables
// -------------------------------------------------
//PIB
extern SK_UB			gnPHY_LowerLayer;
extern SK_UB 			gnPHY_UpperLayer;
extern SK_UB  			gnPHY_CurrentChannel;
extern SK_UB			gnPHY_CurrentTRX;

//Stack dependent
extern SK_BOOL			gnPHY_TestMode;

#ifdef __cplusplus
}
#endif

#endif
