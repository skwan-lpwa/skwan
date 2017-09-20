/**************************************************************************//**
 * file     system_ML7416S.c
 * brief    CMSIS Cortex-M0+ Device Peripheral Access Layer Source File for 
 *           ML7416S.
 * version  V1.00
 * date     25 Feb 2016
 *
 * note
 * Copyright (C) 2016 LAPIS Semiconductor Co., Ltd.  All rights reserved.
 *
 * par
 * This software is provided "as is" and any expressed or implied warranties,
 * including, but not limited to, the implied warranties of merchantability
 * and fitness for a particular purpose are disclaimed.
 * LAPIS Semiconductor shall not be liable for any direct, indirect,
 * consequential or incidental damages arising from using or modifying
 * this software.
 * You can modify and use this software in whole or part on
 * your own responsibility, only for the purpose of developing the software
 * for use with microcontroller manufactured by LAPIS Semiconductor.
 *
 ******************************************************************************/

#include <stdint.h>
#include "ML7416S.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define __SYSTEM_DEFAULT_CLOCK    4000000

#define SYS_CKCON_CK_SEL_DEFAULT 0 /* ToDo: Modify Value */


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = __SYSTEM_DEFAULT_CLOCK;/*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
/* ToDo: add code to calculate the system frequency based upon the current
         register settings.
         This function can be used to retrieve the system core clock frequeny
         after user changed register sittings.                                */
}

void syscon_configSYSCLK (uint32_t sclk)            /* Set Core Clock Frequency      */
{
  /* Update frequency information */
  SystemCoreClockUpdate();

}


/**
 * Initialize the system
 *
 * param  none
 * return none
 *
 * brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
  syscon_configSYSCLK(SYS_CKCON_CK_SEL_DEFAULT);

/* ToDo: add code to initialize the system
         do not use global variables because this function is called before
         reaching pre-main. RW section maybe overwritten afterwards.          */

}

