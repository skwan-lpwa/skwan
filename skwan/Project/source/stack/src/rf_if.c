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

#define DEBUGOUT

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
	ml7404_reg_write8(b0_RF_STATUS, 0x03 ); //Force TRX off

	//CCA Enable
	ml7404_reg_write8(b0_CCA_CTRL, 0x10 );

	//RX_ON
	//Start CCA
	ml7404_reg_write8(b0_RF_STATUS, 0x06 );
}

void	ml7404_go_rx_mode(void)
{	
	ml7404_change_state_and_wait(0x06, TRUE);
}

void	ml7404_go_tx_mode(void)
{
	ml7404_change_state_and_wait(0x09, TRUE);
}

void 	ml7404_trx_off(void)
{
	ml7404_change_state_and_wait(0x03, TRUE);
}


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
	} else {
	  	return FALSE;
	}
	
	//check current status
	stat = ml7404_reg_read8(b0_RF_STATUS);
	if( (stat & cond) == cond ) return TRUE;

	if( mode == 0x06 ){
		//受信パケットRSSI値設定
		//ED_RSLT_SET = 1;
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

SK_UB	ml7404_get_rssi(void)
{
  	SK_UB rssi;
	
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
/**
ch 0 -> 920.7MHz
...
...
ch 36 -> 927.9MHz

チャネル周波数= CH#0周波数 + チャネル間隔 * チャネル設定

CH#0周波数 = 920.7
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
 	//ml7404_trx_off();
	  
  	//RC32K_EN = 0
  	ml7404_reg_write8(b0_CLK_SET2,(ml7404_reg_read8(b0_CLK_SET2) & ~(0x01<<3)));
	
  	//RF sleep1 mode
  	ml7404_reg_write8(b0_SLEEP_WU_SET, (ml7404_reg_read8(b0_SLEEP_WU_SET) | 0x01));
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

	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//OSC_W_SEL
	rf_reg_wr(0x08, 0x40);

	//クロック安定化待ち時間
	wait_no_timer(WAIT_1MS); 

	//CLK安定化完了待ち
	ml7404_wait_for_int_event(INT_CLOCK_STABLE, 255);
	
	//GPIO1_CTRL(EXT_CLK_OUT=OFF)
	rf_reg_wr(0x4F, 0x0);
	
	//XTAL ADJT
	//rf_reg_wr(0x62, 0x88);
	//rf_reg_wr(0x63, 0x80);
	
	//BANK6
	//0x50-51 
	rf_reg_wr(0x00, 0x33);
	rf_reg_wr(0x50, 0x01);
	rf_reg_wr(0x51, 0x00);


	//BANK3
	rf_reg_wr(0x00, 0x88);
	
	//RSSI_SEL
	rf_reg_wr(0x2C, 0x00);
	
	
	//BANK2
	rf_reg_wr(0x00, 0x44);
	
	//AAF_ADJ
	rf_reg_wr(0x11, 0x00);
	
	//VCO_I_CTRL
	rf_reg_wr(0x24, 0x0A);	

	//BANK0
	rf_reg_wr(0x00, 0x11);
	//FIFO_SET
	rf_reg_wr(0x78, 0x02);


	//
	//Frequency Band Setting
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//PLL_DIV_SET
	rf_reg_wr(0x1A, 0x00);
	
	//0x1B-1E TXFREQ_I/FH/FM/FL
	// 920.7MHz
	rf_reg_wr(0x1B, 0x19);
	rf_reg_wr(0x1C, 0x09);
	rf_reg_wr(0x1D, 0x33);
	rf_reg_wr(0x1E, 0x33);
	
	//0x1F-22 RXFREQ_I/FH/FM/FL
	rf_reg_wr(0x1F, 0x19);
	rf_reg_wr(0x20, 0x09);
	rf_reg_wr(0x21, 0x33);
	rf_reg_wr(0x22, 0x33);	
	
	//0x4d-51 VCAL RANGE
	rf_reg_wr(0x4D, 0x19);
	rf_reg_wr(0x4E, 0x05);
	rf_reg_wr(0x4F, 0x05);
	rf_reg_wr(0x50, 0xB0);		
	rf_reg_wr(0x51, 0x05);
	
	
	//BANK2
	rf_reg_wr(0x00, 0x44);
	
	//PLL_CP
	//rf_reg_wr(0x1C, 0x33);  //20170708 change
	
	//VCO_I_CTRL
	rf_reg_wr(0x24, 0x0A);
	
	//LNA setting
	//rf_reg_wr(0x2D, 0x26); //20170708 change
	
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
	rf_reg_wr(0x06, 0xCC);	//200cps
	
	//DATA_SET1
	rf_reg_wr(0x07, 0x15);	//NRZ 20170708 0x05->0x15
	
	//SYNC_CONDITION3
	rf_reg_wr(0x47, 0x10); //20170708 add
	
	//CHFIL_BW
	rf_reg_wr(0x54, 0x01);	//200khz
	
	//DC_FIL_MODE
	rf_reg_wr(0x59, 0x95); //20170708 0x15 -> 0x95
	
	//DEC_GAIN
	//rf_reg_wr(0x60, 0x18);  20170708 out
	
	//IF_FREQ
	//rf_reg_wr(0x61, 0x00); //225KHz 20170708 out
	
	//CHFIL_BW_CCA
	//rf_reg_wr(0x6A, 0x81); //400KHz 20170708 out
	
	//DC_FIL_SEL2
	//rf_reg_wr(0x6C, 0x03);  //20170708 out
	
	
	//BANK1
	rf_reg_wr(0x00, 0x22);
	
	//0x04-05 RX_RATE1
	rf_reg_wr(0x04, 0x00);
	
	//RX_RATE1
	rf_reg_wr(0x05, 0x04);
	
	//RSSI_MAG_ADJ
	//rf_reg_wr(0x13, 0x0C); //20170708 out
	
	
	//BANK2
	rf_reg_wr(0x00, 0x44);
	
	//BPSK_DLY_ADJ
	rf_reg_wr(0x19, 0x08); //20170708 0x03 -> 0x08

	//LNA_RESERVE
	rf_reg_wr(0x29, 0xE6); //20170708 add
	
	//PAREG_OLR2 
	rf_reg_wr(0x2F, 0xE7); //20170708 add
	
	//RSSI_ADJ2
	//rf_reg_wr(0x0E, 0x45); //20170708 out
	
	// AGC/RSSI_OFFSET
//20170708 out
#if 0
	rf_reg_wr(0x76, 0x8E);	
	rf_reg_wr(0x77, 0x32);
	rf_reg_wr(0x78, 0x8E);
	rf_reg_wr(0x79, 0x32);
	rf_reg_wr(0x7A, 0x8E);
	rf_reg_wr(0x7B, 0x32);
	rf_reg_wr(0x7C, 0x22);
	rf_reg_wr(0x7D, 0x47);
	rf_reg_wr(0x7E, 0x6E);	
#endif
	
	//BANK3
//20170708 out
#if 0
	rf_reg_wr(0x00, 0x88);
	
	//DIF_SET0
	rf_reg_wr(0x26, 0x35);
	
	
	//
	//BPSK Setting
	//
	//BANK6
	rf_reg_wr(0x00, 0x33);
	
	//MOD_CTRL
	rf_reg_wr(0x01, 0x01); //BPSK
	
	//MOD_DEV0-15
	rf_reg_wr(0x32, 0x00);	
	rf_reg_wr(0x33, 0x00);
	rf_reg_wr(0x34, 0x00);
	rf_reg_wr(0x35, 0x09);
	rf_reg_wr(0x36, 0x00);
	rf_reg_wr(0x37, 0x1C);
	rf_reg_wr(0x38, 0x00);
	rf_reg_wr(0x39, 0x3A);
	rf_reg_wr(0x3A, 0x00);
	rf_reg_wr(0x3B, 0x56);
	rf_reg_wr(0x3C, 0x00);
	rf_reg_wr(0x3D, 0x78);
	rf_reg_wr(0x3E, 0x00);
	rf_reg_wr(0x3F, 0x9D);
	rf_reg_wr(0x40, 0x00);
	rf_reg_wr(0x41, 0xBE);

	rf_reg_wr(0x42, 0x00);	
	rf_reg_wr(0x43, 0xD7);
	rf_reg_wr(0x44, 0x00);
	rf_reg_wr(0x45, 0xE7);
	rf_reg_wr(0x46, 0x00);
	rf_reg_wr(0x47, 0xF0);
	rf_reg_wr(0x48, 0x00);
	rf_reg_wr(0x49, 0xF6);
	rf_reg_wr(0x4A, 0x00);
	rf_reg_wr(0x4B, 0xFE);
	rf_reg_wr(0x4C, 0x01);
	rf_reg_wr(0x4D, 0x03);
	rf_reg_wr(0x4E, 0x01);
	rf_reg_wr(0x4F, 0x07);
	rf_reg_wr(0x50, 0x01);
	rf_reg_wr(0x51, 0x0A);
	
	//MOD_TIM_ADJ0-14
	rf_reg_wr(0x62, 0x03);	
	rf_reg_wr(0x63, 0x02);
	rf_reg_wr(0x64, 0x02);
	rf_reg_wr(0x65, 0x02);
	rf_reg_wr(0x66, 0x04);
	rf_reg_wr(0x67, 0x05);
	rf_reg_wr(0x68, 0x04);
	rf_reg_wr(0x69, 0x04);
	rf_reg_wr(0x6A, 0x04);
	rf_reg_wr(0x6B, 0x03);
	rf_reg_wr(0x6C, 0x02);
	rf_reg_wr(0x6D, 0x03);
	rf_reg_wr(0x6E, 0x02);
	rf_reg_wr(0x6F, 0x02);
	rf_reg_wr(0x70, 0x02);
#endif
	
	//BANK7
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
//20170708 out
#if 0
	rf_reg_wr(0x14, 0x05);  
	rf_reg_wr(0x15, 0x05); //18/18MHz
#endif
	
	//DSSS_RATE_SYNC_H-DSSS_RATE_L
//20170708 out
#if 0
	rf_reg_wr(0x17, 0x00);
	rf_reg_wr(0x18, 0x59);
	rf_reg_wr(0x19, 0x00);
	rf_reg_wr(0x1A, 0x59); //18/18MHz, chiprate = 200k
#endif
	
	//SS_SYNC_BIT8_GATE_H/L, SS_SYNC_BIT8_GATE2_H/L, SS_SYNC_BIT_GATE_H/L
	rf_reg_wr(0x1B, 0x01);	
	rf_reg_wr(0x1C, 0x00); //20170708
	rf_reg_wr(0x1D, 0x01); //20170708
	rf_reg_wr(0x1E, 0x10); //20170708
	rf_reg_wr(0x1F, 0x00); //20170708
	rf_reg_wr(0x20, 0xFC); //20170708
	

	//SS_SYNC_BIT4_GATE_H/L
//20170708 out
#if 0
	rf_reg_wr(0x21, 0x02);
	rf_reg_wr(0x22, 0x40);
#endif
	
	//SS_SYNC_LOST_GATE
	rf_reg_wr(0x23, 0x0F);

	//DSSS_SET3
	//rf_reg_wr(0x2E, 0x07); //20170708 out

	//AGC_AVE_OFST_SYNC-AGC_IIR_SET1
//20170708 out
#if 0
	rf_reg_wr(0x27, 0x18);
	rf_reg_wr(0x28, 0xA8);
	rf_reg_wr(0x29, 0x43);
	rf_reg_wr(0x2A, 0x07);
#endif
	
	//BIT8_SPDET_TH_H/L
	rf_reg_wr(0x35, 0x02);
	rf_reg_wr(0x36, 0x30);

	rf_reg_wr(0x38, 0x13); 

	
	// ---20170708 add
	//BANK10
	rf_reg_wr(0x00, 0x99);
	
	{
	  	static const SK_UB bank10_1_64[70] = 
		{
		  	0x10, 0x01, 0x00, 0x20, 0x22, 0x22, 0x33, 0x22, 0x43, 0x43, 0x54, 0x45, 0x65, 0x76, 0x87,
			0x88, 0x88, 0x77, 0x76, 0x76, 0x67, 0xA8, 0x8A, 0x58, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x30, 0x63, 0x99, 0x9A, 0x8B, 0x77, 0x86, 0x67, 0x87, 0x77, 0x76, 0x75, 
			0x87, 0x57, 0x34, 0x22, 0x22, 0x23, 0x22, 0x22, 0x22, 0x22, 0x02, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x53, 0x03, 0x02, 0x00, 0x00, 0x01, 0x01
		};
	
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
	//CRC Enable
	rf_reg_wr(0x05, rf_reg_rd(0x05) | 0x4);
	
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
	ml7404_reg_write8(b0_CCA_LVL, 240); //tmp value
	
	//doing later
	//ml7404_go_rx_mode();
	
	RF_UNLOCK();
}
