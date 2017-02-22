/**
* File:		mfcc_1_4.h
* Author:	Gernot A. Fink
* Date:		26.4.2004
*
* Description:	Baseline definitions for MFCC version 1.4
**/

#ifndef __DSP_MFCC_1_4_H_INCLUDED__
#define __DSP_MFCC_1_4_H_INCLUDED__

#include "ev_mfcc.h"
/* ... based on MFCC version 1.1 */
#include "ev_mfcc_1_1.h"

/*
 * Constant Definitions
 */
#define V1_4_PREEMPH_A		1.0

#define	V1_4_CEP_WLENGTH	500

#define	V1_4_EN_HIST_MIN	0.0
#define	V1_4_EN_HIST_MAX	10.0
#define	V1_4_EN_HIST_RES	0.5

#define	V1_4_EN_HIST_PROBLOW	0.05
#define	V1_4_EN_HIST_PROBHIGH	0.95

#define	V1_4_EN_HIST_LIMIT	2000
#define	V1_4_EN_HIST_IWEIGHT	100

#define V1_4_N_CFG_PARAMS	(1 + 2)	/* vad thresh, energy dynamics */
#define V1_4_N_CH_PARAMS	V1_1_N_BASEFEATURES
#define V1_4_N_PARAMS		(V1_4_N_CFG_PARAMS + V1_4_N_CH_PARAMS)

/*
 * Function Prototypes
 */
int _dsp_mfcc_1_4_configure(dsp_fextract_t *fex, char *params);
int _dsp_mfcc_1_4(dsp_fextract_t *fex,
		                mx_real_t *features, dsp_sample_t *signal);
int _dsp_mfcc_1_4_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix);

#endif /* __DSP_MFCC_1_4_H_INCLUDED__ */
