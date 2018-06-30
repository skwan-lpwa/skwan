/**
    Author:        Skyley Networks, Inc.
    Version:
    Description: type defs
    
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

#ifndef __skyley_type_h__
#define __skyley_type_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char		SK_B;
typedef signed short	SK_H;
typedef signed long		SK_W;

typedef unsigned char	SK_UB;
typedef unsigned short 	SK_UH;
typedef unsigned long	SK_UW;

typedef signed char		SK_VB;
typedef signed short	SK_VH;
typedef signed long		SK_VW;

typedef void			*SK_VP;
typedef void			(*SK_FP)(void);

typedef signed   int	SK_INT;
typedef unsigned int	SK_UINT;

typedef SK_UB			SK_BOOL;

typedef SK_INT			SK_ER;	


/* ˆê”Ê */
#ifdef NULL	
#undef NULL
#endif

#ifdef __cplusplus
#define NULL			0	
#else
#define NULL			((void *)0)
#endif

#ifndef TRUE
#define TRUE			1	
#define FALSE			0	
#endif

#define SK_E_OK			0	
#define SK_E_ER			(-1)

#define SK_E_TMOUT		(-50)

/* CPUˆË‘¶ */
#ifdef WIN32
#define SK_VP_I			SK_UW
#endif
#ifdef CPU78K0
#define SK_VP_I			SK_UH
#endif
#ifdef CPU78K0R
#define SK_VP_I			SK_UH
#endif
#ifdef CPURL78
#define SK_VP_I			SK_UH
#endif
#ifdef CPUV850
#define SK_VP_I			SK_UW
#endif
#ifdef CPUMB9AF
#define SK_VP_I			SK_UW
#endif
#ifdef CPUML7416
#define SK_VP_I			SK_UW
#endif
#ifdef CPUSTM32
#define SK_VP_I			SK_UW	
#endif
#ifdef CPUMKL16
#define SK_VP_I			SK_UW	
#endif
#ifndef SK_VP_I
#error		"SK_VP_I undefined"
#endif


// -------------------------------------------------
//   SKMAC address format
// -------------------------------------------------
#pragma pack(1)
typedef struct {
	SK_UB	m_AddrMode;
	union {
		SK_UB	m_Raw[8];
		union {
			SK_UW	m_Long[2];
		}		Extended;
		union {
			SK_UH	m_Word;
		}		Short;
	} Body;
} SK_ADDRESS;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
