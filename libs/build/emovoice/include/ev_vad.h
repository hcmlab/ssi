/**
* Datei:	vad.h
* Autor:	Gernot A. Fink
* Datum:	30.8.2000
*
* Beschreibung:	Definitionen zur "voice activity detection"
**/

#ifndef __DSP_VAD_H_INCLUDED__
#define __DSP_VAD_H_INCLUDED__

#include "ev_real.h"
#include "ev_histogram.h"
#include "ev_dsp.h"

/* interner Zustand der "voice activity detection" ... */
typedef enum {
	dsp_vad_no_decision,
	dsp_vad_silence,
	dsp_vad_starting,
	dsp_vad_voice,
	dsp_vad_stopping
	} dsp_vad_state_t;

/* Typ eines Datenelements zur "voice activity detection" ... */
typedef struct {
	int version;		/* Versionscode */
	int frame_len;		/* Laenge der Signalabschnitte */
	dsp_vad_state_t state;	/* interner Zustand */
	int n_no_va_frames;	/* # Frames OHNE Sprachaktivitaet */
	dsp_sample_t *signal;	/* Puffer fuer weiterverarbeiteten Frame */
	dsp_delay_t *sigbuf;	/* Signalpuffer */
	mx_histogram_t *ehist;	/* Energiehistogramm ... */
	int last_idx;		/* ... und Index der letzten Eintragung */
	} dsp_vad_t;

dsp_vad_t *dsp_vad_create(int version, int frame_len);
void dsp_vad_destroy(dsp_vad_t *vad);
void dsp_vad_reset(dsp_vad_t *vad);
int dsp_vad_calc(dsp_sample_t *voice, dsp_vad_t *vad, dsp_sample_t *samples);

#endif /* __DSP_VAD_H_INCLUDED__ */
