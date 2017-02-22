/**
* Datei:	fextract.h
* Autor:	Gernot A. Fink
* Datum:	5.12.1998
*
* Beschreibung:	allgemeine Definitionen zur Merkmalsberechnung
**/

#ifndef __DSP_FEXTRACT_H_INCLUDED__
#define __DSP_FEXTRACT_H_INCLUDED__

#include "ev_real.h"

/* Type der Merkmalsberechnung ... */
typedef enum {
	dsp_fextype_undefined,	/* undefiniert */
	dsp_fextype_MFCC	/* Mel-Frequency-Cepstral-Coefficients */
	} dsp_fextype_t;

extern char *dsp_fextype_text[]; /* Beschreibung zu 'mm_fextype_t' */

typedef struct {
	int type;		/* Merkmalsberechnungstyp ... */
	int version;		/* ... und Version */
	int samplerate;		/* Abtastrate ... */
	int n_channels;		/* ... # Kanaele (1=mono, 2=stereo) ... */
	int frame_len;		/* ... # Abtastwerte pro Frame ... */
	int frame_shift;	/* ... sowie Frameverschiebung */
	int n_features;		/* Gesamtanzahl berechneter Merkmale */
	void *config;		/* spezielle Konfigurationsparameter */
	} dsp_fextract_t;

dsp_fextract_t *dsp_fextract_create(dsp_fextype_t type, int version,
		char *param);
void dsp_fextract_destroy(dsp_fextract_t *fex);
void dsp_fextract_reset(dsp_fextract_t *fex);
int dsp_fextract_calc(dsp_fextract_t *fex, mx_real_t *features, dsp_sample_t *samples);
int dsp_fextract_fprintparam(FILE *fp, dsp_fextract_t *fex, char *key);

#endif /* __DSP_FEXTRACT_H_INCLUDED__ */
