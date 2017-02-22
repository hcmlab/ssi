/**
* File:		dct.h
* Author:	Gernot A. Fink
* Date:		15.3.2004
*
* Description:	Definitions for the Discrete Cosine Transform (DCT)
*
**/

#ifndef __DSP_DCT_H_INCLUDED__
#define __DSP_DCT_H_INCLUDED__

#include "ev_real.h"

/*
 * Function Prototypes
 */
void dsp_dct(mx_real_t *C, mx_real_t *f, size_t n, size_t m);
void dsp_idct(mx_real_t *f, mx_real_t *C, size_t n, size_t m);

#endif /* __DSP_DCT_H_INCLUDED__ */
