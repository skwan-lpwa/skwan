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

#ifndef	__peripheral_h__
#define	__peripheral_h__

#include "ML7416S.h"
#include "skyley_type.h"

void init_peripheral(void);

/* SPI interface */
char put_char_to_spi(char c);
char get_char_from_spi(void);

/* GPIO interface for rf */
void gpio_rf_chip_enable(void);
void gpio_rf_chip_disable(void);
void gpio_rf_spi_enable(void);
void gpio_rf_spi_disable(void);
void gpio_test_port(SK_BOOL onoff);
SK_UH gpio_rf_int_port(void);

/* IRQ interface for rf */
typedef	void (*irq_handler)(void);

void irq_rf_enable(void);
void irq_rf_disable(void);
SK_BOOL irq_rf_in_progress(void);
void irq_rf_regist_handler(irq_handler handler);

void start_dmac_spi2(void);
void end_dmac_spi2(void);

#endif
