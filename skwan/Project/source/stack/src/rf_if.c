/**
    Author:         Skyley Networks, Inc.
    Version:
    Description:    ML7404 register access and RF initialize
    
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

// -------------------------------------------------
//   Compiler Options
// -------------------------------------------------
#include "compiler_options.h"


// -------------------------------------------------
//   Includes
// -------------------------------------------------
#include "ML7416S.h"
#include "spi2.h"

#include "rf_interface.h"
#include "hardware.h"
#include "skyley_stack.h"

#define USE_DMAC

//#define DEBUGOUT

#ifndef DEBUGOUT
#define SK_print(X) //
#define SK_print_hex(X, Y) //
#endif


// -------------------------------------------------
//	 Prototypes
// -------------------------------------------------
#define WAIT_1MS 7000
static void wait_no_timer(SK_UW msec);
static void wait_msec(SK_UH msec);


// -------------------------------------------------
//   Variables
// -------------------------------------------------
#define DUMMY_BYTE    	0xDB

SK_UW gSHRGoldSeed;
SK_UW gPSDUGoldSeed;


// -------------------------------------------------
//   Register read/write
// -------------------------------------------------

void rf_reg_wr(SK_UB adr, SK_UB dat) {
	SK_UB	cmd;

	cmd = adr<<1 | 0x01;

	RF_Activate_SEN();

	spi2RegAccess(cmd, dat);

	RF_Deactivate_SEN();

    return;
}


void rf_reg_block_wr(SK_UB adr, SK_UB* dat, SK_UH len) {
	SK_UB	cmd;

	cmd = adr<<1 | 0x01;

	RF_Activate_SEN();

	#ifdef USE_DMAC
	start_dmac_spi2();
	spiDMACBurstAccess(cmd, dat, len);
	end_dmac_spi2();
	#else
	spiBurstAccess(cmd, dat, len);
	#endif
	
	RF_Deactivate_SEN();

    return;
}


SK_UB rf_reg_rd(SK_UB adr) {
	SK_UB rdata;
	SK_UB cmd;

	cmd = adr<<1 & 0xFE;

	RF_Activate_SEN();

	rdata = spi2RegAccess(cmd, DUMMY_BYTE);

	RF_Deactivate_SEN();

    return  rdata;
}


SK_UB   rf_reg_block_rd(SK_UB adr, SK_UB* dat, SK_UH len) {
    SK_UB cmd;

	cmd = adr<<1 & 0xFE;

	RF_Activate_SEN();
    
	#ifdef USE_DMAC
	start_dmac_spi2();
	spiDMACBurstAccess(cmd, dat, len);
	end_dmac_spi2();
	#else
	spiBurstAccess(cmd, dat, len);
	#endif
	
	RF_Deactivate_SEN();

    return  len;
}


// -------------------------------------------------
//   Read/write the spi
// -------------------------------------------------

void rf_spi_write(SK_UB dat) {	
	/*
		 -> use spiRegAccess instead of this
	*/
}


SK_UB rf_spi_read(void) {
	/*
		 -> use spiRegAccess instead of this
	*/
	return 0;
}

// -------------------------------------------------
//   read/write the register with bank
// -------------------------------------------------
SK_UB	ml7404_reg_read8(ML7404_Register reg)
{
	//if(((reg >> 8) - 1) != _current_bank){
		SK_UB tbank = (reg >> 8) - 1;
		switch(tbank){
			case 0:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x11);
				break;
			case 1:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x22);
				break;				
			case 2:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x44);
				break;				
			case 3:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x88);
				break;				
			case 4:
				break;				
			case 5:
				break;				
			case 6:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x33);
				break;				
			case 7:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x55);
				break;
			//20180518 add
			case 10:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x99);
				break;
		}
	//}
	return(rf_reg_rd(reg & 0xff));
}

void	ml7404_reg_read_block(ML7404_Register reg, SK_UB *dat, SK_UH len)
{
	rf_reg_block_rd(reg, dat, len);
}

SK_UH	ml7404_fifo_read_block(SK_UB *dat, SK_UH len)
{
	SK_UB i = 0;
	
	while(len-- != 0){
		dat[i] = ml7404_reg_read8(b0_RD_FIFO);
		i++;
	}
	
	return i;
}

void	ml7404_reg_write8(ML7404_Register reg, SK_UB value)
{
	//if(((reg >> 8) - 1) != _current_bank){
		SK_UB tbank = (reg >> 8) - 1;
		switch(tbank){
			case 0:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x11);
				break;
			case 1:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x22);
				break;				
			case 2:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x44);
				break;				
			case 3:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x88);
				break;				
			case 4:
				break;				
			case 5:
				break;				
			case 6:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x33);
				break;				
			case 7:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x55);
				break;
			//20180518 add
			case 10:
				rf_reg_wr((ML7404_Register)BANK_SEL, 0x99);
				break;
		}
	//}
	rf_reg_wr(reg & 0xff, value);
}

void	ml7404_reg_write_block(ML7404_Register reg, SK_UB *dat, SK_UH len)
{
	rf_reg_block_wr(reg, dat, len);
}

void	ml7404_fifo_write_block(SK_UB *dat, SK_UH len)
{
	while(len-- != 0){
		ml7404_reg_write8(b0_WR_TX_FIFO, *dat++);
	}
}

void	ml7404_reg_write_after_bit_or(ML7404_Register reg, SK_UB bitmask)
{
	ml7404_reg_write8(reg, ml7404_reg_read8(reg) | bitmask);
}

void	ml7404_reg_write_after_bit_and(ML7404_Register reg, SK_UB bitmask)
{
	ml7404_reg_write8(reg, ml7404_reg_read8(reg) & bitmask);
}


//
// Mode
//
void	ml7404_start_ecca(void)
{
	ml7404_reg_write_after_bit_and(b0_2DIV_CTRL, 0xFE ); //clear bit0

	//ml7404_reg_write8(b0_RF_STATUS, 0x03 ); //Force TRX off
	ml7404_trx_off();

	//bank3
	/*
	rf_reg_wr(0x00, 0x88);
	rf_reg_wr(0x2D, 1);
	rf_reg_wr(0x1B, 7);	
	*/
	
	//CCA Enable
	ml7404_reg_write8(b0_CCA_CTRL, 0x10 );

	//RX_ON
	//Start CCA
	//ml7404_reg_write8(b0_RF_STATUS, 0x06 );
	//ml7404_go_rx_mode();
	ml7404_go_ed_mode();
}

void	ml7404_go_rx_mode(void)
{	
	ml7404_change_state_and_wait(0x06, TRUE);
}

void	ml7404_go_ed_mode(void)
{	
	ml7404_change_state_and_wait(0xF6, TRUE);
}

void	ml7404_go_tx_mode(void)
{
	ml7404_change_state_and_wait(0x09, TRUE);
}

void 	ml7404_trx_off(void)
{
	ml7404_change_state_and_wait(0x03, TRUE);
}


/*
mode:
	0x03...Force TRX OFF
	0x06...RX ON
	0x08...TRX OFF
	0x09...TX ON
	0xF6...RX ON without ED_RSLT_SET=1
	*) 0xF6はskyley独自追加
*/
SK_BOOL ml7404_change_state_and_wait(SK_UB mode, SK_BOOL wait)
{
	SK_UB stat;
	SK_UB cond;
	SK_BOOL ans = TRUE;
	
	if( mode == 0x03 ){ //force trx off
	  	cond = 0x80;
	} else if( mode == 0x06 ){ //RX on
	  	cond = 0x60;
	} else if( mode == 0x08 ){ //trx off
	  	cond = 0x80;
	} else if( mode == 0x09 ){ //tx on
	  	cond = 0x90;
	} else if( mode == 0xF6 ){ //RX on w/o ED_RSLT_SET
	  	cond = 0x60;
	} else {
	  	return FALSE;
	}
	
	//check current status
	stat = ml7404_reg_read8(b0_RF_STATUS);
	if( (stat & cond) == cond ) {
		if( mode == 0x06 || mode == 0xF6 ){
			//go next
		} else {
			return TRUE;
		}
	}
	
	if( mode == 0x06 || mode == 0xF6 ){
		//受信パケットRSSI値設定
		//ED_RSLT_SET = 1;
		/*
		ED値は、ED_RSLT_SET([ED_CTRL: B0 0x41(3)])=0b0設定されている場合、RX_ON中に
		常時更新されます。ED_RSLT_SET=0b1設定ではSyncWord検出時にED値を獲得し、受信
		データのFIFOリード開始により値が更新されます。
		*/
		if( mode == 0x06 ){
			ml7404_reg_write8(b0_ED_CTRL, (ml7404_reg_read8(b0_ED_CTRL) | 0x08));
			
			//SHR Gold Seed 再設定
			ml7404_reg_write8(b7_SHR_GOLD_SEED3, (SK_UB)(gSHRGoldSeed>>24));
			ml7404_reg_write8(b7_SHR_GOLD_SEED2, (SK_UB)(gSHRGoldSeed>>16));
			ml7404_reg_write8(b7_SHR_GOLD_SEED1, (SK_UB)(gSHRGoldSeed>>8));
			ml7404_reg_write8(b7_SHR_GOLD_SEED0, (SK_UB)(gSHRGoldSeed));
	  
			//PSDU Gold Seed 再設定
			ml7404_reg_write8(b7_PSDU_GOLD_SEED3, (SK_UB)(gPSDUGoldSeed>>24));
			ml7404_reg_write8(b7_PSDU_GOLD_SEED2, (SK_UB)(gPSDUGoldSeed>>16));
			ml7404_reg_write8(b7_PSDU_GOLD_SEED1, (SK_UB)(gPSDUGoldSeed>>8));
			ml7404_reg_write8(b7_PSDU_GOLD_SEED0, (SK_UB)(gPSDUGoldSeed));
			
		} else if( mode == 0xF6 ){
			//ED_RSLT_SET = 0, ED 常時更新
			ml7404_reg_write8(b0_ED_CTRL, (ml7404_reg_read8(b0_ED_CTRL) & ~0x08));
			mode = 0x06; //switch ED mode to RX_ON here 
		} 
	}
	
	//0x06, 0xF6で、すでにRX_ON状態の場合は、Gold seed, ED_CTRL再設定だけ実行する
	if( (stat & cond) == cond ) {
	  	return TRUE;
	}
	
	//INT_STATUS_CHANGEDをポーリングするためRF割り込みマクスは必要
	RF_LOCK();
	
	//change RF status
	ml7404_reg_write8(b0_RF_STATUS, mode);
	
	if( wait == FALSE ){
		RF_UNLOCK();
		return TRUE;
	}

	//wait for RF state change event
	//正常系では10 loops以内に状態遷移完了
	ml7404_wait_for_int_event(INT_STATUS_CHANGED, 100);
	
	RF_UNLOCK();
	return ans;
}



//
// Interrupt
//
SK_UW	ml7404_get_interrupt_source(void)
{
	SK_UW	ret = 0;
	ret |= ml7404_reg_read8(b0_INT_SOURCE_GRP3);
	ret <<= 8;
	ret |= ml7404_reg_read8(b0_INT_SOURCE_GRP2);
	ret <<= 8;
	ret |= ml7404_reg_read8(b0_INT_SOURCE_GRP1);
	return(ret);
}

void	ml7404_enable_interrupt_source(SK_UW enable_bit_mask)
{
	if((enable_bit_mask & INT_STATUS_GRP1) != 0){
		ml7404_reg_write_after_bit_or(b0_INT_EN_GRP1, (enable_bit_mask & INT_STATUS_GRP1) >> 0);
	}
	if((enable_bit_mask & INT_STATUS_GRP2) != 0){
		ml7404_reg_write_after_bit_or(b0_INT_EN_GRP2, (enable_bit_mask & INT_STATUS_GRP2) >> 8);
	}
	if((enable_bit_mask & INT_STATUS_GRP3) != 0){
		ml7404_reg_write_after_bit_or(b0_INT_EN_GRP3, (enable_bit_mask & INT_STATUS_GRP3) >> 16);
	}
}

void	ml7404_disable_interrupt_source(SK_UW disable_bit_mask)
{
	if((disable_bit_mask & INT_STATUS_GRP1) != 0){
		ml7404_reg_write_after_bit_and(b0_INT_EN_GRP1, ~(((disable_bit_mask & INT_STATUS_GRP1) >> 0) & 0xff));
	}
	if((disable_bit_mask & INT_STATUS_GRP2) != 0){
		ml7404_reg_write_after_bit_and(b0_INT_EN_GRP2, ~(((disable_bit_mask & INT_STATUS_GRP2) >> 8) & 0xff));
	}
	if((disable_bit_mask & INT_STATUS_GRP3) != 0){
		ml7404_reg_write_after_bit_and(b0_INT_EN_GRP3, ~(((disable_bit_mask & INT_STATUS_GRP3) >> 16) & 0xff));
	}
}

void	ml7404_clear_interrupt_source(SK_UW clear_bit_mask)
{
  	if((clear_bit_mask & INT_STATUS_GRP1) != 0){
		ml7404_reg_write8(b0_INT_SOURCE_GRP1, ~(((clear_bit_mask & INT_STATUS_GRP1) >> 0) & 0xff));
	}
	if((clear_bit_mask & INT_STATUS_GRP2) != 0){
		ml7404_reg_write8(b0_INT_SOURCE_GRP2, ~(((clear_bit_mask & INT_STATUS_GRP2) >> 8) & 0xff));
	}
	if((clear_bit_mask & INT_STATUS_GRP3) != 0){
		ml7404_reg_write8(b0_INT_SOURCE_GRP3, ~(((clear_bit_mask & INT_STATUS_GRP3) >> 16) & 0xff));
	}
}

//
// Wait for a specified interrupt event
// note: this function must be called in RF_LOCK() context
//
SK_BOOL ml7404_wait_for_int_event(SK_UW stat, SK_UB guard_cnd)
{
	SK_UB loop;
	
	loop = guard_cnd;
	
	while(1){
		SK_UW source;
		source = ml7404_get_interrupt_source();
		if( (source & stat) != 0 ){
			ml7404_clear_interrupt_source(source);
			return TRUE;
		}
		
		loop--;
		if( loop == 0 ) break;
	} 
	
	#ifdef DEBUGOUT
	SK_print("!! int event timeout !!");
	SK_print_hex(stat, 2); SK_print(" ");
	SK_print_hex(guard_cnd, 2); SK_print("\r\n");
	#endif
			
	SS_STATS(phy.err);
	return FALSE;
}


//
// CCA
//
SK_UB	ml7404_get_cca_status(void)
{
	SK_UB	status = ml7404_reg_read8(b0_CCA_CTRL);
	return(status);
}

void	ml7404_clear_cca_status(void)
{
	/*
  	CCA完了割込みをクリアするとCCA_RSLT[1:0]([CCA_CTRL: B0 0x39(1-0)])は初期化(0b00)されます。
  	CCA_RSLT[1:0]はCCA完了割込みをクリアする前に読み出して下さい。
  	*/
  	ml7404_reg_write_after_bit_and(b0_CCA_CTRL, 0xFC);
}

void ml7404_set_cca_threshold(SK_UB threshold){
	ml7404_reg_write8(b0_CCA_LVL, threshold);  
}

SK_UB	ml7404_get_rssi(void)
{
	SK_UB rssi;
	
	rssi = ml7404_reg_read8(b0_ED_RSLT);

	return rssi;
}

SK_UB	ml7404_get_ed_value(void)
{
	SK_UB rssi;
	
	ml7404_go_ed_mode();
	
	while( (ml7404_reg_read8(b0_ED_CTRL) & 0x10) == 0 ); //wait for ED_DONE (ED CTRL bit4)
	
	rssi = ml7404_reg_read8(b0_ED_RSLT);

	return rssi;
}

//
// FIFO
//
SK_UH	ml7404_get_tx_fifo_size(void)
{
	return(64);
}

SK_UH	ml7404_get_rx_fifo_size(void)
{
	return(64);
}

void	ml7404_clear_rx_fifo(void)
{
	ml7404_reg_write8(b0_STATE_CLR, 0x82);
}

void	ml7404_clear_tx_fifo(void)
{
	ml7404_reg_write8(b0_STATE_CLR, 0x81);
}

// -------------------------------------------------
//   Channel setting
// -------------------------------------------------
/*
20171025 5単位チャンネル
ch 0 -> 921.0MHz
...
...
ch 33 -> 927.6MHz

チャネル周波数= CH#0周波数 + チャネル間隔 * チャネル設定

CH#0周波数 = 921.0
チャネル間隔 = 200KHz
*/
void	ml7404_change_channel(SK_UB channel)
{
	//BANK0
	rf_reg_wr(0x00, 0x11);	
	
	//Set CH num
	rf_reg_wr(0x09, channel);
}

// -------------------------------------------------
//   Sleep
// -------------------------------------------------
/**
SLEEP1
[SLEEP/WU_SET: B0 0x2D(5-0)] = 0b00_0111
[CLK_SET2: B0 0x03(3)] = 0b0
*/
void	ml7404_sleep(void)
{
	ml7404_trx_off();

	//RC32K_EN = 0
	ml7404_reg_write8(b0_CLK_SET2,(ml7404_reg_read8(b0_CLK_SET2) & ~(0x01<<3)));

	//RF sleep1 mode
	ml7404_reg_write8(b0_SLEEP_WU_SET, (ml7404_reg_read8(b0_SLEEP_WU_SET) | 0x01));

	/* deep sleep
	gpio_rf_chip_disable(); //RESETN Lo
	
	{ SK_UH cnt = 10000; while(cnt--); }
	
	gpio_rf_regpdin_enable(); //REGPDIN Hi
	*/
}

// -------------------------------------------------
//   Wakeup
// -------------------------------------------------

void	ml7404_wakeup(void)
{
	//wakeup trigger
	ml7404_reg_write8(b0_SLEEP_WU_SET, 0x06);

	wait_no_timer(WAIT_1MS/2); //about 500us

	//TBD
	//RF was sleep mode1, reboot is not necessary
	rf_reboot();
}


void	ml7404_reset_seeds(void)
{
	gSHRGoldSeed 	= 0x016ECD5F;
	gPSDUGoldSeed 	= 0x016ECD5F;
}


// -------------------------------------------------
//   RF initialize
// -------------------------------------------------

//count 7000 => about 1.04ms
//busy loop
static void wait_no_timer(SK_UW count){
	SK_UW     loop1;
	for(loop1=0;loop1<count;loop1++);
	return;
}


//use 10msec timer
static void wait_msec(SK_UH msec){
	SK_UW start, now;
	start  = SK_GetLongTick();
	while(1){
		now = SK_GetLongTick();
		if( (SK_UW)(now - start) > msec ){
			now = SK_GetLongTick();
			break;
		}
	}
}


void rf_init(void){
	rf_reset();

	wait_no_timer(WAIT_1MS);

	rf_reg_init();
}


//ML7404 HW Reset
void rf_reset(void){
	wait_no_timer(WAIT_1MS); //RESETN解除遅延時間 TRDL1
	
	RF_Deactivate_Reset(); //デフォルトLowのため一度hiに入れる必要あり
	
	wait_no_timer(WAIT_1MS);
	
	RF_Activate_Reset(); //Lo

	wait_no_timer(WAIT_1MS); //RESETNパルス時間
	
	RF_Deactivate_Reset(); //Hi
}


//For error recovery
void rf_phy_reset(void){
	//RST_SET RST3 = 1;
	ml7404_reg_write8(b0_RST_SET, 0x88);
}


void rf_reboot(void){
#ifdef DEBUG_STATUS
	SK_print("err:reboot\r\n");
#endif
	
	rf_init();
	SK_PHY_ChangeChannel(gnPHY_CurrentChannel);
	ml7404_go_rx_mode();
}


// -------------------------------------------------
//   Register init
// -------------------------------------------------
void rf_reg_init(void){	
	RF_LOCK();
	
	//BANK 0
	rf_reg_wr(0x00, 0x11);

	//SDO_OD=0, CMOS出力
	rf_reg_wr(0x53, 0x00);
	
	//TCXO
	rf_reg_wr(0x03, 0xC3);
	
#if 0 //20180518 fix
	//BANK1
	rf_reg_wr(0x00, 0x22);
#endif
	
	//GPIO1_CTRL(EXT_CLK_OUT=OFF)
	rf_reg_wr(0x4F, 0x0);

	
	
	//OSC_W_SEL
	//rf_reg_wr(0x08, 0x40); //skyley add

	//クロック安定化待ち時間
	wait_no_timer(WAIT_1MS); 

	//CLK安定化完了待ち
	ml7404_wait_for_int_event(INT_CLOCK_STABLE, 255);
	

	
	//PA_REG_ADJ_H/L 10dBm設定
	rf_reg_wr(0x67, 0x00);
	rf_reg_wr(0x68, 0x89);

	
	//BANK2
	rf_reg_wr(0x00, 0x44);

	//VCO_I_CTRL
	rf_reg_wr(0x24, 0x07);	

	//PLL_CP_ADJ
	rf_reg_wr(0x16, 0x55); //0x33_default(起動時Icp) 	
	rf_reg_wr(0x1C, 0x55); //0x33_default(動作時Icp) 
	
	//PA_CURR
	rf_reg_wr(0x1A, 0x6E);	
	
	//LNA_RESERVE
	rf_reg_wr(0x29, 0x5F);	
	
	//IFAMP_GAIN
	rf_reg_wr(0x2A, 0x1F);		
	
	//RFRX_RSV
	rf_reg_wr(0x2F, 0xA7);		
	
	//DSM_PHASE
	rf_reg_wr(0x32, 0x4C);		
	
	//VBG_TRIM  (注意: 基板毎に個別調整必要!!!)
	rf_reg_wr(0x38, 0x19); //REG_CORE=1.6V	
	
	
	
#if 0 //Xtalの場合
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//FIFO_SET
	rf_reg_wr(0x07, 0x3E); //Xtal 
#endif
	
	
#if 0
	//BANK0
	rf_reg_wr(0x00, 0x11);
	
	//FIFO_SET
	rf_reg_wr(0x78, 0x02);  
#endif
	
	
	// --------------------------------
	// DSSS Setting
	// --------------------------------
	//
	//Frequency Band Setting
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//PLL_DIV_SET
	rf_reg_wr(0x1A, 0x00);
	
#if 1
	//0x1B-1E TXFREQ_I/FH/FM/FL
	// 921.0MHz
	rf_reg_wr(0x1B, 0x19);
	rf_reg_wr(0x1C, 0x09);
	rf_reg_wr(0x1D, 0x55);
	rf_reg_wr(0x1E, 0x55);
	
	//0x1F-22 RXFREQ_I/FH/FM/FL
	// 921.0MHz
	rf_reg_wr(0x1F, 0x19);
	rf_reg_wr(0x20, 0x09);
	rf_reg_wr(0x21, 0x55);
	rf_reg_wr(0x22, 0x55);	
#else
	//0x1B-1E TXFREQ_I/FH/FM/FL
	// 920.7MHz
	rf_reg_wr(0x1B, 0x19);
	rf_reg_wr(0x1C, 0x09);
	rf_reg_wr(0x1D, 0x33);
	rf_reg_wr(0x1E, 0x33);
	
	//0x1F-22 RXFREQ_I/FH/FM/FL
	// 920.7MHz
	rf_reg_wr(0x1F, 0x19);
	rf_reg_wr(0x20, 0x09);
	rf_reg_wr(0x21, 0x33);
	rf_reg_wr(0x22, 0x33);	
#endif
	
	//0x4d-51 VCAL RANGE
	// Frf_MIN:919.6(920-0.4)/Fref:24/12MHzCAL Range
	rf_reg_wr(0x4D, 0x19);
	rf_reg_wr(0x4E, 0x05);
	rf_reg_wr(0x4F, 0x05);
	rf_reg_wr(0x50, 0xB0);		
	rf_reg_wr(0x51, 0x05);
		
	
	//
	//Channel Space Setting
	//
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//0x23-24    CH_SPACE
	//400kHz
	//rf_reg_wr(0x23, 0x2D);
	//rf_reg_wr(0x24, 0x83);

	//0x23-24    CH_SPACE
	//200kHz
	rf_reg_wr(0x23, 0x16);
	rf_reg_wr(0x24, 0xC1);

	
	//
	//Channnel Number
	//
	//BANK0
	rf_reg_wr(0x00, 0x11);	
	
	//ch 0
	rf_reg_wr(0x09, 0x00);
	
	
	//
	//Data rate
	//
	//BANK0
	rf_reg_wr(0x00, 0x11);	
	
	//DRATE_SET
	rf_reg_wr(0x06, 0xCC);	//200 kcps
	
	//SYNC_CONDITION1
	rf_reg_wr(0x45, 0x02);
		
	//SYNC_CONDITION3
	rf_reg_wr(0x47, 0x01);
		
	//CHFIL_BW
	//rf_reg_wr(0x54, 0x01);	
		
	//DC_FIL_MODE
	rf_reg_wr(0x59, 0x15);
		
	//DEC_GAIN
	//rf_reg_wr(0x60, 0x18);
		
	//IF_FREQ
	// 225kHz
	rf_reg_wr(0x61, 0x00);
	
	//CHFIL_BW_CCA
	// 400kHz
	//rf_reg_wr(0x6A, 0x81);

	
	
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//0x04-05 RX_RATE1
	rf_reg_wr(0x04, 0x00);
	
	//RX_RATE1
	rf_reg_wr(0x05, 0x04);
	
	//RSSI_MAG_ADJ
	rf_reg_wr(0x13, 0x0B);
	
	
	
	//BANK2
	rf_reg_wr(0x00, 0x44);
	
	//BPSK_DLY_ADJ
	rf_reg_wr(0x19, 0x08);

	//LNA_RESERVE
	rf_reg_wr(0x29, 0xC6);
	
	//PAREG_OLR2 
	rf_reg_wr(0x2F, 0xE7);
	
	//RSSI_ADJ2
	rf_reg_wr(0x0E, 0x52);
	
	// AGC/RSSI_OFFSET
	rf_reg_wr(0x76, 0x8B);	
	rf_reg_wr(0x77, 0x3C);
	rf_reg_wr(0x78, 0x8B);
	rf_reg_wr(0x79, 0x3C);
	rf_reg_wr(0x7A, 0x8B);
	rf_reg_wr(0x7B, 0x3C);
	rf_reg_wr(0x7C, 0x2B);
	rf_reg_wr(0x7D, 0x4D);
	rf_reg_wr(0x7E, 0x7D);	



	//BANK7
	// SF=64
	rf_reg_wr(0x00, 0x55);

	//DSSS_CTRL/DSSS_MODE/FEC_ENC_CTRL/FEC_DEC_CTRL/SF_CTRL
	//FEC/INTLV Disable
	//rf_reg_wr(0x01, 0x05);
	rf_reg_wr(0x01, 0x25); //PSDU 32 FEC Off
	//rf_reg_wr(0x01, 0x27); //PSDU 32, FEC ON
	//rf_reg_wr(0x01, 0x35); //PSDU 任意
	
	
	rf_reg_wr(0x02, 0x45);
	rf_reg_wr(0x03, 0x00); //FEC off
	rf_reg_wr(0x04, 0x00);
	rf_reg_wr(0x05, 0x00); //FEC off
	rf_reg_wr(0x06, 0x22); //SF=64

	//SS_AFC_RANGE_SYNC/SS_AFC_RANGE
#if 1
	rf_reg_wr(0x14, 0x05);  
	rf_reg_wr(0x15, 0x05); //18/18MHz
#endif
	
	//DSSS_RATE_SYNC_H-DSSS_RATE_L
#if 1
	rf_reg_wr(0x17, 0x00);
	rf_reg_wr(0x18, 0x59);
	rf_reg_wr(0x19, 0x00);
	rf_reg_wr(0x1A, 0x59); //18/18MHz, chiprate = 200k
#endif
	
	//SS_SYNC_BIT8_GATE_H/L, SS_SYNC_BIT8_GATE2_H/L, SS_SYNC_BIT_GATE_H/L
	rf_reg_wr(0x1B, 0x01);
	rf_reg_wr(0x1C, 0x0C);
	rf_reg_wr(0x1D, 0x01);
	rf_reg_wr(0x1E, 0x1C);
	rf_reg_wr(0x1F, 0x00);
	rf_reg_wr(0x20, 0xD2);
	

	//SS_SYNC_BIT4_GATE_H/L
#if 1
	rf_reg_wr(0x21, 0x01);
	rf_reg_wr(0x22, 0x2C);
#endif
	
	//SS_SYNC_LOST_GATE
	rf_reg_wr(0x23, 0x14);


	//AGC_AVE_OFST_SYNC-AGC_IIR_SET1
#if 1
	rf_reg_wr(0x27, 0x2A);
	rf_reg_wr(0x28, 0x9C);
	rf_reg_wr(0x29, 0x44);
	rf_reg_wr(0x2A, 0x25);
#endif
	
	//BIT8_SPDET_TH_H/L
	rf_reg_wr(0x35, 0x01);
	rf_reg_wr(0x36, 0xF4);

	//DSSS_SET7/8
	rf_reg_wr(0x37, 0x34);
	rf_reg_wr(0x38, 0x14); 

	//DSSS_SET8/9
	rf_reg_wr(0x39, 0x32);
	rf_reg_wr(0x3A, 0x25); 
	
	
	// ---20170708 add
	//BANK10
	rf_reg_wr(0x00, 0x99);
	
	{
	  	/*
	  	static const SK_UB bank10_1_64[70] = 
		{
		  	0x10, 0x01, 0x00, 0x20, 0x22, 0x22, 0x33, 0x22, 0x43, 0x43, 0x54, 0x45, 0x65, 0x76, 0x87,
			0x88, 0x88, 0x77, 0x76, 0x76, 0x67, 0xA8, 0x8A, 0x58, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x30, 0x63, 0x99, 0x9A, 0x8B, 0x77, 0x86, 0x67, 0x87, 0x77, 0x76, 0x75, 
			0x87, 0x57, 0x34, 0x22, 0x22, 0x23, 0x22, 0x22, 0x22, 0x22, 0x02, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x53, 0x03, 0x02, 0x00, 0x00, 0x01, 0x01
		};
		*/
	  /*
		static const SK_UB bank10_1_64[70] =
		{
			0x10, 0x01, 0x00, 0x20, 0x11, 0x21, 0x21, 0x21, 0x51, 0x43, 0x34, 0x44, 0x54, 0x65, 0x66, 0x66,
			0x34, 0x36, 0x55, 0x35, 0x36, 0x23, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x20, 0x22, 0x21, 0x22, 0x32, 0x53, 0x55, 0x63, 0x43, 0x66, 0x66, 0x56, 0x45, 0x44,
			0x43, 0x34, 0x15, 0x12, 0x12, 0x12, 0x11, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63,
			0x03, 0x02, 0x00, 0x00, 0x01, 0x03
		};
	*/
#if 0
		static const SK_UB bank10_1_64[70] =
		{
		  	0x10, 0x01, 0x00, 0x20, 0x22, 0x22, 0x32, 0x22, 0x72, 0x65, 0x56, 0x76, 0x85, 0xA7, 0x99, 0x89,
			 0x48, 0x68, 0x77, 0x58, 0x68, 0x33, 0x23, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x30, 0x34, 0x31, 0x33, 0x53, 0x84, 0x77, 0x86, 0x84, 0x98, 0x99, 0x7A, 0x58, 0x67,
			 0x65, 0x56, 0x27, 0x22, 0x23, 0x22, 0x22, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63,
			 0x03, 0x02, 0x00, 0x00, 0x01, 0x03
		};
#endif	
#if 1
		//20180519
		static const SK_UB bank10_1_64[70] =
		{
			0x10, 0x01, 0x00, 0x20, 0x11, 0x21, 0x21, 0x21, 0x51, 0x43, 0x34, 0x44, 0x54, 0x65, 0x66, 0x66, 
			0x34, 0x36, 0x55, 0x35, 0x36, 0x23, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x20, 0x22, 0x21, 0x22, 0x32, 0x53, 0x55, 0x63, 0x43, 0x66, 0x66, 0x56, 0x45, 0x44, 
			0x43, 0x34, 0x15, 0x12, 0x12, 0x12, 0x11, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 
			0x03, 0x02, 0x00, 0x00, 0x01, 0x03
		};
#endif
		SK_UB i;
		for( i = 0; i < sizeof(bank10_1_64); i++ ){
			rf_reg_wr(i + 1, bank10_1_64[i]); 
		}
	}

	// ---
	
	

	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//CRC_POLY3/2/1/0
	rf_reg_wr(0x16, 0x00);
	rf_reg_wr(0x17, 0x00);
	rf_reg_wr(0x18, 0x08);
	rf_reg_wr(0x19, 0x10);


	//BANK0
	rf_reg_wr(0x00, 0x11);
	
	//PKT_CTRL1
	rf_reg_wr(0x04, 0x03);	//Format D
	//rf_reg_wr(0x04, 0x02);	//Format C
	
	//PKT_CTRL2
	//rf_reg_wr(0x05, 0x10);
	//rf_reg_wr(0x05, 0x1C);	
	//Length Field=1, CRC16, TX CRC Enable
	//rf_reg_wr(0x05, 0x14);
	//Length Field=1, CRC16, TRX CRC Enable
	//rf_reg_wr(0x05, 0x1C);
	//TX, RX CRC Enable
	rf_reg_wr(0x05, rf_reg_rd(0x05) | 0xC);
	
	//ED値算出時の平均回数設定 8回
	rf_reg_wr(0x41, rf_reg_rd(0x41) | 0x03);

	
	//CRC_INT_SET debug
	//rf_reg_wr(0x13, 0x10);
	
	//
	//VCO Calibration
	//
	//VCO_CAL_START([0]1:CAL start,0:end)
	rf_reg_wr(0x6F, 0x01);

	ml7404_wait_for_int_event(INT_COMPLETE_VCO, 255);

	//
	// ---- additional setting
	//	
	//Empty/Full not used
	ml7404_reg_write8(b0_TXFIFO_THRH, 0);
	ml7404_reg_write8(b0_TXFIFO_THRL, 0);
	
	//auto tx enable
	//ml7404_reg_write8(b0_RF_STATUS_CTRL, 0x18);
	
	ml7404_clear_interrupt_source(INT_STATUS_GRP1 | INT_STATUS_GRP2 | INT_STATUS_GRP3);
	
	//enable all irq src for debug
	ml7404_enable_interrupt_source(INT_STATUS_GRP1 | INT_STATUS_GRP2 | INT_STATUS_GRP3);
	
	//Set CCA threshold
	//ml7404_reg_write8(b0_CCA_LVL, 240); //tmp value
	
	//doing later
	//ml7404_go_rx_mode();
	
	RF_UNLOCK();
}

