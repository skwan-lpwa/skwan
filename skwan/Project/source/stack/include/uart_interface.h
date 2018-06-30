/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: UART print utilities
    
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

