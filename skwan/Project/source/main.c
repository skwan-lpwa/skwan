/**
    Author:       Skyley Networks, Inc.
    Version:
    Description:  Scalable-Slotted MAC (aka SSMAC) for SkWAN
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/
#include "compiler_options.h"

#include "ML7416S.h"

#include "command.h"
#include "peripheral.h"
#include "rf_interface.h"
#include "uart_interface.h"


int main(void)
{
    //ML7416 ÇÃèâä˙ê›íË( CLKÅCUART,SPI )
  	init_peripheral();

	//Init ML7404
	irq_rf_regist_handler(SK_PHY_Interrupt);
	rf_init();

	//Init protocol stack and command
	CommandInit();
	
	//Timer restart
	Timer_Initialize();
	
	//Start RX
	ml7404_go_rx_mode();

    while(1){
		CommandMain();
    }
}
