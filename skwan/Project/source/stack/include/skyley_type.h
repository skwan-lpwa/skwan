/**
    Copyrights(C) 2016-2017 Skyley Networks, Inc. All Rights Reserved.

    This program and the accompanying materials are made available under 
    the terms of the Eclipse Public License v1.0 which accompanies this 
    distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

    Contributors:
    Skyley Networks, Inc. - initial API, implementation and documentation
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
