/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: Hardware abstraction interfaces
    
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

#ifndef __hardware_h__
#define __hardware_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Includes
// -------------------------------------------------
#include "compiler_options.h"

#ifdef __IAR_SYSTEMS_ICC__
    #include <intrinsics.h>		
#endif
#include	"skyley_type.h"

// -------------------------------------------------
//   Inlines
// -------------------------------------------------

#ifdef CPUML7416
	#define Hardware_DisableInterrupt()			{ __disable_irq(); }
	#define Hardware_EnableInterrupt()			{ __enable_irq(); }
	#define Hardware_No_Operation()				{ __nop; }
    #define RF_LOCK()							{ irq_rf_disable();	}
    #define RF_UNLOCK()							{ irq_rf_enable(); }
	#define TIMER_LOCK()						{ TimerLock(); }
	#define TIMER_UNLOCK()						{ TimerUnlock(); }
	#define	__SK_HARDWARE_H_CPU_DEFINED
#endif


#ifdef WIN32
	void	Hardware_DisableInterrupt(void);
	void	Hardware_EnableInterrupt(void);
	void	Hardware_IndicateError(void);
	void	Hardware_No_Operation(void);
	#define	__SK_HARDWARE_H_CPU_DEFINED
#endif

// -------------------------------------------------
//   Prototypes
// -------------------------------------------------

extern SK_H		UART_GetChar(void);
extern void		UART_PutChar(SK_UB ch);
extern SK_H		UART_GetLen(void);
extern void		UART_RxInt(SK_UB ch);
extern void		UART_Initialize(void);

extern void		Hardware_Initialize(void);
extern void		Timer_Initialize(void);
extern void		SleepTimer_Initialize(void);
extern void		SleepTimer_Go(SK_UH slots);

extern void		MCU_Sleep(void);
extern void 	MCU_Wakeup(void);
extern SK_BOOL	RF_Sleep(void);
extern void		RF_Wakeup(void);

extern void		DebugPort_Set(SK_UB num, SK_BOOL hi_low);
extern SK_UB	DebugPort_Get(SK_UB num);

extern void		TimerLock(void);
extern void 	TimerUnlock(void);
extern void		irq_rf_enable(void);
extern void		irq_rf_disable(void);


// -------------------------------------------------
//   Defined?
// -------------------------------------------------

#ifndef __SK_HARDWARE_H_CPU_DEFINED
	#error "Unknown Compiler"
#endif

#ifdef __cplusplus
}
#endif

#endif
