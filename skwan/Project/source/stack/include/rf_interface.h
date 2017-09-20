/**
    Author:			Skyley Networks, Inc.
    Version:
    Description:	ML7404 register access
    
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

#ifndef __rf_if_h__
#define __rf_if_h__

// -------------------------------------------------
//   Includes
// -------------------------------------------------
#if 1
#include "compiler_options.h"
#include "peripheral.h"
#else
#include "hw.h"
#include "irq.h"
#include "ssio.h"
#include "uart.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#endif

#include "skyley_type.h"
#include "ml7404.h"

#define RF_Activate_SEN()		{ gpio_rf_spi_enable(); }
#define RF_Deactivate_SEN()		{ gpio_rf_spi_disable(); }
#define	RF_Activate_Reset()		{ gpio_rf_chip_disable(); }
#define	RF_Deactivate_Reset()	{ gpio_rf_chip_enable(); }
#define	RF_Disable_Interrupt()	{ irq_rf_disable(); }
#define	RF_Enable_Interrupt()	{ irq_rf_enable(); }

void rf_reg_wr(SK_UB adr, SK_UB dat);
void rf_reg_block_wr(SK_UB adr, SK_UB* dat, SK_UH len);
SK_UB rf_reg_rd(SK_UB adr);
SK_UB rf_reg_block_rd(SK_UB adr, SK_UB* dat, SK_UH len);
void rf_spi_write(SK_UB dat);
SK_UB rf_spi_read(void);
void rf_reset(void);
void rf_reg_init(void);
void rf_init(void);
void rf_phy_reset(void); 
void rf_reboot(void);

// Register accessor with bank
SK_UB	ml7404_reg_read8(ML7404_Register reg);
void	ml7404_reg_read_block(ML7404_Register reg, SK_UB *dat, SK_UH len);
SK_UH	ml7404_fifo_read_block(SK_UB *dat, SK_UH len);
void	ml7404_reg_write8(ML7404_Register reg, SK_UB value);
void	ml7404_reg_write_block(ML7404_Register reg, SK_UB *dat, SK_UH len);
void	ml7404_fifo_write_block(SK_UB *dat, SK_UH len);
void	ml7404_reg_write_after_bit_or(ML7404_Register reg, SK_UB bitmask);
void	ml7404_reg_write_after_bit_and(ML7404_Register reg, SK_UB bitmask);

// Mode
void	ml7404_start_ecca(void);
void	ml7404_go_rx_mode(void);
void	ml7404_go_tx_mode(void);
void	ml7404_trx_off(void);
SK_BOOL ml7404_change_state_and_wait(SK_UB mode, SK_BOOL wait);

// Interrupt
void	ml7404_enable_interrupt_source(SK_UW enable_bit_mask);
void	ml7404_disable_interrupt_source(SK_UW disable_bit_mask);
SK_UW	ml7404_get_interrupt_source(void);
void	ml7404_clear_interrupt_source(SK_UW clear_bit_mask);
SK_BOOL ml7404_wait_for_int_event(SK_UW stat, SK_UB guard_cnd);

// CCA
SK_UB	ml7404_get_cca_status(void);
void	ml7404_clear_cca_status(void);
SK_UB	ml7404_get_rssi(void);

// FIFO
SK_UH	ml7404_get_tx_fifo_size(void);
SK_UH	ml7404_get_rx_fifo_size(void);
void	ml7404_clear_rx_fifo(void);
void	ml7404_clear_tx_fifo(void);

// Other helper
void	ml7404_change_channel(SK_UB channel);
void	ml7404_sleep(void);
void	ml7404_wakeup(void);
void	ml7404_reset_seeds(void);

#endif

