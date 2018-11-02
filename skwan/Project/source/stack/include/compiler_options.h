/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: compiler options
    
    Copyrights(C) 2016-2018 Skyley Networks, Inc. All Rights Reserved.

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

#ifndef	__compiler_options_h__
#define	__compiler_options_h__

#ifdef __cplusplus
extern "C" {
#endif

#define CPUML7416			1
#define SKWAN				1
//#define ENDDEVICE			1
//#define USE_STATION_DB	1


//IEEE 802.15.4K 
#if defined(CPUML7416)	
	// ------------------------------------------
	// これらのマクロはプラットフォーム毎に適切に定義すること
	//
	#define PSDU 						32
	#define DEF_PAYLOAD_LEN				19
	#define DEF_BEACON_TRANS_TIME	 	100000
	#define BASE_PHY_TYPE				2
	// ------------------------------------------

	//
	// GPIO debug mode
	//
	#if defined(ENDDEVICE)
		//#define DEBUG_GPIO_SLOT
		//#define DEBUG_GPIO_TRX
		#define DEBUG_GPIO_SLEEP
		//#define DEBUG_GPIO_TIMER
	#else
		#define DEBUG_GPIO_SLOT
		//#define DEBUG_GPIO_TRX
		#define DEBUG_GPIO_TIMER
		#define DEBUG_GPIO_RFTX
	#endif
	
	//
	// Memory configuration
	//
	#if defined(ENDDEVICE)
		#undef USE_STATION_DB
		#define DEF_MAX_DEVICE_SIZE		1
		#define DEF_PENDING_BUF_SIZE	4
		#define DEF_DATA_MEMBLOCK_NUM	12
	#else
		#ifdef USE_STATION_DB
			#define DEF_MAX_DEVICE_SIZE 	2
			#define DEF_PENDING_BUF_SIZE	204
			#define DEF_DATA_MEMBLOCK_NUM	212
		#else
			#define DEF_MAX_DEVICE_SIZE		818
			#define DEF_PENDING_BUF_SIZE	204
			#define DEF_DATA_MEMBLOCK_NUM	208
		#endif
	#endif
	
	// RL78 dependent. dont remove
	#define __far
#endif


//IEEE 802.15.4g 
#if defined(CPURL78)
	#pragma SFR
	#pragma NOP
	#pragma STOP
	#pragma HALT
	#pragma DI
	#pragma EI

	// ------------------------------------------
	// これらのマクロはプラットフォーム毎に適切に定義すること
	//
	#define PSDU 						255
	#define DEF_PAYLOAD_LEN				240
	#define DEF_BEACON_TRANS_TIME	 	10000
	#define BASE_PHY_TYPE				1 //15.4g
	// ------------------------------------------

	//
	// GPIO debug mode
	//
	#if defined(ENDDEVICE)
		//#define DEBUG_GPIO_SLOT
		#define DEBUG_GPIO_TRX
		#define DEBUG_GPIO_SLEEP
		//#define DEBUG_GPIO_TIMER
	#else
		#define DEBUG_GPIO_SLOT
		//#define DEBUG_GPIO_TRX
		//#define DEBUG_GPIO_TIMER
		//#define DEBUG_GPIO_RFTX
	#endif
	
	#define ADF7023			1
	#define RL7023_STICK	1
	//#define MB_RL7023_09	1
	
	//
	// Memory configuration
	//
	#if defined(ENDDEVICE)
		#define DEF_MAX_DEVICE_SIZE		1
		#define DEF_PENDING_BUF_SIZE	4
		#define DEF_DATA_MEMBLOCK_NUM	8
	#else
		#ifdef USE_STATION_DB
			#define DEF_MAX_DEVICE_SIZE 	2
			#define DEF_PENDING_BUF_SIZE	200
			#define DEF_DATA_MEMBLOCK_NUM	208
		#else
			#define DEF_MAX_DEVICE_SIZE 	200
			#define DEF_PENDING_BUF_SIZE	40
			#define DEF_DATA_MEMBLOCK_NUM	46
		#endif
	#endif
#endif


#ifdef __cplusplus
}
#endif

#endif
