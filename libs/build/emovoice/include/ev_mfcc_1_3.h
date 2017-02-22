/**
* File:		mfcc_1_3.h
* Author:	Gernot A. Fink
* Date:		30.4.2007
*
* Description:	Baseline definitions for MFCC version 1.3
**/

#ifndef __DSP_MFCC_1_3_H_INCLUDED__
#define __DSP_MFCC_1_3_H_INCLUDED__

#include "ev_mfcc.h"
/* ... based on MFCC version 1.2 */
#include "ev_mfcc_1_2.h"

/*
 * Constant Definitions
 */
#define V1_3_MINTIME		500
#define V1_3_MAXTIME		500

#define V1_3_N_CFG_PARAMS	(1 + 2) /* vad thresh, energy dynamics */
#define V1_3_N_CH_PARAMS	V1_1_N_BASEFEATURES
#define V1_3_N_PARAMS		(V1_3_N_CFG_PARAMS + V1_3_N_CH_PARAMS)

/*
 * Function Prototypes
 */
int _dsp_mfcc_1_3_configure(dsp_fextract_t *fex, char *params);
int _dsp_mfcc_1_3(dsp_fextract_t *fex,
		                mx_real_t *features, dsp_sample_t *signal);
int _dsp_mfcc_1_3_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix);

#endif /* __DSP_MFCC_1_3_H_INCLUDED__ */
