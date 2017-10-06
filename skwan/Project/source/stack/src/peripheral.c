/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: Hardware dependant routins for ML7416s
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

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

#include "compiler_options.h"

#include <string.h>

#include "ML7416S.h"
#include "ML7404.h"
#include "mcu.h"
#include "uart.h"
#include "modecnt.h"
#include "stdport.h"
#include "gpio.h"
#include "syscon.h"
#include "spi2.h"
#include "flash.h"
#include "tmr.h"

#include "skyley_stack.h"
#include "peripheral.h"
#include "hardware.h"
#include "rf_interface.h"

//Config debug port
//GPIOA[0], GPIOA[1] output
#define USE_DEBUG_PORT	1

/* -------------------------------------------------
IRQ priority memo
Symbol timer, DMAC:1
UART RX, SysTick timer:2
RF:3
-------------------------------------------------*/


// -------------------------------------------------
//   Private functions
// -------------------------------------------------
static void 	init_system(void);
static void	init_uart(void);
static void	init_uartbuf(void);
static void 	init_gpio(void);
static void 	init_irq(void);

static void 	init_tmr( void );
static void 	start_tmr(uint32_t timer_cnt);
static void 	stop_tmr( void );

static void 	Adjust_960(void);
static void 	SleepTimer_Stop(void);
static void 	SleepTimer_Start(SK_UH counts);
static void 	SleepTimer_Interrupt(void);
static void 	SleepTimer_Finalize(void);
static void 	Timer_Interrupt(void);


// -------------------------------------------------
//   UART0 settings and ring buffer
// -------------------------------------------------
#define	REF_CLK           (40566784UL)
#define	BAUDRATE_115200   (((REF_CLK*2u/115200u/16u)+1u)/2u)
#define UART_DLS_8BIT     (0x3u << 0)
#define UART_STOP_1BIT    (0x0u << 2)
#define UART_PEN_DISABLE  (0x0u << 3)

static UARTCtrlParam_Type CtrlParam;

#define		SK_UART_RX_BUFSIZ			512			// Serial port buffer (must be 2^n)

SK_UB gaSK_UART_RXbuf[SK_UART_RX_BUFSIZ];			// Ring buffer for Recv
static SK_UH gnSK_UART_RXst;						// Buffer start
static SK_UH gnSK_UART_RXed;						// Buffer end


// -------------------------------------------------
//   Working variables
// -------------------------------------------------
//TRUE if RX ring buffer is full
SK_BOOL gUARTOverRun; 

//RF Interrupt handling
static irq_handler rf_irq_handler = NULL;
static SK_UB rf_in_isr;

//SPI DMA control
volatile SK_BOOL dmac_spi2_done = FALSE;


// -------------------------------------------------
//   MCU sleep control
// -------------------------------------------------
volatile SK_BOOL gMCUSleeping;
volatile SK_BOOL gnPHY_SleepTimerCompleted;
volatile SK_BOOL gContinuousSleep;
volatile SK_BOOL gWakeTriggered;
SK_UH gWakeTargetSlot;
SK_BOOL gLastOneShot;


// -------------------------------------------------
//   Time slot management
// -------------------------------------------------
//10.01ms for ML7416S xtal32
#define CLK_10m_VALUE_32XTAL (327)
#define CLK_10m_VALUE_32XTAL_Adjust (318)

//960ms Timer adjustment
static SK_UB gSlotUnitCount = 0;
static SK_BOOL gAdjusted = FALSE;

//Sleep timer count value
SK_UH gRTCValue;
SK_UH gRTCShortValue;

//0: timer is used for 10msec systick
//1: timer is used for 960msec sleep tick
SK_UB gTimerMode;


// -------------------------------------------------
//   Initialize peripheral
// -------------------------------------------------
void 	init_peripheral(void)
{
	/*クロック初期設定 メインクロックをPLLに設定*/
	init_system();

	//Flash controller
	FLC_Init( UX_FLC0 );

#if 1
	//MCU Power save settings
	syscon_writePPM1( ~(0x00000001<<20) ); //TimerA Enable
	syscon_writePPM2( ~(0x00000001<<20) ); //TimerA Enable
	////MODE_CNT0->MODE_CNT_DPSLP    = dpslp_val;
	////MODE_CNT0->MODE_CNT_FLDPSTB  = fldpstb_val;
	//syscon_writePCLKDIS( ~(0x00000001<<20) ); // org = 0x00000000
#endif
  
	UART_Initialize();
	
	spi2Init(USE_DMAC);
	//spi2Init(UNUSE_DMAC);
	
	//clock sync calibration value
	SSMac_SetFineCalib(0x5DC0);
	//
	  
	//Setup timer
	gRTCValue = 0x7AFC; //960.9ms
	gRTCShortValue = 0x7096; //880.0ms
	
	SleepTimer_Finalize();
	Timer_Initialize();
	//
	
	init_gpio();
	init_irq();
	
	//20151205 add
	//add HW AES support
	//aes_init( AES0 );

	gUARTOverRun 	= FALSE;
	dmac_spi2_done 	= FALSE;
	gMCUSleeping 	= FALSE;
	
	ml7404_reset_seeds();
}


static void init_system(void){
	INIT_PARAM_Type init_param = {
		MODECNT_HSCR_ENABLE | MODECNT_CR32K_ENABLE | MODECNT_XTAL32K_ENABLE,
		MODECNT_PLL_WTCR_25 | MODECNT_XTAL_WTCR_3_125m | MODECNT_CR32K_WTCR_10 | MODECNT_HSCR_WTCR_1200,
		MODECNT_PLL_DONE_NOT_MASK | MODECNT_XTAL32K_DONE_NOT_MASK | MODECNT_CR32K_DONE_NOT_MASK | MODECNT_HSCR_DONE_NOT_MASK
	};

	PllCtrlPARAM_Type pllctrl_param = {
		MODECNT_PLL_PDN0_DISABLE | MODECNT_PLL_BYPASS0_DISABLE | (0x4D6 << 2) | MODECNT_PLL_DREF0_1,
		MODECNT_PLL_XTAL32K,
		MODECNT_PLL_DIV_1
	};

	ClkselPARAM_Type clksel_param = { MODECNT_MAIN_PLL , MODECNT_SUB_XTAL32K };

	modecnt_InitClk(&init_param);
	modecnt_StartPll(&pllctrl_param);
	modecnt_SelClk(&clksel_param);
}


// ===========================================================================================
//   Timer
// ===========================================================================================
void TMRA_TIMER1_IRQHandler( void )
{	
	Timer_Interrupt();
}


void init_tmr( void )
{
	MODE_CNT0->MODE_CNT_CLKGEN0 &= ~(0x00000001UL<<4);  ////CLKGEN[4] = 0, A subclock source is changed into XTAL32KHz
	MODE_CNT0->MODE_CNT_CLKGEN1|= 0x00000002UL;     //The clock source of TimerA is changed into a subclock.

	tmr_clrINTSTATUS( UX_TMRA_TIMER1 );

	NVIC_ClearPendingIRQ( TMRA_TIMER1_IRQn );
	NVIC_EnableIRQ( TMRA_TIMER1_IRQn );
}


void start_tmr( uint32_t timer_cnt )
{
	TMR_MODE_Type mode;

	mode.ctrl = TMR_INT_NOT_MASK | TMR_MOD_USER;
	mode.lval = timer_cnt;
	tmr_init( UX_TMRA_TIMER1, &mode );

	tmr_start( UX_TMRA_TIMER1 );
}


void stop_tmr( void )
{
	tmr_stop( UX_TMRA_TIMER1 );
	tmr_clrINTSTATUS( UX_TMRA_TIMER1 );   /* Status Clear */
}

// -------------------------------------------------
//   Timer init
// -------------------------------------------------
void Timer_Initialize(void){
	//clear 960msec calibration context
	gSlotUnitCount = 0;
	gAdjusted = FALSE;
	
	//indicates 10msec systick is running
	//gTimerMode=1 ==> 960msec sleep timer is running
	gTimerMode = 0;
	
	init_tmr();
	start_tmr(CLK_10m_VALUE_32XTAL);
}


static void Timer_Interrupt(void){
	//#ifdef DEBUG_GPIO_TIMER
	//DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));	
	//#endif
	
	tmr_clrINTSTATUS( UX_TMRA_TIMER1 );
	
	if( gTimerMode == 0 ){ //10msec systick is running
		SK_IncrementTimeTick(10);
		
		// 960msecカウントの最後の1 tickを補正する
		Adjust_960();
		//
	}  else if( gTimerMode == 1 ){
	  
		SleepTimer_Interrupt();
	
	}
}


static void Adjust_960(void){
	gSlotUnitCount++;

	if( gSlotUnitCount == 95 ){
		//#ifdef DEBUG_GPIO_TIMER
		//DebugPort_Set(0, ((~DebugPort_Get(0)) & 0x01));	
		//#endif
		
		gAdjusted = TRUE;
		stop_tmr();
		start_tmr(CLK_10m_VALUE_32XTAL_Adjust);
	} else {
	  	if( gAdjusted == TRUE ){
			//#ifdef DEBUG_GPIO_TIMER
			//DebugPort_Set(0, ((~DebugPort_Get(0)) & 0x01));	
			//#endif
			
			gAdjusted = FALSE;
			stop_tmr();
			start_tmr(CLK_10m_VALUE_32XTAL);
		}
	}
	
	if( gSlotUnitCount == 96 ){
		gSlotUnitCount = 0;
	}
}



// -------------------------------------------------
//   Sleep timer (960msec) stop and go
// -------------------------------------------------
void SleepTimer_Initialize(void){
	stop_tmr();
	init_tmr();
	//should not start here
}


void SleepTimer_Go(SK_UH target){
	if( target <= SSMac_GetCurrentSlot() ){
		return;	
	}

	//stop systick here
	TIMER_LOCK();
	
	gTimerMode = 1; //indicates 10msec -> 960msec switch

	//Setup sleep timer context
	gWakeTargetSlot 	= target;
	gnPHY_SleepTimerCompleted = FALSE;
	gContinuousSleep 	= TRUE;
	gLastOneShot 		= FALSE;
	gWakeTriggered 		= FALSE;

	//Kick sleep timer
	SleepTimer_Stop();
	SleepTimer_Start( gRTCValue );
		
	MCU_Sleep();
	
	while(gContinuousSleep == TRUE){
		__WFI();
	}
}


void SleepTimer_Start(SK_UH counts){
	//Use TimerA as sleep timer
	start_tmr(counts);
}


void SleepTimer_Stop(void){
	SleepTimer_Initialize();
}


//
// Initialize sleep timer context
//
static void SleepTimer_Finalize(void){
	gnPHY_SleepTimerCompleted 	= TRUE;
	gContinuousSleep 			= FALSE;
	gLastOneShot 				= FALSE;
	gWakeTriggered 				= FALSE;
	gTimerMode 					= 0;  
}


/*
	... 960 -> 960 -> 880 -> wakeup
ターゲットスロットの80msec手前でウェイクアップするよう制御
*/
static void SleepTimer_Interrupt(void){
	#ifdef DEBUG_GPIO_SLEEP
	DebugPort_Set(0, ((~DebugPort_Get(0)) & 0x01));
	#endif
	
	if( gLastOneShot == FALSE ){
		SSMac_IncCurrentSlot(FALSE);
		SK_IncrementEventTick(960);
	} else {
		SK_IncrementEventTick(880);
	}
	
	//wake interrupt triggered
	if( gLastOneShot == FALSE && gWakeTriggered == TRUE ){
		//Start stack running again
		SleepTimer_Finalize();
		
		SleepTimer_Stop();
		
		MCU_Wakeup(); 
		return;
	}
	
	//2 slots before wakeup slot, trigger last shot
	if( gWakeTargetSlot <= SSMac_GetCurrentSlot() && gLastOneShot == FALSE ){
		gLastOneShot = TRUE;
		SleepTimer_Stop();
		SleepTimer_Start( gRTCShortValue );
		
	} else if( gLastOneShot == TRUE ){
	  	SleepTimer_Finalize();

		SleepTimer_Stop();
		
		//60msec * 14 + 10msec * 4 == 880
		SSMac_SetSlotUnitTick(14, 4);
		
		MCU_Wakeup(); //start timers here
		return;
	}
}


// ===========================================================================================
//   UART
// ===========================================================================================

static void init_uart(void)
{
	UART_PARAM_Type param;

	/*ポート設定 UART0(GPIO8-11)*/
	port_uart(UART0_8_11);

/* =============================== */
/* Data Transmission               */
/* Baud rate : 115200bps           */
/* Character size : 8bit           */
/* Parity : non                    */
/* STOP bit : 1bit                 */
/* Hard flow control : disable     */
/* =============================== */
/*UART通信用初期設定*/

	param.dlh = (BAUDRATE_115200 >> 8) & 0xFFu;
	param.dll = BAUDRATE_115200 & 0xFFu;
	param.lpdlh = (BAUDRATE_115200 >> 8) & 0xFFu;
	param.lpdll = BAUDRATE_115200 & 0xFFu;
	param.lcr = UART_BC_TXD_OUT | UART_STICK_PARITY_DISABLE | UART_EPS_ODD | UART_PEN_DISABLE | UART_STOP_1BIT | UART_DLS_8BIT;
	param.mcr = UART_SIRE_DISABLE | UART_AFCE_DISABLE | UART_LB_DISABLE | UART_OUT2_H | UART_OUT1_H | UART_RTS_H | UART_DTR_H;
	param.far = UART_FAR_DISABLE;
	param.fcr = UART_RT_ONE | UART_TET_EMPTY | UART_XFIFOR_RESET | UART_RFIFOR_RESET | UART_FIFOE_ENABLE;
	param.ier = UART_PTIME_ENABLE | UART_EDSSI_NO_MASK | UART_ELSI_NO_MASK | UART_ETBEI_NO_MASK | UART_ERBFI_NO_MASK;

	uart_initCtrlParam(UX_UART0, &param, &CtrlParam);

	//uart_readINTSTATUS(UX_UART0);

	uart_clrbitIE( UX_UART0, UART_BITMASK_ETBEI );

	/* interrupt enable */
	NVIC_SetPriority( UART0_IRQn, 2 );
	NVIC_ClearPendingIRQ( UART0_IRQn );
	NVIC_EnableIRQ( UART0_IRQn );
}


/**  
	UART0の割込みハンドラ
ToDo: ORE etc uart errorのハンドリング
*/

void UART0_IRQHandler(void)
	{
	uint8_t iir= (uint8_t)(UX_UART0->UARTn_IIR & 0x0F);
	uint8_t input;

	switch(iir)
	{
	case UART_IID_INT_LV1:
		UX_UART0->UARTn_LSR;
		gUARTOverRun = TRUE;
		break;
	case UART_IID_INT_LV4:
		UX_UART0->UARTn_MSR;
		break;
	case UART_IID_INT_LV3:
		break;
	case UART_IID_INT_LV2_1:
		input = UX_UART0->UARTn_RBR;
		UART_RxInt((SK_UB)input);
		break;
	default:
		break;
	}
}


// -------------------------------------------------
//   UART Initialize
// -------------------------------------------------
void UART_Initialize(void) {
	init_uartbuf();
	init_uart();
}


// -------------------------------------------------
//   Input one char
// -------------------------------------------------
void UART_RxInt(SK_UB ch) {
	SK_UH				next;

	// Store buffer
	next = (gnSK_UART_RXed + 1) & (SK_UART_RX_BUFSIZ - 1);
	if (gnSK_UART_RXst != next) {
		gaSK_UART_RXbuf[gnSK_UART_RXed] = ch;
		gnSK_UART_RXed = next;
	} else {
		gUARTOverRun = 1;	
	}
}


// -------------------------------------------------
//   Output one char
// -------------------------------------------------

void UART_PutChar(SK_UB ch) {
	//TX one charactor and wait TE
	uart_writeDATA(UX_UART0, ch);
	
	while (uart_isTransmit(UX_UART0) != UART_TRANS_FIN) {
		;
	}
}


// -------------------------------------------------
//   Read one char from ring buffer
// -------------------------------------------------

SK_H UART_GetChar(void) {
	SK_UB ch;
	
	if (gnSK_UART_RXst == gnSK_UART_RXed) {
		return -1;
	} else {
		ch = gaSK_UART_RXbuf[gnSK_UART_RXst];
		gnSK_UART_RXst = (gnSK_UART_RXst + 1) & (SK_UART_RX_BUFSIZ - 1);
		return (SK_H)ch;
	}
}


// -------------------------------------------------
//   Check num of chars in the ring buffer 
// -------------------------------------------------
SK_H UART_GetLen(void) {
	SK_H	len;

	len = (SK_H)gnSK_UART_RXed - (SK_H)gnSK_UART_RXst;
	if (len < 0) { len += SK_UART_RX_BUFSIZ; }
	return len;
}


// -------------------------------------------------
//   Initialize UART ring buffer
// -------------------------------------------------
static	void	init_uartbuf(void)
{
	gnSK_UART_RXst = 0;
	gnSK_UART_RXed = 0;

	memset(gaSK_UART_RXbuf, 0, SK_UART_RX_BUFSIZ);
}


// ===========================================================================================
//   SPI
// ===========================================================================================

// -------------------------------------------------
//   one SK_UB SPI read/write
// -------------------------------------------------
char	put_char_to_spi(char c)
{
    /*
		 -> use spiRegAccess instead of this
	*/
	return 0;
}


char	get_char_from_spi(void)
{
    /*
		 -> use spiRegAccess instead of this
	*/
	return 0;
}



// ===========================================================================================
//   GPIO
// ===========================================================================================
static	void	init_gpio(void)
{		
	//Config debug port
	//GPIOA[0], GPIOA[1] output
	#ifdef USE_DEBUG_PORT
	MODE_CNT0->MODE_CNT_IOSET_GPIOA0&= ~(SET_IO_EN);  // GPIOA[0]: SET_IO_EN clear
	UX_GPIO0->GPIOn_SWPORTA_DDR|= (0x00000001 << 0);
	
	MODE_CNT0->MODE_CNT_IOSET_GPIOA1&= ~(SET_IO_EN);  // GPIOA[1]: SET_IO_EN clear
	UX_GPIO0->GPIOn_SWPORTA_DDR|= (0x00000001 << 1);
	#endif
	
	//RESETN for RF
	MODE_CNT0->MODE_CNT_IOSET_GPIOC15 = 0x00000081;  // as GPIO
	UX_GPIO2->GPIOn_SWPORTA_DDR|= (0x00000001 << 15); //GPIOC[15] output

	//REGPDIN端子：GPIOC[13]をLow出力する処理が必要です
	MODE_CNT0->MODE_CNT_IOSET_GPIOC13 &= ~(SET_IO_EN); // GPIOC[15]: SET_IO_EN clear
	UX_GPIO2->GPIOn_SWPORTA_DDR |= (0x00000001 << 13); // GPIOC[13] output
	UX_GPIO2->GPIOn_SWPORTA_DR &= ~(0x00000001 << 13); // Low
	
	
	//ML7416S：RFインターフェース端子の有効化
	//spi2Init(USE_DMAC); 内で実行しているのでここでは不要
	#if 0
	MODE_CNT0->MODE_CNT_IOSET_SDI_CPU = 0x000000C0;
	MODE_CNT0->MODE_CNT_IOSET_SDO_CPU = 0x00000185;
	MODE_CNT0->MODE_CNT_IOSET_SCEN_CPU = 0x000000C0;
	MODE_CNT0->MODE_CNT_IOSET_SCLK_CPU = 0x000000C0;
	#endif
}

/*
ML7404のRESET端子はML7416SのGPIOC15と接続されています。
ML7416SではデフォルトでGPIOC15はLow出力されているため、そのまま接続しているとリセット状態が続いてしまいます。
そのため、以下の設定をしリセットのパルスを入れる必要があります。
*/
void	gpio_rf_chip_enable(void)
{
	//GPIOC[15] hi
	UX_GPIO2->GPIOn_SWPORTA_DR |= (0x00000001 << 15);
}

void	gpio_rf_chip_disable(void)
{
	//GPIOC[15] low
	UX_GPIO2->GPIOn_SWPORTA_DR &= ~(0x00000001 << 15);
}

void	gpio_rf_spi_enable(void)
{
	/* nothing to do */
}

void	gpio_rf_spi_disable(void)
{
	/* nothing to do */
}

//GPIOA[0] for debug port
void gpio_test_port(SK_BOOL on_off){
	#ifdef USE_DEBUG_PORT
	if( on_off == TRUE ){
		UX_GPIO0->GPIOn_SWPORTA_DR |= (0x00000001 << 0);
	} else {
		UX_GPIO0->GPIOn_SWPORTA_DR &= ~(0x00000001 << 0);
	}
	#endif
}

/*
  SINTNは、GPIO3(データシートでは、GPIOD)のBit0に接続されておりベクタ番号はIRQ[29]
*/
SK_UH gpio_rf_int_port(void){
	return rf_in_isr;
}


// ===========================================================================================
//   RF IRQ 
// ===========================================================================================
// -------------------------------------------------
//   Setup RF IRQ 
// -------------------------------------------------
void init_irq(void){
	/*NVICの設定*/
	NVIC_SetPriority( RF_IRQn, 3 );
	NVIC_ClearPendingIRQ( RF_IRQn );
	NVIC_EnableIRQ( RF_IRQn );
	
	/* SINTNの割込み設定 */
	UX_GPIO3->GPIOn_INTTYPE_LEVEL= 0x00000001UL; /* エッジ割込み */
	UX_GPIO3->GPIOn_INT_POLARITY= 0; /* 立下りエッジ */
	UX_GPIO3->GPIOn_DEBOUNCE= 0x00000001UL;  /* テ゛ハ゛ウンス回路有効 */
	UX_GPIO3->GPIOn_LS_SYNC= 0x00000001UL;  /* レヘ゛ルセンシティフ゛割り込みを同期して出力 */
	UX_GPIO3->GPIOn_INTEN= 0x00000001UL;	/* 割込み有効 */
}

void	irq_rf_enable(void)
{
	NVIC_EnableIRQ( RF_IRQn );
}


void	irq_rf_disable(void)
{
	NVIC_DisableIRQ( RF_IRQn );
}


SK_BOOL	irq_rf_in_progress(void)
{
 	if( rf_in_isr == 1 ){
		return TRUE;
	} else {
	  	return FALSE;
	}
}


void	irq_rf_regist_handler(irq_handler handler)
{
	rf_irq_handler = handler;
}


void RF_IRQHandler(void)
{
	rf_in_isr = 1;
	
	/* 割込みクリア */
	UX_GPIO3->GPIOn_PORTA_EOI = 0x00000001UL;

	if( rf_irq_handler != NULL ){
		(*rf_irq_handler)();
	}

	rf_in_isr = 0;
}


//20150113 DMAC SPI2 support
// ===========================================================================================
//   SPI2 DMAC
// ===========================================================================================
void start_dmac_spi2(void){
	dmac_spi2_done = 0;
	
	NVIC_ClearPendingIRQ(DMAC0_IRQn);
	NVIC_SetPriority(DMAC0_IRQn, 1);
	NVIC_EnableIRQ(DMAC0_IRQn) ;
}

void end_dmac_spi2(void){
	while(dmac_spi2_done==0);
	
	NVIC_DisableIRQ(DMAC0_IRQn);
	NVIC_ClearPendingIRQ(DMAC0_IRQn);
}


void DMAC0_IRQHandler(void)
{
	uint32_t cnt;
    
	if(dmac_readDAR(UX_DMAC0_CH1) == 0x00000000u){
	    /* SPI2 WRITE */
		if((dmac_readSTAT_TFR(UX_DMAC0_COMM) & DMAC_STAT_CH0) != 0){
			uint32_t irqflg;
			irqflg = dmac_readSTAT_INT(UX_DMAC0_COMM);
			if( (irqflg & DMAC_INTMASK_TFR) == DMAC_INTMASK_TFR ){
				dmac_clrbitMASK_TFR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_TFR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrTFR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrTFR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrBLOCK(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrBLOCK(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrSRC_TRAN(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrSRC_TRAN(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrDST_TRAN(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrDST_TRAN(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH1);
			}
			if( (irqflg & DMAC_INTMASK_ERR) == DMAC_INTMASK_ERR ){
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH1);
			}
		    
		    /* DMAC 処理停止 */
			dmac_clrbitCH_EN(UX_DMAC0_COMM, DMAC_CH0);
			dmac_clrbitCH_EN(UX_DMAC0_COMM, DMAC_CH1);
		    
			for(cnt = 0u; cnt < 0xffu; cnt++){	 /* Wait */
				;
			}
		    
			spi_disable(UX_SPI2);										/* 転送禁止 */
			spi_writeCR(UX_SPI2, spi_readCR(UX_SPI2)&~SPI_SSNL_L);		/* SSn=Lowﾃﾞｨｾｰﾌﾞﾙ */
			spi_clrFIFO(UX_SPI2);
			/* SPI2受信FIFOダミーリード */
			for(cnt = 0u; cnt < (dmac_readCTLH(UX_DMAC0_CH0)&0x00000fffu); cnt++){
				spi_readDATA(UX_SPI2);
				spi_clrALLINTSTATUS(UX_SPI2);
				spi_clrFIFO(UX_SPI2);
			}
		}
	}
	else{
		/* SPI2 READ */
		if((dmac_readSTAT_TFR(UX_DMAC0_COMM) & DMAC_STAT_CH1) != 0){
			uint32_t irqflg;
			irqflg = dmac_readSTAT_INT(UX_DMAC0_COMM);
			if( (irqflg & DMAC_INTMASK_TFR) == DMAC_INTMASK_TFR ){
				dmac_clrbitMASK_TFR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_TFR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrTFR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrTFR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrBLOCK(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrBLOCK(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrSRC_TRAN(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrSRC_TRAN(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrDST_TRAN(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrDST_TRAN(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH1);
			}
			if( (irqflg & DMAC_INTMASK_ERR) == DMAC_INTMASK_ERR ){
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrbitMASK_ERR(UX_DMAC0_COMM, DMAC_CH1);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH0);
				dmac_clrERR(UX_DMAC0_COMM, DMAC_CH1);
			}

			/* DMAC 処理停止 */
			dmac_clrbitCH_EN(UX_DMAC0_COMM, DMAC_CH0);
			dmac_clrbitCH_EN(UX_DMAC0_COMM, DMAC_CH1);

			spi_disable(UX_SPI2);									/* 転送禁止 */
			spi_writeCR(UX_SPI2, spi_readCR(UX_SPI2)&~SPI_SSNL_L);	/* SSn=Lowﾃﾞｨｾｰﾌﾞﾙ */

			spi_clrFIFO(UX_SPI2);
		}
	}
	dmac_spi2_done = 1;
}


// -------------------------------------------------
//   Sleep
// -------------------------------------------------
void MCU_Sleep(void) {
	#ifdef DEBUG_GPIO_SLEEP
	DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
	#endif

	gMCUSleeping = TRUE;

#if 0
	MODE_CNT0->MODE_CNT_IOSET_SINT_CPU		= 0x00000285;
	MODE_CNT0->MODE_CNT_IOSET_SDI_CPU		= 0x00000284;
	MODE_CNT0->MODE_CNT_IOSET_SCEN_CPU		= 0x00000284;
	MODE_CNT0->MODE_CNT_IOSET_SCLK_CPU		= 0x00000284;
#else
	MODE_CNT0->MODE_CNT_IOSET_SDO_CPU		= 0x00000285;
	MODE_CNT0->MODE_CNT_IOSET_SCEN_CPU		= 0x00000288;
#endif

	NVIC_ClearPendingIRQ( RF_IRQn );
	NVIC_DisableIRQ( RF_IRQn );


	*( ( volatile unsigned long *) 0x40050078UL ) = 0x00002020;
	SCB->SCR |= (0x00000004); //deepsleep
	__WFI();
}


//実行=40us
void MCU_Wakeup(void) {
	#ifdef DEBUG_GPIO_SLEEP
	DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
	#endif
	
	gMCUSleeping = FALSE;
	gWakeTriggered = FALSE;

#if 1
	init_system();
	while( (syscon_readPCLKEN() & (1L<<4)) == 0 ){ ; } //UART0 ready?
	while( (syscon_readPCLKEN() & (1L<<26)) == 0 ){ ; } //SPI0 ready?
	while( (syscon_readPCLKEN() & (1L<<30)) == 0 ){ ; } //DMA ready?
	
	MODE_CNT0->MODE_CNT_IOSET_SINT_CPU		= 0x00000181;
	MODE_CNT0->MODE_CNT_IOSET_SDI_CPU		= 0x000000C0;
	MODE_CNT0->MODE_CNT_IOSET_SDO_CPU 		= 0x00000185;
	MODE_CNT0->MODE_CNT_IOSET_SCEN_CPU		= 0x000000C0;
	MODE_CNT0->MODE_CNT_IOSET_SCLK_CPU		= 0x000000C0;

	//Restart 10msec timer
	Timer_Initialize();
	
	//Re-init peripherals
	UART_Initialize();
	spi2Init(USE_DMAC);	
	//aes_init( AES0 );
	
	init_gpio();
	init_irq();
	
	//実行速度測定用
	//#ifdef DEBUG_GPIO_SLEEP
	//DebugPort_Set(1, ((~DebugPort_Get(1)) & 0x01));
	//#endif
#endif
}


SK_BOOL RF_Sleep(void){
	#ifdef DEBUG_GPIO_SLEEP
	DebugPort_Set(0, FALSE);
	#endif

	ml7404_sleep();

	return TRUE;
}


void RF_Wakeup(void){
	#ifdef DEBUG_GPIO_SLEEP
	DebugPort_Set(0, TRUE);
	#endif

	ml7404_wakeup();
}



static SK_UB gDbgPort1 = 0;
static SK_UB gDbgPort2 = 0;

void DebugPort_Set(SK_UB num, SK_BOOL flag){
	if( flag == TRUE ){
		if( num == 0 ){
			UX_GPIO0->GPIOn_SWPORTA_DR |= (0x00000001 << 0); //GPIOA[0]
			gDbgPort1 = 1;
		} else {
			UX_GPIO0->GPIOn_SWPORTA_DR |= (0x00000001 << 1); //GPIOA[1]
			gDbgPort2 = 1;
		}
	} else {
		if( num == 0 ){
			UX_GPIO0->GPIOn_SWPORTA_DR &= ~(0x00000001 << 0); //GPIOA[0]
			gDbgPort1 = 0;
		} else {
			UX_GPIO0->GPIOn_SWPORTA_DR &= ~(0x00000001 << 1); //GPIOA[1]
			gDbgPort2 = 0;
		}
	}
}


SK_UB DebugPort_Get(SK_UB num){
	if( num == 0 ){
		return gDbgPort1;
	} else {
		return gDbgPort2;
	}
}



void TimerLock(void){
	NVIC_DisableIRQ( TMRA_TIMER1_IRQn );
}


void TimerUnlock(void){
	NVIC_EnableIRQ( TMRA_TIMER1_IRQn );
}

