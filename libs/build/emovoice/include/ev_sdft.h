/**
* File:		sdft.h
* Author:	Gernot A. Fink
* Date:		18.3.2004
*
* Description:	Definitions for the sliding DFT
*
**/

#ifndef __DSP_SDFT_H_INCLUDED__
#define __DSP_SDFT_H_INCLUDED__

#include "ev_real.h"
#include "ev_complex.h"

/*
 * Function Prototypes
 */
int dsp_rsdft(mx_complex_t *S, size_t n,
		mx_real_t x_0, mx_real_t x_n, int down);
int dsp_sdft(mx_complex_t *S, size_t n,
		mx_complex_t z_0, mx_complex_t z_n, int down);

#endif /* __DSP_SDFT_H_INCLUDED__ */
