/**
    Author:       Skyley Networks, Inc.
    Version:
    Description: Protocol stack main loop
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
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
//   èâä˙âª
// -------------------------------------------------

void SK_Base_Init(SK_UW macadr1, SK_UW macadr2) {
	SK_Initialize();
	SK_PHY_Init();
	SSMac_Init(macadr1,macadr2);
	gbSK_Base_Initialized = 1;
}


// -------------------------------------------------
//   ÉÅÉCÉìÉãÅ[Év
// -------------------------------------------------

void SK_Base_Main(void) {
	if (gbSK_Base_Initialized != 0) {
		SK_PHY_Task();
		SSMac_Task();
	}
}
