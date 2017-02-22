/**
* File:		shift.h
* Author:	Gernot A. Fink
* Date:		2.3.2004
*
* Description:	Definitions for channel time-shifting elements
*
**/

#ifndef __DSP_SHIFT_H_INCLUDED__
#define __DSP_SHIFT_H_INCLUDED__

#include "ev_dsp.h"

/*
 * Constants
 */
/* status of a time-shifting element ... */
#define DSP_SHIFT_STATUS_INVALID	-1
#define DSP_SHIFT_STATUS_EMPTY	0
#define DSP_SHIFT_STATUS_INIT	1
#define DSP_SHIFT_STATUS_FILLED	2

/*
 * Data Types
 */
/* type of a time-shifting element ... */
typedef struct {
	int n_channels;
	int max_delta;		/* maximum channel shift [in samples] and ... */
	int *delta;		/* 'n_channels' individual shifts (min == 0) */
	int max_samples;
	int n_samples;
	dsp_sample_t *buf;	/* multi-channel signal buffer */
	int status;		/* one of DSP_SHIFT_STATUS_*, see above */
	} dsp_shift_t;

/*
 * Function Prototypes
 */
dsp_shift_t *dsp_shift_create(int n_channels);
int dsp_shift_destroy(dsp_shift_t *shift);

int dsp_shift_configure(dsp_shift_t *shift, int *delta);
int dsp_shift_reset(dsp_shift_t *shift);

int dsp_shift_apply(dsp_sample_t *dest, dsp_shift_t *shift,
		dsp_sample_t *src, int n_samples);

#endif /* __DSP_SHIFT_H_INCLUDED__ */
