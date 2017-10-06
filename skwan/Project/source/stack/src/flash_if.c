/**
    Author:       Skyley Networks, Inc.
    Version:
    Description: Flash read/write interface for ML7416
    
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

// -------------------------------------------------
//	 Include files
// -------------------------------------------------
#include "compiler_options.h"

#include <stdio.h>
#include <string.h>

#include "ML7416S.h"
#include "flash.h"
#include "flash_interface.h"


// -------------------------------------------------
//	 Prototypes
// -------------------------------------------------


// -------------------------------------------------
//	 Variable
// -------------------------------------------------


// -------------------------------------------------
//	 FlashBlockWrite
// -------------------------------------------------
SK_UB FlashBlockWrite(SK_UW sector, SK_UH bank, SK_UB* data, SK_UH size){
	SK_UW ret;
	SK_UW start_addr;
	SK_UW end_addr;
	SK_UW addr;
	SK_UW write_len;
	SK_UW write_addr;
	SK_UB *data_buf;
	SK_UW write_data;
	SK_UB *verify_addr;
	
	if( sector > MAX_BLOCK_NUM ) return 1;
	
	if( size > FLASH_BLOCK_SIZE ) return 1;
	
	//1 word = 4 SK_UBs
	if( size % 4 != 0 ) return 1;
	
	if( data == NULL ) return 1;
	
	//
	// sector erase
	//
	start_addr = BLOCK_BASE_ADDR + ((SK_UW)FLASH_BLOCK_SIZE * (SK_UW)sector);
	end_addr = start_addr + FLASH_BLOCK_SIZE;
	addr = start_addr;

	Hardware_DisableInterrupt();
	
	while( addr < end_addr ){
		ret = FLC_EraseSector( UX_FLC0, addr );
		if( ret != FLC_RESULT_OK ){
			Hardware_EnableInterrupt();
			return( 1 );
		}
		addr += FLASH_BLOCK_SIZE;
	}

	//
	// write data
	//
	write_addr = start_addr;
	write_len = size;         //MAX: 256 SK_UBs
	data_buf = data;

	while( write_len != 0 ){
		write_data = (SK_UW)(data_buf[0]) |
					(SK_UW)(data_buf[1] << 8) |
					(SK_UW)(data_buf[2] << 16)|
					(SK_UW)(data_buf[3] << 24);
		data_buf += 4;
		ret = FLC_WriteFlash( UX_FLC0, write_addr, (uint32_t*)&write_data, 4 );
        
		if( ret != FLC_RESULT_OK ){
			Hardware_EnableInterrupt();
			return( 1 );
		}
		write_addr += 4;
		write_len -= 4;
    }
	
	//
	// verify
	//
	verify_addr = (SK_UB*)start_addr;
	write_len = size;
	data_buf = data;

	while( write_len != 0 ){
		if( *data_buf != *verify_addr ){
			Hardware_EnableInterrupt();
			return( 1 );
		}
		verify_addr++;
		data_buf++;
		write_len -= 1;
	}

	Hardware_EnableInterrupt();
	return 0;
}

// -------------------------------------------------
//	 FlashBlockRead
// -------------------------------------------------
SK_UB FlashBlockRead(SK_UW sector, SK_UH bank, SK_UB* data, SK_UH size){
    SK_UB *start_addr_ptr;
	SK_UH i;
	
	if( sector > MAX_BLOCK_NUM ) return 1;
	
	if( size > FLASH_BLOCK_SIZE ) return 1;
	
	//1 word = 4 SK_UBs
	if( size % 4 != 0 ) return 1;
	
	if( data == NULL ) return 1;
	
	start_addr_ptr = (SK_UB*)(BLOCK_BASE_ADDR + ((SK_UW)FLASH_BLOCK_SIZE * (SK_UW)sector));

	for( i = 0; i < size; i++ ){
		data[i] = start_addr_ptr[i];
	}
	
	return 0;
}

