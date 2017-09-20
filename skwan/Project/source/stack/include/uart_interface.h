/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
*/

#ifndef __uart_interface_h__
#define __uart_interface_h__

#ifdef __cplusplus
extern "C" {
#endif


// -------------------------------------------------
//   Includes
// -------------------------------------------------
#include	"skyley_stack.h"


// -------------------------------------------------
//   Functions
// -------------------------------------------------

#define		SK_putc(ch)		UART_PutChar(ch)
#define		SK_getlen		UART_GetLen
#define		SK_getc			UART_GetChar
void		SK_print(SK_B *str);
void		SK_print_hex(SK_UW l, SK_UB len);
void		SK_print_dec(SK_W d, SK_UB len, SK_UB pre);

#ifdef __cplusplus
}
#endif

#endif

