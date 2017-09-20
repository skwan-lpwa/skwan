/**
    Author:       Skyley Networks, Inc.
    Version:
    Description: UART print utilities
    
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
#include "compiler_options.h"

// -------------------------------------------------
//   Includes
// -------------------------------------------------
#include "uart_interface.h"


// --------------------------------------------------------
//   �P������M(�҂�)
// --------------------------------------------------------
SK_UB SK_getcw(void) {
	while(SK_getlen() == 0) { ; };
	return (SK_UB)SK_getc();
}


// --------------------------------------------------------
//   �����񑗐M
// --------------------------------------------------------
void SK_print(SK_B *str) {
	while (*str != 0) SK_putc((SK_UB)*str++);
}



// --------------------------------------------------------
//   16�i��������
// --------------------------------------------------------

static SK_UB hex[16] = "0123456789ABCDEF";

void SK_print_hex(SK_UW l, SK_UB len) {
	while(len>0) {
		SK_putc(hex[(l >> ((len-1)*4)) & 15]);
		len--;
	}
}


// --------------------------------------------------------
//   �����̏o��
// --------------------------------------------------------

void SK_print_dec(SK_W d, SK_UB len, SK_UB pre) {
	SK_UB flg;
	SK_W p;
	SK_UH n;

	p = 1; for(n=1;n<len;n++) p *= 10;
	flg = 0;
	
	if (d<0) { SK_putc('-'); d = -d; }
	
	while(p>0) {
		n = (SK_UH)((d / p) % 10);
		if (n==0) {
			if ((flg != 0) || (p == 1)) { SK_putc('0'); } else { if (pre != 0) { SK_putc(pre); } }
		} else {
			SK_putc((SK_UB)('0'+n));
			flg = 1;
		}
		d = d - (n * p);
		p = p / 10;
	}
}
