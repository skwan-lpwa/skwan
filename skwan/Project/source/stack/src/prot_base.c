/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: Protoco stack main loop
    
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

// -------------------------------------------------
//   Compiler Options
// -------------------------------------------------
#include	"compiler_options.h"

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include	"skyley_stack.h"

// -------------------------------------------------
//   Globals
// -------------------------------------------------
static SK_UB	gbSK_Base_Initialized = 0;

// -------------------------------------------------
//   ������
// -------------------------------------------------

void SK_Base_Init(SK_UW macadr1, SK_UW macadr2) {
	SK_Initialize();
	SK_PHY_Init();
	SSMac_Init(macadr1,macadr2);
	gbSK_Base_Initialized = 1;
}


// -------------------------------------------------
//   ���C�����[�v
// -------------------------------------------------

void SK_Base_Main(void) {
	if (gbSK_Base_Initialized != 0) {
		SK_PHY_Task();
		SSMac_Task();
	}
}
