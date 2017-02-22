/**
* Datei:	mfcc.h
* Autor:	Gernot A. Fink
* Datum:	5.12.1998
*
* Beschreibung:	allgemeine Definitionen zur Berechnung von MFCCs
**/

#ifndef __DSP_MFCC_H_INCLUDED__
#define __DSP_MFCC_H_INCLUDED__

#include "ev_real.h"
#include "ev_histogram.h"
#include "ev_dsp.h"
#include "ev_fextract.h"

#define DSP_MAX_AUDIO_CHANNELS	2

typedef struct {
	dsp_delay_t *wderiv;	/* Verzoegerungselement fuer Ableitung */

	dsp_channel_t *channel_left;	/* Kanaladaptionsmodell/parameter */
	dsp_channel_t *channel_right;	/* Kanaladaptionsmodell/parameter */
        dsp_channel_t *channel;	        /* Kanaladaptionsmodell/parameter */
        dsp_channel_t *channels[DSP_MAX_AUDIO_CHANNELS];

	mx_histogram_t *ehist_left;	/* Energiehistogramm */
	mx_histogram_t *ehist_right;	/* Energiehistogramm */
	mx_histogram_t *ehist_left_short;	/* Energiehistogramm */
	mx_histogram_t *ehist_right_short;	/* Energiehistogramm */
	mx_histogram_t *ehist;	        /* Energiehistogramm */
	mx_histogram_t *ehists[DSP_MAX_AUDIO_CHANNELS];

	mx_real_t vad_threshold;	/* threshold for VAD in [0.0 .. 1.0] */

	dsp_channel_t *noise;

	mx_histogram_t **phist;
	} dsp_mfcc_t;

dsp_fextract_t *dsp_mfcc_create(dsp_fextract_t *fex, char *param);
void dsp_mfcc_destroy(dsp_fextract_t *fex);
void dsp_mfcc_reset(dsp_fextract_t *fex);
int dsp_mfcc_calc(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *samples);
int dsp_mfcc_fprintparam(FILE *fp, dsp_fextract_t *fex, char *key);

#endif /* __DSP_MFCC_H_INCLUDED__ */




