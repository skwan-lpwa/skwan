/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef	__compiler_options_h__
#define	__compiler_options_h__

#ifdef __cplusplus
extern "C" {
#endif

#define CPUML7416		1
#define SKWAN			1
#define ENDDEVICE		1

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
		#define DEF_MAX_DEVICE_SIZE		1
		#define DEF_PENDING_BUF_SIZE	4
		#define DEF_DATA_MEMBLOCK_NUM	12
	#else
		#define DEF_MAX_DEVICE_SIZE		800
		#define DEF_PENDING_BUF_SIZE	200
		#define DEF_DATA_MEMBLOCK_NUM	208
	#endif
	
	// RL78 dependent. dont remove
	#define __far
#endif

#ifdef __cplusplus
}
#endif

#endif
