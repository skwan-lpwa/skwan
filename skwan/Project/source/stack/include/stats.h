/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __stats_h__
#define __stats_h__

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include "skyley_type.h"


// -------------------------------------------------
//   typedef
// -------------------------------------------------
typedef struct _ss_stats {
	struct {
		SK_UW recv;
		SK_UW recv_drop;
		SK_UW send;
		SK_UW send_drop;
		SK_UW busy;
		SK_UW err;
	} phy; 
	struct {
		SK_UW recv_bcn;
		SK_UW recv_data;
		SK_UW recv_ack;
		SK_UW recv_metabcn;
		SK_UW recv_drop;
		SK_UW send_bcn;
		SK_UW send_data;
		SK_UW send_ack;
		SK_UW send_metabcn;
		SK_UW send_drop;
	} mac; 
	struct {
		SK_UW counter_err;
		SK_UW decode_fail;
	} sec;
	struct {
		SK_UW err;
	} mem;
} SS_STATISTICS;


// -------------------------------------------------
//   Macros for stats count up
// -------------------------------------------------
#define SS_STATS(X) 						\
{ 											\
	SS_STATISTICS* stats;					\
	stats = SSMac_GetStats();				\
	if( stats != NULL ) { 					\
		++(stats->X); 						\
	}										\
}											\

// -------------------------------------------------
//   Public functions
// -------------------------------------------------
void SSMac_InitStats(void);
SS_STATISTICS* SSMac_GetStats(void);


#ifdef __cplusplus
}
#endif

#endif
