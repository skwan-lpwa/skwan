/*+ 
*   COPYRIGHT(C) 2014 LAPIS SEMICONDUCTOR CO., LTD.
*   ALL RIGHT RESERVED
*   Author : (LAPIS Semiconductor Inc.)
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
