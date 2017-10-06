/**
    main.c
     
    Author:        Skyley Networks, Inc.
    Version:
    Description:   SK command for SkWAN
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

// -------------------------------------------------
//   Include files
// -------------------------------------------------
#include "compiler_options.h"

#include <stdio.h>
#include <string.h>

#include "skyley_stack.h"
#include "ml7404.h"
#include "rf_interface.h"

#define CRLF SK_print("\r\n")
#define SPACE SK_print(" ")

#ifdef USE_MEMCPY_F
#define memcpy memcpy_f
#define strncmp strncmp_f
#endif


// -------------------------------------------------
//   Prototypes
// -------------------------------------------------
void Interface(void);
void EventProc(void);
void MetaBeacon(void);
void Tracker(void);
void SetupParams(void);
SK_UB GetParam(SK_UB *lineBuf, SK_UB **ATParam,SK_UH len);
SK_UB CharToNumber(SK_UB *param, SK_UW *num, SK_UB len);
SK_UB ParamCheck(SK_UB *param, SK_UW *num, SK_UB len, SK_UW max, SK_UW min);
SK_UB CheckDigit(SK_UB *ATParam, SK_UB num);
void HexToBin(SK_UB* bin, SK_UB* hex, SK_UH datalen);
SK_BOOL SaveParam(void);
SK_BOOL LoadParam(void);
SK_BOOL EraseParam(void);
SK_BOOL LoadMac(void);
SK_BOOL SaveMac(void);
SK_BOOL IsAutoLoad(void);
SK_BOOL PostJoinReq(SK_UW sta_id);
SK_UB sum(SK_UB* buf, SK_UH len);
void sk_ping_send(SK_UB len);
void SK_print_mac_addr(SK_ADDRESS* addr);
void SK_print_key(SK_UB* data, SK_UB len);
void _putc(char ch);
SK_H _getc(void);
void InitTrackingState(void);

extern SK_BOOL SetEncKeyOf(SK_ADDRESS* addr, SK_UB* key, SK_UB key_len);
extern SK_UW gnSSMac_FrameCounter; 
extern SK_UW gnSSMac_FrameCounterSTA;
extern SK_UH GetAllSlotNum(SK_UB mode);


// -------------------------------------------------
//   State machine
// -------------------------------------------------
SK_STATESTART(METABCN);	
SK_STATESTART(METABCN_STATE);
SK_STATESTART(TRACKER_STATE);


// -------------------------------------------------
//   SK command control
// -------------------------------------------------
#ifndef SK_E_OK
#define 	SK_E_OK							0
#endif
#ifndef SK_E_ER
#define 	SK_E_ER							(-1)
#endif

#define		SAMPLEAPP_STATUS_LINE			0
#define		SAMPLEAPP_STATUS_CMD			1
#define		SAMPLEAPP_STATUS_HEX			2
#define		SAMPLEAPP_STATUS_RAW			3
#define		SAMPLEAPP_STATUS_WAIT			4

#define		SAMPLEAPP_MENU_TOP				1
#define		SAMPLEAPP_MENU_TOP_WAIT			2
#define		SAMPLEAPP_MENU_WAIT_CRLF		13

#define		ER01							1
#define		ER02							2
#define		ER03							3
#define		ER04							4
#define		ER05							5
#define		ER06							6
#define		ER07							7
#define		ER08							8
#define		ER09							9

#define 	MAX_SHORT_DATA_SIZE				256
#define 	LINE_BUFFER_SIZE				(MAX_SHORT_DATA_SIZE + 64)
#define 	NUM_OF_ATPARAM 					48
static SK_UB		gn_aLineBuf[LINE_BUFFER_SIZE];
static SK_UH		gn_nLinePos = 0;
static SK_UB		gn_nLineStatus = SAMPLEAPP_STATUS_LINE;
static SK_UB		gn_nMenuStatus = SAMPLEAPP_MENU_TOP;

#define 	MAX_DATA_SIZE					PSDU
SK_UB gSendDataBuffer[ MAX_DATA_SIZE ];


// -------------------------------------------------
//   SK Command working 
// -------------------------------------------------
//echo back is off when 0
SK_BOOL gEchoBack = TRUE;

//exec join cmd when sync event is received
SK_BOOL gAutoJoin = TRUE;

SK_UB gPingLen = 0;
SK_BOOL gPing = FALSE;

extern SK_BOOL gUARTOverRun;
extern SK_BOOL gnPHY_SleepTimerCompleted;
extern SK_UH gRTCValue;
extern SK_UH gRTCShortValue;

//ML7404 gold seed
extern SK_UW gSHRGoldSeed;
extern SK_UW gPSDUGoldSeed;


// -------------------------------------------------
//   Flash self programming for SKSAVE, SKLOAD
// -------------------------------------------------
#define USE_SAVE_PARAM

#ifdef USE_SAVE_PARAM
	#define PERBLOCK 		498 //= 0x1C03E400
	#define MAC_PERBLOCK	499 //= 0x1C03E400
	#define PERBANK			0
	#define PERDATA_SIZE	128
	static SK_UB			gPerData[ PERDATA_SIZE ];
#endif

static SK_UB 			gAutoLoad;


// -------------------------------------------------
//   Meta beacon states
// -------------------------------------------------
typedef enum {
	eMetaBcnIdle,
	eMetaBcnWait,
	eMetaBcnSend
} MetaBcnState;

SK_UW gMetaBcnDuration;
SK_BOOL gStopMetaBcn;


// -------------------------------------------------
//   Tracker states
// -------------------------------------------------
typedef enum {
	eTrackIdle,
	eTrackStart,
	eTrackWaitForSlot,
	eTrackFinalize
} TrackerState;

SK_UW gTrackingStaId;
SK_UB gTrackingNicIdx;

SK_UH gTrackCurrentSlot;
SK_UB gTrackSlotMode;
SK_UB gTrackTargetCh;


// -------------------------------------------------
//   Protocol stack main
// -------------------------------------------------

void CommandInit(void){
 	if( IsAutoLoad() == TRUE ){
		LoadMac();
		LoadParam();
	} else {
		SK_BOOL ans;
		
		//Init SkWAN Stack
		ans = LoadMac();
		if( ans == FALSE ){
			SK_Base_Init(0x12345678, 0xabcdef01);
			
			//Init app specified params
			SetupParams();
		}
	}
	

	// -----------------------------
	//	Some test code
	// -----------------------------
	#if 0
	{
		extern SK_UH CalcMySlot(SK_UB*);
		static SK_UH cnt = 0;
		SK_UH slot;
		
		for( cnt = 0; cnt < 10000; cnt++ ){
			SSMac_Init(0x12345678, id);
			slot = CalcMySlot(SSMac_GetSlotHashKey());
			
			SK_print("id:"); SK_print_hex(id, 8); SK_print("=>"); SK_print_hex(slot, 4); SK_print("\r\n");
			id++;
		}
	}
	#endif
	
	#if 0
	{
		SK_UH i, j;
		
		SSMac_SetHoppingTable(3);
		
		SSMac_SetChannelTable(0, 1);
		SSMac_SetChannelTable(1, 2);
		SSMac_SetChannelTable(2, 3);
		SSMac_SetChannelTable(3, 4);
		
		for( i = 0; i < 255; i++ ){
			for( j = 0; j < 256; j++ ){
				SSMac_ChannelHopping(j, (SK_UB)i);
				SK_print("bscn "); SK_print_hex(i, 2); SK_print(":slots "); SK_print_hex(j, 4); SK_print("=>");
				SK_print_hex(gnPHY_CurrentChannel, 2); SPACE;
			}
			CRLF;
		}
	}
	#endif
	
	#if 0
	{
		SK_UH i;
		SK_BOOL ans;
		SK_ADDRESS target;
		
		target.m_AddrMode = SK_ADDRMODE_EXTENDED;
		target.Body.Extended.m_Long[0] = 0x12345678;
		
		SSMac_SetSlotMode(SS_SLOTMODE_1024);
		
		for( i = 0; i < 1024; i++ ){
			target.Body.Extended.m_Long[1] = i;
			SK_print("add:"); SK_print_mac_addr(&target); SPACE;
			ans = SSMac_AutoJoin(&target);
			SK_print_hex(ans, 2); CRLF;
		}
		
		SSMac_SetSlotMode(SS_SLOTMODE_64);
	}
	#endif
}


void CommandMain(void){
	Interface();
	
	SK_Base_Main();
	
	EventProc();
	
	MetaBeacon();
		
	Tracker();

//loop performance check
#if 0
	{
	  SK_UW now = SK_GetLongTick();
	  static SK_UW start = 0;
	  static SK_UW cnt = 0;
	  
	  if( (SK_UW)(now - start) > 1000 ){
		SK_print_hex(now, 8); SK_print("\r\n");
		SK_print_dec(cnt, 8, 0); SK_print("\r\n");
		start = now;
		cnt = 0;
	  }
	  cnt++;
	}
#endif
}


// -------------------------------------------------
//   Setup app parameters
// -------------------------------------------------
void SetupParams(void){
	gn_nLinePos = 0;
	gn_nLineStatus = SAMPLEAPP_STATUS_LINE;
	gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
	
	SSMac_SetBaseChannel(24);

	SSMac_SetSlotMode(SS_SLOTMODE_32);
	
	SSMac_SetHoppingTable(SS_HOPPING_TABLE1);
	
	gEchoBack = TRUE;
	gAutoJoin = TRUE;
	gAutoLoad = FALSE;
	gPing = FALSE;
	gPingLen = 0;

	gMetaBcnDuration = SS_META_BEACON_DURATION;

	InitTrackingState();

	SK_INITSTATE(METABCN);
	SK_INITSTATE(METABCN_STATE);
	SK_INITSTATE(TRACKER_STATE);
	
	SK_SETSTATE(eMetaBcnIdle, METABCN_STATE);
	SK_SETSTATE(eTrackIdle, TRACKER_STATE);
	
	//just to avoid compiler warning
	gnSK_Time_METABCN_STATE++;
	gnSK_Time_TRACKER_STATE++;
}


void MetaBeacon(void){
	static MetaBcnState state;

	state = (MetaBcnState)SK_GETSTATE(METABCN_STATE);
	
	SK_STATEADD(METABCN, 1);	
	
	switch( state ){
	case eMetaBcnIdle:
		//do nothing for idle
		gStopMetaBcn = FALSE;
		break;
		
	case eMetaBcnWait:
		//10sec sleep
		SK_SLEEP(gMetaBcnDuration, METABCN, 1);
		
		if( gStopMetaBcn == TRUE ){
			SK_SETSTATE(eMetaBcnIdle, METABCN_STATE);
		} else {
			//move to transmit
			SK_SETSTATE(eMetaBcnSend, METABCN_STATE);
		}
		break;
		
	case eMetaBcnSend:
		//send meta beacon
		SSMac_TransmitMetaBeacon();
		
		//wait for next transmission
		SK_SETSTATE(eMetaBcnWait, METABCN_STATE);
		break;
		
	default:
		break;
	}
	
	SK_STATEEND(METABCN);
}


void Tracker(void){
	static TrackerState state;

	state = (TrackerState)SK_GETSTATE(TRACKER_STATE);

	switch( state ){
	case eTrackIdle:
		//do nothing for idle
		break;
		
	case eTrackStart:{
		SK_UH slot_num = GetAllSlotNum(gTrackSlotMode);
		SK_UH remain;
		
		if( gTrackCurrentSlot >= slot_num ){
			//abnormal case
			remain = 0;
		} else {
			remain = slot_num - gTrackCurrentSlot - 1;
		}
		
		SSMac_SetSlotMode(gTrackSlotMode);
		SSMac_SetCurrentSlot(gTrackCurrentSlot);
		SSMac_Sleep(TRUE);
		
		//残りスロット数が少ない場合はスリープせずに待機
		if( remain <= 2 ){
			SK_SETSTATE(eTrackWaitForSlot, TRACKER_STATE);	
		} else {
			SK_SETSTATE(eTrackWaitForSlot, TRACKER_STATE);
			SleepTimer_Go(slot_num - 1);
		}
		break;
	}
	
	case eTrackWaitForSlot:
		//最終スロットで起床してスロット0のビーコンを待つ
		if( SSMac_GetCurrentSlot() == GetAllSlotNum(gTrackSlotMode) - 1 ){
			SSMac_SetBaseChannel(gTrackTargetCh);
			SSMac_SetStationId(gTrackingStaId);

			SK_print("ESTAT A1\r\n");
			
			//Finish tracking
			InitTrackingState();
		}
		break;

	default:
		break;
	}
}


void InitTrackingState(void){
	gTrackingStaId = 0xFFFFFFFF;
	
	//NIC idx must be in 0-3, so value 0xFF indicates "initialized"
	gTrackingNicIdx = 0xFF;
	
	gTrackCurrentSlot = 0xFFFF;
	gTrackSlotMode = 0xFF;
	gTrackTargetCh = 0;
	
	SK_SETSTATE(eTrackIdle, TRACKER_STATE);
}


// -------------------------------------------------
//   SK command parser
// -------------------------------------------------
static SK_H in;
void Interface(void) {
	SK_UB  NumOfATParam;
	SK_UB* ATParam[NUM_OF_ATPARAM];
	
	// -------------------------------------------------
	//   Get user input from PC (UART)
	// -------------------------------------------------
	in = _getc();
	if (in >= 0) {
		switch (gn_nLineStatus) {

		case SAMPLEAPP_STATUS_HEX:
			// Input Hex value
			if (in >= 0x20) {
				if ((in >= 'a') && (in <='z')) { in -= 0x20; }
				if (((in >= '0') && (in <='9')) || ((in >= 'A') && (in <='F'))) {
					// hex value
				} else {
					in = 0;
				}
			}
		case SAMPLEAPP_STATUS_LINE:
			// Input Strings
			if (in >= 0x20) {
				if (gn_nLinePos < (LINE_BUFFER_SIZE-1)) {
					gn_aLineBuf[gn_nLinePos++]	= (SK_UB)in;
					gn_aLineBuf[gn_nLinePos]	= 0;
					if( gEchoBack == 1 )_putc((SK_UB)in);
				}
				in = 0;
			} else {
				if (in == 13) {
					gn_aLineBuf[gn_nLinePos]	= 0;
					//SK_print("\r\n");
					if( gEchoBack == 1 )SK_print("\r\n");
					in = 1;
				} else {
					if ((in == 8) || (in == 127)) {
						if (gn_nLinePos > 0) {
							if( gEchoBack == 1 )_putc(8);
							if( gEchoBack == 1 )_putc(' ');
							if( gEchoBack == 1 )_putc(8);	
							gn_nLinePos--;
							gn_aLineBuf[gn_nLinePos]	= 0;
						}
					}
					in = 0;
				}
			}
			break;
			
		case SAMPLEAPP_STATUS_WAIT:
			break;
			
		default:
			// Input one ASCII character
			if ((in >= 0x20) || (in == 13)) {
				if ((in >= 'a') && (in <='z')) { in -= 0x20; }
				gn_aLineBuf[0] 			= (SK_UB)in;
				gn_aLineBuf[1] 			= 0;
				gn_nLinePos = 1;
				if( gEchoBack == 1 )_putc((SK_UB)in);
				if( gEchoBack == 1 )SK_print("\r\n");
				in = 1;
			} else {
				in = 0;
			}
			break;
		}
	}
	
	if( gUARTOverRun == TRUE ){
		SK_print("\r\nFAIL ER09\r\n");
		
		//コマンド入力はキャンセル
		gUARTOverRun = FALSE;
		in = 0;
		gn_nMenuStatus = SAMPLEAPP_MENU_WAIT_CRLF;
		gn_nLineStatus = SAMPLEAPP_STATUS_WAIT;
		gn_nLinePos = 0;
	}

	// -------------------------------------------------
	//   Process a input value
	// -------------------------------------------------
	switch(gn_nMenuStatus) {
		case SAMPLEAPP_MENU_WAIT_CRLF:
			if( in == 13 ){
				gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
			} else {
				break;
			}
		case SAMPLEAPP_MENU_TOP:
			gn_nLineStatus		= SAMPLEAPP_STATUS_LINE;
			//gn_nLinePos			= 0;
			gn_nMenuStatus		= SAMPLEAPP_MENU_TOP_WAIT;
			//memset(gn_aLineBuf, 0, LINE_BUFFER_SIZE);
			in = 0;

		case SAMPLEAPP_MENU_TOP_WAIT:
			if (in > 0) {
				NumOfATParam = GetParam(gn_aLineBuf, ATParam, gn_nLinePos);
				if( NumOfATParam == 0 || NumOfATParam == 255 ) {
					if( NumOfATParam == 255 ){
						SK_print("FAIL ER04\r\n");
					}
					gn_nLinePos = 0;
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				}
				
				if(strcmp((const char *)ATParam[0], (const char*)"SKINFO") == 0){
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					SK_print("EINFO ");
					
					//64bit addr
					SK_print_hex(SSMac_GetAddress()->Body.Extended.m_Long[0], 8);
					SK_print_hex(SSMac_GetAddress()->Body.Extended.m_Long[1], 8);
					SPACE;
					
					//channel
					SK_print_hex(SSMac_GetBaseChannel(), 2); SPACE;
					
					//Station Id
					SK_print_hex(SSMac_GetStationId(), 8); SPACE;
					
					//Slot
					SK_print_hex(SSMac_GetSlotNum(), 4); SPACE;

					//In Slot Index
					SK_print_hex(SSMac_GetInSlotIdx(), 2); SPACE;
					
					SK_print("\r\nOK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((const char *)ATParam[0], (const char*)"SKSREG") == 0){
					SK_UW sregno;
					
					if((NumOfATParam < 2) || (3 < NumOfATParam)){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if((strncmp((const char *)ATParam[1], (const char*)"S", 1) != 0) ||
					   (ParamCheck(ATParam[1] + 1, &sregno, 2, 255, 1) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if(NumOfATParam == 2){	/* Read Sreg */
						if(sregno == 1){
							//64bit addr
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetAddress()->Body.Extended.m_Long[0], 8);
							SK_print_hex(SSMac_GetAddress()->Body.Extended.m_Long[1], 8);
							
						} else if(sregno == 2 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetDeviceType(), 2);
							
						} else if(sregno == 3 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetSlotMode(), 2);
							
						} else if(sregno == 4 ){
							SK_print("ESREG ");
							SK_print_hex((SK_UB)SSMac_GetRxOnWhenIdle(), 2);
							
						} else if(sregno == 5 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetStationId(), 8);
							
						} else if(sregno == 6 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetPendExpTime(), 8);
						
						} else if(sregno == 8 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetBaseChannel(), 2);
						
						} else if(sregno == 9 ){
							SK_print("ESREG ");
							SK_print_hex(gAutoJoin, 2);
						
						} else if(sregno == 10 ){
							SK_print("ESREG ");
							SK_print_hex(gMetaBcnDuration, 8);
						
						} else if(sregno == 13 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetHoppingTable(), 2);
						
						} else if(sregno == 14 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetHoppingEnable(), 2);
						
						} else if(sregno == 15 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_IsFullManage(), 2);

						} else if(sregno == 16 ){
						  	SK_print("ESREG ");
							SK_print_hex(gnPHY_CurrentChannel, 2);
							
						} else if(sregno == 17 ){
							SK_print("ESREG ");
							SK_print_hex( MAC_GET_LONG_B(SSMac_GetSlotHashKey()), 8 );

						} else if(sregno == 20 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetSlotCalib(), 2);
						
						} else if(sregno == 21 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetFineCalib(), 8);

						} else if(sregno == 22 ){
							SK_print("ESREG ");
							SK_print_hex(gRTCValue, 4);
						
						} else if(sregno == 23 ){
							SK_print("ESREG ");
							SK_print_hex(gRTCShortValue, 4);

						} else if(sregno == 24 ){
							SK_print("ESREG ");
							SK_print_hex(SSMac_GetCurrentSlot(), 4);

						} else if(sregno == 30 ){
							SK_print("ESREG ");
							SK_print_hex(BASE_PHY_TYPE, 2);

						} else if(sregno == 40 ){
							SK_print("ESREG ");
							SK_print_hex(gSHRGoldSeed, 8);

						} else if(sregno == 41 ){
							SK_print("ESREG ");
							SK_print_hex(gPSDUGoldSeed, 8);

						} else if(sregno == 250 ){
							SK_print("ESREG ");
							SK_print_hex(gnPHY_TestMode, 2);
						
						} else if(sregno == 251 ){
							SK_print("ESREG ");
							SK_print_hex(gAutoLoad, 2);
						
						} else if(sregno == 252 ){
						  	SK_print("ESREG ");
							SK_print_hex(gEchoBack, 2);
						
						} else if(sregno == 253 ){
						  	SK_print("ESREG ");
							SK_print_hex(gPing, 2);
						
						}
						
						SK_print("\r\n");
						SK_print("OK\r\n");
					} else {	/* Write Sreg */
						SK_UW val;
						
						if(sregno == 1){
							SK_UW ieee_upr = 0, ieee_lwr = 0;

							//IEEE 64-bit address
							if(ParamCheck(ATParam[2] + 8, &ieee_lwr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							*(ATParam[2] + 8) = 0;
							if(ParamCheck(ATParam[2], &ieee_upr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
							
							SK_Base_Init(ieee_upr, ieee_lwr);
							SetupParams();
							
							//SymbolTimer_Initialize();
							SleepTimer_Initialize();
							Timer_Initialize();
							
						} else if(sregno == 2){
							if(ParamCheck(ATParam[2], &val, 1, 3, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
							
							SSMac_SetDeviceType((SK_UB)val);
							
						} else if(sregno == 3){
							if(ParamCheck(ATParam[2], &val, 1, 6, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
							
							SSMac_SetSlotMode((SK_UB)val);
							
						} else if(sregno == 4){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
							
							SSMac_SetRxOnWhenIdle((SK_BOOL)val);
							
						} else if(sregno == 5){
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetStationId(val);
							
						} else if(sregno == 6){
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetPendExpTime(val);
							
						} else if(sregno == 8){
							if(ParamCheck(ATParam[2], &val, 2, 60, 24) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetBaseChannel((SK_UB)val);
							
						} else if(sregno == 9){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gAutoJoin = (SK_BOOL)val;
						
						} else if( sregno == 10 ){
							if( (ParamCheck(ATParam[2], (SK_UW *)&val, 8, 0xFFFFFFFF, 0) != SK_E_OK) ){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
							
							if( val == 0 ){
								gStopMetaBcn = TRUE;
							} else {
								if( val < 1000 ) {
									SK_print("FAIL ER06\r\n");	
									gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
									break;
								} else {
									gMetaBcnDuration = val;
									SK_SETSTATE(eMetaBcnWait, METABCN_STATE);	
								}
							}
						
						} else if(sregno == 11){
							if(ParamCheck(ATParam[2], &val, 2, 1024, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetSlotNum((SK_UH)val);
							
						} else if(sregno == 12){
							if(ParamCheck(ATParam[2], &val, 2, (SS_MAX_SHARED_DEVICES-1), 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetInSlotIdx((SK_UB)val);
							
						} else if(sregno == 13){
							if(ParamCheck(ATParam[2], &val, 1, 3, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetHoppingTable((SK_UB)val);
							
						} else if(sregno == 14){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetHoppingEnable((SK_BOOL)val);
							
						} else if(sregno == 15){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetFullManage((SK_BOOL)val);
							
						} else if(sregno == 16){ //gnPHY_CurrentChannel
							SK_print("FAIL ER06\r\n");
							gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
							break;
							
						} else if(sregno == 17){				
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							MAC_SET_LONG_B(SSMac_GetSlotHashKey(), val);
							
						} else if(sregno == 20){
							if(ParamCheck(ATParam[2], &val, 2, 0xFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetSlotCalib((SK_UB)val);
							
						} else if(sregno == 21){
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							SSMac_SetFineCalib(val);
							
						} else if(sregno == 22){
							if(ParamCheck(ATParam[2], &val, 4, 0xFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gRTCValue = (SK_UH)val;
							
						} else if(sregno == 23){
							if(ParamCheck(ATParam[2], &val, 4, 0xFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gRTCShortValue = (SK_UH)val;
						
						} else if(sregno == 24){
						  	extern SK_UH gnSSMac_CurrentSlot;
							
							if(ParamCheck(ATParam[2], &val, 4, 0xFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}
			
							gnSSMac_CurrentSlot = (SK_UH)val;

						} else if(sregno == 40){
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gSHRGoldSeed = val;
							ml7404_trx_off();
							ml7404_go_rx_mode();

						}else if(sregno == 41){
							if(ParamCheck(ATParam[2], &val, 8, 0xFFFFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gPSDUGoldSeed = val;
							ml7404_trx_off();
							ml7404_go_rx_mode();

						//for Frame counter test
						} else if(sregno == 248){
							if(ParamCheck(ATParam[2], &val, 8, 0x00FFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gnSSMac_FrameCounter = val;
							
						//for Frame counter test
						} else if(sregno == 249){
							if(ParamCheck(ATParam[2], &val, 8, 0x00FFFFFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gnSSMac_FrameCounterSTA = val;

						} else if(sregno == 250){
							if(ParamCheck(ATParam[2], &val, 2, 0xFF, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gnPHY_TestMode = (SK_UB)val;
							
						} else if(sregno == 251){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gAutoLoad = (SK_UB)val;
						
						} else if(sregno == 252){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gEchoBack = (SK_BOOL)val;
							
						} else if(sregno == 253){
							if(ParamCheck(ATParam[2], &val, 1, 1, 0) != SK_E_OK){
								SK_print("FAIL ER06\r\n");
								gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
								break;
							}

							gPing = (SK_BOOL)val;
							
						}
						SK_print("OK\r\n");
					}
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
					
				} else if(strcmp((const char *)ATParam[0], (const char*)"SKRESET") == 0){
					SK_ADDRESS* addr;
					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					addr = SSMac_GetAddress();

					//If RF is sleep, wakeup it in SK_Base_Init
					SK_Base_Init(addr->Body.Extended.m_Long[0], addr->Body.Extended.m_Long[1]);
					
					rf_init();
					
					ml7404_reset_seeds();
					
					//Set default stack parameters
					SetupParams();
	
					//Restart timers
					SleepTimer_Initialize();
					Timer_Initialize();
					
					//Start RX
					ml7404_go_rx_mode();
					
					SK_print("OK\r\n");
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
					
				} else if(strcmp((char *)ATParam[0], "SKSAVE") == 0){
					SK_BOOL ans;
					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					ans = SaveMac();
					if( ans == FALSE ){
						SK_print("FAIL ER10\r\n");
						goto save_exit;
					}

					ans = SaveParam();
					if( ans == TRUE ){
						SK_print("OK\r\n");		
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					save_exit:
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
						
				} else if(strcmp((char *)ATParam[0], "SKLOAD") == 0){
					SK_BOOL ans;
					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
										
					ans = LoadMac();
					if( ans == FALSE ){
						SK_print("FAIL ER10\r\n");
						goto load_exit;
					}

					ans = LoadParam();
					if( ans == TRUE ){
						SK_print("OK\r\n");		
					} else {
						SK_print("FAIL ER10\r\n");
					}

					load_exit:
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
						
				} else if(strcmp((char *)ATParam[0], "SKERASE") == 0){
					SK_BOOL ans;
					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					ans = EraseParam();
					if( ans == TRUE ){
						SK_print("OK\r\n");
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
						
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKPHYSEND") == 0){
					SK_PD_DATA_REQUEST *PdReq;
					SK_UW datalen;
					
					if( (ParamCheck(ATParam[1], (SK_UW *)&datalen, 4, (SK_UH)MAX_DATA_SIZE, (SK_UH)0x01) != SK_E_OK) ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
						
				 	if (SK_AllocDataMemory((SK_VP *)&PdReq) != SK_E_OK) { 
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					memset(gSendDataBuffer, 0, MAX_DATA_SIZE);
					HexToBin(gSendDataBuffer, ATParam[2], (SK_UH)datalen);
					
					PdReq->m_PsduLength = (SK_UH)datalen;
					memcpy(PdReq->m_Psdu, gSendDataBuffer, (SK_UH)datalen);

					//Set total len
					PdReq->m_TxOptions = 1;
					PdReq->m_Retry = 0;
					PdReq->m_UseCSMA = TRUE;  //exec CSMA/CA

					if (SK_PostMessageSL(SK_LAYER_PHY, SK_LAYER_APL, SK_PD_DATA_REQUEST_CMD, (SK_VP)PdReq, 30, 0, 0xFFFF) != SK_E_OK) {
						SK_FreeMemory(PdReq);
					}
					
					SK_print("OK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKSEND") == 0){
					SS_DATA_REQUEST *SdReq;
					SK_ADDRESS target;
					SK_UW datalen;
					SK_BOOL result;
					SK_UW addr;

					if(NumOfATParam != 4){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if( (ParamCheck(ATParam[2], (SK_UW *)&datalen, 2, SS_PAYLOAD_LEN, 0x01) != SK_E_OK) ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					//IEEE 64-bit address
					if(ParamCheck(ATParam[1] + 8, &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[1] = addr;

					*(ATParam[1] + 8) = 0;
					if(ParamCheck(ATParam[1], &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[0] = addr;
						
				 	if (SK_AllocDataMemory((SK_VP *)&SdReq) != SK_E_OK) { 
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					memset(gSendDataBuffer, 0, SS_PAYLOAD_LEN);
					HexToBin(gSendDataBuffer, ATParam[3], (SK_UB)datalen);
					
					SdReq->m_MsduLength = (SK_UB)datalen;
					memcpy(SdReq->m_Msdu, gSendDataBuffer, (SK_UB)datalen);

					SdReq->m_Selector = SS_SELECTOR_APP_DATA;
					SdReq->m_TxOptions = 0;
					SdReq->m_TxOptions |= SS_TXOPTIONS_SECURITY;
					SdReq->m_Handle = (SK_UH)(SSMac_GetRand32());
					
					target.m_AddrMode = SK_ADDRMODE_EXTENDED;
					SSMac_CopyAddress(&(SdReq->m_DstAddress), &target);
					
					result = TRUE;
					if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
						SdReq->m_TxOptions |= SS_TXOPTIONS_INDIRECT;
						
						if (SK_PostMessage(SK_LAYER_SS_MAC, SK_LAYER_APL, SS_DATA_REQUEST_CMD, (SK_VP)SdReq) != SK_E_OK) {
							SK_FreeMemory(SdReq);
							result = FALSE;
						} 
					} else {
						SdReq->m_DstAddress.Body.Extended.m_Long[0] = 0;
						SdReq->m_DstAddress.Body.Extended.m_Long[1] = 0;
						
						if (SK_PostMessageS(SK_LAYER_SS_MAC, SK_LAYER_APL, SS_DATA_REQUEST_CMD, (SK_VP)SdReq, SSMac_GetSlotNum()) != SK_E_OK) {
							SK_FreeMemory(SdReq);
							result = FALSE;
						}
					}

					if( result == TRUE ){
						SK_print("OK "); SK_print_hex(SdReq->m_Handle, 4); CRLF; 
					} else {
						SK_print("FAIL ER10\r\n");
					}
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKJOIN") == 0){
					SK_BOOL ans;

					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if( SSMac_GetDeviceType() != SS_TYPE_DEVICE ){
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					//非同期中はJOIN不可
					// ->ビーコンを受信するとSlotNumは非0xFFFFにそのスロットで同期動作を開始する
					if( SSMac_GetStationId() == 0xFFFFFFFF ||
						SSMac_GetSlotNum() == 0xFFFF ){
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
	
					ans = PostJoinReq( SSMac_GetStationId() );
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");	
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKREGDEV") == 0){
					SK_ADDRESS target;
					SK_BOOL ans;
					SK_UW slot, idx;
					SK_UW addr;

					if(NumOfATParam != 4){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
	
					//IEEE 64-bit address
					if(ParamCheck(ATParam[1] + 8, &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[1] = addr;

					*(ATParam[1] + 8) = 0;
					if(ParamCheck(ATParam[1], &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[0] = addr;
					
					if( (ParamCheck(ATParam[2], (SK_UW *)&slot, 4, 1024, 1) != SK_E_OK) ||
						(ParamCheck(ATParam[3], (SK_UW *)&idx, 1, 1, 0) != SK_E_OK) ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					target.m_AddrMode = SK_ADDRMODE_EXTENDED;
					
					ans = SSMac_DirectJoin(&target, (SK_UH)slot, (SK_UB)idx);
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKADDDEV") == 0){
					SK_ADDRESS target;
					SK_BOOL ans;
					SK_UW addr;
					
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
	
					//IEEE 64-bit address
					if(ParamCheck(ATParam[1] + 8, &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[1] = addr;

					*(ATParam[1] + 8) = 0;
					if(ParamCheck(ATParam[1], &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[0] = addr;

					target.m_AddrMode = SK_ADDRMODE_EXTENDED;
					
					ans = SSMac_AutoJoin(&target);
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKSETPSK") == 0){	
					SK_BOOL ans;

					//1:key (must be ASCII 32)
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					memset(gSendDataBuffer, 0, 32);
					HexToBin(gSendDataBuffer, ATParam[1], 16);
					
					ans = SSMac_SetPSK(gSendDataBuffer, 16);

					if( ans == TRUE ){
						SK_print("OK\r\n");
					} else {
						SK_print("FAIL ER10\r\n");
					}
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
					
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKSETKEY") == 0){	
					SK_BOOL ans;

					//1:key (must be ASCII 32)
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					memset(gSendDataBuffer, 0, 32);
					HexToBin(gSendDataBuffer, ATParam[1], 16);
					
					ans = SSMac_SetAESKey(gSendDataBuffer, 16);

					if( ans == TRUE ){
						SK_print("OK\r\n");
					} else {
						SK_print("FAIL ER10\r\n");
					}
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
					
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKREGKEY") == 0){
					SK_ADDRESS target;
					SK_BOOL ans;
					SK_UW addr;

					//1:addr
					//2:AES key
					if(NumOfATParam != 3){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
	
					//IEEE 64-bit address
					if(ParamCheck(ATParam[1] + 8, &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[1] = addr;

					*(ATParam[1] + 8) = 0;
					if(ParamCheck(ATParam[1], &addr, 8, 0xFFFFFFFF, 0x00000000) != SK_E_OK){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					target.Body.Extended.m_Long[0] = addr;
					
					target.m_AddrMode = SK_ADDRMODE_EXTENDED;
					
					memset(gSendDataBuffer, 0, 32);
					HexToBin(gSendDataBuffer, ATParam[2], 16);

					ans = SetEncKeyOf(&target, gSendDataBuffer, 16);
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKSETNIC") == 0){
					SK_BOOL ans;
					SK_UW index, slot, channel, mode;

					if(NumOfATParam != 5){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if( (ParamCheck(ATParam[1], (SK_UW *)&index, 2, 3, 0) != SK_E_OK) ||
						(ParamCheck(ATParam[2], (SK_UW *)&slot, 4, 1023, 0) != SK_E_OK) ||
						(ParamCheck(ATParam[3], (SK_UW *)&channel, 2, 60, 24) != SK_E_OK) ||
						(ParamCheck(ATParam[4], (SK_UW *)&mode, 2, 6, 0) != SK_E_OK)  ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
						
					if( SSMac_GetDeviceType() != SS_TYPE_METABEACON ){
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					ans = SSMac_SetDevInfo((SK_UB)index, (SK_UH)slot, (SK_UB)channel, (SK_UB)mode);
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], "SKTRACK") == 0){
					SK_UW sta_id, nic_idx;

					if(NumOfATParam != 3){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
	
					if( (ParamCheck(ATParam[1], (SK_UW *)&sta_id, 8, 0xFFFFFFFF, 0) != SK_E_OK) ||
						(ParamCheck(ATParam[2], (SK_UW *)&nic_idx, 1, 3, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					//同期中はトラッキング不可
					if( SSMac_GetStationId() != 0xFFFFFFFF ||
						SSMac_GetSlotNum() != 0xFFFF ||
						SSMac_GetInSlotIdx() != 0xFF ){
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					//0xFFFFFFFFはトラッキング停止
					if( sta_id == 0xFFFFFFFF ){
					  	InitTrackingState();
					} else {
						//トラッキング中の多重発行を抑止
						if( gTrackingStaId != 0xFFFFFFFF ){
							SK_print("FAIL ER10\r\n");
							gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
							break;
						}
					
						gTrackingStaId = sta_id;
						gTrackingNicIdx = (SK_UB)nic_idx;
					}
					
					SK_print("OK\r\n");	
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((const char *)ATParam[0], "SKCHTBL") == 0){
					SK_BOOL ans;
					SK_UW index, channel;

					if(NumOfATParam != 3){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if( (ParamCheck(ATParam[1], (SK_UW *)&index, 1, 3, 1) != SK_E_OK) ||
						(ParamCheck(ATParam[2], (SK_UW *)&channel, 2, 38, 24) != SK_E_OK)  ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					ans = SSMac_SetChannelTable((SK_UB)index, (SK_UB)channel);
					if( ans == TRUE ){
						SK_print("OK\r\n");	
					} else {
						SK_print("FAIL ER10\r\n");
					}
					
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				}  else if(strcmp((const char *)ATParam[0], (const char *)"SKVER") == 0){
					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					SK_print( (SK_B*)SSMac_GetVerStr() ); SK_print("\r\n");
					
					SK_print("OK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((char *)ATParam[0], "SKNBR") == 0){
					SK_UB i;
					SK_UW slot;
					SS_SLOT_ADDR_DB_ITEM* item;
					
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if( (ParamCheck(ATParam[1], (SK_UW *)&slot, 4, 1023, 0) != SK_E_OK) ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					item = SSMac_GetDBItemFor((SK_UH)slot);
					if( item == NULL ){
						SK_print("FAIL ER10\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					SK_print("ENBR\r\n");
					SK_print_hex(item->m_SlotNum, 4); SK_print("\r\n");
					for( i = 0; i < SS_MAX_SHARED_DEVICES; i++ ){
						SK_print_mac_addr(&(item->m_Addr[i]));  
						SK_print(" ");
						SK_print_key(&(item->m_Key[i][0]), SS_AES_KEY_LEN); 
						SK_print(" ");
						SK_print_hex(item->m_FrameCounter[i], 8);
						SK_print(" ");
						SK_print_hex(item->m_OutgoingFrameCounter[i], 8);
						SK_print("\r\n");
					}

					SK_print("OK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				
				} else if(strcmp((char *)ATParam[0], "SKRFREG") == 0){
					SK_UW reg_addr;
					SK_UW val;
					
					if(NumOfATParam == 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if((ParamCheck(ATParam[1], &reg_addr, 4, 0xFFFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if( NumOfATParam == 3 ){
						if((ParamCheck(ATParam[2], &val, 2, 0xFF, 0) != SK_E_OK)){
							SK_print("FAIL ER06\r\n");
							gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
							break;
						}
						
						ml7404_reg_write8((ML7404_Register)reg_addr, (SK_UB)val);
						SK_print("OK\r\n");
						
					} else if( NumOfATParam == 2 ){
						SK_print("ERFREG ");
						SK_print_hex( ml7404_reg_read8((ML7404_Register)reg_addr), 2 );
						SK_print("\r\nOK\r\n");
					}

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((char *)ATParam[0], "RESETN") == 0){					
					if(NumOfATParam != 1){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					rf_reset();

					SK_print("OK\r\n");

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((char *)ATParam[0], "WREG") == 0){
					SK_UW reg_addr;
					SK_UW val;
					
					if(NumOfATParam != 3){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if((ParamCheck(ATParam[1], &reg_addr, 4, 0xFFFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if((ParamCheck(ATParam[2], &val, 2, 0xFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
						
					ml7404_reg_write8((ML7404_Register)reg_addr, (SK_UB)val);
					SK_print("OK\r\n");

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((char *)ATParam[0], "BWREG") == 0){
					SK_UW reg_addr;
					SK_UW cnt, val;
					SK_UW reg_offset = 0;
					SK_UB i;
					
					//if(NumOfATParam != 3){
					//	SK_print("FAIL ER05\r\n");
					//	gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					//	break;
					//}
					
					if((ParamCheck(ATParam[1], &reg_addr, 4, 0xFFFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if((ParamCheck(ATParam[2], &cnt, 2, 0xFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if( reg_addr == 0x7C ){
						for( i = 0; i < cnt; i++ ){
							ParamCheck(ATParam[i+3], &val, 2, 0xFF, 0);
							ml7404_reg_write8(b0_WR_TX_FIFO, val);
						}
					} else {
						for( i = 0; i < cnt; i++ ){
							ParamCheck(ATParam[i+3], &val, 2, 0xFF, 0);
							
							SK_print("reg:"); SK_print_hex((reg_addr + reg_offset), 4); SK_print(" ");
							SK_print_hex(val, 2); SK_print("\r\n");
							
							ml7404_reg_write8((ML7404_Register)(reg_addr + reg_offset), (SK_UB)val);
							reg_offset++;
						}
					}
					
					SK_print("OK\r\n");

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((char *)ATParam[0], "RREG") == 0){
					SK_UW reg_addr;
					SK_UB val;
					
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if((ParamCheck(ATParam[1], &reg_addr, 4, 0xFFFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
						
					val = ml7404_reg_read8((ML7404_Register)reg_addr);
					SK_print_hex(val, 2);
					
					SK_print("\r\nOK\r\n");

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((char *)ATParam[0], "BRREG") == 0){
					SK_UW reg_addr;
					SK_UW val, cnt;
					SK_UW reg_offset = 0;
					SK_UB i;
					
					if(NumOfATParam != 3){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if((ParamCheck(ATParam[1], &reg_addr, 4, 0xFFFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if((ParamCheck(ATParam[2], &cnt, 2, 0xFF, 0) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					for( i = 0; i < cnt; i++ ){						
						val = ml7404_reg_read8((ML7404_Register)(reg_addr + reg_offset));
						SK_print_hex(val, 2); SK_print(" ");
						reg_offset++;
					}
						
					SK_print("\r\nOK\r\n");

					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;

				} else if(strcmp((const char *)ATParam[0], (const char*)"SKRFCTRL") == 0){
					SK_UW param;
					
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if((ParamCheck(ATParam[1], &param, 2, 0xFF, 0x00) != SK_E_OK)){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					//PN9出力
					if( param == 1 ){
						SK_H ch;

						ml7404_trx_off();
						ml7404_reg_write8(b0_RF_TEST_MODE, 0x03);
						SK_PHY_ChangeChannel(gnPHY_CurrentChannel);
						ml7404_reg_write8(b0_RF_STATUS, 0x09);
						
						while(1){
							ch = SK_getc();
							if( ch >= 0 ) break;
						}
					}

					//連続キャリア出力
					if( param == 2 ){
						SK_H ch;
						
						ml7404_trx_off();
						ml7404_reg_write8(b0_RF_TEST_MODE, 0x21);
						SK_PHY_ChangeChannel(gnPHY_CurrentChannel);
						ml7404_reg_write8(b0_RF_STATUS, 0x09);
						
						while(1){
							ch = SK_getc();
							if( ch >= 0 ) break;
						}
					}
					
					if( param == 3 ){
						SK_PHY_Sleep();	
					}
					
					if( param == 4 ){
						SK_PHY_Wakeup();
					}

					if( param == 5 ){
						SK_UW rand = SSMac_GetRand32();  
						SK_print_hex(rand, 8); CRLF;
					}
					
					if( param == 6 ){
						SleepTimer_Go(15);	
					}
					
					if( param == 7 ){
						TIMER_LOCK();
						while( _getc() < 0 );
					}
					
					if( param == 8 ){
					  	TIMER_UNLOCK();
						while( _getc() < 0 );
					}
					
					if( param == 9 ){

					}	
					
					if( param == 10 ){
					  	TIMER_LOCK();
						gnPHY_SleepTimerCompleted = FALSE;
					  	MCU_Sleep();
						//gnPHY_SleepTimerCompleted = TRUE;
						//MCU_Wakeup();
					}	
					
					if( param == 11 ){
					  	MCU_Wakeup();
					}
					
					if( param == 12 ){
					  	DebugPort_Set(0, TRUE);
					}
					
					if( param == 13 ){
					  	DebugPort_Set(0, FALSE);
					}
					
					if( param == 14 ){
						Timer_Initialize();  
					}

					SK_print("OK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
					
				} else if(strcmp((const char *)ATParam[0], (const char *)"SKTABLE") == 0){
					SK_UW mode;
										
					if(NumOfATParam != 2){
						SK_print("FAIL ER05\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}
					
					if( (ParamCheck(ATParam[1], (SK_UW *)&mode, 2, 0xFF, 0x01) != SK_E_OK) ){
						SK_print("FAIL ER06\r\n");
						gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
						break;
					}

					if( mode == 1 ){
						SK_UH i, j;
						
						SK_print("ENBR\r\n");
						
						for( i = 0; i < SS_DB_SIZE; i++ ){
							if( gaSSMac_SlotAddr_DB[i].m_SlotNum == 0xFFFF ) continue;
							SK_print_hex(gaSSMac_SlotAddr_DB[i].m_SlotNum, 4); SK_print("\r\n");
							for( j = 0; j < SS_MAX_SHARED_DEVICES; j++ ){
								SK_print_mac_addr(&(gaSSMac_SlotAddr_DB[i].m_Addr[j]));  
								SK_print(" ");
								SK_print_key(&(gaSSMac_SlotAddr_DB[i].m_Key[j][0]), SS_AES_KEY_LEN); 
								SK_print(" ");
								SK_print_hex(gaSSMac_SlotAddr_DB[i].m_FrameCounter[j], 8);
								SK_print(" ");
								SK_print_hex(gaSSMac_SlotAddr_DB[i].m_OutgoingFrameCounter[j], 8);
								SK_print("\r\n");
							}
						}
						
					} else if( mode == 2 ){
						SK_UH i;
						
						SK_print("EPEND\r\n");
						
						for( i = 0; i < SS_PENDINGBUF_SIZE; i++ ){
							if( gaSSMac_PendingBuf[i].m_Packet == NULL ) continue;
							SK_print_mac_addr( &(gaSSMac_PendingBuf[i].m_Packet->m_DstAddress) );
							SK_print(" ");
							SK_print_hex(gaSSMac_PendingBuf[i].m_Packet->m_Handle, 4);
							SK_print(" ");
							SK_print_hex(gaSSMac_PendingBuf[i].m_DownSlot, 4);
							SK_print(" ");
							SK_print_hex(gaSSMac_PendingBuf[i].m_TimeStamp, 8);
							SK_print("\r\n");
						}
						
					} else if( mode == 3 ){
						
						SK_print("ESEC\r\n");
						
						SK_print_key( SSMac_GetPSK(), SS_AES_KEY_LEN );
						SK_print("\r\n");
						SK_print_key( SSMac_GetAESKey(), SS_AES_KEY_LEN );
						SK_print("\r\n");
						SK_print_hex(SSMac_GetFrameCounter(), 8);
						SK_print("\r\n");
						SK_print_hex(SSMac_GetFrameCounterSTA(), 8);
						SK_print("\r\n");
						
					} else if( mode == 4 ){
						SK_UH i;
						
						SK_print("ENIC\r\n");
						
						for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
							SK_print_hex(gaSSMac_DevInfo[i].m_CurrentSlot, 4); SK_print(" ");
							SK_print_hex(gaSSMac_DevInfo[i].m_BaseChannel, 2); SK_print(" ");
							SK_print_hex(gaSSMac_DevInfo[i].m_SlotMode, 1);
							SK_print("\r\n");
						}
						
					} else if( mode == 5 ){
						SK_UH i;
						
						SK_print("ECHTBL\r\n");

						for( i = 0; i < SS_MAX_HOPPING_CH_NUM; i++ ){
							SK_print_hex(gaSSMac_ChannelTable[i], 2); SK_print(" ");
						}
						SK_print("\r\n");
						
					} else if( mode == 10 ){
						SS_STATISTICS* stats;
						
						stats = SSMac_GetStats();
						
						if(stats != NULL ){
							SK_print("ESTATS\r\n"); 							
							SK_print("phy.recv:"); SK_print_hex(stats->phy.recv, 8); CRLF;
							SK_print("phy.recv_drop:"); SK_print_hex(stats->phy.recv_drop, 8); CRLF;
							SK_print("phy.send:"); SK_print_hex(stats->phy.send, 8); CRLF;
							SK_print("phy.send_drop:"); SK_print_hex(stats->phy.send_drop, 8); CRLF;
							SK_print("phy.busy:"); SK_print_hex(stats->phy.busy, 8); CRLF;
							SK_print("phy.err:"); SK_print_hex(stats->phy.err, 8); CRLF;
														
							SK_print("mac.recv_bcn:"); SK_print_hex(stats->mac.recv_bcn, 8); CRLF;
							SK_print("mac.recv_data"); SK_print_hex(stats->mac.recv_data, 8); CRLF;
							SK_print("mac.recv_ack:"); SK_print_hex(stats->mac.recv_ack, 8); CRLF;
							SK_print("mac.recv_metabcn:"); SK_print_hex(stats->mac.recv_metabcn, 8); CRLF;
							SK_print("mac.recv_drop:"); SK_print_hex(stats->mac.recv_drop, 8); CRLF;

							SK_print("mac.send_bcn:"); SK_print_hex(stats->mac.send_bcn, 8); CRLF;
							SK_print("mac.send_data;"); SK_print_hex(stats->mac.send_data, 8); CRLF;
							SK_print("mac.send_ack:"); SK_print_hex(stats->mac.send_ack, 8); CRLF;
							SK_print("mac.send_metabcn:"); SK_print_hex(stats->mac.send_metabcn, 8); CRLF;
							SK_print("mac.send_drop:"); SK_print_hex(stats->mac.send_drop, 8); CRLF;
							
							SK_print("sec.counter_err:"); SK_print_hex(stats->sec.counter_err, 8); CRLF;
							SK_print("sec.decode_fail:"); SK_print_hex(stats->sec.decode_fail, 8); CRLF;
							
							SK_print("mem.err:"); SK_print_hex(stats->mem.err, 8); CRLF;
						}
						
					} else if( mode == 254 ){
					  	extern SK_UW gTotalTxTime;
						SK_UW total;
						
						total = sizeof(SS_SLOT_ADDR_DB_ITEM) * DEF_MAX_DEVICE_SIZE;
						total += SK_DATA_MEMBLOCK_NUM * SK_DATA_MEMBLOCK_SIZE;
						SK_print_hex(total, 8); CRLF;
						
						SK_print_hex(sizeof(SK_MCPS_DATA_INDICATION), 8); CRLF;
						SK_print_hex(sizeof(SK_MLME_SCAN_CONFIRM), 8); CRLF;
						//SK_print_hex(SS_BEACON_TRANS_TIME, 8);  CRLF;
						SK_print_hex(sizeof(SS_SLOT_ADDR_DB_ITEM), 8); CRLF;
						SK_print_hex((SS_MAX_FRAME_LEN), 8); CRLF;
						SK_print_hex(SK_CountFreeDataMemory(), 4); CRLF;
						SK_print_hex(gTotalTxTime, 8); CRLF;
						SK_print_hex(sizeof(SS_MHR), 4); CRLF;
						SK_print_hex(SS_MHR_LEN, 4); CRLF;
						SK_print_hex(SS_JOIN_RES_CMD_LEN, 4); CRLF;
						SK_print("\r\n");
						
					} 
					
					SK_print("OK\r\n");
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
										
				} else {
					if(NumOfATParam != 0){
						SK_print("FAIL ER04\r\n");
					}
					gn_nMenuStatus = SAMPLEAPP_MENU_TOP;
				}
			}
			break;
	}

	if( gn_nMenuStatus == SAMPLEAPP_MENU_TOP ){
		gn_nLinePos = 0;
	}	
}


// -------------------------------------------------
// Split input SK command by white space
// -------------------------------------------------
SK_UB GetParam(SK_UB *lineBuf, SK_UB **ATParam, SK_UH bufLen){
	SK_UB param = 0, skip = 0;
	SK_UH len;
	
	len = 0;

	while(*lineBuf != 0x00){
		if((*lineBuf != ' ') && (*lineBuf != 0x00)){
			if(skip == 0){
				ATParam[param] = lineBuf;
				param++;
				if( param > NUM_OF_ATPARAM ) return 255;
			}
			lineBuf++;
			skip = 1;
		} else {
			*lineBuf = 0x00;
			lineBuf++;
			skip = 0;
		}
		len++;
		if( len > bufLen ) return 255;
	}
	return param;
}



// -------------------------------------------------
// Convert Hex str to number
// -------------------------------------------------
SK_UB CharToNumber(SK_UB *param, SK_UW *num, SK_UB len){

	SK_UB no[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','a','b','c','d','e','f'};
	SK_UB i, j;
	SK_UB *p = param;

	*num = 0;
	if(len > 8)
		return ER05;

	for(i=0; i < len; i++, p++){
		for(j = 0; j < 22; j++){
			if(*p == 0x00)
				return SK_E_OK;
			if(strncmp((const char *)p,(const char *)(&no[j]),1) == 0){
				*num <<= 4;
				if(j < 0xA)
					*num |= (no[j] - '0');
				else if(0xA <= j && j < 0x10)
					*num |= (no[j] - 'A' + 10);
				else
					*num |= (no[j] - 'a' + 10);
				break;
			}
		}
		if(j >= 22)
			return ER05;
	}
	return SK_E_OK;
}


// -------------------------------------------------
// Check value range
// -------------------------------------------------
SK_UB ParamCheck(SK_UB *param, SK_UW *num, SK_UB len, SK_UW max, SK_UW min){
	SK_UB res;

	res = CheckDigit(param, len);
	if(res != SK_E_OK){
		return ER06;
	}

	res = CharToNumber(param, num, len);

	if((res != SK_E_OK) || ((*num < min) || (max < *num))){
		return ER06;
	}
	return SK_E_OK;
}


// -------------------------------------------------
// Check whether the null-terminated string ATParam has a length not bigger than num
// -------------------------------------------------
SK_UB CheckDigit(SK_UB *ATParam, SK_UB num){
	SK_UB i;

	for(i = 0; i < 20; i++){
		if(ATParam[i] == 0) break;
	}

	if(i > num){
		return ER06;
	} else {
		return SK_E_OK;
	}
}


// -------------------------------------------------
// Convert hex strings to binary 
// -------------------------------------------------
void HexToBin(SK_UB* bin, SK_UB* hex, SK_UH datalen){
	SK_UH i, hexi;
	SK_UW val;
	
	hexi = 0;
	for( i = 0; i < datalen; i++ ){
		if( hex[hexi] == 0x00 ) break;
		CharToNumber(hex + hexi, &val, 2);
		hexi+=2;
		bin[i] = (SK_UB)val;
	}	
}


// -------------------------------------------------
// Save stack parameters to flash memory
// -------------------------------------------------
SK_BOOL SaveMac(void){
	SK_UB res = 0;
#ifdef USE_SAVE_PARAM
	SK_UB index;
	
	memset(gPerData, 0, PERDATA_SIZE);
	index = 1;
	
	MAC_SET_LONG_B(gPerData + index, SSMac_GetAddress()->Body.Extended.m_Long[0]);
	index+=4;
	
	MAC_SET_LONG_B(gPerData + index, SSMac_GetAddress()->Body.Extended.m_Long[1]);
	index+=4;
	
	
	//timer calibration params
	gPerData[index] = SSMac_GetSlotCalib();
	index++;
	
	MAC_SET_LONG_B(gPerData + index, SSMac_GetFineCalib());
	index+=4;
	
	MAC_SET_WORD_B(gPerData + index, gRTCValue);
	index+=2;
	
	MAC_SET_WORD_B(gPerData + index, gRTCShortValue);
	index+=2;
	//
	
	
	gPerData[index] = sum(&gPerData[1], index-1);
	index++;
	
	gPerData[0] = index;
	
	res = FlashBlockWrite(MAC_PERBLOCK, 0, gPerData, PERDATA_SIZE);
	
#endif
	if( res == 0 ){
		return TRUE;
	} else {
		return FALSE;
	}	
}


SK_BOOL SaveParam(void){
	SK_UB res = 0;
#ifdef USE_SAVE_PARAM
	SK_UB index;
	
	memset(gPerData, 0, PERDATA_SIZE);
	index = 1;

	gPerData[index] = gAutoLoad;
	index++;

	MAC_SET_LONG_B(gPerData + index, SSMac_GetStationId());
	index+=4;

	gPerData[index] = SSMac_GetDeviceType();
	index++; 
	
	gPerData[index] = SSMac_GetBaseChannel();
	index++; 
	
	gPerData[index] = SSMac_GetSlotMode();
	index++; 
	
	gPerData[index] = SSMac_GetHoppingTable();
	index++; 

	gPerData[index] = gAutoJoin;
	index++; 
	
	gPerData[index] = gEchoBack;
	index++; 
	
	gPerData[index] = gPing;
	index++; 
	
	memcpy(gPerData + index, SSMac_GetPSK(), 16);
	index+=16;
	
	gPerData[index] = sum(&gPerData[1], index-1);
	index++;
	
	gPerData[0] = index;

	res = FlashBlockWrite(PERBLOCK, 0, gPerData, PERDATA_SIZE);
	
#endif
	if( res == 0 ){
		return TRUE;
	} else {
		return FALSE;
	}
}


// -------------------------------------------------
// Initialize flash area for parameter saving
// -------------------------------------------------
SK_BOOL EraseParam(void){
	SK_UB res = 0;
#ifdef USE_SAVE_PARAM
	
	memset(gPerData, 0xFF, PERDATA_SIZE);
	res = FlashBlockWrite(PERBLOCK, 0, gPerData, PERDATA_SIZE);
	
#endif
	if( res == 0 ){
		return TRUE;
	} else {
		return FALSE;
	}
}


// -------------------------------------------------
// Load stack parameters from flash memory 
// -------------------------------------------------
SK_BOOL LoadMac(void){
#ifdef USE_SAVE_PARAM
	SK_UB len, res, index;
	SK_UB val8;
	SK_UW val32_1, val32_2;

	memset(gPerData, 0xff, PERDATA_SIZE);

	res = FlashBlockRead(MAC_PERBLOCK, 0, gPerData, PERDATA_SIZE);
	
	if( res != 0 ) return FALSE;

	len = gPerData[0];
	if( len == 0xFF ) return FALSE;
	if( len > PERDATA_SIZE ) return FALSE;
		
	if( gPerData[len-1] != sum(&gPerData[1], len-2) ){
		return FALSE;
	}
	
	index = 1; //length
	
	val32_1 = MAC_GET_LONG_B(gPerData + index);
	index+=4;

	val32_2 = MAC_GET_LONG_B(gPerData + index);
	index+=4;

#ifdef DEBUGOUT
	SK_print_hex(val32_1, 8);
	SK_print_hex(val32_2, 8);
#endif
	
	SK_Base_Init(val32_1, val32_2);
	
	SetupParams();

	//timer calibration params
	val8 = gPerData[index];
	index++;
	SSMac_SetSlotCalib(val8);
	
	val32_1 = MAC_GET_LONG_B(gPerData + index);
	index+=4;
	SSMac_SetFineCalib(val32_1);
	
	gRTCValue = MAC_GET_WORD_B(gPerData + index);
	index+=2;
	
	gRTCShortValue = MAC_GET_WORD_B(gPerData + index);
	index+=2;
	//
	
	return TRUE;
#else
	return FALSE;
#endif
}


SK_BOOL LoadParam(void){
#ifdef USE_SAVE_PARAM
	SK_UB len, res, index;
	SK_UW val32_1;
	//SK_UH val16;
	SK_UB val8;

	memset(gPerData, 0xff, PERDATA_SIZE);

	res = FlashBlockRead(PERBLOCK, 0, gPerData, PERDATA_SIZE);

	if( res != 0 ) return FALSE;

	len = gPerData[0];
	if( len == 0xFF ) return FALSE;
	if( len > PERDATA_SIZE ) return FALSE;
		
	if( gPerData[len-1] != sum(&gPerData[1], len-2) ){
		return FALSE;
	}

	index = 1; //length
	
	gAutoLoad = gPerData[index];
	index++; 

	val32_1 = MAC_GET_LONG_B(gPerData + index);
	index+=4;
	SSMac_SetStationId(val32_1);

	val8 = gPerData[index];
	index++; 
	SSMac_SetDeviceType(val8);
	
	val8 = gPerData[index];
	index++; 
	SSMac_SetBaseChannel(val8);
	
	val8 = gPerData[index];
	index++; 
	SSMac_SetSlotMode(val8);
	
	val8 = gPerData[index];
	index++; 
	SSMac_SetHoppingTable(val8);

	gAutoJoin = gPerData[index];
	index++;
	
	gEchoBack = gPerData[index];
	index++;

	gPing = gPerData[index];
	index++;
	
	SSMac_SetPSK(gPerData + index, 16);
	index+=16;

	return TRUE;
#else
	return FALSE;
#endif
}


SK_BOOL IsAutoLoad(void){
#ifdef USE_SAVE_PARAM
	SK_UB res, len;
	memset(gPerData, 0xff, PERDATA_SIZE);

	res = FlashBlockRead(PERBLOCK, PERBANK, gPerData, PERDATA_SIZE);
	if( res != 0 ) return FALSE;

	len = gPerData[0];
	if( len == 0xFF ) return FALSE;
	
	if( gPerData[1] == 0 ) return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}


SK_UB sum(SK_UB* buf, SK_UH len){
	SK_UH i;
	SK_UB val = 0;
	
	for( i = 0; i < len; i++ ){
		val += buf[i];
	}
	
	return val;
}


void  SK_print_mac_addr(SK_ADDRESS* addr){
	SK_print_hex(addr->Body.m_Raw[3], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[2], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[1], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[0], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[7], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[6], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[5], 2); SK_print(":");
	SK_print_hex(addr->Body.m_Raw[4], 2);
}


void  SK_print_key(SK_UB* data, SK_UB len){
	SK_UB i;
	for( i = 0; i < len; i++ ){
		//SK_print_hex(data[i], 2); 
		SK_print_hex(0, 2);
	}
}


void sk_ping_send(SK_UB len){
	SS_DATA_REQUEST *SdReq;
	SK_UB i;

 	if (SK_AllocDataMemory((SK_VP *)&SdReq) != SK_E_OK) { 
		return;
	}
					
	memset(gSendDataBuffer, 0, SS_PAYLOAD_LEN);
	
	for( i = 0; i < len; i++ ){
		gSendDataBuffer[i] = i;	
	}
	
	SdReq->m_MsduLength = (SK_UB)len;
	memcpy(SdReq->m_Msdu, gSendDataBuffer, (SK_UB)len);

	SdReq->m_Selector = SS_SELECTOR_APP_DATA;
	SdReq->m_TxOptions = 0;
	SdReq->m_TxOptions |= SS_TXOPTIONS_SECURITY;
	SdReq->m_Handle = (SK_UH)(SSMac_GetRand32());

	SdReq->m_DstAddress.m_AddrMode = SK_ADDRMODE_EXTENDED;
	SdReq->m_DstAddress.Body.Extended.m_Long[0] = 0;
	SdReq->m_DstAddress.Body.Extended.m_Long[1] = 0;

	if (SK_PostMessageS(SK_LAYER_SS_MAC, SK_LAYER_APL, SS_DATA_REQUEST_CMD, (SK_VP)SdReq, SSMac_GetSlotNum()) != SK_E_OK) {
		SK_FreeMemory(SdReq);
	}
}


void EventProc(void){
	static SK_UB nCmd,nResID,*pPkt;

	if (SK_GetMessage(SK_LAYER_APL ,&nResID,&nCmd,(SK_VP *)&pPkt)==SK_E_OK) {
		// A message has been received
		switch(nCmd) {
		case SS_DATA_CONFIRM_CMD:{
			SS_DATA_CONFIRM* DataConf;

			DataConf = (SS_DATA_CONFIRM *)pPkt;
			
			#ifdef DEBUG_EVENT
			SK_print("SS_DATA_CONFIRM_CMD\r\n");
			SK_print("  Status:"); SK_print_hex(DataConf->m_Status, 2); CRLF;
			#endif
			
			SK_print("ECONF ");
			SK_print_hex(DataConf->m_Status, 2); SPACE;
			SK_print_mac_addr(&DataConf->m_DstAddress); SPACE;
			SK_print_hex(DataConf->m_Handle, 4); CRLF;
			
			break;
		}
		
		// ------------------- Receiving data
		case SS_DATA_INDICATION_CMD:{
			SK_UB i;
			SS_DATA_INDICATION* SdInd;
			
			SdInd = (SS_DATA_INDICATION *)pPkt;
			
			#ifdef DEBUG_EVENT
			SK_print("SS_DATA_INDICATION_CMD:"); CRLF;
			SK_print("  Src:"); SK_print_mac_addr(&(SdInd->m_SrcAddress)); CRLF;
			SK_print("  Slot:"); SK_print_hex(SdInd->m_RecvSlot, 4); CRLF;
			SK_print("  Idx:"); SK_print_hex(SdInd->m_RecvInSlotIdx, 2); CRLF;
			SK_print("  Selector:"); SK_print_hex(SdInd->m_Selector, 2); CRLF;
			SK_print("  Len:"); SK_print_hex(SdInd->m_MsduLength, 2); CRLF;
			SK_print("  Rssi:"); SK_print_hex(SdInd->m_Rssi, 2); CRLF;

			for( i = 0; i < SdInd->m_MsduLength; i++ ){
				SK_print_hex(SdInd->m_Msdu[i],2); SK_print(" ");
			}
			CRLF;
			#endif
			
			if( SdInd->m_Selector == SS_SELECTOR_APP_DATA ){
				SK_print("ERXDATA ");
				SK_print_mac_addr(&(SdInd->m_SrcAddress)); SPACE;
				SK_print_hex(SdInd->m_RecvSlot, 4); SPACE;
				SK_print_hex(SdInd->m_RecvInSlotIdx, 2); SPACE;
				SK_print_hex(SdInd->m_Selector, 2); SPACE;
				SK_print_hex(SdInd->m_Rssi, 2); SPACE;
				SK_print_hex(SdInd->m_Channel, 2); SPACE;
				SK_print_hex(SdInd->m_MsduLength, 2); SPACE;
				
				for( i = 0; i < SdInd->m_MsduLength; i++ ){
					SK_print_hex(SdInd->m_Msdu[i],2);
				}
				CRLF;
			}
			
			break;
		}

		// ------------------- Receiving beacon
		case SS_BEACON_NOTIFY_INDICATION_CMD:{
 			SS_BEACON_NOTIFY_INDICATION		*BcnInd;
			
			BcnInd = (SS_BEACON_NOTIFY_INDICATION *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_BEACON_NOTIFY_INDICATION_CMD\r\n");
			SK_print("  Version:"); SK_print_hex(BcnInd->m_Version, 2); CRLF;
			SK_print("  Capacity:"); SK_print_hex(BcnInd->m_Capacity, 2); CRLF;
			SK_print("  SlotMode:"); SK_print_hex(BcnInd->m_SlotMode, 2); CRLF;
			SK_print("  ChannelTable:"); SK_print_hex(BcnInd->m_ChannelTable, 2); CRLF;
			SK_print("  StationId:"); SK_print_hex(BcnInd->m_StationId, 8); CRLF;
			SK_print("  Rssi:"); SK_print_hex(BcnInd->m_Rssi, 2); CRLF;
			#endif
			
			if( SSMac_GetDeviceType() == SS_TYPE_STATION ){
			  	#ifdef DEBUG_GPIO_RFTX
				DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
				#endif
			
				SK_print("ETXBCN "); 
				SK_print_hex(BcnInd->m_BSN, 2); SPACE;
				SK_print_hex(BcnInd->m_SlotMode, 2); SPACE;
				SK_print_hex(BcnInd->m_StationId, 8); CRLF; 
				
			  	#ifdef DEBUG_GPIO_RFTX
				DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
				#endif
			} else {
				SK_print("ERXBCN "); 
				SK_print_hex(BcnInd->m_BSN, 2); SPACE;
				SK_print_hex(BcnInd->m_Capacity, 2); SPACE;
				SK_print_hex(BcnInd->m_SlotMode, 2); SPACE;
				SK_print_hex(BcnInd->m_StationId, 8); SPACE;
				SK_print_hex(BcnInd->m_Rssi, 2); CRLF;
				
				if( gPing == TRUE && SSMac_GetSlotNum() != 0xFFFF && SSMac_GetInSlotIdx() != 0xFF ){
					gPingLen++;
					if( gPingLen > SS_PAYLOAD_LEN ){
						gPingLen = 1;
					}
					sk_ping_send(gPingLen);
				}
			}
			
			break;
		}
		
		// ------------------- Receiving meta-beacon
		case SS_META_BEACON_NOTIFY_INDICATION_CMD:{
			SK_UB i;
 			SS_META_BEACON_NOTIFY_INDICATION		*MetaBcnInd;
			
			MetaBcnInd = (SS_META_BEACON_NOTIFY_INDICATION *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_META_BEACON_NOTIFY_INDICATION_CMD\r\n");
			SK_print("  StationId:"); SK_print_hex(MetaBcnInd->m_StationId, 8); CRLF;
			for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
				SK_print("  Current:"); SK_print_hex(MetaBcnInd->m_DevInfo[i].m_CurrentSlot, 4); SPACE;
				SK_print("  Channel:"); SK_print_hex(MetaBcnInd->m_DevInfo[i].m_BaseChannel, 2); SPACE;
				SK_print("  SlotMode:"); SK_print_hex(MetaBcnInd->m_DevInfo[i].m_SlotMode, 1); CRLF;
			}
			#endif
			
			SK_print("ERXMETABCN "); 
			SK_print_hex(MetaBcnInd->m_StationId, 8); SPACE;
			for( i = 0; i < SS_MAX_NIC_NUM; i++ ){
				SK_print_hex(MetaBcnInd->m_DevInfo[i].m_CurrentSlot, 4); SPACE;
				SK_print_hex(MetaBcnInd->m_DevInfo[i].m_BaseChannel, 2); SPACE;
				SK_print_hex(MetaBcnInd->m_DevInfo[i].m_SlotMode, 2); SPACE;
			}
			CRLF;
			
			//自端末のSta IDが0xFFFFFFFFの時だけトラッキングを開始する
			if( SSMac_GetStationId() == 0xFFFFFFFF &&
			   	gTrackingStaId != 0xFFFFFFFF && gTrackingNicIdx != 0xFF && (SK_GETSTATE(TRACKER_STATE) == eTrackIdle) ){
				if( MetaBcnInd->m_StationId == gTrackingStaId && 
					//情報が空のNIC Infoはslotmode=7になっている
					MetaBcnInd->m_DevInfo[gTrackingNicIdx].m_SlotMode != 7){

					gTrackCurrentSlot = MetaBcnInd->m_DevInfo[gTrackingNicIdx].m_CurrentSlot;
					gTrackSlotMode = MetaBcnInd->m_DevInfo[gTrackingNicIdx].m_SlotMode;
					gTrackTargetCh = MetaBcnInd->m_DevInfo[gTrackingNicIdx].m_BaseChannel;
					
					SK_print("ESTAT A0\r\n");
					SK_SETSTATE(eTrackStart, TRACKER_STATE);
				}
			}
			
			break;
		}
		
		// ------------------- Beacon loss
		case SS_SYNC_INDICATION_CMD:{
			SS_SYNC_INDICATION* SyncInd;
				
			SyncInd = (SS_SYNC_INDICATION *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_SYNC_INDICATION_CMD:"); CRLF;
			SK_print(" Status:"); SK_print_hex(SyncInd->m_Status, 2); CRLF;
			#endif
			
			SK_print("ESTAT ");
			SK_print_hex(SyncInd->m_Status, 2); CRLF;
			
			if( SyncInd->m_Status == SS_MAC_SYNC_START ){
				if( SSMac_GetInSlotIdx() == 0xFF && gAutoJoin == TRUE ){
					PostJoinReq(SSMac_GetStationId());	
				}
			}

			break;
		}
		
		// ------------------- Status notification
		case SS_COMM_STAT_INDICATION_CMD:{
			SS_COMM_STAT_INDICATION* CommInd;

			CommInd = (SS_COMM_STAT_INDICATION *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_COMM_STAT_INDICATION_CMD:"); CRLF;
			SK_print(" Status:"); SK_print_hex(CommInd->m_Status, 2); CRLF;
			#endif

			SK_print("ESTAT ");
			SK_print_hex(CommInd->m_Status, 2); CRLF;
			
			break;
		}
	
		// ------------------- Join result
		case SS_JOIN_CONFIRM_CMD:{
			SS_JOIN_CONFIRM* JoinConf;
				
			JoinConf = (SS_JOIN_CONFIRM *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_JOIN_CONFIRM_CMD:"); CRLF;
			SK_print(" Status:"); SK_print_hex(JoinConf->m_Status, 2); CRLF;
			SK_print(" Slot:"); SK_print_hex(JoinConf->m_SlotNum, 4); CRLF;
			SK_print(" Idx:"); SK_print_hex(JoinConf->m_InSlotIdx, 2); CRLF;
			#endif

			SK_print("EJOIN ");
			SK_print_hex(JoinConf->m_Status, 2); SPACE;	
			SK_print_hex(JoinConf->m_SlotNum, 4); SPACE;	
			SK_print_hex(JoinConf->m_InSlotIdx, 2); CRLF;	

			break;
		}
		
			// ------------------- Ack indication
		case SS_ACK_INDICATION_CMD:{
			SS_ACK_INDICATION* AckInd;

			AckInd = (SS_ACK_INDICATION *)pPkt;

			#ifdef DEBUG_EVENT
			SK_print("SS_ACK_INDICATION_CMD:"); CRLF;
			SK_print(" Status:"); SK_print_hex(AckInd->m_FramePending, 1); CRLF;
			#endif

			SK_print("EACK ");
			SK_print_hex(AckInd->m_FramePending, 2); CRLF;

			break;
		}
		
		case SK_MCPS_DATA_INDICATION_CMD:{
			SK_MCPS_DATA_INDICATION		*MdInd;
			SK_UH data_len;
			SK_UH i;
	
			MdInd = (SK_MCPS_DATA_INDICATION *)pPkt;
			
			data_len = MdInd->m_MsduLength; 

			SK_print("SK_MCPS_DATA_INDICATION_CMD\r\n");
			SK_print_hex(data_len, 4);
			CRLF;

			for( i = 0; i < data_len; i++ ){
				SK_print_hex(MdInd->m_Msdu[i],2); SK_print(" ");
			}
			CRLF;

			break;
		}

		case SK_MCPS_DATA_CONFIRM_CMD:{
			SK_MCPS_DATA_CONFIRM* lMdCon;
			extern SK_UB gNumOfBackOffs;
								
			lMdCon = (SK_MCPS_DATA_CONFIRM *)pPkt;

			SK_print("SK_MCPS_DATA_CONFIRM:"); CRLF;
			SK_print(" Status:"); SK_print_hex(lMdCon->m_Status, 2); CRLF;

			break;
		}
		
		} //end of switch(nCmd) 		
		// Free the memory block
		if (pPkt  != NULL) SK_FreeMemory(pPkt);
	}

	// -------------------------------------------------
	//SK_STATEEND(EVENT); 	// Terminating the state machine
	return;
}


// -------------------------------------------------
//   UART input/output
// -------------------------------------------------
void _putc(char ch){
	UART_PutChar(ch);
}


SK_H _getc(void){
	return UART_GetChar();
}


SK_BOOL PostJoinReq(SK_UW sta_id){
	SS_JOIN_REQUEST *SjReq;	
	
 	if (SK_AllocDataMemory((SK_VP *)&SjReq) != SK_E_OK) { 
		return FALSE;
	}

	SjReq->m_StationID = sta_id;

	if (SK_PostMessage(SK_LAYER_SS_MAC, SK_LAYER_APL, SS_JOIN_REQUEST_CMD, (SK_VP)SjReq) != SK_E_OK) {
		SK_FreeMemory(SjReq);
		return FALSE;
	}
	
	return TRUE;
}
