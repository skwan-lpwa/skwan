/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: skwan stats system
    
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
