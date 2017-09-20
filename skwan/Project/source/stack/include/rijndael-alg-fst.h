/*
	********************************************************************************
	Object:  V850 port of a Rijndael ANSI C code by Rijmen/Bosselaers/Barreto
	         Header file
	Date:    October 2006
	Done by: F. Poulard / NEC Electronics
	********************************************************************************

	Authors of the original source code are mentionned here-below.
	The following adaptations have been made to realize the V850 port:
	1/ Removed function rijndaelEncryptRound() and rijndaelDecryptRound()
	   after the #ifdef INTERMEDIATE_VALUE_KAT
	
	Note: define the symbol FULL_UNROLL to unfold the round loops in the
	encryption and decryption functions.

	********************************************************************************
	**********                          CAUTION                           **********
	********************************************************************************
	THIS SOFTWARE IS PROVIDED ''AS IS'' AND FOR REFERENCE ONLY.
	NEC DOES NOT GUARANTEE ITS CORRECT OPERATION AND DISCLAIM ANY FORM OF WARRANTY.
	********************************************************************************
*/

/**
 * rijndael-alg-fst.h
 *
 * @version 3.0 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __RIJNDAEL_ALG_FST_H
#define __RIJNDAEL_ALG_FST_H

#define MAXKC	(256/32)
#define MAXKB	(256/8)
#define MAXNR	14

typedef unsigned char	u8;	
typedef unsigned short	u16;	
typedef unsigned long	u32;

long rijndaelKeySetupEnc(u32 rk[/*4*(Nr + 1)*/], const u8 cipherKey[], long keyBits);
long rijndaelKeySetupDec(u32 rk[/*4*(Nr + 1)*/], const u8 cipherKey[], long keyBits);
void rijndaelEncrypt(const u32 rk[/*4*(Nr + 1)*/], long Nr, const u8 pt[16], u8 ct[16]);
void rijndaelDecrypt(const u32 rk[/*4*(Nr + 1)*/], long Nr, const u8 ct[16], u8 pt[16]);

#endif /* __RIJNDAEL_ALG_FST_H */

