/**
* Datei:	mfcc_1_6.h
* Autor:	Gernot A. Fink
* Datum:	12.3.2004
*
* Beschreibung:	lokale Definitionen zur Berechnung von MFCCs Version 1.6
**/

#ifndef __DSP_MFCC_1_6_H_INCLUDED__
#define __DSP_MFCC_1_6_H_INCLUDED__

#include <stdio.h>

#include "ev_real.h"
#include "ev_histogram.h"

#include "ev_mfcc.h"
#include "ev_mfcc_1_3.h"
#include "ev_mfcc_1_5.h"

/*
 * Constants
 */
/* ... for frequency analysis and resolution ... */
/*** #define V1_6_FFT_LEN		1024 ***/
#define V1_6_FFT_LEN		512
#define V1_6_N_FRESOLUTION	((mx_real_t)V1_1_SAMPLERATE / V1_6_FFT_LEN)
#define V1_6_MIN_FREQ		(3 * V1_1_N_FRESOLUTION)

/** #define V1_6_N_FILTERS         34 **/
#define V1_6_N_FILTERS         33

/* ... for the energy histogram calculation and normalization ... */
#define V1_6_EN_HIST_MIN	0.0
#define V1_6_EN_HIST_MAX	10.0
#define V1_6_EN_HIST_RES	0.25

#define V1_6_EN_HIST_PROBLOW	0.05
#define V1_6_EN_HIST_PROBHIGH	0.95

#define V1_6_EN_HIST_LIMIT	2000
#define V1_6_EN_HIST_IWEIGHT	100

/* ... for the energy histogram calculation and normalization ... */
#define V1_6_P_HIST_MIN	0
#define V1_6_P_HIST_MAX	1
/** #define V1_6_P_HIST_RES	0.05 **/
#define V1_6_P_HIST_RES	0.01

#define V1_6_P_HIST_PROBLOW	0.05
#define V1_6_P_HIST_PROBHIGH	0.95

#define V1_6_P_HIST_FLOOR	0.05	/* def. Wert fuer min. des P-Spec. */

#define V1_6_P_HIST_LIMIT	2000
#define V1_6_P_HIST_IWEIGHT	50

#ifdef PLACE_THIS_HERE
typedef struct {
	dsp_delay_t *wderiv;	/* Verzoegerungselement fuer Ableitung */

	dsp_channel_t *channel_left;	/* Kanaladaptionsmodell/parameter */
	dsp_channel_t *channel_right;	/* Kanaladaptionsmodell/parameter */
	dsp_channel_t *channel;		/* Kanaladaptionsmodell/parameter */
	dsp_channel_t *channels[DSP_MAX_AUDIO_CHANNELS];

	mx_histogram_t *ehist_left;	/* Energiehistogramm */
	mx_histogram_t *ehist_right;	/* Energiehistogramm */
	mx_histogram_t *ehist;		/* Energiehistogramm */
	mx_histogram_t *ehists[DSP_MAX_AUDIO_CHANNELS];
	} dsp_mfcc_t;
#endif /* PLACE_THIS_HERE */

/*
 * Function Prototypes
 */
/* ... for version 1.6 */
int _dsp_mfcc_1_6_configure(dsp_fextract_t *fex, char *params);
int _dsp_mfcc_1_6(dsp_fextract_t *fex,
		mx_real_t *features, dsp_sample_t *signal);
int _dsp_mfcc_1_6_fprintparam(FILE *fp, dsp_fextract_t *fex, char *prefix);

#endif /* __DSP_MFCC_1_6_H_INCLUDED__ */
