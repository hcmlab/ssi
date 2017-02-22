/**
* File:		shift.c
* Author:	Gernot A. Fink
* Date:		2.3.2004
*
* Description:	Methods for channel time-shifting elements
*
**/

#include <string.h>

#include "ev_memory.h"


#include "ev_shift.h"

dsp_shift_t *dsp_shift_create(int n_channels)
	{
	dsp_shift_t *shift;

	/* first check parameters ... */
	if (n_channels <= 0)
		return(NULL);

	/* ... allocate memory for time-shifting element ... */
	shift = rs_malloc(sizeof(dsp_shift_t), "time-shifting element");

	/* ... and initialize it */
	shift->n_channels =	n_channels;
	shift->max_delta =	-1;	/* i.e. unknown, yet */
	shift->delta = rs_malloc(shift->n_channels * sizeof(int),
				"time-shifting constants");

	shift->max_samples =	0;
	shift->n_samples =	0;
	shift->buf =		NULL;

	shift->status =		DSP_SHIFT_STATUS_INVALID;

	return(shift);
	}

int dsp_shift_destroy(dsp_shift_t *shift)
	{
	/* first check parameters ... */
	if (!shift)
		return(-1);

	/* ... then invalidate and destroy shifting element */
	if (shift->delta)
		rs_free(shift->delta);
	if (shift->buf)
		rs_free(shift->buf);
	memset(shift, -1, sizeof(dsp_shift_t));
        rs_free(shift);

	return(0);
	}

int dsp_shift_configure(dsp_shift_t *shift, int *delta)
	{
	int c, min_delta, max_delta;

	/* first check parameters ... */
	if (!shift || !delta)
		return(-1);

	/* ... calculate minimum and maximum deltas ... */
	min_delta = max_delta = delta[0];
	for (c = 1; c < shift->n_channels; c++) {
		if (delta[c] < min_delta)
			min_delta = delta[c];
		if (delta[c] > max_delta)
			max_delta = delta[c];
		}

	/* ... store normalized deltas [min -> 0] ... */
	for (c = 0; c < shift->n_channels; c++)
		shift->delta[c] = delta[c] - min_delta;

	shift->max_delta = max_delta - min_delta;

	/* ... and declare time-shifting element valid */
	shift->status =		DSP_SHIFT_STATUS_EMPTY;

	return(0);
	}

int dsp_shift_reset(dsp_shift_t *shift)
	{
	/* first check parameters ... */
	if (!shift)
		return(-1);

	/* ... and status ... */
	if (shift->status < DSP_SHIFT_STATUS_EMPTY)
		return(-1);

	/* ... clear contents ... */
	shift->n_samples = 0;
	shift->status = DSP_SHIFT_STATUS_EMPTY;

	return(0);
	}

int dsp_shift_apply(dsp_sample_t *dest, dsp_shift_t *shift,
		dsp_sample_t *src, int n_samples)
	{
	int c, i, j, valid_samples, delta;

	/* first check parameters ... */
	if (!shift || !src || !dest)
		return(-1);

	/* ... evtl. resize internal buffer ... */
	if (n_samples >= shift->max_samples) {
		shift->max_samples = n_samples;
		shift->buf = rs_realloc(shift->buf,
				shift->max_samples * shift->n_channels *
					sizeof(dsp_sample_t),
				"time-shifting buffer");
		}

	/* ... fill in data (w/o lead-in/out!) respecting deltas ... */
if (n_samples > 0) {
	for (c = 0; c < shift->n_channels; c++) {
		delta = shift->delta[c];

		for (i = 0; i < n_samples - delta; i++)
			shift->buf[shift->n_channels * (i + delta) + c] =
				src[shift->n_channels * i + c];
		}
	shift->n_samples = n_samples;

	if (shift->status == DSP_SHIFT_STATUS_EMPTY)
		shift->status = DSP_SHIFT_STATUS_INIT;
	else	shift->status = DSP_SHIFT_STATUS_FILLED;

	/* ... write time-shifted buffer (evtl. w/o invalid lead-in) ... */
	for (c = 0; c < shift->n_channels; c++) {
		if (shift->status == DSP_SHIFT_STATUS_INIT) {
			delta = shift->delta[c];

			for (i = 0; i < delta; i++)
				dest[shift->n_channels * i + c] = 0;
			}
		else	delta = 0;

		for (i = delta; i < shift->n_samples; i++)
			dest[shift->n_channels * i + c] =
				shift->buf[shift->n_channels * i + c];
		}
	valid_samples = shift->n_samples;

	/* ... fill in data for lead-in/out respecting deltas ... */
	for (c = 0; c < shift->n_channels; c++) {
		delta = shift->delta[c];

		for (i = n_samples - delta; i < n_samples; i++) {
			j = (i + delta) % n_samples;

			shift->buf[shift->n_channels * j + c] =
				src[shift->n_channels * i + c];
			}
		}

	shift->n_samples = n_samples;
	}
else	{
	/* ... write time-shifted lead-out ... */
	for (c = 0; c < shift->n_channels; c++) {
		delta = shift->delta[c];

		for (i = 0; i < delta; i++)
			dest[shift->n_channels * i + c] =
				shift->buf[shift->n_channels * i + c];

		for (i = delta; i < shift->max_delta; i++)
			dest[shift->n_channels * i + c] = 0;
		}
	valid_samples = shift->max_delta;

	shift->status = DSP_SHIFT_STATUS_EMPTY;
	shift->n_samples = 0;
	}

	return(valid_samples);
	}
